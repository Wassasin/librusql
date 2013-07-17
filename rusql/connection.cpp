#include "rusql.hpp"

namespace rusql {
	Connection::Connection(std::weak_ptr< Database > database_)
	: database(database_)
	{
		connect();
	}

	//! Connects with the database, disconnects the previous connection, if there was one.
	void Connection::connect(){
		typedef Database::ConstructionInfo::ConstructionInfoType CIType;
		std::shared_ptr<Database> db = database.lock();

		switch(db->info.type) {
		case CIType::TCP:
			connection.connect(
				db->info.host,
				db->info.port,
				db->info.user,
				db->info.password,
				db->info.database,
				0);
			break;

		case CIType::UNIX:
			connection.connect(
				db->info.unix_path,
				db->info.user,
				db->info.password,
				db->info.database,
				0);
			break;

		case CIType::Embedded:
			connection.connect(
				db->info.database,
				0);
			break;

		default:
			assert(!"Unreachable code");
		}
	}
}
