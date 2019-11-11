//���� ������������ ��� ������ ���� �� �������� ������:
//1. ��������� ������� excel-�����
//2. ��������� ������������ ���������� � ����� ������
//3. ��������� �������� ������� � excel-�����
//4. ��������� ������ �� ���� � ���������� �� � ����
//5. ��������� ����

//��� ���������� ������ � ��������� ������� - �������� UNICODE(��������� unicode), 
//�.�. ���� ������������ string �� wstring
#include <iostream>
#include <fstream>
#include "Logger.h"
#include "TuneParam.h"
#include "TSQLParser.h"
#include "TExcel.h"
#include "TOracle.h"
#include "TConverter.h"
#include "TConverter.cpp"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

//������� ��������� ���������� ��� ����������� � ��
NS_Oracle::TConnectParam getConnectParams(const NS_Tune::TUserData& ud) noexcept(false);

//������� ������ ���������� � ������ �� �����������:
void setSqlParamByTune(NS_Oracle::TStatement& sql, const NS_Tune::TSubParam& param, bool exit_on_err = true) noexcept(false);

//������� ���������� ���������� � sql-������� �� ����� ��������:
void setSqlParamsByTunes(const NS_Oracle::TStatement& sql, const NS_Tune::TUserData& ud) noexcept(false);

//������� ������ ������ �� ResultSet � excel-���� �� �������� ��������
bool ResultSet2Excel(NS_Oracle::TResultSet& rs, NS_Excel::TExcelBook& excl) noexcept(true);

//������� ������������ excel-����� �� ��������:
void CreateExcelFile(const NS_Tune::TUserData& param, NS_Oracle::TResultSet& rs) noexcept(true);

//������� �������� �������� ������ - ������ ������� �� ����� � ������������ � ����
bool CreateSimpleReport(const string& config_file) noexcept(true);

//������� ������������ ������:
bool CreateRepotByConfig(const string& directory) noexcept(true);


NS_Oracle::TConnectParam getConnectParams(const NS_Tune::TUserData& ud) noexcept(false)
{
	using NS_Oracle::TConnectParam;
	using NS_Tune::TuneField;
	TConnectParam result;
	if (ud.Empty()) return result;
	result.username = ud.getFieldByCode(TuneField::UserName);
	result.password = ud.getFieldByCode(TuneField::Password);
	result.tns_name = ud.getFieldByCode(TuneField::TNS);
	result.auto_commit = false;
	result.prefetch_rows = 200;
	return result;
}

void setSqlParamByTune(NS_Oracle::TStatement& sql, const NS_Tune::TSubParam& param, bool exit_on_err) noexcept(false)
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
		default: throw string("��������� ��� ������ �� ��������������!");
		}
	}
	catch (const oracle::occi::SQLException& err)
	{
		std::string s;
		s ="������ ��������� ��������: " + param.Value() + " ��� ���������: " + param.Comment() + '\n';
		s += err.what() + '\n';
		if (exit_on_err)
		{
			cerr << s;
			return;
		}
		else throw s;
	}
	catch (...)
	{
		std::string s;
		s = "�������������� ������ ��������� ��������: " + param.Value() + " ��� ���������: " + param.Comment() + '\n';
		if (exit_on_err)
		{
			cerr << s;
			return;
		}
		else throw s;
	}
}

void setSqlParamsByTunes(NS_Oracle::TStatement& sql, const NS_Tune::TUserData& ud) noexcept(false)
{
	using NS_Logger::TLog;
	//���� ������ sql - �����
	if (!sql.isValid()) TLog("�� �������� sql-�������: " + sql.getSQL()).raise(true, "setSqlParamsByTunes");
	string sql_text = sql.getSQL();
	if (sql_text.empty()) TLog("������ ����� sql-�������").raise(true, "setSqlParamsByTunes");
	//��������� ���������� ����������:
	for (const NS_Tune::TSubParam& p : ud.getParams())
		setSqlParamByTune(sql, p);
}

bool ResultSet2Excel(NS_Oracle::TResultSet& rs, NS_Excel::TExcelBook& excl) noexcept(true)
{
	using NS_Excel::TExcelBookSheet;
	using NS_Excel::TExcelCell;
	using NS_Excel::TExcelDate;
	using NS_Excel::TDataType;
	using NS_Oracle::TDataSetState;
	using NS_Oracle::TType;
	using NS_Oracle::UInt;
	using NS_Oracle::TDate;
	using NS_Oracle::SQLException;
	int act_index = excl.getActiveSheet();
	TExcelBookSheet sheet = excl.getSheetByIndex(act_index);
	UInt col_cnt = rs.getColumnsCnt();
	int row = excl.getActiveSheet() + 1;
	while (rs.Next())
	{
		for (UInt i = 1; i <= col_cnt; i++)
		{
			TExcelCell cell(row, i-1);
			switch (rs.getColumnType(i))
			{
			//����� � ��������� ������:
			case TType::OCCIDOUBLE:
			case TType::OCCIFLOAT:
			case TType::OCCINUMBER:
			case TType::OCCIINT:
			case TType::OCCI_SQLT_NUM:
				try
				{
//					if (sheet.getCellType(cell) == TDataType::)
					sheet.WriteAsNumber(cell, rs.getDoubleVal(i));
				}
				catch (const SQLException& err)
				{
					cerr << "������ ���������� ��������� �������� �������: " << i;
					cerr << ", ������: " << row << endl;
					cerr << err.what() << endl;
				}
				catch (...)
				{
					cerr << "�������������� ������ ������ ��������� �������� �������: " << i;
					cerr << ", ������: " << row << endl;
				}
					break;
			case TType::OCCI_SQLT_STR:
			case TType::OCCI_SQLT_CHR:
				try
				{
					sheet.WriteAsString(cell, rs.getStringVal(i));
				}
				catch (const SQLException& err)
				{
					cerr << "������ ���������� ������ �� �������: " << i;
					cerr << ", ������: " << row << endl;
					cerr << err.what() << endl;
				}
				catch (...)
				{
					cerr << "�� ������������ ������ ������ ������ �� �������: " << i;
					cerr << ", ������: " << row << endl;
				}
				break;
			case TType::OCCIBOOL:
				try
				{
					sheet.WriteAsBool(cell, rs.getIntVal(i));
				}
				catch (const SQLException& err)
				{
					cerr << "������ ���������� ����������� �������� �� �������: " << i;
					cerr << ", ������: " << row << endl;
					cerr << err.what() << endl;
				}
				catch (...)
				{
					cerr << "�� ������������ ������ ������ ����������� �������� �� �������: ";
					cerr << i << ", ������: " << row << endl;
				}
				break;
			case TType::OCCIDATE:
			case TType::OCCI_SQLT_DATE:
			case TType::OCCI_SQLT_DAT:
				try
				{
					if (sheet.isDate(cell))
					{
						TDate date = rs.getDateVal(i);
						TExcelDate tmp;
						date.getDate(tmp.year, tmp.month, tmp.day, tmp.hour, tmp.minute, tmp.sec);
						double dbl_date = excl.Date2Double(tmp);
						sheet.WriteAsNumber(cell, dbl_date);
					}
					else
						sheet.WriteAsString(cell, rs.getDateAsStrVal(i));
				}
				catch (const SQLException& err)
				{
					cerr << "������ ���������� ���� �� �������: " << i;
					cerr << ", ������: " << row << endl;
					cerr << err.what();
				}
				catch (...)
				{
					cerr << "�� ������������ ������ ������ ���� ��� �������: " << i;
					cerr << ", ������: " << row << endl;
				}
				break;
			default:
				cerr << "��������� ��� ������ � " << i << " ������� - �� ��������������!";
				break;
			}
		}
		row++;
	}
	return true;
}

void CreateExcelFile(const NS_Tune::TUserData& param, NS_Oracle::TResultSet& rs) noexcept(true)
{
	using NS_Excel::TExcelParam;
	using NS_Excel::TExcelBook;
	using NS_Tune::TuneField;
	using std::cerr;
	using std::endl;
	//������������� ��������� ���������� ������:
	TExcelParam exparam(param.getFieldByCode(TuneField::TemplateName), param.getFieldByCode(TuneField::OutFileName),
		param.getColumns());
	//������������ excel-���������:
	TExcelBook excl;
	if (excl.initByParam(exparam) == false)
	{
		cerr << "������ ��� ��������/������������ excel-���������!" << endl;
		return;
	}
	//���������� ������ � �����
	if (ResultSet2Excel(rs, excl) == false)
	{
		cerr << "������ ��� ������ ������ � excel-���� �� �������!" << endl;
		return;
	}
	if (excl.SaveToFile(exparam.getOutName()) == false)
	{
		cerr << "������ ��� ���������� ����� ������: " << exparam.getOutName() << endl;
		return;
	}
	cout << "����� ����������� � ������� � ����: " << exparam.getOutName() << endl;
}

bool CreateSimpleReport(const string& config_file) noexcept(true)
{
	using NS_Tune::TUserData;
	using NS_Tune::TuneField;
	using NS_Sql::TText;
	using NS_Oracle::TConnectParam;
	using NS_Oracle::TDBConnect;
	using NS_Oracle::TStatement;
	using NS_Oracle::TResultSet;

	using std::ifstream;
	using std::ios_base;
	using std::ostream;
	auto no_report = [](ostream& stream)->bool { if (stream) stream << "����� �� �����������!" << endl; return false; };
	TUserData config(config_file);
	//���� ������ �� �������� ��������� �� ��������� - �����:
	if (config.Empty())
	{
		cerr << "�� �������� ���� ��������: " << config_file << endl;
		return no_report(cerr);;
	}
	//����� ��������� ������ �� ����� ������������ - ��������� ����� sql-�������:
	TText sql;
	if (!config.getFieldByCode(TuneField::SqlText).empty())
		sql = TText(config.getFieldByCode(TuneField::SqlText));
	else
	{
		ifstream sql_txt_file(config.getFieldByCode(TuneField::SqlFile), ios_base::in);
		if (!sql_txt_file.is_open())
		{
			cerr << "������ �������� �����: " << config.getFieldByCode(TuneField::SqlFile) << endl;
			return no_report(cerr);
		}
		sql = TText(sql_txt_file);
		sql_txt_file.close();
	}
	if (sql.isEmpty())
	{
		cerr << "������ ��� ��������� SQL �������! ��������� ���� ��������: " << config_file << endl;
		return no_report(cerr);
	}
	//��������� ������ ��� ����������� � ��:
	TConnectParam connect_param = getConnectParams(config);
	//��������� ����������� � ��:
	TDBConnect connect(connect_param);
	if (connect.isValid())
	{
		//�������� sql-�������:
		TStatement st(connect, sql.toStr(), connect_param.prefetch_rows);
		//������������� ��������� �������:
		setSqlParamsByTunes(st, config);
		//��������� ������:
		TResultSet rs(st);
		//������� ������ ������ � excel �� ���������� �������:
		CreateExcelFile(config, rs);
		rs.close();
	}
	else
	{
		cerr << "����� �������� ���������� � �����: " << connect_param.tns_name << endl;
		return no_report(cerr);
	}
	return true;
}

bool CreateRepotByConfig(const string& directory) noexcept(true)
{
	using std::cerr;
	using std::endl;
	using std::vector;
	using std::string;
	using NS_Tune::TUserData;
	using NS_Excel::TExcelBook;
	if (directory.empty())
	{
		cerr << "������� ������ ���������� ��������� ������!" << endl;
		return false;
	}
	vector<string> name_arr;
	vector<TUserData> confg_arr;
	TExcelBook excl;
	//excl.initByParam
		//�������� ���� ������ �������� ��� ������ �� ��������� �����
		//��� ������� ����� ������� �������� ������������ ������ ��� ��������
		//��������� excel-�������� � ����
	return false;
}
