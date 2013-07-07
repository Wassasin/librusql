#pragma once

#include <string>
#include <memory>

#include "mysql/mysql.hpp"

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
			return connection.ping() == 0;
		}

		//! Returns whether or not the connection is free to do an additional query i.e. there is not a resultset dependent on this connection anymore.
		bool is_free() {
			return result.expired();
		}
		
		ResultSet use_result(){
			return ResultSet(connection);
		}

		ResultSet query (std::string const q) {
			connection.query(q);
			ResultSet set = use_result();
			result = set.get_token();
			return set;
		}

		PreparedStatement prepare (std::string const q) {
			return PreparedStatement(rusql::mysql::Statement(connection, q));
		}
		
		void ping(){
			connection.ping();
		}

	private:
		//! Connects with the database, disconnects the previous connection, if there was one.
		void connect();

		std::shared_ptr<Database> database;
		std::weak_ptr<ResultSet::Token> result;
		
		rusql::mysql::Connection connection;
	};
}
