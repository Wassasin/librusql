#pragma once 

#include <stdexcept>
#include <string>

#include <cppconn/resultset.h>
#include <cppconn/exception.h>

namespace rusql {
	struct NoResults : std::runtime_error {
		NoResults (std::string msg)
		: runtime_error (msg)
		{}
		
		NoResults()
		: runtime_error ("There were no results in your dataset.")
		{}
	};
	
	struct NoMoreResults : std::runtime_error {
		NoMoreResults (std::string msg)
		: runtime_error (msg)
		{}
		
		NoMoreResults()
		: runtime_error ("There were results, but we've iterated past them (of before them, anyhow, no results here)")
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
}