#pragma once

#include <exception>
#include <string>

namespace FileStore
{
	class Exception : public std::exception
	{
	public:
		explicit Exception( const char * msg )
			: _msg( msg )
		{
		}

	public:
		const char * what() const override
		{
			return _msg.c_str();
		}

	private:
		std::string _msg;
	};

}
