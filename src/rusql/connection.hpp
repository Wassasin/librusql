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
		struct info
		{
			std::string host, user, password, database;
		
			info(const std::string host, const std::string user, const std::string password, const std::string database)
			: host(host)
			, user(user)
			, password(password)
			, database(database)
			{}
		};
	
	private:
		const info credentials;
		std::unique_ptr<sql::Connection> conn;
	
		void connect()
		{
			conn = std::unique_ptr<sql::Connection>(sql::mysql::get_mysql_driver_instance()->connect(
				credentials.host,
				credentials.user,
				credentials.password
			));
	
			conn->setSchema(credentials.database);
		}

	public:
		connection(const info& credentials)
		: credentials(credentials)
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
