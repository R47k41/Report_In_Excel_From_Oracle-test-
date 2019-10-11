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

	//��������� - ������ ������ ��� enum:
	const int EmptyType = -1;
	//��������� ��� ������� ������
	enum class TFormatTune {Empty = -1, xlt, xls, xlsx, DefExt, DefName, DefSh};
	//��������� ��� ���� ������:
	enum class TFontType {Italic, Bold, StrikeOut, UnderLine, Script};
	using TScriptFontType = libxl::Script;
	using TUnderLineFontType = libxl::Underline;

	//�������������� ���������� � ������
	string getTuneFormat(const TFormatTune& val);
	//�������� �������� �� ���� ��������:
	bool isTemplate(const string& str);

	//enum class TDataFormat { Default = 0, AsString, AsInteger, AsDate, AsBool };

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
	};
	
	//������ �������������� ��� ������������ excel-���������:
	class TExcelBook;
	class TExcelBookSheet;
	class TExcelBookFont;
	class TExcelBookFormat;
	
	//����� ������ - http://www.libxl.com/font.html
	class TExcelBookFont
	{
	private:
		FontPtr pfont;//��������� �� �����
		//������ ��������
		TExcelBookFont& operator=(const TExcelBookFont& par_fnt);
		void initFont(BookPtr book) noexcept(false);
		bool setFontType(const TFontType& ft);
	public:
		//�����������
//		explicit TExcelBookFont(const TExcelBook& book);
		TExcelBookFont(const TExcelBookFont& par_fnt) : pfont(par_fnt.pfont) {};
		explicit TExcelBookFont(BookPtr book) : pfont(nullptr) { initFont(book); }
		explicit TExcelBookFont(FontPtr par_fnt) : pfont(par_fnt) {}
		//����������
		~TExcelBookFont() { pfont = nullptr; }
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
		FontPtr getRef() { return pfont; }
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
		explicit TExcelBookFormat(BookPtr book) : pformat(nullptr) { initFormat(book); }
		explicit TExcelBookFormat(FormatPtr pf) : pformat(pf) {}
		TExcelBookFormat(const TExcelBookFormat& pf) : pformat(pf.pformat) {};
		//�������� ����������:
		TExcelBookFormat& operator=(const TExcelBookFormat& pf) { if (pf.pformat != pformat) pformat = pf.pformat; return *this; }
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
		TAlignVType getAlignV() const{ return (isValid() ? pformat->alignV() : TAlignVType(EmptyType)); }
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
		TBorderStyle getBorderStyle(const TBorderSide& side) const noexcept(false);
		//��������� ����� ������� ������:
		void setBorderColor(TColor c, const TBorderSide& side);
		//��������� ����� ����� �������
		TColor getBorderColor(const TBorderSide& side) const noexcept(false);
		//������������ �����:
		void setDiagonalBorder(TDiagonal val) noexcept(false) { if (isValid()) pformat->setBorderDiagonal(val); }
		TDiagonal getDiagonalBorder() const noexcept(false) { return pformat->borderDiagonal(); }
		//����������� �������:
		void setPatternFill(TFill val) noexcept(false) { if (isValid()) pformat->setFillPattern(val); }
		TFill getPatternFill() const noexcept(false) { return pformat->fillPattern(); }
		//��������� ������ �� ������:
		FormatPtr getRef() { return pformat; }
	};

	//������� ��������������� ������
	class TBaseObj
	{
	private:
		const int min_val;
		const int max_val;
		int val;//��������
	public:
			explicit TBaseObj(int par_val, int par_min = 0, int par_max = 0);
			//�������� �� ������ ��������:
			bool isEmpty() const { return val == EmptyType; }
			//�������� ������������ ��������:
			bool isValid() const;
			//�������� ��������:
			int Value() const { return val; }
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
			bool operator>=(int x) const { return val >= x; }
			void clear() { val = min_val; }
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
		TExcelCell(int par_row, int par_col) : row(par_row), col(par_col) {}
		//��������� ������� ������ ������:
		int getRow() const { return row.Value(); }
		//��������� ������� ������� ������:
		int getCol() const { return col.Value(); }
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
		bool operator==(const TExcelCell& x) const  { return row == x.row and col == x.col; }
		bool operator<(const TExcelCell& x) const { return row < x.row and col < x.col; }
		bool operator<=(const TExcelCell& x) const { return *this == x or *this < x; }
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
		enum class TObjType {Row, Col};
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

	//����� ��������/���� excel-���������: http://www.libxl.com/spreadsheet.html
	class TExcelBookSheet
	{
	public:
		using TErrorType = libxl::ErrorType;
	private:
		SheetPtr sheet;
		//������ ���������� � �������������
		TExcelBookSheet(const TExcelBookSheet& sh);
		TExcelBookSheet& operator=(const TExcelBookSheet& val);
		//������������� ��������:
		void initSheet(BookPtr book, const string& name, bool active_flg) noexcept(false);
		//������� ������ ��� �������:
		bool RangeOperation(const TExcelRange& range, bool insrt, bool asRow, bool updNameRange = true) noexcept(false);
	public:
		//�����������:
		TExcelBookSheet(BookPtr book, const string& name, bool active_flg = true);
		explicit TExcelBookSheet(SheetPtr sh) : sheet(sh) {}
		//����������:
		~TExcelBookSheet() { sheet = nullptr; }
		//�����������������:
		bool isValid() const { return sheet; }
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
		string ReadAsString(const TExcelCell& cell) noexcept(false);
		//������ �����:
		double ReadAsNumber(const TExcelCell& cell) const;
		//������ ����� � ������:
		bool WriteAsNumber(const TExcelCell& cell, double val, FormatPtr format = 0);
		//������ bool:
		bool ReadAsBool(const TExcelCell& cell) const;
		//������ bool:
		bool WriteAsBool(const TExcelCell& cell, bool val, FormatPtr format = 0) const;
		//��������� ������ ������ ������, ���� ��� ������
		bool isBlank(const TExcelCell& cell) const;
		//��������� ������� ��� ������ ������, ���������� �� ������ - ���������� false
		bool setBlank(const TExcelCell& cell, FormatPtr format = 0) const;
		//������ �������:
		string ReadFormula(const TExcelCell& cell);
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
		bool isDate(const TExcelCell& cell) noexcept(false) { return sheet->isDate(cell.getRow(), cell.getCol()); }
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
		TExcelBookFormat getCellFormat(const TExcelCell& cell) const noexcept(false);
		//��������� ������� ������:
		bool setCellFormat(const TExcelCell& cell, TExcelBookFormat& format) noexcept(false);
		//��������� ����� ��� ������
		bool setCellColor(const TExcelCell& cell, const TColor& color) noexcept(true);
		//�������� ����� ��� ���������:
		bool setRangeColor(const TExcelRange& range, const TColor& color) noexcept(true);
		//��������� ����� ��� ������
		bool setRowColor(int row_indx, const TColor& color) noexcept(true);
		//��������� ����� ��� �������:
		bool setColColor(int col_indx, const TColor& color) noexcept(true);
		//��������� ����� ������ ������:
		bool setCellTextColor(const TExcelCell& cell, const TColor& color) noexcept(true);
		//�������� ����������� ����� �������:
		bool GreedlinesShowed() const { return sheet->displayGridlines(); }
		//��������� ����������� �����:
		void ShowGreedlines(bool flg = true) { if (isValid()) sheet->setDisplayGridlines(flg); }
	};

	//����� Excel-�����:
	class TExcelBook
	{
	private:
		BookPtr book;
		//������ �������� ����������
		TExcelBook& operator=(const TExcelBook& b);
		//������ �������������� ���� �� ��������:
		TExcelBook(const TExcelBook& b);
		//������������� �������������:
		void CrtBook() { book = xlCreateBook(); }
		enum class LoadType {Full, Sheet, Rows};
		struct TLoadParam { string file; int Indx; int first; int last; string tmp_file; };
		//������� ��������:
		bool loadFromFile(const TLoadParam& param, const LoadType& lt, bool raise_err) noexcept(false);
	public:
		//������������:
		TExcelBook() { CrtBook(); }
		//����������:
		~TExcelBook() { if (isValid()) book->release(); book = nullptr; }
		//���������� �������:
		bool isValid() const noexcept(true) { return book; }
		//�������� �� �����:
		bool load(const string& file, bool raise_err = false, const string& tmp_file = string()) noexcept(false);
		//�������� ����� �� �����:
		bool loadSheetOnly(const string& file, int sheet_indx, 
			bool raise_err = false, const string& tmp_file = string()) noexcept(false);
		//�������� ����� ������ �� ����� �����:
		bool loadSheetRowsOnly(const string& file, int sheet_indx, int first_row, int last_row, 
			bool raise_err = false, const string& tmp_file = string());
		//���������� ������ � ����:
		bool SaveToFile(const string& file, bool use_tmp = false, bool raise_err = false) noexcept(false);
		//�������� ���������� � ���������: ����� � ����� ������� - ���������� ��������
		bool LoadSheetsInfo(const string& file) noexcept(false) { if (isValid()) return book->loadInfo(file.c_str()); return false; };

	};

	//�����-���������� ��� �������� ���������� ��� ������ � excel-������
	class TExcel
	{
	private:
		enum DefVal {Empty = -1, MainIndx = 0, TitleIndx = 1,
			FontSize = 10, };
		BookPtr file;//excel-����
		SheetPtr active_sh;//�������� ������� ����
		//Format* main_frmt;//�������� ������
		string name;//��� ��������� �����
		//std::vector<TDataType> OrdColumnsType;
		//��������� ������������� � ����������
		TExcel(const TExcel& exl);
		TExcel& operator=(const TExcel& exl);
		//������� �������� ����� ��������� ����� �� ���������(����� ����� ���� ������ �� ���-�� ����� ��������):
		virtual void setDefOutFileName(void);
		//������� �������� ������ �� ���������:
		virtual void setDefFont(void) noexcept(false);
		//������� �������� ������� �� ���������:
		virtual void setDefFormat(void) noexcept(false);
		//������� ��������� ������ ��� ���������:
		virtual void setTitleFont(Font* src = 0);
		//������� ��������� ������� ��� ���������:
		virtual void setTitleFormat(Format* src = 0);
		//������� ������� ������ �� excel-�����
		virtual void clear(void);
		//������� ��������� ������/��������� ������ � �������:
		int getUsedRow(Sheet* sh, bool last = false) const;
		//������� ��������� ������/��������� ������ � �������
		int getUsedCell(Sheet* sh, bool last = false) const;
		//������� ���������� ������� ����� �� �������(� ������� ������ ������ ���� �������� ���������):
		//����������� ���� � ������ ���������, ����� ������� ��������� ������ �����:
		void FillColumnsFormat(int TitleRow = 1, int ShIndex = DefVal::Empty);
	public:
		//������������� - ��� �������� �����, ���� ����� ��� - �������
		//��� ������������� ������������ ������ ������,
		//����� ���������� ������� ����� � ����������� � ��� ��������
		//���� ���� �� ������ - ������������� ���� �� ���������, ��� ����� ������
		TExcel(const string& tmp_name = "", const string& out_name = "", bool crt_active_sh = false);
		//���������������
		virtual ~TExcel(void) { clear(); };
		//�������� �� ����� ��������:
		bool isTemplate(void) const { return file->isTemplate(); };
		//����������� ��� ������
		virtual void copyFont(int index, Font* src);
		//����������� ��� �������:
		virtual void copyFormat(int index, Format* src);
		//������� ��������� ����� �������:
		int SheetCount(void) const { return (file ? file->sheetCount() : DefVal::Empty); }
		//������� ���������� �������� � ������ �� ��������:
		bool AddSheet(const string& name, bool set_as_active = false);
		//������� ��������� �������� ��������:
		int ActiveSheet(void) const { return file ? file->activeSheet() : DefVal::Empty; }
		//������� ��������� �������� ��������:
		void setActiveSheet(int index);
		//������� ��������� ������ ���������� ������:
		int getFirstUsedfRow(int SheetIndex) const { return getUsedRow(active_sh, false); };
		//������� ��������� ��������� ���������� ������:
		int getLastUsedRow(int SheetIndex) const { return getUsedRow(active_sh, true); };
		//������� ��������� ������ ������ � ������� � ������
		int getFirstUsedCell(int SheetIndex) const { return getUsedCell(active_sh, false); };
		//������� ��������� ��������� ������ � ������� � ������
		int getLastUsedCell(int SheetIndex) const { return getUsedCell(active_sh, true); };
		inline bool WriteBool(bool val, int row, int col) { return active_sh->writeBool(row, col, val); };
		inline bool WriteBlank(int row, int col) { return active_sh->writeBlank(row, col, file->format(DefVal::MainIndx)); };
		inline bool WriteNum(double val, int row, int col) { return active_sh->writeNum(row, col, val); };
		inline bool WriteSrt(const string& str, int row, int col) { return active_sh->writeStr(row, col, str.c_str()); };
		//������� ������ ������:
		inline bool ReadBool(int row, int col) const { return active_sh->readBool(row, col); };
		inline double ReadNum(int row, int col) const { return active_sh->readNum(row, col); };
		inline string ReadStr(int row, int col) const { return string(active_sh->readStr(row, col)); };
	};
};
#endif