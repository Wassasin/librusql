#include <memory>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

static bool is_embedded;
static std::string embedded_dir;

std::string get_tempdir() {
	time_t last_time = time(NULL);
	int seq = 1;
	int attempts = 50;
	int i = 0;
	std::string last_error;
	while(i++ < attempts) {
		time_t t = time(NULL);
		if(t != last_time) {
			last_time = t;
			seq = 1;
		}
		std::stringstream ss;
		ss << "/tmp/rusql-test-embedded-" << t << "-" << seq++;
		std::string s = ss.str();
		if(mkdir(s.c_str(), 0700) == 0) {
			return s;
		} else {
			last_error = strerror(errno);
		}
	}
	std::cout << "1..0 # SKIP Failed to create temporary directory after " << attempts << " attempts." << std::endl;
	std::cout << "# Last error: " << last_error << std::endl;
	exit(0);
}

void cleanup_tempdir(void) {
	mysql_library_end();
	// TODO
}

std::shared_ptr<rusql::Database> get_database(int argc, char *argv[]) {
	is_embedded = false;
	if(argc == 1) {
		is_embedded = true;
		embedded_dir = get_tempdir();
		const char *server_options[] = \
			{"test", "--innodb=OFF", "-h", embedded_dir.c_str(), NULL};
		int num_options = (sizeof(server_options)/sizeof(char*)) - 1;
		mysql_library_init(num_options, const_cast<char**>(server_options), NULL);
		auto db = std::make_shared<rusql::Database>(rusql::Database::ConstructionInfo());
		db->execute("CREATE DATABASE rusqltest");
		return std::make_shared<rusql::Database>(rusql::Database::ConstructionInfo("rusqltest"));
	} else if(argc == 5) {
		return std::make_shared<rusql::Database>(
				rusql::Database::ConstructionInfo{argv[1], argv[2], argv[3], argv[4]});
	} else {
		std::cout << "1..0 # SKIP Invalid parameters" << std::endl;
		exit(0);
	}
}
