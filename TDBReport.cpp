//������ ����������� ����������� ��� TDBReport
#include <iostream>
#include <fstream>
#include "TDBReport.h"
#include "TSQLParser.h"
#include "TConverter.h"
#include "Logger.hpp"


using std::string;
using NS_Logger::TLog;
using NS_Excel::TExcelBook;
using NS_ExcelReport::TSheetReport;
using NS_ExcelReport::TDataBaseSheetReport;
using NS_ExcelReport::TReport;
using NS_Const::TuneField;
using NS_Oracle::TConnectParam;

const int DEF_TEMPL_SH_INDX = 0;

void raise_app_err(const TLog& log, bool as_raise = true);

void raise_app_err(const TLog& log, bool as_raise)
{
	as_raise ? throw log : log.toErrBuff();
}

TSheetReport::TSheetReport(NS_Excel::TExcelBook& book_link, const NS_Tune::TUserTune& config): book(book_link), sheet(nullptr), tune(config)
{
	SetSheetByTune();
}

TSheetReport::TSheetReport(TExcelBook& ref_book, const NS_Excel::TExcelBookSheet& ref_sheet, const NS_Tune::TUserTune& ref_tune):
	book(ref_book), sheet(ref_sheet), tune(ref_tune)
{
	//���� �������� ������:
	if (!ref_sheet.isValid())
		//������� ��
		SetSheetByTune();
}


bool TSheetReport::SetSheetByTune(const string& name) noexcept(true)
{
	using NS_Excel::TStrArr;
	const bool active_sheet = true;
	//�������� ������������� ����������
	if (tune.Empty())
	{
		TLog("������ ������ �� ����� ��������!", "TSheetReport::SetSheetTune").toErrBuff();
		return false;
	}
	//��������� ������ �� ������
	string tmp_val = tune.getTemplateFile();
	string sh_name = name;
	if (sh_name.empty()) sh_name = tune.getOutSheet();
	//���� ������ �� ������ - ������� ��������
	if (tmp_val.empty())
	{
		//��������� ������������ 
		TStrArr head = tune.getColumns();
		book.setHeaderByStrArr(head, false, sh_name);
		sheet = book.getActiveSheet();
	}
	//���� ������ ������ - ������ ���
	else
	{
		if (book.setSheetByTemplate(tmp_val, sh_name, DEF_TEMPL_SH_INDX, active_sheet))
		{
			sheet = book.getActiveSheet();
		}
/*
		book.loadSheetOnly(tmp_val, templ_index_sheet, true);
		sheet = book.getLastSheet(active_sheet);
		sheet.setName(sh_name);
/**/
	}
	if (!sheet.isValid())
		return false;
	return true;
}

bool TSheetReport::CreateNewPage(size_t cur_val, bool byRows)
{ 
	//�� ������� ��� ������ ��� ���������� ����� ��������
	if (!byRows)
	{
		size_t max_val = book.MaxColsCount();
		if (cur_val > max_val)
		{
			TLog log("��������� ����� �������� � ������ �� ����� �����! ����� �������� � �������: ", "WriteFromResultSet");
			log << cur_val << TLog::NL << "����� �������� � excel-�����: " << max_val;
			throw log;
		}
	}
	else
	{
		size_t max_val = book.MaxRowsCount();
		if (cur_val >= max_val)
		{
			int indx = book.getActiveSheetIndx() + 1;
			string sh_delimeter = NS_Const::TConstExclTune::asStr(NS_Const::TExclBaseTune::PageDelimiter);
			//������ ��� ����� ��������
			string new_name = sheet.getName() + sh_delimeter + NS_Converter::toStr(indx+1);
			//���������� ��������
			SetSheetByTune(new_name);
			return true;
		}
	}
	return false;
}

TConnectParam TDataBaseSheetReport::getConnectParam(const NS_Tune::TUserTune& param, int prefetch) noexcept(false)
{
	TConnectParam result;
	if (param.Empty()) throw TLog("��������� ������ �� ���������!", "TDataBaseSheetReport::getConnectParam()");
	result.username = param.getFieldValueByCode(TuneField::UserName);
	result.password = param.getFieldValueByCode(TuneField::Password);
	result.tns_name = param.getFieldValueByCode(TuneField::TNS);
	result.auto_commit = false;
	result.prefetch_rows = prefetch;
	return result;
}

template <typename T>
string TDataBaseSheetReport::getSqlText(bool by_str, const string& str) noexcept(false)
{
	using std::ifstream;
	//��������� ������ �������� sql-�����: ���� ��� ������
	if (by_str)
	{
		T sql(str);
		return NS_Sql::AsString(sql);
	}
	else
	{
		ifstream sql_txt_file(str, std::ios_base::in);
		if (!sql_txt_file.is_open())
		{
			throw TLog("������ �������� �����: " + str, "TDataBaseSheetReport::getSqlText");
		}
		T sql(sql_txt_file);
		sql_txt_file.close();
		return NS_Sql::AsString(sql);
	}
	return string();
}

void TDataBaseSheetReport::setSqlParamByTune(NS_Oracle::TStatement& sql, const NS_Tune::TSubParam& param, bool use_raise) noexcept(false)
{
	using NS_Tune::DataType;
	using NS_Converter::toType;
	//���� �������� �� ������:
	if (param.Value().empty()) return;
	//��������� ������� �� ������ ��� ��������:
	try
	{
		int par_id = param.ID();
		switch (param.DataType())
		{
		case DataType::Integer:
		{
			int val = 0;
			toType(param.Value(), &val);
			sql.setIntVal(par_id, val);
			return;
		}
		case DataType::Double:
		{
			double val = 0.0;
			toType(param.Value(), &val);
			sql.setDoubleVal(par_id, val);
			return;
		}
		case DataType::Date:
			sql.setDateAsStringVal(par_id, param.Value());
			return;
		case DataType::String:
			sql.setStringVal(par_id, param.Value());
			return;
		default: 
			throw TLog("��������� ��� ������ " + NS_Const::TConstType(param.DataType()).toStr() + " �� ��������������!", "TDataBaseSheetReport::setSqlParamByTune");
		}
	}
	catch (const oracle::occi::SQLException& err)
	{
		TLog log("������ ��������� ��������: " + param.Value() + " ��� ���������: " + param.Comment() + TLog::NL, "TDataBaseSheetReport::setSqlParamByTune");
		log << err.what() << TLog::NL;
		raise_app_err(log, use_raise);
	}
	catch (const TLog& er)
	{
		raise_app_err(er, use_raise);
	}
	catch (...)
	{
		TLog log("�������������� ������ ��������� ��������: " + param.Value() + " ��� ���������: " + param.Comment() + TLog::NL);
		raise_app_err(log, use_raise);
	}
}

void TDataBaseSheetReport::setSqlParamsByTune(NS_Oracle::TStatement& sql, const NS_Tune::TUserTune& param) noexcept(false)
{
	//���� ������ sql - �����
	if (!sql.isValid()) throw TLog("�� �������� sql-�������: " + sql.getSQL(), "setSqlParamsByTunes");
	string sql_text = sql.getSQL();
	if (sql_text.empty()) throw TLog("������ ����� sql-�������", "setSqlParamsByTunes");
	//��������� ���������� ����������:
	for (const NS_Tune::TSubParam& p : param.getParams())
		setSqlParamByTune(sql, p);
}

NS_Const::DataType TDataBaseSheetReport::convertOraType(const NS_Oracle::TType& type) noexcept(true)
{
	using NS_Oracle::TType;
	using NS_Const::DataType;
	using NS_Const::TConstType;
	switch (type)
	{
	//����� � ��������� ������:
	case TType::OCCINUMBER:
	case TType::OCCIINT:
		return DataType::Integer;
	case TType::OCCIDOUBLE:
	case TType::OCCIFLOAT:
	case TType::OCCI_SQLT_NUM:
		return DataType::Double;
	case TType::OCCI_SQLT_STR:
	case TType::OCCI_SQLT_CHR:
	case TType::OCCI_SQLT_RDD:
	case TType::OCCI_SQLT_AFC:
	case TType::OCCI_SQLT_VST:
	case TType::OCCI_SQLT_AVC:
	case TType::OCCICHAR:
		return DataType::String;
	case TType::OCCIBOOL:
		return DataType::Boolean;
	case TType::OCCIDATE:
	case TType::OCCI_SQLT_DATE:
	case TType::OCCI_SQLT_DAT:
		return DataType::Date;
	}
	return DataType::ErrorType;
}

void TDataBaseSheetReport::setCellByResultSet(const NS_Oracle::TResultSet& rs, const NS_Excel::TExcelCell& cell) noexcept(false)
{
	using NS_Oracle::TType;
	using NS_Oracle::UInt;
	using NS_Oracle::SQLException;
	using NS_Oracle::TDate;
	using NS_Excel::TExcelDate;
	using NS_Const::DataType;
	if (!rs.isValid()) throw TLog("�� �������� ������ ResultSet!", "TDataBaseSheetReport::setCellByResultSet");
	//��-�� ������������ ������ ������� ��� excel � oracle
	UInt i = cell.getCol() + 1;
	const DataType dt = convertOraType(rs.getColumnType(i));
	try
	{
		switch (dt)
		{
			case DataType::Integer:
			case DataType::Double:
				sheet.WriteAsNumber(cell, rs.getDoubleVal(i));
				break;
			case DataType::String:
				sheet.WriteAsString(cell, rs.getStringVal(i));
				break;
			case DataType::Boolean:
				sheet.WriteAsBool(cell, rs.getIntVal(i));
				break;
			case DataType::Date:
				if (sheet.isDate(cell))
				{
					TDate date = rs.getDateVal(i);
					TExcelDate tmp;
					date.getDate(tmp.year, tmp.month, tmp.day, tmp.hour, tmp.minute, tmp.sec);
					double dbl_date = book.Date2Double(tmp);
					sheet.WriteAsNumber(cell, dbl_date);
				}
				else
					sheet.WriteAsString(cell, rs.getDateAsStrVal(i));
				break;
			default:
				TLog log("��������� ��� ������ � ", "TDataBaseSheetReport::setCellByResultSet");
				log << i << " ������� - �� ��������������!" << TLog::NL;
				log.toErrBuff();
				break;
		}
	}
	catch (const SQLException& err)
	{
		TLog log("������ ���������� ", "TDataBaseSheetReport::setCellByResultSet");
		log << NS_Const::TConstType(dt).toStr() << " ������ �� ������� :";
		log << i << ", ������: " << cell.getRow() << TLog::NL;
		log << err.what() << TLog::NL;
		log.toErrBuff();
	}
	catch (...)
	{
		TLog log("�� ������������ ������ ������ ������ �� �������: ", "TDataBaseSheetReport::setCellByResultSet");
		log << i << ", ������: " << cell.getRow() << TLog::NL;
		log.toErrBuff();
	}
}

bool TDataBaseSheetReport::WriteFromResultSet(NS_Oracle::TResultSet& rs) noexcept(true)
{
	using NS_Oracle::UInt;
	using NS_Excel::TExcelCell;
	try
	{
		UInt col_cnt = rs.getColumnsCnt();
		//���� ����� ������� ��������� ����������� excel - �������
		CreateNewPage(col_cnt, false);
		size_t row = sheet.getLastRow();
		while (rs.Next())
		{
			for (UInt i = 1; i <= col_cnt; i++)
			{
				TExcelCell cell(row, i-1);
				setCellByResultSet(rs, cell);
			}
			row++;
			//���� ��������� ����� ����� �� ��������
			//������� ����� ��������
			if (CreateNewPage(row, true)) 
				row = sheet.getFirstRow() + 1;
		}
		return true;
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
	}
	catch (const NS_Oracle::SQLException& er)
	{
		TLog(er.getMessage(), "TDataBaseSheetReport::WriteFromResultSet").toErrBuff();
	}
	catch (...)
	{
		TLog("������������� ������ ������ ������ �� ���� �� ���� excel-���������!", "TDataBaseSheetReport::WriteFromResultSet").toErrBuff();
	}
	return false;
}

void TDataBaseSheetReport::FillSheetBySql(NS_Oracle::TDBConnect& db, const string& sql_txt, int prefetch)
{
	if (sql_txt.empty())
		throw TLog("������ ������������ ������ - ������ ����� �������!", "TDataBaseSheetReport::FillSheetBySql");
	if (!db.isValid())
		throw TLog("������ ������������ ������ - ������ ��� ����������� � ��!", "TDataBaseSheetReport::FillSheetBySql");
	//�������� sql-�������:
	TStatement st(db, sql_txt, prefetch);
	//������������� ��������� �������:
	setSqlParamsByTune(st, tune);
	//��������� ������:
	TResultSet rs(st);
	//������� ������ ������ � excel �� ���������� �������:
	WriteFromResultSet(rs);
	//��������� �������� ������ �������:
	rs.close();
}

bool TDataBaseSheetReport::useSqlParse() const noexcept(false)
{
	return tune.useFlag(TuneField::UseSqlParser);
}

bool TDataBaseSheetReport::isDQLFirst() const noexcept(false)
{
	return tune.useFlag(TuneField::SqlFirst);
}

string TDataBaseSheetReport::getSqlByTune(bool use_parse, bool by_str, const string& str) noexcept(true)
{
	string sql;
	if (use_parse)
		sql = getSqlText<NS_Sql::TText>(by_str, str);
	else
		sql = getSqlText<NS_Sql::TSimpleSql>(by_str, str);
	return sql;
}

void TDataBaseSheetReport::CrtBySqlLine(NS_Oracle::TDBConnect& db, const string& sql_line, int prefetch) noexcept(false)
{
	bool use_parse = useSqlParse();
	string sql = getSqlByTune(use_parse, true, sql_line);
	//������������ ������ ��� ��������
	FillSheetBySql(db, sql, prefetch);
}

void TDataBaseSheetReport::CrtBySqlFiles(NS_Oracle::TDBConnect& db, int prefetch) noexcept(false)
{
	using std::vector;
	vector<string> sql_lst = tune.getDQLFileLst();
	if (sql_lst.size() < 1)
		throw TLog("������ ����� sql-�������� � ����������!", "TDataBaseSheetReport::CrtBySqlFiles");
	bool use_parse = useSqlParse();
	//���������� ������� ������� �� ������:
	for (const string& sql_file : sql_lst)
	{
		string sql = getSqlByTune(use_parse, false, sql_file);
		//���������� DQL-������� � ���������� ������� �����
		FillSheetBySql(db, sql, prefetch);
	}
}

size_t TDataBaseSheetReport::executeDML(NS_Oracle::TDBConnect& db, const NS_Tune::TUserTune& param,
	const string& dml, bool use_comit) noexcept(false)
{
	//��������� ������ �������:
	//�������� dml-�������:
	TStatement st(db, dml);
	//������������� ��������� �������:
	setSqlParamsByTune(st, param);
	//���������� �������:
	size_t result = 0;
	result = st.executeDML();
	if (result > 0 and use_comit)
	{
		TLog log("������ ", "executeDML");
		log << dml << "\n��������� " << result << " �����!\n";
		log.toErrBuff();
		st.Commit();
	}
	return result;
}

size_t TDataBaseSheetReport::runDML4Directory(NS_Oracle::TDBConnect& db, const NS_Tune::TUserTune& param,
	bool use_comit) noexcept(false)
{
	using std::vector;
	//��������� ������ ������:
	vector<string> dml_lst = param.getDMLFileLst();
	if (dml_lst.size() < 1)
		throw TLog("������ ����� dml-������ � ����������!", "TDataBaseSheetReport::runDML4Directory");
	//������� ������ ����� ������������ �����:
	size_t cnt = 0;
	//���������� ������� ������� �� ������:
	for (const string& dml : dml_lst)
	{
		size_t tmp = 0;
		//��������� ������ dml-�������
		string dml_txt = getSqlByTune(false, false, dml);
		//���������� dml-�������
		tmp = executeDML(db, param, dml_txt, use_comit);
		if (tmp == 0)
			TLog("������ " + dml + " �� ������ ����������!", "TDataBaseSheetReport::runDML4Directory").toErrBuff();
		else
			cnt += tmp;
	}
	return cnt;
}

bool  TDataBaseSheetReport::crtSheet() noexcept(true)
{
	using std::vector;
	try
	{
		if (tune.Empty())
			throw TLog("������ ������������ ������ - ������ ���� ��������!", "TDataBaseSheetReport::crtReport");
		if (!sheet.isValid())
			throw TLog("������ ������������ ������ - �� ������� �������� ��� ������!", "TDataBaseSheetReport::crtReport");
		if (tune.isDQLEmpty())
			throw TLog("�� ������� �� ������ SQL-�������!", "TDataBaseSheetReport::crtReport");
		//��������� �������� ��� ����������� � ��:
		TConnectParam cp = getConnectParam(tune, prefetch_rows);
		//������������ � ��:
		TDBConnect db(cp);
		bool dql_first = isDQLFirst();
		//���� ������ ���� ��������� DML-�������
		if (!dql_first) runDML(db, tune);
		//��������� ����������� �� sql-������� �� �����
		string sql = tune.getFieldValueByCode(TuneField::SqlText);
		if (!sql.empty())
			CrtBySqlLine(db, sql, cp.prefetch_rows);
		else
			CrtBySqlFiles(db, cp.prefetch_rows);
		if (dql_first) runDML(db, tune);
		return true;
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
	}
	catch (const NS_Oracle::SQLException& er)
	{
		TLog log(string(er.what()), "TDataBaseSheetReport::crtReport");
		log.toErrBuff();
	}
	catch (...)
	{
		TLog("����� �� �����������! �� ������������ ������!", "TDataBaseSheetReport::crtReport").toErrBuff();
	}
	return false;
}

//���������� DML �������:
size_t TDataBaseSheetReport::runDML(NS_Oracle::TDBConnect& db, const NS_Tune::TUserTune& param, bool use_comit) noexcept(true)
{
	try
	{
		//�������� ���������� � ����� ��������
		if (!db.isValid()) throw TLog("����������� � �� �� �������!", "TDataBaseSheetReport::runDML");
		if (param.Empty()) throw TLog("������ ���� ��������!", "TDataBaseSheetReport::runDML");
		if (param.isDMLEmpty()) throw TLog("��� DML-������ ��� ����������!", "TDataBaseSheetReport::runDML");
		//��������� dml-������ �������:
		string dml = param.getFieldValueByCode(TuneField::DMLText);
		//��������� ������� �� ����� ������� � ������:
		size_t result = 0;
		if (!dml.empty())
			//���������� dml-������� �� ������:
			result = executeDML(db, param, dml, use_comit);
		else
			result = runDML4Directory(db, param, use_comit);
		return result;
	}
	catch (const NS_Oracle::SQLException& er)
	{
		TLog(er.what(), "TDataBaseSheetReport::runDML").toErrBuff();
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ��� ���������� DML-�������!", "TDataBaseSheetReport::runDML");
	}
	return 0;
}

void TReport::saveReport(TExcelBook& book, const string& file_name) const noexcept(false)
{
	//�������� ������������� ����������:
	string path = config.getOutPath();
	if (TSharedTune::CheckPath(path, true))
		//���������� �����:
		book.SaveToFile(file_name);
	else
		throw TLog("��������� ����������: " + path + " �� �������!\n����� �� ��������!",
			"TReport::One_Report_For_Each_Config");
}

void TReport::One_Report_For_Each_Config() const noexcept(false)
{
	using NS_Const::TuneField;
	using std::vector;
	using NS_Logger::TLog;
	using NS_Excel::TExcelBook;
	using StrArr = vector<string>;
	//��������� ������ ���������������� ������
	StrArr conf_lst = config.getConfFileLst(true);
	if (conf_lst.empty())
		throw TLog("������ ������ ���������������� ������!", "TReport::One_Report_For_Each_Config");
	//��������� ����� ��������� �����:
	string tmp = config.getOutFile();
	//��� ������� ����� ��������� ������������ ������ ��� �����
	for (const string& sub_tune : conf_lst)
	{
		//������������ ����� ��������:
		TUserTune tune(config, sub_tune);
		if (!tune.Empty())
		{
			//�������� excel-���������
			TExcelBook book(tune.getOutFileBySheet());
			//��������� ����� ��� config-�����:
			TDataBaseSheetReport sh(book, tune);
			//���� ����� �� �����������:
			if (!sh.crtSheet())
			{
				TLog log("����� ��� ����� " + sub_tune + " �� �����������!", "TReport::One_Report_For_Each_Config");
				log.toErrBuff();
			}
			//����� ��������� ����:
			else
				saveReport(book);
		}
	}
}

void TReport::One_Sheet_By_One_Config() const noexcept(false)
{
	using NS_Const::TuneField;
	using std::vector;
	using NS_Logger::TLog;
	using NS_Excel::TExcelBook;
	using StrArr = vector<string>;
	//��������� ������ ���������������� ������
	StrArr conf_lst = config.getConfFileLst(true);
	if (conf_lst.empty())
		throw TLog("������ ������ ���������������� ������!", "TReport::One_Sheet_By_One_Config");
	//��������� ���� ��������� �����:
	string tmp = config.getOutFile();
	//�������� excel-���������
	TExcelBook book(tmp);
	size_t fail_cnt = 0;
	//��� ������� ����� ��������� ������������ ������ ��� �����
	for (const string& sub_tune : conf_lst)
	{
		//������������ �������� ������:
		TUserTune tune(config, sub_tune);
		//��������� ����� ��� config-�����:
		TDataBaseSheetReport sh(book, tune);
		//���� ����� �� �����������:
		if (!sh.crtSheet())
		{
			TLog log("����� ��� ����� " + sub_tune + " �� �����������!", "TReport::One_Sheet_By_One_Config");
			log.toErrBuff();
			fail_cnt++;
		}
	}
	if (fail_cnt >= conf_lst.size())
		throw TLog("��� ������������ ������ �������� ������!", "TReport::One_Sheet_By_One_Config");
	else//���������� �����, ���� ���� ��� ���������
		saveReport(book);
}

//����� ���� ������, ��� ��� ������ ����� �������� ����� ���� ��������� ��������
//����� ���������� ���������� �������� � ������� �� ���������� ������
void TReport::One_Sheet_By_Many_Statement() const noexcept(false)
{
	using NS_Const::TuneField;
	using std::vector;
	using NS_Logger::TLog;
	using NS_Excel::TExcelBook;
	using StrArr = vector<string>;
	//��������� ������ ���������������� ������
	StrArr conf_lst = config.getConfFileLst(true);
	if (conf_lst.empty())
		throw TLog("������ ������ ���������������� ������!", "TReport::One_Sheet_By_Many_Statement");
	//��������� ���� ��������� �����:
	string tmp = config.getOutFile();
	//�������� excel-���������
	TExcelBook book(tmp);
	//�������������� ��������, ������� ����� �������:
	TExcelBookSheet sheet(nullptr);
	//����� ������ ��� ������������ ������:
	size_t fail_cnt = 0;
	for (const string& sub_tune: conf_lst)
	{
		//�������� ����� ��������:
		TUserTune tune(config, sub_tune);
		//��������� ����� ��� config-�����:
		TDataBaseSheetReport sh(book, sheet, tune);
		//���� ����� �� �����������:
		if (!sh.crtSheet())
		{
			TLog log("����� ��� ����� " + sub_tune + " �� �����������!", "TReport::One_Sheet_By_Many_Statement");
			log.toErrBuff();
			fail_cnt++;
		}
		if (!sheet.isValid())	sheet = book.getActiveSheet();
	}
	if (fail_cnt >= conf_lst.size())
		throw TLog("��� ������������ ������ �������� ������!", "TReport::One_Sheet_By_Many_Statement");
	else//���������� �����, ���� ���� ��� ���������
		saveReport(book);
}

size_t TReport::runDML_By_Tune(bool use_comit) const noexcept(false)
{
	using std::vector;
	using NS_Logger::TLog;
	using StrArr = vector<string>;
	//��������� ������ ���������������� ������
	StrArr conf_lst = config.getConfFileLst(true);
	if (conf_lst.empty())
		throw TLog("������ ������ ���������������� ������!", "TReport::One_Sheet_By_Many_Statement");
	//����� ������������ ����� dml-��������
	size_t result = 0;
	for (const string& sub_tune : conf_lst)
	{
		//��������� ���������� �� ����� ��������
		TUserTune tune(config, sub_tune);
		//��������� ���������� �����������:
		TConnectParam cp = TDataBaseSheetReport::getConnectParam(tune);
		//��� ������ �������� ��������� ����������� � ��:
		TDBConnect db(cp);
		//��������� dml-�������:
		size_t cnt = 0;
		cnt = TDataBaseSheetReport::runDML(db, tune, use_comit);
		if (cnt > 0) result += cnt;
	}
	return result;
}

void TReport::Create_Report_By_Code(const NS_Const::ReportCode& code) const
{
	using NS_Const::ReportCode;
	using NS_Const::TConstReportCode;
	using NS_Logger::TLog;
	//������������ ���� ������ � �� ������������� ������:
	auto raise_err = [](const ReportCode& rp)->TLog 
	{
		TLog log("����� ", "TReport::Create_Report_By_Code");
		log << TConstReportCode(rp).getName() << " �� ����������!" << TLog::NL;
		return log;
	};
	switch (code)
	{
	//������ ���������� �� ���������� ���������� sql-������� �� ����:
	case ReportCode::RIB_DOCS_FOR_PERIOD:
	case ReportCode::CRED_CASE_MF:
	case ReportCode::POTREB_CRED_BY_FILE:
	case ReportCode::BALANCE_LIST:
	case ReportCode::DOCS_MF_SF_FOR_PERIOD:
		One_Report_For_Each_Config();
		break;
	//case ReportCode::DOCS_MF_SF_FOR_PERIOD:
	case ReportCode::REPAYMENT_FOR_DATE:
		One_Sheet_By_One_Config();
		break;
		//����� ������� �� ������ ����������� ��������� �������� �� ���� �������� � ���� ����:
	case ReportCode::NBKI_NP:
	case ReportCode::NBKI_JP:
		One_Sheet_By_Many_Statement();
		break;
		//���������� �������� ���������:
	case ReportCode::NBKI_APPLY:
	case ReportCode::CLOSE_DAY:
		runDML_By_Tune(true);
		break;
	//������ ��������� �������� + ����������� � excel-������
	case ReportCode::FULL_CRED_REPORT:
		throw raise_err(code);
		break;
	//�������� ������ � oracle �� excel
	case ReportCode::LOAD_FROM_FILE:
		throw raise_err(code);
		break;
	//��������� ������ excel
	case ReportCode::FILE_COMPARE:
		throw raise_err(code);
		break;
	default:
		throw raise_err(code);
	}
}

bool TReport::Execute() const noexcept(true)
{
	using NS_Logger::TLog;
	using NS_Const::TConstReportCode;
	using NS_Const::ReportCode;
	try
	{
		//��������� �������������� ���� ������:
		ReportCode code = TConstReportCode::getIDByCode(config.getMainCode(false), ReportCode::Empty, ReportCode::Last);
		Create_Report_By_Code(code);
		return true;
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ��� ������������ ������!", "TReport::Execute").toErrBuff();
	}
	return false;
}