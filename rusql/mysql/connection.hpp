#pragma once

#include <string>

#include <cstring>

#include <boost/noncopyable.hpp>

#include "error_checked.hpp"

namespace rusql { namespace mysql {
	//! A wrapper around MYSQL (the struct), symbolizing a connection.
	struct Connection : boost::noncopyable {
		Connection() {
			memset(&database, 0, sizeof(MYSQL));
			init();
		}
		
		Connection(MYSQL&& database_)
		: database(std::move(database_))
		{}

		~Connection() {
			rusql::mysql::close(&database);
		}
		
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
		
		inline unsigned long long insert_id() {
			return rusql::mysql::insert_id(&database);
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
}}
