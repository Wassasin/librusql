#include "error_checked.hpp"

#include <string>

constexpr static bool output_calls = false;

#define BARK do { if(output_calls) std::cerr << __FUNCTION__ << std::endl; } while(false)

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
		auto const error_code = mysql_stmt_errno(statement);
		char const * const error = mysql_stmt_error(statement);
		if(error_code != 0 || error[0]){
			throw SQLError(f, error);
		}
	}

	
	#define CHECK ErrorCheckerConnection(connection, __FUNCTION__)
	MYSQL* init(MYSQL* connection){
		BARK;
		CHECK;
		return mysql_init(connection);
	}
	
	int ping(MYSQL* connection){
		BARK;
		CHECK;
		return mysql_ping(connection);
	}
	
	MYSQL_RES* use_result(MYSQL* connection) {
		BARK;
		CHECK;
		return mysql_use_result(connection);
	}
	
	size_t field_count(MYSQL* connection){
		BARK;
		CHECK;
		return mysql_field_count(connection);
	}
	
	MYSQL_STMT* stmt_init(MYSQL* connection){
		BARK;
		CHECK;
		return mysql_stmt_init(connection);
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
		CHECK;
		auto char_ptr = [](boost::optional<std::string const> x) { return (x ? x->c_str() : nullptr); };
		return mysql_real_connect(connection, char_ptr(host), char_ptr(user), char_ptr(password), char_ptr(database), port, char_ptr(unix_socket), client_flags);
	}
	
	int query(MYSQL* connection, std::string const query){
		BARK;
		CHECK;
		return mysql_real_query(connection, query.c_str(), query.length());
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
		CHECK;
		return mysql_fetch_row(result);
	}
	
	unsigned long long insert_id(MYSQL *connection) {
		BARK;
		return mysql_insert_id(connection);
	}

	unsigned long stmt_param_count(MYSQL_STMT* statement){
		BARK;
		return mysql_stmt_param_count(statement);
	}
	#undef CHECK
	
	#define CHECK ErrorCheckerStatement(statement, __FUNCTION__)
	my_bool stmt_bind_param(MYSQL_STMT* statement, MYSQL_BIND* binds){
		BARK;
		CHECK;
		return mysql_stmt_bind_param(statement, binds);
	}
	
	my_bool stmt_bind_result(MYSQL_STMT* statement, MYSQL_BIND* binds){
		BARK;
		CHECK;
		return mysql_stmt_bind_result(statement, binds);
	}
	
	my_bool stmt_close(MYSQL_STMT* statement){
		BARK;
		CHECK;
		return mysql_stmt_close(statement);
	}
	
	int stmt_prepare(MYSQL_STMT* statement, std::string q){
		BARK;
		CHECK;
		return mysql_stmt_prepare(statement, q.c_str(), q.length());
	}
	
	int stmt_execute(MYSQL_STMT* statement){
		BARK;
		int result;
		{
			CHECK;
			result = mysql_stmt_execute(statement);
		}

		if(result != 0){
			throw SQLError(std::string(__FUNCTION__) + " failed, but mysql didn't notice (function returned error, but errno and errmsg unset)");
		}
		return result;
	}

	int stmt_fetch(MYSQL_STMT* statement){
		BARK;
		int result;

		{
			CHECK;
			result = mysql_stmt_fetch(statement);
		}

		if(!(result == 0 || result == MYSQL_NO_DATA || result == MYSQL_DATA_TRUNCATED)){
			throw SQLError(std::string(__FUNCTION__) + "failed, but mysql didn't notice (function returned error, but errno and errmsg unset)");
		}

		return result;
	}
	#undef CHECK
}}
