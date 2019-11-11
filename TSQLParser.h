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


	//класс описывающий область запросов with/select/from/where/order by/group by
	class TSection
	{
	private:
		//по итогу не пригодилось вызывать разные функции поиска в строке
		//using find_function = std::size_t (std::string::*)(const string& s, const size_t pos) const;
		TConstSql name;//наименование блока
		string data;//данные строкой - полный sql-запрос
		//функци€ поиска ключевого слова в строке:
		static std::size_t find_word_pos(const string& str, const string& key, const size_t n, bool must_find = false);
		//функци€ возвращ€юща€ данные между ключевыми словами:
		static string get_data_by_key_range(const string& str, size_t& pos, const TConstSql& br,
			const TConstSql& er) noexcept(false);
		//функци€ добавлени€ пол€ к данным:
		void add_field_to_data(const string& str, const string& ch, bool use_brkt) noexcept(false);
		//функци€ установки данных дл€ пол€ data
		void set_data(const string& str);
		//функци установки полей по данным из пол€ data:
		void set_fields(const string& str) noexcept(true);
		void clear(void);
	protected:
		//получение границ дл€ указанного sql-наименовани€ дл€ строки(select: pair("select", "from"))
		inline string get_first_range_val(void) const { return name.toStr(); };
		inline string get_second_range_val(void) const { return TConstSql(name+1).toStr(); };
	public:
		//инициализаци€ с помощью sql-команды
		explicit TSection(const TSql& title, const string& str = "");
		explicit TSection(const TConstSql& title, const string& str = "");
		//можно заменить стандартным конструктором
		TSection(const TSection& sect) : name(sect.name), data(sect.data) {};
		//можно заменить стандартным присвоением:
		TSection& operator=(const TSection& s);
		//деструктор
		~TSection(void) {};
		bool operator<(const TSection& val) const { return name < val.Title(); };
		//провер€ем на пустоту:
		bool Empty(void) const { return name.isEmpty() || data.empty(); };
		//добавление пол€ данных:
		//str - строка данных, ch - разделитель дл€ полей данных,
		//use_brkt - использование скобок дл€ отделени€ старой части данных от новой
		void AddField(const string& str, const TConstSql& ch, bool use_brkt = false) noexcept(false);
		TConstSql Title(void) const { return name; };
		string Name(void) const { return name.toStr(); };
		string Data(void) const { return data; };
		void Data(const string& str) { set_data(str); }
		//преобразование в строку
		string to_Str(void) const;
		friend vector<string> getColumnValue(const TSection& section);
	};
	
	//класс дл€ работы с sql-текстом
	class TText
	{
	public:
		using TSectIndex = vector<TSection>::iterator;
		using TConstSectIndex = vector<TSection>::const_iterator;
	private:
		vector<TSection> sect;
		//функци€ инициализации секторов по строке:
		void Init_Sectors(const string& str);
		//функци€ получени€/поиска блока запроса по имени select/from/where
		TSectIndex operator[](const TConstSql& title);
	public:
		//инициализаци€ строкой
		TText(const string& str="");
		//инициализаци€ из потока данных
		TText(std::istream& stream);
		TText(const vector<TSection>& val) : sect(val) {};
		TText(const TText& val) : sect(val.sect) {};
		//TText& operator=(const TText& val);
		//деинициализаци€
		~TText() {};
		//проверка на пустоту:
		bool isEmpty() const { return sect.empty(); };
		//функци€ получени€ данных о секции:
		TSection operator[](const TConstSql& title) const;
		//получить текст запроса:
		string toStr(bool use_eoc = false) const;
		//функци€ добавлени€ блока запроса(+):
		void AddSection(const TSql& title, const string& str);
		void AddSection(const TSection& sect);
		//функци€ удалени€ блока запроса(+):
		bool DelSection(const TSql& title) noexcept(true);
		//функци€ добавлени€ данных в блок запроса(+):
		bool AddField2Section(const TSql& title, const string& str,
			const TSql& delimeter = TSql::Empty, bool brkt = false) noexcept(false);
	};
}
#endif // ! TSQL_PARSER_H_

