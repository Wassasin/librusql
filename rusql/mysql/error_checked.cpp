#include "error_checked.hpp"

#include <string>
#include <iostream>

constexpr static bool output_calls = false;

#define BARK do { if(output_calls) std::cerr << __FUNCTION__ << std::endl; } while(false)

namespace rusql { namespace mysql {
	void clear_mysql_error(MYSQL *connection) {
		// If the MySQL server is available, ping() clears the error
		// If not, the current error is "MySQL server is not available"
		// So this is our best guess:
		mysql_ping(connection);
	}

	void check_and_throw_conn(MYSQL* connection, std::string f)
	{
		if(mysql_errno(connection)){
			std::string error = mysql_error(connection);
			if(!error.empty()){
				clear_mysql_error(connection);
				throw SQLError(f, error);
			}
		}
	}

	void check_and_throw_stmt(MYSQL_STMT* statement, std::string f)
	{
		auto const error_code = mysql_stmt_errno(statement);
		char const * const error = mysql_stmt_error(statement);
		if(error_code != 0 || error[0]){
			throw SQLError(f, error);
		}
	}

	void thread_init(void) {
		BARK;
		if(mysql_thread_init() != 0) {
			throw SQLError("mysql_thread_init failed");
		}
	}

	void thread_end(void) {
		BARK;
		mysql_thread_end();
	}
	
	#define CHECK(prefix) check_and_throw_conn(connection, std::string(prefix) + __FUNCTION__)
	#define CHECK_BEFORE CHECK("Before ")
	#define CHECK_AFTER CHECK("After ")

	#define SAFE_RETURN(stmt) { CHECK_BEFORE; auto result_ = stmt; CHECK_AFTER; return result_; }

	MYSQL* init(MYSQL* connection){
		BARK;
		SAFE_RETURN(mysql_init(connection));
	}

	void close(MYSQL* connection) {
		BARK;
		CHECK_BEFORE;
		mysql_close(connection);
		CHECK_AFTER;
	}
	
	int ping(MYSQL* connection){
		BARK;
		SAFE_RETURN(mysql_ping(connection));
	}
	
	MYSQL_RES* use_result(MYSQL* connection) {
		BARK;
		SAFE_RETURN(mysql_use_result(connection));
	}
	
	size_t field_count(MYSQL* connection){
		BARK;
		SAFE_RETURN(mysql_field_count(connection));
	}
	
	MYSQL_STMT* stmt_init(MYSQL* connection){
		BARK;
		SAFE_RETURN(mysql_stmt_init(connection));
	}
	
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
		BARK;
		auto char_ptr = [](boost::optional<std::string const> x) { return (x ? x->c_str() : nullptr); };

		SAFE_RETURN(mysql_real_connect(connection, char_ptr(host), char_ptr(user), char_ptr(password), char_ptr(database), port, char_ptr(unix_socket), client_flags));
	}
	
	void query(MYSQL* connection, std::string const query){
		BARK;
		int result;
		{
			CHECK_BEFORE;
			result = mysql_real_query(connection, query.c_str(), query.length());
			CHECK_AFTER;
		}

		if(result != 0){
			throw SQLError(std::string(__FUNCTION__) + " failed, but mysql didn't notice (function returned error, but errno and errmsg unset)");
		}
	}
	
	MYSQL_FIELD* fetch_field(MYSQL_RES* result) {
		BARK;
		return mysql_fetch_field(result);
	}
	
	MYSQL_FIELD_OFFSET field_seek(MYSQL_RES* result, MYSQL_FIELD_OFFSET offset){
		BARK;
		return mysql_field_seek(result, offset);
	}
	
	unsigned long* fetch_lengths(MYSQL_RES* result){
		BARK;
		auto const r = mysql_fetch_lengths(result);
		if(r == nullptr){
			throw SQLError(__FUNCTION__, "Failed to fetch field lengths (probably no current row: forgot to call fetch_row or no more rows)");
		}
		
		return r;
	}
	
	unsigned int num_fields(MYSQL_RES* result) {
		BARK;
		return mysql_num_fields(result);
	}
	
	void free_result(MYSQL_RES* result){
		BARK;
		mysql_free_result(result);
	}
	
	MYSQL_ROW fetch_row(MYSQL* connection, MYSQL_RES* result){
		BARK;
		SAFE_RETURN(mysql_fetch_row(result));
	}

	unsigned long long num_rows(MYSQL *connection, MYSQL_RES *result) {
		BARK;
		SAFE_RETURN(mysql_num_rows(result));
	}

	unsigned long long insert_id(MYSQL *connection) {
		BARK;
		SAFE_RETURN(mysql_insert_id(connection));
	}

	#undef CHECK
	#define CHECK(prefix) check_and_throw_stmt(statement, std::string(prefix) + __FUNCTION__)

	unsigned long stmt_param_count(MYSQL_STMT* statement){
		BARK;
		SAFE_RETURN(mysql_stmt_param_count(statement));
	}

	unsigned long stmt_field_count(MYSQL_STMT* statement){
		BARK;
		SAFE_RETURN(mysql_stmt_field_count(statement));
	}

	my_bool stmt_bind_param(MYSQL_STMT* statement, MYSQL_BIND* binds){
		BARK;
		SAFE_RETURN(mysql_stmt_bind_param(statement, binds));
	}
	
	my_bool stmt_bind_result(MYSQL_STMT* statement, MYSQL_BIND* binds){
		BARK;
		SAFE_RETURN(mysql_stmt_bind_result(statement, binds));
	}

	void stmt_fetch_column(MYSQL_STMT* statement, MYSQL_BIND* bind, unsigned int column, unsigned long offset) {
		BARK;
		CHECK_BEFORE;
		if(mysql_stmt_fetch_column(statement, bind, column, offset) != 0) {
			throw SQLError(std::string(__FUNCTION__) + " failed, but mysql didn't notice");
		}
		CHECK_AFTER;
	}
	
	my_bool stmt_close(MYSQL_STMT* statement){
		BARK;
		SAFE_RETURN(mysql_stmt_close(statement));
	}
	
	int stmt_prepare(MYSQL_STMT* statement, std::string q){
		BARK;
		SAFE_RETURN(mysql_stmt_prepare(statement, q.c_str(), q.length()));
	}
	
	unsigned long long stmt_insert_id(MYSQL_STMT* statement) {
		BARK;
		return mysql_stmt_insert_id(statement);
	}

	void stmt_store_result(MYSQL_STMT *statement) {
		BARK;
		CHECK_BEFORE;
		if(mysql_stmt_store_result(statement) != 0) {
			throw SQLError(std::string(__FUNCTION__) + " failed, but mysql didn't notice");
		}
		CHECK_AFTER;
	}

	unsigned long long stmt_num_rows(MYSQL_STMT *statement) {
		BARK;
		return mysql_stmt_num_rows(statement);
	}

	int stmt_execute(MYSQL_STMT* statement){
		BARK;

		CHECK_BEFORE;
		int result = mysql_stmt_execute(statement);
		CHECK_AFTER;

		if(result != 0){
			throw SQLError(std::string(__FUNCTION__) + " failed, but mysql didn't notice (function returned error, but errno and errmsg unset)");
		}
		return result;
	}

	int stmt_fetch(MYSQL_STMT* statement){
		BARK;

		CHECK_BEFORE;
		int result = mysql_stmt_fetch(statement);
		CHECK_AFTER;

		if(!(result == 0 || result == MYSQL_NO_DATA || result == MYSQL_DATA_TRUNCATED)){
			throw SQLError(std::string(__FUNCTION__) + "failed, but mysql didn't notice (function returned error, but errno and errmsg unset)");
		}

		return result;
	}

	MYSQL_RES *stmt_result_metadata(MYSQL_STMT *statement){
		BARK;

		CHECK_BEFORE;
		MYSQL_RES *result = mysql_stmt_result_metadata(statement);
		CHECK_AFTER;

		if(result == NULL) {
			throw SQLError(std::string(__FUNCTION__) + " failed: no meta information exists for the prepared query");
		}

		return result;
	}

	#undef CHECK
}}
