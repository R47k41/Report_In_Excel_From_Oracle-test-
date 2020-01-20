#include <iostream>
#include <iterator>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "TuneParam.h"
#include "Logger.h"
#include "TConverter.hpp"


using std::string;
using std::vector;
using std::set;
using std::size_t;
using std::streampos;

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
	//����� ��������� � ������
	size_t pos = (src_data.*ff)(substr, beg);
	//���� ������ ������ �� ������� - �����
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
	//���� ����������� ����������� ��� ��������� ��������
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
	//���� ������ ��� ������� ���� - ������: �����
	if (TBaseParam::isEmpty() or param.isEmpty()) return string();
	//����� ��������� ���� ��������� � ������
	size_t pos = 0;
	pos = get_pos_in_src(param, pos);
	//���� ������ ������ �� ������� - �����
	if (pos == string::npos) return string();
	//���� �������� ���� � ������ �� �����������:
	return Get_Str_Val(pos, b_delimeter, e_delimeter, pose_from_end);
};

void NS_Tune::TStringParam::setValue(const TConstField&, const CtrlSym& open_val, const CtrlSym& close_val)
{
	value = Get_TuneField_Val(param, open_val, close_val, true);
}

NS_Tune::TStringParam::TStringParam(const TuneField& tune_field, const string& str, bool parse_val):
	TBaseParam(str), param(tune_field)
{
	//���� �������� ���������� �� ������:
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
	//���������� ������ ����: ParamName="ParamValue"
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
	//��� ������ ����� � ������ ��������:
	tmpType = LowerCase(tmpType);
	//��������� ������ �� ��� ������:
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
		cout << "������� �������� " << comment << ": ";
		std::getline(cin, value);
	}
	catch (...)
	{
		cerr << "������ ��� �������� ��������� " << comment << endl;
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
	//������� ��� �������� ������������ ���������� �����:
	bool no_ext = file_ext.empty();
	vector<string> result;
	try
	{
		//��������� ����� ����������:
		path dir(file_dir.c_str());
		//���� ��� ����������:
		if (is_directory(dir))
		{
			//�������� ������ ������
			for (directory_entry& x : directory_iterator(dir))
				if (is_regular_file(x) && (no_ext || extension(x) == file_ext))
					result.push_back(x.path().string());
		}
		else
		{
			//���������� ������� ����� � ������
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
	//��������� ������ �� ��������
	switch (code)
	{
	case Types::Config:
		name = getFieldValueByCode(TuneField::ConfigPath);
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
		//�������� �������� ��� �����
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
		NS_Logger::TLog("������ ��� ��������� ������ " + format + " ������!", "TSimpleTune::set_date_format");
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
		//��������� ������� ����
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
		NS_Logger::TLog log("�������������� ������ ���������� ���������: ", "TSimpleTune::AddField");
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
	//������������ ������� ������� ��������
	TuneRange range = getTuneRange(TRead::TuneVal);
	if (range.first == TuneField::Empty) return;
	field_arr = getTuneField4Range(range);
	//������ �����:
	while (file)
	{
		//���������� ������ ���������
		string str;
		getline(file, str);
		if (!std::isalpha(str[0]))
		{
			if (str[0] == GrpFlg)
				Read_Section(file, str);
			continue;
		}
		//������������ ������ �� set<TConstField> �� UserName �� Last
		//��������� ��������� �������� �� ������� � ������
		//���� ������ - ������� �������� �� ������� � ���������� � ������ ��������
		//��������� ���� �� � ������ ������ ������ ��������:
		for (const TConstField& i: field_arr)
			//���� �������� ���������� � ������:
			if (i.StrInclude(str))
			{
				//���������� ���������
				AddField(TStringParam(str, i.Value()));
				//������� �������� �� �������:
				field_arr.erase(i);
				break;
			}
	}
};

NS_Tune::TSharedTune::TSharedTune(const string& file, const string& code) :
	TSimpleTune(), main_code(code)
{
	ReadFromFile(file);
}

NS_Tune::TSharedTune::TSharedTune(const TFields& v, const string& file, const string& code) :
	TSimpleTune(v), main_code(code)
{
	ReadFromFile(file);
}

NS_Tune::TSharedTune::TSharedTune(const TSharedTune& v, const string& file, const string& code) :
	TSimpleTune(v), main_code(code) 
{
	ReadFromFile(file);
}

NS_Tune::TUserTune::TUserTune(const string& tunefile) : TSimpleTune()
{
	ReadFromFile(tunefile);
}

NS_Tune::TUserTune::TUserTune(const TSimpleTune& tune, const string& file) : TSimpleTune(tune)
{
	ReadFromFile(file);
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
	//���� ����� ��� � ��� ���� �������:
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
		throw NS_Logger::TLog("������ �������� �����: " + filename, "NS_Tune::TUserTune::ReadFromFile");
	//��������� �������� ��� ��������:
	Read_Tune_Val(file);
	//����������� ������ ��� �������:
	//Read_Param_Val(file);
	//��������� ������ ��� �������:
	//Read_Col_Val(file);
	//��������� ����
	file.close();
};

template <typename KeyType, typename ValType>
ValType& NS_Tune::TSimpleTune::getElementByID(const KeyType& par_ID, vector<ValType>& arr) noexcept(false)
{
	using NS_Logger::TLog;
	if (arr.empty()) throw TLog("������ �������� ����!", "NS_Tune::TUserTune::getElementByID");
	for (ValType& val: arr)
	{
		if (val > par_ID) break;
		if (val == par_ID) return val;
//		return ValType();
	}
	TLog log("������ �� �������!", "NS_Tune::TUserTune::getElementByID");
	//log << par_ID << TLog::NL;
	throw log;
}

template <typename KeyType, typename ValType>
const ValType& NS_Tune::TSimpleTune::getConstElementByID(const KeyType& par_ID, const vector<ValType>& arr) noexcept(false)
{
	using NS_Logger::TLog;
	if (arr.empty()) throw TLog("������ �������� ����!", "NS_Tune::TUserTune::getElementByID");
	for (const ValType& val : arr)
	{
		//if (val > par_ID) break;
		if (val == par_ID) return val;
		//		return ValType();
	}
	TLog log("������ �� �������!", "NS_Tune::TUserTune::getElementByID");
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
	//��������� ���� � �����
	string path = getPathByCode(code);
	if (only_path) return path;
	//��������� ����� �����:
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
		cerr << "������ ��������� ������ �� ����: " << TConstField::asStr(code) << endl;
		err.toErrBuff();
	}
	catch (...)
	{
		NS_Logger::TLog("�� ������������ ������ ��� ��������� ���������: " + TConstField::asStr(code), "NS_Tune::TUserTune::getFieldByCode").toErrBuff();
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
	//���� ���� � �������� �� ������
	if (!result.empty() and result[result.size() - 1] != TConstCtrlSym::asChr(CtrlSym::dash))
		//�������� ���������� �� �����:
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
	//���� �� ������ ��� �������� ���������:
	if (ID.empty()) return;
	//���� ������ ��� �������� ������:
	if (ID == code)
	{
		//��������� ������ ����������� ��������:
		TuneRange range = getTuneRange(TRead::Section);
		//��������� ������ ������� �����:
		RangeField filed_arr = getTuneField4Range(range);
		//���������� ���������� ���������� �� ������� [
		char block_end = TConstCtrlSym::asChr(CtrlSym::qlbkt);
		//������ �����:
		while (file)
		{
			//���������� ������ �������� ���������
			string str;
			getline(file, str);
			//���������� ����������
			if (!std::isalpha(str[0]))
			{
				//����� ���� ����� �� ����� ����� - �����
				if (block_end == str[0])
					break;
				else
					continue;
			}
			//��������� ��������� �� ������ ������ � ���������:
			for (const TConstField& f : filed_arr)
			{
				//���� ������ ��������� � ����������:
				if (f.StrInclude(str))
				{
					//��������� ��������:
					TStringParam new_param(f.Value(), str);
					//��������� ������� �� ��� ��� � ����������:
					try
					{
						//������ �������� ���������
						TStringParam& tmp = getElementByID(f.Value(), fields);
						tmp.Value(new_param.Value());
					}
					catch (...)
					{
						//���� �� ����� - ���������
						fields.push_back(new_param);
					}
					break;
				}
			}
		}
	}
	skip_block(file, TConstField::asStr(TuneField::Block_End));
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
		else//���� ��� �� ������� - �������
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
	//��������� ��������� ����������� ����������
	TuneRange range = getTuneRange(TSimpleTune::TRead::Section);
	TuneField ID = TConstField::getIDByCode(code, range.first, range.second);
	//���� ������ �� ������� � ����� - �����
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
	//��������� ������ � ������ � �����(���� ����� �������� ���������� ������ - ��������� � switch):
	FileParam param = getFileParamByCode(Types::SQL);
	//��������� �������������� �����:
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
			throw TLog("��������� ��� �� ��������������!", "getStrValue");
		//���� ������ � ����� ������ ��� ��� �� ��������������:
		if (parent_node.empty())
			throw TLog("������ ���������� � JSon-�����!", "getStrValue");
		//��������� ������ � uncode-���������:
		string val = parent_node.get_child(v_tag).get_value<string>();
		if (UTF8ToANSI(val)) return val;
	}
	catch (TLog& er)
	{
		er << "(���: " << v_tag << ")\n";
		er.toErrBuff();
	}
	catch (...)
	{
		TLog log("�� ������������ ������ ��������� ������!", "getStrValue");
		log << "\n(���: " << v_tag << ")\n";
		log.toErrBuff();
	}
	return string();
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
		if (!TConstJson::isTag(tag)) throw TLog("��������� ���: " + tmp + " �� ��������������!", "setByJSon");
		setIndexFromJSon(parent_node, tmp);
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ��� ��������� �������� ������� �� ����: " + tmp, "setByJSon").toErrBuff();
	}
}

void NS_Tune::TIndex::show(std::ostream& stream, const string& front_msg) const noexcept(false)
{
	using std::endl;
	if (!stream) return;
	stream << front_msg;
	if (isEmpty())
		stream << "������ ��������";
	else
		stream << index;
	stream << endl;
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
		col_indx.show(stream, "������ ������� ��� ����������: ");
		stream << "�������� �������: " << value << std::endl;
	}
}

void NS_Tune::TShareData::setData(const ptree::value_type& parent_node,
	const JsonParams& name_tag, const JsonParams& lst_tag, 
	const JsonParams& strt_row_tag, const JsonParams& last_row_tag,
	const JsonParams& fltr_tag) noexcept(true)
{
	using NS_Const::TConstJson;
	if (parent_node.second.empty()) return;
	//������������� ����������
	name = TIndex::getStrValue(parent_node, name_tag);
	lst_indx.setByJSon(parent_node, lst_tag);
	strt_row_indx.setByJSon(parent_node, strt_row_tag);
	last_row_indx.setByJSon(parent_node, last_row_tag);
	//������������� ������� ��������:
	string nodeStr = TConstJson::asStr(fltr_tag);
	for (const ptree::value_type& child : parent_node.second.get_child(nodeStr))
	{
		//���� ��� ������
		if (child.second.empty()) continue;
		TFilterData tmp(child);
		if (!tmp.isEmpty())
			fltr.push_back(tmp);
	}
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
		stream << "������������ �����: " << name << endl;
		lst_indx.show(stream, "������ ��������������� �����: ");
		if (!NoStartRowIndex())
			strt_row_indx.show(stream, "��������� ������: ");
		if (!NoLastRowIndex())
			last_row_indx.show(stream, "�������� ������: ");
		if (!fltr.empty())
		{
			stream << "������ ��� ����������: " << endl;
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

NS_Tune::TCellData& NS_Tune::TCellData::setData(const ptree::value_type& parent_node,
	const JsonParams& dst_tag, const JsonParams& dst_ins_tag,
	const JsonParams& src_param_tag, const JsonParams& src_val_tag) noexcept(true)
{
	dst_indx.setByJSon(parent_node, dst_tag);
	dst_ins_indx.setByJSon(parent_node, dst_ins_tag);
	src_param_indx.setByJSon(parent_node, src_param_tag);
	src_val_indx.setByJSon(parent_node, src_val_tag);
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
	}
	return *this;
}

void NS_Tune::TCellData::show(std::ostream& stream) const noexcept(true)
{
	if (stream)
	{
		dst_indx.show(stream, "������ ������ ���������: ");
		dst_ins_indx.show(stream, "������ ������ ��� ������� ������ � ���������: ");
		src_param_indx.show(stream, "������-�������� � ���������: ");
		src_val_indx.show(stream, "������-�������� � ���������: ");
	}
}

void NS_Tune::TCellMethod::setMethod(size_t meth, size_t find_color, size_t not_find_color) noexcept(true)
{
	using NS_Const::TConstJson;
	using NS_Const::JSonMeth;
	try
	{
		code = JSonMeth(meth);
		if (code >= JSonMeth::Last) throw TLog("��������� ����� �� ��������������!", "TCellMethod::setMethod");
		color_if_found = TColor(find_color);
		color_not_found = TColor(not_find_color);
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ �������������� ������!", "TCellMethod::setMethod").toErrBuff();
	}
}

void NS_Tune::TCellMethod::setMethod(const ptree::value_type& node, const JsonParams& code_tag,	
	const JsonParams& color_find_tag, const JsonParams& color_not_found_tag)
{
	size_t v_code = TIndex(node, code_tag).get();
	size_t v_color_found = TIndex(node, color_find_tag).get();
	size_t v_color_no_found = TIndex(node, color_not_found_tag).get();
	setMethod(v_code, v_color_found, v_color_no_found);
}

NS_Tune::TCellMethod::TCellMethod(ptree& parent_node, const JsonParams& tag_meth) :
	code(JSonMeth::Null), color_if_found(TColor::COLOR_WHITE), color_not_found(TColor::COLOR_WHITE)
{
	using NS_Const::TConstJson;
	string v_tag = TConstJson::asStr(tag_meth);
	if (v_tag.empty()) return;
	ptree::value_type v_node = parent_node.find(v_tag).dereference();
	setMethod(v_node);
}


void NS_Tune::TCellMethod::show(std::ostream& stream) const noexcept(true)
{
	using std::endl;
	using NS_Const::TConstJSMeth;
	if (!stream) return;
	if (isEmpty()) stream << "����� ��������� - ����!" << endl;
	stream << "��� ������ ���������: " << TConstJSMeth::asStr(code) << endl;
	if (!isEmptyIncludeColor())
		stream << "���� ��� ���������� �������: " << color_if_found << endl;
	if (!isEmptyExcludeColor())
		stream << "���� ��� �� ���������� �������: " << color_not_found << endl;
}

void NS_Tune::TProcCell::InitSrcFile(ptree& node, const JsonParams& tag) noexcept(true)
{
	using NS_Const::TConstJson;
	try
	{
		string v_node = TConstJson::asStr(tag);
		ptree::value_type tmp = node.find(v_node).dereference();
		if (tmp.second.empty()) throw TLog("��������� ����: " + v_node + " �� ������!", "TProcCell::InitSrcFile");
		DeInitSrcFile();
		SrcFile = new TShareData(tmp);
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
		DeInitSrcFile();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ������������� �����-���������!", "TProcCell::InitSrcFile").toErrBuff();
		DeInitSrcFile();
	}
}

void NS_Tune::TProcCell::InitDBTune(const ptree& node, string conf_path,
	const JsonParams& tag) noexcept(true)
{
	using NS_Const::TConstJson;
	using NS_Const::TuneField;
	using NS_Const::TConstField;
	//��������� ���� ��� ��������:
	conf_path += TIndex::getStrValue(node, tag);
	//��������� ������� ������ ��������:
	StrArr files = TSimpleTune::getFileLst(conf_path);
	//�������� ������� ��������:
	for (const string& v : files)
	{
		TUserTune v_tune(v);
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

void NS_Tune::TProcCell::InitByMethod(ptree& node, const string& conf_path) noexcept(true)
{
	using NS_Const::JSonMeth;
	try
	{
		switch (meth.getMethod())
		{
		case JSonMeth::CompareColor:
		case JSonMeth::CompareIns:
		{
			InitSrcFile(node);
			break;
		}
		case JSonMeth::GetFromDB:
		case JSonMeth::SendToDB:
		{
			if (conf_path.empty()) throw TLog("�� ������ ���� � �������� ����� ��������!", "TProcCell::InitByMethod");
			InitDBTune(node, conf_path);
			break;
		}
		}
		//������������ �����
		InitCellData(node);
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ��� ������������� �� ������!", "TProcCell::InitByMethod").toErrBuff();
	}
}

NS_Tune::TProcCell::TProcCell(ptree& parent_node, const string& main_path):
	meth(parent_node), SrcFile(nullptr), db_tune(), cel_arr()
{
	if (meth.isEmpty()) return;
	InitByMethod(parent_node, main_path);
}

void NS_Tune::TProcCell::show(std::ostream& stream) const noexcept(true)
{
	using std::endl;
	if (!isEmpty())
	{
		stream << "������ �� ������ ���������:" << endl;
		meth.show(stream);
		if (!NoSrcFile())
		{
			stream << "������ �� ����� ���������:" << endl;
			SrcFile->show(stream);
		}
		if (TuneCnt() > 0)
		{
			stream << "���������������� �����:" << endl;
			for (const TUserTune& t : db_tune)
			{
				t.show_tunes(stream);
				t.show_columns(stream);
				t.show_params(stream);
			}
		}
		if (CellCnt() > 0)
		{
			stream << "������ �� �������������� �������: " << endl;
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

void NS_Tune::TExcelProcData::InitCells(ptree::value_type& node, const string& main_path) noexcept(true)
{
	if (node.second.empty() or main_path.empty()) return;
	cells = new TProcCell(node.second, main_path);
	if (cells->isEmpty()) DeInitCells();
}

bool NS_Tune::TExcelProcData::InitObjByTag(ptree& json, const JsonParams& tag, const string& mpath) noexcept(false)
{
	using NS_Const::TConstJson;
	using NS_Const::JsonParams;
	if (json.empty()) throw TLog("������ json-����!", "TExcelProcData::InitObjByTag");
	string v_tag = TConstJson::asStr(tag);
	ptree::value_type v_node = json.find(v_tag).dereference();
	if (v_node.second.empty()) throw TLog("����: " + v_tag + " �� ������ � json-�����!", "TExcelProcData::InitObjByTag");
	switch (tag)
	{
		case JsonParams::DstFile:
			InitDstFile(v_node, mpath);
			return !(isDstFileEmpty());
		case JsonParams::Cells:
		{
			InitCells(v_node, mpath);
			return !(isCellsEmpty());
		}
	}
	return false;
}

void NS_Tune::TExcelProcData::InitExcelProcData(const string& json_file, const string& main_path) noexcept(true)
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
		//������������� json-�����:
		ptree js;
		read_json(json_file, js);
		if (js.empty()) throw TLog("������ json-����: " + json_file + "!", "TExcelProcData::InitExcelProcData");
		//DstFile:
		if (!InitObjByTag(js, JsonParams::DstFile, main_path))
			throw TLog("���� �������� �� ���������������!", "TExcelProcData::InitExcelProcData");
		//cells
		if (!InitObjByTag(js, JsonParams::Cells, main_path)) 
			throw TLog("������ � ������� �� ����������������!", "TExcelProcData::InitExcelProcData");
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
		TLog("�� ������������ ������ ��� ������������� �������� json!", "TExcelProcData::InitExcelProcData").toErrBuff();
	}
}

NS_Tune::TExcelProcData::TExcelProcData(const string& json_file, const string& main_path): 
	DstFile(nullptr), cells(nullptr)
{
	InitExcelProcData(json_file, main_path);
}

NS_Tune::TExcelProcData::TExcelProcData(const TSharedTune& tune): DstFile(nullptr), cells(nullptr)
{
	string v_file = tune.getConfigPath();
	string v_path = tune.getMainPathVal();
	InitExcelProcData(v_file, v_path);
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
		stream << "������ � ����� ���������: " << endl;
		DstFile->show(stream);
	}
	if (!isCellsEmpty())
	{
		stream << "������ � �������: " << endl;
		cells->show();
	}
}
/**/