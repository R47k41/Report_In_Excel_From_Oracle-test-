#ifndef TDBREPORT_H_
#define TDBREPORT_H_
//модуль для формирования отчета из базы данных
//отчет формируем в excel-документ
//база данных - oracle
#include <string>
#include <vector>
#include "TExcel.h"
#include "TuneParam.h"
#include "TConstants.h"
#include "TOracle.h"


namespace NS_ExcelReport
{
	using std::string;
	using std::vector;
	using NS_Tune::TSimpleTune;
	using NS_Tune::TUserTune;
	using NS_Tune::TSharedTune;
	using NS_Excel::TExcelBook;
	using NS_Excel::TExcelBookSheet;
	using NS_Oracle::TStatement;
	using NS_Oracle::TResultSet;
	using NS_Oracle::TConnectParam;
	using NS_Oracle::TDBConnect;
	
	//число строк извлекаемое за одно обращение к БД
	const int PrefetchRowsCnt = 200;

	//класс функционала для формирования отчета для страницы:
	//инициализация параметров настроек и параметров листа
	class TSheetReport
	{
	protected:
		TExcelBook& book;//ссылка на книгу
		TExcelBookSheet sheet;//лист отчета
		const TUserTune& tune;//настройки для листа отчета
		//функция инициализации страницы по настройкам:
		virtual bool SetSheetByTune(const string& name = "") noexcept(true);
		//функция добавления страницы для отчета, если данные превысили ограничения:
		virtual bool CreateNewPage(size_t val, bool byRows = true);
	public:
		//создание страницы отчета по книге и ссылке на файл настройки
		TSheetReport(TExcelBook& book_link, const TUserTune& config);
		//инициализация объектами: 
		TSheetReport(TExcelBook& ref_book, const TExcelBookSheet& ref_sheet, const TUserTune& ref_tune);
		//деструктор:
		virtual ~TSheetReport() {}
		//функция заполнения страницы отчета
		virtual bool crtSheet() noexcept(true) = 0;
		//установка значения поля по типу данных из индекса ячейки
		//virtual void setDataByType(int indx) noexcept(false) = 0;
	};

	//класс получения отчета для страницы из БД
	class TDataBaseSheetReport: public TSheetReport
	{
	private:
		//константы с параметрами запросов:
		enum class SqlType { DQL, DML };
		int prefetch_rows;//число извлекаемых строк за одно обращение к БД
		//установка значения параметра sql-команды из настроек:
		static void setSqlParamByTune(TStatement& sql, const NS_Tune::TSubParam& param, bool use_raise = false) noexcept(false);
		//функция преобразования типов данных из TOracle(TType) в тип данных TConstants(DataType)
		static NS_Const::DataType convertOraType(const NS_Oracle::TType& type) noexcept(true);
		//установка значений параметров sql-команды по настройкам:
		static void setSqlParamsByTune(TStatement& sql, const TUserTune& param) noexcept(false);
		//установка значения поля по типу данных из индекса ячейки sql-данных
		virtual void setCellByResultSet(const TResultSet& rs, const NS_Excel::TExcelCell& cell) noexcept(false);
		virtual bool WriteFromResultSet(TResultSet& rs) noexcept(true);
		//функция заполнения данных в листе из текста sql-запроса:
		void FillSheetBySql(TDBConnect& db, const string& sql_txt, int prefetch);
		//признак использования парсинга:
		bool useSqlParse() const noexcept(false);
		//признак выполнения DQL запроса перед DML запросом
		//при условии, что они оба указаны в одном config-файле
		bool isDQLFirst() const noexcept(false);
		//получение текста sql-запроса с парсингом или без:
		static string getSqlByTune(bool use_parse, bool by_str, const string& str) noexcept(true);
	protected:
		//получение текста запроса из строки либо на основании настроек, если строка пуста
		template <typename T>
		static string getSqlText(bool by_str, const string& str) noexcept(false);
		//формирование страницы отчета по строfке запроса:
		void CrtBySqlLine(TDBConnect& db, const string& sql_line, int prefetch = 1) noexcept(false);
		//формирование страницы отчета по файлам sql-запросов:
		void CrtBySqlFiles(TDBConnect& db, int prefetch = 1) noexcept(false);
	public:
		//инициализация книгой и файлами настроек:
		TDataBaseSheetReport(TExcelBook& pbook, const TUserTune& config, int prefetch = PrefetchRowsCnt) : 
			TSheetReport(pbook, config), prefetch_rows(prefetch) {}
		//инициализация книгой, страницей и файлом настроек:
		TDataBaseSheetReport(TExcelBook& ref_book, const TExcelBookSheet& ref_sheet, const TUserTune& ref_tune,
			int prefetch = PrefetchRowsCnt) : TSheetReport(ref_book, ref_sheet, ref_tune), prefetch_rows(prefetch) {}
		//формирование параметров подключения:
		static TConnectParam getConnectParam(const TUserTune& param, int prefetch = 1) noexcept(false);
		//формирование страницы отчета:
		virtual bool crtSheet() noexcept(true);
		//вызов выполнения указанной dml-команды:
		static size_t executeDML(TDBConnect& db, const TUserTune& param, const string& dml, bool use_comit = true) noexcept(false);
		//вызов dml-команды для файлов:
		static size_t runDML4Directory(TDBConnect& db, const TUserTune& param, bool use_comit = true) noexcept(false);
		//выполнение DML запроса(можно выполнять без привязки к отчету):
		static size_t runDML(TDBConnect& db, const TUserTune& param, bool use_comit = true) noexcept(true);
	};

	class TReport
	{
	private:
		//основные настройки для формирования отчета
		TSharedTune config;
		//функция сохранения отчета:
		void saveReport(TExcelBook& book, const string& file_name = "") const noexcept(false);
	public:
		//инициализация
		TReport(const string& conf_file, const string& code) : config(conf_file, code) {}
		//деструктор
		virtual ~TReport() {}
		//функция формирования файла-отчета:
		//под каждый запрос формируется собственный excel-файл
		void One_Report_For_Each_Config() const noexcept(false);
		//функция формирования отчета:
		//для каждого файла запроса создается собственная страница
		//сохранение в одном excel-файле
		void One_Sheet_By_One_Config() const noexcept(false);
		//функция формирования отчета:
		//все запросы записываются на общую страницу
		//страница сохраняется в excel-файл
		void One_Sheet_By_Many_Statement() const noexcept(false);
		//выполнение dml-команд(без участия dql-запросов)
		size_t runDML_By_Tune(bool use_comit = true) const noexcept(false);
		//вызов функции, формирующей отчет по коду:
		void Create_Report_By_Code(const NS_Const::ReportCode& code) const;
		//формирование полного отчета по файлу main_config:
		bool Execute() const noexcept(true);
	};
}
#endif