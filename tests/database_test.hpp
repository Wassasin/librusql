#include <memory>

std::shared_ptr<rusql::Database> get_database(int argc, char *argv[]) {
	if(argc == 1) {
		std::cerr << "1..0 # Skipped: Embedded mode not supported yet" << std::endl;
		exit(0);
		//return new rusql::Database(rusql::Database::Embedded);
	} else if(argc == 5) {
		return std::make_shared<rusql::Database>(
				rusql::Database::ConstructionInfo{argv[1], argv[2], argv[3], argv[4]});
	} else {
		std::cout << "1..0 # Skipped: Invalid parameters" << std::endl;
		exit(0);
	}
}
