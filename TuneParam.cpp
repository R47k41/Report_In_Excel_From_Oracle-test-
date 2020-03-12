#include <iostream>
#include <iterator>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "TuneParam.h"
#include "Logger.h"
#include "TConverter.hpp"


using std::string;
using std::vector;
using std::set;
using std::size_t;
using std::streampos;

template void NS_Tune::TShareData::setArrayByJson<NS_Tune::TSheetData>(const ptree::value_type& node, const JsonParams& tag,
	vector<NS_Tune::TSheetData>& arr);

template void NS_Tune::TShareData::setArrayByJson<NS_Tune::TFilterData>(const ptree::value_type& node, const JsonParams& tag,
	vector<NS_Tune::TFilterData>& arr);

namespace NS_Tune
{
	void raise_app_err(const NS_Logger::TLog& log, bool as_raise = true);
}

void NS_Tune::raise_app_err(const NS_Logger::TLog& log, bool as_raise)
{
	as_raise ? throw log : log.toErrBuff();
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

size_t NS_Tune::TBaseParam::get_pos_in_src(const CtrlSym& substr, const size_t beg, find_fnc ff) const
{
	TConstCtrlSym tmp(substr);
	return get_pos_in_src(tmp.toStr(), beg, ff);
}

string NS_Tune::TBaseParam::Get_Str_Val(size_t pos, const CtrlSym& b_delimeter, const CtrlSym& e_delimeter,
	bool from_end) const
{
	if (pos == string::npos) return string();
	//ищем открывающий разделитель для получения значения
	pos = get_pos_in_src(b_delimeter, pos);
	size_t pose = 0;
	if (from_end)
		pose = get_pos_in_src(e_delimeter, string::npos, &string::rfind);
	else
		pose = get_pos_in_src(e_delimeter, pos);
	if (pose == string::npos) return string();
	return srcSubStr(pos, pose - 1);
}

string NS_Tune::TBaseParam::Get_TuneField_Val(const TConstField& param, const CtrlSym& b_delimeter,
	const CtrlSym& e_delimeter, bool pose_from_end) const
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
	return Get_Str_Val(pos, b_delimeter, e_delimeter, pose_from_end);
};

void NS_Tune::TStringParam::setValue(const TConstField&, const CtrlSym& open_val, const CtrlSym& close_val)
{
	value = Get_TuneField_Val(param, open_val, close_val, true);
}

NS_Tune::TStringParam::TStringParam(const TuneField& tune_field, const string& str, bool parse_val):
	TBaseParam(str), param(tune_field)
{
	//если значение выбирается из строки:
	if (parse_val)
		value = Get_Str_Val(0, CtrlSym::crwn, CtrlSym::crwn, true);
	else
		value = str;
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
	using NS_Const::LowerCase;
	TConstField tmp_field(TuneField::SqlParamQuane);
	string tmpID = Get_TuneField_Val(tmp_field, CtrlSym::quane, CtrlSym::semicolon);
	if (!toType(tmpID, &id)) id = EmptyID;
	tmp_field = TuneField::SqlParamType;
	string tmpType = Get_TuneField_Val(tmp_field, CtrlSym::quane, CtrlSym::semicolon);
	//тип данных берем в нижнем регистре:
	tmpType = LowerCase(tmpType);
	//получение ссылки на тип данных:
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

void NS_Tune::TSubParam::show(std::ostream& stream) const
{
	using std::endl;
	if (!stream or isEmpty()) return;
	TConstField field(TuneField::SqlParamQuane);
	const TConstCtrlSym tag(CtrlSym::quane);
	stream << field.toStr() << tag.toStr() << id << endl;
	field = TuneField::SqlParamType;
	stream << field.toStr() << tag.toStr() << type.toStr() << endl;
	field = TuneField::SqlParamNote;
	stream << field.toStr() << tag.toStr() << comment << endl;
	field = TuneField::SqlParamValue;
	stream << field.toStr() << tag.toStr() << Value() << endl;
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

vector<string> NS_Tune::TSimpleTune::getFileLst(const string& file_dir, const string& file_ext, bool use_sort) noexcept(false)
{
	using boost::filesystem::path;
	using boost::filesystem::directory_entry;
	using boost::filesystem::directory_iterator;
	using boost::filesystem::exists;
	using boost::filesystem::is_directory;
	using boost::filesystem::is_regular_file;
	using boost::filesystem::extension;
	//функция для проверки корректности добавления файла:
	bool no_ext = file_ext.empty();
	vector<string> result;
	try
	{
		//получение имени директории:
		path dir(file_dir.c_str());
		//если это директория:
		if (is_directory(dir))
		{
			//форируем список файлоа
			for (directory_entry& x : directory_iterator(dir))
				if (is_regular_file(x) && (no_ext || extension(x) == file_ext))
					result.push_back(x.path().string());
		}
		else
		{
			//добавление данного файла в список
			if (no_ext || extension(dir) == file_ext)
				result.push_back(file_dir);
		}
	}
	catch (const boost::filesystem::filesystem_error& err)
	{
		throw NS_Logger::TLog(err.what(), "getFileLst");
	}
	if (use_sort and result.size() > 1)
		std::sort(result.begin(), result.end());
	return result;
}

NS_Tune::FileParam NS_Tune::TSimpleTune::getFileParamByCode(const Types& code) const noexcept(true)
{
	FileParam result;
	result.first += getPathByCode(code);
	switch (code)
	{
	case Types::Config:
		result.second = getFieldValueByCode(TuneField::ConfigFileExt);
		break;
	case Types::SubConfig:
		result.second = getFieldValueByCode(TuneField::SubTuneFileExt);
		break;
	case Types::SQL:
	case Types::DQL:
	case Types::DML:
		result.second = getFieldValueByCode(TuneField::SqlFileExt);
		break;

	case Types::Template:
		result.second = getFieldValueByCode(TuneField::TemplateFileExt);
		break;
	}
	return result;
}

vector<string> NS_Tune::TSimpleTune::getFileLst(const Types& code, bool use_sort) const noexcept(false)
{
	FileParam param = getFileParamByCode(code);
	return getFileLst(param.first, param.second, use_sort);
}

void NS_Tune::TSimpleTune::AddDelimeter(string& str, const char delim) noexcept(false)
{
	if (!str.empty() && str[str.size() - 1] != delim)
		str += delim;
}

string NS_Tune::TSimpleTune::getPathByCode(const Types& code) const noexcept(true)
{
	string name;
	char delimeter;
	delimeter = TConstCtrlSym::asChr(CtrlSym::dash);
	//получение данных из настроек
	switch (code)
	{
	case Types::Config:
		name = getFieldValueByCode(TuneField::ConfigPath);
		break;
	case Types::SubConfig:
		name = getFieldValueByCode(TuneField::SubTunePath);
		break;
	case Types::SQL:
	case Types::DQL:
	case Types::DML:
		name = getFieldValueByCode(TuneField::SqlPath);
		break;
	case Types::Template:
		name = getFieldValueByCode(TuneField::TemplatePath);
		break;
	case Types::OutPath:
	case Types::OutName:
	{
		name = getFieldValueByCode(TuneField::OutDirectory);
		AddDelimeter(name, delimeter);
		//проверка создания под папки
		string sub_name = AddCurDate2Name(Types::OutPath);
		if (!sub_name.empty())
			name += sub_name;
		AddDelimeter(name, delimeter);
		return name;
	}
	default: return string();
	}
	string path = getFieldValueByCode(TuneField::MainPath);
	AddDelimeter(path, delimeter);
	path += name;
	AddDelimeter(path, delimeter);
	return path;
}

bool NS_Tune::TSimpleTune::set_date_format(std::ostream& stream, const string& format) noexcept(true)
{
	using std::locale;
	using boost::gregorian::date_facet;
	if (format.empty()) return false;
	try
	{
		locale loc(stream.getloc(), new date_facet(format.c_str()));
		stream.imbue(loc);
	}
	catch (...)
	{
		NS_Logger::TLog("Ошибка при установке локали " + format + " потока!", "TSimpleTune::set_date_format");
		return false;
	}
	return true;
}

string NS_Tune::TSimpleTune::cur_date_to_string_by_format(const string& format) noexcept(false)
{
	using boost::gregorian::day_clock;
	using boost::gregorian::date;
	using std::stringstream;
	stringstream ss;
	if (set_date_format(ss, format))
	{
		//получение текущей даты
		date d = day_clock::local_day();
		ss << d;
		return ss.str();
	}
	return string();
}

string NS_Tune::TSimpleTune::AddCurDate2Name(const Types& code) const noexcept(false)
{
	string format;
	switch (code)
	{
	case Types::OutPath:
		format = getFieldValueByCode(TuneField::AddDateToOutPath);
		break;
	case Types::OutName:
		format = getFieldValueByCode(TuneField::AddDateToOutFileName);
		break;
	case Types::OutSheet:
		format = getFieldValueByCode(TuneField::AddDateToSheetName);
		break;
	}
	return cur_date_to_string_by_format(format);
}

bool NS_Tune::TSimpleTune::AddField(const TStringParam& val) noexcept(true)
{
	try
	{
		if (!val.isEmpty())
		{
			//fields.insert(val);
			fields.push_back(val);
			return true;
		}
	}
	catch (...)
	{
		NS_Logger::TLog log("Необработанная ошибка добавления параметра: ", "TSimpleTune::AddField");
		log << val.ParamName() << "(" << val.Value() << ")" << NS_Logger::TLog::NL;
		log.toErrBuff();
	}
	return false;
}

bool NS_Tune::TSimpleTune::setFieldVal(const TuneField& key, const string& val) noexcept(false)
{
	if (!Empty())
	{
		for (TStringParam x : fields)
			if (x.Param() == key)
			{
				x.Value(val);
				return true;
			}
	}
	return false;
}

NS_Tune::RangeField NS_Tune::TSimpleTune::getTuneField4Range(const TuneRange& range)
{
	RangeField result;
	TConstField i(range.first);
	for (i.Next(); i < range.second; i.Next())
		result.insert(i);
	return result;
}

void NS_Tune::TSimpleTune::Read_Tune_Val(ifstream& file)
{
	using std::getline;
	using std::streampos;
	using NS_Const::TConstCtrlSym;
	using NS_Const::CtrlSym;
	RangeField field_arr;
	const char GrpFlg = TConstCtrlSym::asChr(CtrlSym::qlbkt);
	//формирование массива искомых настроек
	TuneRange range = getTuneRange(TRead::TuneVal);
	if (range.first == TuneField::Empty) return;
	field_arr = getTuneField4Range(range);
	//чтение файла:
	while (file)
	{
		//считывание строки настройки
		string str;
		getline(file, str);
		if (!std::isalpha(str[0]))
		{
			if (str[0] == GrpFlg)
				Read_Section(file, str);
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
				//добавление параметра
				AddField(TStringParam(str, i.Value()));
				//удаляем параметр из искомых:
				field_arr.erase(i);
				break;
			}
	}
};

NS_Tune::TSharedTune::TSharedTune(const string& file, const string& code) :
	TSimpleTune(), main_code(code)
{
	//инициализация кода отчета:
	if (main_code.empty()) main_code = getCodeFromtUI();
	//инициализация файла:
	TSimpleTune::Initialize(file);
}

NS_Tune::TSharedTune::TSharedTune(const TFields& v, const string& file, const string& code) :
	TSimpleTune(v), main_code(code)
{
	//инициализация кода отчета:
	if (main_code.empty()) main_code = getCodeFromtUI();
	//инициализация файла:
	TSimpleTune::Initialize(file);
}

NS_Tune::TSharedTune::TSharedTune(const TSharedTune& v, const string& file, const string& code) :
	TSimpleTune(v), main_code(code) 
{
	//инициализация кода отчета:
	if (main_code.empty()) main_code = getCodeFromtUI();
	//инициализация файла:
	TSimpleTune::Initialize(file);
}

NS_Tune::TUserTune::TUserTune(const string& tunefile) : TSimpleTune()
{
	TSimpleTune::ReadFromFile(tunefile);
}

NS_Tune::TUserTune::TUserTune(const TSimpleTune& tune, const string& file) : TSimpleTune(tune)
{
	TSimpleTune::ReadFromFile(file);
}


string NS_Tune::TSharedTune::getMainCode(bool use_brkt) const
{
	string result;
	if (!main_code.empty())
	{
		result = main_code;
		if (use_brkt)
			result = TConstCtrlSym::asStr(CtrlSym::qlbkt) + result + TConstCtrlSym::asStr(CtrlSym::qrbkt);
	}
	return result;
}

bool NS_Tune::TSharedTune::CheckPath(const string& dir, bool crt_if_not_found)
{
	using boost::filesystem::path;
	using boost::filesystem::exists;
	using boost::filesystem::create_directory;
	using boost::filesystem::status;
	using boost::filesystem::is_directory;
	path tmp_dir(dir);
	if (exists(tmp_dir)) return true;
	//если файла нет и его надо создать:
	if (crt_if_not_found)
		return create_directory(tmp_dir);
	return false;
}

void NS_Tune::TSimpleTune::ReadFromFile(const string& filename)
{
	using std::ifstream;
	using std::ios_base;
	using std::sort;
	using std::getline;
	ifstream file(filename.c_str(), ios_base::in);
	if (!file.is_open())
		throw NS_Logger::TLog("Ошибка открытия файла: " + filename, "NS_Tune::TUserTune::ReadFromFile");
	//считываем значения для настроек:
	Read_Tune_Val(file);
	//считываение данных для колонок:
	//Read_Param_Val(file);
	//считываем данные для колонок:
	//Read_Col_Val(file);
	//закрываем файл
	file.close();
};

template <typename KeyType, typename ValType>
ValType& NS_Tune::TSimpleTune::getElementByID(const KeyType& par_ID, vector<ValType>& arr) noexcept(false)
{
	using NS_Logger::TLog;
	if (arr.empty()) throw TLog("Массив значений пуст!", "NS_Tune::TUserTune::getElementByID");
	for (ValType& val: arr)
	{
		if (val > par_ID) break;
		if (val == par_ID) return val;
//		return ValType();
	}
	TLog log("Данные не найдены!", "NS_Tune::TUserTune::getElementByID");
	//log << par_ID << TLog::NL;
	throw log;
}

template <typename KeyType, typename ValType>
const ValType& NS_Tune::TSimpleTune::getConstElementByID(const KeyType& par_ID, const vector<ValType>& arr) noexcept(false)
{
	using NS_Logger::TLog;
	if (arr.empty()) throw TLog("Массив значений пуст!", "NS_Tune::TUserTune::getElementByID");
	for (const ValType& val : arr)
	{
		//if (val > par_ID) break;
		if (val == par_ID) return val;
		//		return ValType();
	}
	TLog log("Данные не найдены!", "NS_Tune::TUserTune::getElementByID");
	//log << par_ID << TLog::NL;
	throw log;
}

void NS_Tune::TSimpleTune::skip_block(ifstream& file, const string& end_block)
{
	while (file)
	{
		string str;
		getline(file, str);
		if (end_block == str) return;
	}
}

string NS_Tune::TSimpleTune::getNameByCode(const Types& code) const noexcept(true)
{
	using NS_Const::TConstCtrlSym;
	using NS_Const::CtrlSym;
	using NS_Const::TConstExclTune;
	using NS_Const::TExclBaseTune;
	string name;
	char delimeter;
	switch (code)
	{
	case Types::SQL:
	case Types::DQL:
	{
		name = getFieldValueByCode(TuneField::SqlFile);
		break;
	}
	case Types::DML:
	{
		name = getFieldValueByCode(TuneField::DMLFile);
		break;
	}
	case Types::Template:
	{
		name = getFieldValueByCode(TuneField::TemplateName);
		break;
	}
	case Types::OutName:
	{
		string sub_name = AddCurDate2Name(code);
		delimeter = TConstCtrlSym::asChr(CtrlSym::Space);
		name = getFieldValueByCode(TuneField::OutFileName);
		if (name.empty())
		{
			name = TConstExclTune::asStr(TExclBaseTune::DefName) + delimeter + sub_name;
			name += TConstExclTune::asStr(TExclBaseTune::DefExt);
			return name;
		}
		size_t pos = name.rfind(".");
		if (pos == string::npos)
		{
			name += delimeter + sub_name + TConstExclTune::asStr(TExclBaseTune::DefExt);
			return name;
		}
		name = name.substr(0, pos) + delimeter + sub_name + name.substr(pos);
		break;
	}
	case Types::OutSheet:
	{
		string sub_name = AddCurDate2Name(code);
		delimeter = TConstCtrlSym::asChr(CtrlSym::Space);
		name = getFieldValueByCode(TuneField::SheetName);
		if (name.empty())
			name = TConstExclTune::asStr(TExclBaseTune::DefSh);
		name += delimeter + sub_name;
		break;
	}
	}
	return name;
}

string NS_Tune::TSimpleTune::getFullFileName(const Types& code, bool only_path) const noexcept(true)
{
	//получение пути к файлу
	string path = getPathByCode(code);
	if (only_path) return path;
	//получение имени файла:
	string name = getNameByCode(code);
	if (name.empty()) return string();
	return path + name;
}

string NS_Tune::TSimpleTune::getFieldValueByCode(const TuneField& code) const noexcept(true)
{
	using std::cerr;
	using std::endl;
	try
	{
		TStringParam tmp = getConstElementByID(code, fields);
		if (!tmp.isEmpty()) return tmp.Value();
	}
	catch (const NS_Logger::TLog& err)
	{
		cerr << "По коду: " << TConstField::asStr(code) << " данные не найдены!" << endl;
		err.toErrBuff();
	}
	catch (...)
	{
		NS_Logger::TLog("Не обработанная ошибка при получении параметра: " + TConstField::asStr(code), "NS_Tune::TUserTune::getFieldByCode").toErrBuff();
	}
	return string();
}

bool NS_Tune::TSimpleTune::FieldValueAsInt(const TuneField& code, int& val) const noexcept(false)
{
	using NS_Converter::toType;
	string tmp = getFieldValueByCode(code);
	if (tmp.empty()) return false;
	if (!toType(tmp, &val))
		return false;
	return true;
}

void NS_Tune::TSimpleTune::show_tunes(std::ostream& stream) const
{
	using std::endl;
	if (!stream or Empty()) return;
	for (TStringParam x : fields)
		stream << x.toStr(false) << endl;
};

string NS_Tune::TSimpleTune::getOutFileBySheet() const noexcept(false)
{
	string result = getPathByCode(Types::OutPath) + getNameByCode(Types::OutSheet);
	//если путь и страница не пустые
	if (!result.empty() and result[result.size() - 1] != TConstCtrlSym::asChr(CtrlSym::dash))
		//получаем расширение из файла:
		result += NS_Const::TConstExclTune::getFileExtention(getNameByCode(Types::OutName));
	return result;
}

bool NS_Tune::TSimpleTune::useFlag(const TuneField& code) const noexcept(true)
{
	int val = 0;
	if (FieldValueAsInt(code, val))
		return val == 1;
	return false;
}

string NS_Tune::TSharedTune::getCodeFromtUI() noexcept(true)
{
	using std::cout;
	using std::endl;
	using std::cin;
	using NS_Const::ReportCode;
	using NS_Const::TConstReportCode;
	TConstReportCode report(ReportCode::Empty);
	do
	{
		TConstReportCode::show();
		size_t val = 0;
		cout << endl << "Укажите код отчета:\t";
		cin >> val;
		report = val;
	} while (report.isEmpty() or !report.isValid(true));
	while (!cin.get());
	cout << "Будет сформирован отчет: " << report.getName() << endl;
	//убираем все из буфера
	//выходим
	return report.toStr();
}

string NS_Tune::TSharedTune::getSectionName() const noexcept(true)
{
	if (main_code.empty()) return string();
	return TConstCtrlSym::asStr(CtrlSym::qlbkt) + main_code + TConstCtrlSym::asStr(CtrlSym::qlbkt);
}

NS_Tune::TuneRange NS_Tune::TSharedTune::getTuneRange(const TRead& x) const noexcept(true)
{
	switch (x)
	{
	case TRead::Section:
	case TRead::TuneVal:
		return TuneRange(TuneField::Start_Shared_Index, TuneField::End_Shared_Index);
	}
	return TuneRange(TuneField::Empty, TuneField::Empty);
}

void NS_Tune::TSharedTune::Read_Section(ifstream& file, const string& code)
{
	using NS_Const::TuneField;
	using NS_Const::TConstField;
	string ID = getMainCode();
	//если не указан код основной настройки:
	if (ID.empty()) return;
	//если указан код основной секции:
	if (ID == code)
	{
		//получение списка считываемых настроек:
		TuneRange range = getTuneRange(TRead::Section);
		//формируем список искомых полей:
		RangeField filed_arr = getTuneField4Range(range);
		//считывание параметров происходит до символа [
		char block_end = TConstCtrlSym::asChr(CtrlSym::qlbkt);
		//чтение файла:
		while (file)
		{
			//считывание строки основной настройки
			string str;
			getline(file, str);
			//пропускаем коментарии
			if (!std::isalpha(str[0]))
			{
				//выход если дошли до конца блока - выход
				if (block_end == str[0])
					break;
				else
					continue;
			}
			//проверяем относится ли данная строка к параметру:
			for (const TConstField& f : filed_arr)
			{
				//если строка относится к параметрам:
				if (f.StrInclude(str))
				{
					//формируем параметр:
					TStringParam new_param(f.Value(), str);
					//проверяем имеется ли она уже в параметрах:
					try
					{
						//меняем значение параметру
						TStringParam& tmp = getElementByID(f.Value(), fields);
						tmp.Value(new_param.Value());
					}
					catch (...)
					{
						//если не нашли - вставляем
						fields.push_back(new_param);
					}
					break;
				}
			}
		}
	}
	skip_block(file, TConstField::asStr(TuneField::Block_End));
}

void NS_Tune::TSimpleTune::Initialize(const string& file) noexcept(true)
{
	using std::cout;
	using std::endl;
	using std::cin;
	using std::getline;
	try
	{
		string file_name;
		//если пустое имя файла:
		if (file.empty())
		{
			//получаем имя файла от пользователя
			do
			{
				cout << "Необходимо указать имя файла настроек:" << endl;
				getline(cin, file_name);
			} while (file_name.empty());
		}
		else
			file_name = file;
		ReadFromFile(file_name);
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработнная ошибка при получении настроек из файла: " + file + '\n', "TSharedTune::Initialize").toErrBuff();
	}
}

void NS_Tune::TUserTune::Read_StreamData_Val(ifstream& file, const TuneField& stream_title, StrArr& str_arr)
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

void NS_Tune::TUserTune::Read_Param_Val(ifstream& file)
{
	StrArr tmp;
	Read_StreamData_Val(file, TuneField::SqlParam, tmp);
	for (string s : tmp)
		if (!s.empty())
		{
			TSubParam sp(s);
			if (!sp.isEmpty()) params.push_back(sp);
		}
}

void NS_Tune::TUserTune::Read_Col_Val(ifstream& file)
{
	Read_StreamData_Val(file, TuneField::Column, cols);
};

NS_Tune::TuneRange NS_Tune::TUserTune::getTuneRange(const TRead& x) const noexcept(true)
{
	switch (x)
	{
	case TRead::Section: return TuneRange(TuneField::SqlParams, TuneField::Block_End);
	case TRead::TuneVal: return TuneRange(TuneField::Start_Unq_Tune, TuneField::End_Unq_Tune);
	}
	return TuneRange(TuneField::Empty, TuneField::Empty);
}

void NS_Tune::TUserTune::Read_Section(ifstream& file, const string& code)
{
	using NS_Const::TuneField;
	using NS_Const::TConstField;
	//получение диапазона считываемых параметров
	TuneRange range = getTuneRange(TSimpleTune::TRead::Section);
	TuneField ID = TConstField::getIDByCode(code, range.first, range.second);
	//если секция не совпала с кодом - выход
	if (ID == TuneField::Empty) return;
	switch (ID)
	{
	case TuneField::Columns:
		Read_Col_Val(file);
		break;
	case TuneField::SqlParams:
		Read_Param_Val(file);
		break;
	}
}

NS_Tune::TSubParam NS_Tune::TUserTune::getParamByID(int par_id, bool exit_on_er) const noexcept(false)
{
	try
	{
		TSubParam tmp = getConstElementByID(par_id, params);
		if (!tmp.isEmpty()) return TSubParam(tmp);
	}
	catch (const NS_Logger::TLog& err)
	{
		raise_app_err(err, !exit_on_er);
	}
	return TSubParam(string());
}

string NS_Tune::TUserTune::getParamValByID(int par_id, bool exit_on_er) const noexcept(false)
{
	TSubParam tmp = getParamByID(par_id, exit_on_er);
	return tmp.Value();
}

void NS_Tune::TUserTune::show_columns(std::ostream& stream) const
{
	using std::endl;
	if (!stream or EmptyColumns()) return;
	for (string s : cols)
		stream << s << endl;
}

void NS_Tune::TUserTune::show_params(std::ostream& stream) const
{
	using std::endl;
	if (!stream or EmptyParams()) return;
	for (TSubParam par : params)
		par.show();
}

vector<string> NS_Tune::TUserTune::getSQLFileLst(const Types& code, bool use_sort) const noexcept(false)
{
	//получение данных о файлах и путях(если будут меняться расширения файлов - запихнуть в switch):
	FileParam param = getFileParamByCode(Types::SQL);
	//получение обрабатываемых кодов:
	switch (code)
	{
	case Types::DQL:
		param.first += getFieldValueByCode(TuneField::SqlFile);
		break;
	case Types::DML:
		param.first += getFieldValueByCode(TuneField::DMLFile);
		break;
	}
	return getFileLst(param.first, param.second, use_sort);
}

bool NS_Tune::TUserTune::isEmptyByCode(const Types& code) const noexcept(false)
{
	switch (code)
	{
	case Types::DQL:
		return getFieldValueByCode(TuneField::SqlFile).empty() && getFieldValueByCode(TuneField::SqlText).empty();
	case Types::DML:
		return getFieldValueByCode(TuneField::DMLFile).empty() && getFieldValueByCode(TuneField::DMLText).empty();
	}
	return false;
}

string NS_Tune::TIndex::getStrValue(const ptree& parent_node, const JsonParams& tag) noexcept(true)
{
	using NS_Converter::UTF8ToANSI;
	using NS_Const::TConstJson;
	string v_tag = TConstJson::asStr(tag);
	try
	{
		if (!TConstJson::isTag(tag))
			throw TLog("Указанный тег не обрабатывается!", "getStrValue");
		//если данные в корне пустые или тег не обрабатывается:
		if (parent_node.empty())
			throw TLog("Пустое содержимое в JSon-файле!", "getStrValue");
		//получение строки в uncode-кодировке:
		string val = parent_node.get_child(v_tag).get_value<string>("");
		if (UTF8ToANSI(val)) return val;
	}
	catch (TLog& er)
	{
		er << "(тег: " << v_tag << ")\n";
		er.toErrBuff();
	}
	catch (...)
	{
		TLog log("Не обработанная ошибка получения данных!", "getStrValue");
		log << "\n(тег: " << v_tag << ")\n";
		log.toErrBuff();
	}
	return string();
}

bool NS_Tune::TIndex::setStrValue(const ptree& parent_node, const JsonParams& tag, const string& val) noexcept(true)
{
	using NS_Converter::MByteToUnicode;
	using NS_Const::TConstJson;

	return false;
}

NS_Tune::TColor NS_Tune::TIndex::getColorValue(const ptree::value_type& parent_node, const JsonParams& tag) noexcept(true)
{
	using NS_Const::TConstJson;
	using NS_Const::EmptyType;
	using boost::property_tree::json_parser_error;
	string v_tag = TConstJson::asStr(tag);
	try
	{
		if (!TConstJson::isTag(tag))
			throw TLog("Указанный тег не обрабатывается!", "getStrValue");
		//если данные в корне пустые или тег не обрабатывается:
		if (parent_node.second.empty())
			throw TLog("Пустое содержимое в JSon-файле!", "getStrValue");
		//получение строки в uncode-кодировке:
		int val = parent_node.second.get_child(v_tag).get_value<int>();
		if (val == EmptyType)
			return TColor::COLOR_NONE;
		else
			return TColor(val);
	}
	catch (const json_parser_error& err)
	{
		TLog("Ошибка получения цвета для тега: " + v_tag + '\n' + err.what(), "TIndex::getColorValue").toErrBuff();
	}
	catch (TLog& er)
	{
		er << "(тег: " << v_tag << ")\n";
		er.toErrBuff();
	}
	catch (...)
	{
		TLog log("Не обработанная ошибка получения данных!", "getStrValue");
		log << "\n(тег: " << v_tag << ")\n";
		log.toErrBuff();
	}
	return TColor::COLOR_NONE;
}

string NS_Tune::TIndex::getStrValue(const ptree::value_type& parent_node, const JsonParams& tag) noexcept(true)
{
	return getStrValue(parent_node.second, tag);
}


void NS_Tune::TIndex::setIndexFromJSon(const ptree::value_type& parent_node, const string& tagStr) noexcept(false)
{
	if (!parent_node.second.empty())
		index = parent_node.second.get<size_t>(tagStr, EmptyIndex);
}

void NS_Tune::TIndex::setByJSon(const ptree::value_type& parent_node, const NS_Const::JsonParams& tag) noexcept(true)
{
	using NS_Const::JsonParams;
	using NS_Const::TConstJson;
	string tmp = TConstJson::asStr(tag);
	try
	{
		if (!TConstJson::isTag(tag)) throw TLog("Указанный тег: " + tmp + " не обрабатывается!", "setByJSon");
		setIndexFromJSon(parent_node, tmp);
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка при установке значения индекса по тегу: " + tmp, "setByJSon").toErrBuff();
	}
}

void NS_Tune::TIndex::show(std::ostream& stream, const string& front_msg) const noexcept(false)
{
	using std::endl;
	if (!stream) return;
	stream << front_msg;
	if (isEmpty())
		stream << "пустое значение";
	else
		stream << index;
	stream << endl;
}

void NS_Tune::TSheetData::setData(const ptree::value_type& parent_node, const JsonParams& indx_tag,
	const JsonParams& col_tag, const JsonParams& first_row_tag, const JsonParams& last_row_tag) noexcept(true)
{
	index.setByJSon(parent_node, indx_tag);
	col_id.setByJSon(parent_node, col_tag);
	first_row.setByJSon(parent_node, first_row_tag);
	last_row.setByJSon(parent_node, last_row_tag);
}

NS_Tune::TSheetData::TSheetData(const ptree::value_type& parent_node)
{
	setData(parent_node);
}


void NS_Tune::TSheetData::show(std::ostream& stream) const noexcept(true)
{
	if (!stream or isEmpty()) return;
	//выводим данные:
	index.show(stream, "Индекс листа страницы: ");
	if (!NoColID())
		col_id.show(stream, "Индекс колонки-идентификатора: ");
	if (!NoFirstRowIndex())
		first_row.show(stream, "Индекс первой строки: ");
	if (!NoLastRowIndex())
		last_row.show(stream, "Индекс последней строки: ");
}

void NS_Tune::TFilterData::setData(const ptree::value_type& parent_node,
	const JsonParams& col_tag, const JsonParams& val_tag) noexcept(true)
{
	col_indx.setByJSon(parent_node, col_tag);
	value = TIndex::getStrValue(parent_node, val_tag);
}

bool NS_Tune::TFilterData::operator==(const string& val) const noexcept(true)
{
	using NS_Const::Trim;
	using NS_Const::LowerCase;
	string a = LowerCase(value);
	string b = LowerCase(val);
	Trim(a);
	Trim(b);
	return a == b;
}

void NS_Tune::TFilterData::show(std::ostream& stream) const noexcept(true)
{
	if (stream and !isEmpty())
	{
		col_indx.show(stream, "Индекс колонки для фильтрации: ");
		stream << "Значение фильтра: " << value << std::endl;
	}
}

template <typename Type>
void NS_Tune::TShareData::setArrayByJson(const ptree::value_type& node, const JsonParams& tag,
	vector<Type>& arr) noexcept(false)
{
	using NS_Const::TConstJson;
	string v_tag = TConstJson::asStr(tag);
	for (const ptree::value_type& child : node.second.get_child(v_tag))
	{
		if (child.second.empty()) continue;
		Type tmp(child);
		if (!tmp.isEmpty())
			arr.push_back(tmp);
	}
}

void NS_Tune::TShareData::setData(const ptree::value_type& parent_node,
	const JsonParams& name_tag, const JsonParams& sheet_tag,
	const JsonParams& fltr_tag) noexcept(true)
{
	using NS_Const::TConstJson;
	if (parent_node.second.empty()) return;
	//инициализация параметров
	name = TIndex::getStrValue(parent_node, name_tag);
	//установка параметров для листов:
	setArrayByJson<TSheetData>(parent_node, sheet_tag, sh_params);
	//инициализация массива фильтров:
	setArrayByJson<TFilterData>(parent_node, fltr_tag, fltr);
}

NS_Tune::TShareData::TShareData(ptree& main_node, const string& main_path)
{
	using boost::property_tree::ptree;
	using NS_Const::JsonParams;
	using NS_Const::TConstJson;
	string tag = TConstJson::asStr(JsonParams::DstFile);
	if (tag.empty()) return;
	const ptree::value_type& sub_node = main_node.find(tag).dereference();
	setData(sub_node);
	if (!name.empty())	name = main_path + name;
}

NS_Tune::TShareData::TShareData(const ptree::value_type& parent_node, const string& main_path)
{
	setData(parent_node);
	if (!name.empty())	name = main_path + name;
}

void NS_Tune::TShareData::show(std::ostream& stream) const noexcept(true)
{
	using std::endl;
	if (stream and !isEmpty())
	{
		stream << "Наименование файла: " << name << endl;
		for (size_t i = 0; i < sh_params.size(); i++)
		{
			stream << "Параметры " << i << " страницы:" << endl;
			sh_params[i].show();
		}
			
		if (!fltr.empty())
		{
			stream << "Данные для фильтрации: " << endl;
			for (const TFilterData& f : fltr)
				if (!f.isEmpty())	f.show();
		}
	}
}

NS_Tune::TCellData& NS_Tune::TCellData::setData(size_t dst, size_t ins, size_t src_param, size_t val) noexcept(true)
{
	setDstIndex(dst);
	setInsIndex(ins);
	setSrcParam(src_param);
	setSrcVal(val);
	return *this;
}

NS_Const::DataType NS_Tune::TCellData::getTypeFromJsonByCode(const ptree::value_type& node, const JsonParams& tag) noexcept(true)
{
	using NS_Const::TConstType;
	string val = TIndex::getStrValue(node.second, tag);
	return TConstType(val).Value();
}

NS_Tune::TCellData& NS_Tune::TCellData::setData(const ptree::value_type& parent_node,
	const JsonParams& dst_tag, const JsonParams& dst_ins_tag,
	const JsonParams& src_param_tag, const JsonParams& src_val_tag,
	const JsonParams& in_type, const JsonParams& out_type) noexcept(true)
{
	using NS_Const::TConstType;
	dst_indx.setByJSon(parent_node, dst_tag);
	dst_ins_indx.setByJSon(parent_node, dst_ins_tag);
	src_param_indx.setByJSon(parent_node, src_param_tag);
	src_val_indx.setByJSon(parent_node, src_val_tag);
	in_data_type = getTypeFromJsonByCode(parent_node, in_type);
	out_data_type = getTypeFromJsonByCode(parent_node, out_type);
	return *this;
}

NS_Tune::TCellData& NS_Tune::TCellData::operator=(const TCellData& cd) noexcept(true)
{
	if (this != &cd)
	{
		dst_indx = cd.dst_indx;
		dst_ins_indx = cd.dst_ins_indx;
		src_param_indx = cd.src_param_indx;
		src_val_indx = cd.src_val_indx;
		in_data_type = cd.in_data_type;
		out_data_type = cd.out_data_type;
	}
	return *this;
}

void NS_Tune::TCellData::show(std::ostream& stream) const noexcept(true)
{
	using NS_Const::TConstType;
	if (stream)
	{
		dst_indx.show(stream, "Индекс ячейки приемника: ");
		dst_ins_indx.show(stream, "Индекс ячейки для вставки данных в приемнике: ");
		src_param_indx.show(stream, "Индекс-параметр в источнике: ");
		src_val_indx.show(stream, "Индекс-значения в источнике: ");
		if (in_data_type != DataType::ErrorType)
			stream << "Тип данных в ячейке приемника: " << TConstType::asStr(in_data_type) << std::endl;
		if (out_data_type != DataType::ErrorType)
			stream << "Тип данных в ячейке источника: " << TConstType::asStr(out_data_type) << std::endl;
	}
}

void NS_Tune::TCellFillType::setFillType(size_t type_code, const TColor& color_find, const TColor& color_nfind) noexcept(false)
{
	code = JsonCellFill(type_code);
	color_if_found = color_find;
	color_not_found = color_nfind;
}

void NS_Tune::TCellFillType::setFillType(const ptree::value_type& node, const JsonParams& code_tag,
	const JsonParams& color_find_tag, const JsonParams& color_not_found_tag)
{
	//считывание из json-дерева
	size_t v_type = TIndex(node, code_tag).get();
	TColor v_color_found = TIndex::getColorValue(node, color_find_tag);
	TColor v_color_no_found = TIndex::getColorValue(node, color_not_found_tag);
	//установка значений
	setFillType(v_type, v_color_found, v_color_no_found);
}

NS_Tune::TCellFillType& NS_Tune::TCellFillType::operator=(const TCellFillType& ftype) noexcept(false)
{
	code = ftype.code;
	color_if_found = ftype.color_if_found;
	color_not_found = ftype.color_not_found;
	return *this;
}

bool NS_Tune::TCellFillType::setByJsonNode(ptree& parent_node, const JsonParams& type_tag) noexcept(true)
{
	try
	{
		//получение тега объекта:
		string v_tag = TConstJson::asStr(type_tag);
		if (v_tag.empty()) return false;
		//получение дерева параметров объекта
		ptree::value_type v_node = parent_node.find(v_tag).dereference();
		//установка значений
		setFillType(v_node);
		return true;
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка получения данных из узла!", "TCellFillType::setByJsonNode").toErrBuff();
	}
	return false;
}

void NS_Tune::TCellFillType::show(std::ostream& stream) const noexcept(true)
{
	using std::endl;
	if (!stream) return;
	if (isEmpty())
		stream << "Не указан метод заливки!" << endl;
	else
		stream << "Метод заливки: " << code.toStr() << endl;
	if (isEmptyColorFind())
		stream << "Не указан цвет заливки при совпадении!" << endl;
	else
		stream << "Код цвета заливки при совпадении: " << color_if_found << endl;
	if (isEmptyColorNFind())
		stream << "Не указан цвет заливки, если данные не найдены!" << endl;
	else
		stream << "Код цвета заливки, если данные не найдены: " << color_not_found << endl;
}

bool NS_Tune::TCellFillType::isSuccess(size_t cnt, size_t fail) const noexcept(true)
{
	using NS_Const::JsonCellFill;
	switch (code.Value())
	{
	case JsonCellFill::CurCell: 
	case JsonCellFill::ID_All_Find:
		return cnt > 0 and fail == 0;
	case JsonCellFill::ID_More_One_Find:
	case JsonCellFill::ID_And_CurCell:
		return cnt > fail;
	}
	return true;
}

void NS_Tune::TCellMethod::setMethod(ptree::value_type& node, const JsonParams& code_tag) noexcept(true)
{
	try
	{
		code = TIndex(node, code_tag).get();
		if (code.isValid(true) == false) throw TLog("Указанный метод не обрабатывается!", "TCellMethod::setMethod");
		fill_type.setByJsonNode(node.second);
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
		code = JSonMeth::Null;
	}
	catch (...)
	{
		TLog("Не обработанная ошибка преобразования данных!", "TCellMethod::setMethod").toErrBuff();
		code = JSonMeth::Null;
	}
}

NS_Tune::TCellMethod::TCellMethod(ptree& parent_node, const JsonParams& parent_tag,
	const JsonParams& tag_meth) : code(JSonMeth::Null), fill_type()
{
	using NS_Const::TConstJson;
	string v_parent_tag = TConstJson::asStr(parent_tag);
	string v_tag = TConstJson::asStr(tag_meth);
	if (v_tag.empty()) return;
	ptree::value_type v_node = parent_node.find(v_tag).dereference();
	setMethod(v_node);
}

void NS_Tune::TCellMethod::show(std::ostream& stream) const noexcept(true)
{
	using std::endl;
	if (!stream) return;
	if (isEmpty())
		stream << "Метод обработки - пуст!" << endl;
	else
		stream << "Метод обработки: " << code.toStr() << endl;
	fill_type.show(stream);
}

void NS_Tune::TProcCell::InitSrcFile(ptree& node, const JsonParams& tag, const string& main_path) noexcept(true)
{
	using NS_Const::TConstJson;
	try
	{
		string v_node = TConstJson::asStr(tag);
		ptree::value_type tmp = node.find(v_node).dereference();
		if (tmp.second.empty()) throw TLog("Указанный узел: " + v_node + " не найден!", "TProcCell::InitSrcFile");
		DeInitSrcFile();
		SrcFile = new TShareData(tmp, main_path);
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
		DeInitSrcFile();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка инициализации файла-источника!", "TProcCell::InitSrcFile").toErrBuff();
		DeInitSrcFile();
	}
}

void NS_Tune::TProcCell::InitDBTune(const ptree& node, const TSimpleTune* tune_ref,
	const JsonParams& tag) noexcept(true)
{
	using NS_Const::TConstJson;
	using NS_Const::TuneField;
	using NS_Const::TConstField;
	//получение пути для конфигов:
	string conf_path = tune_ref->getFieldValueByCode(TuneField::MainPath);
	//полный путь к конфиг. файлам
	conf_path += TIndex::getStrValue(node, tag);
	//получение массива файлов настроек:
	StrArr files = TSimpleTune::getFileLst(conf_path);
	//создание массива настроек:
	for (const string& v : files)
	{
		TUserTune v_tune(*tune_ref, v);
		if (!v_tune.Empty()) db_tune.push_back(v_tune);
	}
}

void NS_Tune::TProcCell::InitCellData(ptree& node, const JsonParams& tag) noexcept(true)
{
	using NS_Const::TConstJson;
	if (node.empty()) return;
	string v_tag = TConstJson::asStr(tag);
	for (const ptree::value_type& js : node.get_child(v_tag))
	{
		if (js.second.empty()) continue;
		TCellData cd(js);
		if (cd.isEmpty()) continue;
		cel_arr.push_back(cd);
	}
}

void NS_Tune::TProcCell::InitByMethod(ptree& node, const TSimpleTune* tune_ref) noexcept(true)
{
	using NS_Const::JSonMeth;
	using NS_Const::JsonParams;
	try
	{
		if (meth.isSrcFileSection())
			InitSrcFile(node, JsonParams::SrcFile, tune_ref->getFieldValueByCode(TuneField::MainPath));
		else
		{
			if (tune_ref == nullptr) throw TLog("Не указана ссылка на родительский файл настроек!", "TProcCell::InitByMethod");
			InitDBTune(node, tune_ref);
		}
		//инициализаци ячеек
		InitCellData(node);
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка при инициализации из метода!", "TProcCell::InitByMethod").toErrBuff();
	}
}

NS_Tune::TProcCell::TProcCell(ptree& parent_node, const TSimpleTune* tune_ref):
	meth(parent_node), SrcFile(nullptr), db_tune(), cel_arr()
{
	if (meth.isEmpty()) return;
	InitByMethod(parent_node, tune_ref);
}

void NS_Tune::TProcCell::show(std::ostream& stream) const noexcept(true)
{
	using std::endl;
	if (!isEmpty())
	{
		stream << "Данные по методу обработки:" << endl;
		meth.show(stream);
		if (!NoSrcFile())
		{
			stream << "Данные по файлу источнику:" << endl;
			SrcFile->show(stream);
		}
		if (TuneCnt() > 0)
		{
			stream << "Конфигурационные файлы:" << endl;
			for (const TUserTune& t : db_tune)
			{
				t.show_tunes(stream);
				t.show_columns(stream);
				t.show_params(stream);
			}
		}
		if (CellCnt() > 0)
		{
			stream << "Данные об обрабатываемых ячейках: " << endl;
			for (const TCellData& cd : cel_arr)
				cd.show(stream);
		}
	}
}
void NS_Tune::TExcelProcData::DeInitDstFile() noexcept(true)
{
	if (DstFile) delete DstFile;
	DstFile = nullptr;
}

void NS_Tune::TExcelProcData::DeInitCells() noexcept(true)
{
	if (cells) delete cells;
	cells = nullptr;
}

void NS_Tune::TExcelProcData::InitDstFile(const ptree::value_type& node, const string& main_path) noexcept(true)
{
	if (node.second.empty()) return;
	DstFile = new TShareData(node, main_path);
	if (DstFile->isEmpty()) DeInitDstFile();
}

void NS_Tune::TExcelProcData::InitCells(ptree::value_type& node, const TSimpleTune* tune_ref) noexcept(true)
{
	if (node.second.empty() or tune_ref == nullptr) return;
	cells = new TProcCell(node.second, tune_ref);
	if (cells->isEmpty()) DeInitCells();
}

bool NS_Tune::TExcelProcData::InitObjByTag(ptree& json, const JsonParams& tag, const TSimpleTune* tune_ref) noexcept(false)
{
	using NS_Const::TConstJson;
	using NS_Const::JsonParams;
	if (json.empty()) throw TLog("Пустой json-файл!", "TExcelProcData::InitObjByTag");
	string v_tag = TConstJson::asStr(tag);
	ptree::value_type v_node = json.find(v_tag).dereference();
	if (v_node.second.empty()) throw TLog("Узел: " + v_tag + " не найден в json-файле!", "TExcelProcData::InitObjByTag");
	switch (tag)
	{
		case JsonParams::DstFile:
		{
			string v_path;
			if (tune_ref)
				v_path = tune_ref->getFieldValueByCode(TuneField::MainPath);
			InitDstFile(v_node, v_path);
			return !(isDstFileEmpty());
		}
		case JsonParams::Cells:
		{
			InitCells(v_node, tune_ref);
			return !(isCellsEmpty());
		}
	}
	return false;
}

void NS_Tune::TExcelProcData::InitExcelProcData(const string& json_file, const TSimpleTune* tune_ref) noexcept(true)
{
	using boost::property_tree::ptree;
	using boost::property_tree::file_parser_error;
	using boost::property_tree::json_parser_error;
	using boost::property_tree::json_parser::read_json;
	using NS_Const::JsonParams;
//	using filter_data = std::pair<size_t, string>;
//	using filters = std::vector<filter_data>;
	if (json_file.empty()) return;
	try
	{
		//инициализация json-файла:
		ptree js;
		read_json(json_file, js);
		if (js.empty()) throw TLog("Пустой json-файл: " + json_file + "!", "TExcelProcData::InitExcelProcData");
		//DstFile:
		if (!InitObjByTag(js, JsonParams::DstFile, tune_ref))
			throw TLog("Файл приемник не инициализирован!", "TExcelProcData::InitExcelProcData");
		//cells
		if (!InitObjByTag(js, JsonParams::Cells, tune_ref)) 
			throw TLog("Данные о ячейках не инициализированы!", "TExcelProcData::InitExcelProcData");
	}
	catch (const json_parser_error& err)
	{
		TLog(err.what()).toErrBuff();
	}
	catch (const std::exception& err)
	{
		TLog(err.what()).toErrBuff();
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка при инициализации объектов json!", "TExcelProcData::InitExcelProcData").toErrBuff();
	}
}

NS_Tune::TExcelProcData::TExcelProcData(const string& json_file, const TSimpleTune* tune_ref):
	DstFile(nullptr), cells(nullptr)
{
	InitExcelProcData(json_file, tune_ref);
}

NS_Tune::TExcelProcData::~TExcelProcData()
{
	DeInitDstFile();
	DeInitCells();
}

void NS_Tune::TExcelProcData::show(std::ostream& stream) const noexcept(true)
{
	using std::endl;
	if (!stream) return;
	if (!isDstFileEmpty())
	{
		stream << "Данные о файле приемнике: " << endl;
		DstFile->show(stream);
	}
	if (!isCellsEmpty())
	{
		stream << "Данные о ячейках: " << endl;
		cells->show();
	}
}

/**/