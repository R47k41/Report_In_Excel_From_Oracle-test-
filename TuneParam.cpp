#include <iostream>
#include <fstream>
#include <vector>
#include "TuneParam.h"
#include "Logger.h"

using std::string;
using std::vector;
using std::map;
using std::size_t;

size_t NS_Tune::get_pos_in_str(const string& str, const TuneField& substr, const size_t beg, find_fnc ff)
{
	string tmp = TuneFieldToStr(substr);
	if (tmp.empty()) return string::npos;
	//поиск подстроки в строке
	size_t pos = (str.*ff)(tmp, beg);
	//если данная строка не найдена - выход
	if (pos != string::npos)	pos += tmp.size() + 1;
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
	pos = get_pos_in_str(str, TuneField::DataRange, pos, &string::find);
	if (pos == string::npos) return string();
	size_t pose = get_pos_in_str(str, TuneField::DataRange, pos, &string::rfind);
	if (pose == string::npos) return string();
	return str.substr(pos, pose - pos + 1);
};

void NS_Tune::TUserData::set_default_fields_val(void)
{
	using std::make_pair;
	for (TuneField i = TuneField::UserName; i < TuneField::Column; i += 1)
		fields.insert(make_pair(i, TuneFieldToStr(i)));
};

void NS_Tune::TUserData::ReadFromFile(const string& filename)
{
	using std::ifstream;
	using std::ios_base;
	using std::getline;
	ifstream file(filename.c_str(), ios_base::in);
	if (!file.is_open())
		throw Logger::TLog("Ошибка открытия файла: ", filename.c_str(), nullptr);
	//ставим значения по умолчанию
	set_default_fields_val();
	//ставим значения из файла настроек
	while (file)
	{
		string tmp;
		getline(file, tmp);

	}
};