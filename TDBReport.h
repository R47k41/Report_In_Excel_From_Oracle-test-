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
	using NS_Tune::TSimpleTune;
	using NS_Tune::TUserTune;
	using NS_Tune::TSharedTune;
	using NS_Excel::TExcelBook;
	using NS_Excel::TExcelBookSheet;
	using NS_Oracle::TStatement;
	using NS_Oracle::TResultSet;
	using NS_Oracle::TConnectParam;
	
	//число строк извлекаемое за одно обращение к БД
	const int PrefetchRowsCnt = 200;

	//класс функционала для формирования отчета для страницы:
	//инициализация параметров настроек и параметров листа
	class TSheetReport
	{
	protected:
		TExcelBook& book;//ссылка на книгу
		TExcelBookSheet sheet;//лист отчета
		TUserTune tune;//настройки для листа отчета
		//функция инициализации страницы по настройкам:
		virtual bool SetSheetByTune(const string& name = "") noexcept(true);
		//функция добавления страницы для отчета, если данные превысили ограничения:
		virtual bool CreateNewPage(size_t val, bool byRows = true);
	public:
		//создание страницы отчета по книге и имени/пути файла конфига
		TSheetReport(TExcelBook& book_link, const string& config);
		//инициализация страницы по книге и настройкам:
		TSheetReport(TExcelBook& book_link, const TSimpleTune& config, const string& sub_conf_file);
		//инициализация книгой с указанной страницей и файлом настроек:
		TSheetReport(TExcelBook& ref_book, TExcelBookSheet& ref_sheet, const TSimpleTune& ref_tune,
			const string& sub_conf_file) :
			book(ref_book), sheet(ref_sheet), tune(ref_tune, sub_conf_file) {}
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
		int prefetch_rows;//число извлекаемых строк за одно обращение к БД
		//формирование параметров подключения:
		TConnectParam getConnectParam() const noexcept(false);
		//установка значения параметра sql-команды из настроек:
		static void setSqlParamByTune(TStatement& sql, const NS_Tune::TSubParam& param, bool use_raise = false) noexcept(false);
		//функция преобразования типов данных из TOracle(TType) в тип данных TConstants(DataType)
		static NS_Const::DataType convertOraType(const NS_Oracle::TType& type) noexcept(true);
		//установка значений параметров sql-команды по настройкам:
		void setSqlParamsByTune(TStatement& sql) const noexcept(false);
		//установка значения поля по типу данных из индекса ячейки sql-данных
		virtual void setCellByResultSet(const TResultSet& rs, const NS_Excel::TExcelCell& cell) noexcept(false);
		virtual bool WriteFromResultSet(TResultSet& rs) noexcept(true);
	protected:
		//получение текста sql-команды
		template <typename T>
		string getSqlText() const noexcept(false);
	public:
		//инициализация книгой и файлами настроек:
		TDataBaseSheetReport(TExcelBook& pbook, const string& config, int prefetch = PrefetchRowsCnt) : 
			TSheetReport(pbook, config), prefetch_rows(prefetch) {}
		TDataBaseSheetReport(TExcelBook& pbook, const TSimpleTune& conf, const string& sub_conf,
			int prefetch = PrefetchRowsCnt) : TSheetReport(pbook, conf, sub_conf), prefetch_rows(prefetch) {}
		//инициализация
		//формирование страницы отчета:
		virtual bool crtSheet() noexcept(true);
	};

	class TReport
	{
	private:
		//основные настройки для формирования отчета
		TSharedTune config;
	public:
		//инициализация
		TReport(const string& conf_file, const string& code) : config(conf_file, code) {}
		//деструктор
		virtual ~TReport() {}
		//функция формирования простого sql-отчета:
		//под каждый запрос формируется собственная страница:
		void Simple_Sql_Report() const;
		//вызов функции, формирующей отчет по коду:
		void Create_Report_By_Code(const NS_Const::ReportCode& code) const;
		//формирование полного отчета по файлу main_config:
		bool Execute() const noexcept(true);
	};
}
#endif