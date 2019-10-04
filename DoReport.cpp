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

bool CreateSimpleReport(const string& config_file) noexcept(true);


bool CreateSimpleReport(const string& config_file) noexcept(true)
{
	using NS_Tune::TUserData;
	using NS_Tune::TuneField;
	using NS_Sql::TText;
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
	//добавляем в sql-запрос параметр:
	//не решена проблема, когда параметр будет в самом тексте запроса
	//надо в конфиге сделать: param1="Дата начала"
	//и заменять :1 в тексте sql-запроса на значениея param1
	return true;
}