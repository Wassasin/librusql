#include <iostream>
#include <string>
#include <sstream>

// next test to run:
static int testnum;
// end of the try{} block if try_block_end == testnum
static int try_block_end;

inline void pass(std::string text) {
	std::cout << "ok " << testnum++ << " " << text << std::endl;
}

inline void fail(std::string text) {
	std::cout << "not ok " << testnum++ << " " << text << std::endl;
}

inline void test(bool c, std::string text) {
	c ? pass(text) : fail(text);
}

inline void test_init(int num_tests) {
	std::cout << "1.." << num_tests << std::endl;
	testnum = 1;
	try_block_end = 0;
}

inline void test_start_try(int num_tests) {
	try_block_end = testnum + num_tests;
}

inline void diag(const std::string &s) {
	std::cout << "# " << s << std::endl;
}

inline void diag(const std::exception &e) {
	diag(e.what());
}

inline void test_finish_try() {
	if(try_block_end == 0) {
		diag("Test is broken: test_finish_try called without test_start_try");
		return;
	}
	if(testnum > try_block_end) {
		diag("Test is broken: More tests already run than promised in test_start_try");
	}
	while(testnum < try_block_end) {
		fail("try {} block failed");
	}
	try_block_end = 0;
}

template <typename T>
inline std::string to_string(const T &v) {
	std::stringstream ss;
	ss << v;
	return ss.str();
}
