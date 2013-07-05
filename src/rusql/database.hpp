#pragma once

#include <memory>
#include <string>

#include "connection.hpp"

namespace rusql {
	struct Database : std::enable_shared_from_this<Database> {
		struct ConstructionInfo {
			std::string host, user, password, database;

			ConstructionInfo (const std::string host_, const std::string user_, const std::string password_, const std::string database_)
				: host (host_)
				, user (user_)
				, password (password_)
				, database (database_)
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

		ResultSet query (std::string const q) {
			return get_connection().query (q);
		}

		template <typename ... T>
		ResultSet query (std::string const q, T const& ... args) {
			return get_connection().prepare (q).bind (args ...).query();
		}

		template <typename ... T>
		void execute (std::string const q, T const& ... args) {
			return get_connection().prepare (q).bind (args ...).execute();
		}
		
		void ping(){
			get_connection().ping();
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