//модуль описания функция классов для соединения с базой данных Oracle
#include <iostream>
#include "TOracle.h"

using oracle::occi::Connection;
using oracle::occi::Statement;
using std::string;

Connection* NS_Oracle::TBaseEnv::createConnection(const string& uname, const string& upass, const string& tns)
{
	if (env)
		return env->createConnection(uname, upass, tns);
	else
		return nullptr;
}

bool NS_Oracle::TBaseEnv::closeConnection(Connection* c)
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
		const string& utns, bool commit_on_exit) noexcept(true)
{
	using oracle::occi::SQLException;
	using std::cerr;
	using std::endl;
	try
	{
		connect = TBaseEnv::createConnection(uname, upass, utns);
		commit_on_close = commit_on_exit;
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
	Connect2DB(uname, upas, utns, commit_on_exit);
}

NS_Oracle::TDBConnect::TDBConnect(const TConnectParam& param)
{
	Connect2DB(param.username, param.password, param.tns_name, param.auto_commit);
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
	return TBaseEnv::closeConnection(connect);
}

bool NS_Oracle::TDBConnect::Commit(Connection* c) noexcept(true)
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

bool NS_Oracle::TDBConnect::RollBack(Connection* c) noexcept(true)
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

bool NS_Oracle::TStatement::createStatement(Connection* connect, const string& sql, bool auto_commit)
{
	using oracle::occi::SQLException;
	using std::cerr;
	using std::endl;
	try
	{
		if (connect)
		{
			con = connect;
			st = con->createStatement(sql);
			st->setAutoCommit(auto_commit);
		}
		else
		{
			con = nullptr;
			st = nullptr;
		}

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

NS_Oracle::TStatement::TStatement(Connection* c, const string& sql, bool auto_commit)
{
	createStatement(c, sql, auto_commit);
}

NS_Oracle::TStatement::TStatement(const TDBConnect& dbc, const string& sql)
{
	createStatement(dbc.connect, sql, dbc.commit_on_close);
}

NS_Oracle::UInt NS_Oracle::TStatement::getCurrentIteration() const
{
	using oracle::occi::SQLException;
	using std::cerr;
	using std::endl;
	try
	{
		if (isValid()) return st->getCurrentIteration();
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

string NS_Oracle::TStatement::getDateAsStr(UInt& paramIndx, const string& date_frmt) const
{
	using oracle::occi::SQLException;
	using std::cerr;
	using std::endl;
	try
	{
		TDate date = getDate(paramIndx);
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

void NS_Oracle::TStatement::setDateAsString(UInt& paramIndx, const string& date, const string& date_frmt)
{
	TDate value;
	value.fromText(date, date_frmt);
	setDate(paramIndx, value);
}

NS_Oracle::TSQLState NS_Oracle::TStatement::execute(const string& sql) noexcept(true)
{
	using oracle::occi::SQLException;
	using std::cerr;
	using std::endl;
	try
	{
		if (getState() == TSQLState::UNPREPARED)
			return st->execute(sql);
		else
			return st->execute();
	}
	catch (const SQLException& er)
	{
		cerr << "Ошибка выполнения запроса: ";
		if (!sql.empty())
			cerr << sql;
		else
			cerr << getSQL();
		cerr << endl << er.what << endl;
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
		return st->getUpdateCount();
	else
		return 0;
}

NS_Oracle::ResultSet* NS_Oracle::TStatement::getResultSet() noexcept(true)
{
	using oracle::occi::SQLException;
	using std::cerr;
	using std::endl;
	try
	{
		if (getState() == TSQLState::RESULT_SET_AVAILABLE)
			return st->getResultSet();
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

NS_Oracle::ResultSet* NS_Oracle::TStatement::executeQuery(const string& sql) noexcept(true)
{
	execute(sql);
	return getResultSet();
}

NS_Oracle::UInt NS_Oracle::TStatement::executeDML(const string& sql) noexcept(true)
{
	execute(sql);
	return getProcessedCntRows();
}

bool NS_Oracle::TStatement::executeSQL(const string& sql) noexcept(true)
{
	if (execute(sql) != TSQLState::UNPREPARED)
		return true;
	st->closeResultSet()
	return false;
}
