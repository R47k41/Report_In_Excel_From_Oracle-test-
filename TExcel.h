#ifndef TEXCEL_H_
#define TEXCEL_H_
#include <string>
#include <vector>
#include "Logger.h"
#include "libxl.h"

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

	//константа - ошибка данных для enum:
	const int EmptyType = -1;
	//константы для формата файлов
	enum class TFormatTune {Empty = -1, xlt, xls, xlsx, DefExt, DefName, DefSh};
	//константы для типа шрифта:
	enum class TFontType {Italic, Bold, StrikeOut, UnderLine, Script};
	using TScriptFontType = libxl::Script;
	using TUnderLineFontType = libxl::Underline;

	//преобразование расширения в строку
	string getTuneFormat(const TFormatTune& val);
	//проверка является ли файл шаблоном:
	bool isTemplate(const string& str);

	//enum class TDataFormat { Default = 0, AsString, AsInteger, AsDate, AsBool };

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
	};
	
	//классы использующиеся для формирования excel-документа:
	class TExcelBook;
	class TExcelBookSheet;
	class TExcelBookFont;
	class TExcelBookFormat;
	
	//Класс шрифта - http://www.libxl.com/font.html
	class TExcelBookFont
	{
	private:
		FontPtr pfont;//указатель на шрифт
		//запрет операций
		TExcelBookFont& operator=(const TExcelBookFont& par_fnt);
		void initFont(BookPtr book) noexcept(false);
		bool setFontType(const TFontType& ft);
	public:
		//конструктор
//		explicit TExcelBookFont(const TExcelBook& book);
		TExcelBookFont(const TExcelBookFont& par_fnt) : pfont(par_fnt.pfont) {};
		explicit TExcelBookFont(BookPtr book) : pfont(nullptr) { initFont(book); }
		explicit TExcelBookFont(FontPtr par_fnt) : pfont(par_fnt) {}
		//деструктор
		~TExcelBookFont() { pfont = nullptr; }
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
		FontPtr getRef() { return pfont; }
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
		explicit TExcelBookFormat(BookPtr book) : pformat(nullptr) { initFormat(book); }
		explicit TExcelBookFormat(FormatPtr pf) : pformat(pf) {}
		TExcelBookFormat(const TExcelBookFormat& pf) : pformat(pf.pformat) {};
		//оператор присвоения:
		TExcelBookFormat& operator=(const TExcelBookFormat& pf) { if (pf.pformat != pformat) pformat = pf.pformat; return *this; }
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
		TAlignVType getAlignV() const{ return (isValid() ? pformat->alignV() : TAlignVType(EmptyType)); }
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
		TBorderStyle getBorderStyle(const TBorderSide& side) const noexcept(false);
		//установка цвета границы ячейки:
		void setBorderColor(TColor c, const TBorderSide& side);
		//получение стиля цвета границы
		TColor getBorderColor(const TBorderSide& side) const noexcept(false);
		//диагональная черта:
		void setDiagonalBorder(TDiagonal val) noexcept(false) { if (isValid()) pformat->setBorderDiagonal(val); }
		TDiagonal getDiagonalBorder() const noexcept(false) { return pformat->borderDiagonal(); }
		//заполнитель шаблона:
		void setPatternFill(TFill val) noexcept(false) { if (isValid()) pformat->setFillPattern(val); }
		TFill getPatternFill() const noexcept(false) { return pformat->fillPattern(); }
		//получение ссылки на формат:
		FormatPtr getRef() { return pformat; }
	};

	//базовый вспомогательный объект
	class TBaseObj
	{
	private:
		const int min_val;
		const int max_val;
		int val;//значение
	public:
			explicit TBaseObj(int par_val, int par_min = 0, int par_max = 0);
			//проверка на пустое значение:
			bool isEmpty() const { return val == EmptyType; }
			//проверка корректности значения:
			bool isValid() const;
			//полчение значения:
			int Value() const { return val; }
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
			bool operator>=(int x) const { return val >= x; }
			void clear() { val = min_val; }
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
		TExcelCell(int par_row, int par_col) : row(par_row), col(par_col) {}
		//получение индекса строки ячейки:
		int getRow() const { return row.Value(); }
		//получение индекса столбца ячейки:
		int getCol() const { return col.Value(); }
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
		bool operator==(const TExcelCell& x) const  { return row == x.row and col == x.col; }
		bool operator<(const TExcelCell& x) const { return row < x.row and col < x.col; }
		bool operator<=(const TExcelCell& x) const { return *this == x or *this < x; }
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
		enum class TObjType {Row, Col};
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

	//класс страница/лист excel-документа: http://www.libxl.com/spreadsheet.html
	class TExcelBookSheet
	{
	public:
		using TErrorType = libxl::ErrorType;
	private:
		SheetPtr sheet;
		//запрет присвоения и инициализации
		TExcelBookSheet(const TExcelBookSheet& sh);
		TExcelBookSheet& operator=(const TExcelBookSheet& val);
		//инициализация страницы:
		void initSheet(BookPtr book, const string& name, bool active_flg) noexcept(false);
		//вставка строки или столбца:
		bool RangeOperation(const TExcelRange& range, bool insrt, bool asRow, bool updNameRange = true) noexcept(false);
	public:
		//конструктор:
		TExcelBookSheet(BookPtr book, const string& name, bool active_flg = true);
		explicit TExcelBookSheet(SheetPtr sh) : sheet(sh) {}
		//деструктор:
		~TExcelBookSheet() { sheet = nullptr; }
		//валидностьобъекта:
		bool isValid() const { return sheet; }
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
		string ReadAsString(const TExcelCell& cell) noexcept(false);
		//чтение числа:
		double ReadAsNumber(const TExcelCell& cell) const;
		//запись числа в ячейку:
		bool WriteAsNumber(const TExcelCell& cell, double val, FormatPtr format = 0);
		//чтение bool:
		bool ReadAsBool(const TExcelCell& cell) const;
		//запись bool:
		bool WriteAsBool(const TExcelCell& cell, bool val, FormatPtr format = 0) const;
		//считываем формат пустой ячейки, если она пустая
		bool isBlank(const TExcelCell& cell) const;
		//установка формата для пустой ячейкт, еслиячейка не пустая - возвращает false
		bool setBlank(const TExcelCell& cell, FormatPtr format = 0) const;
		//чтение формулы:
		string ReadFormula(const TExcelCell& cell);
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
		bool isDate(const TExcelCell& cell) noexcept(false) { return sheet->isDate(cell.getRow(), cell.getCol()); }
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
		TExcelBookFormat getCellFormat(const TExcelCell& cell) const noexcept(false);
		//установка формата ячейки:
		bool setCellFormat(const TExcelCell& cell, TExcelBookFormat& format) noexcept(false);
		//установка цвета для ячейки
		bool setCellColor(const TExcelCell& cell, const TColor& color) noexcept(true);
		//утсновка цвета для диапазона:
		bool setRangeColor(const TExcelRange& range, const TColor& color) noexcept(true);
		//установка цвета для строки
		bool setRowColor(int row_indx, const TColor& color) noexcept(true);
		//установка цвета для столбца:
		bool setColColor(int col_indx, const TColor& color) noexcept(true);
		//установка цвета текста ячейки:
		bool setCellTextColor(const TExcelCell& cell, const TColor& color) noexcept(true);
		//проверка отображения сетки таблицы:
		bool GreedlinesShowed() const { return sheet->displayGridlines(); }
		//установка отображения сетки:
		void ShowGreedlines(bool flg = true) { if (isValid()) sheet->setDisplayGridlines(flg); }
	};

	//класс Excel-книга:
	class TExcelBook
	{
	private:
		BookPtr book;
		//запрет операции присвоения
		TExcelBook& operator=(const TExcelBook& b);
		//запрет инициализацией этим же объектом:
		TExcelBook(const TExcelBook& b);
		//умолчательная инициализация:
		void CrtBook() { book = xlCreateBook(); }
		enum class LoadType {Full, Sheet, Rows};
		struct TLoadParam { string file; int Indx; int first; int last; string tmp_file; };
		//функция загрузки:
		bool loadFromFile(const TLoadParam& param, const LoadType& lt, bool raise_err) noexcept(false);
	public:
		//конструкторы:
		TExcelBook() { CrtBook(); }
		//деструктор:
		~TExcelBook() { if (isValid()) book->release(); book = nullptr; }
		//валидность объекта:
		bool isValid() const noexcept(true) { return book; }
		//загрузка из файла:
		bool load(const string& file, bool raise_err = false, const string& tmp_file = string()) noexcept(false);
		//загрузка листа из файла:
		bool loadSheetOnly(const string& file, int sheet_indx, 
			bool raise_err = false, const string& tmp_file = string()) noexcept(false);
		//загрузка строк данных из листа файла:
		bool loadSheetRowsOnly(const string& file, int sheet_indx, int first_row, int last_row, 
			bool raise_err = false, const string& tmp_file = string());
		//сохранение данных в файл:
		bool SaveToFile(const string& file, bool use_tmp = false, bool raise_err = false) noexcept(false);
		//загрузка информации о страницах: число и имена страниц - становятся доступны
		bool LoadSheetsInfo(const string& file) noexcept(false) { if (isValid()) return book->loadInfo(file.c_str()); return false; };

	};

	//класс-надстройка для создания интерфейса для работы с excel-файлом
	class TExcel
	{
	private:
		enum DefVal {Empty = -1, MainIndx = 0, TitleIndx = 1,
			FontSize = 10, };
		BookPtr file;//excel-файл
		SheetPtr active_sh;//активный рабочий лист
		//Format* main_frmt;//основной формат
		string name;//имя выходного файла
		//std::vector<TDataType> OrdColumnsType;
		//запрещаем инициализацию и присвоение
		TExcel(const TExcel& exl);
		TExcel& operator=(const TExcel& exl);
		//функция полуения имени выходного файла по умолчанию(чтобы можно было менять на что-то более разумное):
		virtual void setDefOutFileName(void);
		//функция создания шрифта по умолчанию:
		virtual void setDefFont(void) noexcept(false);
		//функция создания формата по умолчанию:
		virtual void setDefFormat(void) noexcept(false);
		//функция установки шрифта для заголовка:
		virtual void setTitleFont(Font* src = 0);
		//функция установки формата для заголовка:
		virtual void setTitleFormat(Format* src = 0);
		//функция очистки данных по excel-файлу
		virtual void clear(void);
		//функция получения первой/последней строки с данными:
		int getUsedRow(Sheet* sh, bool last = false) const;
		//функция получения первой/последней ячейки с данными
		int getUsedCell(Sheet* sh, bool last = false) const;
		//функция заполнения формата полей по шаблону(В шаблоне формат должен быть заполнен корректно):
		//указывается лист и строка заголовка, после которой считываем формат ячеек:
		void FillColumnsFormat(int TitleRow = 1, int ShIndex = DefVal::Empty);
	public:
		//инициализация - как открытие файла, если файла нет - создаем
		//для инициализации используется шаблон отчета,
		//чтобы распознать форматы строк и вставляемые в них значения
		//если файл не указан - вставкаданных идет по умолчанию, как Общий формат
		TExcel(const string& tmp_name = "", const string& out_name = "", bool crt_active_sh = false);
		//деинициализация
		virtual ~TExcel(void) { clear(); };
		//является ли книга шаблоном:
		bool isTemplate(void) const { return file->isTemplate(); };
		//копирование для шрифта
		virtual void copyFont(int index, Font* src);
		//копирование для формата:
		virtual void copyFormat(int index, Format* src);
		//функция получения числа страниц:
		int SheetCount(void) const { return (file ? file->sheetCount() : DefVal::Empty); }
		//функция добавления страницы и делаем ее активной:
		bool AddSheet(const string& name, bool set_as_active = false);
		//функция получения активной страницы:
		int ActiveSheet(void) const { return file ? file->activeSheet() : DefVal::Empty; }
		//функция установки активной страницы:
		void setActiveSheet(int index);
		//функция получения первой заполненой строки:
		int getFirstUsedfRow(int SheetIndex) const { return getUsedRow(active_sh, false); };
		//функция получения последней заполненой строки:
		int getLastUsedRow(int SheetIndex) const { return getUsedRow(active_sh, true); };
		//функция получения первой ячейки с данными в строке
		int getFirstUsedCell(int SheetIndex) const { return getUsedCell(active_sh, false); };
		//функция получения последней ячейки с данными в строке
		int getLastUsedCell(int SheetIndex) const { return getUsedCell(active_sh, true); };
		inline bool WriteBool(bool val, int row, int col) { return active_sh->writeBool(row, col, val); };
		inline bool WriteBlank(int row, int col) { return active_sh->writeBlank(row, col, file->format(DefVal::MainIndx)); };
		inline bool WriteNum(double val, int row, int col) { return active_sh->writeNum(row, col, val); };
		inline bool WriteSrt(const string& str, int row, int col) { return active_sh->writeStr(row, col, str.c_str()); };
		//функция чтения данных:
		inline bool ReadBool(int row, int col) const { return active_sh->readBool(row, col); };
		inline double ReadNum(int row, int col) const { return active_sh->readNum(row, col); };
		inline string ReadStr(int row, int col) const { return string(active_sh->readStr(row, col)); };
	};
};
#endif