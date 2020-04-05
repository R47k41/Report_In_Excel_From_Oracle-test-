#pragma once
#include <Windows.h>
#include "Logger.hpp"


using std::string;

//double
bool NS_Converter::toDblType(const std::string& str, double* val) noexcept(true)
{
	using NS_Logger::TLog;
	using std::stringstream;
	try
	{
		if (str.empty())
		{
			throw TLog("Для преобразования указана пустая строка!", "NS_Converter::toDblType");
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
		string msg = "Не обработанная ошибка приведения значения " + str + " к типу double!";
		TLog(msg, "NS_Converter::toDblType").toErrBuff();
	}
	return false;

}

//bool
bool NS_Converter::toBoolType(const std::string& str, bool* val) noexcept(true)
{
	using NS_Logger::TLog;
	using std::stringstream;
	using std::string;
	try
	{
		if (str.empty())
		{
			throw TLog("Для преобразования указана пустая строка!", "NS_Converter::toBoolType");
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
		string msg = "Не обработанная ошибка приведения значения " + str + " к типу bool!";
		TLog(msg, "NS_Converter::toBoolType").toErrBuff();
	}
	return false;
}


//конвертация ansi-строки в unicode
std::wstring NS_Converter::MByteToUnicode(const std::string& str, size_t toCodePage) noexcept(false)
{
	using NS_Logger::TLog;
	std::wstring result;
	wchar_t* unicodeStr = nullptr;
	try
	{
		//получение размера
		size_t size;
		size = MultiByteToWideChar(toCodePage, 0, str.c_str(), -1, NULL, 0);
		if (size < 0)
			throw TLog("Ошибка при определении размера результирующей строки!", "MByteToUnicode");
		//создаем unicode строку:
		unicodeStr = new wchar_t[size];
		//конвертируем в unicode:
		if (MultiByteToWideChar(toCodePage, 0, str.c_str(), -1, unicodeStr, size) < 0)
			throw TLog("Ошибка при конвертации!", "MByteToUnicode");
		result = std::wstring(unicodeStr);
	}
	catch (const std::string& er)
	{
		if (unicodeStr) delete[] unicodeStr;
		TLog log(er, "MByteToUnicode");
		log << "(кодовая страница: " << toCodePage << "; текст: " << str << " )";
		throw log;
	}
	catch (...)
	{
		if (unicodeStr) delete[] unicodeStr;
		TLog log("Не обработанная ошибка при конвертации текста: ", "MByteToUnicode");
		log << str << " для кодовой страницы: " << toCodePage << "!";
		throw log;
	}
	if (unicodeStr) delete[] unicodeStr;
	return result;
}

//конвертация unicode-строки в ansi
std::string NS_Converter::UnicodeToMByte(const std::wstring& unicodeStr, size_t toCodePage) noexcept(false)
{
	using NS_Logger::TLog;
	std::string result;
	char* multiByteStr = nullptr;
	try
	{
		//получение размера
		size_t size;
		size = WideCharToMultiByte(toCodePage, NULL, unicodeStr.c_str(), -1, NULL, 0, NULL, FALSE);
		if (size < 0)
			throw TLog("Ошибка при определении размера результирующей строки!", "MByteToUnicode");
		//создаем unicode строку:
		multiByteStr = new char[size];
		//конвертируем в unicode:
		if (WideCharToMultiByte(toCodePage, NULL, unicodeStr.c_str(), -1, multiByteStr, size, NULL, FALSE) < 0)
			throw TLog("Ошибка при конвертации!", "MByteToUnicode");
		result = std::string(multiByteStr);
	}
	catch (const string& er)
	{
		if (multiByteStr) delete[] multiByteStr;
		TLog log(er, "MByteToUnicode");
		log << "(кодовая страница: " << toCodePage << ")";
		throw log;
	}
	catch (...)
	{
		if (multiByteStr) delete[] multiByteStr;
		TLog log("Не обработанная ошибка при конвертации текста для кодовой страницы: ", "MByteToUnicode");
		log << toCodePage << "!";
		throw log;
	}
	if (multiByteStr) delete[] multiByteStr;
	return result;
}

bool NS_Converter::UTF8ToANSI(std::string& inStr) noexcept(false)
{
	using std::wstring;
	if (inStr.empty()) return false;
	//перекодировака из стандартного потока ввода в utf8:
	wstring wStr = MByteToUnicode(inStr.c_str(), CP_UTF8);
	//преобразование из unicode в ansi:
	inStr = UnicodeToMByte(wStr.c_str(), CP_ACP);
	return true;
}
/**/