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
#include "occi.h"

namespace NS_Oracle
{
	
	//using namespace oracle::occi;
	using std::string;
	using oracle::occi::Environment;
	using oracle::occi::Connection;
	using oracle::occi::Statement;
	using oracle::occi::ResultSet;
	using TDate = oracle::occi::Date;
	using TSQLState = oracle::occi::Statement::Status;
	using UInt = unsigned int;
	
	//��������� ��� ���������� ����������
	struct TConnectParam
	{
		string username;//��� ������������
		string password;//������� ��� �����
		string tns_name;//������ ����������
		bool auto_commit;//������������ ����������
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
		bool Connect2DB(const string& uname, const string& upass, const string& tns, bool commit_on_exit = false) noexcept(true);
		//��������������� �������� ����������:
		bool closeConnection();
		//�������� ��������� ���������� � ��
		bool isValid() const { return (TBaseEnv::isValid() && connect); };
	};

	//������� ����� ��� sql-�������
	class TBaseSet
	{
	protected:
		Statement* statement;//��������� �� sql-�������
		UInt pre_fetch_cnt;//����� �������������� ���������� �����
	private:
		static const int pre_fetch_rows = 200;//����� ����� ���������� ��� ��������� � ��
		//��������� �����������
		TBaseSet(const TBaseSet& b);
		//��������� ����������:
		TBaseSet& operator=(const TBaseSet& b);
		//��������� ���������� ����� �� ���� ���������(�������)
		virtual void setPrefetch() { if (isValid()) statement->setPrefetchRowCount(pre_fetch_cnt); };
	public:
		//�������������
		explicit TBaseSet(Statement* st, UInt prefetch = pre_fetch_rows) : statement(st), pre_fetch_cnt(prefetch) { setPrefetch(); };
		//����������:
		virtual ~TBaseSet() { statement = nullptr; };
		//���������� ������:
		virtual bool isValid() const { return (statement ? true : false); };
		//��������� ��������� ������:
		virtual TSQLState getState() const { return (isValid() ? statement->status() : TSQLState::UNPREPARED); };
		//����� ������� ��������� �������� ���������:
		virtual int getInt(UInt& paramIndx) const { return statement->getInt(paramIndx); } = 0;
		virtual double getDouble(UInt& paramIndx) const { return statement->getDouble(paramIndx); } = 0;
		virtual float getFloat(UInt& paramIndx) const { return statement->getFloat(paramIndx); } = 0;
		virtual string getString(UInt& paramIndex) const { return statement->getString(paramIndex); } = 0;
		virtual TDate getDate(UInt& paramIndx) const { return statement->getDate(paramIndx); } = 0;
		virtual string getDateAsStr(UInt& paramIndx, const string& date_frmt = "DD.MM.YYYY") const = 0;
		//����� ������� ��������� �������� ���������:
		virtual void setInt(UInt& paramIndx, int value) { statement->setInt(paramIndx, value); } = 0;
		virtual void setDouble(UInt& paramIndx, double value) { statement->setDouble(paramIndx, value); } = 0;
		virtual void setFloat(UInt& paramIndx, float value) { statement->setFloat(paramIndx, value); } = 0;
		virtual void setString(UInt& paramIndx, const string& value) { statement->setString(paramIndx, value); } = 0;
		virtual void setDate(UInt& paramIndx, const TDate& date) { statement->setDate(paramIndx, date); } = 0;
		virtual void setDateAsString(UInt& paramIndx, const string& date, const string& date_frmt = "DD.MM.YYYY") = 0;
		//������� �������� �������� �����/����������:
		virtual bool isNull(UInt& paramIndx) const { return statement->isNull(paramIndx); } = 0;
		//��������� ����� ��������� ����� �� ���� ��������� � ��
		virtual void setPrefetchRowCount(UInt row_cnt) { pre_fetch_cnt = row_cnt; setPrefetch(); } = 0;
		//��������� ����� ��������� �����:
		UInt getPrefetchRowCount() const { return pre_fetch_cnt; };
		//���������/���������� ������������� ���������� ��� ������ �������� ���������/�������
		virtual void setExceptionOnNull(UInt& paramIndx, bool flg = false) { statement->setErrorOnNull(paramIndx, flg); } = 0;
	};

	//�����-��������� �������:
	class TStatement: public TBaseSet 
	{
	private:
		Connection* connect;//��������� ����������
		//��������� �����������
		TStatement(Connection* c, Statement* s);
		TStatement& operator=(const TStatement& st);
		//������� ������� �� ����������:
		bool createStatement(Connection* connect, const string& sql = "", bool auto_commit = false);
		//��������� ���������������� sql-�������:
		TSQLState execute(const string& sql = "") noexcept(true);
	protected:
		//��������� ������ �� ������������ �������:
		ResultSet* getResultSet() noexcept(true);
		//��������� ������ ��� DQL:
		ResultSet* executeQuery(const string& sql = "") noexcept(false);
		//��������� ������ ��� DML
		UInt executeDML(const string& sql = "") noexcept(false);
		TStatement(Connection* c, const string& sql = "", bool auto_commit = false);
	public:
		//������������� ������� � ������� �� ����������:
		explicit TStatement(const TDBConnect& dbc, const string& sql = "");
		~TStatement() { if (isValid()) connect->terminateStatement(statement); };
		//���������� �������
		bool isValid() const { return (connect && TBaseSet::isValid()); };
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
		//������� ��� ������ � �����������:
		//��������� �������� ��������� �� �������:
		int getInt(UInt& paramIndx) const { return TBaseSet::getInt(paramIndx); };
		double getDouble(UInt& paramIndx) const { return TBaseSet::getDouble(paramIndx); };
		float getFloat(UInt& paramIndx) const { return TBaseSet::getFloat(paramIndx); };
		string getString(UInt& paramIndex) const { return TBaseSet::getString(paramIndx); };
		TDate getDate(UInt& paramIndx) const { return TBaseSet::getDate(paramIndx); };
		string getDateAsStr(UInt& paramIndx, const string& date_frmt = "DD.MM.YYYY") const;


	};

	//����� �������������� �����:
	class TResultSet
	{
	private:
		const int prefetch_row = 200;//��������� - ������������� � ������  ������ �� ���� ��������� � ��
		Statement* quary;//��������� �� sql-������
		ResultSet* result;//��������� �� ���������� ����� ������
		int prefetch_cnt;//����� ����������� ����� ��� ��������� � ��
		//������ ����������
		TResultSet& operator=(const TResultSet& rs);
		//������ �����������:
		TResultSet(const TResultSet& rs);
		//������������� ���������� �� ResultSet
		TResultSet(ResultSet* r = nullptr) : result(r) {};
	public:
		//����������� ������� sql-�������
		explicit TResultSet(TStatement& st_query);
		//��������������� ������ ������:
		~TResultSet() {};
		//������� ��������� ������ ������ �� �������:




	};

}


#endif