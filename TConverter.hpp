#include <Windows.h>
#include "TConverter.h"


using std::string;

//����������� ansi-������ � unicode
std::wstring NS_Converter::MByteToUnicode(const std::string& str, size_t toCodePage) noexcept(false)
{
	using NS_Logger::TLog;
	std::wstring result;
	wchar_t* unicodeStr = nullptr;
	try
	{
		//��������� �������
		size_t size;
		size = MultiByteToWideChar(toCodePage, 0, str.c_str(), -1, NULL, 0);
		if (size < 0)
			throw string("������ ��� ����������� ������� �������������� ������!");
		//������� unicode ������:
		unicodeStr = new wchar_t[size];
		//������������ � unicode:
		if (MultiByteToWideChar(toCodePage, 0, str.c_str(), -1, unicodeStr, size) < 0)
			throw string("������ ��� �����������!");
		result = std::wstring(unicodeStr);
	}
	catch (const std::string& er)
	{
		if (unicodeStr) delete[] unicodeStr;
		TLog log(er, "MByteToUnicode");
		log << "(������� ��������: " << toCodePage << "; �����: " << str << " )";
		throw log;
	}
	catch (...)
	{
		if (unicodeStr) delete[] unicodeStr;
		TLog log("�� ������������ ������ ��� ����������� ������: ", "MByteToUnicode");
		log << str << " ��� ������� ��������: " << toCodePage << "!";
		throw log;
	}
	if (unicodeStr) delete[] unicodeStr;
	return result;
}

//����������� unicode-������ � ansi
std::string NS_Converter::UnicodeToMByte(const std::wstring& unicodeStr, size_t toCodePage) noexcept(false)
{
	using NS_Logger::TLog;
	std::string result;
	char* multiByteStr = nullptr;
	try
	{
		//��������� �������
		size_t size;
		size = WideCharToMultiByte(toCodePage, NULL, unicodeStr.c_str(), -1, NULL, 0, NULL, FALSE);
		if (size < 0)
			throw string("������ ��� ����������� ������� �������������� ������!");
		//������� unicode ������:
		multiByteStr = new char[size];
		//������������ � unicode:
		if (WideCharToMultiByte(toCodePage, NULL, unicodeStr.c_str(), -1, multiByteStr, size, NULL, FALSE) < 0)
			throw string("������ ��� �����������!");
		result = std::string(multiByteStr);
	}
	catch (const string& er)
	{
		if (multiByteStr) delete[] multiByteStr;
		TLog log(er, "MByteToUnicode");
		log << "(������� ��������: " << toCodePage << ")";
		throw log;
	}
	catch (...)
	{
		if (multiByteStr) delete[] multiByteStr;
		TLog log("�� ������������ ������ ��� ����������� ������ ��� ������� ��������: ", "MByteToUnicode");
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
	//�������������� �� ������������ ������ ����� � utf8:
	wstring wStr = MByteToUnicode(inStr.c_str(), CP_UTF8);
	//�������������� �� unicode � ansi:
	inStr = UnicodeToMByte(wStr.c_str(), CP_ACP);
	return true;
}
/**/