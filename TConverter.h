#pragma once
#ifndef TCONVERTER_H_
#define TCONVERTER_H_

#include <string>
#include <sstream>
#include <typeinfo>
#include <stdlib.h>
#include "Logger.h"

namespace NS_Converter
{
	using std::stringstream;
	using std::cerr;
	using std::endl;
	using std::string;
	using NS_Logger::TLog;

	//������� �������������� ������:

	template <class T>
	bool toStr(T val, std::string& str) noexcept(true);

	template <class T>
	std::string toStr(T val) noexcept(true);

	template <class T>
	bool toType(const std::string& str, T* val) noexcept(true);

	//template <> bool toType<double>(const std::string& str, double* val) noexcept(true);
	
	//template <> bool toType<bool>(const std::string& str, bool* val) noexcept(true);

	bool toDblType(const std::string& str, double* val) noexcept(true);

	bool toBoolType(const std::string& str, bool* val) noexcept(true);

	//�������������� �� unicode-������ � ������������� ������
	std::string UnicodeToMByte(const std::wstring& unicodeStr, size_t toCodePage) noexcept(false);

	//�������������� �� ������������� ������ � unicode ������
	std::wstring MByteToUnicode(const std::string& str, size_t toCodePage) noexcept(false);

	//������������� unicode � ansi
	bool UTF8ToANSI(std::string& inStr) noexcept(false);

	template <class T>
	bool toStr(T val, string& str) noexcept(true)
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
			TLog("������ �������������� �������� ���� " + std::string(ti.name()) + " � ������!", "NS_Converter::toStr").toErrBuff();
		}
		return false;
	}

	template <class T>
	std::string toStr(T val) noexcept(true)
	{
		string tmp;
		if (toStr(val, tmp))
			return tmp;
		return string();
	}

	template <class T>
	bool toType(const string& str, T* val) noexcept(true)
	{
		try
		{
			if (str.empty())
			{
				throw TLog("��� �������������� ������� ������ ������!", "NS_Converter::toType");
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
			string msg = "�� ������������ ������ ���������� �������� " + str + " � ����: " + ti.name();
			TLog(msg, "NS_Converter::toType").toErrBuff();
		}
		return false;
	}
/**/
}

#endif
