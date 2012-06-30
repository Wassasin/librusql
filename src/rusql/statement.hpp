#pragma once

#include <memory>
#include <cppconn/prepared_statement.h>

namespace rusql
{
	class statement
	{
	private:
		std::unique_ptr<sql::PreparedStatement> stmt;
		std::unique_ptr<sql::ResultSet> data;
		
		mutable size_t set_i, get_i;
		
	public:
		statement(sql::PreparedStatement* stmt)
		: stmt(stmt)
		, data()
		, set_i(1)
		, get_i(1)
		{}

		void reset() const
		{
			set_i = 1;
			get_i = 1;
		}

		const statement& operator<<(const std::string value) const
		{
			stmt->setString(set_i++, value);
			return *this;
		}
	
		const statement& operator<<(const uint64_t value) const
		{
			stmt->setUInt64(set_i++, value);
			return *this;
		}
		
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
		
		uint64_t get_uint64(const std::string col) const
		{
			return data->getUInt64(col);
		}
		
		bool next() const
		{
			reset();
			return data->next();
		}
		
		bool execute() const
		{
			auto r = stmt->execute();
			reset();
			return r;
		}
		
		int update() const
		{
			auto r = stmt->executeUpdate();
			reset();
			return r;
		}
		
	private:
		class query_result_row {
		public:
			query_result_row(statement& s)
			: stmt(s)
			{}

			template <typename T>
			const statement& operator>>(T& x) const {
				return stmt >> x;
			}

		private:
			statement& stmt;
		};

		class query_iterator {
		public:
			query_iterator(statement& s)
			: stmt(s)
			, end(false)
			{
				//Advance to the first row
				operator++();
			}
			
			bool operator !=(query_iterator const& rh) {
				return end;
			}
			
			void operator++(){
				end = !stmt.next();
			}

			query_result_row operator*() {
				return query_result_row(stmt);
			}
			
		private:
			statement& stmt;
			bool end;
		};
		
		class query_result {
		public:
			query_result(statement& s)
			: stmt(s)
			{}
			
			query_iterator begin() {
				return query_iterator(stmt);
			}
			
			query_iterator end() {
				return query_iterator(stmt);
			}
		private:
			statement& stmt;
		};
		
	public:
		void query()
		{
			data.reset(stmt->executeQuery());
			reset();
		}
	};
}
