#pragma once

#include <memory>

#include <cppconn/prepared_statement.h>

#include <boost/optional.hpp>

namespace rusql {
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
}