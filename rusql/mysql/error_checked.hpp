#pragma once

#include <stdexcept>

#include <mysql/mysql.h>

#include <boost/optional.hpp>

namespace rusql { namespace mysql {
	struct SQLError : std::runtime_error {
		SQLError(std::string msg)
		: std::runtime_error(msg)
		{}
		
		SQLError(std::string const & function, std::string const & s)
		: std::runtime_error(function + ": " + s)
		{}
	};
	
	struct Connection;
	struct ErrorCheckerConnection {
		MYSQL* connection;
		char const * function;
		ErrorCheckerConnection(MYSQL* connection, char const * f);
		~ErrorCheckerConnection();
		void check_and_throw(std::string const function);
	};
	
	struct Statement;
	struct ErrorCheckerStatement {
		MYSQL_STMT* statement;
		char const * function;
		ErrorCheckerStatement(MYSQL_STMT* statement, char const * f);
		~ErrorCheckerStatement();
		void check_and_throw(std::string const function);
	};
	
	#define CHECK ErrorCheckerConnection(connection, __FUNCTION__)
	MYSQL* init(MYSQL* connection);
	
	int ping(MYSQL* connection);
	
	MYSQL_RES* use_result(MYSQL* connection);
	
	size_t field_count(MYSQL* connection);
	
	MYSQL_STMT* stmt_init(MYSQL* connection);
	
	//! There's no difference for us between connect and "real_connect", so we just have one version: connect.
	MYSQL* connect(
		MYSQL* connection,
		boost::optional<std::string const> host,
		boost::optional<std::string const> user,
		boost::optional<std::string const> password,
		boost::optional<std::string const> database,
		unsigned long const port,
		boost::optional<std::string const> unix_socket,
		unsigned long client_flags
	);
	
	int query(MYSQL* connection, std::string const query);
	
	//! Doesn't return errors
	MYSQL_FIELD* fetch_field(MYSQL_RES* result);
	
	//! Doesn't return errors
	MYSQL_FIELD_OFFSET field_seek(MYSQL_RES* result, MYSQL_FIELD_OFFSET offset);
	
	//! Doesn't return errors
	unsigned long* fetch_lengths(MYSQL_RES* result);
	
	//! Doesn't return errors
	unsigned int num_fields(MYSQL_RES* result);
	
	//! Doesn't return errors
	void free_result(MYSQL_RES* result);
	
	//! Needs a connection for error checking
	MYSQL_ROW fetch_row(MYSQL* connection, MYSQL_RES* result);
	
	unsigned long long insert_id(MYSQL *connection);

	unsigned long long num_rows(MYSQL *connection, MYSQL_RES *result);

	//! Doesn't return errors
	unsigned long stmt_param_count(MYSQL_STMT* statement);

	unsigned long stmt_field_count(MYSQL_STMT* statement);
	
	my_bool stmt_bind_param(MYSQL_STMT* statement, MYSQL_BIND* binds);

	my_bool stmt_bind_result(MYSQL_STMT* statement, MYSQL_BIND* binds);
	
	int stmt_fetch_column(MYSQL_STMT* statement, MYSQL_BIND* bind, unsigned int column, unsigned long offset);

	my_bool stmt_close(MYSQL_STMT* statement);
	
	int stmt_execute(MYSQL_STMT* statement);
	
	int stmt_prepare(MYSQL_STMT* statement, std::string q);

	unsigned long long stmt_insert_id(MYSQL_STMT* statement);

	void stmt_store_result(MYSQL_STMT *statement);

	unsigned long long stmt_num_rows(MYSQL_STMT* statement);

	//! Returns non-zero when there are no more rows to fetch
	int stmt_fetch(MYSQL_STMT* statement);
}}
