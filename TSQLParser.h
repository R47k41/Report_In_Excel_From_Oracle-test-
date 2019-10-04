#ifndef  TSQL_PARSER_H_
#define TSQL_PARSER_H_
#include <string>
#include <vector>
#include <iterator>
#include <ostream>
#include <utility>

namespace NS_Sql
{
	using std::string;
	using std::vector;
	using std::pair;

	//класс контрольных групп:
	struct TCtrlGroup
	{
		vector<pair<char, char> > excluded_symb;
		//операторы sql:
		enum class TCtrlSql { Empty = 0, With, Select, From, Where, Order, Group, As, And, Or, EOC, D4L};
		//контрольные символы:
		enum class TCtrlSym {EOL = '\0', Space = ' ', NL = '\n', EndCommand = ';', EndCol = ',', lbkt = '(', rbkt = ')', crwn = '"', Tab = '\t' };
		//преобразование sql команды в строку
		static string CtrlSql2Str(const TCtrlSql& val);
		//преобразование контрольного символа в символ:
		static inline char CtrlSym2Char(const TCtrlSym& s) { return static_cast<char>(s); };
		static string CtrlSym2Str(const TCtrlSym& ch);
		//проверка обязательности наличия команды в запросе:
		static bool MustFound(const TCtrlSql& s);
		//функция возвращающая разделитель
		static TCtrlGroup::TCtrlSql GetDelimeterByTitle(const TCtrlGroup::TCtrlSql& title) noexcept(false);
		//функция получения разделителя по наименованию
		static bool CorrectDelimeter(const TCtrlSql& title, const string& d);
		//функция проверяющая возможность использования скобок в запросе:
		static bool CanUseBrkt(const TCtrlSql& title) { return title == TCtrlSql::Where; };
		//функция определяющая закрывающий элемент по команде:
		static string getClosedElem(const TCtrlSql& val) noexcept(true);
		//инициализация
		TCtrlGroup();
		//проверка на корректность найденных данных:
		bool IsCorrectSym(const string& str, std::size_t pos, const std::size_t cnt = 1);
		//вывод данныъ TCtrlSym в поток
		friend std::ostream& operator<<(std::ostream& stream, const TCtrlSym& val);
	};

	//функция инкремента для TCtrlSql:
	TCtrlGroup::TCtrlSql operator+(const TCtrlGroup::TCtrlSql& val, int x) noexcept(true);

	//функция сверки со строкой:
	bool operator==(const TCtrlGroup::TCtrlSql& val, const string& str) noexcept(true);

	//класс описывающий область запросов with/select/from/where/order by/group by
	class TSection
	{
	private:
		//по итогу не пригодилось вызывать разные функции поиска в строке
		//using find_function = std::size_t (std::string::*)(const string& s, const size_t pos) const;
		TCtrlGroup::TCtrlSql name;//наименование блока
		string data;//данные строкой - полный sql-запрос
		//функция поиска ключевого слова в строке:
		static std::size_t find_word_pos(const string& str, const string& key, const size_t n, bool must_find = false);
		//функция возвращяющая данные между ключевыми словами:
		static string get_data_by_key_range(const string& str, size_t& pos, const TCtrlGroup::TCtrlSql& br,
			const TCtrlGroup::TCtrlSql& er) noexcept(false);
		//функция добавления поля к данным:
		void add_field_to_data(const string& str, const string& ch, bool use_brkt) noexcept(false);
		//функция установки данных для поля data
		void set_data(const string& str);
		//функци установки полей по данным из поля data:
		void set_fields(const TCtrlGroup::TCtrlSql& title, const string& str) noexcept(false);
		void clear(void);
	protected:
		//установка наименования по контрольному sql-слову: (select -> "select")
		void set_by_sql_name(const TCtrlGroup::TCtrlSql& title) { name = title; };
		//получение границ для указанного sql-наименования для строки(select: pair("select", "from"))
		inline string get_first_range_val(void) const { return TCtrlGroup::CtrlSql2Str(name); };
		inline string get_second_range_val(void) const { return TCtrlGroup::CtrlSql2Str(name + 1); };
	public:
		//инициализация с помощью sql-команды
		TSection(const TCtrlGroup::TCtrlSql& title, const string& str = "");
		//можно заменить стандартным конструктором
		TSection(const TSection& sect) : name(sect.name), data(sect.data) {};
		//можно заменить стандартным присвоением:
		TSection& operator=(const TSection& s);
		//деструктор
		~TSection(void) {};
		bool operator<(const TSection& val) const { return name < val.Title(); };
		//проверяем на пустоту:
		bool Empty(void) const { return name == TCtrlGroup::TCtrlSql::Empty || data.empty(); };
		//добавление поля данных:
		//str - строка данных, ch - разделитель для полей данных,
		//use_brkt - использование скобок для отделения старой части данных от новой
		void AddField(const string& str, const TCtrlGroup::TCtrlSql& ch, bool use_brkt = false) noexcept(false);
		TCtrlGroup::TCtrlSql Title(void) const { return name; };
		string Name(void) const { return TCtrlGroup::CtrlSql2Str(name); };
		string Data(void) const { return data; };
		void Data(const string& str) { set_data(str); }
		//преобразование в строку
		string to_Str(void) const;
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
		TSectIndex operator[](const TCtrlGroup::TCtrlSql& title);
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
		TSection operator[](const TCtrlGroup::TCtrlSql& title) const;
		//получить текст запроса:
		string toStr(void) const;
		//функция добавления блока запроса(+):
		void AddSection(const TCtrlGroup::TCtrlSql& title, const string& str);
		void AddSection(const TSection& sect);
		//функция удаления блока запроса(+):
		bool DelSection(const TCtrlGroup::TCtrlSql& title) noexcept(true);
		//функция добавления данных в блок запроса(+):
		bool AddField2Section(const TCtrlGroup::TCtrlSql& title, const string& str,
			TCtrlGroup::TCtrlSql delimeter = TCtrlGroup::TCtrlSql::Empty, bool brkt = false) noexcept(false);
	};
}
#endif // ! TSQL_PARSER_H_

