#ifndef TCONVERTER_H_
#define TCONVERTER_H_

#include <string>
#include <sstream>
#include <typeinfo>
#include <stdlib.h>
#include "Logger.h"

namespace NS_Converter
{
	//������� �������������� ������:

	template <class T>
	bool toStr(T val, std::string& str) noexcept(true);

	template <class T>
	std::string toStr(T val) noexcept(true);

	template <class T>
	bool toType(const std::string& str, T* val) noexcept(true);

	//�������������� �� unicode-������ � ������������� ������
	std::string UnicodeToMByte(const std::wstring& unicodeStr, size_t toCodePage) noexcept(false);

	//�������������� �� ������������� ������ � unicode ������
	std::wstring MByteToUnicode(const std::string& str, size_t toCodePage) noexcept(false);

	//������������� unicode � ansi
	bool UTF8ToANSI(std::string& inStr) noexcept(false);
}

/**/


#endif
