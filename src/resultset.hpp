#pragma once

#include <string>
#include <memory>

#include <cppconn/resultset.h>

namespace rusql
{
	class resultset
	{
	private:
		std::unique_ptr<sql::ResultSet> data;
	
	public:
		resultset(sql::ResultSet* data)
		: data(data)
		{}
		
		bool next() const
		{
			return data->next();
		}
		
		std::string get_string(const std::string col) const
		{
			return data->getString(col);
		}
	};
}
