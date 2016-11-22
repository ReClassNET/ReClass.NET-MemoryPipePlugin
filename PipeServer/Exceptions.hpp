#pragma once

#include <exception>

class IOException : std::exception
{
public:
	IOException() = default;
	IOException(int)
	{

	}
};

class InvalidOperationException : std::exception
{

};
