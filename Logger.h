#ifndef LOGGER_H_
#define LOGGER_H_
#include <string>
#include <sstream>

//Класс Логирования
namespace NS_Logger
{
	using std::string;
	using std::stringstream;

	//класс логирования
	class TLog
	{
	private:
		stringstream msg;
		mutable string buf;//выключаем ограничения на изменения для const-функций
		void new_line(const char* ch = "\n") { msg << ch; }
		//сброс буфера и состояний, если надо:
		void clear(bool clear_state = false);
	public:
		explicit TLog(const string& str = "");
		TLog(const TLog& log);
		//проверка на пустоту:
		bool isEmpty() const { return msg.str().empty(); }
		//проверка работоспособности:
		bool isGood() const { return msg.good(); }
		//вывод сообщения об ошибке:
		const char* what() const noexcept(true);
		string getStr() const { return msg.str(); }
		//оператор присвоения:
		TLog& operator=(const char* pstr);
		TLog& operator=(const string& str) { return operator=(str.c_str()); }
		TLog& operator=(const TLog& log) { return operator=(log.msg.str()); }
		//добавление текста к сообщению:
		template <class T>
		TLog& operator+=(T val);
		template <class T>
		TLog& operator<<(T val) { operator+=(val); return *this; };
		template <class T>
		TLog& operator+(T val) { operator+=(val); return *this; };
		//опертор вызова исключения/логирования:
		void raise(bool shw_raise = false, const char* place_from = '\0') const;
		//friend stringstream& operator<<(stringstream& ss, const TLog& log);
	};
}

#endif LOGGER_H_
