#pragma once

#include <string>
#include <memory>

#include <cppconn/connection.h>

#include "resultset.hpp"
#include "prepared_statement.hpp"

namespace rusql {
	struct Database;

	struct Connection {
		Connection (std::shared_ptr<Database> database);

		//! Reconnects if lost connection
		void make_valid() {
			if (!is_valid()) {
				connect();
			}
		}

		bool is_valid() {
			return !connection->isClosed();
		}

		//! Returns whether or not the connection is free to do an additional query i.e. there is not a resultset dependent on this connection anymore.
		bool is_free() {
			return result.expired();
		}

		ResultSet query (std::string const query) {
			make_valid();
			ResultSet set = prepare (query).query();
			result = set.get_token();
			return set;
		}

		PreparedStatement prepare (std::string const query) {
			make_valid();
			return PreparedStatement (connection->prepareStatement (query));
		}

	private:
		//! Connects with the database, disconnects the previous connection, if there was one.
		void connect();

		std::shared_ptr<Database> database;
		std::weak_ptr<ResultSet::Token> result;
		std::unique_ptr<sql::Connection> connection;
	};
}
