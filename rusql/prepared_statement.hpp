#pragma once

#include <memory>

#include <boost/optional.hpp>

#include "mysql/mysql.hpp"

namespace rusql {
	struct PreparedStatement {
		PreparedStatement (rusql::mysql::Statement&& statement_)
		: statement (std::move(statement_))
		{}

		void execute() {
			statement.execute();
		}

		template <typename ... T>
		PreparedStatement& bind_parameters(T const& ... values) {
			statement.bind(values ... );
			return *this;
		}

	private:
		rusql::mysql::Statement statement;
	};
}
