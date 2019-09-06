#include <fstream>
#include <ios>
#include "TuneParam.h"
#include "Logger.h"

//преобразование колонки в строку
std::string NS_Tune::TColumn::toString(const string& d) const
{
	using Constant_Data::Space;
	string result;
	result += getTitle();
	if (!d.empty())
		result += Space + d;
	if (!name.empty())
		result += Space + getName();
	return result;
}

//инициализация блока строки с помощью интервала данных:
void NS_Tune::TRow::InitRow(const std::string::iterator& first, const std::string::iterator& seconsd)
{
	using NS_Tune::TColumn;
	if (first >= seconsd) return;
	//ищем в блоке ключевое слово:
	string::iterator i = first + 1;
	while (i != second)
	{
		string tmp;
		if (i + 1 != range.second)
			tmp = *(i + 1);
		if (tmp == Constant_Data::SQL_OPERATOR::As)
		{
			if (i + 2 != range.second)
				result.AddCol(TColumn(*i, *(i + 2)));
			else
				result.AddCol(TColumn(*i, tmp));
			i += 3;
			continue;
		}
		result.AddCol(TColumn(*i, tmp));
		i++;
	}
}

//преобразование Строки таблицы в строку
std::string NS_Tune::TRow::toString(const std::string& row_delimeter, const std::string& col_delimeter) const
{
	string result;
	for (vector<TColumn>::const_iterator i = row.begin(); i != row.end(); i++)
	{
		result += i->toString(col_delimeter);
		if (i + 1 != row.end())
			result += row_delimeter;
	}
	return result;
}

//получение интервала по условию
String_Fnc::TInterval String_Fnc::getInterval(const String_Fnc::TStringsIterator& ibeg, const String_Fnc::TStringsIterator& iend, const std::string& op_beg, const std::string& op_end)
{
	TInterval result = std::make_pair(iend, iend);
	if (ibeg == iend) return result;
	result.first = find(ibeg, iend, op_beg);
	result.second = find(result.first, iend, op_end);
	if (result.second == iend)
		result.second = find(result.first, iend, Constant_Data::SQL_OPERATOR::EndCommand);
	return result;
}

//функция заполнения блока/сектора запроса:
NS_Tune::TSection String_Fnc::fill_sec(const String_Fnc::TInterval& range)
{
	using NS_Tune::TColumn;
	NS_Tune::TSection result;
	if (range.first == range.second) return result;
	//ищем в блоке ключевое слово:
	TStrings::iterator i = range.first + 1;
	while (i != range.second)
	{
		string tmp;
		if (i + 1 != range.second)
			tmp = *(i + 1);
		if (tmp == Constant_Data::SQL_OPERATOR::As)
		{
			if (i + 2 != range.second)
				result.AddCol(TColumn(*i, *(i + 2)));
			else
				result.AddCol(TColumn(*i, tmp));
			i += 3;
			continue;
		}
		result.AddCol(TColumn(*i, tmp));
		i++;
	}
	return result;
}

//фукнция заполнения строки:
std::string String_Fnc::fill_str(const String_Fnc::TInterval& range)
{
	string result;
	if (range.first == range.second) return result;
	for (TStringsConstIterator i = range.first; i != range.second; i++)
	{
		result += *(i);
		if (i + 1 != range.second)
			result += Constant_Data::Space;
	}
	return result;
}

//функция инициализации sql-запроса строкой:
void NS_Tune::SQL_Text::init_by_str(const string& str)
{
	using String_Fnc::TStrings;
	using String_Fnc::splitStr;
	using String_Fnc::TInterval;
	using String_Fnc::getInterval;
	using String_Fnc::fill_sec;
	using String_Fnc::fill_str;
	if (str.empty()) return;
	string tmp;
	//преобразуем к нижнему регистру:
	std::transform(str.begin(), str.end(), std::insert_iterator<string>(tmp, tmp.begin()), std::tolower);
	TStrings arr = splitStr(tmp);
	//задаем интервал:
	TInterval range;
	//обработка секции select-from - получение колонок
	range = getInterval(arr.begin(), arr.end(), Constant_Data::SQL_OPERATOR::Select, Constant_Data::SQL_OPERATOR::From);
	slct_sec = fill_sec(range);
	//обработка секции from-where - получение таблиц
	range = getInterval(range.second, arr.end(), Constant_Data::SQL_OPERATOR::From, Constant_Data::SQL_OPERATOR::Where);
	frm_sec = fill_sec(range);
	//обработка секции where-order by
	range = getInterval(range.second, arr.end(), Constant_Data::SQL_OPERATOR::Where, Constant_Data::SQL_OPERATOR::Order);
	whr_sec = fill_str(range);
	//обработка секции order by
	range = getInterval(range.second, arr.end(), Constant_Data::SQL_OPERATOR::Order, Constant_Data::SQL_OPERATOR::EndCommand);
	ord_sec = fill_str(range);
}

//функция разбиения строки на подстроки по служебным символам
String_Fnc::TStrings String_Fnc::splitStr(const std::string& str, const std::string& delimet)
{
	TStrings result;
	//если строка пустая
	if (str.empty()) return result;
	//если нет разделителей
	if (delimet.empty())
	{
		result.push_back(str);
		return result;
	}
	//разбиваем строку на подстроки используюя разделитель
	string tmp;
	for (char ch : str)
	{
		//если символ не относится к служебным:
		if (delimet.find(ch) == string::npos)
			tmp.push_back(ch);
		else
		{
			result.push_back(tmp);
			tmp.clear();
		}
	}
	return result;
}

//инициализация объекта SQL_Text:
NS_Tune::SQL_Text::SQL_Text(const std::string& str)
{
	init_by_str(str);
}

//добавление условия для запроса:
void NS_Tune::SQL_Text::AddWhereCond(const std::string& val, const std::string& opr)
{
	if (val.empty()) return;
	if (opr.empty() and whr_sec.empty())
		whr_sec += val;
	else
	{
		if (opr != Constant_Data::SQL_OPERATOR::And and opr != Constant_Data::SQL_OPERATOR::Or
			and val.empty()) return;
		whr_sec += opr;
		whr_sec += Constant_Data::Space;
		whr_sec += val;
	}
}

//добавление условия для сортировки
void NS_Tune::SQL_Text::AddOrderVal(const std::string& val)
{
	if (val.empty()) return;
	if (!ord_sec.empty())
		ord_sec += Constant_Data::SQL_OPERATOR::ColumnDelim + Constant_Data::Space;
	ord_sec += val;
}

//представление sql-запроса в виде строки
std::string NS_Tune::SQL_Text::ToString() const
{
	using std::copy;
	using Constant_Data::Space;
	using Constant_Data::SQL_OPERATOR::ColumnDelim;
	if (slct_sec.isEmpty()) throw Logger::TLog("Запрос пуст!");
	if (frm_sec.isEmpty()) throw Logger::TLog("Нет секции from!");
	string result;
	result += Constant_Data::SQL_OPERATOR::Select + Space;
	result += slct_sec.toString(ColumnDelim, Constant_Data::SQL_OPERATOR::As);
	result += Space + Constant_Data::SQL_OPERATOR::From + Space;
	result += frm_sec.toString(Constant_Data::SQL_OPERATOR::ColumnDelim, string{ Space });
	if (!whr_sec.empty())
	{
		result += Space + Constant_Data::SQL_OPERATOR::Where + Space;
		copy(whr_sec.begin(), whr_sec.end(), result.end());
	}
	if (!ord_sec.empty())
	{
		result += Space + Constant_Data::SQL_OPERATOR::Order + Space;
		copy(ord_sec.begin(), ord_sec.end(), result.end());
	}
	/**/
	return result;
}

void NS_Tune::Tune::read_tune_file(const string& file_name) noexcept(false)
{
	using std::ifstream;
	using std::ios_base;
	using Logger::TLog;
	if (file_name.empty()) return;
	ifstream file(file_name.c_str(), ios_base::in);
	if (file.is_open())
		throw TLog("Ошибка открытия файла " , file_name.c_str());
	//если файл открыт:
	while (file)
	{
		
	}
};