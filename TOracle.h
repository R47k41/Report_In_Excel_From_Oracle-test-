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
	using TType = oracle::occi::Type;//тип дпнных для библиотеки OCCI
	using UInt = unsigned int;
	using TMetaDataArr = OCCI_STD_NAMESPACE::vector<MetaData>;
	
	//структура для параметров соединения
	struct TConnectParam
	{
		string username;//имя пользователя
		string password;//паролья для входа
		string tns_name;//строка соединения
		bool auto_commit;//автофиксация транзакции
		unsigned int prefetch_rows;//число строк получаемое за одно обращение к БД
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
		bool Connect2DB(const string& uname, const string& upass, const string& tns) noexcept(true);
		//переопределение закрытия соединения:
		bool closeConnection();
		//проверка созданоли соединение с БД
		bool isValid() const { return (TBaseEnv::isValid() && connect); };
	};

	//базовый класс для набора данных - определяет базовые функции
	class TBaseSet
	{
	private:
		//исключаем присвоение:
		TBaseSet& operator=(const TBaseSet& b);
	public:
		//инициализация
		TBaseSet() {};
		//деструктор:
		virtual ~TBaseSet() {};
		//валидность данных:
		virtual bool isValid() const = 0;
		//набор функция получения значения параметра:
		virtual int getInt(UInt& paramIndx) const = 0;
		virtual double getDouble(UInt& paramIndx) const = 0;
		virtual float getFloat(UInt& paramIndx) const = 0;
		virtual string getString(UInt& paramIndx) const = 0;
		virtual TDate getDate(UInt& paramIndx) const = 0;
		virtual string getDateAsStr(UInt& paramIndx, const string& date_frmt = "DD.MM.YYYY") const = 0;
		//функции проверки значений полей/параметров:
		virtual bool isNull(UInt& paramIndx) const = 0;
		//включение/отключение возникновения исключения при пустом значении параметра/колонки
		virtual void setExceptionOnNull(UInt& paramIndx, bool flg = false) = 0;
	};

	//Класс-интерфейс Команда:
	class TStatement: public TBaseSet 
	{
	private:
		static const int prefetch_rows = 1;//число строк выбираемое при обращении к БД
		Connection* connect;//указатель соединения
		Statement* statement;//указатель на sql-команду
		UInt prefetch_cnt;//число предварительно выбираемых строк
		//запрещаем копирование
		TStatement(Connection* c, Statement* s);
		TStatement& operator=(const TStatement& st);
		//создаем команду на выполнение:
		bool createStatement(Connection* c, const string& sql = "", bool auto_commit = false);
		//выполнить нетипизированную sql-команду:
		TSQLState execute(const string& sql = "") noexcept(true);
		//установка числа выбираемых записей за одно обращение:
		void setPrefetch();
		TStatement(Connection* c, const string& sql = "", bool auto_commit = false,
			UInt prefetch = prefetch_rows);
	public:
		//инициализация строкой и ссылкой на соединение:
		explicit TStatement(const TDBConnect& dbc, const string& sql = "", UInt prefetch = prefetch_rows);
		~TStatement() { if (isValid()) connect->terminateStatement(statement); };
		/*Опреденление функций из базового класса*/
		//валидность объекта
		bool isValid() const { return (connect && statement); };
		//получение состояния sql-команды:
		TSQLState getState() const { return (isValid() ? statement->status() : TSQLState::UNPREPARED); };
		//функции для работы с параметрами:
		//получение значения параметра по индексу:
		int getInt(UInt& paramIndx) const { return statement->getInt(paramIndx); };
		double getDouble(UInt& paramIndx) const { return statement->getDouble(paramIndx); };
		float getFloat(UInt& paramIndx) const { return statement->getFloat(paramIndx); };
		string getString(UInt& paramIndx) const { return statement->getString(paramIndx); };
		TDate getDate(UInt& paramIndx) const { return statement->getDate(paramIndx); };
		string getDateAsStr(UInt& paramIndx, const string& date_frmt = "DD.MM.YYYY") const;
		//набор функций установка значения парамента:
		void setInt(UInt& paramIndx, int value) { statement->setInt(paramIndx, value); };
		void setDouble(UInt& paramIndx, double value) { statement->setDouble(paramIndx, value); };
		void setFloat(UInt& paramIndx, float value) { statement->setFloat(paramIndx, value); };
		void setString(UInt& paramIndx, const string& value) { statement->setString(paramIndx, value); };
		void setDate(UInt& paramIndx, const TDate& date) { statement->setDate(paramIndx, date); };
		void setDateAsString(UInt& paramIndx, const string& date, const string& date_frmt = "DD.MM.YYYY");
		//функции проверки значений полей/параметров:
		virtual bool isNull(UInt& paramIndx) const { return statement->isNull(paramIndx); };
		//включение/отключение возникновения исключения при пустом значении параметра/колонки
		void setExceptionOnNull(UInt& paramIndx, bool flg = false) { statement->setErrorOnNull(paramIndx, flg); };
		//Определение собственных функций класса
		//получение числа выбранных строк:
		UInt getPrefetchRowCount() const { return prefetch_cnt; };
		//установка числа выбранных строк за одно обращение к БД
		void setPrefetchRowCount(UInt row_cnt);
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
		//получение данных из выполненного запроса:
		ResultSet* getResultSet() noexcept(true);
		//выполнить запрос как DQL:
		ResultSet* executeQuery(const string& sql = "") noexcept(true);
		//выполнить запрос как DML
		UInt executeDML(const string& sql = "") noexcept(true);
	};

	//класс Результирующий набор:
	class TResultSet: public TBaseSet
	{
	private:
		ResultSet* result;//указатель на полученный набор данных
		TMetaDataArr meta;//метаданные для колонок
		//запрет присвоения
		TResultSet& operator=(const TResultSet& rs);
		//запрет копирования:
		TResultSet(const TResultSet& rs);
		//инициализация метаданных колонок:
		void InitMetaData();
	protected:
		//очистка ResultSet:
		void cancel() { result->cancel(); };
	public:
		//иниализация объктом sql-команды
		explicit TResultSet(TStatement& query);
		//деинициализация набора данных:
		~TResultSet();
		//Определение общего интерфейса:
		//проверка валидности:
		bool isValid() const { return result; };
		//набор функция получения значения параметра:
		int getInt(UInt& paramIndx) const { return result->getInt(paramIndx); };
		double getDouble(UInt& paramIndx) const { return result->getDouble(paramIndx); };
		float getFloat(UInt& paramIndx) const { return result->getFloat(paramIndx); };
		string getString(UInt& paramIndx) const { return result->getString(paramIndx); };
		TDate getDate(UInt& paramIndx) const { return result->getDate(paramIndx); };
		string getDateAsStr(UInt& paramIndx, const string& date_frmt = "DD.MM.YYYY") const;
		//функции проверки значений полей/параметров:
		bool isNull(UInt& paramIndx) const { return result->isNull(paramIndx); };
		//включение/отключение возникновения исключения при пустом значении параметра/колонки
		void setExceptionOnNull(UInt& paramIndx, bool flg = false) { result->setErrorOnNull(paramIndx, flg); };
		//собственный функционал:
		//получение статуса считываемых данных:
		TDataSetState getState() const { return result->status(); };
		//проверка на урезание данных строки:
		bool isTruncated(UInt& paramIndx) const { return result->isTruncated(paramIndx); };
		//получение статуса
		TDataSetState Next(UInt RowsCnt = 1) { return (isValid() ? result->status() : TDataSetState::END_OF_FETCH); };
		//получение длины параметра перед truncate
		int getPreTruncationLenght(UInt& paramIndx) const { return result->preTruncationLength(paramIndx); };
		//вызывать ошибку при отрезанной длине параметра:
		void setExceptionOnTruncate(UInt& paramIndx, bool flg = false) { result->setErrorOnTruncate(paramIndx, flg); };
		//функция получения числа колонок в запросе
		UInt getColumnsCount(void) const { return meta.size(); };
		//получение типа данных для колонки:
		TType getColumnType(UInt& colIndx) const noexcept(false);
		//дружественные функции по формированию отчетов:
	};

}


#endif