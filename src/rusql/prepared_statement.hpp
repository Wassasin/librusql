#pragma once

#include <memory>

#include <cppconn/prepared_statement.h>

#include <boost/optional.hpp>
#include "rumysql.hpp"

namespace rusql {
	struct PreparedStatement {
		PreparedStatement (rusql::mysql::Statement&& statement_)
		: statement (std::move(statement_))
		{}

		ResultSet query() {
			return std::move(statement.bind_execute());
		}

		void execute() {
			statement.execute();
		}

		template <typename ... T>
		PreparedStatement& bind (T const& ... values) {
			statement.bind(values ... );
			return *this;
		}

	private:
		rusql::mysql::Statement statement;
	};
}