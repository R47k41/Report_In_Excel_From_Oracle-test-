#ifndef TORACLE_H_
#define TORACLE_H_
//������������ ���� ����������� ��������� ��� ������ � �� Oracle
// � �������������� OCCI ����������
//FAQ �� OCCI:
// https://docs.oracle.com/en/database/oracle/oracle-database/19/lncpp/introduction-to-occi.html#GUID-99077609-BFCC-4FF7-9D23-C45623DDD465
//�������� ����� ������:
//https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/Data-Types.html#GUID-7B72E154-677A-4342-A1EA-C74C1EA928E6
#include <string>
#include "occi.h"

namespace NS_Oracle
{
	
	//using namespace oracle::occi;
	using std::string;
	using oracle::occi::Environment;
	using oracle::occi::Connection;
	using oracle::occi::Statement;
	
	//�����-��������� ������:
	class TStatement
	{
		private:
		public:
	};
	//�����-��������� ����������
	class TConnection
	{
		private:
			Connection* cnct;//����������
		public:
			TStatement(Connection* )

	};

	//����� ���������� ��� �������������� � �� oracle
	class TOracle
	{
	private:
		Environment* env;//���������� ���������
		Connection* connect;//������� ����������
	public:
		TOracle(const string& uname = "", const string& pass = "", const string& connect_str = "");
		virtual ~TOracle();
	};
}


#endif