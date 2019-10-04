#include <iostream>
#include <iterator>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include "TuneParam.h"
#include "Logger.h"

using std::string;
using std::vector;
using std::set;
using std::size_t;
using std::streampos;

int NS_Tune::Str2Int(const string& str) noexcept(false)
{
	using std::stringstream;
	if (str.empty()) throw "Пустая строка! Преобразование невозможно!";
	stringstream ss;
	ss << str;
	int val;
	ss >> val;
	return val;
}

string NS_Tune::Int2Str(int val) noexcept(true)
{
	using std::stringstream;
	stringstream ss;
	ss << val;
	return ss.str();
}

template <typename T>
bool NS_Tune::TConstant<T>::isValid(const T& a, const T& b, bool exit_on_err) const noexcept(false)
{
	if (inRange(a, b))
		return true;
	if (exit_on_err)
		return false;
	throw "Ошибка попадания в диапазон значений!";
}

template <typename T>
void NS_Tune::TConstant<T>::setValue(const T& x)
{
	val = static_cast<int>(x);
}

template <typename T>
void NS_Tune::TConstant<T>::Init(int x)
{
	val = x;
	if (!isValid(true))
		val = 0;
}

template <typename T>
NS_Tune::TConstant<T>::TConstant(const T& x)
{
	setValue(x);
}


template <typename T>
NS_Tune::TConstant<T>& NS_Tune::TConstant<T>::Next(bool exit_on_err) noexcept(false)
{
	*this += 1;
	isValid(exit_on_err);
	return *this;
}

template<typename T>
NS_Tune::TConstant<T>& NS_Tune::TConstant<T>::operator=(const T& x)
{
	if (Value() != x) setValue(x);
	return *this;
}

string NS_Tune::TConstField::toStr() const
{
	switch (Value())
	{
	case TuneField::DataBase: return "[DATA BASE]";
	case TuneField::UserName: return "UserName";
	case TuneField::Password: return "Password";
	case TuneField::TNS: return "TNS";
	case TuneField::Report: return "[REPORT]";
	case TuneField::TemplateName: return "TemplateName";
	case TuneField::OutFileName: return "OutFileName";
	case TuneField::SqlFile: return "SQLFile";
	case TuneField::SqlText: return "SQLText";
	case TuneField::Columns: return "[COLUMNS]";
	case TuneField::Column: return "Column";
	case TuneField::SqlParams: return "[PARAMETERS]";
	case TuneField::SqlParam: return "Param";
	case TuneField::SqlParamQuane: return "Quane";
	case TuneField::SqlParamType: return "Type";
	case TuneField::SqlParamNote: return "Comment";
	case TuneField::SqlParamValue: return "Value";
	default: return string();
	}
};

NS_Tune::TConstType::TConstType(const string& str): TConstant<DataType>(DataType::ErrorType)
{
	while (Value() < DataType::Last)
	{
		if (toStr() == str) break;
		Next();
	}
	if (Value() == DataType::Last)
		setValue(DataType::ErrorType);
}

string NS_Tune::TConstType::toStr() const
{
	switch (Value())
	{
	case DataType::String: return "string";
	case DataType::Number: return "number";
	case DataType::Float: return "float";
	case DataType::Date: return "date";
	}
	return string();
}

string NS_Tune::TConstTag::toStr() const
{
	string result;
	switch (Value())
	{
	case Tags::colon: 
	case Tags::dash:
	case Tags::quotes:
	case Tags::rangle:
	case Tags::langle:
	case Tags::quane:
	case Tags::semicolon:
		result.push_back(char(toInt()));
		break;
	case Tags::open_param_tag:
		result = { char(Tags::colon), char(Tags::rangle) };
		break;
	case Tags::close_param_tag:
		result = { char(Tags::langle), char(Tags::dash), char(Tags::rangle) };
		break;
	}
	return result;
}

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

size_t NS_Tune::TBaseParam::get_pos_in_src(const Tags& substr, const size_t beg, find_fnc ff) const
{
	TConstTag tmp(substr);
	return get_pos_in_src(tmp.toStr(), beg, ff);
}

string NS_Tune::TBaseParam::Get_TuneField_Val(const TConstField& param, const Tags& b_delimeter, const Tags& e_delimeter) const
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

string NS_Tune::TStringParam::Get_TuneField_Val(const TConstField& param, const Tags& b_delimeter, const Tags& e_delimeter) const
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

void NS_Tune::TStringParam::setValue(const TConstField&, const Tags& open_val, const Tags& close_val)
{
	value = Get_TuneField_Val(param, open_val, close_val);
}

NS_Tune::TStringParam::TStringParam(const string& full_data, const TuneField& tune_field,
	const Tags& open_val, const Tags& close_val) : TBaseParam(full_data), param(tune_field)
{
	if (!TBaseParam::isEmpty() and !param.isEmpty())
		setValue(param, open_val, close_val);
}

string NS_Tune::TStringParam::toStr(bool use_quotes) const
{
	//возвращаем строку вида: ParamName="ParamValue"
	string result;
	TConstTag tag(Tags::quane);
	result = param.toStr() + tag.toStr();
	if (use_quotes)
	{
		tag = Tags::quotes;
		result += tag.toStr() + value + tag.toStr();
	}
	else
		result += value;
	return result;
}

void NS_Tune::TSubParam::setValue(void)
{
	TConstField tmp_field(TuneField::SqlParamQuane);
	string tmpID = Get_TuneField_Val(tmp_field, Tags::quane, Tags::semicolon);
	try
	{
		id = Str2Int(tmpID);
	}
	catch (...)
	{
		id = EmptyID;
	}
	tmp_field = TuneField::SqlParamType;
	string tmpType = Get_TuneField_Val(tmp_field, Tags::quane, Tags::semicolon);
	type = TConstType(tmpType);
	tmp_field = TuneField::SqlParamNote;
	comment = Get_TuneField_Val(tmp_field, Tags::quane, Tags::semicolon);
	tmp_field = TuneField::SqlParamValue;
	value = Get_TuneField_Val(tmp_field, Tags::quane, Tags::semicolon);
}

void NS_Tune::TSubParam::setValue(const TConstField&, const Tags&, const Tags&)
{
	setValue();
}

void NS_Tune::TSubParam::show() const
{
	using std::cout;
	using std::endl;
	if (isEmpty()) return;
	TConstField field(TuneField::SqlParamQuane);
	const TConstTag tag(Tags::quane);
	cout << field.toStr() << tag.toStr() << id << endl;
	field = TuneField::SqlParamType;
	cout << field.toStr() << tag.toStr() << type.toStr() << endl;
	field = TuneField::SqlParamNote;
	cout << field.toStr() << tag.toStr() << comment;
	field = TuneField::SqlParamValue;
	cout << field.toStr() << tag.toStr() << Value();
}

NS_Tune::TSubParam::TSubParam(const string& str): TBaseParam(str), id(EmptyID),
	type(DataType::ErrorType), comment()
{
	if (!TBaseParam::isEmpty())
		setValue();
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
	while (file)
	{
		string str;
		getline(file, str);
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
	streampos pos = file.tellg();
	file.seekg(pos);
	const TConstField Columns(TuneField::Columns);
	const TConstField Params(TuneField::SqlParams);
	for (TConstField i(TuneField::UserName); i.Value() < TuneField::SqlParam; i.Next())
	{
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
			//берем позицию первой строки с настройками:
			pos = file.tellg();
			//проверяем есть ли в данной строке запись о i-ой настройке
			TStringParam tmp_field(str, i.Value());
			if (!tmp_field.isEmpty())
			{
				fields.insert(tmp_field);
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
	using std::sort;
	using std::getline;
	ifstream file(filename.c_str(), ios_base::in);
	if (!file.is_open())
		throw Logger::TLog("Ошибка открытия файла: ", filename.c_str(), nullptr);
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
	if (arr.empty()) throw "Массив значений пуст!";
	for (const ValType& s: arr)
	{
		if (s > par_ID) break;
		if (s == par_ID) return s;
	}
	throw "Данные не найдены!";
}

string NS_Tune::TUserData::getFieldByCode(const TuneField& code, bool exit_on_er) const noexcept(false)
{
	try
	{
		const TStringParam& tmp = getValueByID(code, fields);
		if (!tmp.isEmpty()) return tmp.Value();
	}
	catch (const string& err)
	{
		if (!exit_on_er) throw err;
		std::cerr << err << std::endl;
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
	catch (const string& err)
	{
		if (!exit_on_er) throw err;
		std::cerr << err << std::endl;
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