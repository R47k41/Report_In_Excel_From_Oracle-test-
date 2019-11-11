#include <sstream>
#include <iterator>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <set>
#include "TSQLParser.h"
#include "Logger.h"

using std::string;
using std::vector;
using std::pair;
using std::cout;
using std::endl;
using std::cerr;
using NS_Logger::TLog;

namespace NS_Sql
{
	//преобразование в нижний регистр:
	string LowerCase(const string& str);
	//убираем из строки служебные символа:
	void DeleteServiceSymb(string& str);
	//функция убирающая пробелы из начали и конца строки:
	void Trim_Left(string& str);
	void Trim_Right(string& str);
	void Trim(string& str);
	
	//класс для сравнения со строками в качестве предиката:
	class TTrimObj
	{
	private:
		string symb;
	public:
		TTrimObj(const string& arr) : symb(arr) {};
		TTrimObj(const TConstSql& title);
		//манипуляция с символами:
		bool operator()(const char& ch) const;
		//операция сравнения секторов по имени:
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

NS_Sql::TTrimObj::TTrimObj(const TConstSql& title)
{
	symb = title.toStr();
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
	string syms = { TConstCtrlSym(CtrlSym::Space).toChar(), TConstCtrlSym(CtrlSym::NL).toChar(),
		TConstCtrlSym(CtrlSym::Tab).toChar() };;
	//std::transform(str.begin(), str.end(), str.begin(), TTrimObj(syms));
	std::copy_if(str.begin(), str.end(), std::insert_iterator<string>(str, str.begin()), TTrimObj(syms));
};

void NS_Sql::Trim_Left(string& str)
{
	string syms = { TConstCtrlSym(CtrlSym::Space).toChar(), TConstCtrlSym(CtrlSym::NL).toChar(),
		TConstCtrlSym(CtrlSym::Tab).toChar() };
	string::const_iterator b = std::find_if_not(str.begin(), str.end(), TTrimObj(syms));
	if (b >= str.begin()) str.erase(str.begin(), b);
};

void NS_Sql::Trim_Right(string& str)
{
	string syms = { TConstCtrlSym(CtrlSym::Space).toChar(), TConstCtrlSym(CtrlSym::NL).toChar(), 
		TConstCtrlSym(CtrlSym::Tab).toChar() };
	string::const_reverse_iterator b = std::find_if_not(str.rbegin(), str.rend(), TTrimObj(syms));
	if (b != str.rbegin()) str.erase(b.base(), str.end());
};

void NS_Sql::Trim(string& str)
{
	Trim_Left(str);
	Trim_Right(str);
}

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
	//проверяем является ли найденная позиция верной:
	TSymGroup ctrl(true);
	while (pos <= str.size())
	{
		pos = str.find(key, pos);
		if (pos == string::npos)
		{
			TLog("Обязательный элемент: " + key + " не найден в выражении: " + str).raise(must_find, "TSection::find_word_pos");
			break;
		}
		if (ctrl.IsCorrectSym(str, pos, key.size())) break;
		else pos += key.size() + n;
	};
	return pos;
};

string NS_Sql::TSection::get_data_by_key_range(const string& str, size_t& pos,const TConstSql& br,
		const TConstSql& er) noexcept(false)
{
	using std::size_t;
	string in_str = LowerCase(str);
	//если строка данных - пуста или не заполнено имя секции - выход
	if (in_str.empty() || br.isEmpty()	|| er.isEmpty()) return string();
	pair<string, string> range(br.toStr(), er.toStr());
	//ищем началюную позицию
	size_t posb = find_word_pos(in_str, range.first, pos, br.MustFound());
	if (posb == string::npos)
		return string();
	posb += range.first.size();
	size_t pose = find_word_pos(in_str, range.second, posb, er.MustFound());
	if (pose == string::npos)
	{
		range.second = er.getClosedElem();
		if (!range.second.empty())
		{
			TConstSql end_of_comand(TSql::EOC);
			pose = find_word_pos(in_str, range.second, posb, end_of_comand.MustFound());
			if (pose == string::npos)
				TLog("Отсутствует символ окончания выражения " + range.second).raise(true, "TSection::get_data_by_key_range");
		}
		else
			pose = str.size();
	}
	//формируем поле данных:
	in_str = in_str.substr(posb + 1, pose - posb - 1);
	pos = pose;
	return in_str;

}

void NS_Sql::TSection::set_fields(const string& str) noexcept(true)
{
	try
	{
		if (str.empty()) return;
		size_t pos = 0;
		TConstSql open_val(name);
		TConstSql close_val = open_val + 1;
		string tmp = get_data_by_key_range(str, pos, open_val, close_val);
		if (!tmp.empty())
			set_data(tmp);
	}
	catch (const TLog& er)
	{
		cerr << "Ошибка инициализации объекта TSection: " << er.what() << endl;
		clear();
	}
	catch (...)
	{
		cerr << "Необработанная ошибка при инициализации объекта TSection" << endl;
		clear();
	}
};

void NS_Sql::TSection::clear(void)
{
	name = TSql::Empty;
	data.clear();
};

NS_Sql::TSection::TSection(const TSql& title, const string& str): name(title)
{
	if (!str.empty())	set_fields(str);
};

NS_Sql::TSection::TSection(const TConstSql& title, const string& str) : name(title)
{
	if (!str.empty())	set_fields(str);
};

void NS_Sql::TSection::add_field_to_data(const string& str, const string& ch, bool use_brkt) noexcept(false)
{
	if (str.empty()) return;
	if (name.CanUseBrkt() != use_brkt)
		TLog("В указанном блоке: " + name.toStr() + " запрещено использовать скобки!").raise(true, "TSection::add_field_to_data");
	if (!name.CorrectDelimeter(ch))
		TLog("Указан не верный разделитель: " + ch + " для блока: " +	name.toStr()).raise(true, "TSection::add_field_to_data");
	string tmp = LowerCase(str);
	if (data.empty())
	{
		data = tmp;
		return;
	}
	if (use_brkt)
	{
		std::stringstream ss;
		ss << TConstCtrlSym(CtrlSym::lbkt).toStr() << data << TConstCtrlSym(CtrlSym::rbkt).toStr();
		data = ss.str();
	}
	data = ch + TConstCtrlSym(CtrlSym::Space) + tmp;
};

void NS_Sql::TSection::AddField(const string& str, const TConstSql& ch, bool use_brkt)
{
	try
	{
		add_field_to_data(str, ch.toStr(), use_brkt);
	}
	catch (const TLog& er)
	{
		std::cerr << er.what() << endl;
		return;
	}
	catch (...)
	{
		std::cerr << "Не обработанная ошибка!" << endl;
		return;
	}
};

string NS_Sql::TSection::to_Str(void) const
{
	return Empty() ? string() : Name() + TConstCtrlSym(CtrlSym::Space) + Data();
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

NS_Sql::TText::TSectIndex NS_Sql::TText::operator[](const TConstSql& title)
{
	if (title.isEmpty()) return sect.end();
	return std::find_if(sect.begin(), sect.end(), TTrimObj(title));
};

NS_Sql::TSection NS_Sql::TText::operator[](const TConstSql& title) const
{
	for (TSection s : sect)
		if (s.Title() == title) return s;
	return TSection(TSql::Empty);
}

vector<string> NS_Sql::getColumnValue(const TSection& section)
{
	vector<string> result;
	if (section.name != TSql::Select) return result;
	size_t pos = 0;
	while (pos < section.data.size())
	{
		string tmp = TSection::get_data_by_key_range(section.data, pos, TConstSql(TSql::As), TConstSql(TSql::D4L));
		if (!tmp.empty())
			result.push_back(tmp);
	}
	return result;
}

void NS_Sql::TText::Init_Sectors(const string& str)
{
	if (str.empty()) return;
	TConstSql title(TSql::With);
	while (title <= TSql::Group)
	{
		TSection tmp(title, str);
		if (!tmp.Empty())	sect.push_back(tmp);
		title.Next();
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

string NS_Sql::TText::toStr(bool use_eoc) const
{
	if (sect.empty()) return string();
	std::stringstream ss;
	char delimeter = TConstCtrlSym(CtrlSym::Space).toChar();
	for (TConstSectIndex i = sect.begin(); i < sect.end(); i++)
	{
		ss << i->to_Str();
		if (i + 1 < sect.end())	ss << delimeter;
	}
	if (use_eoc)
		ss << TConstSql(TSql::EOC);
	return ss.str();
};

bool NS_Sql::TText::DelSection(const TSql& title) noexcept(true)
{
	TConstSql tmp(title);
	TLog log("Ошибка удаления секции " + tmp.toStr());
	try
	{
		if (tmp.MustFound())
		{
			//log += "Блок: " + tmp.toStr() + " не подлежит удалению!";
			log.raise(false, "TText::DelSection");
			return false;
		}
		//ищем указанную секцию:
		TConstSectIndex i = operator[](tmp);
		if (i == sect.end())
		{
			//log << "Блока: " << tmp.toStr() << " не найдено!";
			log.raise(false, "TText::DelSection");
			return false;
		}
		else
			sect.erase(i);
	}
	catch (const std::exception& e)
	{
		std::cerr << log.what() << e.what() << endl;
		return false;
	}
	catch (...)
	{
		std::cerr << "Необработанная ошибка при удалении секции: " << tmp.toStr() << endl;
		return false;
	}
	return true;
};

void NS_Sql::TText::AddSection(const TSection& val)
{
	if (val.Empty()) return;
	TSectIndex indx = operator[](val.Title());
	//если секции не найдено - добавляем новую:
	if (indx == sect.end())
	{
		std::set<TSection> tmp(sect.begin(), sect.end());
		tmp.insert(val);
		std::copy(tmp.begin(), tmp.end(), sect.begin());
	}
	else//если такая секция есть:
		indx->Data(val.Data());
};

void NS_Sql::TText::AddSection(const TSql& title, const string& str)
{
	TSection tmp(title, str);
	AddSection(tmp);
};


bool NS_Sql::TText::AddField2Section(const TSql& title, const string& str,
	const TSql& delimeter, bool brkt) noexcept(false)
{
	TConstSql tmp_title(title);
	try
	{
		TConstSql tmp_delimeter(delimeter);
		//проверяем разделитель:
		if (tmp_delimeter.isEmpty())
			tmp_delimeter = tmp_title.GetDelimeterAsObj();
		TSectIndex indx = operator[](tmp_title);
		//если такой секции нет - выход
		if (indx == sect.end()) return false;
		indx->AddField(str, tmp_delimeter, brkt);
		return true;
	}
	catch (const TLog& er)
	{
		cerr << "Ошибка добавления поля: " << str << " в секцию: " << tmp_title.toStr() << endl;
		cerr << er.what() << endl;
		return false;
	}
};