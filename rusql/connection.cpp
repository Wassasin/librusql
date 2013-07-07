#include "rusql.hpp"

namespace rusql {
	Connection::Connection(std::shared_ptr< Database > database_)
	: database(database_)
	{
		connect();
	}

	//! Connects with the database, disconnects the previous connection, if there was one.
	void Connection::connect(){
		typedef Database::ConstructionInfo::ConstructionInfoType CIType;

		switch(database->info.type) {
		case CIType::TCP:
			connection.connect(
				database->info.host,
				database->info.port,
				database->info.user,
				database->info.password,
				database->info.database,
				0);
			break;

		case CIType::UNIX:
			connection.connect(
				database->info.unix_path,
				database->info.user,
				database->info.password,
				database->info.database,
				0);
			break;

		case CIType::Embedded:
			connection.connect(
				database->info.database,
				0);
			break;

		default:
			assert(!"Unreachable code");
		}
	}
}
