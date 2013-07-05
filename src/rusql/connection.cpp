#include "rusql.hpp"

#include <mysql_driver.h>

namespace rusql {
	Connection::Connection(std::shared_ptr< Database > database_)
	: database(database_)
	{
		connect();
	}

	//! Connects with the database, disconnects the previous connection, if there was one.
	void Connection::connect(){
		connection.connect(
			database->info.host,
			database->info.user,
			database->info.password,
			database->info.database,
			0,
			boost::none,
			0
		);
	}
}