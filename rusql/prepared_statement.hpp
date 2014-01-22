#pragma once

#include <memory>

#include <boost/optional.hpp>

#include "mysql/mysql.hpp"
#include "token.hpp"

namespace rusql {
	struct PreparedStatement {
		PreparedStatement (rusql::mysql::Statement&& statement_)
		: token(std::make_shared<Token>())
		, statement (std::move(statement_))
		{}

		template <typename T>
		PreparedStatement execute(std::vector<T> const &args) {
			bind_parameters(args);
			return execute();
		}

		template <typename Head, typename ... Tail>
		PreparedStatement&& execute(Head const &head, Tail const& ... tail) {
			bind_parameters(head, tail ...);
			return execute();
		}

		PreparedStatement&& execute() {
			statement.execute();
			return std::move(*this);
		}

		//! Get the column number of a column by name. Can only be called
		//! after execute().
		int column_number(std::string name) {
			auto mysql_res = statement.result_metadata();
			int column = 0;
			while(MYSQL_FIELD *field = rusql::mysql::fetch_field(mysql_res.get())) {
				if(name == field->name) {
					return column;
				}
				column++;
			}
			throw std::runtime_error("No column with that name");
		}

		//! Get a column by name, but only if it was bound using
		//! bind_results() before. If it wasn't bound before, this
		//! method behaves as if the cell was NULL.
		template <typename T>
		T get(std::string name) {
			int column = column_number(name);

			T result;
			rusql::mysql::OutputHelper helper;
			auto bind_info = rusql::mysql::get_mysql_output_bind(result, helper);

			MYSQL_BIND &bound = bind_info.first;
			rusql::mysql::OutputProcessor &processor = bind_info.second;

			// if this column is unbound -> fetch_column acts as if it was NULL, but should throw (TODO)
			// if T is optional -> correctly sets optional to uninitialized or initialized value
			// if this cell is NULL -> initializes T to default value, but should throw (TODO)
			statement.fetch_column(&bound, column, 0);

			processor(bound, statement, column);
			return result;
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

		void store_result() {
			statement.store_result();
		}

		/*! You need to call store_result() before this function returns anything other than 0. This
		 * is a MySQL limitation. */
		unsigned long long num_rows() {
			return statement.num_rows();
		}

		template <typename ... T>
		PreparedStatement& bind_parameters(T const& ... values) {
			statement.bind(values ... );
			return *this;
		}

		template <typename T>
		PreparedStatement& bind_parameters(std::vector<T> const &values) {
			statement.bind(values);
			return *this;
		}

		template <typename ... T>
		PreparedStatement& bind_parameters_append(T const& ... values) {
			statement.bind_append(values ...);
			return *this;
		}

		template <typename ... T>
		PreparedStatement& bind_results(T& ... results) {
			statement.bind_results(results ...);
			return *this;
		}

		template <typename ... T>
		PreparedStatement & bind_results_append(T& ... results) {
			statement.bind_results_append(results ...);
			return *this;
		}

		std::weak_ptr<Token> get_token() const {
			assert(token);
			return token;
		}

	private:
		std::shared_ptr<Token> token;
		rusql::mysql::Statement statement;
	};
}
