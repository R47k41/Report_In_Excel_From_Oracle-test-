// ExcelAndOracle.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
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
#include "libxl.h"
#include "occi.h"
#include <locale.h>
//#include "TuneParam.h"
#include "Logger.h"
#include "TSQLParser.h"
//#include <consoleapi2.h>

void excel_example(void);
void oracle_example(void);

//функция проверки парсера для TField:
void TFieldParse(const std::string& str);

//функция проверки парсинга для sql-запроса:
void TSqlParse(const std::string& str);

//inline void SetRuConsole(int cp) { SetConsoleCP(cp); SetConsoleOutputCP(cp); };

int main()
{
	using std::cerr;
	using std::cout;
	using std::endl;
	using std::string;
	//SetRuConsole(1251);
	setlocale(LC_ALL, "RU");
	string file_name("rib_docs_for_period.sql");
	TSqlParse(file_name);
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

void TFieldParse(const std::string& str)
{
	using NS_Sql::TField;
	using std::cout;
	using std::endl;
	TField col(str, NS_Sql::TCtrlGroup::TCtrlSym::Space);
	cout << col.Val() << '\n' << col.Delimeter() << '\n' << col.Title() << endl;
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

//пример работы с документами Excel
void excel_example(void)
{
	using namespace libxl;
	using std::cout;
	using std::endl;
	libxl::Book* book = xlCreateBook();
	book->load(L"C:\\1.xls");
	if (book)
	{
		cout << "book is load" << endl;
		libxl::Sheet* sheet = book->addSheet(L"MyList");
		if (sheet)
		{
			cout << "open sheet and write text!" << endl;
			sheet->writeStr(1, 1, L"Hello World!");
			sheet->writeNum(2, 1, 112);
			sheet->writeNum(3, 1, 2.5);
			libxl::Font* font = book->addFont();
			font->setColor(COLOR_RED);
			font->setBold(true);
		}
	}
	book->save(L"C:\\1.xls");
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
