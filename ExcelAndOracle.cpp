﻿// ExcelAndOracle.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//Описание работы с libxl: http://www.libxl.com/
//FAQ OCCI:
// https://docs.oracle.com/en/database/oracle/oracle-database/19/lncpp/introduction-to-occi.html#GUID-99077609-BFCC-4FF7-9D23-C45623DDD465
//типы данных:
//https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/Data-Types.html#GUID-7B72E154-677A-4342-A1EA-C74C1EA928E6
//Добавить в PATH: D:\NewPrg\C++\Libs\oracle\occi-oracle\sdk\dll;
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <locale.h>
#include "libxl.h"
#include "occi.h"
//#include "TuneParam.h"
#include "Logger.h"
#include "TSQLParser.h"
#include "TuneParam.h"
//#include <consoleapi2.h>

void excel_example(void);
void oracle_example(void);

//функция проверки парсинга для sql-запроса:
void TSqlParse(const std::string& str);

//функция парсинга настроек параметров:
void parse_tune(const std::string& str);

void parse_tune_file(const std::string& filename);

//определение внешней функции
extern void getReport(const std::string& file_name);

//inline void SetRuConsole(int cp) { SetConsoleCP(cp); SetConsoleOutputCP(cp); };

int main()
{
	using std::cerr;
	using std::cout;
	using std::endl;
	using std::string;
	//SetRuConsole(1251);
	setlocale(LC_ALL, "RU");
	string file_name("config.ini");
	getReport(file_name);
	//parse_tune_file(file_name);
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
	using NS_Sql::TCtrlGroup;
	using NS_Sql::TText;

	ifstream f(str.c_str(), ios_base::in);
	if (!f.is_open()) throw Logger::TLog("Ошибка открытия файла: ", str.c_str(), nullptr);
	TText sql_txt(f);
	cout << sql_txt.toStr() << endl;
	sql_txt.AddField2Section(TCtrlGroup::TCtrlSql::Select, "sysdate as \"Дата отчета\"");
	cout << sql_txt.toStr() << endl;
};

void parse_tune(const std::string& str)
{
	using std::string;
	NS_Tune::TuneField f = NS_Tune::TuneField::UserName;
	std::cout << "Значение параметра: " << NS_Tune::TuneFieldToStr(f) << " = " << NS_Tune::Get_TuneFiel_Val_From_Str(f, str) << std::endl;
}

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
	cout << "Значение настройки SQLText: " << tune.getValue(NS_Tune::TuneField::SqlText) << endl;
};

//пример работы с документами Excel
void excel_example(void)
{
	using namespace libxl;
	using std::cout;
	using std::endl;
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

//пример работы с базой oracle
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
	Environment* env = Environment::createEnvironment(Environment::Mode::NO_USERCALLBACKS);
	{
		Connection* con = env->createConnection(userName, password, connectString);
		if (con)
		{
			cout << "Соединение с базой данных установлено!" << endl;
			char ch = std::cin.get();
			env->terminateConnection(con);
			cout << "Соединение с базой данных закрыто!" << endl;
		}
	}
	Environment::terminateEnvironment(env);
}
