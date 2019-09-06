#include "TSQLParser.h"
#include "Logger.h"
#include <sstream>
#include <iterator>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <set>

using std::string;
using std::vector;
using std::pair;
using std::cout;
using std::endl;
using std::cerr;

namespace NS_Sql
{
	//преобразование в нижний регистр:
	string LowerCase(const string& str);
	//убираем из строки служебные символа:
	void DeleteServiceSymb(string& str);
	//функци€ убирающа€ пробелы из начали и конца строки:
	void Trim_Left(string& str);
	void Trim_Right(string& str);
	void Trim(string& str);
	//класс дл€ сравнени€ со строками в качестве предиката:
	class TTrimObj
	{
	private:
		string symb;
	public:
		TTrimObj(const string& arr) : symb(arr) {};
		TTrimObj(const TCtrlGroup::TCtrlSql& title);
		//манипул€ци€ с символами:
		bool operator()(const char& ch) const;
		//операци€ сравнени€ секторов по имени:
		bool operator()(const TSection& sect) const;
	};
};

//преобразование в нижний регистр:
string NS_Sql::LowerCase(const string& str)
{
	string result;
	std::transform(str.begin(), str.end(), std::insert_iterator<std::string>(result, result.begin()), tolower);
	return result;
}

NS_Sql::TTrimObj::TTrimObj(const TCtrlGroup::TCtrlSql& title)
{
	symb = TCtrlGroup::CtrlSql2Str(title);
}

bool NS_Sql::TTrimObj::operator()(const char& ch) const
{
	for (char v : symb)
		if (v == ch)
			return true;
	return false;
};

bool NS_Sql::TTrimObj::operator()(const TSection& sect) const
{
	return sect.Name() == symb;
}

void NS_Sql::DeleteServiceSymb(string& str)
{
	return;
	string syms = { TCtrlGroup::CtrlSym2Char(TCtrlGroup::TCtrlSym::NL),
		TCtrlGroup::CtrlSym2Char(TCtrlGroup::TCtrlSym::Tab) };
	//std::transform(str.begin(), str.end(), str.begin(), TTrimObj(syms));
	std::copy_if(str.begin(), str.end(), std::insert_iterator<string>(str, str.begin()), TTrimObj(syms));
};

void NS_Sql::Trim_Left(string& str)
{
	string syms = { TCtrlGroup::CtrlSym2Char(TCtrlGroup::TCtrlSym::Space),
		TCtrlGroup::CtrlSym2Char(TCtrlGroup::TCtrlSym::NL),
		TCtrlGroup::CtrlSym2Char(TCtrlGroup::TCtrlSym::Tab) };
	string::const_iterator b = std::find_if_not(str.begin(), str.end(), TTrimObj(syms));
	if (b >= str.begin()) str.erase(str.begin(), b);
};
void NS_Sql::Trim_Right(string& str)
{
	string syms = { TCtrlGroup::CtrlSym2Char(TCtrlGroup::TCtrlSym::Space),
		TCtrlGroup::CtrlSym2Char(TCtrlGroup::TCtrlSym::NL),
		TCtrlGroup::CtrlSym2Char(TCtrlGroup::TCtrlSym::Tab) };
	string::const_reverse_iterator b = std::find_if_not(str.rbegin(), str.rend(), TTrimObj(syms));
	if (b != str.rbegin()) str.erase(b.base(), str.end());
};
void NS_Sql::Trim(string& str)
{
	Trim_Left(str);
	Trim_Right(str);
}

NS_Sql::TCtrlGroup::TCtrlSql NS_Sql::operator+(const NS_Sql::TCtrlGroup::TCtrlSql& val, int x) noexcept(true)
{
	int v = int(val);
	if (v + x <= int(NS_Sql::TCtrlGroup::TCtrlSql::Group))
		return NS_Sql::TCtrlGroup::TCtrlSql(v + x);
	else
		return NS_Sql::TCtrlGroup::TCtrlSql::EOC;
		//throw Logger::TLog("¬ыход за границы указанных sql-команд!");

}

string NS_Sql::TCtrlGroup::CtrlSym2Str(const TCtrlSym& ch)
{
	string tmp;
	tmp.push_back(CtrlSym2Char(ch));
	return tmp;
};

//преобразование контрольного sql слова в строку:
string NS_Sql::TCtrlGroup::CtrlSql2Str(const NS_Sql::TCtrlGroup::TCtrlSql& val)
{
	switch (val)
	{
	case TCtrlSql::With: return "with";
	case TCtrlSql::Select: return "select";
	case TCtrlSql::From: return "from";
	case TCtrlSql::Where: return "where";
	case TCtrlSql::Order: return "order by";
	case TCtrlSql::Group: return "group by";
	case TCtrlSql::As: return " as ";
	case TCtrlSql::And: return " and ";
	case TCtrlSql::Or: return " or ";
	case TCtrlSql::EOC: //return ";";//символ окончани€ команды
		return CtrlSym2Str(TCtrlSym::EndCommand);
	case TCtrlSql::D4L: //return ",";//разделитель строк в select/from/order/group
		return CtrlSym2Str(TCtrlSym::EndCol);
	default: return string();
	}
}

//вывод символа в поток:
std::ostream& NS_Sql::operator<<(std::ostream& stream, const NS_Sql::TCtrlGroup::TCtrlSym& val)
{
	if (!stream) return stream;
	return stream << TCtrlGroup::CtrlSym2Char(val);
}


//формируем список запрещенных символов: '(', ')', '"'
NS_Sql::TCtrlGroup::TCtrlGroup()
{
	pair<char, char> p = { CtrlSym2Char(TCtrlSym::lbkt), CtrlSym2Char(TCtrlSym::rbkt) };
	excluded_symb.push_back(p);
	p.first = CtrlSym2Char(TCtrlSym::crwn);
	p.second = CtrlSym2Char(TCtrlSym::crwn);
	excluded_symb.push_back(p);
}

bool NS_Sql::TCtrlGroup::IsCorrectSym(const string& str, std::size_t pos, const std::size_t cnt)
{
	using std::size_t;
	using std::isalnum;
	if (pos == 0 and str.size() > (pos + cnt + 1)
		and !isalnum(str[pos + cnt + 1])) return true;
	//провер€ем, что это не часть слова:
	if (pos - 1 >= 0 and isalnum(str[pos - 1]) and
		(pos + cnt + 1) < str.size() and  isalnum(str[pos + cnt + 1])) return false;
	//провер€ем не вложен ли разделитель в подзапрос:
	for (pair<char, char> v : excluded_symb)
	{
		size_t p = str.find(v.second, pos);
		if (p == string::npos) continue;
		else
		{
			p = str.find(v.first);
			if (v.first == TCtrlGroup::CtrlSym2Char(TCtrlGroup::TCtrlSym::crwn))
				if (p > pos) continue;
				else return false;
			if (p == string::npos || p < pos) continue;
			else return false;
		}
	}
	return true;
}

bool NS_Sql::TCtrlGroup::MustFound(const TCtrlSql& s)
{
	switch (s)
	{
	//case TCtrlSql::With:
	case TCtrlSql::Select:
	case TCtrlSql::From:
	case TCtrlSql::EOC:
		return true;
	default: return false;
	}
}

NS_Sql::TCtrlGroup::TCtrlSql NS_Sql::TCtrlGroup::GetDelimeterByTitle(const TCtrlGroup::TCtrlSql& title) noexcept(false)
{
	switch (title)
	{
	case TCtrlSql::Select:
	case TCtrlSql::From:
	case TCtrlSql::Order:
	case TCtrlSql::Group:
		return TCtrlSql::D4L;
	case TCtrlSql::Where:
		string val = CtrlSql2Str(title);
		throw Logger::TLog("ƒл€ блока: ", val.c_str(), " имеетс€ больше одного разделител€!", nullptr);
	}
	return TCtrlSql::Empty;
};

bool NS_Sql::TCtrlGroup::CorrectDelimeter(const TCtrlSql& title, const string& d)
{
	string result;
	try
	{
		result = CtrlSql2Str(GetDelimeterByTitle(title));
		if (result == d) return true;
		return false;
	}
	catch (const Logger::TLog& er)
	{
		if (title == TCtrlSql::Where)
			return (CtrlSql2Str(TCtrlSql::And) == d || CtrlSql2Str(TCtrlSql::Or) == d);
		else
		{
			cerr << er.what() << endl;
			return false;
		}
	}
};

void NS_Sql::TSection::set_data(const string& str)
{
	data = str;
	Trim(data);
};

std::size_t NS_Sql::TSection::find_word_pos(const string& str, const string& key, const size_t n, bool must_find)
{
	using std::size_t;
	if (str.empty() || key.empty()) return string::npos;
	size_t pos = n;
	//провер€ем €вл€етс€ ли найденна€ позици€ верной:
	TCtrlGroup ctrl;
	while (pos <= str.size())
	{
		pos = str.find(key, pos);
		if (pos == string::npos)
			if (must_find)
				throw Logger::TLog("ќб€зательный элемент: ",
					key.c_str(), " не найден в выражении: ", str.c_str(), nullptr);
			else break;
		if (ctrl.IsCorrectSym(str, pos, key.size())) break;
		else pos += key.size() + n;
	};
	return pos;
};

void NS_Sql::TSection::set_fields(const TCtrlGroup::TCtrlSql& title, const string& str) noexcept(false)
{
	using std::size_t;
	set_by_sql_name(title);
	string in_str = LowerCase(str);
	//если строка данных - пуста или не заполнено им€ секции - выход
	if (in_str.empty()) return;
	//определ€ем границы дл€ получени€ полей данных:
	pair<string, string> range(get_first_range_val(), get_second_range_val());
	//ищем началюную позицию
	size_t posb = find_word_pos(in_str, range.first, 0, TCtrlGroup::MustFound(name));
	if (posb == string::npos)
	{
		clear();
		return;
	};
	posb += range.first.size();
	size_t pose = find_word_pos(in_str, range.second, posb, TCtrlGroup::MustFound(name + 1));
	if (pose == string::npos)
	{
		range.second = TCtrlGroup::CtrlSql2Str(TCtrlGroup::TCtrlSql::EOC);
		pose = find_word_pos(in_str, range.second, posb, TCtrlGroup::MustFound(TCtrlGroup::TCtrlSql::EOC));
		if (pose == string::npos)
			throw Logger::TLog("ќтсутствует символ окончани€ выражени€ ", range.second.c_str(), nullptr);
	}
	//формируем поле данных:
	in_str = in_str.substr(posb + 1, pose - posb - 1);
	set_data(in_str);
};

void NS_Sql::TSection::clear(void)
{
	name = TCtrlGroup::TCtrlSql::Empty;
	data.clear();
};

NS_Sql::TSection::TSection(const TCtrlGroup::TCtrlSql& title, const string& str)
{
	try
	{
		set_fields(title, str);
	}
	catch (const Logger::TLog& e)
	{
		cerr << e.what() << endl;
		clear();
	}
};

void NS_Sql::TSection::add_field_to_data(const string& str, const string& ch, bool use_brkt) noexcept(false)
{
	if (str.empty()) return;
	if (TCtrlGroup::CanUseBrkt(name) != use_brkt)
	{
		string s = TCtrlGroup::CtrlSql2Str(name);
		throw Logger::TLog("¬ указанном блоке: ", s.c_str(),
			" запрещено использовать скобки!", nullptr);
	}
	if (!TCtrlGroup::CorrectDelimeter(name, ch))
	{
		string s = TCtrlGroup::CtrlSql2Str(name);
		throw Logger::TLog("”казан не верный разделитель: ", ch.c_str(), " дл€ блока: ",
			s.c_str(), nullptr);
	}
	string tmp = LowerCase(str);
	if (data.empty())
	{
		data = tmp;
		return;
	}
	if (use_brkt)
		data = TCtrlGroup::CtrlSym2Char(TCtrlGroup::TCtrlSym::lbkt) + data + TCtrlGroup::CtrlSym2Char(TCtrlGroup::TCtrlSym::rbkt);
	data += ch + TCtrlGroup::CtrlSym2Str(TCtrlGroup::TCtrlSym::Space) + tmp;
};

void NS_Sql::TSection::AddField(const string& str, const TCtrlGroup::TCtrlSql& ch, bool use_brkt)
{
	string delimeter = TCtrlGroup::CtrlSql2Str(ch);
	try
	{
		add_field_to_data(str, delimeter, use_brkt);
	}
	catch (const Logger::TLog& er)
	{
		std::cerr << er.what() << endl;
		return;
	}
};

string NS_Sql::TSection::to_Str(void) const
{
	return Empty() ? string() : Name() + TCtrlGroup::CtrlSym2Char(TCtrlGroup::TCtrlSym::Space) + Data();
};

NS_Sql::TSection& NS_Sql::TSection::operator=(const TSection& sect)
{
	if (this != &sect)
	{
		name = sect.name;
		data = sect.data;
	}
	return *this;
};

NS_Sql::TText::TSectIndex NS_Sql::TText::operator[](const TCtrlGroup::TCtrlSql& title)
{
	if (title == TCtrlGroup::TCtrlSql::Empty) return sect.end();
	return std::find_if(sect.begin(), sect.end(), TTrimObj(title));
};

void NS_Sql::TText::Init_Sectors(const string& str)
{
	if (str.empty()) return;
	for (TCtrlGroup::TCtrlSql title = TCtrlGroup::TCtrlSql::With; title <= TCtrlGroup::TCtrlSql::Group; title = title + 1)
	{
		TSection tmp(title, str);
		if (!tmp.Empty())
			sect.push_back(tmp);
	}
};

NS_Sql::TText::TText(const string& str)
{
	Init_Sectors(str);
};

NS_Sql::TText::TText(std::istream& stream)
{
	if (!stream) return;
	std::stringstream ss;
	while (stream)
	{
		string str;
		std::getline(stream, str);
		ss << str;
	}
	Init_Sectors(ss.str());
};

string NS_Sql::TText::toStr(void) const
{
	if (sect.empty()) return string();
	std::stringstream ss;
	char delimeter = TCtrlGroup::CtrlSym2Char(TCtrlGroup::TCtrlSym::Space);
	for (TConstSectIndex i = sect.begin(); i < sect.end(); i++)
	{
		ss << i->to_Str();
		if (i + 1 < sect.end())	ss << delimeter;
	}
	ss << TCtrlGroup::CtrlSql2Str(TCtrlGroup::TCtrlSql::EOC);
	return ss.str();
};

bool NS_Sql::TText::DelSection(const TCtrlGroup::TCtrlSql& title) noexcept(true)
{
	try
	{
		if (TCtrlGroup::MustFound(title))
		{
			string s = TCtrlGroup::CtrlSql2Str(title);
			throw Logger::TLog("Ѕлок: ", s.c_str(), " не подлежит удалению!", nullptr);
		}
		//ищем указанную секцию:
		TConstSectIndex i = operator[](title);
		if (i == sect.end())
		{
			string s = TCtrlGroup::CtrlSql2Str(title);
			throw Logger::TLog("Ѕлока: ", s.c_str(), " не найдено!", nullptr);
		}
		else
			sect.erase(i);
	}
	catch (const Logger::TLog& e)
	{
		std::cerr << e.what() << endl;
		return false;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << endl;
		return false;
	}
	catch (...)
	{
		std::cerr << "ѕроизошла необработанна€ ошибка при удалении блока: " <<
			TCtrlGroup::CtrlSql2Str(title) << endl;
		return false;
	}
	return true;
};

void NS_Sql::TText::AddSection(const TSection& val)
{
	if (val.Empty()) return;
	TSectIndex indx = operator[](val.Title());
	//если секции не найдено - добавл€ем новую:
	if (indx == sect.end())
	{
		std::set<TSection> tmp(sect.begin(), sect.end());
		tmp.insert(val);
		std::copy(tmp.begin(), tmp.end(), sect.begin());
	}
	else//если така€ секци€ есть:
		indx->Data(val.Data());
};

void NS_Sql::TText::AddSection(const TCtrlGroup::TCtrlSql& title, const string& str)
{
	TSection tmp(title, str);
	AddSection(tmp);
};


bool NS_Sql::TText::AddField2Section(const TCtrlGroup::TCtrlSql& title, const string& str,
	TCtrlGroup::TCtrlSql delimeter, bool brkt) noexcept(false)
{
	try
	{
		//провер€ем разделитель:
		if (delimeter == TCtrlGroup::TCtrlSql::Empty)
			delimeter = TCtrlGroup::GetDelimeterByTitle(title);
		TSectIndex indx = operator[](title);
		//если такой секции нет - выход
		if (indx == sect.end()) return false;
		indx->AddField(str, delimeter, brkt);
		return true;
	}
	catch (const Logger::TLog& er)
	{
		cerr << er.what() << endl;
		return false;
	}
};