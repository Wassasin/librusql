#pragma once

#include <boost/noncopyable.hpp>
#include <vector>
#include <iostream>
#include <memory>

#include "error_checked.hpp"
#include "type_traits.hpp"

inline std::ostream &operator<<(std::ostream &os, MYSQL_BIND const &b) {
	os << "== MYSQL_BIND " << (void*)&b << std::endl;
	auto length = b.buffer_length;
	os << "Buffer type: " << b.buffer_type << std::endl;
	os << "Buffer ptr: " << b.buffer;
	if(b.buffer != 0) {
		char* begin = (char*)b.buffer;
		char* end = &((char*)b.buffer)[length];
		os << " (data: '" << std::string(begin, end) << "')";
	}
	os << std::endl;
	os << "Buffer len: " << length << std::endl;
	return os;
}

namespace rusql { namespace mysql {
	struct TooFewBoundParameters : SQLError { TooFewBoundParameters(std::string const msg) : SQLError(msg) {} };
	struct TooManyBoundParameters : SQLError { TooManyBoundParameters(std::string const msg) : SQLError(msg) {} };
	
	struct OutputHelper {
		OutputProcessor post_process;
		my_bool is_null;
		unsigned long field_length;
	};

	template <typename T>
	MYSQL_BIND get_mysql_bind(T const& x){
		MYSQL_BIND b;
		std::memset(&b, 0, sizeof(b));
		b.buffer_type = type_traits<T>::type::get(x);
		b.buffer = type_traits<T>::data::get(const_cast<T&>(x));
		b.buffer_length = type_traits<T>::length::get(x);
		b.is_unsigned = type_traits<T>::is_unsigned::get(x);
		return b;
	}
	
	template <typename T>
	std::pair<MYSQL_BIND, OutputProcessor> get_mysql_output_bind(T& x, OutputHelper &helper){
		MYSQL_BIND b;
		std::memset(&b, 0, sizeof(b));
		b.buffer_type = type_traits<T>::output_type::get(x);
		b.buffer = type_traits<T>::output_data::get(x);
		// the buffer length is always 0 for fixed-width values like int;
		// for dynamic-width values output_data returns nullptr and we will
		// fetch_column the actual value later in field::post_processors::Fetch
		b.buffer_length = 0;
		b.is_unsigned = type_traits<T>::is_unsigned::get(x);
		b.is_null = &helper.is_null;
		b.length = &helper.field_length;
		return std::make_pair(b, type_traits<T>::output_processor::get(x));
	}

	struct Statement : boost::noncopyable {
		Connection& connection;
		MYSQL_STMT* statement;

		//TODO: Rename to input_parameters
		std::vector<MYSQL_BIND> parameters;
		std::vector<MYSQL_BIND> output_parameters;
		std::vector<OutputHelper> output_helpers;

		// Memory for all possible row contents:
		std::map<std::string, rusql::mysql::field::type::Column> auto_binds;
		
		Statement(Connection& connection_, std::string const query)
		: connection(connection_)
		, statement(connection.stmt_init())
		{
			prepare(query);
		}
		
		Statement(Statement&& x)
		: connection(x.connection)
		, statement(std::move(x.statement))
		, parameters(std::move(x.parameters))
		, output_parameters(std::move(x.output_parameters))
		, output_helpers(std::move(x.output_helpers))
		{
			x.statement = nullptr;
		}
		
		Statement& operator=(Statement&& x){
			std::swap(*this, x);
			return *this;
		}
		
		~Statement(){
			if(statement != nullptr){
				close();
			}
		}
		
		inline void check_and_throw(std::string const & function) {
			char const * const error = mysql_stmt_error(statement);
			if(error[0]){
				throw SQLError(function, error);
			}
		}
		
		//! Clear already set binds.
		void reset_bind() {
			parameters.clear();
			parameters.reserve(param_count());
		}

		template<typename T>
		void bind_parameter(T const & v) {
			parameters.emplace_back(get_mysql_bind(v));
			//std::cout << "Bound arg: " << parameters.back();
		}
		
		//! Bind parameters by vector. Resets already bound parameters first.
		template <typename T>
		void bind(std::vector<T> const & args) {
			reset_bind();
			for(const T &t : args) {
				bind_parameter(t);
			}
			bind_append();
		}

		//! Bind parameters. Resets already bound parameters first.
		template <typename... Args>
		void bind(Args const &... args) {
			reset_bind();
			bind_append(args ...);
		}

		//! Call bind_append to bind parameters without clearing already bound ones.
		//! Use regular bind() if you do want to reset the currently bound parameters.
		template<typename T, typename... Tail>
		void bind_append(T const & v, Tail const &... tail) {
			bind_parameter(v);
			bind_append(tail...);
		}
		
		//! Base case for bind_append
		void bind_append(){
			if(param_count() < parameters.size()){
				throw TooManyBoundParameters("You've bound too many parameters");
			} else if(param_count() > parameters.size()){
				throw TooFewBoundParameters("You've bound too few parameters");
			}
			bind_param(parameters.data());
		}

		//! Clear already set result binds.
		void reset_result_bind() {
			output_parameters.clear();
			output_parameters.reserve(field_count());
			output_helpers.clear();
			output_helpers.reserve(field_count());
		}

		template <typename T>
		void bind_result_element(T& v){
			output_helpers.emplace_back(OutputHelper());
			OutputHelper &helper = output_helpers.back();

			auto res = get_mysql_output_bind(v, helper);
			output_parameters.emplace_back(res.first);
			helper.post_process = res.second;
			assert(output_parameters.size() == output_helpers.size());

			//std::cout << "Bound result arg: " << output_parameters.back();
		}

		//! Bind result parameters. Resets already bound parameters first.
		template <typename... Args>
		void bind_results(Args& ... args) {
			reset_result_bind();
			bind_results_append(args ...);
		}

		//! Bind new parameters without resetting already bound parameters first.
		template <typename T, typename ... Tail>
		void bind_results_append(T& v, Tail& ... tail){
			bind_result_element(v);
			bind_results_append(tail ...);
		}

		void bind_results_append(){
			if(field_count() < output_parameters.size()){
				throw TooManyBoundParameters("You've bound too many output parameters");
			} else if(field_count() > output_parameters.size()) {
				throw TooFewBoundParameters("You've bound too few output parameters");
			}
			bind_result(output_parameters.data());
		}
		
		int prepare(std::string const q){
			auto res = rusql::mysql::stmt_prepare(statement, q);
			reset_bind();
			reset_result_bind();
			return res;
		}
		
		size_t param_count(){
			return rusql::mysql::stmt_param_count(statement);
		}

		size_t field_count(){
			return rusql::mysql::stmt_field_count(statement);
		}
		
		my_bool bind_param(MYSQL_BIND* binds){
			return rusql::mysql::stmt_bind_param(statement, binds);
		}

		my_bool bind_result(MYSQL_BIND* binds){
			return rusql::mysql::stmt_bind_result(statement, binds);
		}

		void fetch_column(MYSQL_BIND* b, unsigned int column, unsigned long offset) {
			return rusql::mysql::stmt_fetch_column(statement, b, column, offset);
		}

		int fetch(){
			int res = rusql::mysql::stmt_fetch(statement);
			if(res != MYSQL_NO_DATA) {
				// post-process the bind results
				for(unsigned i = 0; i < output_parameters.size(); ++i) {
					MYSQL_BIND &bound = output_parameters.at(i);
					auto &helper = output_helpers.at(i);
					helper.post_process(bound, *this, i);
				}
			}
			return res;
		}
		
		int execute(){
			return rusql::mysql::stmt_execute(statement);
		}

		unsigned long long insert_id() {
			return rusql::mysql::stmt_insert_id(statement);
		}

		void store_result() {
			rusql::mysql::stmt_store_result(statement);
		}

		/*! You need to call store_result() before this function returns anything other than 0. This
		 * is a MySQL limitation. */
		unsigned long long num_rows() {
			return rusql::mysql::stmt_num_rows(statement);
		}

		my_bool close(){
			auto const result = rusql::mysql::stmt_close(statement);
			statement = nullptr;
			return result;
		}

		std::shared_ptr<MYSQL_RES> result_metadata(){
			return std::shared_ptr<MYSQL_RES>(rusql::mysql::stmt_result_metadata(statement), rusql::mysql::free_result);
		}

		//! Get the column number of a column by name. Can only be called
		//! after execute().
		int column_number(std::string name) {
			auto mysql_res = result_metadata();
			int column = 0;
			while(MYSQL_FIELD *field = rusql::mysql::fetch_field(mysql_res.get())) {
				if(name == field->name) {
					return column;
				}
				column++;
			}
			throw std::runtime_error("No column with that name: " + name);
		}

		template <typename T>
		T get(std::string name) {
			int column = column_number(name);

			T result;
			rusql::mysql::OutputHelper helper;
			auto bind_info = rusql::mysql::get_mysql_output_bind(result, helper);

			MYSQL_BIND &bound = bind_info.first;
			rusql::mysql::OutputProcessor &processor = bind_info.second;

			// if this column is unbound -> fetch_column acts as if it was NULL, but should throw (TODO)
			// if T is optional -> correctly sets optional to uninitialized or initialized value
			// if this cell is NULL -> throws
			fetch_column(&bound, column, 0);

			// result is not actually used by check_null_allowed, but used to overload on optional
			check_null_allowed(bound, result);
			processor(bound, *this, column);
			return result;
		}

		template <typename T>
		void check_null_allowed(MYSQL_BIND&, boost::optional<T>&) {
			// NULL is always allowed for an optional
		}

		template <typename T>
		void check_null_allowed(MYSQL_BIND& bound, T&) {
			if(*bound.is_null) {
				// TODO: how do we find out if field is bound?
				throw std::runtime_error("Fetching a NULL cell in an non-optional variable (or maybe field is unbound?)");
			}
		}

		struct NamedBindFunctor {
			template <enum_field_types t>
			void visit(rusql::mysql::Statement &s, rusql::mysql::field::type::Column &bind) {
				typedef typename rusql::mysql::field::type::TypeName<t>::type TypeName;
				bind = TypeName();
				s.bind_result_element(boost::get<TypeName>(bind));
			}
		};

		void bind_all_self() {
			auto_binds.clear();
			reset_bind();

			auto mysql_res = result_metadata();
			while(MYSQL_FIELD *field = rusql::mysql::fetch_field(mysql_res.get())) {
				std::string name = field->name;
				auto &auto_bind = auto_binds[name];

				rusql::mysql::field::type::visit_mysql_type(field->type, NamedBindFunctor(), *this, auto_bind);
			}

			// finish the binding of these results
			bind_results_append();
		}

	};

	namespace field {
		namespace post_processors {
			template <typename T>
			struct Fetch {
				static void fetch(MYSQL_BIND &b, Statement &s, unsigned int column, T &x) {
					b.buffer = type_traits<T>::data::get(x);
					s.fetch_column(&b, column, 0);
				}
			};

			template <>
			struct Fetch<std::string> {
				static void fetch(MYSQL_BIND &b, Statement &s, unsigned int column, std::string &x){
					assert(b.length);
					x.resize(*b.length);
					b.buffer = type_traits<std::string>::data::get(x);
					b.buffer_length = x.length();
					s.fetch_column(&b, column, 0);
					b.buffer = type_traits<std::string>::output_data::get(x);
					if(b.buffer == nullptr) {
						b.buffer_length = 0;
					}
				}
			};

			template <typename T>
			struct Fetch<boost::optional<T>> {
				static void fetch(MYSQL_BIND &b, Statement &s, unsigned int column, boost::optional<T> &x) {
					auto func = type_traits<boost::optional<T>>::output_processor::get(x);
					func(b, s, column);
					b.buffer = type_traits<boost::optional<T>>::output_data::get(x);
					if(b.buffer == nullptr) {
						b.buffer_length = 0;
					}
				}
			};

			struct String {
				static OutputProcessor get(std::string &x) {
					return [&x](MYSQL_BIND &b, Statement &s, unsigned int column) {
						if(*b.is_null) {
							x.clear();
							throw std::runtime_error("Fetching a NULL cell in an non-optional variable");
						}
						Fetch<std::string>::fetch(b, s, column, x);
					};
				}
			};

			struct Optional {
				template <typename T>
				static OutputProcessor get(boost::optional<T> &x) {
					return [&x](MYSQL_BIND &b, Statement &s, unsigned int column) {
						assert(b.is_null);
						bool is_null = *b.is_null;
						if(is_null) {
							x = boost::none;
						} else {
							if(!x) {
								x = T();
							}
							Fetch<T>::fetch(b, s, column, *x);
						}
					};
				}
			};

			struct CheckNullPostProcessing {
				template <typename T>
				static OutputProcessor get(T &x) {
					return [&x](MYSQL_BIND &b, Statement &, unsigned int) {
						if(*b.is_null) {
							x = T();
							throw std::runtime_error("Fetching a NULL cell into a non-optional variable");
						}
					};
				}
			};
		}
	}


}}
