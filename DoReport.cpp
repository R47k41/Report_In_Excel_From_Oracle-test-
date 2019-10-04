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

bool CreateSimpleReport(const string& config_file) noexcept(true);


bool CreateSimpleReport(const string& config_file) noexcept(true)
{
	using NS_Tune::TUserData;
	using NS_Tune::TuneField;
	using NS_Sql::TText;
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
	//��������� � sql-������ ��������:
	//�� ������ ��������, ����� �������� ����� � ����� ������ �������
	//���� � ������� �������: param1="���� ������"
	//� �������� :1 � ������ sql-������� �� ��������� param1
	return true;
}