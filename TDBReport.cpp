//модуль определения функционала для TDBReport
#include <iostream>
#include <fstream>
#include "TDBReport.h"
#include "TSQLParser.h"
#include "TConverter.h"
#include "Logger.hpp"


using std::string;
using NS_Logger::TLog;
using NS_Excel::TExcelBook;
using NS_ExcelReport::TSheetReport;
using NS_ExcelReport::TDataBaseSheetReport;
using NS_ExcelReport::TReport;
using NS_Const::TuneField;
using NS_Oracle::TConnectParam;

const int DEF_TEMPL_SH_INDX = 0;

void raise_app_err(const TLog& log, bool as_raise = true);

void raise_app_err(const TLog& log, bool as_raise)
{
	as_raise ? throw log : log.toErrBuff();
}

TSheetReport::TSheetReport(NS_Excel::TExcelBook& book_link, const NS_Tune::TUserTune& config): book(book_link), sheet(nullptr), tune(config)
{
	SetSheetByTune();
}

TSheetReport::TSheetReport(TExcelBook& ref_book, const NS_Excel::TExcelBookSheet& ref_sheet, const NS_Tune::TUserTune& ref_tune):
	book(ref_book), sheet(ref_sheet), tune(ref_tune)
{
	//если страница пустая:
	if (!ref_sheet.isValid())
		//создаем ее
		SetSheetByTune();
}


bool TSheetReport::SetSheetByTune(const string& name) noexcept(true)
{
	using NS_Excel::TStrArr;
	const bool active_sheet = true;
	//проверка настраиваемых параметров
	if (tune.Empty())
	{
		TLog("Пустые данные из файла настроек!", "TSheetReport::SetSheetTune").toErrBuff();
		return false;
	}
	//проверяем указан ли шаблон
	string tmp_val = tune.getTemplateFile();
	string sh_name = name;
	if (sh_name.empty()) sh_name = tune.getOutSheet();
	//если шаблон не указан - создаем страницу
	if (tmp_val.empty())
	{
		//получение наименование 
		TStrArr head = tune.getColumns();
		book.setHeaderByStrArr(head, false, sh_name);
		sheet = book.getActiveSheet();
	}
	//если шаблон указан - грузим его
	else
	{
		if (book.setSheetByTemplate(tmp_val, sh_name, DEF_TEMPL_SH_INDX, active_sheet))
		{
			sheet = book.getActiveSheet();
		}
/*
		book.loadSheetOnly(tmp_val, templ_index_sheet, true);
		sheet = book.getLastSheet(active_sheet);
		sheet.setName(sh_name);
/**/
	}
	if (!sheet.isValid())
		return false;
	return true;
}

bool TSheetReport::CreateNewPage(size_t cur_val, bool byRows)
{ 
	//не понятно что делать при превышении числа столбцов
	if (!byRows)
	{
		size_t max_val = book.MaxColsCount();
		if (cur_val > max_val)
		{
			TLog log("Превышено число столбцов в отчете на одном листе! Число столбцов в выборке: ", "WriteFromResultSet");
			log << cur_val << TLog::NL << "Число столбцов в excel-книге: " << max_val;
			throw log;
		}
	}
	else
	{
		size_t max_val = book.MaxRowsCount();
		if (cur_val >= max_val)
		{
			int indx = book.getActiveSheetIndx() + 1;
			string sh_delimeter = NS_Const::TConstExclTune::asStr(NS_Const::TExclBaseTune::PageDelimiter);
			//задаем имя новой странице
			string new_name = sheet.getName() + sh_delimeter + NS_Converter::toStr(indx+1);
			//добавление страницы
			SetSheetByTune(new_name);
			return true;
		}
	}
	return false;
}

TConnectParam TDataBaseSheetReport::getConnectParam(const NS_Tune::TUserTune& param, int prefetch) noexcept(false)
{
	TConnectParam result;
	if (param.Empty()) throw TLog("Параметры отчета не заполнены!", "TDataBaseSheetReport::getConnectParam()");
	result.username = param.getFieldValueByCode(TuneField::UserName);
	result.password = param.getFieldValueByCode(TuneField::Password);
	result.tns_name = param.getFieldValueByCode(TuneField::TNS);
	result.auto_commit = false;
	result.prefetch_rows = prefetch;
	return result;
}

template <typename T>
string TDataBaseSheetReport::getSqlText(bool by_str, const string& str) noexcept(false)
{
	using std::ifstream;
	//проверяем откуда получаем sql-текст: файл или строка
	if (by_str)
	{
		T sql(str);
		return NS_Sql::AsString(sql);
	}
	else
	{
		ifstream sql_txt_file(str, std::ios_base::in);
		if (!sql_txt_file.is_open())
		{
			throw TLog("Ошибка открытия файла: " + str, "TDataBaseSheetReport::getSqlText");
		}
		T sql(sql_txt_file);
		sql_txt_file.close();
		return NS_Sql::AsString(sql);
	}
	return string();
}

void TDataBaseSheetReport::setSqlParamByTune(NS_Oracle::TStatement& sql, const NS_Tune::TSubParam& param, bool use_raise) noexcept(false)
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
		default: 
			throw TLog("Указанный тип данных " + NS_Const::TConstType(param.DataType()).toStr() + " НЕ обрабатывается!", "TDataBaseSheetReport::setSqlParamByTune");
		}
	}
	catch (const oracle::occi::SQLException& err)
	{
		TLog log("Ошибка установки значения: " + param.Value() + " для параметра: " + param.Comment() + TLog::NL, "TDataBaseSheetReport::setSqlParamByTune");
		log << err.what() << TLog::NL;
		raise_app_err(log, use_raise);
	}
	catch (const TLog& er)
	{
		raise_app_err(er, use_raise);
	}
	catch (...)
	{
		TLog log("Неорпеделенная ошибка установки значения: " + param.Value() + " для параметра: " + param.Comment() + TLog::NL);
		raise_app_err(log, use_raise);
	}
}

void TDataBaseSheetReport::setSqlParamsByTune(NS_Oracle::TStatement& sql, const NS_Tune::TUserTune& param) noexcept(false)
{
	//если пустой sql - выход
	if (!sql.isValid()) throw TLog("Не валидная sql-команда: " + sql.getSQL(), "setSqlParamsByTunes");
	string sql_text = sql.getSQL();
	if (sql_text.empty()) throw TLog("Пустой текст sql-команды", "setSqlParamsByTunes");
	//проверяем количество параметров:
	for (const NS_Tune::TSubParam& p : param.getParams())
		setSqlParamByTune(sql, p);
}

NS_Const::DataType TDataBaseSheetReport::convertOraType(const NS_Oracle::TType& type) noexcept(true)
{
	using NS_Oracle::TType;
	using NS_Const::DataType;
	using NS_Const::TConstType;
	switch (type)
	{
	//числа с плавающей точкой:
	case TType::OCCINUMBER:
	case TType::OCCIINT:
		return DataType::Integer;
	case TType::OCCIDOUBLE:
	case TType::OCCIFLOAT:
	case TType::OCCI_SQLT_NUM:
		return DataType::Double;
	case TType::OCCI_SQLT_STR:
	case TType::OCCI_SQLT_CHR:
	case TType::OCCI_SQLT_RDD:
	case TType::OCCI_SQLT_AFC:
	case TType::OCCI_SQLT_VST:
	case TType::OCCI_SQLT_AVC:
	case TType::OCCICHAR:
		return DataType::String;
	case TType::OCCIBOOL:
		return DataType::Boolean;
	case TType::OCCIDATE:
	case TType::OCCI_SQLT_DATE:
	case TType::OCCI_SQLT_DAT:
		return DataType::Date;
	}
	return DataType::ErrorType;
}

void TDataBaseSheetReport::setCellByResultSet(const NS_Oracle::TResultSet& rs, const NS_Excel::TExcelCell& cell) noexcept(false)
{
	using NS_Oracle::TType;
	using NS_Oracle::UInt;
	using NS_Oracle::SQLException;
	using NS_Oracle::TDate;
	using NS_Excel::TExcelDate;
	using NS_Const::DataType;
	if (!rs.isValid()) throw TLog("Не валдиный объект ResultSet!", "TDataBaseSheetReport::setCellByResultSet");
	//из-за несовпадений начала отсчета для excel и oracle
	UInt i = cell.getCol() + 1;
	const DataType dt = convertOraType(rs.getColumnType(i));
	try
	{
		switch (dt)
		{
			case DataType::Integer:
			case DataType::Double:
				sheet.WriteAsNumber(cell, rs.getDoubleVal(i));
				break;
			case DataType::String:
				sheet.WriteAsString(cell, rs.getStringVal(i));
				break;
			case DataType::Boolean:
				sheet.WriteAsBool(cell, rs.getIntVal(i));
				break;
			case DataType::Date:
				if (sheet.isDate(cell))
				{
					TDate date = rs.getDateVal(i);
					TExcelDate tmp;
					date.getDate(tmp.year, tmp.month, tmp.day, tmp.hour, tmp.minute, tmp.sec);
					double dbl_date = book.Date2Double(tmp);
					sheet.WriteAsNumber(cell, dbl_date);
				}
				else
					sheet.WriteAsString(cell, rs.getDateAsStrVal(i));
				break;
			default:
				TLog log("Указанный тип данных в ", "TDataBaseSheetReport::setCellByResultSet");
				log << i << " колонке - НЕ обрпбатывается!" << TLog::NL;
				log.toErrBuff();
				break;
		}
	}
	catch (const SQLException& err)
	{
		TLog log("Ошибка считывания ", "TDataBaseSheetReport::setCellByResultSet");
		log << NS_Const::TConstType(dt).toStr() << " данных из колонки :";
		log << i << ", строки: " << cell.getRow() << TLog::NL;
		log << err.what() << TLog::NL;
		log.toErrBuff();
	}
	catch (...)
	{
		TLog log("Не обработанная ошибка записи строки из колонки: ", "TDataBaseSheetReport::setCellByResultSet");
		log << i << ", строки: " << cell.getRow() << TLog::NL;
		log.toErrBuff();
	}
}

bool TDataBaseSheetReport::WriteFromResultSet(NS_Oracle::TResultSet& rs) noexcept(true)
{
	using NS_Oracle::UInt;
	using NS_Excel::TExcelCell;
	try
	{
		UInt col_cnt = rs.getColumnsCnt();
		//если число колонок превышает ограничения excel - выходим
		CreateNewPage(col_cnt, false);
		size_t row = sheet.getLastRow();
		while (rs.Next())
		{
			for (UInt i = 1; i <= col_cnt; i++)
			{
				TExcelCell cell(row, i-1);
				setCellByResultSet(rs, cell);
			}
			row++;
			//если привысили число строк на странице
			//создаем новую страницу
			if (CreateNewPage(row, true)) 
				row = sheet.getFirstRow() + 1;
		}
		return true;
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
	}
	catch (const NS_Oracle::SQLException& er)
	{
		TLog(er.getMessage(), "TDataBaseSheetReport::WriteFromResultSet").toErrBuff();
	}
	catch (...)
	{
		TLog("Необработання ошибка записи данных из базы на лист excel-документа!", "TDataBaseSheetReport::WriteFromResultSet").toErrBuff();
	}
	return false;
}

void TDataBaseSheetReport::FillSheetBySql(NS_Oracle::TDBConnect& db, const string& sql_txt, int prefetch)
{
	if (sql_txt.empty())
		throw TLog("Ошибка формирования отчета - пустой текст запроса!", "TDataBaseSheetReport::FillSheetBySql");
	if (!db.isValid())
		throw TLog("Ошибка формирования отчета - ошибка при подключении к БД!", "TDataBaseSheetReport::FillSheetBySql");
	//создание sql-команды:
	TStatement st(db, sql_txt, prefetch);
	//устанавливаем параметры запроса:
	setSqlParamsByTune(st, tune);
	//выполняем запрос:
	TResultSet rs(st);
	//функция записи данных в excel из результата запроса:
	WriteFromResultSet(rs);
	//закрываем источник данных запроса:
	rs.close();
}

bool TDataBaseSheetReport::useSqlParse() const noexcept(false)
{
	return tune.useFlag(TuneField::UseSqlParser);
}

bool TDataBaseSheetReport::isDQLFirst() const noexcept(false)
{
	return tune.useFlag(TuneField::SqlFirst);
}

string TDataBaseSheetReport::getSqlByTune(bool use_parse, bool by_str, const string& str) noexcept(true)
{
	string sql;
	if (use_parse)
		sql = getSqlText<NS_Sql::TText>(by_str, str);
	else
		sql = getSqlText<NS_Sql::TSimpleSql>(by_str, str);
	return sql;
}

void TDataBaseSheetReport::CrtBySqlLine(NS_Oracle::TDBConnect& db, const string& sql_line, int prefetch) noexcept(false)
{
	bool use_parse = useSqlParse();
	string sql = getSqlByTune(use_parse, true, sql_line);
	//формирование данных для страницы
	FillSheetBySql(db, sql, prefetch);
}

void TDataBaseSheetReport::CrtBySqlFiles(NS_Oracle::TDBConnect& db, int prefetch) noexcept(false)
{
	using std::vector;
	vector<string> sql_lst = tune.getDQLFileLst();
	if (sql_lst.size() < 1)
		throw TLog("Пустой списк sql-запросов в директории!", "TDataBaseSheetReport::CrtBySqlFiles");
	bool use_parse = useSqlParse();
	//выполнение каждого запроса из списка:
	for (const string& sql_file : sql_lst)
	{
		string sql = getSqlByTune(use_parse, false, sql_file);
		//выполнение DQL-запроса и заполнение данными листа
		FillSheetBySql(db, sql, prefetch);
	}
}

size_t TDataBaseSheetReport::executeDML(NS_Oracle::TDBConnect& db, const NS_Tune::TUserTune& param,
	const string& dml, bool use_comit) noexcept(false)
{
	//выполняем данную команду:
	//создание dml-команды:
	TStatement st(db, dml);
	//устанавливаем параметры запроса:
	setSqlParamsByTune(st, param);
	//выполнение команды:
	size_t result = 0;
	result = st.executeDML();
	if (result > 0 and use_comit)
	{
		TLog log("Запрос ", "executeDML");
		log << dml << "\nОбработал " << result << " строк!\n";
		log.toErrBuff();
		st.Commit();
	}
	return result;
}

size_t TDataBaseSheetReport::runDML4Directory(NS_Oracle::TDBConnect& db, const NS_Tune::TUserTune& param,
	bool use_comit) noexcept(false)
{
	using std::vector;
	//получение списка файлов:
	vector<string> dml_lst = param.getDMLFileLst();
	if (dml_lst.size() < 1)
		throw TLog("Пустой списк dml-команд в директории!", "TDataBaseSheetReport::runDML4Directory");
	//счетчик общего числа обработанных строк:
	size_t cnt = 0;
	//выполнение каждого запроса из списка:
	for (const string& dml : dml_lst)
	{
		size_t tmp = 0;
		//получение текста dml-команды
		string dml_txt = getSqlByTune(false, false, dml);
		//выполнение dml-команды
		tmp = executeDML(db, param, dml_txt, use_comit);
		if (tmp == 0)
			TLog("Запрос " + dml + " не вернул результата!", "TDataBaseSheetReport::runDML4Directory").toErrBuff();
		else
			cnt += tmp;
	}
	return cnt;
}

bool  TDataBaseSheetReport::crtSheet() noexcept(true)
{
	using std::vector;
	try
	{
		if (tune.Empty())
			throw TLog("Ошибка формирования отчета - пустой файл настроек!", "TDataBaseSheetReport::crtReport");
		if (!sheet.isValid())
			throw TLog("Ошибка формирования отчета - не создана страница для отчета!", "TDataBaseSheetReport::crtReport");
		if (tune.isDQLEmpty())
			throw TLog("Не указано ни одного SQL-запроса!", "TDataBaseSheetReport::crtReport");
		//получение настроек для подключения к БД:
		TConnectParam cp = getConnectParam(tune, prefetch_rows);
		//подключаемся к БД:
		TDBConnect db(cp);
		bool dql_first = isDQLFirst();
		//если первым надо выполнить DML-команды
		if (!dql_first) runDML(db, tune);
		//проверяем считывается ли sql-команда из файла
		string sql = tune.getFieldValueByCode(TuneField::SqlText);
		if (!sql.empty())
			CrtBySqlLine(db, sql, cp.prefetch_rows);
		else
			CrtBySqlFiles(db, cp.prefetch_rows);
		if (dql_first) runDML(db, tune);
		return true;
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
	}
	catch (const NS_Oracle::SQLException& er)
	{
		TLog log(string(er.what()), "TDataBaseSheetReport::crtReport");
		log.toErrBuff();
	}
	catch (...)
	{
		TLog("Отчет не сформирован! Не обработанная ошибка!", "TDataBaseSheetReport::crtReport").toErrBuff();
	}
	return false;
}

//выполнение DML запроса:
size_t TDataBaseSheetReport::runDML(NS_Oracle::TDBConnect& db, const NS_Tune::TUserTune& param, bool use_comit) noexcept(true)
{
	try
	{
		//проверка соединения и файла настроек
		if (!db.isValid()) throw TLog("Подключение к БД не валидно!", "TDataBaseSheetReport::runDML");
		if (param.Empty()) throw TLog("Пустой файл настроек!", "TDataBaseSheetReport::runDML");
		if (param.isDMLEmpty()) throw TLog("Нет DML-команд для выполнения!", "TDataBaseSheetReport::runDML");
		//получение dml-текста команды:
		string dml = param.getFieldValueByCode(TuneField::DMLText);
		//проверяем имеется ли текст команды в строке:
		size_t result = 0;
		if (!dml.empty())
			//выполнение dml-команды из строки:
			result = executeDML(db, param, dml, use_comit);
		else
			result = runDML4Directory(db, param, use_comit);
		return result;
	}
	catch (const NS_Oracle::SQLException& er)
	{
		TLog(er.what(), "TDataBaseSheetReport::runDML").toErrBuff();
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка при выполнении DML-команды!", "TDataBaseSheetReport::runDML");
	}
	return 0;
}

void TReport::saveReport(TExcelBook& book, const string& file_name) const noexcept(false)
{
	//проверка существования директории:
	string path = config.getOutPath();
	if (TSharedTune::CheckPath(path, true))
		//сохранение книги:
		book.SaveToFile(file_name);
	else
		throw TLog("Указанная директория: " + path + " не найдена!\nОтчет не сохранен!",
			"TReport::One_Report_For_Each_Config");
}

void TReport::One_Report_For_Each_Config() const noexcept(false)
{
	using NS_Const::TuneField;
	using std::vector;
	using NS_Logger::TLog;
	using NS_Excel::TExcelBook;
	using StrArr = vector<string>;
	//получение списка конфигурационных файлов
	StrArr conf_lst = config.getConfFileLst(true);
	if (conf_lst.empty())
		throw TLog("Пустой список конфигурационных файлов!", "TReport::One_Report_For_Each_Config");
	//получение имени выходного файла:
	string tmp = config.getOutFile();
	//для каждого файла выполняем формирование отчета для листа
	for (const string& sub_tune : conf_lst)
	{
		//формирование файла настроек:
		TUserTune tune(config, sub_tune);
		if (!tune.Empty())
		{
			//создание excel-документа
			TExcelBook book(tune.getOutFileBySheet());
			//формируем отчет для config-файла:
			TDataBaseSheetReport sh(book, tune);
			//если отчет не сформирован:
			if (!sh.crtSheet())
			{
				TLog log("Отчет для файла " + sub_tune + " не сформирован!", "TReport::One_Report_For_Each_Config");
				log.toErrBuff();
			}
			//иначе сохраняем файл:
			else
				saveReport(book);
		}
	}
}

void TReport::One_Sheet_By_One_Config() const noexcept(false)
{
	using NS_Const::TuneField;
	using std::vector;
	using NS_Logger::TLog;
	using NS_Excel::TExcelBook;
	using StrArr = vector<string>;
	//получение списка конфигурационных файлов
	StrArr conf_lst = config.getConfFileLst(true);
	if (conf_lst.empty())
		throw TLog("Пустой список конфигурационных файлов!", "TReport::One_Sheet_By_One_Config");
	//получение пути выходного файла:
	string tmp = config.getOutFile();
	//создание excel-документа
	TExcelBook book(tmp);
	size_t fail_cnt = 0;
	//для каждого файла выполняем формирование отчета для листа
	for (const string& sub_tune : conf_lst)
	{
		//формирование настроек отчета:
		TUserTune tune(config, sub_tune);
		//формируем отчет для config-файла:
		TDataBaseSheetReport sh(book, tune);
		//если отчет не сформирован:
		if (!sh.crtSheet())
		{
			TLog log("Отчет для файла " + sub_tune + " не сформирован!", "TReport::One_Sheet_By_One_Config");
			log.toErrBuff();
			fail_cnt++;
		}
	}
	if (fail_cnt >= conf_lst.size())
		throw TLog("При формировании отчета возникли ошибки!", "TReport::One_Sheet_By_One_Config");
	else//сохранение файла, если есть что сохранять
		saveReport(book);
}

//здесь надо учесть, что для одного файла настроек может быть несколько запросов
//также необходимо передавать страницу в функцию по заполнению данных
void TReport::One_Sheet_By_Many_Statement() const noexcept(false)
{
	using NS_Const::TuneField;
	using std::vector;
	using NS_Logger::TLog;
	using NS_Excel::TExcelBook;
	using StrArr = vector<string>;
	//получение списка конфигурационных файлов
	StrArr conf_lst = config.getConfFileLst(true);
	if (conf_lst.empty())
		throw TLog("Пустой список конфигурационных файлов!", "TReport::One_Sheet_By_Many_Statement");
	//получение пути выходного файла:
	string tmp = config.getOutFile();
	//создание excel-документа
	TExcelBook book(tmp);
	//инициализируем страницу, которая будет создана:
	TExcelBookSheet sheet(nullptr);
	//число ошибок при формировании отчета:
	size_t fail_cnt = 0;
	for (const string& sub_tune: conf_lst)
	{
		//создание файла настроек:
		TUserTune tune(config, sub_tune);
		//формируем отчет для config-файла:
		TDataBaseSheetReport sh(book, sheet, tune);
		//если отчет не сформирован:
		if (!sh.crtSheet())
		{
			TLog log("Отчет для файла " + sub_tune + " не сформирован!", "TReport::One_Sheet_By_Many_Statement");
			log.toErrBuff();
			fail_cnt++;
		}
		if (!sheet.isValid())	sheet = book.getActiveSheet();
	}
	if (fail_cnt >= conf_lst.size())
		throw TLog("При формировании отчета возникли ошибки!", "TReport::One_Sheet_By_Many_Statement");
	else//сохранение файла, если есть что сохранять
		saveReport(book);
}

size_t TReport::runDML_By_Tune(bool use_comit) const noexcept(false)
{
	using std::vector;
	using NS_Logger::TLog;
	using StrArr = vector<string>;
	//получение списка конфигурационных файлов
	StrArr conf_lst = config.getConfFileLst(true);
	if (conf_lst.empty())
		throw TLog("Пустой список конфигурационных файлов!", "TReport::One_Sheet_By_Many_Statement");
	//число обработанных строк dml-командой
	size_t result = 0;
	for (const string& sub_tune : conf_lst)
	{
		//получение параметров из файла настроек
		TUserTune tune(config, sub_tune);
		//получение параметров подключения:
		TConnectParam cp = TDataBaseSheetReport::getConnectParam(tune);
		//для данных настроек формируем подлюечение к БД:
		TDBConnect db(cp);
		//выполняем dml-команду:
		size_t cnt = 0;
		cnt = TDataBaseSheetReport::runDML(db, tune, use_comit);
		if (cnt > 0) result += cnt;
	}
	return result;
}

void TReport::Create_Report_By_Code(const NS_Const::ReportCode& code) const
{
	using NS_Const::ReportCode;
	using NS_Const::TConstReportCode;
	using NS_Logger::TLog;
	//формирование лога ошибки о не реализованном отчете:
	auto raise_err = [](const ReportCode& rp)->TLog 
	{
		TLog log("Отчет ", "TReport::Create_Report_By_Code");
		log << TConstReportCode(rp).getName() << " не реализован!" << TLog::NL;
		return log;
	};
	switch (code)
	{
	//отчеты основанные на выполнении одиночного sql-запроса на лист:
	case ReportCode::RIB_DOCS_FOR_PERIOD:
	case ReportCode::CRED_CASE_MF:
	case ReportCode::POTREB_CRED_BY_FILE:
	case ReportCode::BALANCE_LIST:
	case ReportCode::DOCS_MF_SF_FOR_PERIOD:
		One_Report_For_Each_Config();
		break;
	//case ReportCode::DOCS_MF_SF_FOR_PERIOD:
	case ReportCode::REPAYMENT_FOR_DATE:
		One_Sheet_By_One_Config();
		break;
		//отчет основан на записи результатов различных запросов на одну страницу в один файл:
	case ReportCode::NBKI_NP:
	case ReportCode::NBKI_JP:
		One_Sheet_By_Many_Statement();
		break;
		//выполнение хранимой процедуры:
	case ReportCode::NBKI_APPLY:
	case ReportCode::CLOSE_DAY:
		runDML_By_Tune(true);
		break;
	//полный кредитный портфель + манипуляция с excel-файлом
	case ReportCode::FULL_CRED_REPORT:
		throw raise_err(code);
		break;
	//загрузка данных в oracle из excel
	case ReportCode::LOAD_FROM_FILE:
		throw raise_err(code);
		break;
	//сравнение файлов excel
	case ReportCode::FILE_COMPARE:
		throw raise_err(code);
		break;
	default:
		throw raise_err(code);
	}
}

bool TReport::Execute() const noexcept(true)
{
	using NS_Logger::TLog;
	using NS_Const::TConstReportCode;
	using NS_Const::ReportCode;
	try
	{
		//получение идентификатора кода отчета:
		ReportCode code = TConstReportCode::getIDByCode(config.getMainCode(false), ReportCode::Empty, ReportCode::Last);
		Create_Report_By_Code(code);
		return true;
	}
	catch (const TLog& er)
	{
		er.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка при формировании отчета!", "TReport::Execute").toErrBuff();
	}
	return false;
}