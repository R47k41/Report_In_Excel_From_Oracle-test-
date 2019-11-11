#ifndef LOGGER_H_
#define LOGGER_H_
#include <string>
#include <sstream>

//����� �����������
namespace NS_Logger
{
	using std::string;
	using std::stringstream;

	//����� �����������
	class TLog
	{
	private:
		stringstream msg;
		mutable string buf;//��������� ����������� �� ��������� ��� const-�������
		void new_line(const char* ch = "\n") { msg << ch; }
		//����� ������ � ���������, ���� ����:
		void clear(bool clear_state = false);
	public:
		explicit TLog(const string& str = "");
		TLog(const TLog& log);
		//�������� �� �������:
		bool isEmpty() const { return msg.str().empty(); }
		//�������� �����������������:
		bool isGood() const { return msg.good(); }
		//����� ��������� �� ������:
		const char* what() const noexcept(true);
		string getStr() const { return msg.str(); }
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
		void raise(bool shw_raise = false, const char* place_from = '\0') const;
		//friend stringstream& operator<<(stringstream& ss, const TLog& log);
	};
}

#endif LOGGER_H_
