#include <cstring>
#include "Logger.h"

using std::string;
using std::stringstream;
using std::endl;
using std::cerr;
using NS_Logger::TLog;

TLog::TLog(const string& str, const string& fnc_name): name(fnc_name), buf()
{
	msg << str;
}

TLog::TLog(const TLog& log): name(log.name), buf()
{
	if (log.isEmpty() or this == &log) return;
	msg.str(log.msg.str());
}

void TLog::clear(bool clear_state)
{
	if (!msg.good() and clear_state)
		msg.clear();
	if (msg.good())
		msg.str(string());
	buf.clear();
}

string TLog::getStr(bool full) const noexcept(true)
{
	const char d[3] = ": ";
	if (full)
		return name + d + msg.str();
	return msg.str();
}

const char* TLog::what() const noexcept(true)
{
	if (buf.size() > 0) buf.clear();
	if (msg.str().size() > 0)
	{
		buf = getStr(true);
	}
	return buf.c_str();
}

TLog& TLog::operator=(const char* pstr)
{
	if (pstr)
	{
		msg.clear();
		msg << pstr;
	}
	return *this;
}

void TLog::toErrBuff(std::ostream& stream) const
{
	if (isEmpty()) return;
	string tmp;
	if (stream) stream << getStr() << endl;
}