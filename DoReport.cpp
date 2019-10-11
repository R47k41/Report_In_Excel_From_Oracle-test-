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

//функция записи данных из ResultSet в excel-файл
void ResultSet2Excel(const NS_Oracle::TResultSet& rs, const NS_Excel::TExcel& excl);

//функция формирования excel-файла из настроек:


//функция создания простого отчета - данные берутся из файла и записываются в файл
bool CreateSimpleReport(const string& config_file) noexcept(true);


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
			NS_Tune::Str2Type(param.Value(), val);
			sql.setIntVal(par_id, val);
			return;
		}
		case DataType::Double:
		{
			double val = 0.0;
			NS_Tune::Str2Type(param.Value(), val);
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
	//если пустой sql - выход
	if (!sql.isValid()) throw string("Не валидная sql-команда!");
	string sql_text = sql.getSQL();
	if (sql_text.empty()) throw string("Пустой текст sql-команды");
	//проверяем количество параметров:
	for (const NS_Tune::TSubParam& p : ud.getParams())
		setSqlParamByTune(sql, p);
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

		//закрываем resultSet
		//rs.close();
		//закрываем sql-команду:
		//st.close();
		//закрываем соединение:
		//connect.closeConnection();
	}
	else
	{
		cerr << "Ошбка создания соединения с базой: " << connect_param.tns_name << endl;
		return no_report(cerr);
	}
	return true;
}