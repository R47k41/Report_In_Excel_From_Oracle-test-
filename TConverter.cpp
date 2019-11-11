#ifndef TCONVERTER_CPP_
#define TCONVERTER_CPP_
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <stdlib.h>
#include "TConverter.h"

using std::stringstream;
using std::cerr;
using std::endl;
using std::string;

template <class T>
string NS_Converter::toStr(T val, bool no_except) noexcept(false)
{
	stringstream ss;
	try
	{
		ss << val;
		return ss.str();
	}
	catch (...)
	{
		const std::type_info& ti = typeid(val);
		string tmp("Ошибка преобразования значения типа ");
		tmp += ti.name();
		tmp += " в строку!";
		if (no_except)
			return string();
		else
			throw tmp;
	}
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
bool NS_Converter::toType(const string& str, T* val, bool no_except) noexcept(false)
{
	if (str.empty())
	{
		string tmp("Для преобразования указана пустая строка!");
		if (no_except)
			return false;
		else
			throw tmp;
	}
	stringstream ss;
	try
	{
		ss << str;
		ss >> *val;
		return true;
	}
	catch (...)
	{
		const std::type_info& ti = typeid(val);
		string tmp("Ошибка преобразования из строки в указанный тип: ");
		tmp += ti.name();
		if (no_except) return false;
		else throw tmp;
	}
}
/*
//double
template <>
bool NS_Converter::toType<double>(const string& str, double* val, bool no_except) noexcept(false)
{
	if (str.empty())
	{
		string tmp("Указана пустая строка для преобразования!");
		if (no_except) return false;
		else throw tmp;
	}
	*val = std::atof(str.c_str());
	return true;
}
/**/
#endif TCONVERTER_CPP_