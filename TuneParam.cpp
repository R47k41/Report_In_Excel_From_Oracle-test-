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
#include "TConverter.cpp"


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
	case Types::Sql:
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
	case Types::Sql:
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
	case Types::Sql:
	{
		name = getFieldValueByCode(TuneField::SqlFile);
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

string NS_Tune::TSimpleTune::getFieldValueByCode(const TuneField& code, bool exit_on_er) const noexcept(true)
{
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

void NS_Tune::TSimpleTune::show_tunes(void) const
{
	using std::cout;
	using std::endl;
	if (Empty()) return;
	for (TStringParam x : fields)
		cout << x.toStr(false) << endl;
};

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

void NS_Tune::TUserTune::show_columns(void) const
{
	using std::cout;
	using std::endl;
	if (EmptyColumns()) return;
	for (string s : cols)
		cout << s << endl;
}

void NS_Tune::TUserTune::show_params(void) const
{
	using std::cout;
	using std::endl;
	if (EmptyParams()) return;
	for (TSubParam par : params)
		par.show();
}