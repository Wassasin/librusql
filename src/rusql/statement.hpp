#pragma once

#include <memory>
#include <cppconn/prepared_statement.h>

#include "resultset.hpp"

namespace rusql
{
	class statement
	{
	private:
		std::unique_ptr<sql::PreparedStatement> stmt;
		
	public:
		statement(sql::PreparedStatement* stmt)
		: stmt(stmt)
		{}
		
		void set(const size_t i, const std::string value) const
		{
			stmt->setString(i, value);
		}
		
		bool execute() const
		{
			return stmt->execute();
		}
		
		int update() const
		{
			return stmt->executeUpdate();
		}
		
		resultset query() const
		{
			return resultset(stmt->executeQuery());
		}
	};
}
