#pragma once

#include <string>
#include <memory>

#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/connection.h>
#include <cppconn/exception.h>

#include <boost/optional.hpp>

#include "statement.hpp"

namespace rusql {
	struct NoResults : std::runtime_error {
		NoResults (std::string msg)
			: runtime_error (msg)
		{}

		NoResults()
			: runtime_error ("There were not results in your dataset.")
		{}
	};

	struct NoMoreResults : std::runtime_error {
		NoMoreResults (std::string msg)
			: runtime_error (msg)
		{}

		NoMoreResults()
			: runtime_error ("There were results, but we're out of those")
		{}
	};

	//! An interface for a result from a query. Incopyable.
	//! You can iterate over the rows from a query, or get them all at once.
	//! You can automatically convert a row to a boost::fusion'd struct, given that the column names are the same as the members, and the types are constructible from the values.
	struct ResultSet {
		template <typename T>
		ResultSet (T convertable_to_shared_ptr)
			: data (convertable_to_shared_ptr)
			, token (new Token) {
			next();
		}

		//! Resultsets go invalid when their connection is closed, or performed another query.
		bool is_valid() {
			return data->isClosed();
		}

		//! Invalidates the resultset, so that you can reuse the connection that was used to create this resultset. Use with caution.
		void release() {
			token.reset();
			data.reset();
		}

		uint64_t get_uint64 (size_t const index) {
			try {
				return data->getUInt64 (index);
			} catch (sql::InvalidArgumentException& e) {
				if (data->isBeforeFirst() || data->isAfterLast()) {
					if (data->rowsCount() == 0) {
						throw rusql::NoResults();
					} else {
						throw rusql::NoMoreResults();
					}
				} else {
					throw e;
				}
			}
		}

		uint64_t get_uint64 (std::string const column_name) {
			try {
				return data->getUInt64 (column_name);
			} catch (sql::InvalidArgumentException& e) {
				if (data->isBeforeFirst() || data->isAfterLast()) {
					if (data->rowsCount() == 0) {
						throw rusql::NoResults();
					} else {
						throw rusql::NoMoreResults();
					}
				} else {
					throw e;
				}
			}
		}

		std::string get_string (size_t const index) {
			try {
				return data->getString (index);
			} catch (sql::InvalidArgumentException& e) {
				if (data->isBeforeFirst() || data->isAfterLast()) {
					if (data->rowsCount() == 0) {
						throw rusql::NoResults();
					} else {
						throw rusql::NoMoreResults();
					}
				} else {
					throw e;
				}
			}
		}

		std::string get_string (std::string const column_name) {
			try {
				return data->getString (column_name);
			} catch (sql::InvalidArgumentException& e) {
				if (data->isBeforeFirst() || data->isAfterLast()) {
					if (data->rowsCount() == 0) {
						throw rusql::NoResults();
					} else {
						throw rusql::NoMoreResults();
					}
				} else {
					throw e;
				}
			}
		}

		bool is_null (size_t const index) {
			return data->isNull (index);
		}

		bool is_null (std::string const column_name) {
			return data->isNull (column_name);
		}

		bool is_valid() const {
			return ! (data->isBeforeFirst() || data->isAfterLast() || data->isClosed());
		}

		operator bool() const {
			return is_valid();
		}

		void next() {
			data->next();
		}

		struct Token {}; // What is shared between the connection and the resultset, so the connection knows when the resultset went out of scope.

		//! Returns a weak pointer that will expire if the ResultSet is released (goes out of scope, or release() is called.
		//! Note that this doesn't reflect if the resultset is still valid (due to underlying software). Use is_valid() for check that.
		std::weak_ptr<Token> get_token() {
			return token;
		}

	private:
		std::unique_ptr<sql::ResultSet> data;
		std::shared_ptr<Token> token;
	};

	struct PreparedStatement {
		template <typename ConvertableToShared>
		PreparedStatement (ConvertableToShared && statement)
			: statement (statement)
		{}

		ResultSet query() {
			return ResultSet (statement->executeQuery());
		}

		void execute() {
			statement->execute();
		}

		template <typename Head, typename ... T>
		PreparedStatement& bind (Head const& head, T const& ... values) {
			size_t index = 1;
			return bind_ (index, head, values ...);
		}

		PreparedStatement& bind() {
			return *this;
		}

		template <typename Head, typename ... T>
		PreparedStatement& bind_ (size_t& index, Head const& head, T const& ... tail) {
			bind_element (index, head);
			return bind_ (index, tail ...);
		}

		PreparedStatement& bind_ (size_t const) {
			return *this;
		}

		PreparedStatement& bind_element (size_t& position, std::string const& x) {
			statement->setString (position++, x);
			return *this;
		}

		PreparedStatement& bind_element (size_t& position, uint64_t const& x) {
			statement->setUInt64 (position++, x);
			return *this;
		}

		template <typename T>
		PreparedStatement& bind_element (size_t& position, boost::optional<T> const& x) {
			if (x) {
				bind_element (position, *x);
			}
			return *this;
		}
	private:
		std::unique_ptr<sql::PreparedStatement> statement;
	};

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
