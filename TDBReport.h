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
	using NS_Excel::TExcelBookFormat;
	using NS_Excel::TExcelBookFont;
	using NS_Oracle::TStatement;
	using NS_Oracle::TResultSet;
	using NS_Oracle::TConnectParam;
	using NS_Oracle::TDBConnect;
	using NS_Tune::TExcelProcData;
	using NS_Tune::TShareData;
	using NS_Tune::TProcCell;
	using NS_Tune::TCellData;
	using NS_Tune::TFilterData;
	using NS_Tune::TCellMethod;
	using DBUserTuneArr = vector<TUserTune>;
	using NS_Tune::CellDataArr;
	using NS_Tune::FilterArr;
	using TRowFlag = std::pair<size_t, bool>;
	using TRowsFlag = std::vector<TRowFlag>;
	using TRowsTNS = std::pair<std::string, TRowsFlag>;
	using TFillFormat = std::pair<size_t, size_t>;
	using TFillFrmts = std::vector<TFillFormat>;


	//число строк извлекаемое за одно обращение к БД
	const int PrefetchRowsCnt = 200;

	//класс функционала для формирования отчета для страницы:
	class TBaseSheetReport
	{
	protected:
		TExcelBook& book;//ссылка на excel-файл
		TExcelBookSheet sheet;//лист excel-книги(НУМЕРАЦИЯ ЛИСТОВ ИДЕТ ОТ 0!!!)
		//проверка на необходимость создания новой страницы:
		virtual bool NeedNewPage(size_t item_cnt, bool byRows = true) const noexcept(false);
		//функция получения первой/последней строки страницы:
		virtual size_t getRow(bool first) const noexcept(false) = 0;
		//функция открытия указанной книги на указанной странице:
		virtual bool OpenBookSheet(const string& srcName, size_t page) noexcept(true);
		//функция установки формата для ячейки
		bool setCellFormat(size_t Row, size_t Column, NS_Excel::TExcelBookFormat& format) noexcept(true);
		//проверка типов данных в ячейках разных листов:
		bool EqualCellsType(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
			const NS_Excel::TExcelCell& srcCell) const noexcept(false);
		//функция проверки соответствия значения в ячйеке:
		bool checkCellStrVal(const NS_Excel::TExcelCell& cell, const string& val) const noexcept(false);
	public:
		//инициализация ссылкой на книгу и ссылкой на страницу данной книги:
		explicit TBaseSheetReport(TExcelBook& book_ref, NS_Excel::SheetPtr sheet_ref = nullptr) :
			book(book_ref), sheet(sheet_ref) {}
		TBaseSheetReport(TExcelBook& book_ref, const NS_Excel::TExcelBookSheet& sheet_ref): 
			book(book_ref), sheet(sheet_ref)	{}
		//инициализация книги при помощи открытия другой книги:
		//указанная страница становится активной для обработки:
		TBaseSheetReport(TExcelBook& book_ref, const string& src_file, size_t page_index);
		virtual ~TBaseSheetReport() {}
		//функция получения начальной строки отчета:
		virtual size_t FirstRow() const noexcept(false) { return getRow(true); }
		//функция получения последней строки отчета:
		virtual size_t LastRow() const noexcept(false) { return getRow(false); }
		//функция проверки попадания ячейки в диапазон:
		virtual bool inRange(size_t row, size_t col) const noexcept(false);
		//функция признака пустых данных на excel-странице
		bool isEmptySheetData() const noexcept(true) { return sheet.getFirstRow() == sheet.getLastRow(); }
		//функция проверки страницы:
		bool NoSheet() const noexcept(true) { return !sheet.isValid(); }
		//функция получения имени страницы:
		string getSheetName() const noexcept(true) { return sheet.getName(); }
		//конвертация типа данных excel в тип данных из NS_Const::DataType
		static NS_Const::DataType convertExcelType(const NS_Excel::TDataType& dt, bool isDate = false) noexcept(true);
		static NS_Excel::TDataType convertDataType(const NS_Const::DataType& dt) noexcept(true);
	};

	//расширенный класс для взаимодействия с excel-файлами отчетов
	class TExtendSheetReport: public TBaseSheetReport
	{
	private:
		size_t rowB;//начальная строка отсчета
		size_t rowE;//конечная строка отсчета
		size_t colID;//колонка идентификатор - определяет колонку в которой точно есть данные
		FilterArr filters;//данные о фильтрации строк
		//инициализация файла приемника из настроек:
		void InitDstFile(const TShareData& dstFile, size_t page) noexcept(false);
	protected:
		//функция получения первой/последней строки страницы:
		virtual size_t getRow(bool first) const noexcept(false) { return first ? rowB : rowE; }
		//функция сверки данных в указанной ячейке c ячейкой другого листа:
		bool Compare_Cells(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
			const NS_Excel::TExcelCell& srcCell) const noexcept(true);
	public:
		//инициализация json-файлом настроек:
		TExtendSheetReport(TExcelBook& book_ref, const TShareData& file, size_t page);
		//деинициализация
		virtual ~TExtendSheetReport() { TBaseSheetReport::~TBaseSheetReport(); }
		//функция проверки наличия Col_ID:
		virtual bool noColID() const noexcept(true) { return colID == NS_Tune::TIndex::EmptyIndex; }
		//функция проверки данных из ColID на пустоту:
		virtual bool noDataInColID(size_t Row) const noexcept(false);
		//индекс колонки идентификатора:
		virtual size_t getColID() const noexcept(true) { return colID; }
		//функция получения первой строки обработки:
		virtual size_t FirstRow() const noexcept(true);
		//функция получения последней строки в обработке:
		virtual size_t LastRow() const noexcept(true);
		//проверка выполнения условия фильтрации для строки:
		virtual bool isCorrectFilter(size_t curRow) const noexcept(true);
		//формирование массива отфильтрованных строк для указанных услови фильтрации:
		TRowsFlag setFiltredRowsArr() const noexcept(true);
		//функция поиска данных из ячейки другой страницы - возвращает номер строки совпадения:
		size_t CheckOnSheet(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell, size_t srcCol,
			TRowsFlag* RowsArr) const noexcept(true);
		//функция вставки данных в ячейку-приемник из ячейки-источника:
		virtual bool setDstCellBySrcCell(NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell, 
			const NS_Excel::TExcelCell& srcCell) const noexcept(true);
	};

	//класс для обработки excel-файлов на основании json-настроек
	class TJsonReport : public TExtendSheetReport
	{
	private:
		string main_path;//директория файлов для формирования отчета
		TFillFrmts frmt_arr;//массив форматов закраски ячеек
		const TProcCell& cells_data;//информация по ячейкам
		//функция установки параметра запроса для ячейки:
		void setDQLParamByCell(TStatement& query, const NS_Excel::TExcelCell& cell,
			const TCellData& value) const noexcept(false);
		//функция извлечения данных из excel-файла источника для строки excel-файла приемника:

	protected:
		//функция установки параметров формата:
		virtual size_t crtFillFormat(NS_Excel::TExcelBookFormat& init_format, bool find_flg, bool font_flg) noexcept(false);
		//функция формирования формата для ячейки
		virtual void crtCellFillFormat(size_t Row, size_t Col, bool font_flg) noexcept(false);
		//функция формирования формата для строки:
		virtual void crtRowFillFormat(size_t Row, const NS_Const::JsonCellFill& fill_code) noexcept(true);
		//функция формирования форматов для закраски ячеек:
		virtual void crtCellFillFormatArr() noexcept(true);
		//обработка ячейки строки
		virtual bool procRowCell(size_t Row, size_t Col, size_t index, bool find_flg) noexcept(true);
		virtual bool procRowCell(const NS_Excel::TExcelCell& cell, size_t index, bool find_flg) noexcept(true);
		//функция получения данных на основании excel-строки и запись результата в excel-строку файла
		virtual void writeExcelFromDB(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false);
		//функция проверки существования данных в БД(для одной записи):
		virtual bool checkINDataBase(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false);
		//функция записи данных в БД:
		virtual void insertToDataBase(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false);
		//функция выполнения обработки в зависимости от метода:
		bool ProcessByDataBase(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false);
		//функция выполнения запроса для строки:
		virtual bool runQuery(NS_Oracle::TStatement& query, size_t curRow) noexcept(true);
		//функция обработки строки из страницы excel-файла:
		virtual bool setExcelRowDataByBD(NS_Oracle::TStatement& query, size_t curRow) noexcept(true);
		//функция получения данных для вставки/обновления в странице excel-файла для всех строк:
		virtual void setExcelDataByDB(TStatement& query, size_t& rowFrom) noexcept(false);
		//функция считывания информации из excel-файла
		virtual void UpdExcelDataByDB(TDBConnect& db, const TUserTune& tune, size_t& rowFrom) noexcept(false);
		//фукнция проверки выполнения условий фильтрации для указанной строки:
		virtual bool CorrectFilter(size_t cur_row) const noexcept(true);
		//функция установки параметров в sql-запрос для указанной строки:
		bool setSelectParams(TStatement& query, size_t curRow) const noexcept(true);
		//функция заполнения данных для страницы из базы данных:
		void ProcessSheetDataWithDB() noexcept(false);
		//функция получения наименования сервера из конфигурационных файлов:
		std::string getServerNameByTuneIndex(size_t val) const noexcept(true);
		//функция поиска данных из приемника в источнике:
		bool Search_DstRow_In_SrcSheet(const TExtendSheetReport& srcSheet, const CellDataArr& cols, size_t curRow) noexcept(true);
		//функция проверки наличия строк файла-приемника на листе в файле-источнике:
		bool Search_Dest_Data_In_Src_Sheet(TRowsFlag& DstRows, const TExtendSheetReport& srcSheet) noexcept(true);
		//функция вставки данных из excel-файла в базу данных:

		//функция вставки данных из другого excel-файла:
		void Compare_Excel_Sheets() noexcept(false);
		//функция сравнения Excel-файлов

		//функция получения числа конфигурационных файлов:
		size_t getTuneFilesCnt() const noexcept(true) { return cells_data.getDBTuneArr().size(); }
		//функция получения кода метода обработки:
		NS_Const::JSonMeth getMethCode() const noexcept(true) { return cells_data.getMethodCode(); }
	public:
		//инициализация при помощи объектов из json-файла
		TJsonReport(TExcelBook& book_ref, const TExcelProcData& json_tune, size_t page);
		//деинициализация:
		virtual ~TJsonReport() {}
		//функция формирования отчета
		virtual bool crtSheetReport() noexcept(true);
		//функция признака предобработки:
		bool preProcFlg() const noexcept(true) { return cells_data.getMethodCode() == NS_Const::JSonMeth::GetRowIDByDB; }

	};

	//класс формирования строк excel-отчета на основании js-файла настроек:
	class TJsonMarkReport: public TJsonReport
	{
	private:
		TRowsTNS& procRows;//массив строк для обработки
		bool flg;//признак фильтрации из строк обработки
	protected:
		//фукнция проверки выполнения условий фильтрации для указанной строки:
		virtual bool CorrectFilter(size_t cur_row) const noexcept(true);
		//функция проверки существования данных в БД:
		bool checkINDataBase(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false);
		//функция получения данных для вставки/обновления в странице excel-файла для всех строк:
		virtual void setExcelDataByDB(TStatement& query, size_t& rowFrom) noexcept(false);
		//функция считывания информации из excel-файла
		virtual void UpdExcelDataByDB(TDBConnect& db, const TUserTune& tune, size_t& rowFrom) noexcept(false);
	public:
		//инициализация объекта
		TJsonMarkReport(TExcelBook& book_ref, const TExcelProcData& json_tune, size_t page, TRowsTNS& Rows);
		//деинициализация:
		virtual ~TJsonMarkReport() {}
		//наличие доп. условий отбора строк:
		bool NoFltrRows() const noexcept(true) { return procRows.second.empty(); }
		//возвращаем массив обработанных строк для страницы:
		TRowsTNS& getProcRows() const noexcept(true) { return procRows; }
		//функция получения наименование сервера участвующего при выборке данных:
		std::string getFlgName() const noexcept(true);
		//проверка выполнеят ли данный обект фильтрацию данных:
		bool isFilterObject() const noexcept(true);
	};

	//класс для формирования отчетов на основании файлов настроек
	//считывание инфы из БД и
	class TSheetTuneReport: public TBaseSheetReport
	{
	protected:
		const TUserTune& tune;//настройки для листа отчета
		//функция получения первой/последней строки страницы:
		virtual size_t getRow(bool first) const noexcept(false) { return TBaseSheetReport::getRow(first); }
		//функция инициализации страницы по настройкам:
		virtual bool SetSheetByTune(const string& name = "") noexcept(true);
		//функция добавления страницы для отчета, если данные превысили ограничения:
		virtual bool CreateNewPage(size_t val, bool byRows = true) noexcept(false);
	public:
		//создание страницы отчета по книге и ссылке на файл настройки
		TSheetTuneReport(TExcelBook& book_link, const TUserTune& config);
		//инициализация объектами: 
		TSheetTuneReport(TExcelBook& ref_book, const TExcelBookSheet& ref_sheet, const TUserTune& ref_tune);
		//деструктор:
		virtual ~TSheetTuneReport() {}
		//функция заполнения страницы отчета
		virtual bool crtSheet() noexcept(true) = 0;
	};

	//класс для взаимодействия с БД:
	struct TDataBaseInterface
	{
		static const  NS_Const::DataType DataTypeError = NS_Const::DataType::ErrorType;
		static const NS_Oracle::TType OraTypeError = NS_Oracle::TType::OCCIANYDATA;
		//формирование параметров подключения:
		static TConnectParam getConnectParam(const TUserTune& param, int prefetch = 1) noexcept(false);
		//установка значения параметра sql-команды из настроек:
		static void setSqlParamByTune(TStatement& sql, const NS_Tune::TSubParam& param, bool use_raise = false) noexcept(false);
		//функция преобразования типов данных из TOracle(TType) в тип данных TConstants(DataType)
		static NS_Const::DataType convertOraType(const NS_Oracle::TType& type) noexcept(true);
		//функция преобразования из типов данных excel в типы данных oracle:
		static NS_Oracle::TType convertDataType(const NS_Const::DataType& type) noexcept(true);
		//установка значений параметров sql-команды по настройкам:
		static void setSqlParamsByTune(TStatement& sql, const TUserTune& param) noexcept(false);
		//получение текста sql-запроса с парсингом или без:
		static string getSqlByTune(bool use_parse, bool by_str, const string& str) noexcept(true);
		//получение текста sql-запроса из настроек:
		static string getSqlByTune(const TUserTune& tune) noexcept(true);
		//получение текста запроса из строки либо на основании настроек, если строка пуста
		template <typename T>
		static string getSqlText(bool by_str, const string& str) noexcept(false);
		//вызов выполнения указанной dml-команды:
		static size_t executeDML(TDBConnect& db, const TUserTune& param, const string& dml, bool use_comit = true) noexcept(false);
		//вызов dml-команды для файлов:
		static size_t runDML4Directory(TDBConnect& db, const TUserTune& param, bool use_comit = true) noexcept(false);
		//выполнение DML запроса(можно выполнять без привязки к отчету):
		static size_t runDML(TDBConnect& db, const TUserTune& param, bool use_comit = true) noexcept(true);
		//установка значения поля по типу данных из индекса ячейки sql-данных
		static void setCellByResultSet(TExcelBook& book, TExcelBookSheet& sheet, 
			const TResultSet& rs, size_t resultSetCol, const NS_Excel::TExcelCell& cell) noexcept(false);
		//установка числа итераций для запроса:
		static bool setMaxIterationCnt(TStatement& query, size_t cnt) noexcept(true);
		//добавление итерации для запроса:
		static bool addQueryIteration(TStatement& query) noexcept(true);
	};

	//класс получения отчета для страницы из БД
	class TDataBaseSheetReport: public TSheetTuneReport, public TDataBaseInterface
	{
	private:
		//константы с параметрами запросов:
		enum class SqlType { DQL, DML };
		int prefetch_rows;//число извлекаемых строк за одно обращение к БД
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
	protected:
		//формирование страницы отчета по строfке запроса:
		void CrtBySqlLine(TDBConnect& db, const string& sql_line, int prefetch = 1) noexcept(false);
		//формирование страницы отчета по файлам sql-запросов:
		void CrtBySqlFiles(TDBConnect& db, int prefetch = 1) noexcept(false);
	public:
		//инициализация книгой и файлами настроек:
		TDataBaseSheetReport(TExcelBook& pbook, const TUserTune& config, int prefetch = PrefetchRowsCnt) : 
			TSheetTuneReport(pbook, config), prefetch_rows(prefetch) {}
		//инициализация книгой, страницей и файлом настроек:
		TDataBaseSheetReport(TExcelBook& ref_book, const TExcelBookSheet& ref_sheet, const TUserTune& ref_tune,
			int prefetch = PrefetchRowsCnt) : TSheetTuneReport(ref_book, ref_sheet, ref_tune), prefetch_rows(prefetch) {}
		//формирование страницы отчета:
		virtual bool crtSheet() noexcept(true);
	};

	class TReport
	{
	private:
		//основные настройки для формирования отчета
		TSharedTune config;
		//функция сохранения отчета:
		void saveReport(NS_Excel::TExcelBook& book, const string& file_name = "") const noexcept(false);
		//функция обработки одного excel-файла на основании js-файла настроек
		bool ProcessExcelFileByJson(TExcelBook& book, const string& js_file, 
			std::vector<NS_ExcelReport::TRowsTNS>& Rows) const noexcept(true);
		//функция сравнения одного excel-файла:
		bool Json_Report_By_File_Compare(const string& js_file) const noexcept(true);
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
		//методы формирования отчетов на основании json-файлов:
		//формирование отчета на основании данных из БД: 
		//для  каждой строки собственное обращение к БД
		void Json_One_Row_One_DBQuery() const noexcept(false);
		//формирование отчета на основании сравнения 2х excel-файлов:
		void Json_Report_By_Files_Compare() const noexcept(true);
		//вызов функции, формирующей отчет по коду:
		void Create_Report_By_Code(const NS_Const::ReportCode& code) const;
		//формирование полного отчета по файлу main_config:
		bool Execute() const noexcept(true);
	};
}
#endif