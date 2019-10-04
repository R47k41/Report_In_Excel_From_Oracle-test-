#ifndef LOGGER_H_
#define LOGGER_H_
#include <ostream>
#include <string>

//����� �����������
namespace Logger
{
	using std::string;
	using std::stringstream;
	//����� �����������
	class TLog
	{
	private:
		string msg;
		void new_line(void) { msg += '\n'; }
	public:
		TLog(const string& str) :msg(str) {};
		TLog(const TLog& log) : msg(log.msg) {};
		TLog(const char* str, ...);
		template <class T>
		void add(const T& val);
		//����� ��������� �� ������:
		const char *what() const { return msg.c_str(); };
		//����� ������ � �����
		friend std::ostream& operator<<(std::ostream& stream, const TLog& log);
	};

}

#endif LOGGER_H_
