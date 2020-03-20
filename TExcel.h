#ifndef TEXCEL_H_
#define TEXCEL_H_
#include <string>
#include <vector>
//#include <map>
#include <set>
#include "libxl.h"
#include "TConstants.h"

//Ограничения для excel страниц:
//http://www.excelworld.ru/publ/help/char_and_rest/sheet_char_and_rest/37-1-0-99

namespace NS_Excel
{
	using std::string;
	using BookPtr = libxl::Book*;
	using SheetPtr = libxl::Sheet*;
	using FontPtr = libxl::Font*;
	using FormatPtr = libxl::Format*;
	
	using TDataType = libxl::CellType;
	using TColor = libxl::Color;
	
	using TStrArr = std::vector<std::string>;
	using TDataTypeArr = std::vector<TDataType>;
	//using PFormat = std::pair<FormatPtr, FormatPtr>;
	using PFormatArr = std::set<FormatPtr>;

	using TScriptFontType = libxl::Script;
	using TUnderLineFontType = libxl::Underline;

	using NS_Const::EmptyType;
	using NS_Const::TExclBaseTune;
	using NS_Const::TConstExclTune;

	//enum class TDataFormat { Default = 0, AsString, AsInteger, AsDate, AsBool };

	//структура Дата
	struct TExcelDate
	{
	private:
	public:
		int year;
		unsigned int month;
		unsigned int day;
		unsigned int hour;
		unsigned int minute;
		unsigned int sec;
		unsigned int msec;
		//функция проверки на пустоту:
		bool isEmpty() const noexcept(true);
		//функция преобразования в строку(формта для библиотеки boost::date_time):
		std::string toStr(const std::string& format = "%d.%m.%Y") const noexcept(true);
		static string toStr(double dbl_date, const string& format = "%d.%m.%Y") noexcept(true);
	};

	//структураданных для формирования отчета:
	class TExcelParam
	{
	private:
		string template_file;//имя файла шаблона
		string out_name;//имя выходного файла
		TStrArr header;//наименование колонок для заголовка в порядке следования
	public:
		TExcelParam(const string& par_tmpl_name, const string& par_out_file, const TStrArr& cols) :
			template_file(par_tmpl_name), out_name(par_out_file), header(cols) {}
		//проверка на пустоту
		bool isEmpty() const { return (template_file.empty() or (out_name.empty() and header.empty())); }
		//получение значений:
		//имя файла шаблона:
		string getTemplateName() const { return template_file; }
		//имя выходного файла:
		string getOutName() const { return out_name; }
		//массив колонок заголовка:
		TStrArr getHeader() const { return header; }
		//наименование колонки по индексу:
		string getColumnName(int indx) const noexcept(true);
		//количество колонок:
		int ColumnsCnt() const { return header.size(); }
		//получение расширения файла
		static string getExtensionFile(const string& srt) noexcept(true);
	};

	//базовый вспомогательный объект
	class TBaseObj
	{
	private:
		bool from_zero;//отсчет ведется от 0 иначе от 1
		int val;//значение
	public:
		explicit TBaseObj(int par_val, bool zero_flg = true);
		//проверка на пустое значение:
		bool isEmpty() const { return val < 0; }
		//проверка корректности значения:
		bool isValid() const;
		//полчение значения:
		int Value(bool frm_zero = true) const { return frm_zero ? val : val + 1; }
		//получение значения следующего элемента:
		int Next();
		//получение значения предыдущего объекта:
		int Prev();
		//преобразование в int
		operator int() { return val; }
		//оператор присвоения:
		TBaseObj& operator=(const TBaseObj& v);
		TBaseObj& operator=(int x);
		//сравнения
		bool operator==(int x) const { return val == x; }
		bool operator==(const TBaseObj& x) const { return val == x.val; }
		bool operator<(const TBaseObj& x) const { return val < x.val; }
		bool operator<(int x) const { return val < x; }
		bool operator<=(int x) const { return val <= x; }
		bool operator<=(const TBaseObj& x) const { return val <= x.val; }
		bool operator>=(int x) const { return val >= x; }
		void clear() { from_zero ? val = EmptyType : val = 0; }
		bool isZero() const noexcept(true) { return val == 0; }
		string getName(bool row) const;
	};

	//вспомогательный класс Ячейка:
	class TExcelCell
	{
	private:
		TBaseObj row;//строка
		TBaseObj col;//столбец
	public:
		//конструктор
		TExcelCell(const TBaseObj& par_row, const TBaseObj& par_col) : row(par_row), col(par_col) {}
		TExcelCell(int par_row, int par_col, bool from_zero = true) : row(par_row, from_zero), col(par_col, from_zero) {}
		//получение индекса строки ячейки:
		int getRow(bool frm_zero = true) const { return row.Value(frm_zero); }
		//получение индекса столбца ячейки:
		int getCol(bool frm_zero = true) const { return col.Value(frm_zero); }
		//валидность объекта:
		bool isValid() const { return row.isValid() and col.isValid(); }
		//пустой объект:
		bool isEmpty() const { return row.isEmpty() or col.isEmpty(); }
		//получение следующей ячейки в строке
		int getNextRowCell() { return row.Next(); }
		//получение ссылки на предыдущую ячейку в строке:
		int getPrevRowCell() { return row.Prev(); }
		//получение следующей ячейки в столбце
		int getNextColCell() { return col.Next(); }
		//получение ссылки на предыдущую ячейку в столбце:
		int getPrevColCell() { return col.Prev(); }
		//операторы сравнения:
		bool operator==(const TExcelCell& x) const { return row == x.row and col == x.col; }
		bool operator<(const TExcelCell& x) const { return row < x.row and col < x.col; }
		bool operator<=(const TExcelCell& x) const { return row <= x.row and col <= x.col; }
		void clear() { row.clear(); col.clear(); }
		bool isZero() const { return row.isZero() and col.isZero(); }
		string getName() const { return row.getName(true) + col.getName(false); }
	};

	//класс Диапозон:
	class TExcelRange
	{
	private:
		TExcelCell first;
		TExcelCell last;
		enum class TObjType { Row, Col };
		bool inRange(int val, const TObjType& t) const;
	public:
		//конструкторы:
		TExcelRange(int first_row, int first_col, int last_row, int last_col) :
			first(first_row, first_col), last(last_col, last_col) {}
		TExcelRange(const TExcelCell& first_cell, const TExcelCell& last_cell) :
			first(first_cell), last(last_cell) {}
		//проверка на валидность
		bool isValid() const { return first.isValid() and last.isValid() and first <= last; }
		//проверка на пустоту:
		bool isEmpty() const { return first.isEmpty() or last.isEmpty(); }
		//проверка наличия в диапазоне:
		bool inRange(const TExcelCell& cell) const;
		TExcelCell getFirst() const noexcept(false) { return first; }
		TExcelCell getLast() const noexcept(false) { return last; }
		void clear() { first.clear(); last.clear(); }
		string getName() const { return first.getName() + ":" + last.getName(); }
		TExcelCell operator()(int row_indx, int col_indx) const;
	};

	//классы использующиеся для формирования excel-документа:

	class TExcelBook;
	class TExcelBookSheet;
	class TExcelBookFormat;
	class TExcelBookFont;

	//класс Excel-книга - http://www.libxl.com/workbook.html
	class TExcelBook
	{
	public:
		//первая строка заголовка - т.к. библиотека для работы с excel вставляет свою строку:
		using TType = libxl::SheetType;
		//libxl::SheetState;
		//установка ключей лицензии
		static bool UseLicenseKey(BookPtr* b) noexcept(true);
	private:
		string fname;//наименование файла книги
		int HeaderRow;//индекс строки заголовка
		BookPtr book;//ссылка на книгу
		//ограничительные настройки
		size_t max_cols;//максимальное число колонок в книге
		size_t max_rows;//максимальное число строк в книге
		PFormatArr frmat_arr;//массив форматов текщуй книги:
		//установка ограничений для книги:
		void setConstraints() noexcept(true);
		//запрет операции присвоения
		TExcelBook& operator=(const TExcelBook& b);
		//запрет инициализацией этим же объектом:
		TExcelBook(const TExcelBook& b);
		//функция формирования массива форматов:
		void InitFormatArr() noexcept(true);
		//умолчательная инициализация:
		void CrtBook(int header_row = 0) noexcept(true);
		void InitBook(BookPtr* b);
		enum class LoadType { Full, Sheet, Rows };
		struct TLoadParam { string file; int Indx; int first; int last; string tmp_file; };
		//функция загрузки:
		bool loadFromFile(const TLoadParam& param, const LoadType& lt, bool raise_err) noexcept(false);
		//сформировать имя листа по умолчанию:
		string getDefaultSheetName() const noexcept(true);
		//функция проверки наличия указанной страницы в книге:
		bool checkUniqSheetName(const string& name) const noexcept(false);
		//добавление формата в книгу(без проверок):
		FormatPtr InsertFormat(FormatPtr initFormat = nullptr) noexcept(true);
		//функция добавления формата - возвращает сылку на формат
		NS_Excel::FormatPtr AddFormatPtr(FormatPtr initFormat = nullptr, bool use_check = true) noexcept(false);
	public:
		//конструкторы:
		explicit TExcelBook(const string& book_name, int header_row = 0): fname(book_name), 
			HeaderRow(header_row) { CrtBook(); }
		//деструктор:
		~TExcelBook() { close(); }
		//сообщение об ошибке:
		string getError() const noexcept(true);
		//валидность объекта:
		bool isValid(bool raise_err = false) const noexcept(false);
		//проверка на пустоту:
		bool isEmpty() const noexcept(true) { return book ? !SheetCount() : true; }
		//заполняем заголовок из массива строк:
		void setHeaderByStrArr(const TStrArr& arr, bool use_active_sheet = true, const string& new_sh_name = "") noexcept(true);
		//инициализация книги по параметрам
		bool initByParam(const TExcelParam& param) noexcept(true);
		//загрузка из файла:
		bool load(const string& file, bool raise_err = false, const string& tmp_file = string()) noexcept(false);
		//загрузка листа из файла все остальные данные в книге стираются!!!:
		bool loadSheetOnly(const string& file, int sheet_indx,
			bool raise_err = false, const string& tmp_file = string()) noexcept(false);
		//загрзка страницы из книги:
		bool setSheetByTemplate(const string& file, const string& new_sh_name, int templ_sheet_indx = 0,
			bool as_active = false, const string & tmp_file = string()) noexcept(false);
		//загрузка строк данных из листа файла:
		bool loadSheetRowsOnly(const string& file, int sheet_indx, int first_row, int last_row,
			bool raise_err = false, const string& tmp_file = string());
		//сохранение данных в файл:
		bool SaveToFile(const string& file = "", bool use_tmp = false, bool raise_err = false) noexcept(false);
		//загрузка информации о страницах: число и имена страниц - становятся доступны
		bool LoadSheetsInfo(const string& file) noexcept(false) { if (isValid()) return book->loadInfo(file.c_str()); return false; };
		//добавление страницы:
		TExcelBookSheet AddSheet(const string& name = "", bool asActive = false) noexcept(false);
		//втсавка страницы:
		TExcelBookSheet InsertSheet(int index, const string& name, SheetPtr initSheet = nullptr) noexcept(false);
		//получение страцницы по индексу:
		TExcelBookSheet getSheetByIndex(int index) const noexcept(false);
		//получение ссылки на последнюю страницу:
		TExcelBookSheet getLastSheet(bool set_active) noexcept(false);
		//получение имени страцниы по индексу:
		string getSheetNameByIndex(int index) const noexcept(false);
		//получение типа страницы:
		TType getSheetTypeByIndex(int indx) const noexcept(false) { return book->sheetType(indx); };
		//переместить страницу:
		bool MoveSheetByIndex(int old_indx, int new_indx) noexcept(false);
		//удаление страницы:
		bool DelSheetByIndex(int index) noexcept(false);
		//получение числа страниц в рабочей книге:
		int SheetCount() const noexcept(false) { return book->sheetCount(); };
		//добавление формата в книгу:
		TExcelBookFormat AddFormat(TExcelBookFormat& initFormat, bool use_check = true) noexcept(false);
		//функция получения ссылки на формат по индексу:
		FormatPtr getFormatPrtByIndex(size_t index) const noexcept(false);
		//получение формата по индексу:
		TExcelBookFormat getFormatByIndex(int index) noexcept(false);
		//получение индекса для формата книги:
		size_t getFormatIndex(const TExcelBookFormat& format) const noexcept(false);
		//количество форматов в книге:
		int FormatCount() const noexcept(true) { return (isValid() ? book->formatSize() : EmptyType); }
		//добавление шрифта в книгу:
		TExcelBookFont AddFont(FontPtr initFont = nullptr) noexcept(false);
		//получение ссылки на шрифт по индексу:
		TExcelBookFont getFontByIndex(int index) noexcept(false);
		//число шрифтов в книге:
		int getFontCount() const noexcept(true) { return (isValid() ? book->fontSize() : EmptyType); }
		//индекс активной страницы:
		int getActiveSheetIndx() const noexcept(false) { return (isValid() ? book->activeSheet() : EmptyType); }
		TExcelBookSheet getActiveSheet() const noexcept(false);
		//установить страницу активной:
		bool setActiveSheet(int index) noexcept(true);
		//проверка является ли книга - шаблоном:
		bool isTemplate() const noexcept(true) { return (isValid() ? book->isTemplate() : false); }
		//установка книги в качестве шаблона:
		bool setAsTemplate(bool flg = true) noexcept(true);
		//установка локали:
		bool setLocale(const string& locale) noexcept(true);
		//определяются ли ячейки как ссылки вида: RxxCxx
		bool isRxCxRef() const noexcept(true) { return (isValid() ? book->refR1C1() : false); }
		//определять ячейки как ссылки вида RxxCxx:
		bool setRxCxRef(bool flg = true) noexcept(true);
		//данные о шрифте по умолчанию:
		void DefultFont(string& font_name, int& size) noexcept(false);
		//установить шрифт по умолчанию:
		bool setDefaultFont(const string& font_name, int size) noexcept(true);
		//упаковка даты в тип double:
		double Date2Double(const TExcelDate& date) noexcept(false);
		//представление double в виде даты:
		bool Double2Date(double value, TExcelDate& date) noexcept(false);
		//строка заголовка:
		int getHeaderRow() const noexcept(true) { return HeaderRow; }
		//функция получения имени файла:
		string getFileName() const noexcept(true) { return fname; }
		//функция получения расширения файла:
		//если ошибка - вернет пустую строку
		string getFileExtend() const noexcept(true);
		//функция получения максимального числа строк:
		size_t MaxRowsCount() const { return max_rows; }
		//функция получения максимального числа столбцов:
		size_t MaxColsCount() const { return max_cols; }
		//закрытие книги:
		void close() noexcept(false);
	};

	//класс страница/лист excel-документа: http://www.libxl.com/spreadsheet.html
	class TExcelBookSheet
	{
	public:
		using TErrorType = libxl::ErrorType;
	private:
		SheetPtr sheet;
		//инициализация страницы:
		void initSheet(BookPtr book, const string& name, bool active_flg) noexcept(false);
		//вставка строки или столбца:
		bool RangeOperation(const TExcelRange& range, bool insrt, bool asRow, bool updNameRange = true) noexcept(false);
		//функционал для копирования листа:
		//копирование параметров ячеек с другого листа
		bool copySheetColsParam(const TExcelBookSheet& src_sh) noexcept(true);
		//установка объединения ячеек из исходного листа:
		bool copySheetCellsMerge(TExcelBookSheet& src_sh, const TExcelCell& cell) noexcept (true);
		//копирование значения ячейки из листа-источника:
		bool copySheetCellValue(TExcelBookSheet& src_sh, const TExcelCell& cell, FormatPtr format) noexcept(true);
	public:
		//конструктор:
		TExcelBookSheet(BookPtr book, const string& name, bool active_flg = true);
		TExcelBookSheet(const TExcelBookSheet& sh) : sheet(sh.sheet) {};
		explicit TExcelBookSheet(SheetPtr sh) : sheet(sh) {}
		//деструктор:
		~TExcelBookSheet() { sheet = nullptr; }
		//присвоенение
		TExcelBookSheet& operator=(const SheetPtr sh) { if (sheet != sh) sheet = sh; return *this; }
		TExcelBookSheet& operator=(const TExcelBookSheet& sh) { return operator=(sh.sheet); }
		//валидностьобъекта:
		bool isValid() const { return sheet; }
		//проверка ячейки на пустое значение:
		bool isEmptyCell(const TExcelCell& cell) const noexcept(false);
		//получение ссылки на страницу/лист:
		SheetPtr getRef() { return sheet; }
		//получение типа данных в ячейке:
		TDataType getCellType(const TExcelCell& cell) const noexcept(false) { return sheet->cellType(cell.getRow(), cell.getCol()); }
		//номер первой использующейся строки
		int getFirstRow() const { return sheet->firstRow(); }
		//номер последней использующейся строки:
		int getLastRow() const { return sheet->lastRow(); }
		//номер первого ипользующегося столбца:
		int getFirstCol() const { return sheet->firstCol(); }
		//номер последнего использующегося столбца:
		int getLastCol() const { return sheet->lastCol(); }
		//запись строки в ячейку с указанием формата и типа данных:
		bool WriteAsString(const TExcelCell& cell, const string& val, FormatPtr format = nullptr, const TDataType& type = TDataType::CELLTYPE_STRING);
		//чтение строки из ячейки, формат не считывается:
		std::string ReadAsString(const TExcelCell& cell, FormatPtr format = nullptr) const noexcept(false);
		//чтение числа:
		double ReadAsNumber(const TExcelCell& cell, const FormatPtr format = nullptr) const;
		//запись числа в ячейку:
		bool WriteAsNumber(const TExcelCell& cell, double val, FormatPtr format = nullptr);
		//чтение bool:
		bool ReadAsBool(const TExcelCell& cell, const FormatPtr format = nullptr) const;
		//запись bool:
		bool WriteAsBool(const TExcelCell& cell, bool val, FormatPtr format = nullptr) const;
		//считываем формат пустой ячейки, если она пустая
		bool isBlank(const TExcelCell& cell) const { FormatPtr tmp = nullptr; return ReadBlankFormat(cell, &tmp); }
		bool ReadBlankFormat(const TExcelCell& cell, FormatPtr* format) const { return sheet->readBlank(cell.getRow(), cell.getCol(), format); };
		//установка формата для пустой ячейкт, еслиячейка не пустая - возвращает false
		bool setBlank(const TExcelCell& cell, FormatPtr format = 0) const;
		//чтение формулы:
		std::string ReadFormula(const TExcelCell& cell);
		//запись в ячейку формулы:
		bool WirteFormula(const TExcelCell& cell, const string& formula, FormatPtr format = 0);
		//запись в ячейку формулы с установленных значением по умолчанию в виде числа:
		bool WirteFormulaNumAsDef(const TExcelCell& cell, const string& formula, double def_val, FormatPtr format = 0);
		//запись в ячейку формулы с установленных значением по умолчанию в виде строки:
		bool WirteFormulaStrAsDef(const TExcelCell& cell, const string& formula, const string& def_val, FormatPtr format = 0);
		//запись в ячейку формулы с установленных значением по умолчанию в виде bool:
		bool WirteFormulaBoolAsDef(const TExcelCell& cell, const string& formula, bool def_val, FormatPtr format = 0);
		//имеется ли в ячейке формула:
		bool isFormula(const TExcelCell& cell) const noexcept(false) { return sheet->isFormula(cell.getRow(), cell.getCol()); }
		//чтение коментария из ячейки:
		string getComment(const TExcelCell& cell) const noexcept(false);
		//запись коментария в ячейку:
		bool setComment(const TExcelCell& cell, const string& val, const string& author = "", int width = 129, int height = 75) noexcept(false);
		//удаление коментария из ячейки:
		bool delComment(const TExcelCell& cell) noexcept(false);
		//проверка является ли значение в ячейки датой
		bool isDate(const TExcelCell& cell) const noexcept(false) { return sheet->isDate(cell.getRow(), cell.getCol()); }
		//чтение вида ошибки в ячейки:
		TErrorType getErrorType(const TExcelCell& cell) const { return sheet->readError(cell.getRow(), cell.getCol()); }
		//запись в ячейку ошибки:
		bool setErrorType(const TExcelCell& cell, const TErrorType& er, FormatPtr format = 0);
		//ширина столбца
		double getColWidth(int col_indx) const noexcept { return sheet->colWidth(col_indx); }
		//высота строки:
		double getRowHeight(int row_indx) const noexcept(false) { return sheet->rowHeight(row_indx); }
		//установка ширины для диапозона:
		bool setColsParams(const TExcelRange& r, double width, bool hide = false, FormatPtr format = nullptr) noexcept(true);
		//утсановка высоты для диапазона:
		bool setRowParams(int row, double height, bool hide = false, FormatPtr format = nullptr) noexcept(false) { return sheet->setRow(row, height, format, hide); };
		//проверяем скрыта ли строка:
		bool isRowHide(int row_indx) const noexcept(false) { return sheet->rowHidden(row_indx); }
		//скрываем строку:
		bool setRowHide(int row_indx, bool hide);
		//проверяем скрыт ли столбец:
		bool isColHide(int col_indx) const noexcept(false) { return sheet->colHidden(col_indx); }
		//скрываем столбец:
		bool setColHide(int col_indx, bool hide);
		//работа с объединениями ячеек:
		//проверяем попадает ли ячейка в область объединения - на выходе область в которую попадает ячейка:
		bool inMergeRange(const TExcelCell& cell, TExcelRange& result_range) noexcept(true);
		//объединение ячеек из диапазона:
		bool setMergeRange(const TExcelRange& range) noexcept(true);
		//удаляем ячейку из объединения:
		bool delMerge(const TExcelCell& cell) noexcept(true);
		//количество объединений ячеек на листе:
		int getMergeSize() const noexcept(false) { return sheet->mergeSize(); }
		//полчаем диапазон объединенных ячеек по индексу:
		bool getMergeRangeByIndx(int indx, TExcelRange& result_range) noexcept(true);
		//удаление объединения ячеек по индексу:
		bool delMergeByIndx(int indx) noexcept(true) { return (isValid() ? sheet->delMergeByIndex(indx) : false); };
		//разрыв:
		//установить
		bool setSplit(const TExcelCell& cell);
		//получить ячейку из разрыва
		bool getSplit(TExcelCell& cell) const;
		//очисткалиста от данных(только для xls???)
		bool clear(const TExcelRange& range) noexcept(true);
		//вставка строк:
		bool insRows(const TExcelRange& range, bool updNameRange = true) noexcept(false) { return RangeOperation(range, true, true, updNameRange); }
		//вставка столбцов:
		bool insCols(const TExcelRange& range, bool updNameRange = true) noexcept(false) { return RangeOperation(range, true, false, updNameRange); }
		//удаление строк:
		bool delRows(const TExcelRange& range, bool updNameRange = true) noexcept(false) { return RangeOperation(range, false, true, updNameRange); }
		//удаление столбцов:
		bool delCols(const TExcelRange& range, bool updNameRange = true) noexcept(false) { return RangeOperation(range, false, false, updNameRange); }
		//копирование ячейки
		bool copyCell(const TExcelCell& src, const TExcelCell& dst) noexcept(false) { return sheet->copyCell(src.getRow(), src.getCol(), dst.getRow(), dst.getCol()); }
		//ориентация страницы:
		bool isLandScape() const noexcept(false) { return sheet->landscape(); }
		//установка ориентации страницы:
		void setLandScape(bool landscape = true) noexcept(false) { sheet->setLandscape(landscape); }
		//получение наименования листа:
		string getName() const;
		//установка имени страницы
		void setName(const string& str) noexcept(false) { sheet->setName(str.c_str()); }
		//получение формата ячейки:
		FormatPtr getCellFormatPtr(const TExcelCell& cell) const noexcept(false);
		//получение формата ячейки:
		TExcelBookFormat getCellFormat(const TExcelCell& cell) const noexcept(false);
		//установка формата ячейки:
		bool setCellFormat(const TExcelCell& cell, TExcelBookFormat& format) noexcept(false);
		//установка цвета для формата в указанной ячейке!!!!
		bool setCellColor(const TExcelCell& cell, const TColor& color) noexcept(true);
		//утсновка цвета для формата в диапазоне:
		bool setRangeColor(const TExcelRange& range, const TColor& color) noexcept(true);
		//установка цвета для формата в строке
		bool setRowColor(int row_indx, const TColor& color) noexcept(true);
		//установка цвета для формата в столбце:
		bool setColColor(int col_indx, const TColor& color) noexcept(true);
		//установка цвета текста ячейки:
		bool setCellTextColor(const TExcelCell& cell, const TColor& color) noexcept(true);
		//проверка отображения сетки таблицы:
		bool GreedlinesShowed() const { return sheet->displayGridlines(); }
		//установка отображения сетки:
		void ShowGreedlines(bool flg = true) { if (isValid()) sheet->setDisplayGridlines(flg); }
		//дружественные функции:
		friend bool TExcelBook::setSheetByTemplate(const string& file, const string& new_sh_name, int templ_sheet_indx,
			bool raise_err, const string & tmp_file) noexcept(false);
	};
	
	//класс формата - http://www.libxl.com/format.html
	class TExcelBookFormat
	{
	public:
		using TNumFormat = libxl::NumFormat;
		using TAlignHType = libxl::AlignH;
		using TAlignVType = libxl::AlignV;
		using TBorderStyle = libxl::BorderStyle;
		using TDiagonal = libxl::BorderDiagonal;
		using TFill = libxl::FillPattern;
		//класс описывающий стороны для ячейки
		enum class TBorderSide { Full, Left, Right, Top, Bottom, Diagonal, Background, Foreground };
	private:
		FormatPtr pformat;//ссылка на формат книги
		//инициализация
		void initFormat(BookPtr book) noexcept(false);
	public:
		//конструктор
		//explicit TExcelBookFormat(BookPtr book) : pformat(nullptr) { initFormat(book); }
		explicit TExcelBookFormat(FormatPtr pf) : pformat(pf) {}
		TExcelBookFormat(const TExcelBookFormat& pf) : pformat(pf.pformat) {};
		//оператор присвоения:
		TExcelBookFormat& operator=(FormatPtr format) { if (pformat != format) pformat = format; return *this; }
		TExcelBookFormat& operator=(const TExcelBookFormat& pf) { return operator=(pf.pformat); }
		//		explicit TExcelBookFormat(const TExcelBook& book);
		~TExcelBookFormat() { pformat = nullptr; }
		//валидность:
		inline bool isValid() const { return pformat; }
		//шрифт
		TExcelBookFont getFont() const noexcept(false);
		bool setFont(TExcelBookFont& par_fnt) noexcept(false);
		//числовой формат:
		int getNumFormat() const;
		void setNumFormat(TNumFormat val);
		//выравнивание в ячейках:
		//горизонтальное:
		TAlignHType getAlignH() const { return (isValid() ? pformat->alignH() : TAlignHType(EmptyType)); }
		void setAlignH(TAlignHType val) { if (isValid()) pformat->setAlignH(val); }
		//вертикальное:
		TAlignVType getAlignV() const { return (isValid() ? pformat->alignV() : TAlignVType(EmptyType)); }
		void setAlignV(TAlignVType val) { if (isValid()) pformat->setAlignV(val); }
		//функция разделения текста: т.е. текст разбивается для заполнения ячейки
		bool isWrap() const noexcept(false) { return pformat->wrap(); }
		void setWrap(bool flg = true) { if (isValid()) pformat->setWrap(flg); }
		//вращение текста(угол наклона):
		int getRotation() const noexcept(false) { return pformat->rotation(); }
		void setRotation(int par_rotation) { if (isValid()) pformat->setRotation(par_rotation); }
		//отступ:
		int getIndent() const noexcept(false) { return pformat->indent(); }
		void setIndent(int par_indent) { if (isValid()) pformat->setIndent(par_indent); }
		//уменьшение ячейки до размера текста
		bool isShrink2Fit() const noexcept(false) { return pformat->shrinkToFit(); }
		void setSrink2Fit(bool flg = true) { if (isValid()) pformat->setShrinkToFit(flg); }
		//защита данных ячейки:
		bool isLocked() const noexcept(false) { return pformat->locked(); }
		void setLocked(bool flag = true) { if (isValid()) pformat->setLocked(flag); }
		//Свойство скрыть ячейку/строку
		bool isHidden() const noexcept(false) { return pformat->hidden(); }
		void setHidden(bool flg = true) { if (isValid()) pformat->setHidden(flg); }
		//работа с границами ячейки:
		//установка границ ячейки:
		void setBorderStyle(TBorderStyle style, const TBorderSide& side);
		//получение стиля границ ячейки
		TBorderStyle getBorderStyle(const TBorderSide& side) const noexcept(true);
		//установка цвета границы ячейки:
		void setBorderColor(const TColor& c, const TBorderSide& side);
		//получение стиля цвета границы
		TColor getBorderColor(const TBorderSide& side) const noexcept(false);
		//диагональная черта:
		void setDiagonalBorder(TDiagonal val) noexcept(false) { if (isValid()) pformat->setBorderDiagonal(val); }
		TDiagonal getDiagonalBorder() const noexcept(false) { return pformat->borderDiagonal(); }
		//заполнитель шаблона:
		void setPatternFill(TFill val) noexcept(false) { if (isValid()) pformat->setFillPattern(val); }
		TFill getPatternFill() const noexcept(false) { return pformat->fillPattern(); }
		//получение ссылки на формат:
		//FormatPtr getRef() { return pformat; }
		friend bool NS_Excel::TExcelBookSheet::setCellFormat(const TExcelCell& cell, TExcelBookFormat& format) noexcept(false);
		//friend FormatPtr TExcelBook::InsertFormat(const TExcelBookFormat& frmt) noexcept(false);
		friend void TExcelBook::setHeaderByStrArr(const TStrArr& arr, bool use_active_sheet, const string& new_sh_name) noexcept(true);
		friend TExcelBookFormat TExcelBook::AddFormat(TExcelBookFormat& initFormat, bool use_check) noexcept(false);
		friend size_t TExcelBook::getFormatIndex(const TExcelBookFormat& format) const noexcept(false);
	};

	//Класс шрифта - http://www.libxl.com/font.html
	class TExcelBookFont
	{
	public:
		//константы для типа шрифта:
		enum class TFontType { Italic, Bold, StrikeOut, UnderLine, Script };
	private:
		FontPtr pfont;//указатель на шрифт
		void initFont(BookPtr book) noexcept(false);
		bool setFontType(const TFontType& ft);
	public:
		//конструктор
//		explicit TExcelBookFont(const TExcelBook& book);
		TExcelBookFont(const TExcelBookFont& par_fnt) : pfont(par_fnt.pfont) {};
//		explicit TExcelBookFont(BookPtr book) : pfont(nullptr) { initFont(book); }
		explicit TExcelBookFont(FontPtr par_fnt) : pfont(par_fnt) {}
		//деструктор
		~TExcelBookFont() { pfont = nullptr; }
		//присвоенение:
		TExcelBookFont& operator=(const FontPtr font) { if (font != pfont) pfont = font; return *this; }
		TExcelBookFont& operator=(const TExcelBookFont& par_fnt) { return operator=(par_fnt.pfont); };
		//валидность объекта:
		inline bool isValid() const { return pfont; }
		//размер шрифта
		int getSize() const { return pfont->size(); }
		void setSize(int val) { pfont->setSize(val); }
		//утсановка типа шрифта
		void setBold() { setFontType(TFontType::Bold); }
		void setItalic() { setFontType(TFontType::Italic); }
		void setStrikeOut() { setFontType(TFontType::StrikeOut); }
		void setScript(const TScriptFontType& sft);
		void setUnderLine(const TUnderLineFontType& uft);
		bool isFontType(const TFontType& ft) const;
		TScriptFontType isScript() const;
		TUnderLineFontType isUnderLine() const;
		//установка цвета шрифта:
		bool setColor(const TColor& c);
		TColor getColor() const;
		//наименование шрифта:
		bool setName(const string& s);
		string getName() const { return string(pfont->name()); }
		//получение ссылки на шрифт:
		//FontPtr getRef() { return pfont; }
		friend bool NS_Excel::TExcelBookFormat::setFont(TExcelBookFont& par_fnt) noexcept(false);
	};
	
};
#endif