#pragma once

#include <memory>
#include <string>

#include "connection.hpp"

namespace rusql {
	struct Database : std::enable_shared_from_this<Database> {
		struct ConstructionInfo {
			std::string host, user, password, database;

			ConstructionInfo (const std::string host, const std::string user, const std::string password, const std::string database)
				: host (host)
				, user (user)
				, password (password)
				, database (database)
			{}
		};


		Database (ConstructionInfo const& rh)
			: info (rh) {
		}

		Connection& get_connection() {
			for (auto & c : connections) {
				if (c->is_free()) return *c;
			}

			return create_connection();
		}

		ResultSet query (std::string const query) {
			return get_connection().query (query);
		}

		template <typename ... T>
		ResultSet query (std::string const query, T const& ... args) {
			return get_connection().prepare (query).bind (args ...).query();
		}

		template <typename ... T>
		void execute (std::string const query, T const& ... args) {
			return get_connection().prepare (query).bind (args ...).execute();
		}

	private:
		friend struct Connection;
		ConstructionInfo const info;

		std::vector<std::shared_ptr<Connection>> connections;

		Connection& create_connection() {
			connections.emplace_back (std::make_shared<Connection> (shared_from_this()));
			return *connections.back();
		}
	};
}