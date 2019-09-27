#ifndef TORACLE_H_
#define TORACLE_H_
//������������ ���� ����������� ��������� ��� ������ � �� Oracle
// � �������������� OCCI ����������
//FAQ �� OCCI:
// https://docs.oracle.com/en/database/oracle/oracle-database/19/lncpp/accessing-oracle-database-using-cplusplus.html#GUID-F273EF54-652C-4905-A60D-2368ADE31C81
//���� ����� ��� ddl/dml: https://www.geeksforgeeks.org/sql-ddl-dql-dml-dcl-tcl-commands/
//�������� ����� ������:
//https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/Data-Types.html#GUID-7B72E154-677A-4342-A1EA-C74C1EA928E6
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
	};

	//�������� ����� ��� ����� ���������:
	class TBaseEnv
	{
	private:
		Environment* env;
		TBaseEnv(const TBaseEnv& e);
		TBaseEnv& operator=(const TBaseEnv& e);
	protected:
		//������� ���������� � �����
		Connection* createConnection(const string& uname, const string& upass, const string& tns) noexcept(false);
		virtual bool Connect2DB(const string& uname, const string& upass, const string& tns) = 0;
		//������� ���������� � �����
		virtual bool closeConnection(Connection* c = nullptr);
	public:
		TBaseEnv() { env = Environment::createEnvironment(); };
		virtual ~TBaseEnv() { Environment::terminateEnvironment(env); };
		//������� ���������� � �����
		virtual bool createConnection(const TConnectParam& param) { return Connect2DB(param.username, param.password, param.tns_name); };
		virtual bool isValid(void) const { return env; };
	};
	
	//����� ���������� � ����� ������:
	class TDBConnect : public TBaseEnv
	{
	private:
		bool commit_on_close;
		Connection* connect;
		TDBConnect(const TDBConnect& c);
		TDBConnect(Connection* c);
		TDBConnect& operator=(const TDBConnect& c);
		//��������� ����������:
		static bool Commit(Connection* c) noexcept(true);
		//����� ����������:
		static bool RollBack(Connection* c) noexcept(true);
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
		virtual int getInt(UInt& paramIndx) const = 0;
		virtual double getDouble(UInt& paramIndx) const = 0;
		virtual float getFloat(UInt& paramIndx) const = 0;
		virtual string getString(UInt& paramIndx) const = 0;
		virtual TDate getDate(UInt& paramIndx) const = 0;
		virtual string getDateAsStr(UInt& paramIndx, const string& date_frmt = "DD.MM.YYYY") const = 0;
		//������� �������� �������� �����/����������:
		virtual bool isNull(UInt& paramIndx) const = 0;
		//���������/���������� ������������� ���������� ��� ������ �������� ���������/�������
		virtual void setExceptionOnNull(UInt& paramIndx, bool flg = false) = 0;
	};

	//�����-��������� �������:
	class TStatement: public TBaseSet 
	{
	private:
		static const int prefetch_rows = 1;//����� ����� ���������� ��� ��������� � ��
		Connection* connect;//��������� ����������
		Statement* statement;//��������� �� sql-�������
		UInt prefetch_cnt;//����� �������������� ���������� �����
		//��������� �����������
		TStatement(Connection* c, Statement* s);
		TStatement& operator=(const TStatement& st);
		//������� ������� �� ����������:
		bool createStatement(Connection* c, const string& sql = "", bool auto_commit = false);
		//��������� ���������������� sql-�������:
		TSQLState execute(const string& sql = "") noexcept(true);
		//��������� ����� ���������� ������� �� ���� ���������:
		void setPrefetch();
		TStatement(Connection* c, const string& sql = "", bool auto_commit = false,
			UInt prefetch = prefetch_rows);
	public:
		//������������� ������� � ������� �� ����������:
		explicit TStatement(const TDBConnect& dbc, const string& sql = "", UInt prefetch = prefetch_rows);
		~TStatement() { if (isValid()) connect->terminateStatement(statement); };
		/*������������ ������� �� �������� ������*/
		//���������� �������
		bool isValid() const { return (connect && statement); };
		//��������� ��������� sql-�������:
		TSQLState getState() const { return (isValid() ? statement->status() : TSQLState::UNPREPARED); };
		//������� ��� ������ � �����������:
		//��������� �������� ��������� �� �������:
		int getInt(UInt& paramIndx) const { return statement->getInt(paramIndx); };
		double getDouble(UInt& paramIndx) const { return statement->getDouble(paramIndx); };
		float getFloat(UInt& paramIndx) const { return statement->getFloat(paramIndx); };
		string getString(UInt& paramIndx) const { return statement->getString(paramIndx); };
		TDate getDate(UInt& paramIndx) const { return statement->getDate(paramIndx); };
		string getDateAsStr(UInt& paramIndx, const string& date_frmt = "DD.MM.YYYY") const;
		//����� ������� ��������� �������� ���������:
		void setInt(UInt& paramIndx, int value) { statement->setInt(paramIndx, value); };
		void setDouble(UInt& paramIndx, double value) { statement->setDouble(paramIndx, value); };
		void setFloat(UInt& paramIndx, float value) { statement->setFloat(paramIndx, value); };
		void setString(UInt& paramIndx, const string& value) { statement->setString(paramIndx, value); };
		void setDate(UInt& paramIndx, const TDate& date) { statement->setDate(paramIndx, date); };
		void setDateAsString(UInt& paramIndx, const string& date, const string& date_frmt = "DD.MM.YYYY");
		//������� �������� �������� �����/����������:
		virtual bool isNull(UInt& paramIndx) const { return statement->isNull(paramIndx); };
		//���������/���������� ������������� ���������� ��� ������ �������� ���������/�������
		void setExceptionOnNull(UInt& paramIndx, bool flg = false) { statement->setErrorOnNull(paramIndx, flg); };
		//����������� ����������� ������� ������
		//��������� ����� ��������� �����:
		UInt getPrefetchRowCount() const { return prefetch_cnt; };
		//��������� ����� ��������� ����� �� ���� ��������� � ��
		void setPrefetchRowCount(UInt row_cnt);
		//�������� ����������
		bool Commit() { return TDBConnect::Commit(connect); };
		//����� ����������
		bool RollBack() { return TDBConnect::RollBack(connect); };
		//���������� ����� sql-�������:
		void setSQL(const string& sql) { if (isValid()) statement->setSQL(sql); };
		//�������� ����� �������� �������:
		string getSQL(void) const { return (isValid() ? statement->getSQL() : string()); };
		//��������� ������ ������� ��������:
		UInt getCurrentIteration() const;
		//��������� ������������ ����� dml-��������
		UInt getProcessedCntRows() const;
		//��������� ������ �� �������� �� ���� sql-�������:
		bool executeSQL(const string& sql = "") noexcept(true);
		//��������� ������ �� ������������ �������:
		ResultSet* getResultSet() noexcept(true);
		//��������� ������ ��� DQL:
		ResultSet* executeQuery(const string& sql = "") noexcept(true);
		//��������� ������ ��� DML
		UInt executeDML(const string& sql = "") noexcept(true);
	};

	//����� �������������� �����:
	class TResultSet: public TBaseSet
	{
	private:
		ResultSet* result;//��������� �� ���������� ����� ������
		TMetaDataArr meta;//���������� ��� �������
		//������ ����������
		TResultSet& operator=(const TResultSet& rs);
		//������ �����������:
		TResultSet(const TResultSet& rs);
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
		int getInt(UInt& paramIndx) const { return result->getInt(paramIndx); };
		double getDouble(UInt& paramIndx) const { return result->getDouble(paramIndx); };
		float getFloat(UInt& paramIndx) const { return result->getFloat(paramIndx); };
		string getString(UInt& paramIndx) const { return result->getString(paramIndx); };
		TDate getDate(UInt& paramIndx) const { return result->getDate(paramIndx); };
		string getDateAsStr(UInt& paramIndx, const string& date_frmt = "DD.MM.YYYY") const;
		//������� �������� �������� �����/����������:
		bool isNull(UInt& paramIndx) const { return result->isNull(paramIndx); };
		//���������/���������� ������������� ���������� ��� ������ �������� ���������/�������
		void setExceptionOnNull(UInt& paramIndx, bool flg = false) { result->setErrorOnNull(paramIndx, flg); };
		//����������� ����������:
		//��������� ������� ����������� ������:
		TDataSetState getState() const { return result->status(); };
		//�������� �� �������� ������ ������:
		bool isTruncated(UInt& paramIndx) const { return result->isTruncated(paramIndx); };
		//��������� �������
		TDataSetState Next(UInt RowsCnt = 1) { return (isValid() ? result->status() : TDataSetState::END_OF_FETCH); };
		//��������� ����� ��������� ����� truncate
		int getPreTruncationLenght(UInt& paramIndx) const { return result->preTruncationLength(paramIndx); };
		//�������� ������ ��� ���������� ����� ���������:
		void setExceptionOnTruncate(UInt& paramIndx, bool flg = false) { result->setErrorOnTruncate(paramIndx, flg); };
		//������� ��������� ����� ������� � �������
		UInt getColumnsCount(void) const { return meta.size(); };
		//��������� ���� ������ ��� �������:
		TType getColumnType(UInt& colIndx) const noexcept(false);
		//������������� ������� �� ������������ �������:
	};

}


#endif