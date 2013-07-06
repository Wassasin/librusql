#pragma once 

#include <stdexcept>
#include <string>
#include <memory>

#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include "rumysql.hpp"

namespace rusql {

struct Connection;
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
		ResultSet (rusql::mysql::Connection& connection)
		: data (rusql::mysql::UseResult(&connection))
		, token (new Token)
		{
			next();
		}
		
		ResultSet (rusql::mysql::UseResult&& use_result)
		: data (std::move(use_result))
		, token (new Token)
		{
			next();
		}

		//! Invalidates the resultset, so that you can reuse the connection that was used to create this resultset. Use with caution.
		void release() {
			token.reset();
			data.close();
		}
		
		template <typename T>
		T get(size_t const index){
			return data.get<T>(index);
		}
		
		template <typename T>
		T get(std::string const column_name){
			return data.get<T>(column_name);
		}
		
		uint64_t get_uint64 (size_t const index) {
			return get<uint64_t>(index);
		}
		
		uint64_t get_uint64 (std::string const column_name) {
			return get<uint64_t>(column_name);
		}
		
		std::string get_string (size_t const index) {
			return get<std::string>(index);
		}
		
		std::string get_string (std::string const column_name) {
			return get<std::string>(column_name);
		}
		
		bool is_null (size_t const index) {
			return data.raw_get(index) == nullptr;
		}
		
		bool is_null (std::string const column_name) {
			return data.raw_get(column_name) == nullptr;
		}
		
		bool is_closed() const {
			return data.current_row == nullptr;
		}
		
		operator bool() const {
			return !is_closed();
		}
		
		void next() {
			data.fetch_row();
		}
		
		struct Token {}; // What is shared between the connection and the resultset, so the connection knows when the resultset went out of scope.
		
		//! Returns a weak pointer that will expire if the ResultSet is released (goes out of scope, or release() is called.
		//! Note that this doesn't reflect if the resultset is still valid (due to underlying software). Use is_valid() for check that.
		std::weak_ptr<Token> get_token() {
			return token;
		}

	private:
		rusql::mysql::UseResult data;
		std::shared_ptr<Token> token;
	};
}