#include <rusql/rusql.hpp>
#include <cassert>

int main(int argc, char *argv[]) {
	assert(argc == 5);
	try {
		rusql::Database(rusql::Database::ConstructionInfo{argv[1], argv[2], argv[3], argv[4]});
	} catch(...) {
		return 0;
	}
	return 1;
}
