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
#include <exception>
//#include "Logger.hpp"
#include "TConstants.h"
//#include "TConverter.hpp"
#include "TOracle.h"
#include "TSQLParser.h"
#include "TuneParam.h"
#include "TDBReport.h"
/**/
#include "libxl.h"
#include "occi.h"
//#include "LibXL_Example.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/date_facet.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include "TSMLVCH_IMP.h"

using std::string;
//using NS_Logger::TLog;

void excel_example(void);
void excel_test(void);
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
void CreateReport(const std::string& file_name, const std::string& code = "") noexcept(true);

//inline void SetRuConsole(int cp) { SetConsoleCP(cp); SetConsoleOutputCP(cp); };

void test_Const_Module(void);

void JsonParse(const std::string& filename);

//функционал обработки данных по Смолевичу
//считываени txt файла


int main()
{
	using std::cerr;
	using std::cout;
	using std::endl;
	using std::string;
	using std::locale;
	using std::cout;
	using NS_SMLVCH_IMP::TImportAccount;
	//SetRuConsole(1251);
	setlocale(LC_ALL, "RU");
	/*
	//Формирование ведомости остатков Смолевич
	string path("F:\\Projects\\SomeThing\\TypicalReport\\Смолевич\\Ведомость\\template\\files\\");
	//imp_data(path, NS_Const::CtrlSym::txt_delimeter)
	NS_ExcelReport::TReport::Smolevich_Sld_Report(path, "Ведомость Смолевич.xls");
	return 0;
	/**/
	//Формирование документов на импорт для Смолевича
	string path("F:\\Projects\\SomeThing\\TypicalReport\\Смолевич\\Иморт документов\\template");
	NS_ExcelReport::TReport::Smolevich_Imp_Docs(path, "imp_docs.txt");
	return 0;
	//	excel_test();
/*
	NS_Tune::TExcelProcData exl("F:\\Projects\\SomeThing\\TypicalReport\\Полный портфель\\config\\json\\nat_person_tune.json",
		"F:\\Projects\\SomeThing\\TypicalReport\\Полный портфель\\");
	exl.show();
	return 0;
/**/
	//	excel_test();
	//oracle_example();
	//TOracleTest();
	string config("config.ini");
	//parse_tune_file(file_name);
	CreateReport(config);
	//CreateReport(config, "REPAYMENT_FOR_DATE");
	/*
	Test_toStr();
	Test_Logger();
	test_Const_Module();
	TOracleTest();
	TSqlParse("rib_docs_for_period.sql");
	parse_tune_file(file_name);
	/**/
	//CreateReport(file_name);
	//excel_example();
	//CreateSimpleReport(file_name);
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
/*
void JsonParse(const string& file)
{
	using boost::property_tree::ptree;
	using boost::property_tree::file_parser_error;
	using boost::property_tree::json_parser_error;
	using boost::property_tree::json_parser::read_json;
	using std::string;
	using NS_Const::JsonParams;
	using NS_Const::TConstJson;
	using filter_data = std::pair<size_t, string>;
	using filters = std::vector<filter_data>;
	const size_t EmptyVal = 0;
	try
	{
		ptree json;
		read_json(file, json);
		if (json.empty()) throw TLog("Пустой файл: " + file, "JsonParse");
		filters fltr;
		size_t ilist = EmptyVal;
		size_t istart = EmptyVal;
		string name;
		//параметры json
		string dst_file = "DstFile";
		string src_file = "SrcFile";
		string cells = "Cells";
		std::vector<string> file_param = { "name", "lst_indx", "strt_indx", "fltr" };
		std::vector<string> filter_param = { "col_indx", "value" };
		std::vector<string> cells_param = { "dst_indx", "src_indx", "dst_ins_indx" };
		//сам парсинг:
		//проверка наличия DstFile
		ptree::value_type it = json.find(dst_file).dereference();
		//выход, если данный параметр пустой
		if (it.second.empty()) return;
		name = it.second.get_child(file_param[0]).get_value<string>();
		if (!NS_Converter::UTF8ToANSI(name)) return;
		ilist = it.second.get<size_t>(file_param[1], 0);
		ilist = it.second.get_child(file_param[1]).get_value<size_t>();
		istart = it.second.get_child(file_param[2]).get_value<size_t>();
		for (const ptree::value_type& v : it.second.get_child(file_param[3]))
		{
			if (v.second.empty()) continue;
			filter_data val;
			val.first = v.second.get_child(filter_param[0]).get_value<size_t>();
			if (val.first != EmptyVal)
			{
				val.second = v.second.get_child(filter_param[1]).get_value<string>();
				if (!NS_Converter::UTF8ToANSI(val.second)) continue;
				fltr.push_back(val);
			}
		}

		for (auto& v : fltr)
		{
			std::cout << "col index: " << v.first << '\t' << "col_val: " << v.second << std::endl;
		}
	}
	catch (const json_parser_error& err)
	{
		TLog(err.what()).toErrBuff();
	}
	catch (const std::exception& err)
	{
		TLog(err.what()).toErrBuff();
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка!").toErrBuff();
	}
	return;
}

void test_Const_Module(void)
{
	using namespace NS_Const;
	using std::cout;
	using std::endl;
	cout << "Тестирование модуля TConstant:" << endl;
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
	cout << endl;
}

void Test_toStr()
{
	using std::cout;
	using std::endl;
	std::string tmp;
	cout << "Тестирование модуля TConverter" << endl;
	tmp += NS_Converter::toStr("Hello World");
	tmp += NS_Converter::toStr(unsigned short(2));
	tmp += NS_Converter::toStr(2.005);
	cout << tmp << endl;
	cout << endl;
	cout << endl;
}

void Test_Logger()
{
	using std::cout;
	using std::endl;
	using boost::gregorian::date;
	using boost::gregorian::day_clock;
	using NS_Logger::TLog;
	cout << "Тестирование модуля Logger" << endl;
	TLog log("Text for error: ");
	log << "integer " << 4;
	log << '\n';
	log << "string: " << string("Hello world!") << '\n';
	date d = day_clock::local_day();
	log << "boost::date: " << d.day() << '\n';
	log << "double: " << 2.005 << '\n';
	cout << log.what() << endl;
	cout << endl;
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
	cout << "Тестирование модуля TSqlParse" << endl;
	ifstream f(str.c_str(), ios_base::in);
	//if (!f.is_open()) TLog("Ошибка открытия файла: " + str).raise(true, "TSqlParse");
	TText sql_txt(f);
	cout << sql_txt.toStr() << endl;
	sql_txt.AddField2Section(TSql::Select, "sysdate as \"Дата отчета\"");
	cout << sql_txt.toStr() << endl;
	f.close();
	cout << endl;
};

void parse_tune_file(const std::string& filename)
{
	using NS_Tune::TUserTune;
	using NS_Tune::TSharedTune;
	using NS_Const::ReportCode;
	using NS_Const::TuneField;
	using std::cout;
	using std::endl;
	cout << "Тестирование модуля TTune:" << endl;
	cout << "Считывание общих настроек файла:" << endl;
	//TSharedTune main_tune(filename, "DOCS_MF_SF_FOR_PERIOD");
	TSharedTune main_tune(filename, "RIB_DOCS_FOR_PERIOD");
	cout << "Путь к файлам отчета: " << main_tune.getMainPathVal() << endl;
	main_tune.show_tunes();
	string subFile = main_tune.getMainPathVal() + main_tune.getFieldValueByCode(TuneField::ConfigPath) + filename;
	TUserTune tune(main_tune, subFile);
	cout << "Список колонок:" << endl;
	tune.show_columns();
	cout << "Список настроек:" << endl;
	tune.show_tunes();
	cout << "Список параметров: " << endl;
	tune.show_params();
	cout << "Значение настройки SQLText: " << tune.getFieldValueByCode(NS_Tune::TuneField::SqlText) << endl;
	cout << endl;
};

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
	cout << "Тестирование модуля TOracle:" << endl;
	TConnectParam param{ "ZP_IBS", "IBS", "SML_SM", false, 10 };
	string sql = "select ID, Code, Name, DayTo from Admin.M_User";
	TDBConnect connect(param);
	TStatement st(connect, sql, param.prefetch_rows);
	TResultSet rs(st);
	std::cout << "Число выбранных столбцов: " << rs.getColumnsCnt() << '\n';
	for (UInt i = 1; i < rs.getColumnsCnt(); i++)
		std::cout << "Тип " << i << " столбца: " << static_cast<int>(rs.getColumnType(i)) << std::endl;
	std::cin.get();
}
/**/
void CreateReport(const std::string& file_name, const string& code) noexcept(true)
{
	using NS_ExcelReport::TReport;
	using std::cout;
	using std::cin;
	using std::endl;
	using std::getline;
	using NS_Const::LowerCase;
	using NS_Tune::TSharedTune;
	char answer;
	do
	{
		TReport report(file_name);
		cout << "Завершить работу программы?\n";
		cin >> answer;
		answer = tolower(answer);
	} while (answer != 'y');
}

void excel_test(void)
{
	using namespace libxl;
	Book* srcBook = xlCreateXMLBook();
	NS_Excel::TExcelBook::UseLicenseKey(&srcBook);
	//F:\\Projects\\SomeThing\\TypicalReport\\Кредитный портфель excel\\RIB\\template\\RIB_Data.xlsx
	string name("F:\\Projects\\SomeThing\\TypicalReport\\Кредитный портфель excel\\RIB\\template\\RIB_Data.xlsx");
//	bool b = srcBook->loadSheet(name.c_str(), 0);
	string tmp = "";
	bool b = srcBook->load(name.c_str());
	if (!b)
	{
		std::cerr << srcBook->errorMessage() << std::endl;
		return;
	}
	//ключи лицензии:
	Sheet* srcSheet = srcBook->getSheet(srcBook->activeSheet());

	Book* dstBook = xlCreateBook();
	NS_Excel::TExcelBook::UseLicenseKey(&dstBook);
	Sheet* dstSheet = dstBook->addSheet("Страница 1");

	// setting column widths
	for (int col = srcSheet->firstCol(); col < srcSheet->lastCol(); ++col)
	{
		dstSheet->setCol(col, col, srcSheet->colWidth(col), 0, srcSheet->colHidden(col));
	}

	std::map<Format*, Format*> formats;

	for (int row = srcSheet->firstRow(); row < srcSheet->lastRow(); ++row)
	{
		// setting row heights
		dstSheet->setRow(row, srcSheet->rowHeight(row), 0, srcSheet->rowHidden(row));

		for (int col = srcSheet->firstCol(); col < srcSheet->lastCol(); ++col)
		{
			// copying merging blocks
			int rowFirst, rowLast, colFirst, colLast;
			if (srcSheet->getMerge(row, col, &rowFirst, &rowLast, &colFirst, &colLast))
			{
				dstSheet->setMerge(rowFirst, rowLast, colFirst, colLast);
			}

			// copying formats
			Format* srcFormat, * dstFormat;

			srcFormat = srcSheet->cellFormat(row, col);
			if (!srcFormat) continue;

			// checking formats
			if (formats.count(srcFormat) == 0)
			{
				// format is not found, creating it in the output file
				dstFormat = dstBook->addFormat(srcFormat);
				formats[srcFormat] = dstFormat;
			}
			else
			{
				// format was already created
				dstFormat = formats[srcFormat];
			}

			// copying cell's values
			CellType ct = srcSheet->cellType(row, col);
			switch (ct)
			{
			case CELLTYPE_NUMBER:
			{
				double value = srcSheet->readNum(row, col, &srcFormat);
				dstSheet->writeNum(row, col, value, dstFormat);
				break;
			}
			case CELLTYPE_BOOLEAN:
			{
				bool value = srcSheet->readBool(row, col, &srcFormat);
				dstSheet->writeBool(row, col, value, dstFormat);
				break;
			}
			case CELLTYPE_STRING:
			{
				const char* s = srcSheet->readStr(row, col, &srcFormat);
				dstSheet->writeStr(row, col, s, dstFormat);
				break;
			}
			case CELLTYPE_BLANK:
			{
				srcSheet->readBlank(row, col, &srcFormat);
				dstSheet->writeBlank(row, col, dstFormat);
				break;
			}
			}
		}
	}

	dstBook->save("out.xls");

	dstBook->release();

	srcBook->release();

}
/**/
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