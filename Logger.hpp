//Реализация шаблонных функций модуля Logger.h
#pragma once
#ifndef LOGGER_HPP_
#define LOGGER_HPP_
#include <typeinfo>
#include "TConverter.h"

namespace NS_Logger
{
	//добавление текста к сообщению:
	template <class T>
	TLog& operator+=(TLog& log, T val);
	template <class T>
	TLog& operator<<(TLog& log, T val);
	template <class T>
	TLog& operator+(TLog& log, T val);
}

template <class T>
NS_Logger::TLog& NS_Logger::operator+=(TLog& log, T val)
{
	using std::cerr;
	using std::endl;
	using std::string;
	try
	{
		string tmp = NS_Converter::toStr(val);
		log.AddStr(tmp);
	}
	catch (const string& er)
	{
		cerr << er << endl;
	}
	catch (...)
	{
		const std::type_info& ti = typeid(val);
		cerr << "TLog::operator+=: Ошибка преобразования " << ti.name() << endl;
	}
	return log;
}

template <class T>
NS_Logger::TLog& NS_Logger::operator<<(TLog& log, T val)
{ 
	operator+=(log, val);
	return log; 
}

template <class T>
NS_Logger::TLog& NS_Logger::operator+(TLog& log, T val)
{
	operator+=(log, val);
	return log;
}

#endif LOGGER_HPP_