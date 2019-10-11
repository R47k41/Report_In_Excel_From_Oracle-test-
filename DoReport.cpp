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

//������� ������ ������ �� ResultSet � excel-����
void ResultSet2Excel(const NS_Oracle::TResultSet& rs, const NS_Excel::TExcel& excl);

//������� ������������ excel-����� �� ��������:


//������� �������� �������� ������ - ������ ������� �� ����� � ������������ � ����
bool CreateSimpleReport(const string& config_file) noexcept(true);


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
			NS_Tune::Str2Type(param.Value(), val);
			sql.setIntVal(par_id, val);
			return;
		}
		case DataType::Double:
		{
			double val = 0.0;
			NS_Tune::Str2Type(param.Value(), val);
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
	//���� ������ sql - �����
	if (!sql.isValid()) throw string("�� �������� sql-�������!");
	string sql_text = sql.getSQL();
	if (sql_text.empty()) throw string("������ ����� sql-�������");
	//��������� ���������� ����������:
	for (const NS_Tune::TSubParam& p : ud.getParams())
		setSqlParamByTune(sql, p);
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

		//��������� resultSet
		//rs.close();
		//��������� sql-�������:
		//st.close();
		//��������� ����������:
		//connect.closeConnection();
	}
	else
	{
		cerr << "����� �������� ���������� � �����: " << connect_param.tns_name << endl;
		return no_report(cerr);
	}
	return true;
}