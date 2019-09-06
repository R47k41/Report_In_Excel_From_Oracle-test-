#include <sstream>
#include <cstdarg>
#include <cstring>
#include "Logger.h"

//Добавление данных:
template <class T>
void Logger::TLog::add(const T& val)
{
	std::stringstream ss;
	ss << val;
	msg += ss.str();
	//new_line();
	//return *this;
}

std::ostream& Logger::operator<<(std::ostream& stream, const Logger::TLog& log)
{
	if (stream)
		stream << log.what();
	return stream;
}

Logger::TLog::TLog(const char* str, ...) : msg(str)
{
	va_list arr;
	va_start(arr, str);
	while (true)
	{
		const char* tmp = va_arg(arr, const char*);
		if (tmp == nullptr) break;
		else
		{
			msg += string(tmp);
		}
	}
	va_end(arr);
/**/
}
/**/