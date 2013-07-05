#pragma once

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

#include <mysql/mysql.h>

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>

#define INFORMATION __FILE__ << ":" << __LINE__ << "(" << __FUNCTION__ << ")"
#define COUT std::cout << INFORMATION << "\n"
#define CERR std::cerr << INFORMATION << "\n"


namespace rusql { namespace mysql {
	struct SQLError : std::runtime_error {
		SQLError(std::string const & function, std::string const & s)
		: std::runtime_error(function + ": " + s)
		{}
	};
	
	struct ColumnNotFound : std::runtime_error {
		ColumnNotFound(std::string const msg) : std::runtime_error(msg) {}
	};
	
	struct TooFewBoundParameters : std::runtime_error {
		TooFewBoundParameters(std::string const msg) : std::runtime_error(msg) {}
	};
	
	struct TooManyBoundParameters : std::runtime_error {
		TooManyBoundParameters(std::string const msg) : std::runtime_error(msg) {}
	};
	
	struct Connection;

	//TODO: Rename to ErrorChecker
	struct error_checker {
		#ifndef RUSQL_IGNORE_SQL_ERRORS
		Connection& database;
		char const * function;
		error_checker(Connection& database, char const * f);
		~error_checker();
		#else
		constexpr error_checker(char const *) {}
		#endif
	};
	
	struct Statement;
	struct StatementErrorChecker {
		#ifndef RUSQL_IGNORE_SQL_ERRORS
		Statement& statement;
		char const * function;
		StatementErrorChecker(Statement& statement, char const * f);
		~StatementErrorChecker();
		#else
		constexpr error_checker(char const *) {}
		#endif
	};
	
	// Generic wrapper for error-handling
	#define RUSQL_WRAP(name, mysql, this_name, connection_name, error_checker_type) \
	template<typename... Args> \
	inline decltype(mysql(this_name, std::declval<Args>()...)) \
	name(Args && ... args) { \
		error_checker_type c{connection_name, #mysql " (mysql::" #name ")"}; \
		return mysql(this_name, std::forward<Args>(args)...); \
	}

	struct Connection : boost::noncopyable {
		Connection()
		{
			init();
		}

		Connection(MYSQL&& database_)
		: database(std::move(database_))
		{}

		MYSQL database;

		//TODO: rename to check_and_throw
		inline void throw_error(std::string const & function) {
			if(mysql_errno(&database)){
				char const * const error = mysql_error(&database);
				if(error){
					throw SQLError(function, error);
				}
			}
		}

		#define CONNECTION_WRAP(name, function) RUSQL_WRAP(name, function, &database, *this, error_checker)
		CONNECTION_WRAP(init, mysql_init)
		CONNECTION_WRAP(ping, mysql_ping)
		CONNECTION_WRAP(use_result, mysql_use_result)
		CONNECTION_WRAP(field_count, mysql_field_count);
		CONNECTION_WRAP(stmt_init, mysql_stmt_init);
		#undef CONNECTION_WRAP
		
		inline MYSQL* connect(std::string const host, std::string const user, std::string const password, std::string const database_, int const port, boost::optional<std::string> const unix_socket, unsigned long const client_flag){
			error_checker c(*this, __FUNCTION__);
			return mysql_real_connect(&database, host.c_str(), user.c_str(), password.c_str(), database_.c_str(), port, (unix_socket ? unix_socket->c_str() : NULL), client_flag);
		}
		
		inline int query(std::string const query_string) {
			error_checker c(*this, __FUNCTION__);
			return mysql_real_query(&database, query_string.c_str(), query_string.length());
		}
	};
	
	//! A minimalistic wrapper around MYSQL_RES, in "mysql_use_result"-mode. For a wrapper around "mysql_store_result", use MySQLStoreResult.
	//! Read the documentation of mysql for pros and cons of use versus store.
	//! Throws on:
	//! - Any function called that returns an error code
	//! - When the resultset is has no fields, but should have fields.
	//TODO: Rename to UseResult
	struct MySQLUseResult : boost::noncopyable {
		//! Grabs the current result from that connection
		MySQLUseResult(Connection* connection_)
		: connection(connection_)
		, result(connection->use_result())
		, current_row(nullptr)
		{
			if(result == nullptr){
				if(connection->field_count() != 0){
					connection->throw_error(__FUNCTION__);
				}
			}
		}
		
		MySQLUseResult(MySQLUseResult&& x)
		: connection(x.connection)
		, result(nullptr)
		, current_row(nullptr)
		{
			std::swap(result, x.result);
			std::swap(current_row, x.current_row);
		}
		
		MySQLUseResult& operator=(MySQLUseResult&& x){
			connection = x.connection;
			std::swap(result, x.result);
			std::swap(current_row, x.current_row);
			return *this;
		}
		
		~MySQLUseResult(){
			if(result){
				close();
			}
		}
		
		//! The connection we were created from (useful for errors)
		Connection* connection;
		
		//! The native-handle for the result
		MYSQL_RES* result;
		
		//! The native-handle for the result-row.
		MYSQL_ROW current_row;
		
		//! Closes and frees the set.
		void close(){
			assert(result != nullptr);
			while(fetch_row() != nullptr);
			free_result();
			result = nullptr;
		}
		
		MYSQL_ROW get_row(){
			assert(result != nullptr);
			return current_row;
		}
		
		size_t get_index(std::string const column_name){
			assert(result != nullptr);

			MYSQL_FIELD* field = nullptr;
			size_t index = 0;
			while((field = fetch_field())){
				++index;
				if(field->name == column_name){
					return index;
				}
			}
			
			throw ColumnNotFound("Column '" + column_name + "' not found");
		}
		
		template <typename T>
		T get(size_t const index){
			assert(result != nullptr);

			return boost::lexical_cast<T>(raw_get(index));
		}
		
		template <typename T>
		T get(std::string const column_name){
			assert(result != nullptr);

			return boost::lexical_cast<T>(raw_get(column_name));
		}
		
		char* raw_get(size_t const index){
			assert(result != nullptr);
			assert(current_row != nullptr);
			assert(index < num_fields());

			return current_row[index];
		}
		
		char* raw_get(std::string const column_name){
			assert(result != nullptr);

			return raw_get(get_index(column_name));
		}
		
		MYSQL_ROW fetch_row() {
			{
				error_checker e(*connection, __FUNCTION__);
				current_row = mysql_fetch_row(result);
			}
			
			/*if(current_row != nullptr){
				auto* lengths = fetch_lengths();
				auto const n = num_fields();
				for(size_t i = 0; i < n; ++i){
					COUT << "Column " << i << " is " << lengths[i] << " bytes containing: " << std::string(&current_row[i][0], &current_row[i][lengths[i]]) << std::endl;
				}
			}*/
			
			return current_row;
		}

		#define RESULT_WRAP(name, function) RUSQL_WRAP(name, function, result, *connection, error_checker)
		RESULT_WRAP(fetch_field, mysql_fetch_field)
		RESULT_WRAP(fetch_lengths, mysql_fetch_lengths);
		RESULT_WRAP(num_fields, mysql_num_fields);
		
	private:
		RESULT_WRAP(free_result, mysql_free_result) // Call close(), which also makes sure all rows are fetched.
		
		#undef RESULT_WRAP
	};
	
	template <typename T>
	struct type_traits;
	
	template <typename T>
	struct primitive_data {
		static char const* get(T const& x){
			return reinterpret_cast<char const*>(&x);
		}
	};
	
	struct std_string_data {
		static char const* get(std::string const& x){
			return x.c_str();
		}
	};
	
	struct char_pointer_data {
		static char const* get(char const* x){
			return x;
		}
	};
	
	template <typename T>
	struct optional_data_wrapper {
		static char const* get(boost::optional<T> const &x) {
			if(x) {
				return type_traits<T>::data::get(x.get());
			} else {
				return nullptr;
			}
		}
	};
	
	template <typename T>
	struct null_data {
		static char const* get(T const&) {
			return nullptr;
		}
	};
	
	template <>
	struct type_traits<uint8_t> {
		static constexpr enum_field_types type = MYSQL_TYPE_TINY;
		typedef primitive_data<uint8_t> data;
	};
	
	template <>
	struct type_traits<uint16_t> {
		static constexpr enum_field_types type = MYSQL_TYPE_SHORT;
		typedef primitive_data<uint16_t> data;
	};
	
	template <>
	struct type_traits<uint32_t> {
		static constexpr enum_field_types type = MYSQL_TYPE_LONG;
		typedef primitive_data<uint32_t> data;
	};
	
	template <>
	struct type_traits<uint64_t> {
		static constexpr enum_field_types type = MYSQL_TYPE_LONGLONG;
		typedef primitive_data<uint64_t> data;
	};
	
	template <>
	struct type_traits<int32_t> {
		static constexpr enum_field_types type = MYSQL_TYPE_LONG;
		typedef primitive_data<int32_t> data;
	};
	
	template <>
	struct type_traits<std::string>{
		static constexpr enum_field_types type = MYSQL_TYPE_STRING;
		typedef std_string_data data;
	};
	
	template <typename T>
	struct type_traits<boost::optional<T>> {
		static constexpr enum_field_types type = type_traits<T>::type;
		typedef optional_data_wrapper<T> data;
	};
	
	template <size_t size>
	struct type_traits<char[size]> {
		static constexpr enum_field_types type = MYSQL_TYPE_STRING;
		typedef char_pointer_data data;
	};
	
	template <>
	struct type_traits<boost::none_t> {
		static constexpr enum_field_types type = MYSQL_TYPE_NULL;
		typedef null_data<boost::none_t> data;
	};

	inline size_t get_field_length(double const&){
		return 0;
	}
	
	inline size_t get_field_length(std::string const& x){
		return x.length();
	}
	
	inline size_t get_field_length(boost::none_t const&){
		return 0;
	}
	
	inline size_t get_field_length(char const * const x){
		return std::strlen(x);
	}
	
	template <typename T>
	inline size_t get_field_length(boost::optional<T> const& x){
		if(x){
			return get_field_length(*x);
		} else {
			return 0;
		}
	}
	
	template <typename T>
	inline bool get_field_is_null(T const&){
		return false;
	}
	
	template <typename T>
	inline bool get_field_is_null(T const* x){
		if(x == nullptr){
			return true;
		} else {
			return false;
		}
	}
	
	template <typename T>
	inline bool get_field_is_null(boost::optional<T> const& x){
		return !x;
	}
	
	inline bool get_field_is_null(boost::none_t const&){
		return true;
	}

	static const my_bool field_is_null = 1;
	static const my_bool field_is_not_null = 0;
	
	template <typename T>
	MYSQL_BIND get_mysql_bind(T const& x){
		MYSQL_BIND b;
		std::memset(&b, 0, sizeof(b));
		b.buffer_type = type_traits<T>::type;
		// Remove constness: meaning this cannot be used with mysql_stmt_bind_result
		b.buffer = const_cast<char*>(type_traits<T>::data::get(x));
		b.is_null = const_cast<my_bool*>(get_field_is_null(x) ? &field_is_null : &field_is_not_null);
		b.buffer_length = get_field_length(x);
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
		
		inline void throw_error(std::string const & function) {
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
		MySQLUseResult bind_execute(T const &... v) {
			bind(v...);
			execute();
			return MySQLUseResult(&connection);
		}

		#define STATEMENT_WRAP(name, function) RUSQL_WRAP(name, function, statement, *this, StatementErrorChecker)
		int prepare(std::string const q){
			StatementErrorChecker e(*this, __FUNCTION__);
			return mysql_stmt_prepare(statement, q.c_str(), q.length());
		}

		STATEMENT_WRAP(param_count, mysql_stmt_param_count)
		STATEMENT_WRAP(bind_param, mysql_stmt_bind_param)
		STATEMENT_WRAP(execute, mysql_stmt_execute)
		#undef STATEMENT_WRAP

	private:
		#define STATEMENT_WRAP_CONNECTION(name, function) RUSQL_WRAP(name, function, statement, connection, error_checker)
		STATEMENT_WRAP_CONNECTION(close, mysql_stmt_close)
		#undef STATMENT_WRAP_CONNECTION
	};
}}
