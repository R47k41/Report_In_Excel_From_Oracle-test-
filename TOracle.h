#ifndef TORACLE_H_
#define TORACLE_H_
//Заголовочный файл описывающий интерфейс для работы с БД Oracle
// с использованием OCCI библиотеки
//FAQ по OCCI:
// https://docs.oracle.com/en/database/oracle/oracle-database/19/lncpp/accessing-oracle-database-using-cplusplus.html#GUID-F273EF54-652C-4905-A60D-2368ADE31C81
//если забыл про ddl/dml: https://www.geeksforgeeks.org/sql-ddl-dql-dml-dcl-tcl-commands/
//описание типов данных:
//https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/Data-Types.html#GUID-7B72E154-677A-4342-A1EA-C74C1EA928E6
//про методанные:
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
		//проверка на пустоту
		bool isEmpty() const { return username.empty() or password.empty() or tns_name.empty(); }
	};

	//Базыовый класс для среды обработки:
	class TBaseEnv
	{
	private:
		TBaseEnv(const TBaseEnv& e);
		TBaseEnv& operator=(const TBaseEnv& e);
	protected:
		EnvironmentPtr env;
		//создать соединение с базой
		Connection* crtConnection(const string& uname, const string& upass, const string& tns) noexcept(false);
		virtual bool Connect2DB(const string& uname, const string& upass, const string& tns) = 0;
		//закрыть соединение с базой
		virtual bool closeConnection(ConnectionPtr c = nullptr);
	public:
		TBaseEnv() { env = Environment::createEnvironment(); };
		virtual ~TBaseEnv() { Environment::terminateEnvironment(env); };
		//создать соединение с базой
		virtual bool crtConnection(const TConnectParam& param) { return Connect2DB(param.username, param.password, param.tns_name); };
		virtual bool isValid(void) const { return env; };
		virtual const EnvironmentPtr getenviroument() const { return env; }
	};
	
	//класс соединение с базой данных:
	class TDBConnect : public TBaseEnv
	{
	private:
		bool commit_on_close;
		ConnectionPtr connect;
		TDBConnect(const TDBConnect& c);
		TDBConnect(ConnectionPtr c);
		TDBConnect& operator=(const TDBConnect& c);
		//фиксакция транзакции:
		static bool Commit(ConnectionPtr c) noexcept(true);
		//откат транзакции:
		static bool RollBack(ConnectionPtr c) noexcept(true);
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
		virtual int getIntVal(UInt paramIndx) const = 0;
		virtual double getDoubleVal(UInt paramIndx) const = 0;
		virtual float getFloatVal(UInt paramIndx) const = 0;
		virtual string getStringVal(UInt paramIndx) const = 0;
		virtual TDate getDateVal(UInt paramIndx) const = 0;
		virtual string getDateAsStrVal(UInt paramIndx, const string& date_frmt = "DD.MM.YYYY") const noexcept(true) = 0;
		//функции проверки значений полей/параметров:
		virtual bool isNullVal(UInt paramIndx) const = 0;
		//включение/отключение возникновения исключения при пустом значении параметра/колонки
		virtual void setExceptionOnNull(UInt paramIndx, bool flg = false) = 0;
	};

	//Класс-интерфейс Команда:
	class TStatement: public TBaseSet 
	{
	private:
		static const int prefetch_rows = 1;//число строк выбираемое при обращении к БД
		const EnvironmentPtr environment;//указатель на окружение
		ConnectionPtr connect;//указатель соединения
		StatementPtr statement;//указатель на sql-команду
		UInt prefetch_cnt;//число предварительно выбираемых строк
		//запрещаем копирование
		TStatement(ConnectionPtr c, StatementPtr s);
		TStatement& operator=(const TStatement& st);
		//создаем команду на выполнение:
		bool crtStatement(ConnectionPtr c, const string& sql = "", bool auto_commit = false);
		//выполнить нетипизированную sql-команду:
		TSQLState Execute(const string& sql = "") noexcept(true);
		//установка числа выбираемых записей за одно обращение:
		void setPrefetchVal();
		TStatement(EnvironmentPtr env, ConnectionPtr c, const string& sql = "", bool auto_commit = false,
			UInt prefetch = prefetch_rows);
	public:
		//инициализация строкой и ссылкой на соединение:
		explicit TStatement(const TDBConnect& dbc, const string& sql = "", UInt prefetch = prefetch_rows);
		//функция закрытия sql-команды:
		bool close() noexcept(false);
		~TStatement();
		/*Опреденление функций из базового класса*/
		//валидность объекта
		bool isValid() const { return (connect && statement); };
		//получение состояния sql-команды:
		TSQLState getState() const { return (isValid() ? statement->status() : TSQLState::UNPREPARED); };
		//функции для работы с параметрами:
		//получение значения параметра по индексу:
		int getIntVal(UInt paramIndx) const { return statement->getInt(paramIndx); };
		double getDoubleVal(UInt paramIndx) const { return statement->getDouble(paramIndx); };
		float getFloatVal(UInt paramIndx) const { return statement->getFloat(paramIndx); };
		string getStringVal(UInt paramIndx) const { return statement->getString(paramIndx); };
		TDate getDateVal(UInt paramIndx) const { return statement->getDate(paramIndx); };
		string getDateAsStrVal(UInt paramIndx, const string& date_frmt = "DD.MM.YYYY") const noexcept(true);
		//набор функций установка значения парамента:
		void setIntVal(UInt paramIndx, int value) { statement->setInt(paramIndx, value); };
		void setDoubleVal(UInt paramIndx, double value) { statement->setDouble(paramIndx, value); };
		void setFloatVal(UInt paramIndx, float value) { statement->setFloat(paramIndx, value); };
		void setStringVal(UInt paramIndx, const string& value) { statement->setString(paramIndx, value); };
		void setDateVal(UInt paramIndx, const TDate& date) { statement->setDate(paramIndx, date); };
		void setDateAsStringVal(UInt paramIndx, const string& date, const string& date_frmt = "DD.MM.YYYY");
		//функции проверки значений полей/параметров:
		virtual bool isNullVal(UInt paramIndx) const { return statement->isNull(paramIndx); };
		//включение/отключение возникновения исключения при пустом значении параметра/колонки
		void setExceptionOnNull(UInt paramIndx, bool flg = false) { statement->setErrorOnNull(paramIndx, flg); };
		//Определение собственных функций класса
		//получение числа параметров запроса:
		int getParamsCnt(const string& ch) const;
		//получение числа выбранных строк:
		UInt getPrefetchRowCnt() const { return prefetch_cnt; };
		//установка числа выбранных строк за одно обращение к БД
		void setPrefetchRowCnt(UInt row_cnt);
		//фиксация транзакции
		bool Commit() { return TDBConnect::Commit(connect); };
		//откат транзакции
		bool RollBack() { return TDBConnect::RollBack(connect); };
		//установить текст sql-запроса:
		void setSQL(const string& sql) { if (isValid()) statement->setSQL(sql); };
		//получить текст текущего запроса:
		string getSQL(void) const { return (isValid() ? statement->getSQL() : string()); };
		//получение номера текущей итерации:
		UInt getCurIteration() const;
		//получение обработанных строк dml-командой
		UInt getProcessedCntRows() const;
		//выполнить запрос не зависимо от типа sql-команды:
		bool ExecuteSQL(const string& sql = "") noexcept(true);
		//получение данных из выполненного запроса:
		ResultSetPtr getResultSetVal() noexcept(true);
		//выполнить запрос как DQL:
		ResultSetPtr executeQuery(const string& sql = "") noexcept(true);
		//выполнить запрос как DML
		UInt executeDML(const string& sql = "") noexcept(true);
	};

	//класс Результирующий набор:
	//https://docs.oracle.com/en/database/oracle/oracle-database/19/lncpp/resultset-class.html#GUID-9E0C0A68-57B7-4C77-8865-A306BF6953CA
	class TResultSet: public TBaseSet
	{
	private:
		ResultSetPtr result;//указатель на полученный набор данных
		TMetaDataArr meta;//метаданные для колонок
		//запрет присвоения
		TResultSet& operator=(const TResultSet& rs);
		//запрет копирования:
		TResultSet(const TResultSet& rs);
		//утсновка минимального размера данных при значениях NULL в ячейках
		void setSize4NullCol();
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
		//колонки при считывании начинаются с 1!!!
		int getIntVal(UInt paramIndx) const noexcept(false) { return result->getInt(paramIndx); };
		double getDoubleVal(UInt paramIndx) const noexcept(false) { return result->getDouble(paramIndx); };
		float getFloatVal(UInt paramIndx) const noexcept(false) { return result->getFloat(paramIndx); };
		string getStringVal(UInt paramIndx) const noexcept(false) { return result->getString(paramIndx); };
		TDate getDateVal(UInt paramIndx) const noexcept(false) { return result->getDate(paramIndx); };
		string getDateAsStrVal(UInt paramIndx, const string& date_frmt = "DD.MM.YYYY") const noexcept(true);
		//функции проверки значений полей/параметров:
		bool isNullVal(UInt paramIndx) const { return result->isNull(paramIndx); };
		//включение/отключение возникновения исключения при пустом значении параметра/колонки
		void setExceptionOnNull(UInt paramIndx, bool flg = false) { result->setErrorOnNull(paramIndx, flg); };
		//собственный функционал:
		//получение статуса считываемых данных:
		TDataSetState getState() const { return result->status(); };
		//проверка на урезание данных строки:
		bool isTruncatedVal(UInt paramIndx) const { return result->isTruncated(paramIndx); };
		//получение статуса
		TDataSetState Next(UInt RowsCnt = 1) { return (isValid() ? result->next() : TDataSetState::END_OF_FETCH); };
		//получение длины параметра перед truncate
		int getPreTruncLenght(UInt paramIndx) const { return result->preTruncationLength(paramIndx); };
		//вызывать ошибку при отрезанной длине параметра:
		void setExceptOnTruncate(UInt paramIndx, bool flg = false) { result->setErrorOnTruncate(paramIndx, flg); };
		//функция получения числа колонок в запросе
		size_t getColumnsCnt(void) const { return meta.size(); };
		//получение типа данных для колонки:
		TType getColumnType(UInt colIndx) const noexcept(false);
		//закрытие resultSet
		bool close(void) noexcept(false);
		//установка максимально допустимого числа байт для считывания данных из колонки:
		void setMaxColumnSize(UInt colIndx, UInt size = 1) noexcept(false) { result->setMaxColumnSize(colIndx, size); }
	};
}

#endif