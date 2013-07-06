#include "rumysql.hpp"

namespace rusql { namespace mysql {
	ErrorCheckerConnection::ErrorCheckerConnection(rusql::mysql::Connection& database_, const char* f)
	: database(database_)
	, function(f) {
		database.check_and_throw(std::string("Before ") + f);
	}

	ErrorCheckerConnection::~ErrorCheckerConnection() {
		database.check_and_throw(function);
	}
	
	ErrorCheckerStatement::ErrorCheckerStatement(Statement& statement_, char const* f)
	: statement(statement_)
	, function(f)
	{
		statement.check_and_throw(std::string("Before ") + f);
	}
	
	ErrorCheckerStatement::~ErrorCheckerStatement() {
		statement.check_and_throw(function);
	}
}}