//�������� ������� ��� ������ Constants.h
#include <iostream>
#include <sstream>
#include <cstring>
#include "TConstants.h"

using std::string;

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
	if (str.find(toStr(), 0) != string::npos) return true;
	return false;
}

string NS_Const::TConstField::toStr() const
{
	switch (Value())
	{
	case TuneField::DataBase: return "[DATA BASE]";
	case TuneField::UserName: return "UserName";
	case TuneField::Password: return "Password";
	case TuneField::TNS: return "TNS";
	case TuneField::Report: return "[REPORT]";
	case TuneField::OutDirectory: return "OutDirectory";
	case TuneField::OutFileName: return "OutFileName";
	case TuneField::SheetName: return "SheetName";
	case TuneField::AddDateToName: return "AddDateToName";
	case TuneField::TemplateName: return "TemplateName";
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
	case TuneField::Block_End: return "[END]";
	default: return string();
	}
};

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

string NS_Const::TConstType::toStr() const
{
	switch (Value())
	{
	case DataType::String: return "string";
	case DataType::Integer: return "integer";
	case DataType::Double: return "double";
	case DataType::Date: return "date";
	}
	return string();
}

string NS_Const::TConstExclTune::toStr() const
{
	switch (Value())
	{
	case TExclBaseTune::xlt: ".xlt";
	case TExclBaseTune::xls:
	case TExclBaseTune::DefExt:
		return ".xls";
	case TExclBaseTune::xlsx: return ".xlsx";
	case TExclBaseTune::DefName: return "�����";
	case TExclBaseTune::DefSh: return "�����";
	}
	return string();
}

string NS_Const::TConstReportCode::toStr() const
{
	switch (Value())
	{
	case ReportCode::RIB_DOCS_FOR_PERIOD: return "�������: ��������� ��� �� ������";
	case ReportCode::DOCS_MF_SF_FOR_PERIOD: return "����������: ��������� �� ������ �� + ��";
	case ReportCode::REPAYMENT_FOR_DATE: return "��������: ������� �� ������ �� + ��";
	case ReportCode::POTREB_CRED_BY_FILE: return "��������: ��������������� ������� �� �� �����";
	case ReportCode::CRED_CASE_MF: return "��������: ��������� �������� ��";
	case ReportCode::BALANCE_LIST: return "��������: ��������� ��������";
	case ReportCode::FULL_CRED_REPORT: return "��������: ������ ��������� ��������";
	case ReportCode::NBKI: return "���� �������";
	case ReportCode::NBKI_APPLY: return "���� �������� ���������";
	case ReportCode::LOAD_FROM_FILE: return "�������� �������� �� ����� � OraBank";
	case ReportCode::FILE_COMPARE: return "��������� excel-������";
	}
	return  string();
}

string NS_Const::TConstCtrlSym::toStr() const
{
	switch (Value())
	{
	case CtrlSym::EOL: return "\0";
	case CtrlSym::Space: return " ";
	case CtrlSym::EndCommand:
	case CtrlSym::semicolon: return ";";
	case CtrlSym::EndCol: return ",";
	case CtrlSym::lbkt: return "(";
	case CtrlSym::rbkt: return ")";
	case CtrlSym::crwn:
	case CtrlSym::quotes: return "\"";
	case CtrlSym::Tab: return "\t";
	case CtrlSym::NL: return "\n";
	case CtrlSym::colon: return ":";
	case CtrlSym::dash: return "\\";
	case CtrlSym::quane: return "=";
	case CtrlSym::rangle: return ">";
	case CtrlSym::langle: return "<";
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
	char ch = TConstCtrlSym(CtrlSym::crwn).toStr()[0];
	for (const Syms& v : arr)
	{
		size_t p = str.find(v.second, pos);
		if (p == string::npos) continue;
		else
		{
			p = str.find(v.first);
			if (v.first == ch)
				if (p > pos) continue;
				else return false;
			if (p == string::npos || p < pos) continue;
			else return false;
		}
	}
	return true;
}

//����� ��������������� ��� ��������
//http://www.cyberforum.ru/cpp-beginners/thread1798717.html#post9488987
template NS_Const::TConstant<NS_Const::TuneField, NS_Const::TuneField::Empty, NS_Const::TuneField::Last>;
template NS_Const::TConstant<NS_Const::DataType, NS_Const::DataType::ErrorType, NS_Const::DataType::Last>;
template NS_Const::TConstant<NS_Const::TExclBaseTune, NS_Const::TExclBaseTune::Empty, NS_Const::TExclBaseTune::Last>;
template NS_Const::TConstant<NS_Const::ReportCode, NS_Const::ReportCode::Empty, NS_Const::ReportCode::Last>;
template NS_Const::TConstant<NS_Const::TSql, NS_Const::TSql::Empty, NS_Const::TSql::Last>;
template NS_Const::TConstant<NS_Const::CtrlSym, NS_Const::CtrlSym::Empty, NS_Const::CtrlSym::Last>;
