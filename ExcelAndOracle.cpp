// ExcelAndOracle.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//Описание работы с libxl: http://www.libxl.com/
//FAQ OCCI:
// https://docs.oracle.com/en/database/oracle/oracle-database/19/lncpp/introduction-to-occi.html#GUID-99077609-BFCC-4FF7-9D23-C45623DDD465
//типы данных:
//https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/Data-Types.html#GUID-7B72E154-677A-4342-A1EA-C74C1EA928E6
//Добавить в PATH: D:\NewPrg\C++\Libs\oracle\occi-oracle\sdk\dll;
//#define _ITERATOR_DEBUG_LEVEL 0
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <string.h>
#include <locale.h>
#include "libxl.h"
#include "occi.h"
#include "TConverter.h"
#include "TConverter.cpp"
#include "Logger.h"
#include "Logger.hpp"
#include "TConstants.h"
#include "TOracle.h"
#include "TSQLParser.h"
#include "TuneParam.h"
//#include "DoReport.cpp"
//#include "LibXL_Example.h"
#include <boost/date_time/gregorian/gregorian.hpp>

void excel_example(void);
void oracle_example(void);

//функция проверки работы конвертера:
void Test_toStr();

//функция провекри логирования:
void Test_Logger();

//функция проверки парсинга для sql-запроса:
void TSqlParse(const std::string& str);

void parse_tune_file(const std::string& filename);

//функция проверки работы с БД для TOracle:
void TOracleTest();

//определение внешней функции
extern bool CreateSimpleReport(const std::string& file_name) noexcept(true);

//inline void SetRuConsole(int cp) { SetConsoleCP(cp); SetConsoleOutputCP(cp); };

void test_Const_Module(void);

int main()
{
	using std::cerr;
	using std::cout;
	using std::endl;
	using std::string;
	//SetRuConsole(1251);
	setlocale(LC_ALL, "RU");
	//oracle_example();
	//TOracleTest();
	string file_name("config.ini");
	//Test_toStr();
	//Test_Logger();
	//test_Const_Module();
	//TOracleTest();
	//TSqlParse("rib_docs_for_period.sql");
	//parse_tune_file(file_name);
	
	//excel_example();
	CreateSimpleReport(file_name);
	/*
	cout << "Начало работы!" << endl;
	try
	{
		SQL_Text st("select ID as \"ID\" from dual;");
		cout << st.ToString() << endl;
//		oracle_example();
	}
	catch (const Logger::TLog& er)
	{
		cerr << "Ошибка " << er.what() << endl;
	}
	catch (const oracle::occi::SQLException& er)
	{
		cerr << "Ошибка соединения: " << er.what() << endl;
	}
	catch (const std::exception& er)
	{
		cerr << "Ошибка: " << er.what() << endl;
	}
/**/
	return 0;
}

void test_Const_Module(void)
{
	using namespace NS_Const;
	using std::cout;
	using std::endl;
	cout << "Тестирование модуля констант!" << endl;
	cout << "TConstCtrlSym: " << endl;
	TConstCtrlSym cs(CtrlSym::EndCommand);
	cout << cs.toStr() << endl;
	cout << "TConstType: " << endl;
	TConstType ct(DataType::Date);
	cout << ct.toStr() << endl;
	cout << "TConstSql: " << endl;
	TConstSql sql(TSql::Group);
	cout << sql.Next().toStr() << endl;
	cout << "TConstReportCode: " << endl;
	TConstReportCode rc(ReportCode::BALANCE_LIST);
	cout << rc.toStr() << endl;
	cout << "TConstField: " << endl;
	NS_Const::TConstField cf(NS_Const::TuneField::Column);
	cout << cf.toStr() << endl;
	cout << "TConstExclTune: " << endl;
	NS_Const::TConstExclTune et(NS_Const::TExclBaseTune::DefSh);
	cout << et.toStr() << endl;
}

void Test_toStr()
{
	using std::cout;
	using std::endl;
	using NS_Converter::toStr;
	std::string tmp;
	tmp += toStr("Hello World");
	tmp += toStr(unsigned short(2));
	tmp += toStr(2.005);
	cout << tmp << endl;
}

void Test_Logger()
{
	using std::cout;
	using std::endl;
	using boost::gregorian::date;
	using boost::gregorian::day_clock;
	using NS_Logger::TLog;
	TLog log("Text for error: ");
	log << "integer " << 4;
	log << '\n';
	log << "string: " << string("Hello world!") << '\n';
	date d = day_clock::local_day();
	log << "boost::date: " << d.day() << '\n';
	log << "double: " << 2.005 << '\n';
	cout << log.what() << endl;
}

void TSqlParse(const std::string& str)
{
	using std::ifstream;
	using std::cout;
	using std::endl;
	using std::ios_base;
	using std::stringstream;
	//using std::istream_iterator;
	using std::istream_iterator;
	using std::ostream_iterator;
	using std::insert_iterator;
	using std::string;
	using NS_Sql::TSection;
	using NS_Sql::TSql;
	using NS_Sql::TText;
	//using NS_Logger::TLog;

	ifstream f(str.c_str(), ios_base::in);
	//if (!f.is_open()) TLog("Ошибка открытия файла: " + str).raise(true, "TSqlParse");
	TText sql_txt(f);
	cout << sql_txt.toStr() << endl;
	sql_txt.AddField2Section(TSql::Select, "sysdate as \"Дата отчета\"");
	cout << sql_txt.toStr() << endl;
	f.close();
};

void parse_tune_file(const std::string& filename)
{
	using NS_Tune::TUserData;
	using std::cout;
	using std::endl;
	TUserData tune(filename);
	cout << "Список колонок:" << endl;
	tune.show_columns();
	cout << "Список настроек:" << endl;
	tune.show_tunes();
	cout << "Список параметров: " << endl;
	tune.show_params();
	cout << "Значение настройки SQLText: " << tune.getFieldByCode(NS_Tune::TuneField::SqlText) << endl;
};
/**/
void TOracleTest()
{
	using NS_Oracle::TDBConnect;
	using NS_Oracle::TConnectParam;
	using NS_Oracle::TStatement;
	using NS_Oracle::TResultSet;
	using NS_Oracle::TDate;
	using NS_Oracle::UInt;
	using std::string;
	using std::cout;
	using std::endl;
	TConnectParam param{ "ZP_IBS", "IBS", "SML_SM", false, 10 };
	string sql = "select ID, Code, Name, DayTo from Admin.M_User";
	TDBConnect connect(param);
	TStatement st(connect, sql, param.prefetch_rows);
	TResultSet rs(st);
	std::cout << "Число выбранных столбцов: " << rs.getColumnsCnt() << '\n';
	for (UInt i = 1; i < rs.getColumnsCnt(); i++)
		std::cout << "Тип " << i << " столбца: " << static_cast<int>(rs.getColumnType(i)) << std::endl;
	std::cin.get();
	return;
}

/*
//пример работы с документами Excel
void excel_example(void)
{
	using namespace libxl;
	using std::cout;
	using std::endl;
	Example e;
	e.run();
	return;
	libxl::Book* book = xlCreateBook();
	book->load("C:\\1.xls");
	if (book)
	{
		cout << "book is load" << endl;
		libxl::Sheet* sheet = book->addSheet("MyList");
		if (sheet)
		{
			cout << "open sheet and write text!" << endl;
			sheet->writeStr(1, 1, "Hello World!");
			sheet->writeNum(2, 1, 112);
			sheet->writeNum(3, 1, 2.5);
			libxl::Font* font = book->addFont();
			font->setColor(COLOR_RED);
			font->setBold(true);
		}
	}
	book->save("C:\\1.xls");
}

/**/
//пример работы с базой oracle
/*
void oracle_example(void) noexcept(false)
{
	using namespace oracle::occi;
	using std::string;
	using std::cout;
	using std::endl;
	const string userName = "ADMIN";
	const string password = "ADMIN";
	const string connectString = "SML_SM_COPY";
		//"(DESCRIPTION=(ADDRESS=(PROTOCOL=TCP)(HOST=10.67.0.120)(PORT=1521))(CONNECT_DATA=(SERVER=dedicated)(SERVICE_NAME=ORC1)))";
	Environment* env = Environment::createEnvironment(Environment::Mode::DEFAULT);
	{
		try
		{
			Connection* con = env->createConnection(userName, password, connectString);
			if (con)
			{
				cout << "Соединение с базой данных установлено!" << endl;
				string sql = "select ID, Code from Admin.M_User";
				Statement* st = con->createStatement(sql);
				ResultSet* rs = st->executeQuery();
				rs->setPrefetchRowCount(10);
				if (st->status() != Statement::Status::UNPREPARED)
				{
					cout << "Был выполнен запрос: " << sql << endl;
					if (rs)
					{
						cout << "В зрезультате получено: ";
						static unsigned int val;
						while (rs->next())
						{
							val = rs->getInt(1);
//							tmps = rs->getString(2);
							cout << "ID: " << val << '\t' << endl;
							cout << "Code: " << rs->getString(2) << endl;
						}

					}
					st->closeResultSet(rs);
					con->terminateStatement(st);
				}
				env->terminateConnection(con);
				cout << "Соединение с базой данных закрыто!" << endl;
			}
		}
		catch (const SQLException& er)
		{
			std::cerr << er.what() << endl;
		}
	}
	Environment::terminateEnvironment(env);
}
/**/