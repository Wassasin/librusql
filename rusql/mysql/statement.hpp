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
	MYSQL_BIND get_mysql_output_bind(T& x){
		MYSQL_BIND b;
		std::memset(&b, 0, sizeof(b));
		b.buffer_type = type_traits<T>::output_type::get(x);
		b.buffer = type_traits<T>::data::get(x);
		b.buffer_length = type_traits<T>::length::get(x);
		b.is_unsigned = type_traits<T>::is_unsigned::get(x);
		return b;
	}

	struct Statement : boost::noncopyable {
		Connection& connection;
		MYSQL_STMT* statement;
		
		//TODO: Rename to input_parameters
		std::vector<MYSQL_BIND> parameters;
		std::vector<MYSQL_BIND> output_parameters;
		
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
		
		template<typename T>
		void bind_parameter(T const & v) {
			parameters.emplace_back(get_mysql_bind(v));
			//auto const& b = parameters.back();
			//auto length = b.buffer_length;
			//char* begin = (char*)b.buffer;
			//char* end = &((char*)b.buffer)[length];
			//COUT << "Bound arg: type: " << b.buffer_type << " data (length: " << length << " ): " << std::string(begin, end) <<  std::endl;
		}
		
		template<typename T, typename... Tail>
		void bind(T const & v, Tail const &... tail) {
			bind_parameter(v);
			bind(tail...);
		}
		
		//! Base case for bind
		void bind(){
			if(param_count() < parameters.size()){
				throw TooManyBoundParameters("You've bound too many parameters");
			} else if(param_count() > parameters.size()){
				throw TooFewBoundParameters("You've bound too few parameters");
			}
			bind_param(parameters.data());
		}

		template <typename T>
		void bind_result_element(T& v){
			output_parameters.emplace_back(get_mysql_output_bind(v));
		}

		template <typename T, typename ... Tail>
		void bind_results(T& v, Tail& ... tail){
			bind_result_element(v);
			bind_results(tail ...);
		}

		void bind_results(){
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
			return rusql::mysql::stmt_fetch(statement);
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
