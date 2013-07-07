#include "mysql_error_checked.hpp"

#include <string>

namespace rusql { namespace mysql {
	ErrorCheckerConnection::ErrorCheckerConnection(MYSQL* connection_, const char* f)
	: connection(connection_)
	, function(f) {
		check_and_throw(std::string("Before ") + function);
	}
	
	void ErrorCheckerConnection::check_and_throw(std::string const f) {
		if(mysql_errno(connection)){
			char const * const error = mysql_error(connection);
			if(error){
				throw SQLError(f, error);
			}
		}
	}
	
	ErrorCheckerConnection::~ErrorCheckerConnection() {
		check_and_throw(function);
	}
	
	ErrorCheckerStatement::ErrorCheckerStatement(MYSQL_STMT* statement_, char const* f)
	: statement(statement_)
	, function(f)
	{
		check_and_throw(std::string("Before ") + function);
	}
	
	ErrorCheckerStatement::~ErrorCheckerStatement() {
		check_and_throw(function);
	}
	
	void ErrorCheckerStatement::check_and_throw(std::string const f) {
		if(mysql_stmt_errno(statement)){
			char const * const error = mysql_stmt_error(statement);
			if(error){
				throw SQLError(f, error);
			}
		}
	}

	
	#define CHECK ErrorCheckerConnection(connection, __FUNCTION__)
	MYSQL* init(MYSQL* connection){
		CHECK;
		return mysql_init(connection);
	}
	
	int ping(MYSQL* connection){
		CHECK;
		return mysql_ping(connection);
	}
	
	MYSQL_RES* use_result(MYSQL* connection) {
		CHECK;
		return mysql_use_result(connection);
	}
	
	size_t field_count(MYSQL* connection){
		CHECK;
		return mysql_field_count(connection);
	}
	
	MYSQL_STMT* stmt_init(MYSQL* connection){
		CHECK;
		return mysql_stmt_init(connection);
	}
	
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
	) {
		CHECK;
		auto char_ptr = [](boost::optional<std::string const> x) { return (x ? x->c_str() : nullptr); };
		return mysql_real_connect(connection, char_ptr(host), char_ptr(user), char_ptr(password), char_ptr(database), port, char_ptr(unix_socket), client_flags);
	}
	
	int query(MYSQL* connection, std::string const query){
		return mysql_real_query(connection, query.c_str(), query.length());
	}
	
	//! Doesn't return errors
	MYSQL_FIELD* fetch_field(MYSQL_RES* result) {
		return mysql_fetch_field(result);
	}
	
	//! Doesn't return errors
	unsigned long* fetch_lengths(MYSQL_RES* result){
		auto const r = mysql_fetch_lengths(result);
		if(r == nullptr){
			throw SQLError(__FUNCTION__, "Failed to fetch field lengths (probably no current row: forgot to call fetch_row or no more rows)");
		}
		
		return r;
	}
	
	//! Doesn't return errors
	unsigned int num_fields(MYSQL_RES* result) {
		return mysql_num_fields(result);
	}
	
	//! Doesn't return errors
	void free_result(MYSQL_RES* result){
		mysql_free_result(result);
	}
	
	//! Needs a connection for error checking
	MYSQL_ROW fetch_row(MYSQL* connection, MYSQL_RES* result){
		CHECK;
		return mysql_fetch_row(result);
	}
	
	//! Doesn't return errors
	unsigned long stmt_param_count(MYSQL_STMT* statement){
		return mysql_stmt_param_count(statement);
	}
	#undef CHECK
	
	#define CHECK ErrorCheckerStatement(statement, __FUNCTION__)
	my_bool stmt_bind_param(MYSQL_STMT* statement, MYSQL_BIND* binds){
		CHECK;
		return mysql_stmt_bind_param(statement, binds);
	}
	
	my_bool stmt_close(MYSQL_STMT* statement){
		CHECK;
		return mysql_stmt_close(statement);
	}
	
	int stmt_execute(MYSQL_STMT* statement){
		CHECK;
		return mysql_stmt_execute(statement);
	}
	
	int stmt_prepare(MYSQL_STMT* statement, std::string q){
		CHECK;
		return mysql_stmt_prepare(statement, q.c_str(), q.length());
	}
	#undef CHECK
}}