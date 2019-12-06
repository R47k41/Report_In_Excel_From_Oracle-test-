#ifndef  TSQL_PARSER_H_
#define TSQL_PARSER_H_
#include <string>
#include <vector>
#include <iterator>
#include <ostream>
#include <utility>
#include "TConstants.h"

namespace NS_Sql
{
	using std::string;
	using std::vector;
	using std::pair;
	using NS_Const::TSql;
	using NS_Const::CtrlSym;
	using NS_Const::TConstSql;
	using NS_Const::TConstCtrlSym;
	using NS_Const::TSymGroup;

	//базовый класс для хранения sql-запросов:
	class TSimpleSql
	{
	private:
		string text;//текст запроса
	protected:
		//стороны для избавления от пробелов:
		enum class Side { Left, Right, Full };
		enum class KeyCase {Lower, Upper};
		//установка выражения для sql-запроса
		void set_data(const string& sql, bool use_trim = true);
		//получение данных о sql-запросе:
		string get_data() const noexcept(true) { return text; }
		//функция поиска ключевого слова в строке:
		static std::size_t find_word_pos(const string& str, const string& key, const size_t n, bool must_find = false);
		//проверка является ли строка коментарием
		static bool isCommentLine(const string& str) noexcept(true);
	public:
		//дефолтная инициализация
		TSimpleSql() :text() {}
		//инициализация строкой
		explicit TSimpleSql(const string& txt, bool use_trim = false);
		//инициализация файлом
		explicit TSimpleSql(std::istream& file);
		TSimpleSql(const TSimpleSql& sql) : text(sql.text) {}
		//проверка на пустоту:
		virtual bool Empty() const { return text.empty(); }
		//установка данных запроса:
		virtual void Data(const string& val) { set_data(val); }
		//получение данных запроса:
		virtual string Data() const noexcept(true) { return get_data(); }
		//очистка данных:
		virtual void clear() { text.clear(); }
		//убираем пробелы по краям :
		void Trim(const Side& flg = Side::Full);
		//меняем регистр данных:
		string toCase(const KeyCase& flg) const;
		//присвоение:
		TSimpleSql& operator=(const string& val) { set_data(val); }
		//признак наличия параметров в запросе:
		bool hasParams() const;
		//функция считывания sql-кода и из файла:
		//пропуск коментариев работает только для строчных коментариев
		//коментарии должны начинаться с начала строки
		static string read_sql_file(std::istream& file, bool skip_coment = true) noexcept(false);
		//функция преобрпзования в строку:
		friend string AsString(const TSimpleSql& sql);
	};

	//класс описывающий область запросов with/select/from/where/order by/group by
	class TSection: public TSimpleSql
	{
	private:
		TConstSql name;//наименование блока
		//функция возвращяющая данные между ключевыми словами:
		static string get_data_by_key_range(const string& str, size_t& pos, const TConstSql& br,
			const TConstSql& er) noexcept(false);
		//функция добавления поля к данным:
		void add_field_to_data(const string& str, const string& ch, bool use_brkt) noexcept(false);
		//функци установки полей по данным из поля data:
		void set_fields() noexcept(true);
	protected:
		//получение границ для указанного sql-наименования для строки(select: pair("select", "from"))
		inline string get_first_range_val(void) const { return name.toStr(); };
		inline string get_second_range_val(void) const { return TConstSql(name+1).toStr(); };
	public:
		//инициализация с помощью sql-команды
		explicit TSection(const TSql& title, const string& str = "");
		explicit TSection(const TSql& title, const TSimpleSql& slq);
		explicit TSection(const TConstSql& title, const string& str = "");
		//можно заменить стандартным конструктором
		TSection(const TSection& sect) : TSimpleSql(sect.Data()), name(sect.name) {};
		//можно заменить стандартным присвоением:
		TSection& operator=(const TSection& s);
		//деструктор
		~TSection(void) {};
		bool operator<(const TSection& val) const { return name < val.Title(); };
		//проверяем на пустоту:
		bool Empty(void) const { return TSimpleSql::Empty() || name.isEmpty(); };
		//добавление поля данных:
		//str - строка данных, ch - разделитель для полей данных,
		//use_brkt - использование скобок для отделения старой части данных от новой
		void AddField(const string& str, const TConstSql& ch, bool use_brkt = false) noexcept(false);
		TConstSql Title(void) const { return name; };
		string Name(void) const { return name.toStr(); };
		//преобразование в строку
		string to_Str(void) const;
		//функция очистки данных:
		void clear() { TSimpleSql::clear(); name = TSql::Empty; }
		friend vector<string> getColumnValue(const TSection& section);
	};
	
	//класс для работы с sql-текстом
	class TText
	{
	public:
		using TSectIndex = vector<TSection>::iterator;
		using TConstSectIndex = vector<TSection>::const_iterator;
	private:
		vector<TSection> sect;
		//функция инициализации секторов по строке:
		void Init_Sectors(const string& str);
		//функция получения/поиска блока запроса по имени select/from/where
		TSectIndex operator[](const TConstSql& title);
	public:
		//инициализация строкой
		TText(const string& str="");
		//инициализация из потока данных
		TText(std::istream& stream);
		TText(const vector<TSection>& val) : sect(val) {};
		TText(const TText& val) : sect(val.sect) {};
		//TText& operator=(const TText& val);
		//деинициализация
		~TText() {};
		//проверка на пустоту:
		bool isEmpty() const { return sect.empty(); };
		//функция получения данных о секции:
		TSection operator[](const TConstSql& title) const;
		//получить текст запроса:
		string toStr(bool use_eoc = false) const;
		//функция добавления блока запроса(+):
		void AddSection(const TSql& title, const string& str);
		void AddSection(const TSection& sect);
		//функция удаления блока запроса(+):
		bool DelSection(const TSql& title) noexcept(true);
		//функция добавления данных в блок запроса(+):
		bool AddField2Section(const TSql& title, const string& str,
			const TSql& delimeter = TSql::Empty, bool brkt = false) noexcept(false);
		//функция преобрпзования в строку:
		friend string AsString(const TText& sql);
	};

	//преобразование распарсинного текста в строку:
	std::string AsString(const TText& sql);
	//получение текста из строки:
	std::string AsString(const TSimpleSql& slq);
}
#endif // ! TSQL_PARSER_H_

