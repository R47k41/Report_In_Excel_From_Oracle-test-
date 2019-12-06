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
	string UpperCase(const string& str);
	//убираем из строки служебные символа:
	void DeleteServiceSymb(string& str);
	//функция убирающая пробелы из начали и конца строки:
	void Trim_Left(string& str);
	void Trim_Right(string& str);
	void Trim(string& str);
	void raise_app_err(const TLog& log, bool as_raise = true);
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

void NS_Sql::raise_app_err(const TLog& log, bool as_raise)
{
	as_raise ? throw log : log.toErrBuff();
}

//преобразование в нижний регистр:
string NS_Sql::LowerCase(const string& str)
{
	string result;
	std::transform(str.begin(), str.end(), std::insert_iterator<std::string>(result, result.begin()), tolower);
	return result;
}

string NS_Sql::UpperCase(const string& str)
{
	string result;
	std::transform(str.begin(), str.end(), std::insert_iterator<std::string>(result, result.begin()), toupper);
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

string NS_Sql::AsString(const TText& sql)
{
	return sql.toStr();
}

string NS_Sql::AsString(const TSimpleSql& sql)
{
	return sql.Data();
}

void NS_Sql::TSimpleSql::Trim(const Side& flg)
{
	if (text.empty()) return;
	switch (flg)
	{
	case Side::Left:
		NS_Sql::Trim_Left(text);
		break;
	case Side::Right:
		NS_Sql::Trim_Right(text);
		break;
	case Side::Full:
		NS_Sql::Trim(text);
		break;
	}
}

string NS_Sql::TSimpleSql::toCase(const KeyCase& flg) const
{
	if (text.empty()) return string();
	switch (flg)
	{
	case KeyCase::Lower: return NS_Sql::LowerCase(text);
	case KeyCase::Upper: return NS_Sql::UpperCase(text);
	}
	return text;
}

bool NS_Sql::TSimpleSql::isCommentLine(const string& str) noexcept(true)
{
	using std::vector;
	using std::find;
	if (!str.empty())
	{
		//формируем массив из символов относяхщихся к коментариям:
		vector<string> arr = {TConstCtrlSym::asStr(CtrlSym::dies_comment),
			TConstCtrlSym::asStr(CtrlSym::minus_comment), TConstCtrlSym::asStr(CtrlSym::dash_comment) };
		if (find(arr.begin(), arr.end(), str.substr(0, 2)) == arr.end())
			return false;
		return true;
	}
	return false;
}

string NS_Sql::TSimpleSql::read_sql_file(std::istream& file, bool skip_coment) noexcept(false)
{
	using std::stringstream;
	using std::getline;
	stringstream ss;
	while (file)
	{
		string tmp;
		getline(file, tmp);
		if (!isCommentLine(tmp))
			ss << tmp;
	}
	return ss.str();
}

NS_Sql::TSimpleSql::TSimpleSql(std::istream& file)
{
	text = read_sql_file(file);
}

NS_Sql::TSimpleSql::TSimpleSql(const string& data, bool use_trim) : text(data)
{
	if (use_trim) Trim();
}

void NS_Sql::TSimpleSql::set_data(const string& sql, bool use_trim)
{
	text = sql;
	if (use_trim) Trim();
}

std::size_t NS_Sql::TSimpleSql::find_word_pos(const string& str, const string& key, const size_t n, bool must_find)
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
			if (must_find)
				raise_app_err(TLog("Обязательный элемент: " + key + " не найден в выражении: " + str, 
					"TSection::find_word_pos"), must_find);
			break;
		}
		if (ctrl.IsCorrectSym(str, pos, key.size())) break;
		else pos += key.size() + n;
	};
	return pos;
};

bool NS_Sql::TSimpleSql::hasParams() const
{
	if (text.empty()) return false;
	try
	{
		char ch = NS_Const::TConstCtrlSym::asChr(NS_Const::CtrlSym::colon);
		size_t pos = text.rfind(ch);
		if (pos == string::npos) return false;
		if (pos == 0 and text.size() > pos and std::isalnum(text[pos + 1]))
			return true;
		if (pos > 1 && pos <= text.size() - 1 && std::isalnum(text[pos - 1]) && std::isalnum(text[pos + 1]))
			return true;
	}
	catch (...)
	{
		NS_Logger::TLog("Ошибка определения наличия параметров в строке: " + text, "TSimpleSql::hasParams").toErrBuff();
	}
	return false;
}

string NS_Sql::TSection::get_data_by_key_range(const string& str, size_t& pos,const TConstSql& br,
		const TConstSql& er) noexcept(false)
{
	using std::size_t;
	string in_str = LowerCase(str);
	//если строка данных - пуста или не заполнено имя секции - выход
	if (in_str.empty() || br.isEmpty()	|| er.isEmpty()) return string();
	pair<string, string> range(br.toStr(), er.toStr());
	//ищем началюную позицию
	size_t posb = TSimpleSql::find_word_pos(in_str, range.first, pos, br.MustFound());
	if (posb == string::npos)
		return string();
	posb += range.first.size();
	size_t pose = TSimpleSql::find_word_pos(in_str, range.second, posb, er.MustFound());
	if (pose == string::npos)
	{
		range.second = er.getClosedElem();
		if (!range.second.empty())
		{
			TConstSql end_of_comand(TSql::EOC);
			pose = TSimpleSql::find_word_pos(in_str, range.second, posb, end_of_comand.MustFound());
			if (pose == string::npos)
				throw TLog("Отсутствует символ окончания выражения " + range.second, "TSection::get_data_by_key_range");
		}
		else
			pose = str.size();
	}
	//формируем поле данных:
	in_str = in_str.substr(posb + 1, pose - posb - 1);
	pos = pose;
	return in_str;
}

void NS_Sql::TSection::set_fields() noexcept(true)
{
	try
	{
		if (TSimpleSql::Empty()) return;
		size_t pos = 0;
		TConstSql open_val(name);
		TConstSql close_val = open_val + 1;
		string tmp = get_data_by_key_range(TSimpleSql::toCase(TSimpleSql::KeyCase::Lower), pos, open_val, close_val);
		if (!tmp.empty())
			TSimpleSql::Data(tmp);
		else
			clear();
	}
	catch (const string& er)
	{
		raise_app_err(TLog("Ошибка инициализации объекта TSection : " + er, "NS_Sql::TSection::set_fields"), false);
		clear();
	}
	catch (...)
	{
		raise_app_err(TLog("Необработанная ошибка при инициализации объекта TSection", "NS_Sql::TSection::set_fields"), false);
		clear();
	}
};

NS_Sql::TSection::TSection(const TSql& title, const TSimpleSql& sql): TSimpleSql(sql), name(title)
{
	set_fields();
}

NS_Sql::TSection::TSection(const TSql& title, const string& str): TSimpleSql(str), name(title)
{
	set_fields();
};

NS_Sql::TSection::TSection(const TConstSql& title, const string& str) : TSimpleSql(str), name(title)
{
	set_fields();
};

void NS_Sql::TSection::add_field_to_data(const string& str, const string& ch, bool use_brkt) noexcept(false)
{
	if (str.empty()) return;
	if (name.CanUseBrkt() != use_brkt)
		throw TLog("В указанном блоке: " + name.toStr() + " запрещено использовать скобки!", "TSection::add_field_to_data");
	if (!name.CorrectDelimeter(ch))
		throw TLog("Указан не верный разделитель: " + ch + " для блока: " +	name.toStr(), "TSection::add_field_to_data");
	string tmp = LowerCase(str);
	if (TSimpleSql::Empty())
	{
		TSimpleSql::Data(tmp);
		return;
	}
	if (use_brkt)
	{
		std::stringstream ss;
		ss << TConstCtrlSym(CtrlSym::lbkt).toStr() << TSimpleSql::Data() << TConstCtrlSym(CtrlSym::rbkt).toStr();
		TSimpleSql::Data(ss.str());
	}
	TSimpleSql::Data(ch + TConstCtrlSym(CtrlSym::Space) + tmp);
};

void NS_Sql::TSection::AddField(const string& str, const TConstSql& ch, bool use_brkt)
{
	try
	{
		add_field_to_data(str, ch.toStr(), use_brkt);
	}
	catch (const TLog& er)
	{
		throw er;
	}
	catch (...)
	{
		throw TLog("Не обработанная ошибка!", "NS_Sql::TSection::AddField");
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
		TSimpleSql::Data(sect.Data());
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
	while (pos < section.Data().size())
	{
		string tmp = TSection::get_data_by_key_range(section.Data(), pos, TConstSql(TSql::As), TConstSql(TSql::D4L));
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
	Init_Sectors(TSimpleSql::read_sql_file(stream));
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
	TLog log("Ошибка удаления секции " + tmp.toStr(), "TText::DelSection");
	try
	{
		if (tmp.MustFound())
		{
			log.toErrBuff();
			return false;
		}
		//ищем указанную секцию:
		TConstSectIndex i = operator[](tmp);
		if (i == sect.end())
		{
			log.toErrBuff();
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
		TLog("Необработанная ошибка при удалении секции: " + tmp.toStr() + TLog::NL, "TText::DelSection").toErrBuff();
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
	catch (const string& er)
	{
		TLog log("Ошибка добавления поля: " + str + " в секцию: " + tmp_title.toStr() + TLog::NL, "NS_Sql::TText::AddField2Section");
		log << er << TLog::NL;
		log.toErrBuff();
		return false;
	}
};