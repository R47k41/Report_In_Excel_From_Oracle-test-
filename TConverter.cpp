#pragma once
//#ifndef TCONVERTER_CPP_
//#define TCONVERTER_CPP_
//#include <iostream>
#include "TConverter.h"

using std::stringstream;
using std::cerr;
using std::endl;
using std::string;
using NS_Logger::TLog;

template <class T>
bool NS_Converter::toStr(T val, string& str) noexcept(true)
{
	stringstream ss;
	try
	{
		ss << val;
		str = ss.str();
		return true;
	}
	catch (...)
	{
		const std::type_info& ti = typeid(val);
		TLog("Ошибка преобразования значения типа " + std::string(ti.name()) + " в строку!", "NS_Converter::toStr").toErrBuff();
	}
	return false;
}

template <class T>
std::string NS_Converter::toStr(T val) noexcept(true)
{
	string tmp;
	if (toStr(val, tmp))
		return tmp;
	return string();
}

/*
template <>
string NS_Converter::toStr<const string&>(const string& val, bool no_except) noexcept(false)
{
	return val;
}


/*
//const char*
template <>
string toStr<const char*>(const char* val, bool no_except) noexcept(false)
{
	if (!val) return string();
	return string(val);
}

/*
template <>
string toStr<string>(string val, bool no_except) noexcept(false)
{
	return val;
}

template <>
string toStr<const string&>(const string& val, bool no_except) noexcept(false)
{
	return val;
}
/**/
/*
//unsigned short
template <>
string toStr<unsigned short>(unsigned short val, bool no_except) noexcept(false)
{
	try
	{
		stringstream ss;
		ss << val;
		return ss.str();
	}
	catch (...)
	{
		string tmp("Ошибка преобразования значения типа unsigned short в строку!");
		if (no_except) return string();
		else throw tmp;
	}
}

//char
template <>
string toStr<char>(char val, bool no_except) noexcept(false)
{
	string s;
	if (val) s.push_back(val);
	return s;
}
/**/

template <class T>
bool NS_Converter::toType(const string& str, T* val) noexcept(true)
{
	try
	{
		if (str.empty())
		{
			throw TLog("Для преобразования указана пустая строка!", "NS_Converter::toType");
		}
		stringstream ss;
		ss << str;
		ss >> *val;
		return true;
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
	}
	catch (...)
	{
		const std::type_info& ti = typeid(val);
		TLog("Не обработанная ошибка приведения значения " + str + " к типу: " + ti.name(), "NS_Converter::toType").toErrBuff();
	}
	return false;
}

//double
template <> bool NS_Converter::toType<double>(const std::string& str, double* val) noexcept(true)
{
	using NS_Logger::TLog;
	using std::stringstream;
	try
	{
		if (str.empty())
		{
			throw TLog("Для преобразования указана пустая строка!", "NS_Converter::toType");
		}
		stringstream ss;
		ss << str;
		ss >> *val;
		return true;
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка приведения значения " + str + " к типу double!", "NS_Converter::toType").toErrBuff();
	}
	return false;

}

//bool
template <>
bool NS_Converter::toType<bool>(const std::string& str, bool* val) noexcept(true)
{
	using NS_Logger::TLog;
	using std::stringstream;
	using std::string;
	try
	{
		if (str.empty())
		{
			throw TLog("Для преобразования указана пустая строка!", "NS_Converter::toType");
		}
		stringstream ss;
		ss << str;
		if (ss.str().empty()) return false;
		string tmp = ss.str();//LowerCase(ss.str());
		if (tmp == "true")
		{
			*val = true;
			return true;
		}
		if (tmp == "false")
		{
			*val = false;
			return true;
		}
		int x = 0;
		ss >> x;
		*val = x;
		return true;
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка приведения значения " + str + " к типу double!", "NS_Converter::toType").toErrBuff();
	}
	return false;
}

//#endif TCONVERTER_CPP_