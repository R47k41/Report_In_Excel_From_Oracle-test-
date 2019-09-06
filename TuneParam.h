//#pragma once
#ifndef TUNE_PARAM_H_
#define TUNE_PARAM_H_
//модуль предназначен для описания класса работающего с файлом настроек
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <ctype.h>
#include <iterator>

//область видимости для константных объектов:
namespace Constant_Data
{
	using std::string;
	const char Space = ' ';
	const string delimeters(" \n\t,;");
	//операторы SQL запроса
	namespace SQL_OPERATOR
	{
		const string Select = "select";
		const string From = "from";
		const string Where = "where";
		const string Order = "order by";
		const string As = "as";
		const string EndCommand = ";";
		const string And = "and";
		const string Or = "or";
		const string ColumnDelim = ",";
	}
	//параметры файла настроки
	namespace Config_Param
	{
		const string DataDevider = "=";//разделитель данных от наименований
		const string DataBase("[DATA BASE]");
		const string ReportParam("[REPORT]");
		const string ColumnsParam("[COLUMNS]");
		const string UserName("UserName");
		const string Pass("Password");
		const string ConnectionStr("TNS");
		const string ColumnName("Column");
	}
}

//область видимости для функций работы со строками:
namespace String_Fnc {};

//обаласти пространства имен для работы с настройками
namespace NS_Tune
{
	using std::string;
	using std::ifstream;
	using std::pair;
	using std::vector;

	//Класс - данные для подключения:
	class UserData
	{
	private:
		string name;
		string pass;
		string ConnStr;
	public:
		UserData(const string& par_name, const string& par_pass, const string& par_DB) :
			name(par_name), pass(par_pass), ConnStr(par_DB) {};
		//установка значений
		void setName(const string& val) { name = val; };
		void setPass(const string& val) { pass = val; };
		void setConnStr(const string& val) { ConnStr = val; };
		//считываение значений
		string getName() const { return name; };
		string getPass() const { return pass; };
		string getConnStr() const { return ConnStr; };
	};

	class TColumn;
	class TRow;
	//тип строка таблицы
	using TRowIterator = vector<TColumn>::iterator;
	using TRowConstIterator = vector<TColumn>::const_iterator;


	//родительский класс для колонки и строки
	class TSection
	{
		private:
			string name;
		public:
			TSection(const string& val) : name(val) {};
			string toString() const { return name; };
	};

	//описание класса Колонка
	class TColumn
	{
		private:
			string title;
			string name;
		public:
			TColumn(const string& p_title = "", const string& p_name = "") : title(p_title), name(p_name) {};
			TColumn(const std::pair<string, string>& p) : title(p.first), name(p.second) {};
			std::pair<string, string> getPair() const { return std::make_pair(title, name); };
			bool isEmpty() const { return title.empty() && name.empty(); };
			string getName() const { return name; };
			string getTitle() const { return title; };
			string toString(const string& d) const;
			string toString(const char d) const { return toString(std::string{ d }); };
	};

	//описание класса Строка:
	class TRow
	{
		private:
			string name;
			vector<TColumn> row;
			void InitRow(const string::iterator& first, const string::iterator& second);
		public:
			TRow(): name(), row() {};
			//инициализация блока интервалом данных
			TRow(const string& bblock, const string& eblock);
			void AddCol(const TColumn& col) { row.push_back(col); };
			void Clear() { row.clear(); };
			bool isEmpty() const { return row.empty(); };
			TRowConstIterator Begin() const { return row.begin(); };
			TRowConstIterator End()	const { return row.end(); };
			string toString(const string& row_delimeter, const string& col_delimeter) const;
			string toString(const char& row_delimeter, const char& col_delimeter) const { return toString(string{ row_delimeter }, string{ col_delimeter }); };
	};

	//Класс для работы с SQL запросом(без вложенных запросов!):
	class SQL_Text
	{
	private:
		TRow slct_sec;//сектор с колонками запроса SELECT
		TRow frm_sec;//сектор с таблицам FROM
		string whr_sec;//сектор условия WHERE
		string ord_sec;//сектор сортировки ORDER
		//получеие строки из колонки
		//static string TRow
		//инициализация строкой
		void init_by_str(const string& str);
	public:
		//операции для условия where
		SQL_Text(const string& str = "");
		//возвращаем массив колонок
		TRow getColumns() const { return slct_sec; };
		//возвращаем массив таблиц
		TRow getTables() const { return frm_sec; };
		//возвращаем строку условий
		string getWhereClause() const { return whr_sec; };
		//возвращаем строку сортировки:
		string getOrderClause() const { return ord_sec; };
		//добавление условия для запроса:
		void AddWhereCond(const string& val, const string& opr);
		//удаление условия для выборки
		void ClearWhereSec() { whr_sec = ""; };
		//добавление условия для сортировки
		void AddOrderVal(const string& val);
		//удаление условия для сортировки:
		void ClearOrderSec() { ord_sec = ""; };
		//представление sql-запроса в виде строки
		string ToString() const;
	};

	//класс работы с файлом настроек
	class Tune
	{
	private:
		//наименование файла с данными настроек
		string src_file;
		//наименование выходного файла
		string out_file;
		//данные пользователя для подключения
		UserData* usr_data;
		//текст запроса для отчета
		SQL_Text* rep_SQL;
		//строка заголовка столбцов для отчета
		TRow* rep_cols;
		//функция инициализации данных из файла настроек
		void read_tune_file(const string& file_name) noexcept(false);
	public:
		//конструктор по умолчанию
		Tune(const string& file_name = "");
		//деструктор
		~Tune();
		//получение имени файла:
		string getSrcName() const { return src_file; };
		//получение данных для запроса
		string getSQL(void) const { return rep_SQL->ToString(); };
		//установка условия для запроса
	};

}

namespace String_Fnc
{
	using std::string;
	using NS_Tune::TRow;
	using TStrings = std::vector<string>;
	using TStringsIterator = TStrings::iterator;
	using TStringsConstIterator = TStrings::const_iterator;
	using TInterval = std::pair<TStrings::iterator, TStrings::iterator>;
	//функция преобразования строки в массив строк за счет разделения строк по служебным символам:
	TStrings splitStr(const string& str, const string& delimen = Constant_Data::delimeters);
	//функция получения интервала для копирования:
	TInterval getInterval(const TStringsIterator& ibeg, const TStringsIterator& iend,
		const string& op_beg, const string& op_end);
	//функция заполнения сектора/блока
	TRow fill_sec(const TInterval& range);
	//получение строки из интервала данных
	string fill_str(const TInterval& range);
}

#endif TUNE_PARAM_H_