#pragma once

#include "error_checked.hpp"

#include <boost/lexical_cast.hpp>

namespace rusql { namespace mysql {
	struct ColumnNotFound : SQLError { ColumnNotFound(std::string const msg) : SQLError(msg) {} };

	//! A  wrapper around MYSQL_RES, in "mysql_use_result"-mode. For a wrapper around "mysql_store_result", use MySQLStoreResult (doesn't exist at the moment of writing, sorry).
	//! Read the documentation of mysql for pros and cons of use versus store.
	//! Throws on:
	//! - Any function called that returns an error code
	//! - When the resultset is has no fields, but should have fields.
	struct UseResult : boost::noncopyable {
		//! Grabs the current result from that connection
		UseResult(Connection* connection_)
		: connection(connection_)
		, result(connection->use_result())
		, current_row(nullptr)
		{
			if(result == nullptr) {
				throw SQLError(__FUNCTION__, "Asked for UseResult on a Connection which does not have a UseResult ready (query failed, or not a SELECT-type query?)");
			}
		}
		
		UseResult(UseResult&& x)
		: connection(x.connection)
		, result(nullptr)
		, current_row(nullptr)
		{
			std::swap(result, x.result);
			std::swap(current_row, x.current_row);
		}
		
		UseResult& operator=(UseResult&& x){
			connection = x.connection;
			std::swap(result, x.result);
			std::swap(current_row, x.current_row);
			return *this;
		}
		
		~UseResult(){
			if(result){
				close();
			}
		}
		
		//! The connection we were created from (useful for errors)
		Connection* connection;
		
		//! The native-handle for the result
		MYSQL_RES* result;
		
		//! The native-handle for the result-row.
		MYSQL_ROW current_row;
		
		//! Closes and frees the set.
		void close(){
			assert(result != nullptr);
			// MySQL use_result documentation says "you must
			// execute mysql_fetch_row() until a NULL value is
			// returned, otherwise, the unfetched rows are returned
			// as part of the result set for your next query."
			while(fetch_row() != nullptr);
			rusql::mysql::free_result(result);
			result = nullptr;
		}
		
		MYSQL_ROW get_row(){
			assert(result != nullptr);
			return current_row;
		}
		
		size_t get_index(std::string const column_name){
			assert(result != nullptr);

			MYSQL_FIELD* field = nullptr;
			size_t index = 0;
			rusql::mysql::field_seek(result, 0);

			while((field = rusql::mysql::fetch_field(result))){
				if(field->name == column_name){
					return index;
				}
				++index;
			}
			
			throw ColumnNotFound("Column '" + column_name + "' not found");
		}

		template <typename T>
		struct Getter {
			static T get(size_t const index, UseResult& result){
				auto r = result.raw_get(index);

				if(r == nullptr){
					throw std::runtime_error("There's nothing to be found!");
				} else {
					return boost::lexical_cast<T>(r);
				}
			}
		};

		template <typename T>
		struct Getter<boost::optional<T>> {
			static boost::optional<T> get(size_t const index, UseResult& result){
				auto r = result.raw_get(index);

				if(r == nullptr){
					return boost::none;
				} else {
					return boost::lexical_cast<T>(r);
				}
			}
		};
		
		template <typename T>
		T get(size_t const index){
			return Getter<T>::get(index, *this);
		}
		
		template <typename T>
		T get(std::string const column_name){
			assert(result != nullptr);

			return get<T>(get_index(column_name));
		}
		
		char* raw_get(size_t const index){
			assert(result != nullptr);
			assert(current_row != nullptr);
			assert(index < rusql::mysql::num_fields(result));

			return current_row[index];
		}
		
		char* raw_get(std::string const column_name){
			assert(result != nullptr);

			return raw_get(get_index(column_name));
		}
		
		MYSQL_ROW fetch_row() {
			current_row = rusql::mysql::fetch_row(&connection->database, result);
			
			// Output the fetched row.
// 			if(current_row != nullptr){
// 				auto* lengths = fetch_lengths();
// 				auto const n = num_fields();
// 				for(size_t i = 0; i < n; ++i){
// 					COUT << "Column " << i << " is " << lengths[i] << " bytes containing: " << std::string(&current_row[i][0], &current_row[i][lengths[i]]) << std::endl;
// 				}
// 			}
			
			return current_row;
		}

		unsigned long long num_rows() {
			return rusql::mysql::num_rows(&connection->database, result);
		}
	};
}}
