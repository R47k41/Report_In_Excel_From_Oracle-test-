#include <iostream>
#include <iterator>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include "TuneParam.h"
#include "TConverter.h"
#include "TConverter.cpp"
#include "Logger.h"

using std::string;
using std::vector;
using std::set;
using std::size_t;
using std::streampos;

size_t NS_Tune::TBaseParam::get_pos_in_src(const string& substr, const size_t beg, find_fnc ff) const
{
	if (substr.empty()) return string::npos;
	//поиск подстроки в строке
	size_t pos = (src_data.*ff)(substr, beg);
	//если данная строка не найдена - выход
	if (pos != string::npos)	pos += substr.size();
	return pos;
}

size_t NS_Tune::TBaseParam::get_pos_in_src(const TConstField& substr, const size_t beg, find_fnc ff) const
{
	return get_pos_in_src(substr.toStr(), beg, ff);
};

size_t NS_Tune::TBaseParam::get_pos_in_src(const CtrlSym& substr, const size_t beg, find_fnc ff) const
{
	TConstCtrlSym tmp(substr);
	return get_pos_in_src(tmp.toStr(), beg, ff);
}

string NS_Tune::TBaseParam::Get_TuneField_Val(const TConstField& param, const CtrlSym& b_delimeter, const CtrlSym& e_delimeter) const
{
	using std::size_t;
	//если строка или кодовое поле - пустые: выход
	if (TBaseParam::isEmpty() or param.isEmpty()) return string();
	//поиск подстроки кода параметра в строке
	size_t pos = 0;
	pos = get_pos_in_src(param, pos);
	//если данная строка не найдена - выход
	if (pos == string::npos) return string();
	//ищем значение поля в строке по разделителю:
	pos = get_pos_in_src(b_delimeter, pos);
	if (pos == string::npos) return string();
	size_t pose = get_pos_in_src(e_delimeter, pos);
	if (pose == string::npos) return string();
	return srcSubStr(pos, pose-1);
};

string NS_Tune::TStringParam::Get_TuneField_Val(const TConstField& param, const CtrlSym& b_delimeter, const CtrlSym& e_delimeter) const
{
	using std::size_t;
	//если строка или кодовое поле - пустые: выход
	if (TBaseParam::isEmpty() or param.isEmpty()) return string();
	//поиск подстроки кода параметра в строке
	size_t pos = 0;
	pos = get_pos_in_src(param, pos);
	//если данная строка не найдена - выход
	if (pos == string::npos) return string();
	//ищем значение поля в строке по разделителю:
	pos = get_pos_in_src(b_delimeter, pos);
	if (pos == string::npos) return string();
	size_t pose = get_pos_in_src(e_delimeter, srcSize(), &string::rfind);
	if (pose == string::npos) return string();
	return srcSubStr(pos, pose - 1);
};

void NS_Tune::TStringParam::setValue(const TConstField&, const CtrlSym& open_val, const CtrlSym& close_val)
{
	value = Get_TuneField_Val(param, open_val, close_val);
}

NS_Tune::TStringParam::TStringParam(const string& full_data, const TuneField& tune_field,
	const CtrlSym& open_val, const CtrlSym& close_val) : TBaseParam(full_data), param(tune_field)
{
	if (!TBaseParam::isEmpty() and !param.isEmpty())
		setValue(param, open_val, close_val);
}

string NS_Tune::TStringParam::toStr(bool use_quotes) const
{
	//возвращаем строку вида: ParamName="ParamValue"
	string result;
	TConstCtrlSym tag(CtrlSym::quane);
	result = param.toStr() + tag.toStr();
	if (use_quotes)
	{
		tag = CtrlSym::quotes;
		result += tag.toStr() + value + tag.toStr();
	}
	else
		result += value;
	return result;
}

void NS_Tune::TSubParam::setValue(void)
{
	using NS_Converter::toType;
	TConstField tmp_field(TuneField::SqlParamQuane);
	string tmpID = Get_TuneField_Val(tmp_field, CtrlSym::quane, CtrlSym::semicolon);
	if (!toType(tmpID, &id, false)) id = EmptyID;
	tmp_field = TuneField::SqlParamType;
	string tmpType = Get_TuneField_Val(tmp_field, CtrlSym::quane, CtrlSym::semicolon);
	type = TConstType(tmpType);
	tmp_field = TuneField::SqlParamNote;
	comment = Get_TuneField_Val(tmp_field, CtrlSym::quane, CtrlSym::semicolon);
	tmp_field = TuneField::SqlParamValue;
	value = Get_TuneField_Val(tmp_field, CtrlSym::quane, CtrlSym::semicolon);
	if (value.empty()) setValByUser();
}

void NS_Tune::TSubParam::setValue(const TConstField&, const CtrlSym&, const CtrlSym&)
{
	setValue();
}

void NS_Tune::TSubParam::show() const
{
	using std::cout;
	using std::endl;
	if (isEmpty()) return;
	TConstField field(TuneField::SqlParamQuane);
	const TConstCtrlSym tag(CtrlSym::quane);
	cout << field.toStr() << tag.toStr() << id << endl;
	field = TuneField::SqlParamType;
	cout << field.toStr() << tag.toStr() << type.toStr() << endl;
	field = TuneField::SqlParamNote;
	cout << field.toStr() << tag.toStr() << comment << endl;
	field = TuneField::SqlParamValue;
	cout << field.toStr() << tag.toStr() << Value() << endl;
}

NS_Tune::TSubParam::TSubParam(const string& str): TBaseParam(str), id(EmptyID),
	type(DataType::ErrorType), comment()
{
	if (!TBaseParam::isEmpty())
		setValue();
}

string NS_Tune::TSubParam::getCode() const
{
	using std::stringstream;
	if (id == EmptyID) return string();
	stringstream ss;
	ss << TConstCtrlSym(CtrlSym::colon).toStr() << id;
	return ss.str();
}

bool NS_Tune::TSubParam::setValByUser()
{
	using std::cin;
	using std::cout;
	using std::endl;
	using std::cerr;
	try
	{
		cout << "Укажите параметр " << comment << ": ";
		std::getline(cin, value);
	}
	catch (...)
	{
		cerr << "Ошибка при указанни параметра " << comment << endl;
		return false;
	}
	return true;
}

void NS_Tune::TUserData::Read_StreamData_Val(ifstream& file, const TuneField& stream_title, StrArr& str_arr)
{
	using std::getline;
	TConstField block_end(TuneField::Block_End);
	while (file)
	{
		string str;
		getline(file, str);
		if (block_end == str) break;
		if (!std::isalpha(str[0])) continue;
		TStringParam col(str, stream_title);
		if (!col.isEmpty())
			str_arr.push_back(col.Value());
		else//если это не колонка - выходим
			break;
	}
}

void NS_Tune::TUserData::Read_Param_Val(ifstream& file)
{
	StrArr tmp;
	Read_StreamData_Val(file, TuneField::SqlParam, tmp);
	for (string s: tmp)
		if (!s.empty())
		{
			TSubParam sp(s);
			if (!sp.isEmpty()) params.insert(sp);
		}
}

void NS_Tune::TUserData::Read_Col_Val(ifstream& file)
{
	Read_StreamData_Val(file, TuneField::Column, cols);
};

void NS_Tune::TUserData::Read_Tune_Val(ifstream& file)
{
	using std::getline;
	using std::streampos;
	const TConstField Columns(TuneField::Columns);
	const TConstField Params(TuneField::SqlParams);
	std::set<TConstField> field_arr;
	//формирование массива искомых настроек
	for (TConstField i(TuneField::UserName); i < TuneField::Last; i.Next())
		field_arr.insert(i);
	//чтение файла:
	while (file)
	{
		//считывание строки настройки
		string str;
		getline(file, str);
		if (!std::isalpha(str[0]))
		{
			//считываем данные по колонкам
			if (Columns == str)
				Read_Col_Val(file);
			if (Params == str)
				Read_Param_Val(file);
			continue;
		}
		//сформировать массив из set<TConstField> от UserName до Last
		//проверять вхождение значения из массива в строку
		//если входит - удаляем значение из массива и записываем в список настроек
		//проверяем есть ли в данной строке нужный параметр:
		for (const TConstField& i: field_arr)
			//если параметр содержится в строке:
			if (i.StrInclude(str))
			{
				//записываем параметр в массив:
				TStringParam tmp_field(str, i.Value());
				if (!tmp_field.isEmpty()) fields.insert(tmp_field);
				//удаляем параметр из искомых:
				field_arr.erase(i);
				break;
			}
	}
};

void NS_Tune::TUserData::ReadFromFile(const string& filename)
{
	using std::ifstream;
	using std::ios_base;
	using std::sort;
	using std::getline;
	ifstream file(filename.c_str(), ios_base::in);
	if (!file.is_open())
		NS_Logger::TLog("Ошибка открытия файла: " + filename).raise(true, "NS_Tune::TUserData::ReadFromFile");
	//считываем значения для настроек:
	Read_Tune_Val(file);
	//считываение данных для колонок:
	//Read_Param_Val(file);
	//считываем данные для колонок:
	//Read_Col_Val(file);
	//закрываем файл
	file.close();
};

NS_Tune::TUserData::TUserData(const string& filename)
{
	ReadFromFile(filename);
};

template <typename KeyType, typename ValType>
const ValType& NS_Tune::TUserData::getValueByID(const KeyType& par_ID, const set<ValType>& arr) const noexcept(false)
{
	using NS_Logger::TLog;
	if (arr.empty()) TLog("Массив значений пуст!").raise(true, "NS_Tune::TUserData::getValueByID");
	for (const ValType& s: arr)
	{
		if (s > par_ID) break;
		if (s == par_ID) return s;
	}
	TLog("Данные не найдены!").raise(true, "NS_Tune::TUserData::getValueByID");
}

string NS_Tune::TUserData::getFieldByCode(const TuneField& code, bool exit_on_er) const noexcept(false)
{
	try
	{
		TStringParam tmp = getValueByID(code, fields);
		if (!tmp.isEmpty()) return tmp.Value();
	}
	catch (const NS_Logger::TLog& err)
	{
		err.raise(exit_on_er, "NS_Tune::TUserData::getFieldByCode");
	}
	catch (...)
	{
		TConstField v(code);
		NS_Logger::TLog("Не обработанная ошибка при получении параметра: " + v.toStr()).raise(false, "NS_Tune::TUserData::getFieldByCode");
	}
	return string();
}

NS_Tune::TSubParam NS_Tune::TUserData::getParamByID(int par_id, bool exit_on_er) const noexcept(false)
{
	try
	{
		const TSubParam& tmp = getValueByID(par_id, params);
		if (!tmp.isEmpty()) return TSubParam(tmp);
	}
	catch (const NS_Logger::TLog& err)
	{
		err.raise(exit_on_er, "NS_Tune::TUserData::getParamByID");
	}
	return TSubParam(string());
}

string NS_Tune::TUserData::getParamValByID(int par_id, bool exit_on_er) const noexcept(false)
{
	TSubParam tmp = getParamByID(par_id, exit_on_er);
	return tmp.Value();
}

void NS_Tune::TUserData::show_tunes(void) const
{
	using std::cout;
	using std::endl;
	if (EmptyFields()) return;
	for (TStringParam x : fields)
		cout << x.toStr(false) << endl;
};

void NS_Tune::TUserData::show_columns(void) const
{
	using std::cout;
	using std::endl;
	if (EmptyColumns()) return;
	for (string s : cols)
		cout << s << endl;
}

void NS_Tune::TUserData::show_params(void) const
{
	using std::cout;
	using std::endl;
	if (EmptyParams()) return;
	for (TSubParam par : params)
		par.show();
}