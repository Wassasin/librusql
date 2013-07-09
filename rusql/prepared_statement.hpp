#pragma once

#include <memory>

#include <boost/optional.hpp>

#include "mysql/mysql.hpp"
#include "token.hpp"

namespace rusql {
	struct PreparedStatement {
		PreparedStatement (rusql::mysql::Statement&& statement_)
		: statement (std::move(statement_))
		{}

		void execute() {
			statement.execute();
		}

		//! Fetches a new row of data from the db, and puts the data into the variables you bound in bind_results which you need to call first, but only once, unless you want to store each row in different variables or something.
		//! @return True if there's more to fetch/everthing went alright. False when not.
		bool fetch() {
			return statement.fetch() != MYSQL_NO_DATA;
		}

		bool is_closed() const {
			return statement.statement == nullptr;
		}

		unsigned long long insert_id() {
			return statement.insert_id();
		}

		operator bool() const {
			return is_closed();
		}

		template <typename ... T>
		PreparedStatement& bind_parameters(T const& ... values) {
			statement.bind(values ... );
			return *this;
		}

		template <typename ... T>
		PreparedStatement& bind_results(T& ... results) {
			statement.bind_results(results ...);
			return *this;
		}

		std::weak_ptr<Token> get_token() const {
			return token;
		}

	private:
		std::shared_ptr<Token> token;
		rusql::mysql::Statement statement;
	};
}
