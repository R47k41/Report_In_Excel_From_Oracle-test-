//модуль описания функция классов для соединения с базой данных Oracle
#include <iostream>
#include <exception>
#include <stdexcept>
#include "TOracle.h"
#include "Logger.hpp"

using oracle::occi::Connection;
using oracle::occi::Statement;
using std::string;

Connection* NS_Oracle::TBaseEnv::crtConnection(const string& uname, const string& upass, const string& tns)
{
	if (env)
		return env->createConnection(uname, upass, tns);
	else
		return nullptr;
}

bool NS_Oracle::TBaseEnv::closeConnection(ConnectionPtr c)
{
	using oracle::occi::SQLException;
	using std::cerr;
	using std::endl;
	try
	{
		if (env && c) env->terminateConnection(c);
		return true;
	}
	catch (const SQLException& er)
	{
		cerr << "Ошибка закрытия соединения: " << er.what() << endl;
		return false;
	}
	catch (...)
	{
		cerr << "Неизвестная ошибка закрытия соединения!" << endl;
		return false;
	}
	return false;
}

bool NS_Oracle::TDBConnect::Connect2DB(const string& uname, const string& upass,
		const string& utns) noexcept(true)
{
	using oracle::occi::SQLException;
	using std::cerr;
	using std::endl;
	try
	{
		connect = TBaseEnv::crtConnection(uname, upass, utns);
		if (connect) return true;
	}
	catch (const SQLException& er)
	{
		cerr << "Ошибка соединения с базой данных: " << er.what() << endl;
		return false;
	}
	catch (...)
	{
		cerr << "Неизвестная ошибка соединения с базой данных\nlogin: "<< uname <<
			"\npassword: " << upass << "\ntns: " << utns << endl;
		return false;
	}
	return false;
}

NS_Oracle::TDBConnect::TDBConnect(const string& uname, const string& upas, const string& utns, bool commit_on_exit)
{
	Connect2DB(uname, upas, utns);
	commit_on_close = commit_on_exit;
}

NS_Oracle::TDBConnect::TDBConnect(const TConnectParam& param)
{
	Connect2DB(param.username, param.password, param.tns_name);
	commit_on_close = param.auto_commit;
}

bool NS_Oracle::TDBConnect::closeConnection()
{
	if (isValid())
	{
		if (commit_on_close)
			Commit(connect);
		else
			RollBack(connect);
	}
	if (TBaseEnv::closeConnection(connect))	connect = nullptr;
	return !connect;
}

bool NS_Oracle::TDBConnect::Commit(ConnectionPtr c) noexcept(true)
{
	using oracle::occi::SQLException;
	using std::cerr;
	using std::endl;
	try
	{
		if (c)
		{
			c->commit();
			return true;
		}
	}
	catch (const SQLException& er)
	{
		cerr << "Ошибка при фиксации транзакции: " << er.what() << endl;
	}
	catch (...)
	{
		cerr << "Неизвестная ошибка фиксации транзакции!" << endl;
	}
	return false;
}

bool NS_Oracle::TDBConnect::RollBack(ConnectionPtr c) noexcept(true)
{
	using oracle::occi::SQLException;
	using std::cerr;
	using std::endl;
	try
	{
		if (c)
		{
			c->rollback();
			return true;
		}
	}
	catch (const SQLException& er)
	{
		cerr << "Ошибка при откате транзакции: " << er.what() << endl;
	}
	catch (...)
	{
		cerr << "Неизвестная ошибка отката транзакции!" << endl;
	}
	return false;
}

void NS_Oracle::TStatement::setPrefetchVal()
{
	if (isValid())
		statement->setPrefetchRowCount(prefetch_cnt);
	else
		prefetch_cnt = 0;
}

bool NS_Oracle::TStatement::crtStatement(ConnectionPtr c, const string& sql, 
	bool auto_commit, UInt maxIteration)
{
	using oracle::occi::SQLException;
	using std::cerr;
	using std::endl;
	try
	{
		if (c)
		{
			connect = c;
			statement = connect->createStatement();
			setMaxIterationCnt(maxIteration);
			setSQL(sql);
			statement->setAutoCommit(auto_commit);
		}
		else
			connect = nullptr;
		setPrefetchVal();
	}
	catch (const SQLException& er)
	{
		cerr << er.what() << endl;
		return false;
	}
	catch (...)
	{
		cerr << "Неизвестная ошибка создания sql-команды: " << sql << endl;
		return false;
	}
	return false;
}

NS_Oracle::TStatement::TStatement(EnvironmentPtr env, ConnectionPtr c, const string& sql, 
	bool auto_commit, UInt prefetch, UInt maxIteration): environment(env), prefetch_cnt(prefetch)
{
	crtStatement(c, sql, auto_commit, maxIteration);
}

NS_Oracle::TStatement::TStatement(const TDBConnect& dbc, const string& sql, UInt prefetch, UInt maxIteration): 
	environment(dbc.getenviroument()), prefetch_cnt(prefetch)
{
	crtStatement(dbc.connect, sql, dbc.commit_on_close, maxIteration);
}

NS_Oracle::UInt NS_Oracle::TStatement::getCurIteration() const
{
	using oracle::occi::SQLException;
	using std::cerr;
	using std::endl;
	try
	{
		if (isValid()) return statement->getCurrentIteration();
	}
	catch (const SQLException& er)
	{
		cerr << "Ошибка получения текущей итерации: " << er.what() << endl;
	}
	catch (...)
	{
		cerr << "Неизвестная ошибка получения текущей итерации!" << endl;
	}
	return 0;
}

bool NS_Oracle::TStatement::close() noexcept(false)
{
	if (isValid())
	{
		connect->terminateStatement(statement);
		statement = nullptr;
	}
	return !statement;
}

NS_Oracle::TStatement::~TStatement()
{
	using oracle::occi::SQLException;
	using std::cerr;
	using std::endl;
	try
	{
		close();
	}
	catch (const SQLException& err)
	{
		cerr << "Ошибка закрытия sql-команды: " << endl;
		cerr << getSQL() << endl;
		cerr << err.what() << endl;
	}
	catch (...)
	{
		cerr << "Неизвестная ошибка при закрытии sql-команды:" << endl;
		cerr << getSQL() << endl;
	}
}

string NS_Oracle::TStatement::getDateAsStrVal(UInt paramIndx, const string& date_frmt) const noexcept(true)
{
	using oracle::occi::SQLException;
	using std::cerr;
	using std::endl;
	try
	{
		TDate date = getDateVal(paramIndx);
		if (!date.isNull())
			return date.toText(date_frmt);
	}
	catch (const SQLException& er)
	{
		cerr << "Ошибка получения значения параметра :" << paramIndx << " : " << er.what() << endl;
	}
	catch (...)
	{
		cerr << "Неопределенная ошибка получения значения параметра :" << paramIndx << endl;
	}
	return string();
}

void NS_Oracle::TStatement::setDateAsStringVal(UInt paramIndx, const string& date, const string& date_frmt)
{
	TDate value;
	value.fromText(date, date_frmt, "", environment);
	setDateVal(paramIndx, value);
}

void NS_Oracle::TStatement::setSqlStringVal(UInt paramIndx, const string& str) noexcept(false)
{
	using std::stringstream;
	if (isValid())
	{
		string tmp_sql = getSQL();
		if (tmp_sql.empty())
			throw TLog("Пустой запрос!", "TStatement::setSqlStringVal");
		stringstream ss;
		ss << ':' << paramIndx;
		size_t pos = tmp_sql.find(ss.str(), 0);
		while (pos != string::npos)
		{
			tmp_sql.replace(pos, ss.str().size(), str);
			pos = tmp_sql.find(ss.str(), pos + str.size());
		}
		if (tmp_sql.empty()) return;
		setSQL(tmp_sql);
	}
}

NS_Oracle::TDate NS_Oracle::TStatement::initOCCIDate(int yy, UInt mm, UInt dd, UInt hh, UInt mi, UInt sec) const noexcept(true)
{
	try
	{
		TDate dateVal(environment, yy, mm, dd, hh, mi, sec);
		return dateVal;
	}
	catch (const SQLException& err)
	{
		TLog log("Ошибка преобразования даты: ", "TStatement::initOCCIDate");
		log << dd << '.' << mm << '.' << yy << ' ' << hh << ':' << mi << ':' << sec << '\n' << err.getMessage() << '\n';
		log.toErrBuff();
	}
	catch (...)
	{
		TLog log("Не обработанная ошибка преобразрвания даты: ", "TStatement::initOCCIDate");
		log << dd << '.' << mm << '.' << yy << ' ' << hh << ':' << mi << ':' << sec << '\n';
		log.toErrBuff();
	}
	return TDate();
}

void NS_Oracle::TStatement::registerOutParam(UInt paramIndx, const TType& data_type,
	UInt max_size, const string& sql_type) noexcept(false)
{
	if (isValid())
	{
		//для OCCIBYTES и OCCISTRING max_size должен быть больше 0
		if (max_size == 0 and (data_type == TType::OCCIBYTES 
			or data_type == TType::OCCISTRING)) max_size = max_bytes_size;
		statement->registerOutParam(paramIndx, data_type, max_size, sql_type);
	}
}

int NS_Oracle::TStatement::getParamsCnt(const string& ch) const
{
	int cnt = 0, s = ch.size();
	if (s == 0) return cnt;
	string sql = getSQL();
	if (sql.empty()) return cnt;
	size_t pos = 0;
	pos = sql.find(ch, pos);
	while (pos != string::npos)
	{
		pos = sql.find(ch, pos + s);
		cnt++;
	}
	return cnt;
}

void NS_Oracle::TStatement::setPrefetchRowCnt(UInt row_cnt)
{
	prefetch_cnt = row_cnt;
	setPrefetchVal();
}

NS_Oracle::TSQLState NS_Oracle::TStatement::Execute(const string& sql) noexcept(true)
{
	using oracle::occi::SQLException;
	using std::cerr;
	using std::endl;
	try
	{
		if (getState() == TSQLState::UNPREPARED)
			return statement->execute(sql);
		else
			return statement->execute();
	}
	catch (const SQLException& er)
	{
		cerr << "Ошибка выполнения запроса: ";
		if (!sql.empty())
			cerr << sql;
		else
			cerr << getSQL();
		cerr << endl << er.what() << endl;
	}
	catch (...)
	{
		cerr << "Неопределенная ошибка при выполнении запроса: ";
		if (!sql.empty())
			cerr << sql;
		else
			cerr << getSQL();
		cerr << endl;
	}
	return TSQLState::UNPREPARED;
}

NS_Oracle::UInt NS_Oracle::TStatement::getProcessedCntRows() const
{
	if (getState() == TSQLState::UPDATE_COUNT_AVAILABLE)
		return statement->getUpdateCount();
	else
		return 0;
}

NS_Oracle::ResultSet* NS_Oracle::TStatement::getResultSetVal() noexcept(true)
{
	using oracle::occi::SQLException;
	using std::cerr;
	using std::endl;
	try
	{
		if (getState() == TSQLState::RESULT_SET_AVAILABLE)
			return statement->getResultSet();
	}
	catch (const SQLException& er)
	{
		cerr << "Ошибка получения данных для запроса: ";
		cerr << getSQL() << endl << er.what() << endl;
	}
	catch (...)
	{
		cerr << "Неопределенная ошибка получения данных для запроса: ";
		cerr << getSQL() << endl;
	}
	return nullptr;
}

void NS_Oracle::TStatement::closeResultSet(NS_Oracle::ResultSetPtr dataSet) noexcept(false)
{
	if (dataSet)
	{
		statement->closeResultSet(dataSet);
	}
}


NS_Oracle::ResultSetPtr NS_Oracle::TStatement::executeQuery(const string& sql) noexcept(true)
{
	Execute(sql);
	return getResultSetVal();
}

NS_Oracle::UInt NS_Oracle::TStatement::executeDML(const string& sql) noexcept(true)
{
	Execute(sql);
	return getProcessedCntRows();
}

bool NS_Oracle::TStatement::ExecuteSQL(const string& sql) noexcept(true)
{
	if (Execute(sql) != TSQLState::UNPREPARED)
		return true;
	return false;
}

void NS_Oracle::TResultSet::setSize4NullCol()
{
	if (meta.size() < 1) return;
	for (UInt i = 0; i < meta.size(); i++)
	{
		if (meta[i].getInt(MetaData::ATTR_DATA_SIZE) == 0)
			//колонки нумеруются от 1
			setMaxColumnSize(i+1);
	}
}

void NS_Oracle::TResultSet::InitMetaData()
{
	using oracle::occi::SQLException;
	using std::cerr;
	using std::endl;
	try
	{
		if (result)
		{
			meta = result->getColumnListMetaData();
		}
	}
	catch (const SQLException& er)
	{
		cerr << "Ошибка при получении данных о колонках: " << er.what() << endl;
	}
	catch (...)
	{
		cerr << "Неизвестная ошибка получения данных о колонках!" << endl;
	}
}

NS_Oracle::TResultSet::TResultSet(TStatement& query)
{
	switch (query.getState())
	{
	case TSQLState::PREPARED:
		result = query.executeQuery();
		break;
	case TSQLState::RESULT_SET_AVAILABLE:
		result = query.getResultSetVal();
		break;
	default:
		result = nullptr;
	}
	InitMetaData();
	setSize4NullCol();
}

NS_Oracle::TResultSet::TResultSet(ResultSetPtr dataSet)
{
	//если пустой датасет или он уже есть - выход
	if (dataSet == nullptr or dataSet == result) return;
	result = dataSet;
	InitMetaData();
	setSize4NullCol();
}

bool NS_Oracle::TResultSet::close() noexcept(false)
{
	if (isValid())
	{
		StatementPtr st = result->getStatement();
		if (st) st->closeResultSet(result);
		result = nullptr;
	}
	return !result;
}

NS_Oracle::TResultSet::~TResultSet()
{
	using oracle::occi::SQLException;
	using std::cerr;
	using std::endl;
	try
	{
		close();
	}
	catch (const SQLException& err)
	{
		cerr << "Ошибка при закрытии ResultSet!" << endl;
		cerr << err.what() << endl;
	}
	catch (...)
	{
		cerr << "Неизвестная ошибка закрытия ResultSet!" << endl;
	}
}

string NS_Oracle::TResultSet::getDateAsStrVal(UInt paramIndx, const string& date_frmt) const noexcept(true)
{
	using std::cerr;
	using std::endl;
	try
	{
		TDate date = getDateVal(paramIndx);
		if (!date.isNull())
			return date.toText(date_frmt);
	}
	catch (const SQLException& er)
	{
		cerr << "Ошибка получения значения параметра :" << paramIndx << " : " << er.what() << endl;
	}
	catch (...)
	{
		cerr << "Неопределенная ошибка получения значения параметра :" << paramIndx << endl;
	}
	return string();
}

NS_Oracle::TType NS_Oracle::TResultSet::getColumnType(UInt colIndx) const noexcept(false)
{
	using std::cerr;
	using std::endl;
	using NS_Logger::TLog;
	if (colIndx - 1 > getColumnsCnt() or colIndx < 1)
	{
		// std::out_of_range
		TLog log("Индекс колонки превышает реальное число колонок или нулевой!!!", "::TResultSet::getColumnType");
		log << TLog::NL << "Индекс колонки: " << colIndx << TLog::NL;
		throw log;
	}
	return TType(meta[colIndx-1].getInt(MetaData::ATTR_DATA_TYPE));
}
