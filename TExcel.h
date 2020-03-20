#ifndef TEXCEL_H_
#define TEXCEL_H_
#include <string>
#include <vector>
//#include <map>
#include <set>
#include "libxl.h"
#include "TConstants.h"

//����������� ��� excel �������:
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

	//��������� ����
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
		//������� �������� �� �������:
		bool isEmpty() const noexcept(true);
		//������� �������������� � ������(������ ��� ���������� boost::date_time):
		std::string toStr(const std::string& format = "%d.%m.%Y") const noexcept(true);
		static string toStr(double dbl_date, const string& format = "%d.%m.%Y") noexcept(true);
	};

	//��������������� ��� ������������ ������:
	class TExcelParam
	{
	private:
		string template_file;//��� ����� �������
		string out_name;//��� ��������� �����
		TStrArr header;//������������ ������� ��� ��������� � ������� ����������
	public:
		TExcelParam(const string& par_tmpl_name, const string& par_out_file, const TStrArr& cols) :
			template_file(par_tmpl_name), out_name(par_out_file), header(cols) {}
		//�������� �� �������
		bool isEmpty() const { return (template_file.empty() or (out_name.empty() and header.empty())); }
		//��������� ��������:
		//��� ����� �������:
		string getTemplateName() const { return template_file; }
		//��� ��������� �����:
		string getOutName() const { return out_name; }
		//������ ������� ���������:
		TStrArr getHeader() const { return header; }
		//������������ ������� �� �������:
		string getColumnName(int indx) const noexcept(true);
		//���������� �������:
		int ColumnsCnt() const { return header.size(); }
		//��������� ���������� �����
		static string getExtensionFile(const string& srt) noexcept(true);
	};

	//������� ��������������� ������
	class TBaseObj
	{
	private:
		bool from_zero;//������ ������� �� 0 ����� �� 1
		int val;//��������
	public:
		explicit TBaseObj(int par_val, bool zero_flg = true);
		//�������� �� ������ ��������:
		bool isEmpty() const { return val < 0; }
		//�������� ������������ ��������:
		bool isValid() const;
		//�������� ��������:
		int Value(bool frm_zero = true) const { return frm_zero ? val : val + 1; }
		//��������� �������� ���������� ��������:
		int Next();
		//��������� �������� ����������� �������:
		int Prev();
		//�������������� � int
		operator int() { return val; }
		//�������� ����������:
		TBaseObj& operator=(const TBaseObj& v);
		TBaseObj& operator=(int x);
		//���������
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

	//��������������� ����� ������:
	class TExcelCell
	{
	private:
		TBaseObj row;//������
		TBaseObj col;//�������
	public:
		//�����������
		TExcelCell(const TBaseObj& par_row, const TBaseObj& par_col) : row(par_row), col(par_col) {}
		TExcelCell(int par_row, int par_col, bool from_zero = true) : row(par_row, from_zero), col(par_col, from_zero) {}
		//��������� ������� ������ ������:
		int getRow(bool frm_zero = true) const { return row.Value(frm_zero); }
		//��������� ������� ������� ������:
		int getCol(bool frm_zero = true) const { return col.Value(frm_zero); }
		//���������� �������:
		bool isValid() const { return row.isValid() and col.isValid(); }
		//������ ������:
		bool isEmpty() const { return row.isEmpty() or col.isEmpty(); }
		//��������� ��������� ������ � ������
		int getNextRowCell() { return row.Next(); }
		//��������� ������ �� ���������� ������ � ������:
		int getPrevRowCell() { return row.Prev(); }
		//��������� ��������� ������ � �������
		int getNextColCell() { return col.Next(); }
		//��������� ������ �� ���������� ������ � �������:
		int getPrevColCell() { return col.Prev(); }
		//��������� ���������:
		bool operator==(const TExcelCell& x) const { return row == x.row and col == x.col; }
		bool operator<(const TExcelCell& x) const { return row < x.row and col < x.col; }
		bool operator<=(const TExcelCell& x) const { return row <= x.row and col <= x.col; }
		void clear() { row.clear(); col.clear(); }
		bool isZero() const { return row.isZero() and col.isZero(); }
		string getName() const { return row.getName(true) + col.getName(false); }
	};

	//����� ��������:
	class TExcelRange
	{
	private:
		TExcelCell first;
		TExcelCell last;
		enum class TObjType { Row, Col };
		bool inRange(int val, const TObjType& t) const;
	public:
		//������������:
		TExcelRange(int first_row, int first_col, int last_row, int last_col) :
			first(first_row, first_col), last(last_col, last_col) {}
		TExcelRange(const TExcelCell& first_cell, const TExcelCell& last_cell) :
			first(first_cell), last(last_cell) {}
		//�������� �� ����������
		bool isValid() const { return first.isValid() and last.isValid() and first <= last; }
		//�������� �� �������:
		bool isEmpty() const { return first.isEmpty() or last.isEmpty(); }
		//�������� ������� � ���������:
		bool inRange(const TExcelCell& cell) const;
		TExcelCell getFirst() const noexcept(false) { return first; }
		TExcelCell getLast() const noexcept(false) { return last; }
		void clear() { first.clear(); last.clear(); }
		string getName() const { return first.getName() + ":" + last.getName(); }
		TExcelCell operator()(int row_indx, int col_indx) const;
	};

	//������ �������������� ��� ������������ excel-���������:

	class TExcelBook;
	class TExcelBookSheet;
	class TExcelBookFormat;
	class TExcelBookFont;

	//����� Excel-����� - http://www.libxl.com/workbook.html
	class TExcelBook
	{
	public:
		//������ ������ ��������� - �.�. ���������� ��� ������ � excel ��������� ���� ������:
		using TType = libxl::SheetType;
		//libxl::SheetState;
		//��������� ������ ��������
		static bool UseLicenseKey(BookPtr* b) noexcept(true);
	private:
		string fname;//������������ ����� �����
		int HeaderRow;//������ ������ ���������
		BookPtr book;//������ �� �����
		//��������������� ���������
		size_t max_cols;//������������ ����� ������� � �����
		size_t max_rows;//������������ ����� ����� � �����
		PFormatArr frmat_arr;//������ �������� ������ �����:
		//��������� ����������� ��� �����:
		void setConstraints() noexcept(true);
		//������ �������� ����������
		TExcelBook& operator=(const TExcelBook& b);
		//������ �������������� ���� �� ��������:
		TExcelBook(const TExcelBook& b);
		//������� ������������ ������� ��������:
		void InitFormatArr() noexcept(true);
		//������������� �������������:
		void CrtBook(int header_row = 0) noexcept(true);
		void InitBook(BookPtr* b);
		enum class LoadType { Full, Sheet, Rows };
		struct TLoadParam { string file; int Indx; int first; int last; string tmp_file; };
		//������� ��������:
		bool loadFromFile(const TLoadParam& param, const LoadType& lt, bool raise_err) noexcept(false);
		//������������ ��� ����� �� ���������:
		string getDefaultSheetName() const noexcept(true);
		//������� �������� ������� ��������� �������� � �����:
		bool checkUniqSheetName(const string& name) const noexcept(false);
		//���������� ������� � �����(��� ��������):
		FormatPtr InsertFormat(FormatPtr initFormat = nullptr) noexcept(true);
		//������� ���������� ������� - ���������� ����� �� ������
		NS_Excel::FormatPtr AddFormatPtr(FormatPtr initFormat = nullptr, bool use_check = true) noexcept(false);
	public:
		//������������:
		explicit TExcelBook(const string& book_name, int header_row = 0): fname(book_name), 
			HeaderRow(header_row) { CrtBook(); }
		//����������:
		~TExcelBook() { close(); }
		//��������� �� ������:
		string getError() const noexcept(true);
		//���������� �������:
		bool isValid(bool raise_err = false) const noexcept(false);
		//�������� �� �������:
		bool isEmpty() const noexcept(true) { return book ? !SheetCount() : true; }
		//��������� ��������� �� ������� �����:
		void setHeaderByStrArr(const TStrArr& arr, bool use_active_sheet = true, const string& new_sh_name = "") noexcept(true);
		//������������� ����� �� ����������
		bool initByParam(const TExcelParam& param) noexcept(true);
		//�������� �� �����:
		bool load(const string& file, bool raise_err = false, const string& tmp_file = string()) noexcept(false);
		//�������� ����� �� ����� ��� ��������� ������ � ����� ���������!!!:
		bool loadSheetOnly(const string& file, int sheet_indx,
			bool raise_err = false, const string& tmp_file = string()) noexcept(false);
		//������� �������� �� �����:
		bool setSheetByTemplate(const string& file, const string& new_sh_name, int templ_sheet_indx = 0,
			bool as_active = false, const string & tmp_file = string()) noexcept(false);
		//�������� ����� ������ �� ����� �����:
		bool loadSheetRowsOnly(const string& file, int sheet_indx, int first_row, int last_row,
			bool raise_err = false, const string& tmp_file = string());
		//���������� ������ � ����:
		bool SaveToFile(const string& file = "", bool use_tmp = false, bool raise_err = false) noexcept(false);
		//�������� ���������� � ���������: ����� � ����� ������� - ���������� ��������
		bool LoadSheetsInfo(const string& file) noexcept(false) { if (isValid()) return book->loadInfo(file.c_str()); return false; };
		//���������� ��������:
		TExcelBookSheet AddSheet(const string& name = "", bool asActive = false) noexcept(false);
		//������� ��������:
		TExcelBookSheet InsertSheet(int index, const string& name, SheetPtr initSheet = nullptr) noexcept(false);
		//��������� ��������� �� �������:
		TExcelBookSheet getSheetByIndex(int index) const noexcept(false);
		//��������� ������ �� ��������� ��������:
		TExcelBookSheet getLastSheet(bool set_active) noexcept(false);
		//��������� ����� �������� �� �������:
		string getSheetNameByIndex(int index) const noexcept(false);
		//��������� ���� ��������:
		TType getSheetTypeByIndex(int indx) const noexcept(false) { return book->sheetType(indx); };
		//����������� ��������:
		bool MoveSheetByIndex(int old_indx, int new_indx) noexcept(false);
		//�������� ��������:
		bool DelSheetByIndex(int index) noexcept(false);
		//��������� ����� ������� � ������� �����:
		int SheetCount() const noexcept(false) { return book->sheetCount(); };
		//���������� ������� � �����:
		TExcelBookFormat AddFormat(TExcelBookFormat& initFormat, bool use_check = true) noexcept(false);
		//������� ��������� ������ �� ������ �� �������:
		FormatPtr getFormatPrtByIndex(size_t index) const noexcept(false);
		//��������� ������� �� �������:
		TExcelBookFormat getFormatByIndex(int index) noexcept(false);
		//��������� ������� ��� ������� �����:
		size_t getFormatIndex(const TExcelBookFormat& format) const noexcept(false);
		//���������� �������� � �����:
		int FormatCount() const noexcept(true) { return (isValid() ? book->formatSize() : EmptyType); }
		//���������� ������ � �����:
		TExcelBookFont AddFont(FontPtr initFont = nullptr) noexcept(false);
		//��������� ������ �� ����� �� �������:
		TExcelBookFont getFontByIndex(int index) noexcept(false);
		//����� ������� � �����:
		int getFontCount() const noexcept(true) { return (isValid() ? book->fontSize() : EmptyType); }
		//������ �������� ��������:
		int getActiveSheetIndx() const noexcept(false) { return (isValid() ? book->activeSheet() : EmptyType); }
		TExcelBookSheet getActiveSheet() const noexcept(false);
		//���������� �������� ��������:
		bool setActiveSheet(int index) noexcept(true);
		//�������� �������� �� ����� - ��������:
		bool isTemplate() const noexcept(true) { return (isValid() ? book->isTemplate() : false); }
		//��������� ����� � �������� �������:
		bool setAsTemplate(bool flg = true) noexcept(true);
		//��������� ������:
		bool setLocale(const string& locale) noexcept(true);
		//������������ �� ������ ��� ������ ����: RxxCxx
		bool isRxCxRef() const noexcept(true) { return (isValid() ? book->refR1C1() : false); }
		//���������� ������ ��� ������ ���� RxxCxx:
		bool setRxCxRef(bool flg = true) noexcept(true);
		//������ � ������ �� ���������:
		void DefultFont(string& font_name, int& size) noexcept(false);
		//���������� ����� �� ���������:
		bool setDefaultFont(const string& font_name, int size) noexcept(true);
		//�������� ���� � ��� double:
		double Date2Double(const TExcelDate& date) noexcept(false);
		//������������� double � ���� ����:
		bool Double2Date(double value, TExcelDate& date) noexcept(false);
		//������ ���������:
		int getHeaderRow() const noexcept(true) { return HeaderRow; }
		//������� ��������� ����� �����:
		string getFileName() const noexcept(true) { return fname; }
		//������� ��������� ���������� �����:
		//���� ������ - ������ ������ ������
		string getFileExtend() const noexcept(true);
		//������� ��������� ������������� ����� �����:
		size_t MaxRowsCount() const { return max_rows; }
		//������� ��������� ������������� ����� ��������:
		size_t MaxColsCount() const { return max_cols; }
		//�������� �����:
		void close() noexcept(false);
	};

	//����� ��������/���� excel-���������: http://www.libxl.com/spreadsheet.html
	class TExcelBookSheet
	{
	public:
		using TErrorType = libxl::ErrorType;
	private:
		SheetPtr sheet;
		//������������� ��������:
		void initSheet(BookPtr book, const string& name, bool active_flg) noexcept(false);
		//������� ������ ��� �������:
		bool RangeOperation(const TExcelRange& range, bool insrt, bool asRow, bool updNameRange = true) noexcept(false);
		//���������� ��� ����������� �����:
		//����������� ���������� ����� � ������� �����
		bool copySheetColsParam(const TExcelBookSheet& src_sh) noexcept(true);
		//��������� ����������� ����� �� ��������� �����:
		bool copySheetCellsMerge(TExcelBookSheet& src_sh, const TExcelCell& cell) noexcept (true);
		//����������� �������� ������ �� �����-���������:
		bool copySheetCellValue(TExcelBookSheet& src_sh, const TExcelCell& cell, FormatPtr format) noexcept(true);
	public:
		//�����������:
		TExcelBookSheet(BookPtr book, const string& name, bool active_flg = true);
		TExcelBookSheet(const TExcelBookSheet& sh) : sheet(sh.sheet) {};
		explicit TExcelBookSheet(SheetPtr sh) : sheet(sh) {}
		//����������:
		~TExcelBookSheet() { sheet = nullptr; }
		//������������
		TExcelBookSheet& operator=(const SheetPtr sh) { if (sheet != sh) sheet = sh; return *this; }
		TExcelBookSheet& operator=(const TExcelBookSheet& sh) { return operator=(sh.sheet); }
		//�����������������:
		bool isValid() const { return sheet; }
		//�������� ������ �� ������ ��������:
		bool isEmptyCell(const TExcelCell& cell) const noexcept(false);
		//��������� ������ �� ��������/����:
		SheetPtr getRef() { return sheet; }
		//��������� ���� ������ � ������:
		TDataType getCellType(const TExcelCell& cell) const noexcept(false) { return sheet->cellType(cell.getRow(), cell.getCol()); }
		//����� ������ �������������� ������
		int getFirstRow() const { return sheet->firstRow(); }
		//����� ��������� �������������� ������:
		int getLastRow() const { return sheet->lastRow(); }
		//����� ������� �������������� �������:
		int getFirstCol() const { return sheet->firstCol(); }
		//����� ���������� ��������������� �������:
		int getLastCol() const { return sheet->lastCol(); }
		//������ ������ � ������ � ��������� ������� � ���� ������:
		bool WriteAsString(const TExcelCell& cell, const string& val, FormatPtr format = nullptr, const TDataType& type = TDataType::CELLTYPE_STRING);
		//������ ������ �� ������, ������ �� �����������:
		std::string ReadAsString(const TExcelCell& cell, FormatPtr format = nullptr) const noexcept(false);
		//������ �����:
		double ReadAsNumber(const TExcelCell& cell, const FormatPtr format = nullptr) const;
		//������ ����� � ������:
		bool WriteAsNumber(const TExcelCell& cell, double val, FormatPtr format = nullptr);
		//������ bool:
		bool ReadAsBool(const TExcelCell& cell, const FormatPtr format = nullptr) const;
		//������ bool:
		bool WriteAsBool(const TExcelCell& cell, bool val, FormatPtr format = nullptr) const;
		//��������� ������ ������ ������, ���� ��� ������
		bool isBlank(const TExcelCell& cell) const { FormatPtr tmp = nullptr; return ReadBlankFormat(cell, &tmp); }
		bool ReadBlankFormat(const TExcelCell& cell, FormatPtr* format) const { return sheet->readBlank(cell.getRow(), cell.getCol(), format); };
		//��������� ������� ��� ������ ������, ���������� �� ������ - ���������� false
		bool setBlank(const TExcelCell& cell, FormatPtr format = 0) const;
		//������ �������:
		std::string ReadFormula(const TExcelCell& cell);
		//������ � ������ �������:
		bool WirteFormula(const TExcelCell& cell, const string& formula, FormatPtr format = 0);
		//������ � ������ ������� � ������������� ��������� �� ��������� � ���� �����:
		bool WirteFormulaNumAsDef(const TExcelCell& cell, const string& formula, double def_val, FormatPtr format = 0);
		//������ � ������ ������� � ������������� ��������� �� ��������� � ���� ������:
		bool WirteFormulaStrAsDef(const TExcelCell& cell, const string& formula, const string& def_val, FormatPtr format = 0);
		//������ � ������ ������� � ������������� ��������� �� ��������� � ���� bool:
		bool WirteFormulaBoolAsDef(const TExcelCell& cell, const string& formula, bool def_val, FormatPtr format = 0);
		//������� �� � ������ �������:
		bool isFormula(const TExcelCell& cell) const noexcept(false) { return sheet->isFormula(cell.getRow(), cell.getCol()); }
		//������ ���������� �� ������:
		string getComment(const TExcelCell& cell) const noexcept(false);
		//������ ���������� � ������:
		bool setComment(const TExcelCell& cell, const string& val, const string& author = "", int width = 129, int height = 75) noexcept(false);
		//�������� ���������� �� ������:
		bool delComment(const TExcelCell& cell) noexcept(false);
		//�������� �������� �� �������� � ������ �����
		bool isDate(const TExcelCell& cell) const noexcept(false) { return sheet->isDate(cell.getRow(), cell.getCol()); }
		//������ ���� ������ � ������:
		TErrorType getErrorType(const TExcelCell& cell) const { return sheet->readError(cell.getRow(), cell.getCol()); }
		//������ � ������ ������:
		bool setErrorType(const TExcelCell& cell, const TErrorType& er, FormatPtr format = 0);
		//������ �������
		double getColWidth(int col_indx) const noexcept { return sheet->colWidth(col_indx); }
		//������ ������:
		double getRowHeight(int row_indx) const noexcept(false) { return sheet->rowHeight(row_indx); }
		//��������� ������ ��� ���������:
		bool setColsParams(const TExcelRange& r, double width, bool hide = false, FormatPtr format = nullptr) noexcept(true);
		//��������� ������ ��� ���������:
		bool setRowParams(int row, double height, bool hide = false, FormatPtr format = nullptr) noexcept(false) { return sheet->setRow(row, height, format, hide); };
		//��������� ������ �� ������:
		bool isRowHide(int row_indx) const noexcept(false) { return sheet->rowHidden(row_indx); }
		//�������� ������:
		bool setRowHide(int row_indx, bool hide);
		//��������� ����� �� �������:
		bool isColHide(int col_indx) const noexcept(false) { return sheet->colHidden(col_indx); }
		//�������� �������:
		bool setColHide(int col_indx, bool hide);
		//������ � ������������� �����:
		//��������� �������� �� ������ � ������� ����������� - �� ������ ������� � ������� �������� ������:
		bool inMergeRange(const TExcelCell& cell, TExcelRange& result_range) noexcept(true);
		//����������� ����� �� ���������:
		bool setMergeRange(const TExcelRange& range) noexcept(true);
		//������� ������ �� �����������:
		bool delMerge(const TExcelCell& cell) noexcept(true);
		//���������� ����������� ����� �� �����:
		int getMergeSize() const noexcept(false) { return sheet->mergeSize(); }
		//������� �������� ������������ ����� �� �������:
		bool getMergeRangeByIndx(int indx, TExcelRange& result_range) noexcept(true);
		//�������� ����������� ����� �� �������:
		bool delMergeByIndx(int indx) noexcept(true) { return (isValid() ? sheet->delMergeByIndex(indx) : false); };
		//������:
		//����������
		bool setSplit(const TExcelCell& cell);
		//�������� ������ �� �������
		bool getSplit(TExcelCell& cell) const;
		//������������ �� ������(������ ��� xls???)
		bool clear(const TExcelRange& range) noexcept(true);
		//������� �����:
		bool insRows(const TExcelRange& range, bool updNameRange = true) noexcept(false) { return RangeOperation(range, true, true, updNameRange); }
		//������� ��������:
		bool insCols(const TExcelRange& range, bool updNameRange = true) noexcept(false) { return RangeOperation(range, true, false, updNameRange); }
		//�������� �����:
		bool delRows(const TExcelRange& range, bool updNameRange = true) noexcept(false) { return RangeOperation(range, false, true, updNameRange); }
		//�������� ��������:
		bool delCols(const TExcelRange& range, bool updNameRange = true) noexcept(false) { return RangeOperation(range, false, false, updNameRange); }
		//����������� ������
		bool copyCell(const TExcelCell& src, const TExcelCell& dst) noexcept(false) { return sheet->copyCell(src.getRow(), src.getCol(), dst.getRow(), dst.getCol()); }
		//���������� ��������:
		bool isLandScape() const noexcept(false) { return sheet->landscape(); }
		//��������� ���������� ��������:
		void setLandScape(bool landscape = true) noexcept(false) { sheet->setLandscape(landscape); }
		//��������� ������������ �����:
		string getName() const;
		//��������� ����� ��������
		void setName(const string& str) noexcept(false) { sheet->setName(str.c_str()); }
		//��������� ������� ������:
		FormatPtr getCellFormatPtr(const TExcelCell& cell) const noexcept(false);
		//��������� ������� ������:
		TExcelBookFormat getCellFormat(const TExcelCell& cell) const noexcept(false);
		//��������� ������� ������:
		bool setCellFormat(const TExcelCell& cell, TExcelBookFormat& format) noexcept(false);
		//��������� ����� ��� ������� � ��������� ������!!!!
		bool setCellColor(const TExcelCell& cell, const TColor& color) noexcept(true);
		//�������� ����� ��� ������� � ���������:
		bool setRangeColor(const TExcelRange& range, const TColor& color) noexcept(true);
		//��������� ����� ��� ������� � ������
		bool setRowColor(int row_indx, const TColor& color) noexcept(true);
		//��������� ����� ��� ������� � �������:
		bool setColColor(int col_indx, const TColor& color) noexcept(true);
		//��������� ����� ������ ������:
		bool setCellTextColor(const TExcelCell& cell, const TColor& color) noexcept(true);
		//�������� ����������� ����� �������:
		bool GreedlinesShowed() const { return sheet->displayGridlines(); }
		//��������� ����������� �����:
		void ShowGreedlines(bool flg = true) { if (isValid()) sheet->setDisplayGridlines(flg); }
		//������������� �������:
		friend bool TExcelBook::setSheetByTemplate(const string& file, const string& new_sh_name, int templ_sheet_indx,
			bool raise_err, const string & tmp_file) noexcept(false);
	};
	
	//����� ������� - http://www.libxl.com/format.html
	class TExcelBookFormat
	{
	public:
		using TNumFormat = libxl::NumFormat;
		using TAlignHType = libxl::AlignH;
		using TAlignVType = libxl::AlignV;
		using TBorderStyle = libxl::BorderStyle;
		using TDiagonal = libxl::BorderDiagonal;
		using TFill = libxl::FillPattern;
		//����� ����������� ������� ��� ������
		enum class TBorderSide { Full, Left, Right, Top, Bottom, Diagonal, Background, Foreground };
	private:
		FormatPtr pformat;//������ �� ������ �����
		//�������������
		void initFormat(BookPtr book) noexcept(false);
	public:
		//�����������
		//explicit TExcelBookFormat(BookPtr book) : pformat(nullptr) { initFormat(book); }
		explicit TExcelBookFormat(FormatPtr pf) : pformat(pf) {}
		TExcelBookFormat(const TExcelBookFormat& pf) : pformat(pf.pformat) {};
		//�������� ����������:
		TExcelBookFormat& operator=(FormatPtr format) { if (pformat != format) pformat = format; return *this; }
		TExcelBookFormat& operator=(const TExcelBookFormat& pf) { return operator=(pf.pformat); }
		//		explicit TExcelBookFormat(const TExcelBook& book);
		~TExcelBookFormat() { pformat = nullptr; }
		//����������:
		inline bool isValid() const { return pformat; }
		//�����
		TExcelBookFont getFont() const noexcept(false);
		bool setFont(TExcelBookFont& par_fnt) noexcept(false);
		//�������� ������:
		int getNumFormat() const;
		void setNumFormat(TNumFormat val);
		//������������ � �������:
		//��������������:
		TAlignHType getAlignH() const { return (isValid() ? pformat->alignH() : TAlignHType(EmptyType)); }
		void setAlignH(TAlignHType val) { if (isValid()) pformat->setAlignH(val); }
		//������������:
		TAlignVType getAlignV() const { return (isValid() ? pformat->alignV() : TAlignVType(EmptyType)); }
		void setAlignV(TAlignVType val) { if (isValid()) pformat->setAlignV(val); }
		//������� ���������� ������: �.�. ����� ����������� ��� ���������� ������
		bool isWrap() const noexcept(false) { return pformat->wrap(); }
		void setWrap(bool flg = true) { if (isValid()) pformat->setWrap(flg); }
		//�������� ������(���� �������):
		int getRotation() const noexcept(false) { return pformat->rotation(); }
		void setRotation(int par_rotation) { if (isValid()) pformat->setRotation(par_rotation); }
		//������:
		int getIndent() const noexcept(false) { return pformat->indent(); }
		void setIndent(int par_indent) { if (isValid()) pformat->setIndent(par_indent); }
		//���������� ������ �� ������� ������
		bool isShrink2Fit() const noexcept(false) { return pformat->shrinkToFit(); }
		void setSrink2Fit(bool flg = true) { if (isValid()) pformat->setShrinkToFit(flg); }
		//������ ������ ������:
		bool isLocked() const noexcept(false) { return pformat->locked(); }
		void setLocked(bool flag = true) { if (isValid()) pformat->setLocked(flag); }
		//�������� ������ ������/������
		bool isHidden() const noexcept(false) { return pformat->hidden(); }
		void setHidden(bool flg = true) { if (isValid()) pformat->setHidden(flg); }
		//������ � ��������� ������:
		//��������� ������ ������:
		void setBorderStyle(TBorderStyle style, const TBorderSide& side);
		//��������� ����� ������ ������
		TBorderStyle getBorderStyle(const TBorderSide& side) const noexcept(true);
		//��������� ����� ������� ������:
		void setBorderColor(const TColor& c, const TBorderSide& side);
		//��������� ����� ����� �������
		TColor getBorderColor(const TBorderSide& side) const noexcept(false);
		//������������ �����:
		void setDiagonalBorder(TDiagonal val) noexcept(false) { if (isValid()) pformat->setBorderDiagonal(val); }
		TDiagonal getDiagonalBorder() const noexcept(false) { return pformat->borderDiagonal(); }
		//����������� �������:
		void setPatternFill(TFill val) noexcept(false) { if (isValid()) pformat->setFillPattern(val); }
		TFill getPatternFill() const noexcept(false) { return pformat->fillPattern(); }
		//��������� ������ �� ������:
		//FormatPtr getRef() { return pformat; }
		friend bool NS_Excel::TExcelBookSheet::setCellFormat(const TExcelCell& cell, TExcelBookFormat& format) noexcept(false);
		//friend FormatPtr TExcelBook::InsertFormat(const TExcelBookFormat& frmt) noexcept(false);
		friend void TExcelBook::setHeaderByStrArr(const TStrArr& arr, bool use_active_sheet, const string& new_sh_name) noexcept(true);
		friend TExcelBookFormat TExcelBook::AddFormat(TExcelBookFormat& initFormat, bool use_check) noexcept(false);
		friend size_t TExcelBook::getFormatIndex(const TExcelBookFormat& format) const noexcept(false);
	};

	//����� ������ - http://www.libxl.com/font.html
	class TExcelBookFont
	{
	public:
		//��������� ��� ���� ������:
		enum class TFontType { Italic, Bold, StrikeOut, UnderLine, Script };
	private:
		FontPtr pfont;//��������� �� �����
		void initFont(BookPtr book) noexcept(false);
		bool setFontType(const TFontType& ft);
	public:
		//�����������
//		explicit TExcelBookFont(const TExcelBook& book);
		TExcelBookFont(const TExcelBookFont& par_fnt) : pfont(par_fnt.pfont) {};
//		explicit TExcelBookFont(BookPtr book) : pfont(nullptr) { initFont(book); }
		explicit TExcelBookFont(FontPtr par_fnt) : pfont(par_fnt) {}
		//����������
		~TExcelBookFont() { pfont = nullptr; }
		//������������:
		TExcelBookFont& operator=(const FontPtr font) { if (font != pfont) pfont = font; return *this; }
		TExcelBookFont& operator=(const TExcelBookFont& par_fnt) { return operator=(par_fnt.pfont); };
		//���������� �������:
		inline bool isValid() const { return pfont; }
		//������ ������
		int getSize() const { return pfont->size(); }
		void setSize(int val) { pfont->setSize(val); }
		//��������� ���� ������
		void setBold() { setFontType(TFontType::Bold); }
		void setItalic() { setFontType(TFontType::Italic); }
		void setStrikeOut() { setFontType(TFontType::StrikeOut); }
		void setScript(const TScriptFontType& sft);
		void setUnderLine(const TUnderLineFontType& uft);
		bool isFontType(const TFontType& ft) const;
		TScriptFontType isScript() const;
		TUnderLineFontType isUnderLine() const;
		//��������� ����� ������:
		bool setColor(const TColor& c);
		TColor getColor() const;
		//������������ ������:
		bool setName(const string& s);
		string getName() const { return string(pfont->name()); }
		//��������� ������ �� �����:
		//FontPtr getRef() { return pfont; }
		friend bool NS_Excel::TExcelBookFormat::setFont(TExcelBookFont& par_fnt) noexcept(false);
	};
	
};
#endif