#ifndef TORACLE_H_
#define TORACLE_H_
//Заголовочный файл описывающий интерфейс для работы с БД Oracle
// с использованием OCCI библиотеки
//FAQ по OCCI:
// https://docs.oracle.com/en/database/oracle/oracle-database/19/lncpp/accessing-oracle-database-using-cplusplus.html#GUID-F273EF54-652C-4905-A60D-2368ADE31C81
//если забыл про ddl/dml: https://www.geeksforgeeks.org/sql-ddl-dql-dml-dcl-tcl-commands/
//описание типов данных:
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
	
	//структура для параметров соединения
	struct TConnectParam
	{
		string username;//имя пользователя
		string password;//паролья для входа
		string tns_name;//строка соединения
		bool auto_commit;//автофиксация транзакции
	};

	//Базыовый класс для среды обработки:
	class TBaseEnv
	{
	private:
		Environment* env;
		TBaseEnv(const TBaseEnv& e);
		TBaseEnv& operator=(const TBaseEnv& e);
	protected:
		//создать соединение с базой
		Connection* createConnection(const string& uname, const string& upass, const string& tns) noexcept(false);
		virtual bool Connect2DB(const string& uname, const string& upass, const string& tns) = 0;
		//закрыть соединение с базой
		virtual bool closeConnection(Connection* c = nullptr);
	public:
		TBaseEnv() { env = Environment::createEnvironment(); };
		virtual ~TBaseEnv() { Environment::terminateEnvironment(env); };
		//создать соединение с базой
		virtual bool createConnection(const TConnectParam& param) { return Connect2DB(param.username, param.password, param.tns_name); };
		virtual bool isValid(void) const { return env; };
	};
	
	//класс соединение с базой данных:
	class TDBConnect : public TBaseEnv
	{
	private:
		bool commit_on_close;
		Connection* connect;
		TDBConnect(const TDBConnect& c);
		TDBConnect(Connection* c);
		TDBConnect& operator=(const TDBConnect& c);
		//фиксакция транзакции:
		static bool Commit(Connection* c) noexcept(true);
		//откат транзакции:
		static bool RollBack(Connection* c) noexcept(true);
		friend class TStatement;
	public:
		TDBConnect(const string& uname, const string& upas, const string& utns, bool commit_on_exit = false);
		explicit TDBConnect(const TConnectParam& param);
		virtual ~TDBConnect() { closeConnection(); };
		//соединение с БД
		bool Connect2DB(const string& uname, const string& upass, const string& tns, bool commit_on_exit = false) noexcept(true);
		//переопределение закрытия соединения:
		bool closeConnection();
		//проверка созданоли соединение с БД
		bool isValid() const { return (TBaseEnv::isValid() && connect); };
	};

	//базовый класс для sql-команды
	class TBaseSet
	{
	protected:
		Statement* statement;//указатель на sql-команду
		UInt pre_fetch_cnt;//число предварительно выбираемых строк
	private:
		static const int pre_fetch_rows = 200;//число строк выбираемое при обращении к БД
		//исключаем копирование
		TBaseSet(const TBaseSet& b);
		//исключаем присвоение:
		TBaseSet& operator=(const TBaseSet& b);
		//установка выбираемых строк за одно обращение(базовое)
		virtual void setPrefetch() { if (isValid()) statement->setPrefetchRowCount(pre_fetch_cnt); };
	public:
		//инициализация
		explicit TBaseSet(Statement* st, UInt prefetch = pre_fetch_rows) : statement(st), pre_fetch_cnt(prefetch) { setPrefetch(); };
		//деструктор:
		virtual ~TBaseSet() { statement = nullptr; };
		//валидность данных:
		virtual bool isValid() const { return (statement ? true : false); };
		//получение состояния набора:
		virtual TSQLState getState() const { return (isValid() ? statement->status() : TSQLState::UNPREPARED); };
		//набор функция получения значения параметра:
		virtual int getInt(UInt& paramIndx) const { return statement->getInt(paramIndx); } = 0;
		virtual double getDouble(UInt& paramIndx) const { return statement->getDouble(paramIndx); } = 0;
		virtual float getFloat(UInt& paramIndx) const { return statement->getFloat(paramIndx); } = 0;
		virtual string getString(UInt& paramIndex) const { return statement->getString(paramIndex); } = 0;
		virtual TDate getDate(UInt& paramIndx) const { return statement->getDate(paramIndx); } = 0;
		virtual string getDateAsStr(UInt& paramIndx, const string& date_frmt = "DD.MM.YYYY") const = 0;
		//набор функций установка значения парамента:
		virtual void setInt(UInt& paramIndx, int value) { statement->setInt(paramIndx, value); } = 0;
		virtual void setDouble(UInt& paramIndx, double value) { statement->setDouble(paramIndx, value); } = 0;
		virtual void setFloat(UInt& paramIndx, float value) { statement->setFloat(paramIndx, value); } = 0;
		virtual void setString(UInt& paramIndx, const string& value) { statement->setString(paramIndx, value); } = 0;
		virtual void setDate(UInt& paramIndx, const TDate& date) { statement->setDate(paramIndx, date); } = 0;
		virtual void setDateAsString(UInt& paramIndx, const string& date, const string& date_frmt = "DD.MM.YYYY") = 0;
		//функции проверки значений полей/параметров:
		virtual bool isNull(UInt& paramIndx) const { return statement->isNull(paramIndx); } = 0;
		//установка числа выбранных строк за одно обращение к БД
		virtual void setPrefetchRowCount(UInt row_cnt) { pre_fetch_cnt = row_cnt; setPrefetch(); } = 0;
		//получение числа выбранных строк:
		UInt getPrefetchRowCount() const { return pre_fetch_cnt; };
		//включение/отключение возникновения исключения при пустом значении параметра/колонки
		virtual void setExceptionOnNull(UInt& paramIndx, bool flg = false) { statement->setErrorOnNull(paramIndx, flg); } = 0;
	};

	//Класс-интерфейс Команда:
	class TStatement: public TBaseSet 
	{
	private:
		Connection* connect;//указатель соединения
		//запрещаем копирование
		TStatement(Connection* c, Statement* s);
		TStatement& operator=(const TStatement& st);
		//создаем команду на выполнение:
		bool createStatement(Connection* connect, const string& sql = "", bool auto_commit = false);
		//выполнить нетипизированную sql-команду:
		TSQLState execute(const string& sql = "") noexcept(true);
	protected:
		//получение данных из выполненного запроса:
		ResultSet* getResultSet() noexcept(true);
		//выполнить запрос как DQL:
		ResultSet* executeQuery(const string& sql = "") noexcept(false);
		//выполнить запрос как DML
		UInt executeDML(const string& sql = "") noexcept(false);
		TStatement(Connection* c, const string& sql = "", bool auto_commit = false);
	public:
		//инициализация строкой и ссылкой на соединение:
		explicit TStatement(const TDBConnect& dbc, const string& sql = "");
		~TStatement() { if (isValid()) connect->terminateStatement(statement); };
		//валидность объекта
		bool isValid() const { return (connect && TBaseSet::isValid()); };
		//фиксация транзакции
		bool Commit() { return TDBConnect::Commit(connect); };
		//откат транзакции
		bool RollBack() { return TDBConnect::RollBack(connect); };
		//установить текст sql-запроса:
		void setSQL(const string& sql) { if (isValid()) statement->setSQL(sql); };
		//получить текст текущего запроса:
		string getSQL(void) const { return (isValid() ? statement->getSQL() : string()); };
		//получение номера текущей итерации:
		UInt getCurrentIteration() const;
		//получение обработанных строк dml-командой
		UInt getProcessedCntRows() const;
		//выполнить запрос не зависимо от типа sql-команды:
		bool executeSQL(const string& sql = "") noexcept(true);
		//функции для работы с параметрами:
		//получение значения параметра по индексу:
		int getInt(UInt& paramIndx) const { return TBaseSet::getInt(paramIndx); };
		double getDouble(UInt& paramIndx) const { return TBaseSet::getDouble(paramIndx); };
		float getFloat(UInt& paramIndx) const { return TBaseSet::getFloat(paramIndx); };
		string getString(UInt& paramIndex) const { return TBaseSet::getString(paramIndx); };
		TDate getDate(UInt& paramIndx) const { return TBaseSet::getDate(paramIndx); };
		string getDateAsStr(UInt& paramIndx, const string& date_frmt = "DD.MM.YYYY") const;


	};

	//класс Результирующий набор:
	class TResultSet
	{
	private:
		const int prefetch_row = 200;//константа - считывающиеся в память  строки за одно обращение к БД
		Statement* quary;//указатель на sql-запрос
		ResultSet* result;//указатель на полученный набор данных
		int prefetch_cnt;//число считываемых строк при обращении к БД
		//запрет присвоения
		TResultSet& operator=(const TResultSet& rs);
		//запрет копирования:
		TResultSet(const TResultSet& rs);
		//инициализация указателем на ResultSet
		TResultSet(ResultSet* r = nullptr) : result(r) {};
	public:
		//иниализация объктом sql-команды
		explicit TResultSet(TStatement& st_query);
		//деинициализация набора данных:
		~TResultSet() {};
		//функция получения набора данных из запроса:




	};

}


#endif