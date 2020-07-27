#ifndef TDBREPORT_H_
#define TDBREPORT_H_
//модуль для формирования отчета из базы данных
//отчет формируем в excel-документ
//база данных - oracle
#include <string>
#include <vector>
#include <map>
#include "TExcel.h"
#include "TuneParam.h"
#include "TConstants.h"
#include "TOracle.h"
#include "TSMLVCH_IMP.h"



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
	using TRowFlag = std::pair<size_t, bool>;//строка и признак ее обработки
	//был vector
	using TRowsFlag = std::map<size_t, bool>;//массив строк с признаком обработки
	using TRowsTNS = std::pair<std::string, TRowsFlag>;
//	using TFillFormat = std::pair<size_t, size_t>;
//	using TFillFrmts = std::vector<TFillFormat>;
//	using TConvertType = std::pair<NS_Oracle::TType, size_t>;

	//структура описывающая взаимодействие индексов форматов листа excel-файла:
	struct TCellFormatIndex
	{
		size_t Current;//индекс текущего формата ячейки в excel-файле
		size_t NotFound;//формат ячейки если данные не найдены
		size_t Found;//формат ячейки если данные найдены
		bool InitFlg;//признак инициализации значений
		//инициализация:
		TCellFormatIndex(): Current(), NotFound(), Found(), InitFlg(false) {}
		explicit TCellFormatIndex(size_t par_Curent);
		TCellFormatIndex(const TCellFormatIndex& val): Current(val.Current), NotFound(val.NotFound), 
			Found(val.Found), InitFlg(true) {}
		TCellFormatIndex(size_t par_Curent, size_t par_NotFound, size_t par_Found);
		~TCellFormatIndex() {}
	};

	using TCellFormat = std::pair<size_t, TCellFormatIndex>;//индекс колонки, индекс ее форматов в книге
	using TCellsFormatArr = std::map<size_t, TCellFormatIndex>;//индекс колонки, индекс ее форматов в книге

	//число строк извлекаемое за одно обращение к БД
	const int PrefetchRowsCnt = 200;

	//класс функционала для формирования отчета для страницы:
	class TBaseSheetReport
	{
	private:
		enum Tune { DEF_TEMPL_SH_INDX = 0 };
	protected:
		TExcelBook& book;//ссылка на excel-файл
		TExcelBookSheet sheet;//лист excel-книги(НУМЕРАЦИЯ ЛИСТОВ ИДЕТ ОТ 0!!!)
		TCellsFormatArr cells_format_indexs;//индексы форматов ячеек(хранит индексы ячеек в формате excel - от 0!!!)
		//функция добавления формата ячейки в массив форматов:
		virtual bool addCurCellFormat(size_t Row, size_t Col) noexcept(true);
		//функция инициализации массива форматов для ячеек строки из шаблона:
		virtual bool initRowFormat() noexcept(true);
		//функция проверки наличия форматов для книги:
		bool EmptyCellsIndexFormat() const noexcept(true) { return cells_format_indexs.empty(); }
		//функция установки формата для ячейки
		bool setCellFormat(const NS_Excel::TExcelCell& cell, NS_Excel::TExcelBookFormat& format) noexcept(true);
		bool setCellFormat(size_t Row, size_t Column, NS_Excel::TExcelBookFormat& format) noexcept(true);
		//функция получения ссылки на форматы ячейки/колонки:
		TCellFormatIndex& getFormatIndexByColl(size_t Column) noexcept(false) { return cells_format_indexs[Column]; }
		//функция получения ссылки на формат для указанной колонки:
		NS_Excel::FormatPtr getCellFormatPtr(const NS_Excel::TExcelCell& cell) noexcept(true);
		//функция уставноки формата для ячейки для элемента массива:
		virtual bool setCellFormatByIndexArr(const NS_Excel::TExcelCell& cell) noexcept(true);
		virtual bool setCellFormatByIndexArr(size_t Row, size_t IndexArr) noexcept(true);
		//функция добавления формата ячейки по индексу из массива с обновлением форматов:
		virtual bool addFillFormat(size_t init_format_index, const NS_Excel::TColor& color, bool font_flg,
			size_t& AddedFormatIndex) noexcept(true);
		//функция выставления форматов ячеек для строки из массива:
		bool setRowCellsFormat(size_t Row) noexcept(true);
		//проверка на необходимость создания новой страницы:
		virtual bool NeedNewPage(size_t item_cnt, bool byRows = true) const noexcept(false);
		//функция получения первой/последней строки страницы:
		virtual size_t getRow(bool first) const noexcept(false) = 0;
		//функция открытия указанной книги на указанной странице:
		virtual bool OpenBookSheet(const string& srcName, size_t page) noexcept(true);
		//проверка типов данных в ячейках разных листов:
		bool EqualCellsType(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
			const NS_Excel::TExcelCell& srcCell) const noexcept(false);
		//функция добавления новой строки в лист отчета
		bool InsNewRow(size_t curRow, size_t newRow) noexcept(true);
		//функция закраски ячейки цветом из настройки:
		bool setCellColorByFormatIndxArr(const NS_Excel::TExcelCell& cell, bool foundFlg) noexcept(true);
		//функция инициализации страницы книги по шаблону:
		virtual bool InitSheetByTemplate(const string& tmpl_name, const string& sh_name,
			bool set_as_active = true, size_t tmpl_sh_index = Tune::DEF_TEMPL_SH_INDX) noexcept(true);
	public:
		//инициализация ссылкой на книгу и ссылкой на страницу данной книги:
		explicit TBaseSheetReport(TExcelBook& book_ref, NS_Excel::SheetPtr sheet_ref = nullptr) :
			book(book_ref), sheet(sheet_ref), cells_format_indexs() {}
		TBaseSheetReport(TExcelBook& book_ref, const NS_Excel::TExcelBookSheet& sheet_ref): 
			book(book_ref), sheet(sheet_ref), cells_format_indexs()	{}
		//инициализация книги при помощи открытия другой книги:
		//указанная страница становится активной для обработки:
		TBaseSheetReport(TExcelBook& book_ref, const string& src_file, size_t page_index);
		virtual ~TBaseSheetReport() { /*if (cells_format_indexs.size() > 0) cells_format_indexs.clear();*/ }
		//функция получения начальной строки отчета:
		virtual size_t FirstRow() const noexcept(false) { return getRow(true); }
		//функция получения последней строки отчета:
		virtual size_t LastRow() const noexcept(false) { return getRow(false); }
		//функция проверки попадания ячейки в диапазон:
		virtual bool inRange(size_t row, size_t col) const noexcept(false);
		//функция признака пустых данных на excel-странице
		bool isEmptySheetData() const noexcept(true) { return sheet.getFirstRow() == sheet.getLastRow(); }
		//признак пустого массива форматов:
		bool EmptyRowFormat() const noexcept(true) { return cells_format_indexs.empty(); }
		//функция проверки страницы:
		bool NoSheet() const noexcept(true) { return !sheet.isValid(); }
		//функция получения имени страницы:
		string getSheetName() const noexcept(true) { return sheet.getName(); }
		//конвертация типа данных excel в тип данных из NS_Const::DataType
		static NS_Const::DataType convertExcelType(const NS_Excel::TDataType& dt, bool isDate = false) noexcept(true);
		static NS_Excel::TDataType convertDataType(const NS_Const::DataType& dt) noexcept(true);
	};

	//расширенный класс для взаимодействия с excel-файлами отчетов
	class TExtendSheetReport : public TBaseSheetReport
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
			const NS_Excel::TExcelCell& srcCell, const NS_Const::JsonFilterOper& operation) const noexcept(true);
		//функция проверки корректности условия фильтра для ячейки:
		bool checkByFilter(const NS_Tune::TFilterData& filter, size_t Row) const noexcept(true);
	public:
		//инициализация json-файлом настроек:
		TExtendSheetReport(TExcelBook& book_ref, const TShareData& file, size_t page);
		//деинициализация
		virtual ~TExtendSheetReport() { }
		//функция проверки наличия Col_ID:
		virtual bool noColID() const noexcept(true) { return colID == NS_Tune::TIndex::EmptyIndex; }
		//функция проверки ячейки на пустоту:
		virtual bool isEmptyCell(const NS_Excel::TExcelCell& cell) const noexcept(false);
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
		//функция поиска данных о ячейке приемника в текущей строке источника
		bool CheckInCell(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
			const NS_Excel::TExcelCell& srcCell, bool NoSpaceNoCase = true) const noexcept(true);
		bool CheckInCell(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell, 
			size_t srcRow, size_t srcCol, bool NoSpaceNoCase = true) const noexcept(true);
		//функция поиска данных в ячейке других страниц - возвращает номер строки совпадения(без массива строк):
		size_t CheckOnSheet(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
			size_t srcCol, bool NoSpaceNoCase = true) const noexcept(true);
		//функция поиска данных из ячейки другой страницы - возвращает номер строки совпадения(с массивом строк):
		size_t CheckOnSheet(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell, 
			size_t srcCol, TRowsFlag& RowsArr, bool NoSpaceNoCase = true) const noexcept(true);
		//функция проверки наличия изменений в ячейках приемника и источника:
		bool NotEquality(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
			const NS_Excel::TExcelCell& srcCell, bool NoCaseNoSpace = false) const noexcept(true);
		//функция поиска указанной ячейки приемника в ячейке строки источника:
		bool getDstCell_In_SrcCell(const TExtendSheetReport& srcSheet, const NS_Excel::TExcelCell& DstCell,
			const NS_Excel::TExcelCell& SrcCell, bool NoSpaceNoCase = true) const noexcept(true);
		//функция получения строки источника для ячейки приемника:
		size_t getSrcRow_By_Cell(const TExtendSheetReport& srcSheet, NS_ExcelReport::TRowsFlag& srcRows,
			const NS_Excel::TExcelCell& DstCell, size_t SrcCol, bool NoSpaceNoCase = true) const noexcept(true);
		//функция поиска строки источника по параметрам ячеек строки приемника:
		size_t getSrcRow_By_Dest_Params(const TExtendSheetReport& srcSheet, NS_ExcelReport::TRowsFlag& srcRows,
			const NS_Tune::CellDataArr& params, size_t curRow, bool NoSpaceNoCase = true) const noexcept(true);
		//функция получения строки источника для ячейки приемника по параметрам:
		size_t getSrcRow_By_Params(const TExtendSheetReport& srcSheet, NS_ExcelReport::TRowsFlag& srcRows,
			const NS_Tune::CellDataArr& params, size_t curRow, size_t& param_index, bool NoSpaceNoCase = true) const noexcept(true);
		//функция вставки данных в ячейку-приемник из ячейки-источника:
		virtual bool setDstCellBySrcCell(NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell, 
			const NS_Excel::TExcelCell& srcCell) const noexcept(true);
	};

	//класс для обработки excel-файлов на основании json-настроек
	class TJsonReport : public TExtendSheetReport
	{
	private:
		string main_path;//директория файлов для формирования отчета
		const TProcCell& cells_data;//информация по ячейкам
		NS_Const::JSonMeth meth_code;//метод обработки данных
		//функция проверки принадлежности ячейки к параметрам:
		bool isParamColumn(size_t Col) const noexcept(true);
		//функция проверки необходимости проверять изменения:
		bool WithChangeMeth() const noexcept(true) { return meth_code == NS_Const::JSonMeth::CompareCellChange; }
		//функция установки параметра запроса для ячейки:
		void setDQLParamByCell(TStatement& query, const TCellData& value, size_t curRow) const noexcept(false);
		//функция проверки наличия изменений в ячейках:
		bool CellHasChanged(const TExtendSheetReport& srcSheet, const NS_Tune::TCellData& param,
			size_t dstRow, size_t srcRow) const noexcept(true);
		//функция вставки данных в ячейку файла приемника 
		bool InsertDstCellBySrcCell(const TExtendSheetReport& srcSheet, const NS_Tune::TCellData& param,
			size_t dstRow, size_t srcRow) noexcept(true);
		//функция инверсии значений параметров источника и приемника:
		static bool Inverse_Dst2Src_Param(const NS_Tune::CellDataArr& params, NS_Tune::CellDataArr& new_param) noexcept(false);
	protected:
		//функция установки выходного параметра для хранимой процедуры/функции oracle:
		static void setDMLOutParam(NS_Oracle::TStatement& query, const TCellData& param) noexcept(false);
		//функция установки параметров для выражения:
		bool setStatementParam(NS_Oracle::TStatement& query, const NS_Tune::TCellData& value, size_t Row) const noexcept(true);
		//функция формирования формата для ячейки
		virtual bool addCellFillFormat(size_t Row, size_t Col, bool font_flg) noexcept(true);
		//функция инициализации формата ячейки:
		virtual bool addCurCellFormat(size_t Row, size_t Col) noexcept(true);
		//функция инициализации массива форматов для ячеек строки из шаблона:
		virtual bool initRowFormat() noexcept(true);
		//проверка можно ли закрашивать ячейку в зависимости от метода:
		bool useColoring(bool FndFlg, bool ChngFlg) const noexcept(true);
		//закраска ячейки строки
		virtual bool ColoringRowCell(size_t Row, size_t Col, bool find_flg, bool proc_flg) noexcept(true);
		//закраска ячейки в зависимости от выполненной обработки найденной ячейки
		virtual bool ColoringRowCell(const NS_Excel::TExcelCell& cell, bool find_flg, bool procFlg) noexcept(true);
		//функция закраски строки:
		virtual bool ColoringRowByFlg(size_t curRow, bool FndFlg, bool ChngFlg) noexcept(true);
		virtual bool ColoringRowByCnt(size_t curRow, size_t FindCnt, size_t FailCnt) noexcept(true);
		//окраска ячейки в зависимости от метода и параметров
		virtual bool ColoringCellByParam(const NS_Tune::TCellData& param, size_t curRow, size_t frmt_index, 
			bool fing_flg) noexcept(true);
		//функция записи выходного параметра в Excel-строку:
		virtual void WriteOutParamToExcel(NS_Oracle::TBaseSet& bs, const NS_Const::DataType& type_code,
			const NS_Tune::TCellData& param, size_t OutParamIndex, size_t Row) noexcept(false);
		//функция получения данных на основании oracle-строки и запись результата в excel-строку файла
		virtual void writeExcelFromDB(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false);
		//функция проверки существования данных в БД(для одной записи):
		virtual bool checkINDataBase(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false);
		//функция записи данных в БД:
		virtual void insertToDataBase(NS_Oracle::TStatement& query, size_t curRow) noexcept(false);
		//функция выполнения обработки в зависимости от метода:
		//получение/обработка данных из ResultSet
		bool ProcessByResultSet(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false);
		//получение данных из базы в выходной параметр:
		void setOutStatementParam(NS_Oracle::TStatement& query, size_t curRow) noexcept(false);
		//обработка данных из Query/Statement
		void ProcessByStatement(NS_Oracle::TStatement& query, size_t curRow) noexcept(false);
		//функция выполнения запроса для строки:
		virtual bool runQuery(NS_Oracle::TStatement& query, size_t curRow) noexcept(true);
		//функция обработки строки из страницы excel-файла:
		virtual bool setExcelRowDataByBD(NS_Oracle::TStatement& query, size_t curRow) noexcept(true);
		//функция получения данных для вставки/обновления в странице excel-файла для всех строк:
		virtual void setExcelDataByDB(NS_Oracle::TStatement& query, size_t& rowFrom) noexcept(false);
		//функция вставки данных из excel-файла в базу данных:
		virtual void SetDBStatementData(NS_Oracle::TDBConnect& db, const NS_Tune::TUserTune& tune, 
			const string& sql, size_t& rowFrom) noexcept(false);
		//процедура обработки массива запросов для БД:
		void runDBStatementLst(TDBConnect& db, const TUserTune& tune, const NS_Tune::StrArr& sqlLst, size_t& rowFrom) noexcept(false);
		//функция считывания информации из excel-файла в базу данных
		virtual void UpdExcelDataByDB(TDBConnect& db, const TUserTune& tune, size_t& rowFrom) noexcept(false);
		//функция очистки таблиц для импорта в БД:
		virtual bool ClearImportDBTbl(TDBConnect& db, const TUserTune& tune) const noexcept(true);
		//фукнция проверки выполнения условий фильтрации для указанной строки:
		virtual bool CorrectFilter(size_t cur_row) const noexcept(true);
		//функция установки параметров в sql-запрос для указанной строки:
		bool setSelectParams(TStatement& query, size_t curRow) const noexcept(true);
		//функция заполнения данных для страницы из базы данных:
		void ProcessSheetDataWithDB() noexcept(false);
		//функция получения наименования сервера из конфигурационных файлов:
		std::string getServerNameByTuneIndex(size_t val) const noexcept(true);
		//функция обработки строки источника и строки приемника:
		bool procFindRow(const TExtendSheetReport& srcSheet, const CellDataArr& params,
			size_t dstRow, size_t srcRow) noexcept(true);
		//функция обработки ячеек приемника и источника:
		bool procFindCell(const TExtendSheetReport& srcSheet, const NS_Tune::TCellData& param, 
			size_t dstRow, size_t srcRow) noexcept(true);
		//функция поиска и обработки каждой ячейки строки приемника на листе источника:
		bool Proc_DstRowCells_In_SrcSheet(const TExtendSheetReport& srcSheet, NS_ExcelReport::TRowsFlag& srcRows,
			const CellDataArr& params, size_t curRow, bool NoSpaceNoCase = true) noexcept(true);
		//функция обработки метода поиска ячейки приемника на странице источника
		bool Execute_Seek_Dst_Cell_In_Src_Sht(const TExtendSheetReport& srcSheet, TRowsFlag& DstRows,
			TRowsFlag& SrcRows, const CellDataArr& params, bool NoSpaceNoCase = true) noexcept(true);
		//функция поиска строи источника по параметрам приемника(применяя поиск в большем меньшего)
		static size_t get_SrcRow_By_Dst_Params(const TJsonReport& Dst, const TExtendSheetReport& Src,
			TRowsFlag& DstRows, TRowsFlag& SrcRows, const NS_Tune::CellDataArr& original_params,
			bool NoSpaceNoCase = true) noexcept(false);
		//функция обработки метода поиска b обработки строки приемника на странице источника
		bool Execute_Seek_Dst_Row_In_Src_Sht(const TExtendSheetReport& srcSheet, TRowsFlag& DstRows,
			TRowsFlag& SrcRows, const CellDataArr& params, bool NoSpaceNoCase = true) noexcept(false);
		//функция проверки наличия данных файла-приемника на листе в файле-источнике:
		bool Search_DestData_In_SrcSheet(TRowsFlag& DstRows, const TExtendSheetReport& srcSheet, 
			bool NoSpaceNoCase = true) noexcept(true);
		//функция вставки данных из другого excel-файла:
		void Compare_Excel_Sheets(bool NoSpaceNoCase = true) noexcept(false);
		//функция получения числа конфигурационных файлов:
		size_t getTuneFilesCnt() const noexcept(true) { return cells_data.getDBTuneArr().size(); }
		//функция получения кода метода обработки:
		NS_Const::JSonMeth getMethCode() const noexcept(true) { return meth_code; }
	public:
		//инициализация при помощи объектов из json-файла
		TJsonReport(TExcelBook& book_ref, const TExcelProcData& json_tune, size_t page);
		//инициализация отдельными объектами:
		TJsonReport(TExcelBook& book_ref, const TShareData& DstFile, const TProcCell& cell_arr, size_t page);
		//деинициализация:
		virtual ~TJsonReport() {}
		//функция формирования отчета
		virtual bool crtSheetReport() noexcept(true);
		//функция признака предобработки:
		bool preProcFlg() const noexcept(true) { return meth_code == NS_Const::JSonMeth::GetRowIDByDB; }

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
		//static string getSqlByTune(bool use_parse, bool by_str, const string& str) noexcept(true);
		//получение текста sql-запроса из настроек:
		static string getSqlByTune(const TUserTune& tune, bool asDQL) noexcept(true);
		//вызов выполнения указанной dml-команды:
		static size_t executeDML(TDBConnect& db, const TUserTune& param, const string& dml, bool use_comit = true) noexcept(false);
		//вызов dml-команды для файлов:
		static size_t runDML4Directory(TDBConnect& db, const TUserTune& param, bool use_comit = true) noexcept(false);
		//выполнение DML запроса(можно выполнять без привязки к отчету):
		static size_t runDML(TDBConnect& db, const TUserTune& param, bool use_comit = true) noexcept(true);
		//установка значения поля по типу данных из индекса ячейки sql-данных
		static void setCellByResultSet(TExcelBook& book, TExcelBookSheet& sheet, const NS_Const::DataType& dt,
			const NS_Oracle::TBaseSet& bs, size_t resultSetCol, const NS_Excel::TExcelCell& cell, 
			const NS_Excel::FormatPtr format) noexcept(false);
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
		//формирование страницы отчета по строке запроса:
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

	//класс объектов для формирования отчетов по Смолевичу:
	class TSmlvchReport : public TBaseSheetReport
	{
		protected:
			const NS_Tune::TSharedTune& config;//ссылка на общие настройки отчета
			//функция определения первой/последней строки:
			virtual size_t getRow(bool first) const noexcept(false) { return TBaseSheetReport::getRow(first); }
			//функция инициализации страницы для отчета:
			virtual bool InitSheet(const string& sh_name, bool set_as_active = true) noexcept(true);
		public:
			//инициализация объекта:
			TSmlvchReport(NS_Excel::TExcelBook& book_link, const NS_Tune::TSharedTune& tune) :
				TBaseSheetReport(book_link), config(tune) {}
			//функция формирования страницы отчета для импортируемого файла баланса:
			//virtual bool crtSheet() const noexcept(true) = 0;
	};

	//Отчет по Ведомости остатков Смолевич:
	class TSmlvchBalance: public TSmlvchReport
	{
		private:
			using SubHeaderRows = std::vector<size_t>;
			//функция установки форматов для ячейки из массива форматов отчета:
			bool UpdFormatArrByColor(const NS_Excel::TColor& color, bool fnd_flg, bool font_flg) noexcept(true);
			//обработка подзаголовков:
			bool setSubHeadersFont(const SubHeaderRows& rows) noexcept(true);
			//функция получения настроек для составления отчета:
			NS_Tune::TBalanceTune getBalanceTune() const noexcept(true);
			//функция записи итогов:
			bool setTotalFields(size_t curRow, bool active_flg, double sld_rub, double sld_val, 
				const NS_Tune::CellDataArr& params) noexcept(true);
			//функция записи счета в строку excel-документа
			bool setAccount2Row(size_t Row, const NS_SMLVCH_IMP::TAccount& acc, const NS_Tune::CellDataArr& params, 
				const NS_Excel::TColor& color = NS_Excel::TColor::COLOR_NONE,	double rate = 1.0) noexcept(true);
			//функция записи блока счетов на текущий лист, так же подаем первую и последнюю строчки для обрамления данных:
			bool setAccounts2Sheet(size_t& curRow, const NS_SMLVCH_IMP::TAccounts& arr, const NS_Tune::CellDataArr& params,
				const NS_Tune::TCurrencyBlock& rates, SubHeaderRows& headers, const string& row_name_grp, bool last_row_as_sum = true) noexcept(true);
			//функция формирования страницы отчета для загружаемого файла:
			bool crtSheet(const string& imp_file, const NS_Tune::TBalanceTune& tune) noexcept(true);
		public:
			//инициализация
			TSmlvchBalance(NS_Excel::TExcelBook& book_link, const NS_Tune::TSharedTune& tune) : TSmlvchReport(book_link, tune) {}
			//функция формирования страницы отчета для импортируемого файла баланса:
			virtual bool crtReport() noexcept(true);
	};

	//класс реализации импорта документов в RS Bank:
	class TSmlvchImp : public TSmlvchReport
	{
	private:
		NS_SMLVCH_IMP::TRSBankImp imp_data;//данные для импорта
		NS_Tune::TImpDocsTune imp_tune;//настройки импорта данных:
		//функция считывания данных из строки:
		bool readRowData(size_t curRow) noexcept(true);
		//функция считывания страницы книги:
		bool readSheet(size_t first_row, size_t last_row) noexcept(true);
		//функция заполнения массива документов для одного excel-файла:
		bool readFile(const string& file) noexcept(true);
	public:
		//инициализация
		TSmlvchImp(NS_Excel::TExcelBook& book_lnk, const NS_Tune::TSharedTune& tune_lnk, const string& json_file);
		//функция счиытвания excel-файлов в структуру:
		bool setDocsByFiles() noexcept(true);
		//функция записи данных в файл:
		bool crtOutFile() const noexcept(true);
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
		//функция обработки одного файла подконфигурации
		bool Json_SubTune_File_Run(NS_Excel::TExcelBook& book, const string& js_file) const noexcept(true);
		//функция загрузки данных из excel-файла
		bool loadFromJson(const string& js_file) const noexcept(true);
	public:
		//инициализация
		explicit TReport(const string& config_file = string());
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
		//формирование отчета в 2 этапа конфигурационных файлов:
		//на первом этапе формируется отчет на основании файлов в пааке config
		//на втором этапе обрабатывается указанный отчет на основании файлов в папке SubConfig
		//функция выполнения 2ого этапа обработки отчета для поднастроек(subconfig):
		void SubConfig_Json_UpdOutExlFile() const noexcept(true);
		//функция выполнения 2ого этапа обработки - обработка ini-файла:
		void SubConfig_IniFile_Execute() const noexcept(true);
		//функция загрузки данных из excel в базу данных:
		void load2DBFromExcel() const noexcept(false);
		//формирование ведомости остатков для Смолевича по OEM-файлу из RS-Bank
		void Smolevich_Balance_Report() const noexcept(true);
		//формирование файла списка документов для загрузки в RS-Bank:
		bool Smolevich_Docs_Import() const noexcept(true);
		//вызов функции, формирующей отчет по коду:
		void Create_Report_By_Code(const NS_Const::ReportCode& code) const;
		//формирование полного отчета по файлу main_config:
		bool Execute() const noexcept(true);
		//!!!Переделать!!!
		//формирование файла с документами на импорт в RS Bank на основании excel-документа
		static void Smolevich_Imp_Docs(const string& path, const string& out_file) noexcept(true);
	};

}
#endif