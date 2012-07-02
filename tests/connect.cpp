#include <rusql/connection.hpp>
#include <cassert>

int main(int argc, char *argv[]) {
	assert(argc == 5);
	try {
		rusql::connection(rusql::connection::connection_info(argv[1], argv[2], argv[3], argv[4]));
	} catch(...) {
		return 0;
	}
	return 1;
}
