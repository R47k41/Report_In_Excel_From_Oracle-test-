#pragma once
#include <Windows.h>
#include <errhandlingapi.h>
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

std::string NS_Converter::OEMChar2UChar(const char* pstr, unsigned int size) noexcept(true)
{
	//отбрасываем пустые строки
	if (size == 0 or *pstr == '\0') return string();
	string str;
	for (; size > 0; size--)
		str += (unsigned char)(*pstr++);
	pstr -= size;
	return str;
}

std::string NS_Converter::OEM2Char(const string& src, int OEM_BUF_LEN) noexcept(true)
{
	//инициализация буфера для строки:
	char* buf = new char[src.size()+1];
	string str;
	//конвертация
	if (OEM2Char(src, buf, OEM_BUF_LEN))
		str = OEMChar2UChar(buf, src.size());
	delete[] buf;
	return str;
}

bool NS_Converter::OEM2Char(const string& src, char* buf, int OEM_BUF_LEN) noexcept(true)
{
	if (src.empty()) return false;
	try
	{
		OemToChar(src.c_str(), buf);
		return true;
	}
	catch (const std::exception& err)
	{
		TLog(err.what(), "NS_Converter::OEM2Char").toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка при конвертации строки!", "NS_Converter::OEM2Char").toErrBuff();
	}
	return false;
}

bool NS_Converter::Char2OEM(const string& src, char* buf, int OEM_BUF_LEN) noexcept(true)
{
	if (src.empty()) return false;
	try
	{
		CharToOem(src.c_str(), buf);
		return true;
	}
	catch (const std::exception& err)
	{
		TLog(err.what(), "NS_Converter::Char2OEM").toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка при конвертации строки!", "NS_Converter::Char2OEM").toErrBuff();
	}
	return false;
}

string NS_Converter::ANSI2OEMStr(const string& src, int OEM_BUF_SIZE) noexcept(true)
{
	if (src.empty()) return string();
	//инициализация буфера:
	char* buf = new char[src.size()+ 1];
	string str;
	if (Char2OEM(src, buf, OEM_BUF_SIZE))
		str = string(buf, src.size());
	delete[] buf;
	return str;
}

/**/