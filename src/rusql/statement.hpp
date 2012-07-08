#pragma once

#include <memory>
#include <iterator>
#include <cppconn/prepared_statement.h>

namespace rusql
{
	/*! A prepared statement, which encapsulates the result as well.
	While this class aims to be stateless, it does have some state, so... stay a while, and listen!
	You can create a statement with placeholders, and fill them in with the stream-insertion-operator.
	After you filled all the placeholders (the statement will 'friendly' remind you if you forgot one), you can execute/update/query the query.
	An execute returns success/failure, you need to check this value yourself
	An update returns the number of updated records
	A query puts the statement in a different state. After the query, you can iterate over the statement (it has begin() and end() members) and then you get the rows!
	The rows can be read with >>, if you just want the first N columns of the row. The other way is to use operator[string] on the row, which gets you that column.
	*/
	class statement
	{
	private:
		std::unique_ptr<sql::PreparedStatement> stmt;
		std::unique_ptr<sql::ResultSet> data;
		
		mutable size_t set_i, get_i;
		
		/*! Resets the (internal) state of the statement, which is used for keeping track of which placeholders to replace when users to >> and << */
		void reset() const
		{
			set_i = 1;
			get_i = 1;
		}

	private:
		/*! An interface to a row, you can extract values from columns with >> as well as operator[string] as well as get_<type>(string column)
		Currently has a reference to the statment (with the ResultSet), so it makes now copy. Changing the statement (eg calling .next()), changes this query_result_row. That's why this class is private (but we have auto now...).
		TODO: consider copying the data?
		*/
		class query_result_row {
		public:
			query_result_row(statement& s)
			: stmt(s)
			{}

			template <typename T>
			const statement& operator>>(T& x) const {
				return stmt >> x;
			}
			
			std::string get_string(const std::string col) const {
				return stmt.get_string(col);
			}
			
			uint64_t get_uint64(const std::string col) const {
				return stmt.get_uint64(col);
			}

			bool is_null(const std::string col) const {
				return stmt.is_null(col);
			}

		private:
			statement& stmt;
		};

		/*! An iterator that iterates over rows of a result. Knows when he has hit the last row. */
		class query_iterator : public std::iterator<std::input_iterator_tag, query_result_row, std::ptrdiff_t, query_result_row, query_result_row> {
		public:
			query_iterator()
			: stmt(nullptr)
			{}

			query_iterator(statement& s)
			: stmt(&s)
			{
				//Advance to the first row
				operator++();
			}
			
			bool operator ==(query_iterator const & rh) {
				return stmt == rh.stmt;
			}

			bool operator !=(query_iterator const& rh) {
				return !(stmt == rh.stmt);
			}
			
			query_iterator& operator++(){
				if(!stmt->next())
					stmt = nullptr;
				return *this;
			}

			query_result_row operator*() {
				return query_result_row(*stmt);
			}
			
		private:
			statement* stmt;
		};

	public:
		/* Initializes a statement, with a statement. The object takes ownership of the statement.
		\todo Change the ctor to accept a std::string, which we give to sql and intialize ourselves with. This should be a private-ctor or something because we take ownership of the pointer.
		*/
		statement(sql::PreparedStatement* stmt)
		: stmt(stmt)
		, data()
		, set_i(1)
		, get_i(1)
		{}

		/*! Inserts the given argument in the current placeholder */
		const statement& operator<<(const std::string value) const
		{
			stmt->setString(set_i++, value);
			return *this;
		}
	
		/*! Inserts the given argument in the current placeholder */
		const statement& operator<<(const uint64_t value) const
		{
			stmt->setUInt64(set_i++, value);
			return *this;
		}
		
		/*! Extracts the value from the current placeholder */
		const statement& operator>>(std::string& value) const
		{
			value = data->getString(get_i++);
			return *this;
		}
		
		const statement& operator>>(uint64_t& value) const
		{
			value = data->getUInt64(get_i++);
			return *this;
		}
		
		std::string get_string(const std::string col) const
		{
			return data->getString(col);
		}
		
		/*! Fetch the int at the given column
		\param col Tells which column to use
		\return The value in that row in the given column
		*/
		uint64_t get_uint64(const std::string col) const
		{
			return data->getUInt64(col);
		}
		
		bool is_null(const std::string col) const
		{
			return data->isNull(col);
		}
		
		/*! Advances the statement-result to the next row
		\return Returns whether the statement-result is now on a row.
		*/
		bool next() const
		{
			reset();
			return data->next();
		}

		query_iterator begin() {
			return query_iterator(*this);
		}

		query_iterator end() {
			return query_iterator();
		}
		
		void execute() const
		{
			auto r = stmt->execute();
			// TODO: figure out when execute doesn't return 0.
			// TODO: make better error handling.
			if(r){
				auto ptr = stmt->getWarnings();
				if(ptr){
					std::cout << "Fail: " << ptr->getMessage() << std::endl;
				} else {
					std::cout << "Fail without a message..." << std::endl;
				}
			}

			reset();
		}
		
		int update() const
		{
			auto r = stmt->executeUpdate();
			reset();
			return r;
		}

	public:
		statement& query()
		{
			data.reset(stmt->executeQuery());
			reset();
			return *this;
		}
	};
}
