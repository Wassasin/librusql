#include <rusql/connection.hpp>
#include <rusql/statement.hpp>

int main() {
	rusql::Database(rusql::Database::ConstructionInfo{"", "", "", ""});
	return 0;
}
