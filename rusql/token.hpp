#pragma once

namespace rusql {
	//! Is shared between a Connection and a (Statement or ResultSet), so a connection knows when it's still in use
	struct Token{};
}
