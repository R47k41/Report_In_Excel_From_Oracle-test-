#ifndef TCONVERTER_H_
#define TCONVERTER_H_

#include <string>

namespace NS_Converter
{
	//функции преобразования данных:

	template <class T>
	std::string toStr(T val, bool no_except = true) noexcept(false);

	template <class T>
	bool toType(const std::string& str, T* val, bool no_except = true) noexcept(false);

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

}

#endif
