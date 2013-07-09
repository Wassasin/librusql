#pragma once

#include <memory>
#include <string>

#include "connection.hpp"

namespace rusql {
	struct Database : std::enable_shared_from_this<Database> {
		struct ConstructionInfo {
			enum class ConstructionInfoType {
				TCP,
				UNIX,
				Embedded,
			};
			ConstructionInfoType type;
			// if type == TCP:
			std::string host;
			uint16_t port;
			// if type == UNIX:
			std::string unix_path;
			// if type IS NOT Embedded:
			std::string user, password;
			// always optional:
			std::string database;

			ConstructionInfo (const std::string &host_, uint16_t port_, const std::string &user_, const std::string &password_, const std::string &database_ = std::string())
				: type (ConstructionInfoType::TCP)
				, host (host_)
				, port (port_)
				, user (user_)
				, password (password_)
				, database (database_)
			{}

			ConstructionInfo (const std::string &unix_path_, const std::string &user_, const std::string &password_, const std::string &database_ = std::string())
				: type (ConstructionInfoType::UNIX)
				, unix_path (unix_path_)
				, user (user_)
				, password (password_)
				, database (database_)
			{}

			ConstructionInfo (const std::string &database_ = std::string())
				: type (ConstructionInfoType::Embedded)
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

		int number_of_active_connections() const {
			int num = 0;
			for(auto const &c : connections) {
				if(!c->is_free()) num++;
			}
			return num;
		}

		ResultSet query(std::string const q) {
			return get_connection().query(q);
		}

		PreparedStatement prepare(std::string const q){
			return get_connection().prepare(q);
		}

		template <typename ... T>
		PreparedStatement execute(std::string const q, T const& ... args) {
			return get_connection().execute(q, args ...);
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
