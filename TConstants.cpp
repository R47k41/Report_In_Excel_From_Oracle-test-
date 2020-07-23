//�������� ������� ��� ������ Constants.h
#include <iostream>
#include <iterator>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "TConstants.h"
#include "Logger.hpp"

using std::string;

//����� ��������������� ��� ��������
//http://www.cyberforum.ru/cpp-beginners/thread1798717.html#post9488987
template NS_Const::TConstant<NS_Const::TuneField, NS_Const::TuneField::Empty, NS_Const::TuneField::Last>;
template NS_Const::TConstant<NS_Const::DataType, NS_Const::DataType::ErrorType, NS_Const::DataType::Last>;
template NS_Const::TConstant<NS_Const::TExclBaseTune, NS_Const::TExclBaseTune::Empty, NS_Const::TExclBaseTune::Last>;
template NS_Const::TConstant<NS_Const::ReportCode, NS_Const::ReportCode::Empty, NS_Const::ReportCode::Last>;
template NS_Const::TConstant<NS_Const::TSql, NS_Const::TSql::Empty, NS_Const::TSql::Last>;
template NS_Const::TConstant<NS_Const::CtrlSym, NS_Const::CtrlSym::Empty, NS_Const::CtrlSym::Last>;
template NS_Const::TConstant<NS_Const::JsonParams, NS_Const::JsonParams::Null, NS_Const::JsonParams::Last>;
template NS_Const::TConstant<NS_Const::JSonMeth, NS_Const::JSonMeth::Null, NS_Const::JSonMeth::Last>;
template NS_Const::TConstant<NS_Const::JsonCellFill, NS_Const::JsonCellFill::Null, NS_Const::JsonCellFill::Last>;
template NS_Const::TConstant<NS_Const::JsonFilterOper, NS_Const::JsonFilterOper::Null, NS_Const::JsonFilterOper::Last>;


//���������� ������� � ������� DateInterface
namespace NS_Const
{
	namespace DateInteface
	{
		//�������������� ���� � ������:
		string date_to_string(const boost::gregorian::date& v_date,
			const string& format = "%d.&m.%Y") noexcept(false);
	}
}

//��������������� ��������� �������:
//template <typename Type>
//bool NS_Const::TConstJSFilterOper::runBaseOperation(const Type& val1, const Type& val2, 
//	const NS_Const::JsonFilterOper& oper_code) noexcept(true);

//template <typename Type>
//bool NS_Const::TConstJSFilterOper::RunOperation(const Type& val1, const Type& val2,
//	const NS_Const::JsonFilterOper& oper_code) noexcept(true);

//�������������� � ������ �������:
string NS_Const::LowerCase(const string& str)
{
	string result;
	std::transform(str.begin(), str.end(), std::insert_iterator<std::string>(result, result.begin()), tolower);
	return result;
}

string NS_Const::UpperCase(const string& str)
{
	string result;
	std::transform(str.begin(), str.end(), std::insert_iterator<std::string>(result, result.begin()), toupper);
	return result;
}

NS_Const::TTrimObj::TTrimObj(const TConstSql& title)
{
	symb = title.toStr();
}

bool NS_Const::TTrimObj::operator()(const char& ch) const
{
	for (char v : symb)
		if (v == ch)
			return true;
	return false;
};

void NS_Const::Trim_Left(string& str)
{
	string syms = { TConstCtrlSym(CtrlSym::Space).toChar(), TConstCtrlSym(CtrlSym::NL).toChar(),
		TConstCtrlSym(CtrlSym::Tab).toChar() };
	string::const_iterator b = std::find_if_not(str.begin(), str.end(), TTrimObj(syms));
	if (b >= str.begin()) str.erase(str.begin(), b);
};

void NS_Const::Trim_Right(string& str)
{
	string syms = { TConstCtrlSym(CtrlSym::Space).toChar(), TConstCtrlSym(CtrlSym::NL).toChar(),
		TConstCtrlSym(CtrlSym::Tab).toChar() };
	string::const_reverse_iterator b = std::find_if_not(str.rbegin(), str.rend(), TTrimObj(syms));
	if (b != str.rbegin()) str.erase(b.base(), str.end());
};

void NS_Const::Trim(string& str)
{
	Trim_Left(str);
	Trim_Right(str);
}

char NS_Const::getNLSNumPoint() noexcept(true)
{
	using std::locale;
	//������� ������
	locale loc("");
	//������������� ������ �����:
	const std::numpunct<char>& numsep = std::use_facet<std::numpunct<char> >(loc);
	return numsep.decimal_point();
}

double NS_Const::Round(double x, int sz) noexcept(true)
{
	if (x > 0)
		return ((std::floor(x) * sz)+0.5) / sz;
	else
		return ((std::ceil(x) * sz)-0.5) / sz;
}

bool NS_Const::DateInteface::set_stream_date_format(std::ostream& stream, const string& format) noexcept(true)
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

string NS_Const::DateInteface::date_to_string(const boost::gregorian::date& v_date,	const string& format) noexcept(false)
{
	using boost::gregorian::date;
	using std::stringstream;
	//���� ��� �� ����
	if (v_date.is_not_a_date()) return string();
	stringstream ss;
	//��������� ������� ������ ������
	if (set_stream_date_format(ss, format))
	{
		ss << v_date;
		return ss.str();
	}
	return string();
}


string NS_Const::DateInteface::cur_date_to_string_by_format(const string& format) noexcept(false)
{
	using boost::gregorian::day_clock;
	return date_to_string(day_clock::local_day(), format);
}

string NS_Const::DateInteface::from_date(int yy, size_t mm, size_t dd, const string& format) noexcept(true)
{
	using boost::gregorian::date;
	//using boost::date_time::date;
	date v_date = date(yy, mm, dd);
	return date_to_string(v_date, format);
}

string NS_Const::DateInteface::from_date(double date_as_dbl, const string& format) noexcept(true)
{
	using boost::gregorian::date;
	date v_date = date(date_as_dbl);
	return date_to_string(v_date, format);
}

template <typename T, T min_val, T max_val>
bool NS_Const::TConstant<T, min_val, max_val>::isValid(const T& a, const T& b, bool exit_on_err) const noexcept(false)
{
	if (inRange(a, b))
		return true;
	if (exit_on_err)
		return false;
	throw string("������ ��������� � �������� ��������!");
}

template <typename T, T min_val, T max_val>
void NS_Const::TConstant<T, min_val, max_val>::setValue(const T& x)
{
	val = static_cast<int>(x);
}

template <typename T, T min_val, T max_val>
void NS_Const::TConstant<T, min_val, max_val>::setValue(int x)
{
	val = x;
}

template <typename T, T min_val, T max_val>
void NS_Const::TConstant<T, min_val, max_val>::Init(const T& def_val) noexcept(true)
{
	if (!isValid(true)) setValue(def_val);
}

template <typename T, T min_val, T max_val>
NS_Const::TConstant<T, min_val, max_val>& NS_Const::TConstant<T, min_val, max_val>::Next(bool exit_on_err) noexcept(false)
{
	*this += 1;
	if (!isValid(exit_on_err)) setValue(max_val);
	return *this;
}

template<typename T, T min_val, T max_val>
NS_Const::TConstant<T, min_val, max_val>& NS_Const::TConstant<T, min_val, max_val>::operator=(const T& x)
{
	if (Value() != x) setValue(x);
	return *this;
}

bool NS_Const::TConstField::StrInclude(const string& str) const
{
	if (str.empty()) return false;
	string tmp = toStr();
	if (str.find(tmp, 0) != string::npos)
	{
		size_t i = 0;
		while (!isalnum(str[i])) i++;
		//���� ������������ ������ ���������, �� ������ - ������� �����������
		if (str.compare(tmp) >= 0)	return true;
	}
	return false;
}

string NS_Const::TConstField::description(const TuneField& code) noexcept(true)
{
	switch (code)
	{
		//Shared, Paths, Report_Code
	case TuneField::Shared: return "���� � ������ ����������� ��� �������";
	case TuneField::Paths: return "���� � ������ ������� � �� �����������";
	case TuneField::DataBase: return "���� �������� ��";
	case TuneField::AddDateToOutFileName: return "���������� ���� � ����� ��������� �����";
	case TuneField::AddDateToSheetName: return "���������� ���� � �������� ��������/�����";
	case TuneField::AddDateToOutPath: return "���������� ���� � ����";
	case TuneField::ConfigPath: return "���� � ����������� ������ ������ ini-�����";
	case TuneField::ConfigFileExt: return "���������� ������ ��������";
	case TuneField::SubTunePath: return "���� � �������������� ���������� ������: ������ json-�����";
	case TuneField::SubTuneFileExt: return "���������� ������ � ���. �����������";
	case TuneField::SqlPath: return "���� � ������ sql-������";
	case TuneField::SqlFileExt: return "���������� ������ sql-������";
	case TuneField::TemplatePath: return "���� � ������ ������� ������";
	case TuneField::TemplateFileExt: return "���������� ����� �������";
	case TuneField::OutDirectory: return "���� ��� ������������ ��������� �����";
	case TuneField::MainPath: return "���� � �������� ���������� ������";
	case TuneField::OutFileName: return "��� ��������� �����";
	case TuneField::UserName: return "��� ������������ ��";
	case TuneField::Password: return "������ ������������ ��";
	case TuneField::TNS: return "TNS";
	case TuneField::Report: return "���� �������� ��� ������ ini-�����";
	case TuneField::SheetName: return "��� �����/�������� ������ � ini-�����";
	case TuneField::TemplateName: return "��� ����� ������� � ini-�����";
	case TuneField::SqlFirst: return "������� �������������� ���������� DQL-������ ����� ������ ����������� DML-�������";
	case TuneField::SqlFile: return "����/��� ��� ������ DQL-������";
	case TuneField::SqlText: return "����� DQL-�������";
	case TuneField::UseSqlParser: return "������� ������������� SQL-�������(�� ������������, �.�. ��������� ����� ������� �������)";
	case TuneField::DMLText: return "����� DML-�������";
	case TuneField::DMLFile: return "����/���� � DML-���������";
	case TuneField::ClearSLQText: return "����� ������� ��� ������� ������(������������ ����� ��������/��������)";
	case TuneField::ClearSQLFile: return "����/���� � �������� ��� ������� ������";
	case TuneField::Columns: return "���� ������ � �������� excel-������";
	case TuneField::Column: return "��������� � �������� ���� ���������� ������������ ������� ������";
	case TuneField::SqlParams: return "���� ����������� ���������� ��� ������(��������� �� ���������� ����� ������)";
	case TuneField::SqlParam: return "��� ���� Param � �������� ����������� ������ ����������";
	case TuneField::SqlParamQuane: return "����������� ���������";
	case TuneField::SqlParamType: return "��� ������ ���������";
	case TuneField::SqlParamNote: return "�������� ���������(���������, ���� ��� ���� ������ ������������)";
	case TuneField::SqlParamValue: return "�������� ���������(���� �� ������� - ��������� ������������)";
	case TuneField::Block_End: return "���� ���������� ������ ���������� ����� [����]. ���������� ��� ��������";
	default: return "��������� ������ " + asStr(code) + " �� ������!";
	}
}

string NS_Const::TConstField::asStr(const TuneField& val)
{
	switch (val)
	{
		//Shared, Paths, Report_Code
	case TuneField::Shared: return "[SHARED]";
	case TuneField::Paths: return "[PATHS]";
	case TuneField::DataBase: return "[DATA BASE]";
	case TuneField::AddDateToOutFileName: return "AddDateToOutFileName";
	case TuneField::AddDateToSheetName: return "AddDateToSheetName";
	case TuneField::AddDateToOutPath: return "AddDateToOutPath";
	case TuneField::ConfigPath: return "ConfigPath";
	case TuneField::ConfigFileExt: return "ConfigFileExt";
	case TuneField::SubTunePath: return "SubTunePath";
	case TuneField::SubTuneFileExt: return "SubTuneFileExt";
	case TuneField::SqlPath: return "SqlPath";
	case TuneField::SqlFileExt: return "SqlFileExt";
	case TuneField::TemplatePath: return "TemplatePath";
	case TuneField::TemplateFileExt: return "TemplateFileExt";
	case TuneField::OutDirectory: return "OutDirectory";
	case TuneField::MainPath: return "MainPath";
	case TuneField::OutFileName: return "OutFileName";
	case TuneField::UserName: return "UserName";
	case TuneField::Password: return "Password";
	case TuneField::TNS: return "TNS";
	case TuneField::Report: return "[REPORT]";
	case TuneField::SheetName: return "SheetName";
	case TuneField::TemplateName: return "TemplateName";
	case TuneField::SqlFirst: return "SqlFirst";
	case TuneField::SqlFile: return "SQLFile";
	case TuneField::SqlText: return "SQLText";
	case TuneField::UseSqlParser: return "UseSqlParser";
	case TuneField::DMLText: return "DMLText";
	case TuneField::DMLFile: return "DMLFile";
	case TuneField::ClearSLQText: return "ClearSQLText"; 
	case TuneField::ClearSQLFile: return "ClearSQLFile";
	case TuneField::Columns: return "[COLUMNS]";
	case TuneField::Column: return "Column";
	case TuneField::SqlParams: return "[PARAMETERS]";
	case TuneField::SqlParam: return "Param";
	case TuneField::SqlParamQuane: return "Quane";
	case TuneField::SqlParamType: return "Type";
	case TuneField::SqlParamNote: return "Comment";
	case TuneField::SqlParamValue: return "Value";
	case TuneField::Block_End: return "[END]";
	default: return string();
	}
};

NS_Const::TuneField NS_Const::TConstField::getIDByCode(const string& code, const TuneField& bval, const TuneField& eval)
{
	for (TConstField i(bval); i < eval; i.Next())
		if (i == code) return i.Value();
	return TuneField::Empty;
}

NS_Const::TConstType::TConstType(const string& str): NS_Const::DF_const(DataType::ErrorType)
{
	while (Value() < DataType::Last)
	{
		if (toStr() == str) break;
		Next();
	}
	if (Value() == DataType::Last)
		setValue(DataType::ErrorType);
}

string NS_Const::TConstType::asStr(const DataType& dt) noexcept(true)
{
	switch (dt)
	{
	case DataType::String: return "string";
	case DataType::Integer: return "integer";
	case DataType::Double: return "double";
	case DataType::Date: return "date";
	case DataType::Boolean: return "boolean";
	case DataType::SQL_String: return "sql_string";
	}
	return string();
}

NS_Const::DataType NS_Const::TConstType::getCodeByName(const string& name) noexcept(true)
{
	if (name.empty()) return DataType::ErrorType;
	TConstType dt(DataType::ErrorType);
	string tmp = LowerCase(name);
	Trim(tmp);
	//�������� �� ���� ����� ����� ������
	while (dt.Next() != DataType::Last or dt.toStr() != tmp) ;
	if (dt.Value() == DataType::Last) return DataType::ErrorType;
	return dt.Value();	
}


string NS_Const::TConstExclTune::asStr(const TExclBaseTune& val) noexcept(true)
{
	switch (val)
	{
	case TExclBaseTune::xlt: return ".xlt";
	case TExclBaseTune::xls: return ".xls";
	//���������� �� ���������
	case TExclBaseTune::DefExt:
	case TExclBaseTune::xlsx: 
		return ".xlsx";
	//��� ����� � ����� �� ���������
	case TExclBaseTune::DefName: return "�����";
	case TExclBaseTune::DefSh: return "�����";
	//����������� ������� ������ ������
	case TExclBaseTune::PageDelimiter: return " ���. ";
	}
	return string();
}

string NS_Const::TConstExclTune::getFileExtention(const string& val) noexcept(true)
{
	if (val.empty()) return string();
	const char delimeter = TConstCtrlSym::asChr(CtrlSym::point);
	//��������� ����� ����� �� ��������:
	int pos = val.find_last_of(delimeter);
	if (pos == string::npos) return string();
	return val.substr(pos);
}

bool NS_Const::TConstExclTune::isValidExtensions(const string& val) noexcept(true)
{
	if (val.empty()) return false;
	if (val == asStr(TExclBaseTune::xls) or val == asStr(TExclBaseTune::xlsx)
		or val == asStr(TExclBaseTune::xlt))
		return true;
	return false;
}

bool NS_Const::TConstExclTune::isValidFileByExtension(const string& name) noexcept(true)
{
	if (name.empty()) return false;
	string ext = getFileExtention(name);
	return isValidExtensions(ext);
}

NS_Const::TExclBaseTune NS_Const::TConstExclTune::getFileExtCode(const string& ext) noexcept(true)
{
	if (ext.empty()) return TExclBaseTune::Empty;
	TConstExclTune val(TExclBaseTune::xlt);
	for (; val <= TExclBaseTune::xlsx; val.Next())
		if (val.toStr() == ext) return val.Value();
	return TExclBaseTune::Empty;
}

bool NS_Const::TConstExclTune::isTemplate(const TExclBaseTune& val) noexcept(true)
{
	switch (val)
	{
		case TExclBaseTune::xlt: return true;
		default: return false;
	}
	return false;
}

string NS_Const::TConstReportCode::toStr() const
{
	switch (Value())
	{
	case ReportCode::RIB_DOCS_FOR_PERIOD: return "RIB_DOCS_FOR_PERIOD";
	case ReportCode::DOCS_MF_SF_FOR_PERIOD: return "DOCS_MF_SF_FOR_PERIOD";
	case ReportCode::REPAYMENT_FOR_DATE: return "REPAYMENT_FOR_DATE";
	case ReportCode::POTREB_CRED_BY_FILE: return "POTREB_CRED_BY_FILE";
	case ReportCode::CRED_CASE_MF: return "CRED_CASE_MF";
	case ReportCode::BALANCE_LIST: return "BALANCE_LIST";
	case ReportCode::BALANCE_SUA: return "BALANCE_SUA";
	case ReportCode::FULL_CRED_REPORT: return "FULL_CRED_REPORT";
	case ReportCode::FULL_CRED_REPORT_SUA: return "FULL_CRED_REPORT_SUA";
	case ReportCode::NBKI_NP: return "NBKI_NP";
	case ReportCode::NBKI_JP: return "NBKI_JP";
	case ReportCode::NBKI_APPLY: return "NBKI_APPLY";
	case ReportCode::CLOSE_DAY: return "CLOSE_DAY";
	case ReportCode::LOAD_FROM_FILE: return "LOAD_FROM_FILE";
	case ReportCode::FILE_COMPARE_RIB: return "FILE_COMPARE_RIB";
	case ReportCode::FILE_COMPARE_RTBK: return "FILE_COMPARE_RTBK";
	case ReportCode::EXCEL_SET_DATA_FROM_BASE: return "EXCEL_SET_DATA_FROM_BASE";
	case ReportCode::EXCEL_DOC_LOAD: return "EXCEL_DOC_LOAD";
	case ReportCode::EXCEL_PAY_LOAD_SF: return "EXCEL_PAY_LOAD_SF";
	case ReportCode::EXCEL_PAY_LOAD_MF: return "EXCEL_PAY_LOAD_MF";
	case ReportCode::ACCOUNT_BALANCE: return "ACCOUNT_BALANCE";
	case ReportCode::ACCOUNT_BALANCE_STREAM: return "ACCOUNT_BALANCE_STREAM";
	case ReportCode::LOTS: return "LOTS";
	case ReportCode::SMLVCH_BALANCE: return "SMLVCH_BALANCE";
	case ReportCode::SMLVCH_IMP: return "SMLVCH_IMP";
	case ReportCode::QUIT_REPORT: return "QUIT_REPORT";
	}
	return  string();
}

string NS_Const::TConstReportCode::getName() const
{
	switch (Value())
	{
	case ReportCode::RIB_DOCS_FOR_PERIOD: return "�������: ��������� ��� �� ������";
	case ReportCode::DOCS_MF_SF_FOR_PERIOD: return "����������: ��������� �� ������ �� + ��";
	case ReportCode::REPAYMENT_FOR_DATE: return "��������: ������� �� ������ �� + ��";
	case ReportCode::POTREB_CRED_BY_FILE: return "��������: ��������������� ������� �� �� �����";
	case ReportCode::CRED_CASE_MF: return "��������: ��������� �������� ��";
	case ReportCode::BALANCE_LIST: return "��������: ��������� ��������";
	case ReportCode::BALANCE_SUA: return "��������: ��������� �������� ��� �������� � ���";
	case ReportCode::FULL_CRED_REPORT: return "��������: ������ ��������� ��������";
	case ReportCode::FULL_CRED_REPORT_SUA: return "��������: ����� �� ���� �������� ��� ���";
	case ReportCode::NBKI_NP: return "��������: ���� ���. ����";
	case ReportCode::NBKI_JP: return "��������: ���� ��. ����";
	case ReportCode::NBKI_APPLY: return "��������: ���� �������� ���������";
	case ReportCode::CLOSE_DAY: return "�������� �������";
	case ReportCode::LOAD_FROM_FILE: return "�������� �������� �� ����� � OraBank(������� insert)";
	case ReportCode::FILE_COMPARE_RIB: return "�������: ���������� �������� �� ��� �� ����� �� �������";
	case ReportCode::FILE_COMPARE_RTBK: return "��������: ���������� �������� �� ���� �� ����� �� �������";
	case ReportCode::EXCEL_SET_DATA_FROM_BASE: return "Excel: ���������� ����� ����� �� ��";
	case ReportCode::EXCEL_DOC_LOAD: return "��������: �������� ���������� �� ���";
	case ReportCode::EXCEL_PAY_LOAD_SF: return "��������: �������� ������ �� ��� ��� ��";
	case ReportCode::EXCEL_PAY_LOAD_MF: return "��������: �������� ������ �� ��� ��� ��";
	case ReportCode::ACCOUNT_BALANCE: return "������� �� �����";
	case ReportCode::ACCOUNT_BALANCE_STREAM: return "��������� ������� �� ������ ������";
	case ReportCode::LOTS: return "��������: ����";
	case ReportCode::SMLVCH_BALANCE: return "��������: ��������� ��������";
	case ReportCode::SMLVCH_IMP: return "��������: ������������ ��������� ��� �������";
	case ReportCode::QUIT_REPORT: return "�����";
	}
	return  string();
}

void NS_Const::TConstReportCode::show() noexcept(true)
{
	using std::cout;
	using std::endl;
	TConstReportCode report(ReportCode::Empty);
	while (report.Next().Value() < ReportCode::Last)
		cout << report.toInt() << ". " << report.getName() << endl;
}

NS_Const::ReportCode NS_Const::TConstReportCode::getIDByCode(const string& code, const ReportCode& bval, const ReportCode& eval)
{
	if (!code.empty())
	{
		for (TConstReportCode i(bval); i < eval; i.Next())
			if (i == code) return i.Value();
	}
	return ReportCode::Empty;
}

string NS_Const::TConstCtrlSym::asStr(const CtrlSym& val)
{
	switch (val)
	{
	case CtrlSym::EOL: return "\0";
	case CtrlSym::Space: return " ";
	case CtrlSym::EndCommand:
	case CtrlSym::semicolon: return ";";
	case CtrlSym::EndCol: return ",";
	case CtrlSym::point: return ".";
	case CtrlSym::lbkt: return "(";
	case CtrlSym::rbkt: return ")";
	case CtrlSym::qlbkt: return "[";
	case CtrlSym::qrbkt: return "]";
	case CtrlSym::crwn:
	case CtrlSym::quotes: return "\"";
	case CtrlSym::Tab: return "\t";
	case CtrlSym::NL: return "\n";
	case CtrlSym::colon: return ":";
	case CtrlSym::dash: return "\\";
	case CtrlSym::quane: return "=";
	case CtrlSym::rangle: return ">";
	case CtrlSym::langle: return "<";
	case CtrlSym::dies_comment: return "#";
	case CtrlSym::minus_comment: return "--";
	case CtrlSym::dash_comment: return "//";
	}
	return string();
}

string NS_Const::operator+(const string& str, const TConstCtrlSym& ch)
{
	return str + ch.toStr();
}

string NS_Const::operator+(const TConstCtrlSym& ch, const string& str)
{
	string result;
	result += ch.toStr() + str;
	return result;
}

string NS_Const::TConstSql::toStr() const
{
	switch (Value())
	{
	case TSql::With: return "with";
	case TSql::Select: return "select";
	case TSql::From: return "from";
	case TSql::Where: return "where";
	case TSql::Order: return "order by";
	case TSql::Group: return "group by";
	case TSql::As: return " as ";
	case TSql::And: return "and";
	case TSql::Or: return "or";
	case TSql::EOC: //return ";";//������ ��������� �������
		return TConstCtrlSym(CtrlSym::EndCommand).toStr();
	case TSql::D4L: //return ",";//����������� ����� � select/from/order/group
		return TConstCtrlSym(CtrlSym::EndCol).toStr();
	}
	return string();
}

bool NS_Const::TConstSql::MustFound() const
{
	switch (Value())
	{
		//case TCtrlSql::With:
		case TSql::Select:
		case TSql::From:
		case TSql::EOC:
			return true;
	}
	return false;
}

NS_Const::TSql NS_Const::TConstSql::GetDelimeter() const
{
	switch (Value())
	{
	case TSql::Select:
	case TSql::From:
	case TSql::Order:
	case TSql::Group:
		return TSql::D4L;
	case TSql::Where:
		string val = toStr();
		throw string(string("��� �����: ") + val.c_str() + " ������� ������ ������ �����������!");
	}
	return TSql::Empty;
}

bool NS_Const::TConstSql::CorrectDelimeter(const string& d) const noexcept(true)
{
	try
	{
		TConstSql tmp = GetDelimeterAsObj();
		if (tmp.toStr() == d) return true;
	}
	catch (const string& err)
	{
		if (Value() == TSql::Where)
			return (TConstSql(TSql::And).toStr() == d || TConstSql(TSql::Or).toStr() == d || d.empty());
		else
		{
			std::cerr << err << std::endl;
			return false;
		}
	}
	return false;
}

string NS_Const::TConstSql::getClosedElem() const noexcept(false)
{
	switch (Value())
	{
	case TSql::From:
	case TSql::Where:
	case TSql::Order:
	case TSql::Group:
		TConstCtrlSym tmp(CtrlSym::EndCommand);
		return tmp.toStr();
	}
	return string();
}

std::ostream& NS_Const::operator<<(std::ostream& stream, const TConstSql& val) noexcept(false)
{
	if (stream) stream << val.toStr();
	return stream;
}

NS_Const::TConstSql NS_Const::TConstSql::operator+(int x) const noexcept(true)
{
	TConstSql tmp(x + toInt());
	return tmp;
}

NS_Const::TSymGroup::TSymGroup(bool set_default)
{
	if (set_default)
		//��������� ������ ����������� ��������: '(', ')', '"'
		arr = { {TConstCtrlSym(CtrlSym::lbkt).toStr()[0], TConstCtrlSym(CtrlSym::rbkt).toStr()[0]},
			{TConstCtrlSym(CtrlSym::crwn).toStr()[0], TConstCtrlSym(CtrlSym::crwn).toStr()[0]} };
}

bool NS_Const::TSymGroup::IsCorrectSym(const string& str, std::size_t pos, const std::size_t cnt) const noexcept(false)
{
	using std::size_t;
	using std::isalnum;
	if (pos == 0 and str.size() > (pos + cnt + 1)
		and !isalnum(str[pos + cnt + 1])) return true;
	//���������, ��� ��� �� ����� �����:
	if (pos != 0 and isalnum(str[pos - 1]) and
		(pos + cnt + 1) < str.size() and isalnum(str[pos + cnt + 1])) return false;
	//��������� �� ������ �� ����������� � ���������:
	char ch = TConstCtrlSym::asChr(CtrlSym::crwn);
	for (const Syms& v : arr)
	{
		size_t pe = str.find(v.second, pos);
		//���� ����������� ���������� ���
		if (pe == string::npos) continue;
		else
		{
			//������� ����������� ������
			size_t pb = str.find(v.first, pos);
			//���� ����������� ������� �� �����������:
			//�� ����� ����� ������ � ��������
			if (pe < pb || pb == string::npos) return false;
		}
	}
	return true;
}

string NS_Const::TConstJson::description(const JsonParams& val) noexcept(true)
{
	switch (val)
	{
	case JsonParams::Null: return "null";
	case JsonParams::False: return "�������� false";
	case JsonParams::True: return "�������� true";
	case JsonParams::DstFile: return "������ ���� ��������";
	case JsonParams::Sheet: return "������ ��������";
	case JsonParams::Cells: return "������ ��������� ��������� �����";
	case JsonParams::Method: return "������ ����� ���������";
	case JsonParams::SrcFile: return "������ ���� ��������";
	case JsonParams::DataArr: return "������ ��������� ������� ��� ���������";
	case JsonParams::DB_Config: return "���� � �����/���������� �������� ��� ����� � ��";
	case JsonParams::name: return "������������";
	case JsonParams::list_index: return "������ ��������";
	case JsonParams::col_id: return "������ ������ ID";
	case JsonParams::first_row: return "������ � ������� ���� ����������";
	case JsonParams::last_row: return "������ �� ������� ���� ����������";
	case JsonParams::filter: return "������ ������";
	case JsonParams::column_index: return "������ �������";
	case JsonParams::operation: return "��� ��������";
	case JsonParams::value: return "��������";
	case JsonParams::dst_index: return "������ ��� ���������� � ���������";
	case JsonParams::dst_insert_index: return "������ ��� ������� � ���������";
	case JsonParams::src_param_index: return "������ ��� ���������� ��������������� �������� � ���������";
	case JsonParams::src_val_index: return "������ ������������ �������� � ���������, ������� ����������� � ��������";
	case JsonParams::in_data_type: return "��� ������� ������";
	case JsonParams::out_data_type: return "��� ������, ������� ������������";
	case JsonParams::code: return "���/������������";
	case JsonParams::color_if_found: return "������ �����, ���� �������� �������";
	case JsonParams::color_not_found: return "������ �����, ���� �������� �� �������";
	case JsonParams::fill_type: return "������ ��� �������";
	case JsonParams::iftrue: return "��������� ��������, ���� ������� ���������";
	case JsonParams::iffalse: return "��������� ��������, ���� ������� �� ���������";
	case JsonParams::currency: return "������ ������";
	case JsonParams::rates: return "������ ����� �����";
	case JsonParams::pattern: return "������ ������";
	case JsonParams::fields: return "�������� ������ �������� �����";
	case JsonParams::empty_block: return "������ ������ ����";
	}
	return "��������� ��������" + asStr(val) + " �� ������!";
}

string NS_Const::TConstJson::asStr(const JsonParams& val) noexcept(true)
{
	switch (val)
	{
	case JsonParams::Null: return "null";
	case JsonParams::False: return "false";
	case JsonParams::True: return "true";
	case JsonParams::DstFile: return "DstFile";
	case JsonParams::Sheet: return "Sheet"; 
	case JsonParams::Cells: return "Cells";
	case JsonParams::Method: return "Method";
	case JsonParams::SrcFile: return "SrcFile";
	case JsonParams::DataArr: return "DataArr";
	case JsonParams::DB_Config: return "DB_Config";
	case JsonParams::name: return "name";
	case JsonParams::list_index: return "lst_indx";
	case JsonParams::col_id: return "col_id";
	case JsonParams::first_row: return "first_row";
	case JsonParams::last_row: return "last_row";
	case JsonParams::filter: return "fltr";
	case JsonParams::column_index: return "col_indx";
	case JsonParams::operation: return "operation";
	case JsonParams::value: return "value";
	case JsonParams::dst_index: return "dst_indx";
	case JsonParams::dst_insert_index: return "dst_ins_indx";
	case JsonParams::src_param_index: return "src_param_indx";
	case JsonParams::src_val_index: return "src_val_indx";
	case JsonParams::in_data_type: return "in_data_type";
	case JsonParams::out_data_type: return "out_data_type";
	case JsonParams::code: return "code";
	case JsonParams::color_if_found: return "color_if_found";
	case JsonParams::color_not_found: return "color_not_found";
	case JsonParams::fill_type: return "fill_type";
	case JsonParams::iftrue: return "iftrue";
	case JsonParams::iffalse: return "iffalse";
	case JsonParams::currency: return "currency";
	case JsonParams::rates: return "rates";
	case JsonParams::pattern: return "pattern";
	case JsonParams::fields: return "fields";
	case JsonParams::empty_block: return "empty_block";
	}
	return string();
}

bool NS_Const::TConstJson::inRange(const JsonParams& val, const JsonParams& b, const JsonParams& e) noexcept(true)
{
	if (val > b and val < e) return true;
	return false;
}

bool NS_Const::TConstJson::isTag(const JsonParams& val) noexcept(true)
{
	return isObjectTag(val) || isFileTag(val) || isFilterTag(val) || isCellTag(val) || isMethTag(val) || 
		isBalanceTag(val) || isImpDocsTag(val);
}

string& NS_Const::operator<<(string& str, const JsonParams& param)
{
	string tmp;
	if (param == JsonParams::Null)
		tmp = string();
	else
		tmp = TConstJson::asStr(param);
	if (str.empty())
		return str += tmp;
	char div = TConstCtrlSym::asChr(CtrlSym::point);
	str += div + tmp;
	return str;
}

string NS_Const::TConstJson::Concate(const std::vector<JsonParams>& arr) noexcept(true)
{
	if (arr.size() < 1) return string();
	string rslt;
	for (size_t i = 0; i < arr.size(); i++)
	{
		if (!rslt.empty() and i+1 < arr.size())
			rslt << arr[i];
	}
	return rslt;
}

string NS_Const::TConstJSMeth::asStr(const JSonMeth& val) noexcept(true)
{
	switch (val)
	{
		case JSonMeth::CompareRow: return "����� ���� ����� ������ ��������� �� ����� ���������";
		case JSonMeth::CompareCell: return "����� ������ ������ ������ ��������� �� ����� ���������";
		case JSonMeth::GetFromDB: return "��������� �������� �� ��";
		case JSonMeth::SendToDB: return "������ �������� � ��";
		case JSonMeth::GetRowIDByDB: return "��������� ������ �����, ����������� � ��������� ��";
		case JSonMeth::CompareCellChange: return "�������� ��������� � ������, ���� ���� ���������";
		case JSonMeth::InsertRowCompare: return "������� ������ � ��������, ���� ������� ������ � ����������";
	}
	return "��������� ����� �� ��������������!";
}

bool NS_Const::TConstJSMeth::HasSrcFileObj() const noexcept(false)
{
	switch (Value())
	{
		//������ ��� ��������� ������ �� ������ ������
		case JSonMeth::CompareRow:
		case JSonMeth::CompareCell:
		case JSonMeth::InsertRowCompare:
		case JSonMeth::CompareCellChange:
			return true;
		case JSonMeth::GetFromDB:
		case JSonMeth::SendToDB:
		case JSonMeth::GetRowIDByDB:
			return false;
		default:  
			throw NS_Logger::TLog("��������� �����: " + toStr() + " �� ��������������!", "TConstJSMeth::isComareMeth");
	}

}

string NS_Const::TConstJSCellFill::asStr(const NS_Const::JsonCellFill& val) noexcept(true)
{
	using NS_Const::JsonCellFill;
	switch (val)
	{
	case JsonCellFill::CurCell: return "������� ������� ������";
	case JsonCellFill::ID_All_Find: return "������� ������-��������������, ���� ��� ������ �������";
	case JsonCellFill::ID_More_One_Find: return "������� ������-��������������, ���� ���� ���� ���� ����������";
	case JsonCellFill::ID_And_CurCell:
		return "������� ������-��������������, ���� ���� ���� ���� ����������, � ������� ������ �� ��������� ������";
	default: return "����� �� ��������������!";
	}
	return string();
}

string NS_Const::TConstJSFilterOper::asStr(const NS_Const::JsonFilterOper& val) noexcept(true)
{
	using NS_Const::JsonFilterOper;
	switch (val)
	{
		case JsonFilterOper::Equal: return "==";
		case JsonFilterOper::NotEqual: return "!=";
		case JsonFilterOper::MoreThan: return ">";
		case JsonFilterOper::MoreEqualThan: return ">=";
		case JsonFilterOper::LessThan: return "<";
		case JsonFilterOper::LessEqualThan: return "<=";
		case JsonFilterOper::Like: return "like";
		case JsonFilterOper::LikeNoCase: return "lower(trim(like))";
		case JsonFilterOper::NotLike: return "!like";
		case JsonFilterOper::StrEqualNoCase: return "lower(trim(==))";
		case JsonFilterOper::isEmpty: return "is null";
		case JsonFilterOper::NotEmpty: return "is not null";
	}
	return "�������� �� ����������";
}

/*
template <typename Type>
bool NS_Const::TConstJSFilterOper::runBaseOperation(const Type& val1, const Type& val2,
	const NS_Const::JsonFilterOper& oper_code) noexcept(true)
{
	using NS_Logger::TLog;
	try
	{
		//���������� �������� ��������� � ����������� �� ����:
		switch (oper_code)
		{
			case JsonFilterOper::Equal: return val1 == val2;
			case JsonFilterOper::NotEqual: return val1 != val2;
			case JsonFilterOper::MoreThan: return val1 > val2;
			case JsonFilterOper::MoreEqualThan: return val1 >= val2;
			case JsonFilterOper::LessThan: return val1 < val2;
			case JsonFilterOper::LessEqualThan: return val1 <= val2;
			default:
			{
				TLog log("��������� ��������: ", "TConstJSFilterOper::runBaseOperation");
				log << asStr(oper_code) << " �� ��������� � ������ ������� �������� ����: ";
				const std::type_info& ti = typeid(val1);
				log << ti.name() << '\n';
				throw log;
			}
		}
	}
	catch (const std::exception& err)
	{
		TLog(err.what(), "TConstJSFilterOper::runBaseOperation").toErrBuff();
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("������ ���������� �������: ", "TConstJSFilterOper::runBaseOperation");
		log << asStr(oper_code) << " ��� ��������: " << val1 << " � " << val2 << '\n';
		log.toErrBuff();
	}
	return false;
}

template <typename Type>
bool NS_Const::TConstJSFilterOper::RunOperation(const Type& val1, const Type& val2,
	const NS_Const::JsonFilterOper& oper_code) noexcept(true)
{
	return runBaseOperation(val1, val2, oper_code);
}
/**/

bool NS_Const::TConstJSFilterOper::DoubleBaseOperation(double val1, double val2, const NS_Const::JsonFilterOper& oper_code)
noexcept(true)
{
	using NS_Logger::TLog;
	try
	{
		//���������� �������� ��������� � ����������� �� ����:
		switch (oper_code)
		{
		case JsonFilterOper::StrEqualNoCase:
		case JsonFilterOper::Equal: return val1 == val2;
		case JsonFilterOper::NotEqual: return val1 != val2;
		case JsonFilterOper::MoreThan: return val1 > val2;
		case JsonFilterOper::MoreEqualThan: return val1 >= val2;
		case JsonFilterOper::LessThan: return val1 < val2;
		case JsonFilterOper::LessEqualThan: return val1 <= val2;
		default:
		{
			TLog log("��������� ��������: ", "TConstJSFilterOper::DoubleBaseOperation");
			log << asStr(oper_code) << " �� ��������� � ������ ������� �������� ���� double!\n";
			throw log;
		}
		}
	}
	catch (const std::exception& err)
	{
		TLog(err.what(), "TConstJSFilterOper::DoubleBaseOperation").toErrBuff();
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("������ ���������� �������: ", "TConstJSFilterOper::DoubleBaseOperation");
		log << asStr(oper_code) << " ��� ��������: " << val1 << " � " << val2 << '\n';
		log.toErrBuff();
	}
	return false;
}

bool NS_Const::TConstJSFilterOper::BoolBaseOperation(bool val1, bool val2, const NS_Const::JsonFilterOper& oper_code)
noexcept(true)
{
	using NS_Logger::TLog;
	try
	{
		//���������� �������� ��������� � ����������� �� ����:
		switch (oper_code)
		{
		case JsonFilterOper::Equal: return val1 == val2;
		case JsonFilterOper::NotEqual: return val1 != val2;
		default:
		{
			TLog log("��������� ��������: ", "TConstJSFilterOper::BoolBaseOperation");
			log << asStr(oper_code) << " �� ��������� � ������ ������� �������� ���� bool!\n";
			throw log;
		}
		}
	}
	catch (const std::exception& err)
	{
		TLog(err.what(), "TConstJSFilterOper::BoolBaseOperation").toErrBuff();
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("������ ���������� �������: ", "TConstJSFilterOper::BoolBaseOperation");
		log << asStr(oper_code) << " ��� ��������: ";
		if (val1)
			log << "true ";
		else
			log << "false ";
		log << " � ";
		if (val2)
			log << "true";
		else
			log << "false\n";
		log.toErrBuff();
	}
	return false;
}

bool NS_Const::TConstJSFilterOper::IntBaseOperation(int val1, int val2, const NS_Const::JsonFilterOper& oper_code)
noexcept(true)
{
	using NS_Logger::TLog;
	try
	{
		//���������� �������� ��������� � ����������� �� ����:
		switch (oper_code)
		{
		case JsonFilterOper::Equal: return val1 == val2;
		case JsonFilterOper::NotEqual: return val1 != val2;
		case JsonFilterOper::MoreThan: return val1 > val2;
		case JsonFilterOper::MoreEqualThan: return val1 >= val2;
		case JsonFilterOper::LessThan: return val1 < val2;
		case JsonFilterOper::LessEqualThan: return val1 <= val2;
		default:
		{
			TLog log("��������� ��������: ", "TConstJSFilterOper::IntBaseOperation");
			log << asStr(oper_code) << " �� ��������� � ������ ������� �������� ���� int!\n";
			throw log;
		}
		}
	}
	catch (const std::exception& err)
	{
		TLog(err.what(), "TConstJSFilterOper::IntBaseOperation").toErrBuff();
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("������ ���������� �������: ", "TConstJSFilterOper::IntBaseOperation");
		log << asStr(oper_code) << " ��� ��������: " << val1 << " � " << val2 << '\n';
		log.toErrBuff();
	}
	return false;
}


bool NS_Const::TConstJSFilterOper::StringBaseOperation(const std::string& val1,
	const std::string& val2, const NS_Const::JsonFilterOper& oper_code) noexcept(true)
{
	using NS_Logger::TLog;
	//���� ����� ������������ ��������� � ������ ��������:
	if (oper_code == JsonFilterOper::isEmpty) 
		return val1.empty();
	//���� ����� ������������ �������� �� �� ������ ��������:
	if (oper_code == JsonFilterOper::NotEmpty)
		return !val1.empty();
	//���� ���� ���� ������ ������ - �����
	if (val1.empty() or val2.empty()) return false;
	switch (oper_code)
	{
	case JsonFilterOper::Equal: return val1 == val2;
	case JsonFilterOper::NotEqual: return val1 != val2;
	case JsonFilterOper::MoreThan: return val1 > val2;
	case JsonFilterOper::MoreEqualThan: return val1 >= val2;
	case JsonFilterOper::LessThan: return val1 < val2;
	case JsonFilterOper::LessEqualThan: return val1 <= val2;
	case JsonFilterOper::LikeNoCase:
	case JsonFilterOper::StrEqualNoCase:
		{
			string v1 = LowerCase(val1);
			Trim(v1);
			string v2 = LowerCase(val2);
			Trim(v2);
			if (oper_code == JsonFilterOper::LikeNoCase)
				return v1.find(v2, 0) != string::npos;
			else
				return v1 == v2;
		}
	case JsonFilterOper::Like: return val1.find(val2, 0) != 0;
	case JsonFilterOper::NotLike: return val1.find(val2, 0) == string::npos;
	default:
	{
		TLog log("��������� ��������: ", "TConstJSFilterOper::RunOperation");
		log << asStr(oper_code) << " �� ��������������!\n";
		log.toErrBuff();
	}
	}
	return false;
}
/**/