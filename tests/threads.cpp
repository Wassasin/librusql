#include <rusql/rusql.hpp>
#include <boost/thread.hpp>
#include "test.hpp"
#include "database_test.hpp"

int main(int argc, char *argv[]) {
	auto db = get_database(argc, argv);
	const int NUM_THREADS = 50;

	test_init(7 + NUM_THREADS);
	db->execute("CREATE TABLE rusqltest (`value` INT(2) NOT NULL)");
	db->execute("INSERT INTO rusqltest VALUES (20)");

	{
		auto statement = db->prepare("SELECT value FROM rusqltest");

		statement.execute();

		boost::thread thread([&db]() {
			auto thread_handle = db->get_thread_handle();
			db->execute("UPDATE rusqltest SET value=30");
		});
		thread.join();

		uint64_t value = 10;
		statement.bind_results(value);
		test(statement.fetch(), "one result");
		test(value == 20, "value was correct (" + std::to_string(value) + ")");
		test(!statement.fetch(), "exactly one result");

		statement.execute();
		statement.bind_results(value);
		test(statement.fetch(), "one result");
		test(value == 30, "value was correct (" + std::to_string(value) + ")");
		test(!statement.fetch(), "exactly one result");
	}

	db->execute("DELETE FROM rusqltest");
	db->execute("INSERT INTO rusqltest VALUES (0)");

	// Test simultaneous use of the database
	std::vector<std::shared_ptr<boost::thread>> threads;
	boost::mutex output_mutex;
	for(int i = 0; i < NUM_THREADS; ++i) {
		threads.emplace_back(std::make_shared<boost::thread>([i, &db, &output_mutex]() {
			auto thread_handle = db->get_thread_handle();
			try {
				for(int j = 0; j < 1000; ++j) {
					std::string iteration = std::to_string(i) + "," + std::to_string(j);
					{
						auto statement = db->prepare("SELECT value FROM rusqltest");
						statement.execute();
						uint64_t value;
						statement.bind_results(value);
						if(!statement.fetch()) {
							boost::mutex::scoped_lock lock(output_mutex);
							fail("First statement should succeed (iteration " + iteration + ")");
							return;
						}
						if(statement.fetch()) {
							boost::mutex::scoped_lock lock(output_mutex);
							fail("Second statement should fail (iteration " + iteration + ")");
							return;
						}
					}

					db->execute("UPDATE rusqltest SET value=value+1");
				}
				{
					boost::mutex::scoped_lock lock(output_mutex);
					pass("Thread " + std::to_string(i) + " succeeded");
				}
			} catch(...) {
				boost::mutex::scoped_lock lock(output_mutex);
				fail("Thread " + std::to_string(i) + " threw an exception");
			}
		}));
	}

	for(auto &thread : threads) {
		thread->join();
	}

	{
		auto statement = db->prepare("SELECT value FROM rusqltest");
		statement.execute();
		uint64_t result;
		statement.bind_results(result);
		statement.fetch();
		test(result == NUM_THREADS * 1000, "the right amount of increments was done");
	}

	db->execute("DROP TABLE rusqltest");
}
