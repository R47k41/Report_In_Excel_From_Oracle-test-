//описание функций для модуля Constants.h
#include <iostream>
#include <iterator>
#include <algorithm>
#include <sstream>
#include <cstring>
#include "TConstants.h"

using std::string;

//преобразование в нижний регистр:
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


template <typename T, T min_val, T max_val>
bool NS_Const::TConstant<T, min_val, max_val>::isValid(const T& a, const T& b, bool exit_on_err) const noexcept(false)
{
	if (inRange(a, b))
		return true;
	if (exit_on_err)
		return false;
	throw string("Ошибка попадания в диапазон значений!");
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
	}
	return string();
}

string NS_Const::TConstExclTune::asStr(const TExclBaseTune& val) noexcept(true)
{
	switch (val)
	{
	case TExclBaseTune::xlt: return ".xlt";
	case TExclBaseTune::xls: return ".xls";
	//расширение по умолчанию
	case TExclBaseTune::DefExt:
	case TExclBaseTune::xlsx: 
		return ".xlsx";
	//имя файла и листа по умолчанию
	case TExclBaseTune::DefName: return "Отчет";
	case TExclBaseTune::DefSh: return "Отчет";
	//разделитель страниц одного отчета
	case TExclBaseTune::PageDelimiter: return " стр. ";
	}
	return string();
}

string NS_Const::TConstExclTune::getFileExtention(const string& val) noexcept(true)
{
	if (val.empty()) return string();
	const char delimeter = TConstCtrlSym::asChr(CtrlSym::point);
	//получение имени файла из настроек:
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
	}
	return  string();
}

string NS_Const::TConstReportCode::getName() const
{
	switch (Value())
	{
	case ReportCode::RIB_DOCS_FOR_PERIOD: return "Фоминых: Документы РИБ за период";
	case ReportCode::DOCS_MF_SF_FOR_PERIOD: return "Кривошеева: Документы за период МФ + СФ";
	case ReportCode::REPAYMENT_FOR_DATE: return "Скачкова: Гашения за период МФ + СФ";
	case ReportCode::POTREB_CRED_BY_FILE: return "Борисова: Потребительские кредиты МФ по файлу";
	case ReportCode::CRED_CASE_MF: return "Ермакова: Кредитный портфель МФ";
	case ReportCode::BALANCE_LIST: return "Ермакова: Ведомость остатков";
	case ReportCode::BALANCE_SUA: return "Борисова: Ведомость остатков для загрузки в СУА";
	case ReportCode::FULL_CRED_REPORT: return "Ермакова: Полный кредитный портфель";
	case ReportCode::FULL_CRED_REPORT_SUA: return "Борисова: Отчет по всем кредитам для СУА";
	case ReportCode::NBKI_NP: return "Борисова: НБКИ Физ. лица";
	case ReportCode::NBKI_JP: return "Борисова: НБКИ Юр. лица";
	case ReportCode::NBKI_APPLY: return "Борисова: НБКИ фиксация изменений";
	case ReportCode::CLOSE_DAY: return "Закрытие баланса";
	case ReportCode::LOAD_FROM_FILE: return "Загрузка проводок из файла в OraBank";
	case ReportCode::FILE_COMPARE_RIB: return "Фоминых: Заполнение остатков по РИБ из файла со счетами";
	case ReportCode::FILE_COMPARE_RTBK: return "Борисова: Заполнение остатков по РТБК из файла со счетами";
	case ReportCode::EXCEL_SET_DATA_FROM_BASE: return "Excel: Заполнение из БД";
	}
	return  string();
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
	case TSql::EOC: //return ";";//символ окончания команды
		return TConstCtrlSym(CtrlSym::EndCommand).toStr();
	case TSql::D4L: //return ",";//разделитель строк в select/from/order/group
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
		throw string(string("Для блока: ") + val.c_str() + " имеется больше одного разделителя!");
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
		//формируем список запрещенных символов: '(', ')', '"'
		arr = { {TConstCtrlSym(CtrlSym::lbkt).toStr()[0], TConstCtrlSym(CtrlSym::rbkt).toStr()[0]},
			{TConstCtrlSym(CtrlSym::crwn).toStr()[0], TConstCtrlSym(CtrlSym::crwn).toStr()[0]} };
}

bool NS_Const::TSymGroup::IsCorrectSym(const string& str, std::size_t pos, const std::size_t cnt) const noexcept(false)
{
	using std::size_t;
	using std::isalnum;
	if (pos == 0 and str.size() > (pos + cnt + 1)
		and !isalnum(str[pos + cnt + 1])) return true;
	//проверяем, что это не часть слова:
	if (pos != 0 and isalnum(str[pos - 1]) and
		(pos + cnt + 1) < str.size() and isalnum(str[pos + cnt + 1])) return false;
	//проверяем не вложен ли разделитель в подзапрос:
	char ch = TConstCtrlSym::asChr(CtrlSym::crwn);
	for (const Syms& v : arr)
	{
		size_t pe = str.find(v.second, pos);
		//если закрывающих элеменетов нет
		if (pe == string::npos) continue;
		else
		{
			//находим открывающий символ
			size_t pb = str.find(v.first, pos);
			//если открывающий элемент за закрывающим:
			//то слово часть строки в кавычках
			if (pe < pb || pb == string::npos) return false;
		}
	}
	return true;
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
	return isObjectTag(val) || isFileTag(val) || isFilterTag(val) || isCellTag(val) || isMethTag(val);
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
		case JSonMeth::CompareColor: return "Окрашивание цветом при выполнении условия";
		case JSonMeth::CompareIns: return "Вставка данных при выполнении условия";
		case JSonMeth::GetFromDB: return "Получение значений из БД";
		case JSonMeth::SendToDB: return "Запись значения в БД";
		case JSonMeth::GetRowIDByDB: return "Формируем массив строк, относящихся к указанной БД";
	}
	return "Указанный метод не обрабатывается!";
}

string NS_Const::TConstJSCellFill::asStr(const NS_Const::JsonCellFill& val) noexcept(true)
{
	using NS_Const::JsonCellFill;
	switch (val)
	{
	case JsonCellFill::CurCell: return "Заливка текущей ячейки";
	case JsonCellFill::ID_All_Find: return "Заливка ячейки-идентификатора, если все ячейки совпали";
	case JsonCellFill::ID_More_One_Find: return "Заливка ячейки-идентификатора, если есть хоть одно совпадение";
	case JsonCellFill::ID_And_CurCell:
		return "Заливка ячейки-идентификатора, если есть хоть одно совпадение, и заливка каждой не совпавшей ячейки";
	default: return "Метод не обрабатывается!";
	}
	return string();
}

//явное инстанцирование для шаблонов
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