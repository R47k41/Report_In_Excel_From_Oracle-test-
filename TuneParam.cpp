#include <iostream>
#include <vector>
#include <iterator>
#include <stdexcept>
#include "TuneParam.h"
#include "Logger.h"

using std::string;
using std::vector;
using std::map;
using std::size_t;
using std::streampos;

size_t NS_Tune::get_pos_in_str(const string& str, const TuneField& substr, const size_t beg, find_fnc ff)
{
	string tmp = TuneFieldToStr(substr);
	if (tmp.empty()) return string::npos;
	//поиск подстроки в строке
	size_t pos = (str.*ff)(tmp, beg);
	//если данная строка не найдена - выход
	if (pos != string::npos)	pos += tmp.size();
	return pos;
};

string NS_Tune::TuneFieldToStr(const TuneField& val)
{
	switch (val)
	{
	case TuneField::DataRange: return "\"";
	case TuneField::DataBase: return "[DATA BASE]";
	case TuneField::UserName: return "UserName";
	case TuneField::Password: return "Password";
	case TuneField::TNS: return "TNS";
	case TuneField::Report: return "[REPORT]";
	case TuneField::OutFile: return "OutFileName";
	case TuneField::SqlFile: return "SQLFile";
	case TuneField::SqlText: return "SQLText";
	case TuneField::Columns: return "[COLUMNS]";
	case TuneField::Column: return "Column";
	default: return string();
	}
};

bool NS_Tune::operator==(const TuneField& val, const string& str)
{
	if (TuneFieldToStr(val) == str) return true;
	return false;
};

string NS_Tune::Get_TuneFiel_Val_From_Str(const TuneField& val, const string& str)
{
	using std::size_t;
	//если строка или кодовое поле - пустые: выход
	if (str.empty() or val == TuneField::Empty) return string();
	//поиск подстроки кода параметра в строке
	size_t pos = get_pos_in_str(str, val, 0, &string::find);
	//если данная строка не найдена - выход
	if (pos == string::npos) return string();
	//ищем значение поля в строке по разделителю:
	pos = get_pos_in_str(str, TuneField::DataRange, pos+1, &string::find);
	if (pos == string::npos) return string();
	size_t pose = get_pos_in_str(str, TuneField::DataRange, str.size(), &string::rfind);
	if (pose == string::npos) return string();
	return str.substr(pos, pose - pos - 1);
};

void NS_Tune::TUserData::set_default_fields_val(void)
{
	using std::make_pair;
	for (TuneField i = TuneField::UserName; i < TuneField::Column; i += 1)
		fields.insert(make_pair(i, TuneFieldToStr(i)));
};

void NS_Tune::TUserData::Read_Col_Val(ifstream& file)
{
	using std::getline;
	while (file)
	{
		string str;
		getline(file, str);
		if (!std::isalpha(str[0])) continue;
		string col = Get_TuneFiel_Val_From_Str(TuneField::Column, str);
		if (!col.empty())
			cols.push_back(col);
		else//если это не колонка - выходим
			break;
	}
};

void NS_Tune::TUserData::Read_Tune_Val(ifstream& file)
{
	using std::getline;
	using std::streampos;
	streampos pos = file.tellg();
	file.seekg(pos);
	TuneField indx = TuneField::UserName;
	for (TuneField i = indx; i < TuneField::Column; i = i + 1)
	{
		while (file)
		{
			//считывание строки настройки
			string str;
			getline(file, str);
			if (!std::isalpha(str[0]))
			{
				//считываем данные по колонкам
				if (TuneField::Columns == str)
					Read_Col_Val(file);
				else
					continue;
			}
			//берем позицию первой строки с настройками:
			pos = file.tellg();
			//проверяем есть ли в данной строке запись о i-ой настройке
			str = Get_TuneFiel_Val_From_Str(i, str);
			if (!str.empty())
			{
				fields.insert(make_pair(i, str));
				indx += 1;
				break;
			}
		}
		//если настройки не найдено - ищем следующее поле настройки
		file.seekg(pos);
	}
};


void NS_Tune::TUserData::ReadFromFile(const string& filename)
{
	using std::ifstream;
	using std::ios_base;
	using std::make_pair;
	using std::getline;
	ifstream file(filename.c_str(), ios_base::in);
	if (!file.is_open())
		throw Logger::TLog("Ошибка открытия файла: ", filename.c_str(), nullptr);
	//считываем значения для настроек:
	Read_Tune_Val(file);
	//считываем данные для колонок:
	Read_Col_Val(file);
	//закрываем файл
	file.close();
};

NS_Tune::TUserData::TUserData(const string& filename)
{
	ReadFromFile(filename);
};

string NS_Tune::TUserData::getValue(const TuneField& code) const
{
	try
	{
		return operator[](code);
	}
	catch (const std::out_of_range& er)
	{
		std::cerr << er.what() << std::endl;
		return string();
	}
};

void NS_Tune::TUserData::operator()(const TuneField& code, const string& val)
{
	try
	{
		operator[](code) = val;
	}
	catch (const std::out_of_range& er)
	{
		std::cerr << er.what() << std::endl;
		return;
	}
};

void NS_Tune::TUserData::show_columns(void) const
{
	using std::cout;
	using std::endl;
	if (EmptyColumns()) return;
	for (string x : cols)
		cout << x << endl;
};

void NS_Tune::TUserData::show_tunes(void) const
{
	using std::cout;
	using std::endl;
	if (EmptyFields()) return;
	for (TField x : fields)
		cout << TuneFieldToStr(x.first) << " = " << "\"" << x.second << "\"" << endl;
};