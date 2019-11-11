//блок предназначен для сборки кода по созданию отчета:
//1. проверить наличие excel-файла
//2. проверить корректность соединения с базой данных
//3. заполняем названия колонок в excel-файле
//4. считываем данные из базы и записываем их в файл
//5. сохраняем файл

//для корректной работы в свойствах проекта - отключил UNICODE(кодировка unicode), 
//т.к. лень переписывать string на wstring
#include <iostream>
#include <fstream>
#include "Logger.h"
#include "TuneParam.h"
#include "TSQLParser.h"
#include "TExcel.h"
#include "TOracle.h"
#include "TConverter.h"
#include "TConverter.cpp"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

//функция получения параметров для подключения к БД
NS_Oracle::TConnectParam getConnectParams(const NS_Tune::TUserData& ud) noexcept(false);

//функция замены параметров в строке по разделителю:
void setSqlParamByTune(NS_Oracle::TStatement& sql, const NS_Tune::TSubParam& param, bool exit_on_err = true) noexcept(false);

//функция заполнения параметров в sql-команде из файла настроек:
void setSqlParamsByTunes(const NS_Oracle::TStatement& sql, const NS_Tune::TUserData& ud) noexcept(false);

//функция записи данных из ResultSet в excel-файл на активную страницу
bool ResultSet2Excel(NS_Oracle::TResultSet& rs, NS_Excel::TExcelBook& excl) noexcept(true);

//функция формирования excel-файла из настроек:
void CreateExcelFile(const NS_Tune::TUserData& param, NS_Oracle::TResultSet& rs) noexcept(true);

//функция создания простого отчета - данные берутся из файла и записываются в файл
bool CreateSimpleReport(const string& config_file) noexcept(true);

//функция формирования отчета:
bool CreateRepotByConfig(const string& directory) noexcept(true);


NS_Oracle::TConnectParam getConnectParams(const NS_Tune::TUserData& ud) noexcept(false)
{
	using NS_Oracle::TConnectParam;
	using NS_Tune::TuneField;
	TConnectParam result;
	if (ud.Empty()) return result;
	result.username = ud.getFieldByCode(TuneField::UserName);
	result.password = ud.getFieldByCode(TuneField::Password);
	result.tns_name = ud.getFieldByCode(TuneField::TNS);
	result.auto_commit = false;
	result.prefetch_rows = 200;
	return result;
}

void setSqlParamByTune(NS_Oracle::TStatement& sql, const NS_Tune::TSubParam& param, bool exit_on_err) noexcept(false)
{
	using NS_Tune::DataType;
	using NS_Converter::toType;
	//если параметр не указан:
	if (param.Value().empty()) return;
	//проверяем имеется ли нужный нам параметр:
	try
	{
		int par_id = param.ID();
		switch (param.DataType())
		{
		case DataType::Integer:
		{
			int val = 0;
			toType(param.Value(), &val);
			sql.setIntVal(par_id, val);
			return;
		}
		case DataType::Double:
		{
			double val = 0.0;
			toType(param.Value(), &val);
			sql.setDoubleVal(par_id, val);
			return;
		}
		case DataType::Date:
			sql.setDateAsStringVal(par_id, param.Value());
			return;
		case DataType::String:
			sql.setStringVal(par_id, param.Value());
			return;
		default: throw string("Указанный тип данных НЕ обрабатывается!");
		}
	}
	catch (const oracle::occi::SQLException& err)
	{
		std::string s;
		s ="Ошибка установки значения: " + param.Value() + " для параметра: " + param.Comment() + '\n';
		s += err.what() + '\n';
		if (exit_on_err)
		{
			cerr << s;
			return;
		}
		else throw s;
	}
	catch (...)
	{
		std::string s;
		s = "Неорпеделенная ошибка установки значения: " + param.Value() + " для параметра: " + param.Comment() + '\n';
		if (exit_on_err)
		{
			cerr << s;
			return;
		}
		else throw s;
	}
}

void setSqlParamsByTunes(NS_Oracle::TStatement& sql, const NS_Tune::TUserData& ud) noexcept(false)
{
	using NS_Logger::TLog;
	//если пустой sql - выход
	if (!sql.isValid()) TLog("Не валидная sql-команда: " + sql.getSQL()).raise(true, "setSqlParamsByTunes");
	string sql_text = sql.getSQL();
	if (sql_text.empty()) TLog("Пустой текст sql-команды").raise(true, "setSqlParamsByTunes");
	//проверяем количество параметров:
	for (const NS_Tune::TSubParam& p : ud.getParams())
		setSqlParamByTune(sql, p);
}

bool ResultSet2Excel(NS_Oracle::TResultSet& rs, NS_Excel::TExcelBook& excl) noexcept(true)
{
	using NS_Excel::TExcelBookSheet;
	using NS_Excel::TExcelCell;
	using NS_Excel::TExcelDate;
	using NS_Excel::TDataType;
	using NS_Oracle::TDataSetState;
	using NS_Oracle::TType;
	using NS_Oracle::UInt;
	using NS_Oracle::TDate;
	using NS_Oracle::SQLException;
	int act_index = excl.getActiveSheet();
	TExcelBookSheet sheet = excl.getSheetByIndex(act_index);
	UInt col_cnt = rs.getColumnsCnt();
	int row = excl.getActiveSheet() + 1;
	while (rs.Next())
	{
		for (UInt i = 1; i <= col_cnt; i++)
		{
			TExcelCell cell(row, i-1);
			switch (rs.getColumnType(i))
			{
			//числа с плавающей точкой:
			case TType::OCCIDOUBLE:
			case TType::OCCIFLOAT:
			case TType::OCCINUMBER:
			case TType::OCCIINT:
			case TType::OCCI_SQLT_NUM:
				try
				{
//					if (sheet.getCellType(cell) == TDataType::)
					sheet.WriteAsNumber(cell, rs.getDoubleVal(i));
				}
				catch (const SQLException& err)
				{
					cerr << "Ошибка считывания числового значения колонки: " << i;
					cerr << ", строки: " << row << endl;
					cerr << err.what() << endl;
				}
				catch (...)
				{
					cerr << "Необработанная ошибка записи числового значения колонки: " << i;
					cerr << ", строки: " << row << endl;
				}
					break;
			case TType::OCCI_SQLT_STR:
			case TType::OCCI_SQLT_CHR:
				try
				{
					sheet.WriteAsString(cell, rs.getStringVal(i));
				}
				catch (const SQLException& err)
				{
					cerr << "Ошибка считывания строки из колонки: " << i;
					cerr << ", строки: " << row << endl;
					cerr << err.what() << endl;
				}
				catch (...)
				{
					cerr << "Не обработанная ошибка записи строки из колонки: " << i;
					cerr << ", строки: " << row << endl;
				}
				break;
			case TType::OCCIBOOL:
				try
				{
					sheet.WriteAsBool(cell, rs.getIntVal(i));
				}
				catch (const SQLException& err)
				{
					cerr << "Ошибка считывания логического значения из колонки: " << i;
					cerr << ", строки: " << row << endl;
					cerr << err.what() << endl;
				}
				catch (...)
				{
					cerr << "Не обработанная ошибка записи логического значения из колонки: ";
					cerr << i << ", строки: " << row << endl;
				}
				break;
			case TType::OCCIDATE:
			case TType::OCCI_SQLT_DATE:
			case TType::OCCI_SQLT_DAT:
				try
				{
					if (sheet.isDate(cell))
					{
						TDate date = rs.getDateVal(i);
						TExcelDate tmp;
						date.getDate(tmp.year, tmp.month, tmp.day, tmp.hour, tmp.minute, tmp.sec);
						double dbl_date = excl.Date2Double(tmp);
						sheet.WriteAsNumber(cell, dbl_date);
					}
					else
						sheet.WriteAsString(cell, rs.getDateAsStrVal(i));
				}
				catch (const SQLException& err)
				{
					cerr << "Ошибка считывания даты из колонки: " << i;
					cerr << ", строки: " << row << endl;
					cerr << err.what();
				}
				catch (...)
				{
					cerr << "Не обработанная ошибка записи даты для колонки: " << i;
					cerr << ", строки: " << row << endl;
				}
				break;
			default:
				cerr << "Указанный тип данных в " << i << " колонке - НЕ обрпбатывается!";
				break;
			}
		}
		row++;
	}
	return true;
}

void CreateExcelFile(const NS_Tune::TUserData& param, NS_Oracle::TResultSet& rs) noexcept(true)
{
	using NS_Excel::TExcelParam;
	using NS_Excel::TExcelBook;
	using NS_Tune::TuneField;
	using std::cerr;
	using std::endl;
	//инициализация структуры параметров отчета:
	TExcelParam exparam(param.getFieldByCode(TuneField::TemplateName), param.getFieldByCode(TuneField::OutFileName),
		param.getColumns());
	//формирование excel-документа:
	TExcelBook excl;
	if (excl.initByParam(exparam) == false)
	{
		cerr << "Ошибка при загрузке/формировании excel-документа!" << endl;
		return;
	}
	//записываем данные в отчет
	if (ResultSet2Excel(rs, excl) == false)
	{
		cerr << "Ошибка при записи данных в excel-файл из запроса!" << endl;
		return;
	}
	if (excl.SaveToFile(exparam.getOutName()) == false)
	{
		cerr << "Ошибка при сохранении файла отчета: " << exparam.getOutName() << endl;
		return;
	}
	cout << "Отчет сформирован и записан в файл: " << exparam.getOutName() << endl;
}

bool CreateSimpleReport(const string& config_file) noexcept(true)
{
	using NS_Tune::TUserData;
	using NS_Tune::TuneField;
	using NS_Sql::TText;
	using NS_Oracle::TConnectParam;
	using NS_Oracle::TDBConnect;
	using NS_Oracle::TStatement;
	using NS_Oracle::TResultSet;

	using std::ifstream;
	using std::ios_base;
	using std::ostream;
	auto no_report = [](ostream& stream)->bool { if (stream) stream << "Отчет не сформирован!" << endl; return false; };
	TUserData config(config_file);
	//если данные из настроек заполнены не корректно - выход:
	if (config.Empty())
	{
		cerr << "Не заполнен файл настроек: " << config_file << endl;
		return no_report(cerr);;
	}
	//после получения данных из файла конфигурации - формируем текст sql-запроса:
	TText sql;
	if (!config.getFieldByCode(TuneField::SqlText).empty())
		sql = TText(config.getFieldByCode(TuneField::SqlText));
	else
	{
		ifstream sql_txt_file(config.getFieldByCode(TuneField::SqlFile), ios_base::in);
		if (!sql_txt_file.is_open())
		{
			cerr << "Ошибка открытия файла: " << config.getFieldByCode(TuneField::SqlFile) << endl;
			return no_report(cerr);
		}
		sql = TText(sql_txt_file);
		sql_txt_file.close();
	}
	if (sql.isEmpty())
	{
		cerr << "Ошибка при получении SQL запроса! Проверьте файл настроек: " << config_file << endl;
		return no_report(cerr);
	}
	//формируем данные для подключения к БД:
	TConnectParam connect_param = getConnectParams(config);
	//формируем подключение к БД:
	TDBConnect connect(connect_param);
	if (connect.isValid())
	{
		//создание sql-команды:
		TStatement st(connect, sql.toStr(), connect_param.prefetch_rows);
		//устанавливаем параметры запроса:
		setSqlParamsByTunes(st, config);
		//выполняем запрос:
		TResultSet rs(st);
		//функция записи данных в excel из результата запроса:
		CreateExcelFile(config, rs);
		rs.close();
	}
	else
	{
		cerr << "Ошбка создания соединения с базой: " << connect_param.tns_name << endl;
		return no_report(cerr);
	}
	return true;
}

bool CreateRepotByConfig(const string& directory) noexcept(true)
{
	using std::cerr;
	using std::endl;
	using std::vector;
	using std::string;
	using NS_Tune::TUserData;
	using NS_Excel::TExcelBook;
	if (directory.empty())
	{
		cerr << "Указана пустая директория источника отчета!" << endl;
		return false;
	}
	vector<string> name_arr;
	vector<TUserData> confg_arr;
	TExcelBook excl;
	//excl.initByParam
		//загрузка имен файлов конфигов для отчета из указанной папки
		//для каждого файла конфига вызываем формирование отчета для страницы
		//сохраняем excel-документ в файл
	return false;
}
