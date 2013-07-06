#include "rumysql.hpp"

namespace rusql { namespace mysql {
	ErrorCheckerConnection::ErrorCheckerConnection(rusql::mysql::Connection& database_, const char* f)
	: database(database_)
	, function(f) {
		database.throw_error(std::string("Before ") + f);
	}

	ErrorCheckerConnection::~ErrorCheckerConnection() {
		database.throw_error(function);
	}
	
	ErrorCheckerStatement::ErrorCheckerStatement(Statement& statement_, char const* f)
	: statement(statement_)
	, function(f)
	{
		statement.throw_error(std::string("Before ") + f);
	}
	
	ErrorCheckerStatement::~ErrorCheckerStatement() {
		statement.throw_error(function);
	}
}}