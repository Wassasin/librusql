#pragma once

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>

#include <mysql/mysql.h>

#define INFORMATION __FILE__ << ":" << __LINE__ << "(" << __FUNCTION__ << ")"
#define COUT std::cout << INFORMATION << "\n"
#define CERR std::cerr << INFORMATION << "\n"

#include "mysql_type_traits.hpp"
#include "mysql_error_checked.hpp"

namespace rusql { namespace mysql {
	struct ColumnNotFound : SQLError { ColumnNotFound(std::string const msg) : SQLError(msg) {} };
	struct TooFewBoundParameters : SQLError { TooFewBoundParameters(std::string const msg) : SQLError(msg) {} };
	struct TooManyBoundParameters : SQLError { TooManyBoundParameters(std::string const msg) : SQLError(msg) {} };
	
	//! A wrapper around MYSQL (the struct), symbolizing a connection.
	struct Connection : boost::noncopyable {
		Connection() {
			memset(&database, 0, sizeof(MYSQL));
			init();
		}

		Connection(MYSQL&& database_)
		: database(std::move(database_))
		{}

		MYSQL database;
		
		inline MYSQL* init(){
			return rusql::mysql::init(&database);
		}
		
		inline int ping(){
			return rusql::mysql::ping(&database);
		}
		
		inline MYSQL_RES* use_result(){
			return rusql::mysql::use_result(&database);
		}
		
		inline size_t field_count(){
			return rusql::mysql::field_count(&database);
		}
		
		inline MYSQL_STMT* stmt_init(){
			return rusql::mysql::stmt_init(&database);
		}
		
		inline MYSQL* connect(std::string const host, int const port, std::string const user, std::string const password, std::string const database_, unsigned long const client_flag){
			return rusql::mysql::connect(&database, host, user, password, database_, port, boost::none, client_flag);
		}
		
		inline MYSQL* connect(std::string const unix_socket, std::string const user, std::string const password, std::string const database_, unsigned long const client_flag){
			return rusql::mysql::connect(&database, boost::none, user, password, database_, 0, unix_socket, client_flag);
		}
		
		inline MYSQL* connect(std::string const database_, unsigned long const client_flag){
			return rusql::mysql::connect(&database, boost::none, boost::none, boost::none, database_, 0, boost::none, client_flag);
		}
		
		inline int query(std::string const query_string) {
			return rusql::mysql::query(&database, query_string);
		}
	};
	
	//! A  wrapper around MYSQL_RES, in "mysql_use_result"-mode. For a wrapper around "mysql_store_result", use MySQLStoreResult (doesn't exist at the moment of writing, sorry).
	//! Read the documentation of mysql for pros and cons of use versus store.
	//! Throws on:
	//! - Any function called that returns an error code
	//! - When the resultset is has no fields, but should have fields.
	struct UseResult : boost::noncopyable {
		//! Grabs the current result from that connection
		UseResult(Connection* connection_)
		: connection(connection_)
		, result(connection->use_result())
		, current_row(nullptr)
		{}
		
		UseResult(UseResult&& x)
		: connection(x.connection)
		, result(nullptr)
		, current_row(nullptr)
		{
			std::swap(result, x.result);
			std::swap(current_row, x.current_row);
		}
		
		UseResult& operator=(UseResult&& x){
			connection = x.connection;
			std::swap(result, x.result);
			std::swap(current_row, x.current_row);
			return *this;
		}
		
		~UseResult(){
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
			rusql::mysql::free_result(result);
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
			while((field = rusql::mysql::fetch_field(result))){
				if(field->name == column_name){
					return index;
				}
				++index;
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
			assert(index < rusql::mysql::num_fields(result));

			return current_row[index];
		}
		
		char* raw_get(std::string const column_name){
			assert(result != nullptr);

			return raw_get(get_index(column_name));
		}
		
		MYSQL_ROW fetch_row() {
			current_row = rusql::mysql::fetch_row(&connection->database, result);
			
			// Output the fetched row.
// 			if(current_row != nullptr){
// 				auto* lengths = fetch_lengths();
// 				auto const n = num_fields();
// 				for(size_t i = 0; i < n; ++i){
// 					COUT << "Column " << i << " is " << lengths[i] << " bytes containing: " << std::string(&current_row[i][0], &current_row[i][lengths[i]]) << std::endl;
// 				}
// 			}
			
			return current_row;
		}
	};
	
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
