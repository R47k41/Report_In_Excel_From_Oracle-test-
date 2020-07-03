#ifndef LOGGER_H_
#define LOGGER_H_
#include <string>
#include <iostream>
#include <sstream>

//Класс Логирования
namespace NS_Logger
{
	using std::string;
	using std::stringstream;

	//класс логирования
	class TLog
	{
	public:
		static const char NL = '\n';
	private:
		string name;
		stringstream msg;
		mutable string buf;//выключаем ограничения на изменения для const-функций
		void new_line(const char ch = NL) { msg << ch; }
	public:
		explicit TLog(const string& str = "", const string& fnc_name = "");
		TLog(const TLog& log);
		//сброс буфера и состояний, если надо:
		void clear(bool clear_state = false);
		//проверка на пустоту:
		bool isEmpty() const { return msg.str().empty(); }
		//проверка работоспособности:
		bool isGood() const { return msg.good(); }
		//установка имени
		void setFncName(const string& fnc_name) noexcept(true) { name = fnc_name; }
		string Name() const { return name; }
		//добавление строки к сообщению:
		inline void AddStr(const char const* str) { msg << str; }
		inline void AddStr(const string& str) { msg << str; }
		//вывод сообщения об ошибке:
		const char* what() const noexcept(true);
		//получение вывода данных
		string getStr(bool full = true) const noexcept(true);
		//оператор присвоения:
		TLog& operator=(const char* pstr);
		TLog& operator=(const string& str) { return operator=(str.c_str()); }
		TLog& operator=(const TLog& log) { return operator=(log.msg.str()); }
		//опертор вызова исключения/логирования:
		void toErrBuff(std::ostream& stream = std::cerr) const;
		//friend stringstream& operator<<(stringstream& ss, const TLog& log);
	};
}

#endif LOGGER_H_
