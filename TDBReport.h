#ifndef TDBREPORT_H_
#define TDBREPORT_H_
//������ ��� ������������ ������ �� ���� ������
//����� ��������� � excel-��������
//���� ������ - oracle
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
	using TRowFlag = std::pair<size_t, bool>;//������ � ������� �� ���������
	//��� vector
	using TRowsFlag = std::map<size_t, bool>;//������ ����� � ��������� ���������
	using TRowsTNS = std::pair<std::string, TRowsFlag>;
//	using TFillFormat = std::pair<size_t, size_t>;
//	using TFillFrmts = std::vector<TFillFormat>;
//	using TConvertType = std::pair<NS_Oracle::TType, size_t>;

	//��������� ����������� �������������� �������� �������� ����� excel-�����:
	struct TCellFormatIndex
	{
		size_t Current;//������ �������� ������� ������ � excel-�����
		size_t NotFound;//������ ������ ���� ������ �� �������
		size_t Found;//������ ������ ���� ������ �������
		bool InitFlg;//������� ������������� ��������
		//�������������:
		TCellFormatIndex(): Current(), NotFound(), Found(), InitFlg(false) {}
		explicit TCellFormatIndex(size_t par_Curent);
		TCellFormatIndex(const TCellFormatIndex& val): Current(val.Current), NotFound(val.NotFound), 
			Found(val.Found), InitFlg(true) {}
		TCellFormatIndex(size_t par_Curent, size_t par_NotFound, size_t par_Found);
		~TCellFormatIndex() {}
	};

	using TCellFormat = std::pair<size_t, TCellFormatIndex>;//������ �������, ������ �� �������� � �����
	using TCellsFormatArr = std::map<size_t, TCellFormatIndex>;//������ �������, ������ �� �������� � �����

	//����� ����� ����������� �� ���� ��������� � ��
	const int PrefetchRowsCnt = 200;

	//����� ����������� ��� ������������ ������ ��� ��������:
	class TBaseSheetReport
	{
	private:
		enum Tune { DEF_TEMPL_SH_INDX = 0 };
	protected:
		TExcelBook& book;//������ �� excel-����
		TExcelBookSheet sheet;//���� excel-�����(��������� ������ ���� �� 0!!!)
		TCellsFormatArr cells_format_indexs;//������� �������� �����(������ ������� ����� � ������� excel - �� 0!!!)
		//������� ���������� ������� ������ � ������ ��������:
		virtual bool addCurCellFormat(size_t Row, size_t Col) noexcept(true);
		//������� ������������� ������� �������� ��� ����� ������ �� �������:
		virtual bool initRowFormat() noexcept(true);
		//������� �������� ������� �������� ��� �����:
		bool EmptyCellsIndexFormat() const noexcept(true) { return cells_format_indexs.empty(); }
		//������� ��������� ������� ��� ������
		bool setCellFormat(const NS_Excel::TExcelCell& cell, NS_Excel::TExcelBookFormat& format) noexcept(true);
		bool setCellFormat(size_t Row, size_t Column, NS_Excel::TExcelBookFormat& format) noexcept(true);
		//������� ��������� ������ �� ������� ������/�������:
		TCellFormatIndex& getFormatIndexByColl(size_t Column) noexcept(false) { return cells_format_indexs[Column]; }
		//������� ��������� ������ �� ������ ��� ��������� �������:
		NS_Excel::FormatPtr getCellFormatPtr(const NS_Excel::TExcelCell& cell) noexcept(true);
		//������� ��������� ������� ��� ������ ��� �������� �������:
		virtual bool setCellFormatByIndexArr(const NS_Excel::TExcelCell& cell) noexcept(true);
		virtual bool setCellFormatByIndexArr(size_t Row, size_t IndexArr) noexcept(true);
		//������� ���������� ������� ������ �� ������� �� ������� � ����������� ��������:
		virtual bool addFillFormat(size_t init_format_index, const NS_Excel::TColor& color, bool font_flg,
			size_t& AddedFormatIndex) noexcept(true);
		//������� ����������� �������� ����� ��� ������ �� �������:
		bool setRowCellsFormat(size_t Row) noexcept(true);
		//�������� �� ������������� �������� ����� ��������:
		virtual bool NeedNewPage(size_t item_cnt, bool byRows = true) const noexcept(false);
		//������� ��������� ������/��������� ������ ��������:
		virtual size_t getRow(bool first) const noexcept(false) = 0;
		//������� �������� ��������� ����� �� ��������� ��������:
		virtual bool OpenBookSheet(const string& srcName, size_t page) noexcept(true);
		//�������� ����� ������ � ������� ������ ������:
		bool EqualCellsType(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
			const NS_Excel::TExcelCell& srcCell) const noexcept(false);
		//������� ���������� ����� ������ � ���� ������
		bool InsNewRow(size_t curRow, size_t newRow) noexcept(true);
		//������� �������� ������ ������ �� ���������:
		bool setCellColorByFormatIndxArr(const NS_Excel::TExcelCell& cell, bool foundFlg) noexcept(true);
		//������� ������������� �������� ����� �� �������:
		virtual bool InitSheetByTemplate(const string& tmpl_name, const string& sh_name,
			bool set_as_active = true, size_t tmpl_sh_index = Tune::DEF_TEMPL_SH_INDX) noexcept(true);
	public:
		//������������� ������� �� ����� � ������� �� �������� ������ �����:
		explicit TBaseSheetReport(TExcelBook& book_ref, NS_Excel::SheetPtr sheet_ref = nullptr) :
			book(book_ref), sheet(sheet_ref), cells_format_indexs() {}
		TBaseSheetReport(TExcelBook& book_ref, const NS_Excel::TExcelBookSheet& sheet_ref): 
			book(book_ref), sheet(sheet_ref), cells_format_indexs()	{}
		//������������� ����� ��� ������ �������� ������ �����:
		//��������� �������� ���������� �������� ��� ���������:
		TBaseSheetReport(TExcelBook& book_ref, const string& src_file, size_t page_index);
		virtual ~TBaseSheetReport() { /*if (cells_format_indexs.size() > 0) cells_format_indexs.clear();*/ }
		//������� ��������� ��������� ������ ������:
		virtual size_t FirstRow() const noexcept(false) { return getRow(true); }
		//������� ��������� ��������� ������ ������:
		virtual size_t LastRow() const noexcept(false) { return getRow(false); }
		//������� �������� ��������� ������ � ��������:
		virtual bool inRange(size_t row, size_t col) const noexcept(false);
		//������� �������� ������ ������ �� excel-��������
		bool isEmptySheetData() const noexcept(true) { return sheet.getFirstRow() == sheet.getLastRow(); }
		//������� ������� ������� ��������:
		bool EmptyRowFormat() const noexcept(true) { return cells_format_indexs.empty(); }
		//������� �������� ��������:
		bool NoSheet() const noexcept(true) { return !sheet.isValid(); }
		//������� ��������� ����� ��������:
		string getSheetName() const noexcept(true) { return sheet.getName(); }
		//����������� ���� ������ excel � ��� ������ �� NS_Const::DataType
		static NS_Const::DataType convertExcelType(const NS_Excel::TDataType& dt, bool isDate = false) noexcept(true);
		static NS_Excel::TDataType convertDataType(const NS_Const::DataType& dt) noexcept(true);
	};

	//����������� ����� ��� �������������� � excel-������� �������
	class TExtendSheetReport : public TBaseSheetReport
	{
	private:
		size_t rowB;//��������� ������ �������
		size_t rowE;//�������� ������ �������
		size_t colID;//������� ������������� - ���������� ������� � ������� ����� ���� ������
		FilterArr filters;//������ � ���������� �����
		//������������� ����� ��������� �� ��������:
		void InitDstFile(const TShareData& dstFile, size_t page) noexcept(false);
	protected:
		//������� ��������� ������/��������� ������ ��������:
		virtual size_t getRow(bool first) const noexcept(false) { return first ? rowB : rowE; }
		//������� ������ ������ � ��������� ������ c ������� ������� �����:
		bool Compare_Cells(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
			const NS_Excel::TExcelCell& srcCell, const NS_Const::JsonFilterOper& operation) const noexcept(true);
		//������� �������� ������������ ������� ������� ��� ������:
		bool checkByFilter(const NS_Tune::TFilterData& filter, size_t Row) const noexcept(true);
	public:
		//������������� json-������ ��������:
		TExtendSheetReport(TExcelBook& book_ref, const TShareData& file, size_t page);
		//���������������
		virtual ~TExtendSheetReport() { }
		//������� �������� ������� Col_ID:
		virtual bool noColID() const noexcept(true) { return colID == NS_Tune::TIndex::EmptyIndex; }
		//������� �������� ������ �� �������:
		virtual bool isEmptyCell(const NS_Excel::TExcelCell& cell) const noexcept(false);
		//������� �������� ������ �� ColID �� �������:
		virtual bool noDataInColID(size_t Row) const noexcept(false);
		//������ ������� ��������������:
		virtual size_t getColID() const noexcept(true) { return colID; }
		//������� ��������� ������ ������ ���������:
		virtual size_t FirstRow() const noexcept(true);
		//������� ��������� ��������� ������ � ���������:
		virtual size_t LastRow() const noexcept(true);
		//�������� ���������� ������� ���������� ��� ������:
		virtual bool isCorrectFilter(size_t curRow) const noexcept(true);
		//������������ ������� ��������������� ����� ��� ��������� ������ ����������:
		TRowsFlag setFiltredRowsArr() const noexcept(true);
		//������� ������ ������ � ������ ��������� � ������� ������ ���������
		bool CheckInCell(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
			const NS_Excel::TExcelCell& srcCell, bool NoSpaceNoCase = true) const noexcept(true);
		bool CheckInCell(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell, 
			size_t srcRow, size_t srcCol, bool NoSpaceNoCase = true) const noexcept(true);
		//������� ������ ������ � ������ ������ ������� - ���������� ����� ������ ����������(��� ������� �����):
		size_t CheckOnSheet(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
			size_t srcCol, bool NoSpaceNoCase = true) const noexcept(true);
		//������� ������ ������ �� ������ ������ �������� - ���������� ����� ������ ����������(� �������� �����):
		size_t CheckOnSheet(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell, 
			size_t srcCol, TRowsFlag& RowsArr, bool NoSpaceNoCase = true) const noexcept(true);
		//������� �������� ������� ��������� � ������� ��������� � ���������:
		bool NotEquality(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
			const NS_Excel::TExcelCell& srcCell, bool NoCaseNoSpace = false) const noexcept(true);
		//������� ������ ��������� ������ ��������� � ������ ������ ���������:
		bool getDstCell_In_SrcCell(const TExtendSheetReport& srcSheet, const NS_Excel::TExcelCell& DstCell,
			const NS_Excel::TExcelCell& SrcCell, bool NoSpaceNoCase = true) const noexcept(true);
		//������� ��������� ������ ��������� ��� ������ ���������:
		size_t getSrcRow_By_Cell(const TExtendSheetReport& srcSheet, NS_ExcelReport::TRowsFlag& srcRows,
			const NS_Excel::TExcelCell& DstCell, size_t SrcCol, bool NoSpaceNoCase = true) const noexcept(true);
		//������� ������ ������ ��������� �� ���������� ����� ������ ���������:
		size_t getSrcRow_By_Dest_Params(const TExtendSheetReport& srcSheet, NS_ExcelReport::TRowsFlag& srcRows,
			const NS_Tune::CellDataArr& params, size_t curRow, bool NoSpaceNoCase = true) const noexcept(true);
		//������� ��������� ������ ��������� ��� ������ ��������� �� ����������:
		size_t getSrcRow_By_Params(const TExtendSheetReport& srcSheet, NS_ExcelReport::TRowsFlag& srcRows,
			const NS_Tune::CellDataArr& params, size_t curRow, size_t& param_index, bool NoSpaceNoCase = true) const noexcept(true);
		//������� ������� ������ � ������-�������� �� ������-���������:
		virtual bool setDstCellBySrcCell(NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell, 
			const NS_Excel::TExcelCell& srcCell) const noexcept(true);
	};

	//����� ��� ��������� excel-������ �� ��������� json-��������
	class TJsonReport : public TExtendSheetReport
	{
	private:
		string main_path;//���������� ������ ��� ������������ ������
		const TProcCell& cells_data;//���������� �� �������
		NS_Const::JSonMeth meth_code;//����� ��������� ������
		//������� �������� �������������� ������ � ����������:
		bool isParamColumn(size_t Col) const noexcept(true);
		//������� �������� ������������� ��������� ���������:
		bool WithChangeMeth() const noexcept(true) { return meth_code == NS_Const::JSonMeth::CompareCellChange; }
		//������� ��������� ��������� ������� ��� ������:
		void setDQLParamByCell(TStatement& query, const TCellData& value, size_t curRow) const noexcept(false);
		//������� �������� ������� ��������� � �������:
		bool CellHasChanged(const TExtendSheetReport& srcSheet, const NS_Tune::TCellData& param,
			size_t dstRow, size_t srcRow) const noexcept(true);
		//������� ������� ������ � ������ ����� ��������� 
		bool InsertDstCellBySrcCell(const TExtendSheetReport& srcSheet, const NS_Tune::TCellData& param,
			size_t dstRow, size_t srcRow) noexcept(true);
		//������� �������� �������� ���������� ��������� � ���������:
		static bool Inverse_Dst2Src_Param(const NS_Tune::CellDataArr& params, NS_Tune::CellDataArr& new_param) noexcept(false);
	protected:
		//������� ��������� ��������� ��������� ��� �������� ���������/������� oracle:
		static void setDMLOutParam(NS_Oracle::TStatement& query, const TCellData& param) noexcept(false);
		//������� ��������� ���������� ��� ���������:
		bool setStatementParam(NS_Oracle::TStatement& query, const NS_Tune::TCellData& value, size_t Row) const noexcept(true);
		//������� ������������ ������� ��� ������
		virtual bool addCellFillFormat(size_t Row, size_t Col, bool font_flg) noexcept(true);
		//������� ������������� ������� ������:
		virtual bool addCurCellFormat(size_t Row, size_t Col) noexcept(true);
		//������� ������������� ������� �������� ��� ����� ������ �� �������:
		virtual bool initRowFormat() noexcept(true);
		//�������� ����� �� ����������� ������ � ����������� �� ������:
		bool useColoring(bool FndFlg, bool ChngFlg) const noexcept(true);
		//�������� ������ ������
		virtual bool ColoringRowCell(size_t Row, size_t Col, bool find_flg, bool proc_flg) noexcept(true);
		//�������� ������ � ����������� �� ����������� ��������� ��������� ������
		virtual bool ColoringRowCell(const NS_Excel::TExcelCell& cell, bool find_flg, bool procFlg) noexcept(true);
		//������� �������� ������:
		virtual bool ColoringRowByFlg(size_t curRow, bool FndFlg, bool ChngFlg) noexcept(true);
		virtual bool ColoringRowByCnt(size_t curRow, size_t FindCnt, size_t FailCnt) noexcept(true);
		//������� ������ � ����������� �� ������ � ����������
		virtual bool ColoringCellByParam(const NS_Tune::TCellData& param, size_t curRow, size_t frmt_index, 
			bool fing_flg) noexcept(true);
		//������� ������ ��������� ��������� � Excel-������:
		virtual void WriteOutParamToExcel(NS_Oracle::TBaseSet& bs, const NS_Const::DataType& type_code,
			const NS_Tune::TCellData& param, size_t OutParamIndex, size_t Row) noexcept(false);
		//������� ��������� ������ �� ��������� oracle-������ � ������ ���������� � excel-������ �����
		virtual void writeExcelFromDB(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false);
		//������� �������� ������������� ������ � ��(��� ����� ������):
		virtual bool checkINDataBase(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false);
		//������� ������ ������ � ��:
		virtual void insertToDataBase(NS_Oracle::TStatement& query, size_t curRow) noexcept(false);
		//������� ���������� ��������� � ����������� �� ������:
		//���������/��������� ������ �� ResultSet
		bool ProcessByResultSet(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false);
		//��������� ������ �� ���� � �������� ��������:
		void setOutStatementParam(NS_Oracle::TStatement& query, size_t curRow) noexcept(false);
		//��������� ������ �� Query/Statement
		void ProcessByStatement(NS_Oracle::TStatement& query, size_t curRow) noexcept(false);
		//������� ���������� ������� ��� ������:
		virtual bool runQuery(NS_Oracle::TStatement& query, size_t curRow) noexcept(true);
		//������� ��������� ������ �� �������� excel-�����:
		virtual bool setExcelRowDataByBD(NS_Oracle::TStatement& query, size_t curRow) noexcept(true);
		//������� ��������� ������ ��� �������/���������� � �������� excel-����� ��� ���� �����:
		virtual void setExcelDataByDB(NS_Oracle::TStatement& query, size_t& rowFrom) noexcept(false);
		//������� ������� ������ �� excel-����� � ���� ������:
		virtual void SetDBStatementData(NS_Oracle::TDBConnect& db, const NS_Tune::TUserTune& tune, 
			const string& sql, size_t& rowFrom) noexcept(false);
		//��������� ��������� ������� �������� ��� ��:
		void runDBStatementLst(TDBConnect& db, const TUserTune& tune, const NS_Tune::StrArr& sqlLst, size_t& rowFrom) noexcept(false);
		//������� ���������� ���������� �� excel-����� � ���� ������
		virtual void UpdExcelDataByDB(TDBConnect& db, const TUserTune& tune, size_t& rowFrom) noexcept(false);
		//������� ������� ������ ��� ������� � ��:
		virtual bool ClearImportDBTbl(TDBConnect& db, const TUserTune& tune) const noexcept(true);
		//������� �������� ���������� ������� ���������� ��� ��������� ������:
		virtual bool CorrectFilter(size_t cur_row) const noexcept(true);
		//������� ��������� ���������� � sql-������ ��� ��������� ������:
		bool setSelectParams(TStatement& query, size_t curRow) const noexcept(true);
		//������� ���������� ������ ��� �������� �� ���� ������:
		void ProcessSheetDataWithDB() noexcept(false);
		//������� ��������� ������������ ������� �� ���������������� ������:
		std::string getServerNameByTuneIndex(size_t val) const noexcept(true);
		//������� ��������� ������ ��������� � ������ ���������:
		bool procFindRow(const TExtendSheetReport& srcSheet, const CellDataArr& params,
			size_t dstRow, size_t srcRow) noexcept(true);
		//������� ��������� ����� ��������� � ���������:
		bool procFindCell(const TExtendSheetReport& srcSheet, const NS_Tune::TCellData& param, 
			size_t dstRow, size_t srcRow) noexcept(true);
		//������� ������ � ��������� ������ ������ ������ ��������� �� ����� ���������:
		bool Proc_DstRowCells_In_SrcSheet(const TExtendSheetReport& srcSheet, NS_ExcelReport::TRowsFlag& srcRows,
			const CellDataArr& params, size_t curRow, bool NoSpaceNoCase = true) noexcept(true);
		//������� ��������� ������ ������ ������ ��������� �� �������� ���������
		bool Execute_Seek_Dst_Cell_In_Src_Sht(const TExtendSheetReport& srcSheet, TRowsFlag& DstRows,
			TRowsFlag& SrcRows, const CellDataArr& params, bool NoSpaceNoCase = true) noexcept(true);
		//������� ������ ����� ��������� �� ���������� ���������(�������� ����� � ������� ��������)
		static size_t get_SrcRow_By_Dst_Params(const TJsonReport& Dst, const TExtendSheetReport& Src,
			TRowsFlag& DstRows, TRowsFlag& SrcRows, const NS_Tune::CellDataArr& original_params,
			bool NoSpaceNoCase = true) noexcept(false);
		//������� ��������� ������ ������ b ��������� ������ ��������� �� �������� ���������
		bool Execute_Seek_Dst_Row_In_Src_Sht(const TExtendSheetReport& srcSheet, TRowsFlag& DstRows,
			TRowsFlag& SrcRows, const CellDataArr& params, bool NoSpaceNoCase = true) noexcept(false);
		//������� �������� ������� ������ �����-��������� �� ����� � �����-���������:
		bool Search_DestData_In_SrcSheet(TRowsFlag& DstRows, const TExtendSheetReport& srcSheet, 
			bool NoSpaceNoCase = true) noexcept(true);
		//������� ������� ������ �� ������� excel-�����:
		void Compare_Excel_Sheets(bool NoSpaceNoCase = true) noexcept(false);
		//������� ��������� ����� ���������������� ������:
		size_t getTuneFilesCnt() const noexcept(true) { return cells_data.getDBTuneArr().size(); }
		//������� ��������� ���� ������ ���������:
		NS_Const::JSonMeth getMethCode() const noexcept(true) { return meth_code; }
	public:
		//������������� ��� ������ �������� �� json-�����
		TJsonReport(TExcelBook& book_ref, const TExcelProcData& json_tune, size_t page);
		//������������� ���������� ���������:
		TJsonReport(TExcelBook& book_ref, const TShareData& DstFile, const TProcCell& cell_arr, size_t page);
		//���������������:
		virtual ~TJsonReport() {}
		//������� ������������ ������
		virtual bool crtSheetReport() noexcept(true);
		//������� �������� �������������:
		bool preProcFlg() const noexcept(true) { return meth_code == NS_Const::JSonMeth::GetRowIDByDB; }

	};

	//����� ������������ ����� excel-������ �� ��������� js-����� ��������:
	class TJsonMarkReport: public TJsonReport
	{
	private:
		TRowsTNS& procRows;//������ ����� ��� ���������
		bool flg;//������� ���������� �� ����� ���������
	protected:
		//������� �������� ���������� ������� ���������� ��� ��������� ������:
		virtual bool CorrectFilter(size_t cur_row) const noexcept(true);
		//������� �������� ������������� ������ � ��:
		bool checkINDataBase(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false);
		//������� ��������� ������ ��� �������/���������� � �������� excel-����� ��� ���� �����:
		virtual void setExcelDataByDB(TStatement& query, size_t& rowFrom) noexcept(false);
		//������� ���������� ���������� �� excel-�����
		virtual void UpdExcelDataByDB(TDBConnect& db, const TUserTune& tune, size_t& rowFrom) noexcept(false);
	public:
		//������������� �������
		TJsonMarkReport(TExcelBook& book_ref, const TExcelProcData& json_tune, size_t page, TRowsTNS& Rows);
		//���������������:
		virtual ~TJsonMarkReport() {}
		//������� ���. ������� ������ �����:
		bool NoFltrRows() const noexcept(true) { return procRows.second.empty(); }
		//���������� ������ ������������ ����� ��� ��������:
		TRowsTNS& getProcRows() const noexcept(true) { return procRows; }
		//������� ��������� ������������ ������� ������������ ��� ������� ������:
		std::string getFlgName() const noexcept(true);
		//�������� ��������� �� ������ ����� ���������� ������:
		bool isFilterObject() const noexcept(true);
	};

	//����� ��� ������������ ������� �� ��������� ������ ��������
	//���������� ���� �� �� �
	class TSheetTuneReport: public TBaseSheetReport
	{
	protected:
		const TUserTune& tune;//��������� ��� ����� ������
		//������� ��������� ������/��������� ������ ��������:
		virtual size_t getRow(bool first) const noexcept(false) { return TBaseSheetReport::getRow(first); }
		//������� ������������� �������� �� ����������:
		virtual bool SetSheetByTune(const string& name = "") noexcept(true);
		//������� ���������� �������� ��� ������, ���� ������ ��������� �����������:
		virtual bool CreateNewPage(size_t val, bool byRows = true) noexcept(false);
	public:
		//�������� �������� ������ �� ����� � ������ �� ���� ���������
		TSheetTuneReport(TExcelBook& book_link, const TUserTune& config);
		//������������� ���������: 
		TSheetTuneReport(TExcelBook& ref_book, const TExcelBookSheet& ref_sheet, const TUserTune& ref_tune);
		//����������:
		virtual ~TSheetTuneReport() {}
		//������� ���������� �������� ������
		virtual bool crtSheet() noexcept(true) = 0;
	};

	//����� ��� �������������� � ��:
	struct TDataBaseInterface
	{
		static const  NS_Const::DataType DataTypeError = NS_Const::DataType::ErrorType;
		static const NS_Oracle::TType OraTypeError = NS_Oracle::TType::OCCIANYDATA;
		//������������ ���������� �����������:
		static TConnectParam getConnectParam(const TUserTune& param, int prefetch = 1) noexcept(false);
		//��������� �������� ��������� sql-������� �� ��������:
		static void setSqlParamByTune(TStatement& sql, const NS_Tune::TSubParam& param, bool use_raise = false) noexcept(false);
		//������� �������������� ����� ������ �� TOracle(TType) � ��� ������ TConstants(DataType)
		static NS_Const::DataType convertOraType(const NS_Oracle::TType& type) noexcept(true);
		//������� �������������� �� ����� ������ excel � ���� ������ oracle:
		static NS_Oracle::TType convertDataType(const NS_Const::DataType& type) noexcept(true);
		//��������� �������� ���������� sql-������� �� ����������:
		static void setSqlParamsByTune(TStatement& sql, const TUserTune& param) noexcept(false);
		//��������� ������ sql-������� � ��������� ��� ���:
		//static string getSqlByTune(bool use_parse, bool by_str, const string& str) noexcept(true);
		//��������� ������ sql-������� �� ��������:
		static string getSqlByTune(const TUserTune& tune, bool asDQL) noexcept(true);
		//����� ���������� ��������� dml-�������:
		static size_t executeDML(TDBConnect& db, const TUserTune& param, const string& dml, bool use_comit = true) noexcept(false);
		//����� dml-������� ��� ������:
		static size_t runDML4Directory(TDBConnect& db, const TUserTune& param, bool use_comit = true) noexcept(false);
		//���������� DML �������(����� ��������� ��� �������� � ������):
		static size_t runDML(TDBConnect& db, const TUserTune& param, bool use_comit = true) noexcept(true);
		//��������� �������� ���� �� ���� ������ �� ������� ������ sql-������
		static void setCellByResultSet(TExcelBook& book, TExcelBookSheet& sheet, const NS_Const::DataType& dt,
			const NS_Oracle::TBaseSet& bs, size_t resultSetCol, const NS_Excel::TExcelCell& cell, 
			const NS_Excel::FormatPtr format) noexcept(false);
		//��������� ����� �������� ��� �������:
		static bool setMaxIterationCnt(TStatement& query, size_t cnt) noexcept(true);
		//���������� �������� ��� �������:
		static bool addQueryIteration(TStatement& query) noexcept(true);
	};

	//����� ��������� ������ ��� �������� �� ��
	class TDataBaseSheetReport: public TSheetTuneReport, public TDataBaseInterface
	{
	private:
		//��������� � ����������� ��������:
		enum class SqlType { DQL, DML };
		int prefetch_rows;//����� ����������� ����� �� ���� ��������� � ��
		//��������� �������� ���� �� ���� ������ �� ������� ������ sql-������
		virtual void setCellByResultSet(const TResultSet& rs, const NS_Excel::TExcelCell& cell) noexcept(false);
		virtual bool WriteFromResultSet(TResultSet& rs) noexcept(true);
		//������� ���������� ������ � ����� �� ������ sql-�������:
		void FillSheetBySql(TDBConnect& db, const string& sql_txt, int prefetch);
		//������� ������������� ��������:
		bool useSqlParse() const noexcept(false);
		//������� ���������� DQL ������� ����� DML ��������
		//��� �������, ��� ��� ��� ������� � ����� config-�����
		bool isDQLFirst() const noexcept(false);
	protected:
		//������������ �������� ������ �� ������ �������:
		void CrtBySqlLine(TDBConnect& db, const string& sql_line, int prefetch = 1) noexcept(false);
		//������������ �������� ������ �� ������ sql-��������:
		void CrtBySqlFiles(TDBConnect& db, int prefetch = 1) noexcept(false);
	public:
		//������������� ������ � ������� ��������:
		TDataBaseSheetReport(TExcelBook& pbook, const TUserTune& config, int prefetch = PrefetchRowsCnt) : 
			TSheetTuneReport(pbook, config), prefetch_rows(prefetch) {}
		//������������� ������, ��������� � ������ ��������:
		TDataBaseSheetReport(TExcelBook& ref_book, const TExcelBookSheet& ref_sheet, const TUserTune& ref_tune,
			int prefetch = PrefetchRowsCnt) : TSheetTuneReport(ref_book, ref_sheet, ref_tune), prefetch_rows(prefetch) {}
		//������������ �������� ������:
		virtual bool crtSheet() noexcept(true);
	};

	//����� �������� ��� ������������ ������� �� ���������:
	class TSmlvchReport : public TBaseSheetReport
	{
		protected:
			const NS_Tune::TSharedTune& config;//������ �� ����� ��������� ������
			//������� ����������� ������/��������� ������:
			virtual size_t getRow(bool first) const noexcept(false) { return TBaseSheetReport::getRow(first); }
			//������� ������������� �������� ��� ������:
			virtual bool InitSheet(const string& sh_name, bool set_as_active = true) noexcept(true);
		public:
			//������������� �������:
			TSmlvchReport(NS_Excel::TExcelBook& book_link, const NS_Tune::TSharedTune& tune) :
				TBaseSheetReport(book_link), config(tune) {}
			//������� ������������ �������� ������ ��� �������������� ����� �������:
			//virtual bool crtSheet() const noexcept(true) = 0;
	};

	//����� �� ��������� �������� ��������:
	class TSmlvchBalance: public TSmlvchReport
	{
		private:
			using SubHeaderRows = std::vector<size_t>;
			//������� ��������� �������� ��� ������ �� ������� �������� ������:
			bool UpdFormatArrByColor(const NS_Excel::TColor& color, bool fnd_flg, bool font_flg) noexcept(true);
			//��������� �������������:
			bool setSubHeadersFont(const SubHeaderRows& rows) noexcept(true);
			//������� ��������� �������� ��� ����������� ������:
			NS_Tune::TBalanceTune getBalanceTune() const noexcept(true);
			//������� ������ ������:
			bool setTotalFields(size_t curRow, bool active_flg, double sld_rub, double sld_val, 
				const NS_Tune::CellDataArr& params) noexcept(true);
			//������� ������ ����� � ������ excel-���������
			bool setAccount2Row(size_t Row, const NS_SMLVCH_IMP::TAccount& acc, const NS_Tune::CellDataArr& params, 
				const NS_Excel::TColor& color = NS_Excel::TColor::COLOR_NONE,	double rate = 1.0) noexcept(true);
			//������� ������ ����� ������ �� ������� ����, ��� �� ������ ������ � ��������� ������� ��� ���������� ������:
			bool setAccounts2Sheet(size_t& curRow, const NS_SMLVCH_IMP::TAccounts& arr, const NS_Tune::CellDataArr& params,
				const NS_Tune::TCurrencyBlock& rates, SubHeaderRows& headers, const string& row_name_grp, bool last_row_as_sum = true) noexcept(true);
			//������� ������������ �������� ������ ��� ������������ �����:
			bool crtSheet(const string& imp_file, const NS_Tune::TBalanceTune& tune) noexcept(true);
		public:
			//�������������
			TSmlvchBalance(NS_Excel::TExcelBook& book_link, const NS_Tune::TSharedTune& tune) : TSmlvchReport(book_link, tune) {}
			//������� ������������ �������� ������ ��� �������������� ����� �������:
			virtual bool crtReport() noexcept(true);
	};

	//����� ���������� ������� ���������� � RS Bank:
	class TSmlvchImp : public TSmlvchReport
	{
	private:
		NS_SMLVCH_IMP::TRSBankImp imp_data;//������ ��� �������
		NS_Tune::TImpDocsTune imp_tune;//��������� ������� ������:
		//������� ���������� ������ �� ������:
		bool readRowData(size_t curRow) noexcept(true);
		//������� ���������� �������� �����:
		bool readSheet(size_t first_row, size_t last_row) noexcept(true);
		//������� ���������� ������� ���������� ��� ������ excel-�����:
		bool readFile(const string& file) noexcept(true);
	public:
		//�������������
		TSmlvchImp(NS_Excel::TExcelBook& book_lnk, const NS_Tune::TSharedTune& tune_lnk, const string& json_file);
		//������� ���������� excel-������ � ���������:
		bool setDocsByFiles() noexcept(true);
		//������� ������ ������ � ����:
		bool crtOutFile() const noexcept(true);
	};


	class TReport
	{
	private:
		//�������� ��������� ��� ������������ ������
		TSharedTune config;
		//������� ���������� ������:
		void saveReport(NS_Excel::TExcelBook& book, const string& file_name = "") const noexcept(false);
		//������� ��������� ������ excel-����� �� ��������� js-����� ��������
		bool ProcessExcelFileByJson(TExcelBook& book, const string& js_file, 
			std::vector<NS_ExcelReport::TRowsTNS>& Rows) const noexcept(true);
		//������� ��������� ������ excel-�����:
		bool Json_Report_By_File_Compare(const string& js_file) const noexcept(true);
		//������� ��������� ������ ����� ���������������
		bool Json_SubTune_File_Run(NS_Excel::TExcelBook& book, const string& js_file) const noexcept(true);
		//������� �������� ������ �� excel-�����
		bool loadFromJson(const string& js_file) const noexcept(true);
	public:
		//�������������
		explicit TReport(const string& config_file = string());
		TReport(const string& conf_file, const string& code) : config(conf_file, code) {}
		//����������
		virtual ~TReport() {}
		//������� ������������ �����-������:
		//��� ������ ������ ����������� ����������� excel-����
		void One_Report_For_Each_Config() const noexcept(false);
		//������� ������������ ������:
		//��� ������� ����� ������� ��������� ����������� ��������
		//���������� � ����� excel-�����
		void One_Sheet_By_One_Config() const noexcept(false);
		//������� ������������ ������:
		//��� ������� ������������ �� ����� ��������
		//�������� ����������� � excel-����
		void One_Sheet_By_Many_Statement() const noexcept(false);
		//���������� dml-������(��� ������� dql-��������)
		size_t runDML_By_Tune(bool use_comit = true) const noexcept(false);
		//������ ������������ ������� �� ��������� json-������:
		//������������ ������ �� ��������� ������ �� ��: 
		//���  ������ ������ ����������� ��������� � ��
		void Json_One_Row_One_DBQuery() const noexcept(false);
		//������������ ������ �� ��������� ��������� 2� excel-������:
		void Json_Report_By_Files_Compare() const noexcept(true);
		//������������ ������ � 2 ����� ���������������� ������:
		//�� ������ ����� ����������� ����� �� ��������� ������ � ����� config
		//�� ������ ����� �������������� ��������� ����� �� ��������� ������ � ����� SubConfig
		//������� ���������� 2��� ����� ��������� ������ ��� �����������(subconfig):
		void SubConfig_Json_UpdOutExlFile() const noexcept(true);
		//������� ���������� 2��� ����� ��������� - ��������� ini-�����:
		void SubConfig_IniFile_Execute() const noexcept(true);
		//������� �������� ������ �� excel � ���� ������:
		void load2DBFromExcel() const noexcept(false);
		//������������ ��������� �������� ��� ��������� �� OEM-����� �� RS-Bank
		void Smolevich_Balance_Report() const noexcept(true);
		//������������ ����� ������ ���������� ��� �������� � RS-Bank:
		bool Smolevich_Docs_Import() const noexcept(true);
		//����� �������, ����������� ����� �� ����:
		void Create_Report_By_Code(const NS_Const::ReportCode& code) const;
		//������������ ������� ������ �� ����� main_config:
		bool Execute() const noexcept(true);
		//!!!����������!!!
		//������������ ����� � ����������� �� ������ � RS Bank �� ��������� excel-���������
		static void Smolevich_Imp_Docs(const string& path, const string& out_file) noexcept(true);
	};

}
#endif