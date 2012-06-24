#pragma once

#include <string>
#include <memory>

#include <mysql_connection.h>
#include <mysql_driver.h>

#include "statement.hpp"

namespace rusql
{
	class connection
	{
	public:
		struct connection_info
		{
			const std::string host, user, password, database;
		
			connection_info(const std::string host, const std::string user, const std::string password, const std::string database)
			: host(host)
			, user(user)
			, password(password)
			, database(database)
			{}
		};
	
	private:
		connection_info info;
		std::unique_ptr<sql::Connection> conn;
	
		void connect()
		{
			conn = std::unique_ptr<sql::Connection>(sql::mysql::get_mysql_driver_instance()->connect(
				info.host,
				info.user,
				info.password
			));
	
			conn->setSchema(info.database);
		}

	public:
		connection(const connection_info& info)
		: info(info)
		, conn()
		{
			connect();
		}
	
		bool execute(const std::string q)
		{
			return prepare(q).execute();
		}
		
		statement prepare(const std::string q)
		{
			return statement(conn->prepareStatement(q));
		}
	};
}
