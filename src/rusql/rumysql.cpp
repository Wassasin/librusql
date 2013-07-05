#include "rumysql.hpp"

namespace rusql { namespace mysql {
	error_checker::error_checker(rusql::mysql::Connection& database_, const char* f)
	: database(database_)
	, function(f) {
		database.throw_error(std::string("Before ") + f);
	}

	error_checker::~error_checker() {
		database.throw_error(function);
	}
	
	StatementErrorChecker::StatementErrorChecker(Statement& statement_, char const* f)
	: statement(statement_)
	, function(f)
	{
		statement.throw_error(std::string("Before ") + f);
	}
	
	StatementErrorChecker::~StatementErrorChecker() {
		statement.throw_error(function);
	}
}}