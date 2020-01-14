#ifndef TCONVERTER_H_
#define TCONVERTER_H_

#include <string>


namespace NS_Converter
{
	//функции преобразования данных:

	template <class T>
	bool toStr(T val, std::string& str) noexcept(true);

	template <class T>
	std::string toStr(T val) noexcept(true);

	template <class T>
	bool toType(const std::string& str, T* val) noexcept(true);

/*
	template <>
	string toStr<const char*>(const char* val, bool no_except) noexcept(false);

	template <>
	string toStr<const std::string&>(const std::string& val, bool no_except) noexcept(false);

	template <>
	string toStr<unsigned short>(unsigned short val, bool no_except) noexcept(false);

	template <>
	string toStr<char>(char val, bool no_except) noexcept(false);

	template <>
	bool toType<double>(const string& str, double* val, bool no_except) noexcept(false);
/**/
	//преобразование из unicode-строки в многобайтовую строку
	std::string UnicodeToMByte(const std::wstring& unicodeStr, size_t toCodePage) noexcept(false);
	//преобразование из многобайтовой строки в unicode строку
	std::wstring MByteToUnicode(const std::string& str, size_t toCodePage) noexcept(false);
	//перекодировка unicode в ansi
	//bool UTF8ToANSI(std::string& inStr) noexcept(false);
}

#endif
