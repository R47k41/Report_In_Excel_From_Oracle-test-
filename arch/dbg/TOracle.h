#ifndef TORACLE_H_
#define TORACLE_H_
//������������ ���� ����������� ��������� ��� ������ � �� Oracle
// � �������������� OCCI ����������
//FAQ �� OCCI:
// https://docs.oracle.com/en/database/oracle/oracle-database/19/lncpp/accessing-oracle-database-using-cplusplus.html#GUID-F273EF54-652C-4905-A60D-2368ADE31C81
//���� ����� ��� ddl/dml: https://www.geeksforgeeks.org/sql-ddl-dql-dml-dcl-tcl-commands/
//�������� ����� ������:
//https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/Data-Types.html#GUID-7B72E154-677A-4342-A1EA-C74C1EA928E6
//��� ����������:
//https://docs.oracle.com/database/121/LNCPP/metadata.htm#LNCPP20275
#include <string>
#include <vector>
#include "occi.h"

namespace NS_Oracle
{
	
	//using namespace oracle::occi;
	using std::string;
	using oracle::occi::Environment;
	using oracle::occi::Connection;
	using oracle::occi::Statement;
	using oracle::occi::ResultSet;
	using oracle::occi::MetaData;
	using oracle::occi::SQLException;
	using EnvironmentPtr = Environment*;
	using ConnectionPtr = Connection*;
	using StatementPtr = Statement*;
	using ResultSetPtr = ResultSet*;
	//using MetaDataPtr = MetaData *;
	using TDate = oracle::occi::Date;
	using TSQLState = oracle::occi::Statement::Status;
	using TDataSetState = oracle::occi::ResultSet::Status;
	using TType = oracle::occi::Type;//��� ������ ��� ���������� OCCI
	using UInt = unsigned int;
	using TMetaDataArr = OCCI_STD_NAMESPACE::vector<MetaData>;
	
	//��������� ��� ���������� ����������
	struct TConnectParam
	{
		string username;//��� ������������
		string password;//������� ��� �����
		string tns_name;//������ ����������
		bool auto_commit;//������������ ����������
		unsigned int prefetch_rows;//����� ����� ���������� �� ���� ��������� � ��
		//�������� �� �������
		bool isEmpty() const { return username.empty() or password.empty() or tns_name.empty(); }
	};

	//�������� ����� ��� ����� ���������:
	class TBaseEnv
	{
	private:
		TBaseEnv(const TBaseEnv& e);
		TBaseEnv& operator=(const TBaseEnv& e);
	protected:
		EnvironmentPtr env;
		//������� ���������� � �����
		Connection* crtConnection(const string& uname, const string& upass, const string& tns) noexcept(false);
		virtual bool Connect2DB(const string& uname, const string& upass, const string& tns) = 0;
		//������� ���������� � �����
		virtual bool closeConnection(ConnectionPtr c = nullptr);
	public:
		TBaseEnv() { env = Environment::createEnvironment(); };
		virtual ~TBaseEnv() { Environment::terminateEnvironment(env); };
		//������� ���������� � �����
		virtual bool crtConnection(const TConnectParam& param) { return Connect2DB(param.username, param.password, param.tns_name); };
		virtual bool isValid(void) const { return env; };
		virtual const EnvironmentPtr getenviroument() const { return env; }
	};
	
	//����� ���������� � ����� ������:
	class TDBConnect : public TBaseEnv
	{
	private:
		bool commit_on_close;
		ConnectionPtr connect;
		TDBConnect(const TDBConnect& c);
		TDBConnect(ConnectionPtr c);
		TDBConnect& operator=(const TDBConnect& c);
		//��������� ����������:
		static bool Commit(ConnectionPtr c) noexcept(true);
		//����� ����������:
		static bool RollBack(ConnectionPtr c) noexcept(true);
		friend class TStatement;
	public:
		TDBConnect(const string& uname, const string& upas, const string& utns, bool commit_on_exit = false);
		explicit TDBConnect(const TConnectParam& param);
		virtual ~TDBConnect() { closeConnection(); };
		//���������� � ��
		bool Connect2DB(const string& uname, const string& upass, const string& tns) noexcept(true);
		//��������������� �������� ����������:
		bool closeConnection();
		//�������� ��������� ���������� � ��
		bool isValid() const { return (TBaseEnv::isValid() && connect); };
	};

	//������� ����� ��� ������ ������ - ���������� ������� �������
	class TBaseSet
	{
	private:
		//��������� ����������:
		TBaseSet& operator=(const TBaseSet& b);
	public:
		//�������������
		TBaseSet() {};
		//����������:
		virtual ~TBaseSet() {};
		//���������� ������:
		virtual bool isValid() const = 0;
		//����� ������� ��������� �������� ���������:
		virtual int getIntVal(UInt paramIndx) const = 0;
		virtual double getDoubleVal(UInt paramIndx) const = 0;
		virtual float getFloatVal(UInt paramIndx) const = 0;
		virtual string getStringVal(UInt paramIndx) const = 0;
		virtual TDate getDateVal(UInt paramIndx) const = 0;
		virtual string getDateAsStrVal(UInt paramIndx, const string& date_frmt = "DD.MM.YYYY") const noexcept(true) = 0;
		//������� �������� �������� �����/����������:
		virtual bool isNullVal(UInt paramIndx) const = 0;
		//���������/���������� ������������� ���������� ��� ������ �������� ���������/�������
		virtual void setExceptionOnNull(UInt paramIndx, bool flg = false) = 0;
	};

	//�����-��������� �������:
	class TStatement: public TBaseSet 
	{
	private:
		static const int prefetch_rows = 1;//����� ����� ���������� ��� ��������� � ��
		const EnvironmentPtr environment;//��������� �� ���������
		ConnectionPtr connect;//��������� ����������
		StatementPtr statement;//��������� �� sql-�������
		UInt prefetch_cnt;//����� �������������� ���������� �����
		//��������� �����������
		TStatement(ConnectionPtr c, StatementPtr s);
		TStatement& operator=(const TStatement& st);
		//������� ������� �� ����������:
		bool crtStatement(ConnectionPtr c, const string& sql = "", bool auto_commit = false);
		//��������� ���������������� sql-�������:
		TSQLState Execute(const string& sql = "") noexcept(true);
		//��������� ����� ���������� ������� �� ���� ���������:
		void setPrefetchVal();
		TStatement(EnvironmentPtr env, ConnectionPtr c, const string& sql = "", bool auto_commit = false,
			UInt prefetch = prefetch_rows);
	public:
		//������������� ������� � ������� �� ����������:
		explicit TStatement(const TDBConnect& dbc, const string& sql = "", UInt prefetch = prefetch_rows);
		//������� �������� sql-�������:
		bool close() noexcept(false);
		~TStatement();
		/*������������ ������� �� �������� ������*/
		//���������� �������
		bool isValid() const { return (connect && statement); };
		//��������� ��������� sql-�������:
		TSQLState getState() const { return (isValid() ? statement->status() : TSQLState::UNPREPARED); };
		//������� ��� ������ � �����������:
		//��������� �������� ��������� �� �������:
		int getIntVal(UInt paramIndx) const { return statement->getInt(paramIndx); };
		double getDoubleVal(UInt paramIndx) const { return statement->getDouble(paramIndx); };
		float getFloatVal(UInt paramIndx) const { return statement->getFloat(paramIndx); };
		string getStringVal(UInt paramIndx) const { return statement->getString(paramIndx); };
		TDate getDateVal(UInt paramIndx) const { return statement->getDate(paramIndx); };
		string getDateAsStrVal(UInt paramIndx, const string& date_frmt = "DD.MM.YYYY") const noexcept(true);
		//����� ������� ��������� �������� ���������:
		void setIntVal(UInt paramIndx, int value) { statement->setInt(paramIndx, value); };
		void setDoubleVal(UInt paramIndx, double value) { statement->setDouble(paramIndx, value); };
		void setFloatVal(UInt paramIndx, float value) { statement->setFloat(paramIndx, value); };
		void setStringVal(UInt paramIndx, const string& value) { statement->setString(paramIndx, value); };
		void setDateVal(UInt paramIndx, const TDate& date) { statement->setDate(paramIndx, date); };
		void setDateAsStringVal(UInt paramIndx, const string& date, const string& date_frmt = "DD.MM.YYYY");
		//������� �������� �������� �����/����������:
		virtual bool isNullVal(UInt paramIndx) const { return statement->isNull(paramIndx); };
		//���������/���������� ������������� ���������� ��� ������ �������� ���������/�������
		void setExceptionOnNull(UInt paramIndx, bool flg = false) { statement->setErrorOnNull(paramIndx, flg); };
		//����������� ����������� ������� ������
		//��������� ����� ���������� �������:
		int getParamsCnt(const string& ch) const;
		//��������� ����� ��������� �����:
		UInt getPrefetchRowCnt() const { return prefetch_cnt; };
		//��������� ����� ��������� ����� �� ���� ��������� � ��
		void setPrefetchRowCnt(UInt row_cnt);
		//�������� ����������
		bool Commit() { return TDBConnect::Commit(connect); };
		//����� ����������
		bool RollBack() { return TDBConnect::RollBack(connect); };
		//���������� ����� sql-�������:
		void setSQL(const string& sql) { if (isValid()) statement->setSQL(sql); };
		//�������� ����� �������� �������:
		string getSQL(void) const { return (isValid() ? statement->getSQL() : string()); };
		//��������� ������ ������� ��������:
		UInt getCurIteration() const;
		//��������� ������������ ����� dml-��������
		UInt getProcessedCntRows() const;
		//��������� ������ �� �������� �� ���� sql-�������:
		bool ExecuteSQL(const string& sql = "") noexcept(true);
		//��������� ������ �� ������������ �������:
		ResultSetPtr getResultSetVal() noexcept(true);
		//��������� ������ ��� DQL:
		ResultSetPtr executeQuery(const string& sql = "") noexcept(true);
		//��������� ������ ��� DML
		UInt executeDML(const string& sql = "") noexcept(true);
	};

	//����� �������������� �����:
	//https://docs.oracle.com/en/database/oracle/oracle-database/19/lncpp/resultset-class.html#GUID-9E0C0A68-57B7-4C77-8865-A306BF6953CA
	class TResultSet: public TBaseSet
	{
	private:
		ResultSetPtr result;//��������� �� ���������� ����� ������
		TMetaDataArr meta;//���������� ��� �������
		//������ ����������
		TResultSet& operator=(const TResultSet& rs);
		//������ �����������:
		TResultSet(const TResultSet& rs);
		//�������� ������������ ������� ������ ��� ��������� NULL � �������
		void setSize4NullCol();
		//������������� ���������� �������:
		void InitMetaData();
	protected:
		//������� ResultSet:
		void cancel() { result->cancel(); };
	public:
		//����������� ������� sql-�������
		explicit TResultSet(TStatement& query);
		//��������������� ������ ������:
		~TResultSet();
		//����������� ������ ����������:
		//�������� ����������:
		bool isValid() const { return result; };
		//����� ������� ��������� �������� ���������:
		//������� ��� ���������� ���������� � 1!!!
		int getIntVal(UInt paramIndx) const noexcept(false) { return result->getInt(paramIndx); };
		double getDoubleVal(UInt paramIndx) const noexcept(false) { return result->getDouble(paramIndx); };
		float getFloatVal(UInt paramIndx) const noexcept(false) { return result->getFloat(paramIndx); };
		string getStringVal(UInt paramIndx) const noexcept(false) { return result->getString(paramIndx); };
		TDate getDateVal(UInt paramIndx) const noexcept(false) { return result->getDate(paramIndx); };
		string getDateAsStrVal(UInt paramIndx, const string& date_frmt = "DD.MM.YYYY") const noexcept(true);
		//������� �������� �������� �����/����������:
		bool isNullVal(UInt paramIndx) const { return result->isNull(paramIndx); };
		//���������/���������� ������������� ���������� ��� ������ �������� ���������/�������
		void setExceptionOnNull(UInt paramIndx, bool flg = false) { result->setErrorOnNull(paramIndx, flg); };
		//����������� ����������:
		//��������� ������� ����������� ������:
		TDataSetState getState() const { return result->status(); };
		//�������� �� �������� ������ ������:
		bool isTruncatedVal(UInt paramIndx) const { return result->isTruncated(paramIndx); };
		//��������� �������
		TDataSetState Next(UInt RowsCnt = 1) { return (isValid() ? result->next() : TDataSetState::END_OF_FETCH); };
		//��������� ����� ��������� ����� truncate
		int getPreTruncLenght(UInt paramIndx) const { return result->preTruncationLength(paramIndx); };
		//�������� ������ ��� ���������� ����� ���������:
		void setExceptOnTruncate(UInt paramIndx, bool flg = false) { result->setErrorOnTruncate(paramIndx, flg); };
		//������� ��������� ����� ������� � �������
		size_t getColumnsCnt(void) const { return meta.size(); };
		//��������� ���� ������ ��� �������:
		TType getColumnType(UInt colIndx) const noexcept(false);
		//�������� resultSet
		bool close(void) noexcept(false);
		//��������� ����������� ����������� ����� ���� ��� ���������� ������ �� �������:
		void setMaxColumnSize(UInt colIndx, UInt size = 1) noexcept(false) { result->setMaxColumnSize(colIndx, size); }
	};
}

#endif