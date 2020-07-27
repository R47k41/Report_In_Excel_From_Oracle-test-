#include <iostream>
#include <iterator>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include "TuneParam.h"
#include "TSQLParser.h"
#include "Logger.hpp"

using std::string;
using std::vector;
using std::set;
using std::size_t;
using std::streampos;
using NS_Logger::TLog;

template void NS_Tune::TShareData::setArrayByJson<NS_Tune::TSheetData>(const ptree::value_type& node, const JsonParams& tag,
	vector<NS_Tune::TSheetData>& arr);

template void NS_Tune::TShareData::setArrayByJson<NS_Tune::TFilterData>(const ptree::value_type& node, const JsonParams& tag,
	vector<NS_Tune::TFilterData>& arr);

template string NS_Tune::TUserTune::getSqlText<NS_Sql::TText>(const NS_Tune::TNotNullRslt& data) noexcept(false);

template string NS_Tune::TUserTune::getSqlText<NS_Sql::TSimpleSql>(const NS_Tune::TNotNullRslt& data) noexcept(false);


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

string NS_Tune::TSimpleTune::getOnlyName(const string& file) noexcept(true)
{
	using boost::filesystem::path;
	using boost::filesystem::is_directory;
	path tmp(file);
	//���� ��������� ���� - ���������� - �����
	if (is_directory(tmp)) return string();
	return tmp.filename().string();
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
	case Types::SubConfig:
		result.second = getFieldValueByCode(TuneField::SubTuneFileExt);
		break;
	case Types::SQL:
	case Types::DQL:
	case Types::DML:
	case Types::ClearSQL:
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

NS_Tune::TNotNullRslt NS_Tune::TSimpleTune::CoalesceKeys(const NS_Const::TuneField& key1,
	const NS_Const::TuneField& key2) const noexcept(true)
{
	using NS_Const::TConstField;
	TNotNullRslt result(string(), true);
	try
	{
		result.first = getFieldValueByCode(key1);
		if (result.first.empty())
		{
			result.first = getFieldValueByCode(key2);
			result.second = false;
		}
		return result;
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("�� ������������ ������ ��������� �� ������� �������� ����� �������: ", "TSimpleTune::CoalesceKeys");
		log << TConstField::asStr(key1) << " � " << TConstField::asStr(key2) << '\n';
		log.toErrBuff();
	}
	return result;
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


string NS_Tune::TSimpleTune::AddCurDate2Name(const Types& code) const noexcept(false)
{
	using NS_Const::DateInteface::cur_date_to_string_by_format;
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
	//������������� ���� ������:
	if (main_code.empty()) main_code = getCodeFromtUI();
	//������������� �����:
	TSimpleTune::Initialize(file);
}

NS_Tune::TSharedTune::TSharedTune(const TFields& v, const string& file, const string& code) :
	TSimpleTune(v), main_code(code)
{
	//������������� ���� ������:
	if (main_code.empty()) main_code = getCodeFromtUI();
	//������������� �����:
	TSimpleTune::Initialize(file);
}

NS_Tune::TSharedTune::TSharedTune(const TSharedTune& v, const string& file, const string& code) :
	TSimpleTune(v), main_code(code) 
{
	//������������� ���� ������:
	if (main_code.empty()) main_code = getCodeFromtUI();
	//������������� �����:
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
		//���� �� ��������� ���. ��������� � ����� - �����
		if (sub_name.empty()) return name;
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
		cerr << "�� ����: " << TConstField::asStr(code) << " ������ �� �������!" << endl;
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
		cout << endl << "������� ��� ������:\t";
		cin >> val;
		report = val;
	} while (report.isEmpty() or !report.isValid(true));
	//������� ����� �� ������
	while (!cin.get());
	//���� ����� �����
	if (report == ReportCode::QUIT_REPORT)
		cout << "����� �� ���������!" << endl;
	else
		cout << "����� ����������� �����: " << report.getName() << endl;
	//�������
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

void NS_Tune::TSimpleTune::Initialize(const string& file) noexcept(true)
{
	using std::cout;
	using std::endl;
	using std::cin;
	using std::getline;
	try
	{
		string file_name;
		//���� ������ ��� �����:
		if (file.empty())
		{
			//�������� ��� ����� �� ������������
			do
			{
				cout << "���������� ������� ��� ����� ��������:" << endl;
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
		TLog("�� ����������� ������ ��� ��������� �������� �� �����: " + file + '\n', "TSharedTune::Initialize").toErrBuff();
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

template <typename T>
string NS_Tune::TUserTune::getSqlText(const TNotNullRslt& data) noexcept(false)
{
	using std::ifstream;
	//��������� ������ �������� sql-�����: ���� ��� ������
	if (!data.second)
	{
		T sql(data.first);
		return NS_Sql::AsString(sql);
	}
	else
	{
		ifstream sql_txt_file(data.first, std::ios_base::in);
		if (!sql_txt_file.is_open())
		{
			throw TLog("������ �������� �����: " + data.first, "TUserTune::getSqlText");
		}
		T sql(sql_txt_file);
		sql_txt_file.close();
		return NS_Sql::AsString(sql);
	}
	return string();
}

string NS_Tune::TUserTune::getSqlString(bool parsingFlg, bool byStrFlg, const string& str) noexcept(true)
{
	try
	{
		if (str.empty()) throw TLog("������ ������ �������!", "TUserTune::getSqlString");
		TNotNullRslt tmp(str, !byStrFlg);
		if (parsingFlg)
			return getSqlText<NS_Sql::TText>(tmp);
		else
			return getSqlText<NS_Sql::TSimpleSql>(tmp);
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ��������� sql-������� �� ������: " + str, "TUserTune::getSqlString").toErrBuff();
	}
	return string();
}

NS_Tune::TNotNullRslt NS_Tune::TUserTune::checkLines(const Types& type_code) const noexcept(true)
{
	using NS_Const::TuneField;
	using NS_Tune::TNotNullRslt;
	TNotNullRslt tmp(string(), true);
	switch (type_code)
	{
	case Types::DQL:
	{
		tmp = CoalesceKeys(TuneField::SqlFile, TuneField::SqlText);
		break;
	}
	case Types::DML:
	{
		tmp = CoalesceKeys(TuneField::DMLFile, TuneField::DMLText);
		break;
	}
	case Types::ClearSQL:
	{
		tmp = CoalesceKeys(TuneField::ClearSQLFile, TuneField::ClearSLQText);
		break;
	}
	default:
		throw TLog("��������� ��� �� ��������� � SQL ��������!", "TUserTune::getSqlText");
	}
	return tmp;
}

string NS_Tune::TUserTune::getSqlByParsingFlg(const TNotNullRslt& param, bool parsing) const noexcept(false)
{
	if (parsing)
		return getSqlText<NS_Sql::TText>(param);
	else
		return getSqlText<NS_Sql::TSimpleSql>(param);
}

string NS_Tune::TUserTune::getSqlStr(const Types& type_code) const noexcept(true)
{
	using NS_Const::TuneField;
	using NS_Tune::TNotNullRslt;
	try
	{
		TNotNullRslt tmp = checkLines(type_code);
		//���� ��������� ��� �����:
		if (!tmp.first.empty())
		{
			//��������� ����� ��������:
			bool use_parsing = useFlag(TuneField::UseSqlParser);
			return getSqlByParsingFlg(tmp, use_parsing);
		}
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ��������� sql-�������!", "TUserTune::getSqlText").toErrBuff();
	}
	return string();
}

vector<string> NS_Tune::TUserTune::getSQLList(const Types& kind, bool use_sort) const noexcept(false)
{
	StrArr lst;
	//���� ������ �������� �� �������� - �����
	if (isEmptyByCode(kind)) return lst;
	//��������� ����� �� ����� ���������:
	TNotNullRslt SqlField = checkLines(kind);
	//���� ������ ��� sql-���� �� �������� - �����
	if (SqlField.first.empty()) return lst;
	//��������� ��������� ��������:
	bool use_parsing = useFlag(TuneField::UseSqlParser);
	//��������� ������ �� ����/���� ��� ������:
	if (SqlField.second == false)
	{
		//��������� ������:
		string sql = getSqlByParsingFlg(SqlField, use_parsing);
		if (sql.empty() == false)
			//���� ������� ������:
			lst.push_back(sql);
	}
	else
	{
		//��������� ���������� ���������� ������
		FileParam param = getFileParamByCode(kind);
		param.first += SqlField.first;
		//���� ������ ����/���� - ��������� ������ ������:
		StrArr fileLst = getFileLst(param.first, param.second, use_sort);
		//�������� �� ������� ����� � �������� ����� ��� sql-�������:
		for (const string& file : fileLst)
		{
			SqlField.first = file;
			string sql = getSqlByParsingFlg(SqlField, use_parsing);
			if (sql.empty()) continue;
			lst.push_back(sql);
		}
	}
	return lst;
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
	case Types::ClearSQL:
		param.first += getFieldValueByCode(TuneField::ClearSQLFile);
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
	case Types::ClearSQL:
		return getFieldValueByCode(TuneField::ClearSQLFile).empty() && getFieldValueByCode(TuneField::ClearSLQText).empty();

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
		string val = parent_node.get_child(v_tag).get_value<string>("");
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
			throw TLog("��������� ��� �� ��������������!", "getStrValue");
		//���� ������ � ����� ������ ��� ��� �� ��������������:
		if (parent_node.second.empty())
			throw TLog("������ ���������� � JSon-�����!", "getStrValue");
		//��������� ������ � uncode-���������:
		int val = parent_node.second.get_child(v_tag).get_value<int>();
		if (val == EmptyType)
			return TColor::COLOR_NONE;
		else
			return TColor(val);
	}
	catch (const json_parser_error& err)
	{
		TLog("������ ��������� ����� ��� ����: " + v_tag + '\n' + err.what(), "TIndex::getColorValue").toErrBuff();
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
	return TColor::COLOR_NONE;
}

NS_Tune::TColor NS_Tune::TIndex::getColorValue(const ptree& parent_node, const JsonParams& tag) noexcept(true)
{
	using NS_Const::TConstJson;
	using NS_Const::EmptyType;
	using boost::property_tree::json_parser_error;
	string v_tag = TConstJson::asStr(tag);
	try
	{
		if (!TConstJson::isTag(tag))
			throw TLog("��������� ��� �� ��������������!", "getStrValue");
		//���� ������ � ����� ������ ��� ��� �� ��������������:
		if (parent_node.empty())
			throw TLog("������ ���������� � JSon-�����!", "getStrValue");
		//��������� ������ � uncode-���������:
		int val = parent_node.get_child(v_tag).get_value<int>();
		if (val == EmptyType)
			return TColor::COLOR_NONE;
		else
			return TColor(val);
	}
	catch (const json_parser_error& err)
	{
		TLog("������ ��������� ����� ��� ����: " + v_tag + '\n' + err.what(), "TIndex::getColorValue").toErrBuff();
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
	return TColor::COLOR_NONE;
}

NS_Const::JsonFilterOper NS_Tune::TIndex::getOperationCode(const ptree::value_type& parent_node, 
	const NS_Const::JsonParams& tag) noexcept(true)
{
	using NS_Const::JsonFilterOper;
	using NS_Const::TConstJson;
	using NS_Const::EmptyType;
	using boost::property_tree::json_parser_error;
	string v_tag = TConstJson::asStr(tag);
	try
	{
		if (v_tag.empty())
			throw TLog("�� ������� ���������� ������������� ��� ����!", "TIndex::getOperationCode");
		//��������� ����:
		if (TConstJson::isTag(tag) == false)
		{
			throw TLog("��������� ��� �� ��������������!");
		}
		//���� ������ � ����� ������ ��� ��� �� ��������������:
		if (parent_node.second.empty())
			throw TLog("������ ���������� � JSon-�����!", "TIndex::getOperationCode");
		//��������� ������:
		int val = parent_node.second.get_child(v_tag).get_value<int>();
		return JsonFilterOper(val);

	}
	catch (const json_parser_error& err)
	{
		TLog log("������ �������� json-�����: ", "TIndex::getOperationCode");
		log << err.what() << '\n';
		log.toErrBuff();
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ��� ��������� ���� �������� �� json-����� ��� ����: " + v_tag, 
			"TIndex::getOperationCode").toErrBuff();
	}
	return JsonFilterOper::Null;
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
	//������� ������:
	index.show(stream, "������ ����� ��������: ");
	if (!NoColID())
		col_id.show(stream, "������ �������-��������������: ");
	if (!NoFirstRowIndex())
		first_row.show(stream, "������ ������ ������: ");
	if (!NoLastRowIndex())
		last_row.show(stream, "������ ��������� ������: ");
}

void NS_Tune::TFilterData::setStrFlg() noexcept(true)
{
	using NS_Const::getNLSNumPoint;
	if (isNumberValue(getNLSNumPoint()) == true)
		str_flg = false;
	else
		str_flg = true;
}


void NS_Tune::TFilterData::setData(const ptree::value_type& parent_node,
	const JsonParams& col_tag, const JsonParams& oper_tag, const JsonParams& val_tag) noexcept(true)
{
	col_indx.setByJSon(parent_node, col_tag);
	operation = TIndex::getOperationCode(parent_node, oper_tag);
	value = TIndex::getStrValue(parent_node, val_tag);
	setStrFlg();
}

bool NS_Tune::TFilterData::isNumberValue(const char separate) const noexcept(true)
{
	//using std::isdigit;
	try
	{
		if (value.empty()) return false;
		size_t i = 0;
		//���� ������ ������ �� ����� - �����
		for (const unsigned char& ch: value)
		{
			//���� ���� �� ���� ������ �� ����� - ��� ������
			if (isdigit(ch) or ch == separate) continue;
			return false;
		}
		return true;
	}
	catch (const std::exception& err)
	{
		TLog log("������ ����������� ��������� ��������: ", "TFilterData::isNumberValue");
		log << value << " � �����:\n" << err.what() << '\n';
		log.toErrBuff();
	}
	catch (...)
	{
		TLog("������ ����������� ��������� ��������: " + value + " � �����!", "TFilterData::isNumberValue").toErrBuff();
	}
	return false;
}

void NS_Tune::TFilterData::show(std::ostream& stream) const noexcept(true)
{
	if (stream and !isEmpty())
	{
		col_indx.show(stream, "������ ������� ��� ����������: ");
		stream << "�������� ���������: " << getOperationName() << std::endl;
		stream << "�������� �������: " << value << std::endl;
	}
}

bool NS_Tune::TFilterData::isFiltredBoolValue(const bool& val) const noexcept(true)
{
	using NS_Const::TConstJSFilterOper;
	using NS_Converter::toBoolType;

	//�������������� �������� �� ������� � �������� �� ���:
	bool tmp = false;
	if (NS_Converter::toBoolType(value, &tmp) == false)
		return false;
	return TConstJSFilterOper::BoolBaseOperation(val, tmp, operation.Value());
}

bool NS_Tune::TFilterData::isFiltredDblValue(const double& val) const noexcept(true)
{
	using NS_Const::TConstJSFilterOper;
	using NS_Const::JsonFilterOper;
	using NS_Converter::toDblType;
	//�������������� �������� �� ������� � �������� �� ���:
	double tmp = 0;
	if (NS_Converter::toDblType(value, &tmp) == false)
		return false;
	return TConstJSFilterOper::DoubleBaseOperation(val, tmp, operation.Value());
}

bool NS_Tune::TFilterData::isFiltredIntValue(const int& val) const noexcept(true)
{
	using NS_Const::TConstJSFilterOper;
	//�������������� �������� �� ������� � �������� �� ���:
	int tmp = 0;
	if (NS_Converter::toType(value, &tmp) == false)
		return false;
	return TConstJSFilterOper::IntBaseOperation(val, tmp, operation.Value());
}


bool NS_Tune::TFilterData::isFiltredStrValue(const std::string& val) const noexcept(true)
{
	using NS_Const::TConstJSFilterOper;
	//���� ����� ������� ���������� ��������
	if (str_flg)
		return TConstJSFilterOper::StringBaseOperation(val, value, operation.Value());
	else
		return false;
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
	//������������� ����������
	name = TIndex::getStrValue(parent_node, name_tag);
	//��������� ���������� ��� ������:
	setArrayByJson<TSheetData>(parent_node, sheet_tag, sh_params);
	//������������� ������� ��������:
	setArrayByJson<TFilterData>(parent_node, fltr_tag, fltr);
}

NS_Tune::TShareData::TShareData(ptree& main_node, const string& main_path, const JsonParams& src_tag)
{
	using boost::property_tree::ptree;
	using NS_Const::JsonParams;
	using NS_Const::TConstJson;
	string tag = TConstJson::asStr(src_tag);
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
		stream << "������������ �����: " << name << endl;
		for (size_t i = 0; i < sh_params.size(); i++)
		{
			stream << "��������� " << i << " ��������:" << endl;
			sh_params[i].show();
		}
			
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
		dst_indx.show(stream, "������ ������ ���������: ");
		dst_ins_indx.show(stream, "������ ������ ��� ������� ������ � ���������: ");
		src_param_indx.show(stream, "������-�������� � ���������: ");
		src_val_indx.show(stream, "������-�������� � ���������: ");
		if (in_data_type != DataType::ErrorType)
			stream << "��� ������ � ������ ���������: " << TConstType::asStr(in_data_type) << std::endl;
		if (out_data_type != DataType::ErrorType)
			stream << "��� ������ � ������ ���������: " << TConstType::asStr(out_data_type) << std::endl;
	}
}

NS_Tune::TCellData NS_Tune::TCellData::Inverse_Src2Dst(void) const noexcept(true)
{
	return TCellData(SrcParam(), SrcVal(), DstIndex(), InsIndex(), getOutType(), getInType());
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
	//���������� �� json-������
	size_t v_type = TIndex(node, code_tag).get();
	TColor v_color_found = TIndex::getColorValue(node, color_find_tag);
	TColor v_color_no_found = TIndex::getColorValue(node, color_not_found_tag);
	//��������� ��������
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
		//��������� ���� �������:
		string v_tag = TConstJson::asStr(type_tag);
		if (v_tag.empty()) return false;
		//��������� ������ ���������� �������
		ptree::value_type v_node = parent_node.find(v_tag).dereference();
		//��������� ��������
		setFillType(v_node);
		return true;
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ��������� ������ �� ����!", "TCellFillType::setByJsonNode").toErrBuff();
	}
	return false;
}

void NS_Tune::TCellFillType::show(std::ostream& stream) const noexcept(true)
{
	using std::endl;
	if (!stream) return;
	if (isEmpty())
		stream << "�� ������ ����� �������!" << endl;
	else
		stream << "����� �������: " << code.toStr() << endl;
	if (isEmptyColorFind())
		stream << "�� ������ ���� ������� ��� ����������!" << endl;
	else
		stream << "��� ����� ������� ��� ����������: " << color_if_found << endl;
	if (isEmptyColorNFind())
		stream << "�� ������ ���� �������, ���� ������ �� �������!" << endl;
	else
		stream << "��� ����� �������, ���� ������ �� �������: " << color_not_found << endl;
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

bool NS_Tune::TCellFillType::useFont() const noexcept(true)
{
	using NS_Const::JsonCellFill;
	switch (code.Value())
	{
		case JsonCellFill::CurCell:
		case JsonCellFill::ID_And_CurCell:
			return true;
	}
	return false;
}

void NS_Tune::TCellMethod::setMethod(ptree::value_type& node, const JsonParams& code_tag) noexcept(true)
{
	try
	{
		code = TIndex(node, code_tag).get();
		if (code.isValid(true) == false) throw TLog("��������� ����� �� ��������������!", "TCellMethod::setMethod");
		fill_type.setByJsonNode(node.second);
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
		code = JSonMeth::Null;
	}
	catch (...)
	{
		TLog("�� ������������ ������ �������������� ������!", "TCellMethod::setMethod").toErrBuff();
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
		stream << "����� ��������� - ����!" << endl;
	else
		stream << "����� ���������: " << code.toStr() << endl;
	fill_type.show(stream);
}

bool NS_Tune::TCellMethod::isSuccess(size_t cnt, size_t fail) const noexcept(true)
{ 
	using NS_Const::JSonMeth;
	switch (code.Value())
	{
		case JSonMeth::CompareCellChange:
//			return cnt > fail;
		case JSonMeth::CompareCell:
		case JSonMeth::CompareRow:
		case JSonMeth::InsertRowCompare:
			return fill_type.isSuccess(cnt, fail);
	}
	return false;
}

void NS_Tune::setCellDataArrByNode(CellDataArr& arr, ptree& node,
	const NS_Const::JsonParams& tag) noexcept(true)
{
	using NS_Const::TConstJson;
	using NS_Tune::TCellData;
	if (node.empty()) return;
	string v_tag = TConstJson::asStr(tag);
	for (const ptree::value_type& js : node.get_child(v_tag))
	{
		if (js.second.empty()) continue;
		TCellData cd(js);
		if (cd.isEmpty()) continue;
		arr.push_back(cd);
	}
}

void NS_Tune::showCellDataArr(const CellDataArr& arr, std::ostream& stream) noexcept(true)
{
	using NS_Tune::TCellData;
	if (arr.empty() or !stream) return;
	for (const TCellData& param : arr)
		param.show(stream);
}

void NS_Tune::TProcCell::InitSrcFile(ptree& node, const JsonParams& tag, const string& main_path) noexcept(true)
{
	using NS_Const::TConstJson;
	try
	{
		string v_node = TConstJson::asStr(tag);
		ptree::value_type tmp = node.find(v_node).dereference();
		if (tmp.second.empty()) throw TLog("��������� ����: " + v_node + " �� ������!", "TProcCell::InitSrcFile");
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
		TLog("�� ������������ ������ ������������� �����-���������!", "TProcCell::InitSrcFile").toErrBuff();
		DeInitSrcFile();
	}
}

void NS_Tune::TProcCell::InitDBTune(const ptree& node, const TSimpleTune* tune_ref,
	const JsonParams& tag) noexcept(true)
{
	using NS_Const::TConstJson;
	using NS_Const::TuneField;
	using NS_Const::TConstField;
	//��������� ���� ��� ��������:
	string conf_path = tune_ref->getFieldValueByCode(TuneField::MainPath);
	//������ ���� � ������. ������
	conf_path += TIndex::getStrValue(node, tag);
	//��������� ������� ������ ��������:
	StrArr files = TSimpleTune::getFileLst(conf_path);
	//�������� ������� ��������:
	for (const string& v : files)
	{
		TUserTune v_tune(*tune_ref, v);
		if (!v_tune.Empty()) db_tune.push_back(v_tune);
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
			if (tune_ref == nullptr) throw TLog("�� ������� ������ �� ������������ ���� ��������!", "TProcCell::InitByMethod");
			InitDBTune(node, tune_ref);
		}
		//������������ �����
		setCellDataArrByNode(cel_arr, node);
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
	if (json.empty()) throw TLog("������ json-����!", "TExcelProcData::InitObjByTag");
	string v_tag = TConstJson::asStr(tag);
	ptree::value_type v_node = json.find(v_tag).dereference();
	if (v_node.second.empty()) throw TLog("����: " + v_tag + " �� ������ � json-�����!", "TExcelProcData::InitObjByTag");
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
		//������������� json-�����:
		ptree js;
		read_json(json_file, js);
		if (js.empty()) throw TLog("������ json-����: " + json_file + "!", "TExcelProcData::InitExcelProcData");
		//DstFile:
		if (!InitObjByTag(js, JsonParams::DstFile, tune_ref))
			throw TLog("���� �������� �� ���������������!", "TExcelProcData::InitExcelProcData");
		//cells
		if (!InitObjByTag(js, JsonParams::Cells, tune_ref)) 
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

bool NS_Tune::TExcelProcData::setDstFileName(const string& name) noexcept(true)
{
	//��������� ������ ������������� DstFile
	if (DstFile == nullptr or name.empty()) return false;
	DstFile->setName(name);
	return true;
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

void NS_Tune::TCurRates::InitRates(ptree& sub_node, const NS_Const::JsonParams& root_tag,
	const NS_Const::JsonParams& code_tag,	const NS_Const::JsonParams& value_tag) noexcept(true)
{
	using NS_Const::TConstJson;
	string main_tag = TConstJson::asStr(root_tag);
	string val_tag = TConstJson::asStr(value_tag);
	//�������� �� �������� ������� ���� rates
	for (ptree::value_type& node : sub_node.get_child(main_tag))
	{
		string code;
		double val = 0;
		//��������� ������ ���� rates\code
		code = TIndex::getStrValue(node.second, code_tag);
		if (code.empty()) continue;
		//��������� ������ ���� rates\value
		val = node.second.get_child(val_tag).get_value<double>(0);
		//���� �������� ������� - ��������� ������:
		if (val > 0)
			arr.push_back(std::make_pair(code, val));
	}
}

NS_Tune::TCurRates::TCurRates(ptree::value_type& node, const NS_Const::JsonParams& rates_tag, 
	const NS_Const::JsonParams& arr_tag, const NS_Const::JsonParams& color_tag, 
	const NS_Const::JsonParams& rate_code, const NS_Const::JsonParams& rate_val): color(TColor::COLOR_NONE)
{
	using NS_Const::TConstJson;
	//���� ������ json-���� - �����
	if (node.second.empty()) return;
	string tag = TConstJson::asStr(rates_tag);
	//��������� ���������� � ������� ������:
	InitRates(node.second.get_child(tag), arr_tag, rate_code, rate_val);
	//��������� ������ �� �����:
	color = TIndex::getColorValue(node, color_tag);
}

bool NS_Tune::TCurRates::setByNode(ptree& node, const NS_Const::JsonParams& arr_tag,
	const NS_Const::JsonParams& color_tag, const NS_Const::JsonParams& rate_code,
	const NS_Const::JsonParams& rate_val) noexcept(true)
{
	using boost::property_tree::json_parser_error;
	if (node.empty()) return false;
	try
	{
		//�������� ���������� ������:
		clear();
		color = TIndex::getColorValue(node, color_tag);
		//���������� ���������:
		InitRates(node, arr_tag, rate_code, rate_val);
		return true;
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (const json_parser_error& err)
	{
		TLog(err.what(), "TCurRates::setByNode").toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ��������� ������ ����� ��� ����!", "TCurRates::setByNode").toErrBuff();
	}
	return false;
}

void NS_Tune::TCurRates::show(std::ostream& stream) const noexcept(true)
{
	using std::endl;
	if (!stream) return;
	for (const CurRate& cur : arr)
	{
		stream << "��� ������: " << cur.first << "\t����: " << cur.second << '\n';
	}
	if (color == TColor::COLOR_NONE)
		stream << "������ ���� �������!";
	else
		stream << "��� ����� �������: " << color;
	stream << '\n';
}

double NS_Tune::TCurRates::getRateByCode(const string& code) const noexcept(true)
{
	//�������� �� ���� ������ � ���������� ����
	for (size_t i = 0; i < arr.size(); i++)
		if (arr[i].first == code) return arr[i].second;
	return 0;
}

bool NS_Tune::TCurrencyBlock::setByJson(ptree& parent_node, const NS_Const::JsonParams& main_tag,
	const NS_Const::JsonParams& code_tag, const NS_Const::JsonParams& rates_tag,
	const NS_Const::JsonParams& arr_tag, const NS_Const::JsonParams& color_tag,
	const NS_Const::JsonParams& rate_code, const NS_Const::JsonParams& rate_val) noexcept(true)
{
	using NS_Const::TConstJson;
	string tag = TConstJson::asStr(main_tag);
	//��������� ����� ������� currency
	ptree& sub_node = parent_node.get_child(tag);
	if (sub_node.empty()) return false;
	//��������� ���� �������� ������:
	code = TIndex::getStrValue(sub_node, code_tag);
	//��������� ���� rates
	tag = TConstJson::asStr(rates_tag);
	sub_node = sub_node.get_child(tag);
	//������������� ��������� ������ ������:
	curArr.setByNode(sub_node, arr_tag, color_tag, rate_code, rate_val);
	return true;
}


NS_Tune::TCurrencyBlock::TCurrencyBlock(ptree& parent_node, const NS_Const::JsonParams& main_tag,
	const NS_Const::JsonParams& code_tag, const NS_Const::JsonParams& rates_tag,
	const NS_Const::JsonParams& arr_tag, const NS_Const::JsonParams& color_tag,
	const NS_Const::JsonParams& rate_code, const NS_Const::JsonParams& rate_val)
{
	setByJson(parent_node, main_tag, code_tag, rates_tag, arr_tag, color_tag, rate_code, rate_val);
}

double NS_Tune::TCurrencyBlock::getCurRateByCode(const string& cur) const noexcept(true)
{
	//���� ���� ��������� � ������� - �� �� ��������
	if (code == cur) return 1;
	//���� ���� � �������
	return curArr.getRateByCode(cur);
}

void NS_Tune::TCurrencyBlock::show(std::ostream& stream) const noexcept(true)
{
	if (!stream) return;
	stream << "��� �������� ������: " << code << '\n';
	curArr.show(stream);
}

void NS_Tune::TConditionValue::InitCaseVals(ptree::value_type& sub_node, const NS_Const::JsonParams& true_tag,
	const NS_Const::JsonParams& false_tag) noexcept(true)
{
	//��������� ������ ���������� iftrue � iffalse:
	vals.first = TIndex::getStrValue(sub_node, true_tag);
	vals.second = TIndex::getStrValue(sub_node, false_tag);
}

NS_Tune::TConditionValue::TConditionValue(ptree::value_type& sub_node,
	const NS_Const::JsonParams& true_tag,
	const NS_Const::JsonParams& false_tag): TFilterData(sub_node)
{
	//������������� ��� ����������:
	InitCaseVals(sub_node, true_tag, false_tag);
}

void NS_Tune::TConditionValue::show(std::ostream& stream) const noexcept(true)
{
	//����������� ������������� ������
	TFilterData::show(stream);
	//����������� ����������� ������:
	stream << "���� ������ ��������� ��������: " << getValue() << " - " << vals.first;
	stream << ", ����� - " << vals.second << std::endl;
}

void NS_Tune::TBalanceTune::InitDelimeters(ptree& node, const NS_Const::JsonParams& div_tag) noexcept(true)
{
	using NS_Const::JsonParams;
	using NS_Converter::UTF8ToANSI;
	//��������� ������������ ����:
	string tag = TConstJson::asStr(div_tag);
	//�������� �� ���� � ��������� ������
	for (ptree::value_type& sub_node : node.get_child(tag))
	{
		//��������� ������ �� ������� ������������
		string s = sub_node.second.get_value<string>("");
		if (s.empty()) continue;
		UTF8ToANSI(s);
		delimeters.push_back(s);
	}
}

void NS_Tune::TBalanceTune::InitConditions(ptree& node, const NS_Const::JsonParams& cond_tag) noexcept(false)
{
	using NS_Const::TConstJson;
	//��������� ������������ ����:
	string tag = TConstJson::asStr(cond_tag);
	//�������� �� ������� �������� �������:
	for (ptree::value_type& sub_node : node.get_child(tag))
	{
		TConditionValue cond(sub_node);
		if (cond.isEmpty()) continue;
		//����� ������ �������������� ��������, ����� �� ������� ��� ���������
		conditions.push_back(cond);
	}
}

void NS_Tune::TBalanceTune::InitByJson(ptree& node, const NS_Const::JsonParams& code_tag,
	const NS_Const::JsonParams& div_tag, const NS_Const::JsonParams& cond_tag,
	const NS_Const::JsonParams& param_tag, const NS_Const::JsonParams& cur_tag) noexcept(true)
{
	if (node.empty()) return;
	//������������ ���� � ������ ��������:
	source = TIndex::getStrValue(node, code_tag);
	//������������� ������� ������������:
	InitDelimeters(node, div_tag);
	//������������� �������� ��������:
	InitConditions(node, cond_tag);
	//������������� ����������:
	setCellDataArrByNode(params, node, param_tag);
}


NS_Tune::TBalanceTune::TBalanceTune(ptree& node, const NS_Const::JsonParams& code_tag,
	const NS_Const::JsonParams& div_tag, const NS_Const::JsonParams& cond_tag, 
	const NS_Const::JsonParams& param_tag, const NS_Const::JsonParams& cur_tag): cur(node, cur_tag)
{
	InitByJson(node, code_tag, div_tag, cond_tag, param_tag, cur_tag);
}

NS_Tune::TBalanceTune::TBalanceTune(const string& json_file,
	const NS_Const::JsonParams& code_tag,
	const NS_Const::JsonParams& div_tag, const NS_Const::JsonParams& cond_tag, 
	const NS_Const::JsonParams& param_tag, const NS_Const::JsonParams& cur_tag)
{
	using boost::property_tree::ptree;
	using boost::property_tree::file_parser_error;
	using boost::property_tree::json_parser_error;
	using boost::property_tree::json_parser::read_json;
	if (json_file.empty()) return;
	try
	{
		ptree js;
		read_json(json_file, js);
		if (js.empty()) throw TLog("������ json-����: " + json_file + "!", "TBalanceTune::TBalanceTune");
		//������������� ���������� �� lson-������
		InitByJson(js, code_tag, div_tag, cond_tag, param_tag, cur_tag);
		//������������� ������:
		cur.setByJson(js, cur_tag);
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

void NS_Tune::TBalanceTune::show(std::ostream& stream) const noexcept(true)
{
	using std::endl;
	if (!stream) return;
	stream << "���� � �����(��) ����������:" << source << endl;
	stream << "�������� ��������: ";
	if (conditions.empty())
		stream << "�� ���������!";
	else
	{
		stream << endl;
		for (const TConditionValue& v: conditions)
			v.show(stream);
	}
	stream << "������ �� ����������:";
	if (params.empty())
		 stream << " �� ���������!" << endl;
	else
	{
		stream << endl;
		showCellDataArr(params, stream);
	}
	stream << "������ �� ������ �����: ";
	if (cur.isEmpty())
		stream << "�� ���������!";
	else
	{
		stream << endl;
		cur.show(stream);
	}
}

NS_Tune::StrArr NS_Tune::TBalanceTune::getImportFiles(const string& main_path) const noexcept(true)
{
	using NS_Tune::TSimpleTune;
	try
	{
		return TSimpleTune::getFileLst(main_path + source);
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("�� ������������ ������ ��������� ������ ������ �� ����: ", "TBalanceTune::getImportFiles");
		log << source;
		log.toErrBuff();
	}
	return StrArr();
}

void NS_Tune::setIntArrByJson(IntArr& arr, ptree& node, const NS_Const::JsonParams& tag) noexcept(true)
{
	using NS_Const::TConstJson;
	using NS_Const::JsonParams;
	if (node.empty()) return;
	string v_tag = TConstJson::asStr(tag);
	if (v_tag.empty()) return;
	for (const ptree::value_type& js : node.get_child(v_tag))
	{
		//if (js.second.empty()) continue;
		//��������� ��������:
		size_t val = js.second.get_value<size_t>(TIndex::EmptyIndex);
		//���� �������� ������ - �����
		if (val == TIndex::EmptyIndex) continue;
		arr.push_back(val);
	}
}

void NS_Tune::showIntArr(const IntArr& arr, std::ostream& stream) noexcept(true)
{
	try
	{
		size_t i = 0;
		stream << "[";
		while (i < arr.size() - 1)
			stream << arr[i++];
		stream << arr[i] << "]";
	}
	catch (...)
	{
		TLog("�� ������������ ������ ��� ������ �������!", "NS_Tune::showIntArr");
	}
}

void NS_Tune::TImpPatternTune::InitBySubNode(ptree::value_type& sub_node, const NS_Const::JsonParams& src_tag,
	const NS_Const::JsonParams& fields_tag, const NS_Const::JsonParams& blocks_tag) noexcept(true)
{
	//��������� ����� ��������� ��� ��������:
	source = TIndex::getStrValue(sub_node, src_tag);
	//���������� ������� �������� ����� �������:
	setIntArrByJson(fields, sub_node.second, fields_tag);
	//���������� ������� ������� ������ ������:
	setIntArrByJson(blocks, sub_node.second, blocks_tag);
}

NS_Tune::TImpPatternTune::TImpPatternTune(ptree::value_type& sub_node, const NS_Const::JsonParams& src_tag,
	const NS_Const::JsonParams& fields_tag, const NS_Const::JsonParams& blocks_tag)
{
	InitBySubNode(sub_node, src_tag, fields_tag, blocks_tag);
}

NS_Tune::TImpPatternTune::TImpPatternTune(ptree& node, const NS_Const::JsonParams& main_tag,
	const NS_Const::JsonParams& src_tag, const NS_Const::JsonParams& fields_tag,
	const NS_Const::JsonParams& blocks_tag)
{
	using NS_Const::TConstJson;
	//��������� ������������ ����:
	string tag = TConstJson::asStr(main_tag);
	//��������� ��������� ��������� ���������
	ptree::value_type& sub_node = node.find(tag).dereference();
	if (sub_node.second.empty()) return;
	InitBySubNode(sub_node, src_tag, fields_tag, blocks_tag);
}

void NS_Tune::TImpPatternTune::show(std::ostream& stream)const noexcept(true)
{
	using std::endl;
	//���� ����� ��������� - �����
	if (!stream) return;
	stream << "��� ������� ����� ��������: ";
	if (source.empty())
		stream << "�� �������!";
	else
	 stream << source;
	stream << endl << "���� ����������� �� �������: ";
	if (fields.empty())
		stream << "�� ���������!";
	else
		showIntArr(fields, stream);
	stream << endl << "������ � ������ ������ � �������: ";
	if (NoEmptyBlocks())
		stream << "�� ���������!";
	else
		showIntArr(blocks, stream);
}

NS_Tune::TImpDocsTune::TImpDocsTune(ptree& node, const string& main_path, const NS_Const::JsonParams& src_tag,
	const NS_Const::JsonParams& par_tag, const NS_Const::JsonParams& pattern_tag, 
	const NS_Const::JsonParams& ptrn_name_tag,	const NS_Const::JsonParams& ptrn_flds_tag, 
	const NS_Const::JsonParams& ptrn_blck_tag) : 
	//������������� ���������� �������� � ��������� ��������:
	src_data(node, main_path, src_tag), 
	pattern(node, pattern_tag, ptrn_name_tag, ptrn_flds_tag, ptrn_blck_tag)
{
	//������������� ���������� ����� ��� ���������� �� excel-�����
	setCellDataArrByNode(params, node, par_tag);
}

void NS_Tune::TImpDocsTune::InitByJsonFile(const string& json_file, const string& main_path, 
	const NS_Const::JsonParams& src_tag, const NS_Const::JsonParams& par_tag,
	const NS_Const::JsonParams& pattern_tag, const NS_Const::JsonParams& ptrn_name_tag,
	const NS_Const::JsonParams& ptrn_flds_tag, const NS_Const::JsonParams& ptrn_blck_tag) noexcept(true)
{
	using boost::property_tree::ptree;
	using boost::property_tree::file_parser_error;
	using boost::property_tree::json_parser_error;
	using boost::property_tree::json_parser::read_json;
	if (json_file.empty()) return;
	try
	{
		//�������������� json-���� ��� ����������:
		ptree js;
		read_json(json_file, js);
		//���� ������ ���
		if (js.empty()) throw TLog("������ ������ � �����: " + json_file, "TImpDocsTune::InitByJsonFile");
		//������������� ���������� �������
		src_data = TShareData(js, main_path, src_tag);
		//������������� ���������� ��������:	
		pattern = TImpPatternTune(js, pattern_tag, ptrn_name_tag, ptrn_flds_tag, ptrn_blck_tag);
		//������������� ���������� ����� ������ ��������:
		setCellDataArrByNode(params, js, par_tag);
	}
	catch (const json_parser_error& err)
	{
		TLog(err.what(), "InitByJsonFile").toErrBuff();
	}
	catch (const file_parser_error& err)
	{
		TLog(err.what(), "InitByJsonFile").toErrBuff();
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ��� ���������� json-�����: " + json_file, "TImpDocsTune::InitByJsonFile").toErrBuff();
	}
}

NS_Tune::TImpDocsTune::TImpDocsTune(const string& json_file, const string& main_path,
	const NS_Const::JsonParams& src_tag, const NS_Const::JsonParams& par_tag,
	const NS_Const::JsonParams& pattern_tag, const NS_Const::JsonParams& ptrn_name_tag,
	const NS_Const::JsonParams& ptrn_flds_tag, const NS_Const::JsonParams& ptrn_blck_tag)
{
	//������������� �� json-�����
	InitByJsonFile(json_file, main_path);
}


void NS_Tune::TImpDocsTune::show(std::ostream& stream) const noexcept(true)
{
	using std::endl;
	if (!stream) return;
	stream << "��������� ������ ��� ��������: ";
	if (src_data.isEmpty())
		stream << "�� �������!";
	else
		src_data.show(stream << '\n');
	stream << "������ � ���������� ����������: ";
	if (params.empty())
		stream << "�� �����������!";
	else
		showCellDataArr(params, stream << '\n');
	stream << endl << "������ � �������: " << endl;
	pattern.show(stream);
}