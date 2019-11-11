#include <iostream>
#include <cstring>
#include "Logger.h"

using std::string;
using std::stringstream;
using std::endl;
using std::cerr;
using NS_Logger::TLog;

TLog::TLog(const string& str): buf()
{
	msg << str;
}

TLog::TLog(const TLog& log):buf()
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

const char* TLog::what() const noexcept(true)
{
	if (buf.size() > 0) buf.clear();
	if (msg.str().size() > 0)
	{
		buf = msg.str();
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

void TLog::raise(bool shw_raise, const char* place_from) const
{
	if (isEmpty()) return;
	string tmp;
	if (place_from) tmp.append(place_from).append(": ");
	tmp += getStr();
	if (shw_raise)
		throw tmp;
	else
		cerr << tmp << endl;
}