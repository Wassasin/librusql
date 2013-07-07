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
		// Remove constness: meaning this cannot be used with mysql_stmt_bind_result
		b.buffer = const_cast<char*>(type_traits<T>::data::get(x));
		b.buffer_length = type_traits<T>::length::get(x);
		b.is_unsigned = type_traits<T>::is_unsigned::get(x);
		return b;
	}
	
	struct Statement : boost::noncopyable {
		Connection& connection;
		MYSQL_STMT* statement;
		
		std::vector<MYSQL_BIND> parameters;
		
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
		
		template<typename... T>
		UseResult bind_execute(T const &... v) {
			bind(v...);
			execute();
			return UseResult(&connection);
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
		
		int execute(){
			return rusql::mysql::stmt_execute(statement);
		}
		
		my_bool close(){
			auto const result = rusql::mysql::stmt_close(statement);
			statement = nullptr;
			return result;
		}
	};
}}
