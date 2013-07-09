#pragma once

#include <boost/noncopyable.hpp>

#include "error_checked.hpp"
#include "type_traits.hpp"

namespace rusql { namespace mysql {
	struct TooFewBoundParameters : SQLError { TooFewBoundParameters(std::string const msg) : SQLError(msg) {} };
	struct TooManyBoundParameters : SQLError { TooManyBoundParameters(std::string const msg) : SQLError(msg) {} };
	
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
	std::pair<MYSQL_BIND, OutputProcessor> get_mysql_output_bind(T& x, my_bool &is_null){
		MYSQL_BIND b;
		std::memset(&b, 0, sizeof(b));
		b.buffer_type = type_traits<T>::output_type::get(x);
		b.buffer = type_traits<T>::data::get(x);
		b.buffer_length = type_traits<T>::length::get(x);
		b.is_unsigned = type_traits<T>::is_unsigned::get(x);
		b.is_null = &is_null;
		return std::make_pair(b, type_traits<T>::output_processor::get(x));
	}

	struct Statement : boost::noncopyable {
		Connection& connection;
		MYSQL_STMT* statement;

		struct OutputHelper {
			OutputProcessor post_process;
			my_bool is_null;
		};

		//TODO: Rename to input_parameters
		std::vector<MYSQL_BIND> parameters;
		std::vector<MYSQL_BIND> output_parameters;
		std::vector<OutputHelper> output_helpers;
		
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
		}

		template<typename T>
		void bind_parameter(T const & v) {
			parameters.emplace_back(get_mysql_bind(v));
			//auto const& b = parameters.back();
			//auto length = b.buffer_length;
			//char* begin = (char*)b.buffer;
			//char* end = &((char*)b.buffer)[length];
			//COUT << "Bound arg: type: " << b.buffer_type << " data (length: " << length << " ): " << std::string(begin, end) <<  std::endl;
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
			output_helpers.clear();
		}

		template <typename T>
		void bind_result_element(T& v){
			output_helpers.emplace_back(OutputHelper());
			OutputHelper &helper = output_helpers.back();

			auto res = get_mysql_output_bind(v, helper.is_null);
			output_parameters.emplace_back(res.first);
			helper.post_process = res.second;
			assert(output_parameters.size() == output_helpers.size());
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
			bind_result(output_parameters.data());
		}
		
		int prepare(std::string const q){
			return rusql::mysql::stmt_prepare(statement, q);
		}
		
		size_t param_count(){
			return rusql::mysql::stmt_param_count(statement);
		}
		
		my_bool bind_param(MYSQL_BIND* binds){
			return rusql::mysql::stmt_bind_param(statement, binds);
		}

		my_bool bind_result(MYSQL_BIND* binds){
			return rusql::mysql::stmt_bind_result(statement, binds);
		}

		int fetch(){
			int res = rusql::mysql::stmt_fetch(statement);
			if(res != MYSQL_NO_DATA) {
				// post-process the bind results
				for(unsigned i = 0; i < output_parameters.size(); ++i) {
					MYSQL_BIND &bound = output_parameters.at(i);
					auto &helper = output_helpers.at(i);
					helper.post_process(bound);
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
	};
}}
