#ifndef LOGGER_H_
#define LOGGER_H_
#include <string>
#include <iostream>
#include <sstream>

//����� �����������
namespace NS_Logger
{
	using std::string;
	using std::stringstream;

	//����� �����������
	class TLog
	{
	public:
		static const char NL = '\n';
	private:
		string name;
		stringstream msg;
		mutable string buf;//��������� ����������� �� ��������� ��� const-�������
		void new_line(const char ch = NL) { msg << ch; }
		//����� ������ � ���������, ���� ����:
		void clear(bool clear_state = false);
	public:
		explicit TLog(const string& str = "", const string& fnc_name = "");
		TLog(const TLog& log);
		//�������� �� �������:
		bool isEmpty() const { return msg.str().empty(); }
		//�������� �����������������:
		bool isGood() const { return msg.good(); }
		//��������� �����
		void setFncName(const string& fnc_name) noexcept(true) { name = fnc_name; }
		string Name() const { return name; }
		//����� ��������� �� ������:
		const char* what() const noexcept(true);
		//��������� ������ ������
		string getStr(bool full = true) const noexcept(true);
		//�������� ����������:
		TLog& operator=(const char* pstr);
		TLog& operator=(const string& str) { return operator=(str.c_str()); }
		TLog& operator=(const TLog& log) { return operator=(log.msg.str()); }
		//���������� ������ � ���������:
		template <class T>
		TLog& operator+=(T val);
		template <class T>
		TLog& operator<<(T val) { operator+=(val); return *this; };
		template <class T>
		TLog& operator+(T val) { operator+=(val); return *this; };
		//������� ������ ����������/�����������:
		void toErrBuff(std::ostream& stream = std::cerr) const;
		//friend stringstream& operator<<(stringstream& ss, const TLog& log);
	};
}

#endif LOGGER_H_
