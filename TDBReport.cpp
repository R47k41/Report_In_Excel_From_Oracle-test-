//������ ����������� ����������� ��� TDBReport
#include <iostream>
#include <fstream>
#include "TDBReport.h"
#include "TSQLParser.h"
#include "Logger.hpp"
#include "TSMLVCH_IMP.h"


using std::string;
using NS_Logger::TLog;
using NS_Const::TuneField;
using NS_Oracle::TConnectParam;
using NS_Oracle::TStatement;
using NS_ExcelReport::DBUserTuneArr;
using NS_Tune::TShareData;
using NS_Tune::TExcelProcData;
using NS_Tune::TProcCell;
using NS_Tune::CellDataArr;
using NS_Tune::FilterArr;
using NS_Excel::TExcelBook;
using NS_ExcelReport::TBaseSheetReport;
using NS_ExcelReport::TDataBaseInterface;
using NS_ExcelReport::TSheetTuneReport;
using NS_ExcelReport::TExtendSheetReport;
using NS_ExcelReport::TJsonReport;
using NS_ExcelReport::TJsonMarkReport;
using NS_ExcelReport::TDataBaseSheetReport;
using NS_ExcelReport::TReport;
using NS_ExcelReport::TRowsFlag;
using NS_ExcelReport::TCellFormatIndex;

const int DEF_TEMPL_SH_INDX = 0;

void raise_app_err(const TLog& log, bool as_raise = true);

void raise_app_err(const TLog& log, bool as_raise)
{
	as_raise ? throw log : log.toErrBuff();
}

TCellFormatIndex::TCellFormatIndex(size_t par_Curent) :
	Current(par_Curent), NotFound(par_Curent), Found(par_Curent), InitFlg(true)
{
}

TCellFormatIndex::TCellFormatIndex(size_t par_Curent, size_t par_NotFound, size_t par_Found):
	Current(par_Curent), NotFound(par_NotFound), Found(par_Found), InitFlg(true)
{
}

bool TBaseSheetReport::addCurCellFormat(size_t Row, size_t Col) noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_Excel::TExcelBookFormat;
	//using NS_Tune::TIndex;
	using NS_ExcelReport::TCellFormatIndex;
	//������������� ������:
	TExcelCell cell(Row, Col, false);
	//���� ������ ������� �� ���������� - �����
	if (!cell.isValid())
		return false;
	try
	{
		//��������� ������� �� ������� ������:
		TExcelBookFormat format = sheet.getCellFormat(cell);
		//���� ������ ������� - ���������
		if (format.isValid())
		{
			//��������� ������� ������� ������� ������ � excel-�����:
			size_t curCellFormatIndex = book.getFormatIndex(format);
			//������������� �������:
			TCellFormatIndex cell_format_indx(curCellFormatIndex);
			if (cell_format_indx.InitFlg == false)
			{
				TLog log("�� ������� ���������������� ������ ��� ������: ", "TBaseSheetReport::addCurCellFormat");
				log << cell.getName() << '\n';
				throw log;
			}
			//���������� ��� � ������
			cells_format_indexs.insert(TCellFormat(cell.getCol(), cell_format_indx));
		}
		return true;
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("�� ������������ ������ ��� ���������� ������� ��� ������: ", "TBaseSheetReport::add2CellFormatArr");
		log << cell.getName() << '\n';
		if (!book.getError().empty()) log << book.getError() << '\n';
	}
	return false;
}

bool TBaseSheetReport::initRowFormat() noexcept(true)
{
	//���� �� ������� ����� ��� ����:
	if (!book.isValid() or !sheet.isValid()
		or book.isEmpty()) return false;
	//��������� ������ �� ������ ������ � �������:
	//���� ����� �������� �������� - �� ������ ����� �������� � ������ ������ ������
	size_t curRow = LastRow();
	//�������� �� ������� ������� ������
	//�.�. ������ ������� � ������� excel - �� 0
	//����������� �������� �� 1, ����� ��������� ������ ���������
	size_t curCol = sheet.getFirstCol() + 1;
	size_t last_col = sheet.getLastCol();
	//��� ������ ������ ��������� �� ������� ������:
	size_t err_cnt = 0;
	for (; curCol <= last_col; curCol++)
		if (addCurCellFormat(curRow, curCol) == false) err_cnt++;
	if (err_cnt >= curCol) return false;
	return true;
}

bool TBaseSheetReport::setCellFormatByIndexArr(const NS_Excel::TExcelCell& cell) noexcept(true)
{
	using NS_Excel::TExcelBookFormat;
	if (cell.getCol() >= cells_format_indexs.size()) return false;
	size_t FormatIndex = cells_format_indexs[cell.getCol()].Current;
	//��������� ������� �� �����:
	TExcelBookFormat format = book.getFormatByIndex(FormatIndex);
	//��������� �������:
	return setCellFormat(cell, format);
}

bool TBaseSheetReport::setCellFormatByIndexArr(size_t Row, size_t ColIndex) noexcept(true)
{
	using NS_Excel::TExcelCell;
	TExcelCell cell(Row, ColIndex, false);
	return setCellFormatByIndexArr(cell);
}

bool TBaseSheetReport::setRowCellsFormat(size_t Row) noexcept(true)
{
	using NS_Excel::TExcelCell;
	if (cells_format_indexs.empty()) return true;
	size_t cnt = 0;
	//�������� �� ������ ������ ��� ������� �������� ������
	for (const TCellFormat& i: cells_format_indexs)
	{
		//�.�. � ������� ��������� ���������� � �������� � ������� excel - �� 0
		//���� ��������� ������ ������ � ������������:
		TExcelCell cell(Row, i.first + 1, false);
		if (setCellFormatByIndexArr(cell)) cnt++;
	}
	return cnt == cells_format_indexs.size();
}

bool TBaseSheetReport::NeedNewPage(size_t item_cnt, bool byRows) const noexcept(false)
{
	if (byRows == false)
	{
		size_t max_val = book.MaxColsCount();
		if (item_cnt > max_val)
		{
			TLog log("��������� ����� �������� � ������ �� ����� �����!\n ����� �������� � �������: ", "WriteFromResultSet");
			log << item_cnt << TLog::NL << "\n����� �������� � excel-�����: " << max_val;
			throw log;
			//return true;
		}
	}
	else
	{
		size_t max_val = book.MaxRowsCount();
		if (item_cnt >= max_val) return true;
	}
	return false;
}

size_t TBaseSheetReport::getRow(bool first) const noexcept(false)
{
	if (!sheet.isValid())
		throw TLog("�������� ��� ������: " + book.getFileName() + 
			" �� ����������������!", "TBaseSheetReport::FirstRow");
	return first ? sheet.getFirstRow() : sheet.getLastRow();
}

bool TBaseSheetReport::inRange(size_t row, size_t col) const noexcept(false)
{
	return (row >= getRow(true) and row <= getRow(false)
		and col >= sheet.getFirstCol() and col <= sheet.getLastCol());
}

bool TBaseSheetReport::OpenBookSheet(const string& srcName, size_t page) noexcept(true)
{
	if (book.isValid() and !srcName.empty() and page >= 0)
	{
		bool flg = false;
		//���� ����� ������ - ��������� �����
		if (book.isEmpty() and !book.load(srcName)) return false;
		//������������� �������� � ������� ����� ��������
		sheet = book.getSheetByIndex(page);
		return true;
	}
	return false;
}

TBaseSheetReport::TBaseSheetReport(TExcelBook& book_ref, const string& src_file, size_t page_index):
	book(book_ref), sheet(nullptr)
{
	OpenBookSheet(src_file, page_index);
}

NS_Const::DataType TBaseSheetReport::convertExcelType(const NS_Excel::TDataType& dt, bool isDate) noexcept(true)
{
	using NS_Excel::TDataType;
	using NS_Const::DataType;
	//enum CellType { CELLTYPE_EMPTY, CELLTYPE_NUMBER, CELLTYPE_STRING, CELLTYPE_BOOLEAN, CELLTYPE_BLANK, CELLTYPE_ERROR }
	switch (dt)
	{
	case TDataType::CELLTYPE_NUMBER:
		return isDate ? DataType::Date : DataType::Double;
	case TDataType::CELLTYPE_STRING:
	case TDataType::CELLTYPE_EMPTY://????
	case TDataType::CELLTYPE_BLANK://????
		return DataType::String;
	case TDataType::CELLTYPE_BOOLEAN:
		return DataType::Boolean;
	}
	return DataType::ErrorType;
}

NS_Excel::TDataType TBaseSheetReport::convertDataType(const NS_Const::DataType& dt) noexcept(true)
{
	using NS_Excel::TDataType;
	using NS_Const::DataType;
	switch (dt)
	{
	case DataType::Integer:
	case DataType::Double:
	case DataType::Date:
		return TDataType::CELLTYPE_NUMBER;
	case DataType::String:
		return TDataType::CELLTYPE_STRING;
	case DataType::Boolean:
		return TDataType::CELLTYPE_BOOLEAN;
	}
	return TDataType::CELLTYPE_ERROR;
}

bool TBaseSheetReport::setCellFormat(const NS_Excel::TExcelCell& cell, NS_Excel::TExcelBookFormat& format) noexcept(true)
{
	//���� ������ ������������ ����
	if (!format.isValid()) return false;
	//���� ������� ������ ������ - ���� �� ������� �� ������???
	if (cell.isEmpty()) return false;
	try
	{
		bool rslt = sheet.setCellFormat(cell, format);
		if (rslt == false)
		{
			TLog log("������ ��������� ������� ��� ������: ", "TExtendSheetReport::setCellColor");
			log << cell.getName() << '\n' << book.getError() << '\n';
			throw log;
		}
		return rslt;
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("�� ������������ ������ ��� ������� ������:", "TExtendSheetReport::setCellColor");
		log << cell.getName() << '\n';
		log.toErrBuff();
	}
return false;
}


bool TBaseSheetReport::setCellFormat(size_t Row, size_t Column, NS_Excel::TExcelBookFormat& format) noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_Excel::TExcelBookFormat;
	//����������� ������� ������ � ������� � excel-����
	TExcelCell cell(Row, Column, false);
	return setCellFormat(cell, format);
}

NS_Excel::FormatPtr TBaseSheetReport::getCellFormatPtr(const NS_Excel::TExcelCell& cell) noexcept(true)
{
	using NS_Excel::FormatPtr;
	try
	{
		//��������� ������� ������� ������ �� ������� ��������:
		TCellFormatIndex format = getFormatIndexByCell(cell.getCol());
		//������� ��������� ������ �� ������ � ����� �� �������
		FormatPtr pformat = book.getFormatPrtByIndex(format.Current);
		return pformat;
	}
	catch (const std::exception& err)
	{
		TLog log("������ ��� ��������� ������ �� ������ ��� ������: ", "TBaseSheetReport::getCellFormatPtr");
		log << cell.getName() << '\n' << err.what() << '\n';
		log.toErrBuff();
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ��������� ������� ������: " + cell.getName(), 
			"TBaseSheetReport::getCellFormatPtr").toErrBuff();
	}
	return nullptr;
}

bool TBaseSheetReport::EqualCellsType(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
	const NS_Excel::TExcelCell& srcCell) const noexcept(false)
{
	using NS_Excel::TDataType;
	//��������� ���� ������ � ������ ���������:
	TDataType srcType = sheet.getCellType(srcCell);
	//��������� ���� ������ � ������ ���������:
	TDataType dstType = dstSheet.getCellType(dstCell);
	return srcType == dstType;
}

bool TBaseSheetReport::InsNewRow(size_t curRow, size_t newRow) noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_Excel::TExcelRange;
	//if (curRow == newRow) return true;
	try
	{
		//��������� �������� ����� ��� �������:
		TExcelCell cellB(curRow, 0);
		TExcelCell cellE(newRow, 0);
		TExcelRange range(cellB, cellE);
		return sheet.insRows(range);
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("�� ������������ ������ ��� ���������� ����� ������: ", "TBaseSheetReport::InsNewRow");
		log << newRow << " ����� " << curRow << " ������\n";
		log.toErrBuff();
	}
	return false;
}

void TExtendSheetReport::InitDstFile(const TShareData& dstFile, size_t page) noexcept(false)
{
	using NS_Tune::TSheetData;
	const TSheetData& tmp_sh = dstFile.getSheetParam(page);
	rowB = tmp_sh.getStartRow();
	rowE = tmp_sh.getLastRow();
	filters = dstFile.getFilterLst();
	colID = tmp_sh.getColID();
	//��������� ��������� excel-���� ��������:
	OpenBookSheet(dstFile.getName(), tmp_sh.getListIndex()-1);
}

TExtendSheetReport::TExtendSheetReport(TExcelBook& book_ref, const TShareData& file, size_t page) :
	TBaseSheetReport(book_ref), rowB(0), rowE(0), filters()
{
	InitDstFile(file, page);
}

bool TExtendSheetReport::isEmptyCell(const NS_Excel::TExcelCell& cell) const noexcept(false)
{
	try
	{
		return sheet.isEmptyCell(cell);
	}
	catch (...)
	{
		string s = book.getError();
		TLog log("�� ������������ ������ ��� �������� ������ � ������: ", "TExtendSheetReport::isEmptyCell");
		log << cell.getName() << "\n";
		if (!s.empty())
			log << s << '\n';
		throw log;
	}
	return true;
}

bool TExtendSheetReport::noDataInColID(size_t Row) const noexcept(false)
{
	using NS_Excel::TExcelCell;
	TExcelCell cell(Row, colID, false);
	return isEmptyCell(cell);
}

size_t TExtendSheetReport::FirstRow() const noexcept(true)
{
	//����� ������ � ������� ����� - �� 1
	size_t row = getRow(true);
	return row == NS_Tune::TIndex::EmptyIndex ? sheet.getFirstRow() + 1 : row;
}

size_t TExtendSheetReport::LastRow() const noexcept(true)
{
	//����� ������ � ������� ����� - �� 1
	size_t row = getRow(false);
	return row == NS_Tune::TIndex::EmptyIndex ? sheet.getLastRow() + 1 : row;
}

bool TExtendSheetReport::isCorrectFilter(size_t curRow) const noexcept(true)
{
	using NS_Tune::TFilterData;
	using NS_Excel::TExcelCell;
	//���� ������� ��� - ������� ������
	if (filters.empty())
	{
		if (noColID()) return true;
		return noDataInColID(curRow) == false;
	}
	for (const TFilterData& fltr : filters)
	{
		//����������� ������� ���������� �� AND - ���� ���� ���� �� ������� - �����
		if (checkByFilter(fltr, curRow) == false)
			return false;
	}
	return true;
}

NS_ExcelReport::TRowsFlag TExtendSheetReport::setFiltredRowsArr() const noexcept(true)
{
	using NS_ExcelReport::TRowsFlag;
	using NS_ExcelReport::TRowFlag;
	using std::make_pair;
	TRowsFlag rows;
	size_t i = FirstRow();
	size_t size = LastRow();
	bool all_flg = filters.empty();
	//������� ��� ������
	TLog log("����������� �������� ������� ������ ��� ", "TExtendSheetReport::setFiltredRowsArr");
	log << size << " �����!\n";
	log.toErrBuff();
	for (; i <= size; i++) 
	{
		if (all_flg or isCorrectFilter(i))
			rows.insert(TRowFlag(i, true));
	}
	log.clear(true);
	log << "��� ��������� �������: " << rows.size() << " �����!\n";
	log.toErrBuff();
	return rows;
}

bool TExtendSheetReport::Compare_Cells(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
	const NS_Excel::TExcelCell& srcCell, const NS_Const::JsonFilterOper& operation) const noexcept(true)
{
	using NS_Excel::TDataType;
	using NS_Excel::TExcelDate;
	using NS_Excel::TExcelBookFormat;
	using NS_Excel::FormatPtr;
	using NS_Const::TConstJSFilterOper;
	using NS_Const::JsonFilterOper;
	using std::stringstream;
	try
	{
		//�������� ����� �� �������:
		//if (sheet.isEmptyCell(srcCell) or dstSheet.isEmptyCell(dstCell)) return false;
		//if (QualCellsType(dstSheet, dstCell, srcCell) == false) return false;
		//��������� ���� ������ ��� ������ � ������� ����������:
		TDataType src_dt = sheet.getCellType(srcCell);
		switch (src_dt)
		{
			case TDataType::CELLTYPE_BOOLEAN: 
			{
				bool srcVal = sheet.ReadAsBool(srcCell);
				bool dstVal = dstSheet.ReadAsBool(dstCell);
				return TConstJSFilterOper::BoolBaseOperation(dstVal, srcVal, operation);
			}
			case TDataType::CELLTYPE_NUMBER:
			{
				//���������� ������ � ���������:
				double srcVal = sheet.ReadAsNumber(srcCell);
				//��������� �������� �� ������ � ������ - �����!!!!:
				if (sheet.isDate(srcCell))
				{
					//���� � ��������� �� ����:
					if (dstSheet.isDate(dstCell) == false)
					{
						//������� � �������������� ������ ������ �� �������
						//���� ���������� ���� � ������ � ������ � ����
						//���������� ��� ������:
						string dstVal = dstSheet.ReadAsString(dstCell);
						TExcelDate srcDate;
						book.Double2Date(srcVal, srcDate);
						string tmp = srcDate.toStr();
						return TConstJSFilterOper::StringBaseOperation(dstVal, tmp, operation);
					}
				}
				double dstVal = dstSheet.ReadAsNumber(dstCell);
				return TConstJSFilterOper::DoubleBaseOperation(dstVal, srcVal, operation);
				/**/
			}
			case TDataType::CELLTYPE_STRING: 
			//case TDataType::CELLTYPE_EMPTY:
			{
				string srcVal = sheet.ReadAsString(srcCell);
				string dstVal = dstSheet.ReadAsString(dstCell);
				//��������� ��� ��������
				//���������� ��������� ��������
				return TConstJSFilterOper::StringBaseOperation(dstVal, srcVal, operation);
			}
			default: 
			{
				TLog log("��� ������ � ID: ", "TExtendSheetReport::Compare_Cells");
				log << src_dt << " �� ��������������!\n";
				throw log;
			}
		}
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		string tmp = book.getError();
		TLog log("������ ��������� ������ � ������ ���������: ", "TExtendSheetReport::Compare_Cells");
		log << srcCell.getName() << " � ������ ���������: " << dstCell.getName() << '\n';
		if (!tmp.empty()) log << tmp << '\n';
		log.toErrBuff();
	}
	return false;
}

bool TExtendSheetReport::checkByFilter(const NS_Tune::TFilterData& filter, size_t Row) const noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_Excel::TDataType;
	using NS_Const::TConstJSFilterOper;
	//���� ������ ������ - �����
	if (filter.isEmpty())
		return true;
	//������������ ������ ��� ���������:
	TExcelCell cell(Row, filter.getColIndx(), false);
	try
	{
		//���� � ������ ��� ������ - �����
		if (sheet.isEmptyCell(cell))
			//�������:
			//throw TLog("������ ������: " + cell.getName(), "TExtendSheetReport::checkCellByFilter");
			return false;
		//�������� ��� ������ � ������:
		TDataType dt = sheet.getCellType(cell);
		//��������� ������ �� ������:
		switch (dt)
		{
			case TDataType::CELLTYPE_BOOLEAN:
			{
				bool srcVal = sheet.ReadAsBool(cell);
				return filter.isFiltredBoolValue(srcVal);
			}
			case TDataType::CELLTYPE_NUMBER:
			{
				//���������� ������ � ���������:
				double srcVal = sheet.ReadAsNumber(cell);
				//��������� �������� �� ������ � ������ - �����!!!:
				if (sheet.isDate(cell))
				{
					throw TLog("�� ����������� ��������� ����� ������!", "TExtendSheetReport::checkCellByFilter");
					//return dstVal == srcDate;
				}
				return filter.isFiltredDblValue(srcVal);
			}
			case TDataType::CELLTYPE_STRING:
				//case TDataType::CELLTYPE_EMPTY:
			{
				string srcVal = sheet.ReadAsString(cell);
				return filter.isFiltredStrValue(srcVal);
			}
			default:
			{
				TLog log("��� ������ � ID: ", "TExtendSheetReport::checkCellByFilter");
				log << dt << " � ������ : " << cell.getName() << " �� ��������������!\n";
				throw log;
			}
		}
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("�� ������������ ������ ��� ���������� ������ � ������: ", "TExtendSheetReport::checkCellByFilter");
		log << cell.getName() << '\n';
		log.toErrBuff();
	}
	return false;
}

bool TExtendSheetReport::CheckInCell(const NS_Excel::TExcelBookSheet& dstSheet, 
	const NS_Excel::TExcelCell& dstCell, const NS_Excel::TExcelCell& srcCell,
	bool NoSpaceNoCase) const noexcept(true)
{
	using NS_Const::JsonFilterOper;
	//���� � ������ ��� ������ - �������
	if (sheet.isEmptyCell(srcCell)) return false;
	//��������� ����������:
	JsonFilterOper operation = JsonFilterOper::Equal;
	if (NoSpaceNoCase) operation = JsonFilterOper::StrEqualNoCase;
	return Compare_Cells(dstSheet, dstCell, srcCell, operation);
}

bool TExtendSheetReport::CheckInCell(const NS_Excel::TExcelBookSheet& dstSheet, 
	const NS_Excel::TExcelCell& dstCell, size_t srcRow, size_t srcCol, 
	bool NoSpaceNoCase) const noexcept(true)
{
	using NS_Excel::TExcelCell;
	//������������ ������ ���������:
	TExcelCell srcCell(srcRow, srcCol, false);
	return CheckInCell(dstSheet, dstCell, srcCell, NoSpaceNoCase);
}

size_t TExtendSheetReport::CheckOnSheet(const NS_Excel::TExcelBookSheet& dstSheet, 
	const NS_Excel::TExcelCell& dstCell, size_t srcCol, bool NoSpaceNoCase) const noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_Tune::TIndex;
	size_t curRow = FirstRow();
	size_t lastRow = LastRow();
	//�������� �� ������� �����:
	for (; curRow <= lastRow; curRow++)
	{
		//��������� ������� �������:
		if (isCorrectFilter(curRow))
		{
			//���� ������ � ������:
			if (CheckInCell(dstSheet, dstCell, curRow, srcCol, NoSpaceNoCase))
				//�������
				return curRow;
		}
	}
	return TIndex::EmptyIndex;
}

size_t TExtendSheetReport::CheckOnSheet(const NS_Excel::TExcelBookSheet& dstSheet,
	const NS_Excel::TExcelCell& dstCell, size_t srcCol, NS_ExcelReport::TRowsFlag& RowsArr,
	bool NoSpaceNoCase) const noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_Tune::TIndex;
	//���� ������ �������� ���� - �����
	if (RowsArr.empty()) return TIndex::EmptyIndex;
	//������� ������������� ���������� �� ���������:
	//�������� �� ������� �����:
	for (const TRowFlag& curRow: RowsArr)
	{
		//���� ���������� ��� ������ ������ �������:
		if (curRow.second == true)
		{
			//���� ������ � ������:
			if (CheckInCell(dstSheet, dstCell, curRow.first, srcCol, NoSpaceNoCase))
			{
				//������� ������ �� ������� ��������������� �����
				//RowsArr.erase(curRow.first);//RowsArr[curRow.first] = false;
				//�������
				return curRow.first;
			}
		}
	}
	return TIndex::EmptyIndex;
}

bool TExtendSheetReport::NotEquality(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
	const NS_Excel::TExcelCell& srcCell, bool NoCaseNoSpace) const noexcept(true)
{
	using NS_Const::JsonFilterOper;
	JsonFilterOper operation = JsonFilterOper::NotEqual;
//	if (NoCaseNoSpace) operation = JsonFilterOper::StrEqualNoCase;
	bool flg = Compare_Cells(dstSheet, dstCell, srcCell, operation);
	return flg;
}

bool TExtendSheetReport::setDstCellBySrcCell(NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
	const NS_Excel::TExcelCell& srcCell) const noexcept(true)
{
	using NS_Excel::TDataType;
	try
	{
		//���� ������ � ������ ��������� - ������:
		if (sheet.isBlank(srcCell))
		{
			//������ ������ ������
			if (dstSheet.isBlank(dstCell) == false)
				dstSheet.setBlank(dstCell);
			return true;
		}
		//���� ���� ������ � ������� �� ���������:
		//if (QualCellsType(dstSheet, dstCell, srcCell) == false) return false;
		//��������� ���� ������ � ������ ���������:
		TDataType srcType = sheet.getCellType(srcCell);
		switch (srcType)
		{
		case TDataType::CELLTYPE_BOOLEAN:
		{
			bool srcVal = sheet.ReadAsBool(srcCell);
			dstSheet.WriteAsBool(dstCell, srcVal);
			break;
		}
		case TDataType::CELLTYPE_NUMBER:
		{
			double srcVal = sheet.ReadAsNumber(srcCell);
			//���� �������� ���������� - ��������
			//double dstVal = dstSheet.ReadAsNumber(dstCell);
			//if (srcVal != dstVal)
			dstSheet.WriteAsNumber(dstCell, srcVal);
			break;
		}
		case TDataType::CELLTYPE_STRING:
		{
			string srcVal = sheet.ReadAsString(srcCell);
			dstSheet.WriteAsString(dstCell, srcVal);
			break;
		}
		default: 
		{
			TLog log("��������� ��� ������: ", "TExtendSheetReport::setDstCellBySrcCell");
			log << srcType << " �� ��������������\n";
			throw log;
		}
		}
		return true;
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		string tmp = book.getError();
		TLog log("������ ���������� ������ � ������: ", "TExtendSheetReport::setDstCellBySrcCell");
		log << dstCell.getName() << " �����: " << dstSheet.getName() << " �� ������: " << srcCell.getName()
			<< " �����: " << sheet.getName() << '\n';
		if (!tmp.empty()) log << tmp << '\n';
		log.toErrBuff();
	}
	return false;
}

TJsonReport::TJsonReport(NS_Excel::TExcelBook& book_ref, const TShareData& DstFile, const TProcCell& cell_arr, size_t page):
	TExtendSheetReport(book_ref, DstFile, page), cells_data(cell_arr), meth_code(NS_Const::JSonMeth::Null)
{
	meth_code = cells_data.getMethodCode();
	//������������ �������� ��������:
	initRowFormat();
}

TJsonReport::TJsonReport(NS_Excel::TExcelBook& book_ref, const TExcelProcData& json_tune, size_t page) :
	TExtendSheetReport(book_ref, json_tune.getDstFile(), page), cells_data(json_tune.getCellsData()),
	meth_code(NS_Const::JSonMeth::Null)
{
	meth_code = cells_data.getMethodCode();
	//������������ �������� ��������:
	initRowFormat();
}

bool TJsonReport::CorrectFilter(size_t cur_row) const noexcept(true)
{
	return TExtendSheetReport::isCorrectFilter(cur_row);
}

void TJsonReport::setDMLOutParam(NS_Oracle::TStatement& query, const NS_Tune::TCellData& param) noexcept(false)
{
	NS_Oracle::TType out_type = TDataBaseInterface::convertDataType(param.getOutType());
	if (out_type == TDataBaseInterface::OraTypeError)
		throw TLog("������ �������������� ������ � ��� ������ Oracle!", "TJsonReport::setDMLOutParam");
	query.registerOutParam(param.SrcParam(), out_type);
}

bool TJsonReport::isParamColumn(size_t Col) const noexcept(true)
{
	using NS_Tune::TCellData;
	using NS_Tune::CellDataArr;
	//������ ���������� � ������� �����: ������ �� 1
	if (Col == getColID()) return true;
	//��������� ������ �� ������ ����������:
	CellDataArr arr = cells_data.getCellDataArr();
	for (const NS_Tune::TCellData& param : arr)
		//���� ������� ��������� � ������������ ��� ������������ �������
		if (param.DstIndex() == Col or param.InsIndex() == Col)
			return true;
	return false;
}


void TJsonReport::setDQLParamByCell(TStatement& query, const NS_Tune::TCellData& value, size_t curRow) const noexcept(false)
{
	using NS_Excel::TExcelCell;
	using NS_Const::DataType;
	using NS_Const::TConstType;
	using NS_Oracle::TDate;
	using NS_Excel::TExcelDate;
	const DataType& data_type = value.getInType();
	//�������������� ������
	TExcelCell cell(curRow, value.DstIndex(), false);
	//���� ������ ������
	if (sheet.isBlank(cell))
	{
		query.setNullVal(value.SrcParam(), TDataBaseInterface::convertDataType(data_type));
		return;
	}
	//���� ��� ���� ������
	switch (data_type)
	{
		case DataType::String:
		{
			//��������� �� excel-�����
			string tmp = sheet.ReadAsString(cell);
			if (tmp.empty())
				query.setNullVal(value.SrcParam(), TDataBaseInterface::convertDataType(DataType::String));
			else
				//��������� � �������� �������
				query.setStringVal(value.SrcParam(), tmp);
			break;
		}
		case DataType::Integer:
		case DataType::Boolean:
		{
			int tmp = sheet.ReadAsNumber(cell);
			query.setIntVal(value.SrcParam(), tmp);
			break;
		}
		case DataType::Double:
		{
			double tmp = sheet.ReadAsNumber(cell);
			query.setDoubleVal(value.SrcParam(), tmp);
			break;
		}
		case DataType::Date:
		{
			if (sheet.isDate(cell))
			{
				double tmp = sheet.ReadAsNumber(cell);
				TExcelDate exl_date;
				if (book.Double2Date(tmp, exl_date))
				{
					TDate ora_date = query.initOCCIDate(exl_date.year, exl_date.month, exl_date.day,
						exl_date.hour, exl_date.minute, exl_date.sec);
					query.setDateVal(value.SrcParam(), ora_date);
				}
			}
			break;
		}
		default: 
			TLog log("��������� ��� ������: ", "TJsonReport::setDQLParamByCell");
			log << TConstType::asStr(value.getInType()) << " �� ��������������!\n";
			throw log;
	}
}

bool TJsonReport::setStatementParam(NS_Oracle::TStatement& query, const NS_Tune::TCellData& value, size_t Row) const noexcept(true)
{
	using NS_Const::DataType;
	//���������� ������������ ������ ��� ��������� i-��� ���������
	size_t ColIndx = 0;
	try
	{
		//���� �������� ������/��������� - ����������
		if (value.isEmpty())
			return true;
		//��������� ��������� ���������:
		if (value.isOutParam())
		{
			ColIndx = value.SrcParam();
			//��������� ��������� ���������:
			setDMLOutParam(query, value);
		}
		//��������� �������� ���������
		else
		{
			ColIndx = value.DstIndex();
			//��������� ��������� ������� ��� ������ ������:
			setDQLParamByCell(query, value, Row);
		}
		return true;
	}
	catch (TLog& err)
	{
		err << "������ ��� ��������� ���������: " << ColIndx << '\n';
		err.toErrBuff();
	}
	catch (const NS_Oracle::SQLException& err)
	{
		TLog log("������ ��������� ���������: ", "TJsonReport::setStatementParam");
		log << ColIndx << '\n' << err.what() << '\n';
		log.toErrBuff();
	}
	catch (...)
	{
		TLog log("�� ������������ ������ ��� ��������� ���������: ", "TJsonReport::setStatementParam");
		log << ColIndx << '\n';
		log.toErrBuff();
	}
	return false;
}

bool TJsonReport::setSelectParams(TStatement& query, size_t curRow) const noexcept(true)
{
	using NS_Tune::TCellData;
	using NS_Excel::TExcelCell;
	try
	{
		//��������� ������� ����������:
		vector<TCellData> params = cells_data.getCellDataArr();
		//���� ��������� ������ - ������������� �� ���� �� ����
		if (params.empty()) return true;
		//�������� �� ���� ���������� � ������������� �� � ������:
		for (const TCellData& cd : params)
		{
			if (setStatementParam(query, cd, curRow) == false)
				return false;
		}
		return true;
	}
	catch (TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("�� ������������ ������ ��� ��������� ���������� ������� ��� ������: ", "TJsonReport::setSelectParams");
		log << curRow << '\n';
		log.toErrBuff();
	}
	return false;
}

void TJsonReport::WriteOutParamToExcel(NS_Oracle::TBaseSet& bs, const NS_Const::DataType& type_code,
	const NS_Tune::TCellData& param, size_t OutParamIndex, size_t Row) noexcept(false)
{
	using NS_Excel::TExcelCell;
	using NS_Excel::FormatPtr;
	//���� ������ ��� ������� ������ - ����������:
	//if (param.EmptyInsIndx()) return;
	//��������� �������:
	TExcelCell cell(Row, param.InsIndex(), false);
	//��������� ������� ��� ������:
	FormatPtr format = getCellFormatPtr(cell);
	//��������� ������ � ������
	TDataBaseInterface::setCellByResultSet(book, sheet, type_code, bs, OutParamIndex, cell, format);
}

void TJsonReport::writeExcelFromDB(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false)
{
	using NS_Excel::TExcelCell;
	using NS_Tune::TIndex;
	using NS_Const::DataType;
	//��������� ������� ����������:
	vector<TCellData> params = cells_data.getCellDataArr();
	//���� ��������� ������ - ������������� �� ���� �� ����
	if (params.empty()) return;
	//��������� ����� ������� �������:
	//size_t colCnt = rs.getColumnsCnt();
	//�������� �� ���������� �������:
	while (rs.Next())
	{
		//�������� �� ���� ���������� � ��������� ��:
		for (const TCellData& cd : params)
		{
			size_t OutParamIndx = cd.SrcVal();
			//��������� ���� ������ ��� ������:
			DataType col_type = TDataBaseInterface::convertOraType(rs.getColumnType(OutParamIndx));
			WriteOutParamToExcel(rs, col_type, cd, OutParamIndx, curRow);
		}
	}
}

bool TJsonReport::checkINDataBase(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false)
{
	bool f = false;
	if (rs.Next())
	{
		size_t cnt = rs.getIntVal(1);
		if (cnt > 0)
			f = true;
	}
	return f;
}

void TJsonReport::insertToDataBase(NS_Oracle::TStatement& query, size_t curRow) noexcept(false)
{
	//���������� DML-������� � ������� ������������ �����:
	size_t cnt = query.getProcessedCntRows();
	if (cnt > 0)
	{
		TLog log("������ ", "TJsonReport::insertToDataBase");
		log << curRow  << " ���� ������� �������������!\n";
		log.toErrBuff();
		//query.Commit();
	}
	else
	{
		TLog log("������ ������� ", "TJsonReport::insertToDataBase");
		log << curRow  << " ������!\n";
		throw log;
	}
}

bool TJsonReport::ProcessByResultSet(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false)
{
	using NS_Const::JSonMeth;
	switch (meth_code)
	{
		case JSonMeth::GetFromDB:
		{
			writeExcelFromDB(rs, curRow);
			return true;
		}
		case JSonMeth::GetRowIDByDB:
		{
			return checkINDataBase(rs, curRow);
		}
	}
	return false;
}

void TJsonReport::setOutStatementParam(NS_Oracle::TStatement& query, size_t curRow) noexcept(false)
{
	using NS_Excel::TExcelCell;
	using NS_Const::DataType;
	//��������� ������� ����������:
	vector<TCellData> params = cells_data.getCellDataArr();
	//���� ��������� ������ - ������������� �� ���� �� ����
	if (params.empty()) return;
	//�������� �� ���� ���������� � ��������� ��:
	for (const TCellData& cd : params)
	{
		if (cd.EmptyInsIndx()) continue;
		size_t OutParamIndx = cd.SrcParam();
		//��������� ���� ������ ��� ������:
		DataType col_type = cd.getOutType();
		WriteOutParamToExcel(query, col_type, cd, OutParamIndx, curRow);
	}
}

void TJsonReport::ProcessByStatement(NS_Oracle::TStatement& query, size_t curRow) noexcept(false)
{
	using NS_Const::JSonMeth;
	//��������� ��������� � ����������� �� ������ ���������:
	switch (meth_code)
	{
		//���������� ������ � ����
		case JSonMeth::SendToDB:
		{
			//�� ������ ������ ������ ��� ��������!
			insertToDataBase(query, curRow);
			break;
		}
		//�� ��������� �������, ��� �� ���� �������� �������� ���������
		default: 
		{
			setOutStatementParam(query, curRow);
			break;
		}
	}
}

bool TJsonReport::runQuery(NS_Oracle::TStatement& query, size_t curRow) noexcept(true)
{
	using NS_Oracle::TSQLState;
	try
	{
		//�������� ������:
		if (query.ExecuteSQL() == false) return false;
		//�������� ������ �������:
		TSQLState state = query.getState();
		switch (state)
		{
			//������������ ������ �� ����������� �������
			case TSQLState::RESULT_SET_AVAILABLE:
			{
				//��������� ������ �� ResultSet
				NS_Oracle::ResultSetPtr rsp = query.getResultSetVal();
				//���������� �������
				TResultSet rs(rsp);
				//��������� �������� �� ������� � excel-����
				bool rslt = ProcessByResultSet(rs, curRow);
				//��������� ���������� �����
				query.closeResultSet(rsp);
				return rslt;
			}
			//������������ ������ ����� ���������� �������� ���������
			case TSQLState::UPDATE_COUNT_AVAILABLE:
			{
				ProcessByStatement(query, curRow);
				return true;
			}
			default:
			{
				TLog log("��������� ��������� ", "TJsonReport::runQuery");
				log << state << " �� ��������������!\n";
				throw log;
			}
		}
	}
	catch (const NS_Oracle::SQLException& err)
	{
		TLog log("������ ���������� �������: ", "TJsonReport::runQuery");
		log << query.getSQL() << "\n��� ������: " << curRow << '\n' << err.what() << '\n';
		log.toErrBuff();
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("�� ������������ ������ ��� ���������� �������: ", "TJsonReport::runQuery");
		log << query.getSQL() << "\n��� ������: " << curRow << '\n';
		log.toErrBuff();
	}
	return false;
}

bool TJsonReport::setExcelRowDataByBD(NS_Oracle::TStatement& query, size_t curRow) noexcept(true)
{
	//��������� ������� ��� ���������� ����� � excel-�����:
	if (CorrectFilter(curRow))
	{
		//��������� ���������� �������:
		if (setSelectParams(query, curRow))
		{
			//��������� ������:
			bool find_flg = runQuery(query, curRow);
			//������� ������-��������������
			ColoringRowCell(curRow, getColID(), find_flg, true);
			return find_flg;
		}
	}
	else//���� ������� ���������� �� ���������:
		return true;
	return false;
}

void TJsonReport::setExcelDataByDB(NS_Oracle::TStatement& query, size_t& rowFrom) noexcept(false)
{
	using NS_Tune::TIndex;
	using NS_Const::JSonMeth;
	if (rowFrom <= 0)
	{
		TLog log("�� ����� ������� ��������� ������ ���������: ", "TJsonReport::setExcelDataByDB");
		log << rowFrom << '\n';
		throw log;
	}
	//��������� ������ ��������� �������������� ������
	size_t rowTo = LastRow();
	size_t errCnt = 0;
	//���������� ����� excel-�����
	for (rowFrom; rowFrom <= rowTo; rowFrom++)
	{
		bool rslt = setExcelRowDataByBD(query, rowFrom);
		if (rslt == false) errCnt++;
	}
	//���� ���� ������ ��� ������� ������:
	if (meth_code == JSonMeth::SendToDB)
		if (errCnt > 0)
		{
			TLog log("��� ��������� �������: ", "setExcelDataByDB");
			log << query.getSQL() << " ��������: " << errCnt << " ������! ��������� ��������!\n";
			throw log;
		}
		else
		{
			query.Commit();
			TLog("��� ������ ������� �������������!\n", "setExcelDataByDB").toErrBuff();
		}
}

void TJsonReport::SetDBStatementData(NS_Oracle::TDBConnect& db, const NS_Tune::TUserTune& tune, 
	const string& sql, size_t& rowFrom) noexcept(false)
{
	using NS_Oracle::TStatement;
	//������������ ������� � ��:
	TStatement query(db, sql, 1);
	//��������� ���������� ����������:
	TDataBaseInterface::setSqlParamsByTune(query, tune);
	//���������� �������:
	setExcelDataByDB(query, rowFrom);
	query.close();
}

void TJsonReport::runDBStatementLst(TDBConnect& db, const TUserTune& tune, const NS_Tune::StrArr& sqlLst, size_t& rowFrom) noexcept(false)
{
	using NS_Tune::StrArr;
	for (const string& sql : sqlLst)
	{
		if (sql.empty()) continue;
		SetDBStatementData(db, tune, sql, rowFrom);
	}
}

void TJsonReport::UpdExcelDataByDB(NS_Oracle::TDBConnect& db, const NS_Tune::TUserTune& tune, size_t& rowFrom) noexcept(false)
{
	using NS_Tune::TIndex;
	using NS_Tune::TUserTune;
	using NS_Tune::StrArr;
	//���� ������ �� ������� ���� ���������� �� ����������� - ����� �� �� ��������
	if (rowFrom == TIndex::EmptyIndex)
		rowFrom = FirstRow();
	//��������� ������ ��������:
	StrArr sqlLst = tune.getDMLList();
	//��������� ��������
	runDBStatementLst(db, tune, sqlLst, rowFrom);
}

bool TJsonReport::ClearImportDBTbl(TDBConnect& db, const TUserTune& tune) const noexcept(true)
{
	using NS_Oracle::TStatement;
	using NS_Tune::TUserTune;
	using NS_Tune::StrArr;
	try
	{
		//��������� ������� ������� ��� ������� ������� � ��:
		StrArr clearLst = tune.getClearList();
		for (const string& sql : clearLst)
		{
			//������������ ������� � ��:
			TStatement query(db, sql, 1);
			//��������� ���������� ����������:
			//TDataBaseInterface::setSqlParamsByTune(query, tune);
			//���������� �������:
			size_t cnt = query.executeDML();
			query.close();
		}
		return true;
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ������� ������ �������!", "ClearImportDBTbl").toErrBuff();
	}
	return false;
}

void TJsonReport::ProcessSheetDataWithDB() noexcept(false)
{
	if (!book.isValid()) 
		throw TLog("����� �� ����������������!", "TJsonReport::SheetDataFromDataBase");
	//��������� ������ ������ ��� ���������
	size_t row = FirstRow();
	//��� ������ ��������� ����������� ���������:
	for (const TUserTune& config : cells_data.getDBTuneArr())
	{
		//��������� ���������� ����������� � ��
		NS_Oracle::TConnectParam cp = TDataBaseInterface::getConnectParam(config);
		//������� ����������� � ��:
		TDBConnect db(cp);
		if (!db.isValid()) 
			throw TLog("������ ����������� � ��: " + cp.tns_name, "TJsonReport::SheetDataFromDataBase");
		try
		{
			//���� ������� - ������� ������� �������:
			ClearImportDBTbl(db, config);
			//��������� ������ � excel-�����:
			UpdExcelDataByDB(db, config, row);
		}
		catch (TLog& err)
		{
			err << "��������� ������������ ������: " << row << '\n';
			err.toErrBuff();
		}
		catch (...)
		{
			TLog log("�� ������������ ������ ��� ������ �� �������: ", "TJsonReport::SheetDataFromDataBase");
			log << row << '\n';
			log.toErrBuff();
			break;
		}
		db.closeConnection();
	}
}

bool TJsonReport::addFillFormat(size_t init_format_index, bool find_flg, bool font_flg, 
	size_t& AddedFormatIndex) noexcept(false)
{
	using NS_Excel::TExcelBookFormat;
	using NS_Excel::TExcelBookFont;
	using NS_Excel::TColor;
	using NS_Tune::TCellMethod;
	//������������� ������� �� �������:
	TExcelBookFormat tmpFormat = book.getFormatByIndex(init_format_index);
	//���� ������ �� ������� - �������
	if (tmpFormat.isValid())
	{
		TColor color = TColor::COLOR_NONE;
		//��������� ����� ������� �� ��������:
		const TCellMethod& meth = cells_data.getMethod();
		if (find_flg)
			color = meth.getIncludeColor();
		else
			color = meth.getExcludeColor();
		//���� ���� ������ - �������
		if (color == TColor::COLOR_NONE) return false;
		//���������� ������� � �����:
		TExcelBookFormat result = book.AddFormat(tmpFormat, false);
		if (result.isValid())
		{
			if (font_flg)
			{
				//������ ���� ������
				TExcelBookFont font = result.getFont();
				font.setColor(color);
				result.setFont(font);
			}
			else
			{
				//������ ���� ������
				result.setPatternFill(TExcelBookFormat::TFill::FILLPATTERN_SOLID);
				result.setBorderColor(color, TExcelBookFormat::TBorderSide::Foreground);
			}
			//��������� ������� ������ �������
			AddedFormatIndex = book.FormatCount() - 1;
			return true;
		}
	}
	return false;
}

bool TJsonReport::addCellFillFormat(size_t Row, size_t Col, bool font_flg) noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_Excel::TExcelBookFormat;
	using std::exception;
	//������������� ������:
	TExcelCell cell(Row, Col, false);
	try
	{
		//��������� ������� ������ ������ �� ������� ��������:
		TCellFormatIndex& tmpIndex = TBaseSheetReport::getFormatIndexByCell(cell.getCol());
		if (tmpIndex.InitFlg == false)
			throw TLog("������� ��� ������: " + cell.getName() + " �� ������!\n", "TJsonReport::addCellFillFormat");
		//������������ ����� �������� ��� ������� ������
		bool fnd_flg = addFillFormat(tmpIndex.Current, true, font_flg, tmpIndex.Found);
		bool not_fnd_flg = addFillFormat(tmpIndex.Current, false, font_flg, tmpIndex.NotFound);
		return fnd_flg && not_fnd_flg;
		return true;
	}
	catch (const exception& err)
	{
		TLog log("������ ��� ���������� ������� ��� ������: ", "TJsonReport::addCellFillFormat");
		log << cell.getName() << '\n' << err.what() << '\n';
		log.toErrBuff();
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ��� ��������� ������� ��� ������: " + cell.getName(), 
			"TJsonReport::addCellFillFormat").toErrBuff();
	}
	return false;
}

bool TJsonReport::addCurCellFormat(size_t Row, size_t Col) noexcept(true)
{
	using NS_Const::JsonCellFill;
	//��������� ��������� �� ������ ������ � ���������� ��� ID-������:
	//��������� ������� �������������� �����:
	if (isParamColumn(Col) == false) return false;
	//������������� �������������� ��� ������� ������:
	bool setCurCellFormatFlg = TBaseSheetReport::addCurCellFormat(Row, Col);
	//��������� ������ �� ����� ���������:
	const TCellMethod& meth = cells_data.getMethod();
	//���� �� ������ ����� ������� ��� ����� ������
	if (meth.isEmptyColor())
		//���������� ������ ������ ������� ������
		return setCurCellFormatFlg;
	bool use_font = false;
	//���� ������ �� �������� ID - �������� ����������� ����������� ����� ������
	if (getColID() != Col) use_font = meth.useFont();
	return addCellFillFormat(Row, Col, use_font);
}

bool TJsonReport::initRowFormat() noexcept(true)
{
	//���� ���������� ��� - �������
	if (cells_data.CellCnt() <= 0) return false;
	//��������� �������� ������ ��� ����������������� �����
	return TBaseSheetReport::initRowFormat();
}

bool TJsonReport::useColoring(bool FndFlg, bool ChngFlg) const noexcept(true)
{
	//���� ������������ ����� ������������� ��������� � ������ �������
	if (WithChangeMeth() and FndFlg)
		//�� ������� �������� ����� ������� ���������:
		return ChngFlg;
	return true;
}

bool TJsonReport::ColoringRowCell(const NS_Excel::TExcelCell& cell, bool find_flg, bool procFlg) noexcept(true)
{
	//�������� ������� ������� �������� excel-�����
	if (EmptyCellsIndexFormat()) return false;
	//���� ������������ ����� ������������ ��������� � ��������� �� ���� - �����
	if (useColoring(find_flg, procFlg) == false) return false;
	try
	{
		//��������� ������� ������ �� ������ ��������:
		TCellFormatIndex& formats = getFormatIndexByCell(cell.getCol());
		if (formats.InitFlg == false)
			throw TLog("������� ��� ������: " + cell.getName() + " �� ������!", "TJsonReport::addCellFillFormat");
		size_t formatIndex = find_flg ? formats.Found : formats.NotFound;
		TExcelBookFormat format = book.getFormatByIndex(formatIndex);
		return TBaseSheetReport::setCellFormat(cell, format);
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("�� ������������ ������ ��� ��������� ������� ������: ", "TJsonReport::ColoringRowCell");
		log << cell.getName() << '\n';
		log.toErrBuff();
	}
	return false;
}

bool TJsonReport::ColoringRowCell(size_t Row, size_t Col, bool find_flg, bool procFlg) noexcept(true)
{
	using NS_Excel::TExcelCell;
	//������������� ������:
	TExcelCell cell(Row, Col, false);
	return ColoringRowCell(cell, find_flg, procFlg);
}

bool TJsonReport::ColoringRowByFlg(size_t curRow, bool FndFlg, bool ChngFlg) noexcept(true)
{
	//� ������ ������ ������� ���������� ������ �� ColID
	//���� ��� ������ �������������� - �����
	if (noColID()) return false;
	return ColoringRowCell(curRow, getColID(), FndFlg, ChngFlg);
}

bool TJsonReport::ColoringRowByCnt(size_t curRow, size_t FindCnt, size_t FailCnt) noexcept(true)
{
	//������� �������� �� ������� ������ ��� �� ����������������� ������
	//������������ ��� ������ ������ ������, ����� ���� ��������� ��� ���������
	//��������� ����� �������, ��� ������ ���� �������
	//� ����������� �� ������ ������:
	bool flg = cells_data.getMethod().isSuccess(FindCnt, FailCnt);
	//�������� ������
	return ColoringRowByFlg(curRow, flg, true);
}


bool TJsonReport::ColoringCellByParam(const NS_Tune::TCellData& param, size_t curRow, size_t frmt_index, bool fing_flg) noexcept(true)
{
	using NS_Const::JSonMeth;
	using NS_Const::TConstJSMeth;
	using NS_Tune::TIndex;
	using NS_Excel::TExcelCell;
	size_t column = 0;
	//����� ������ � ������� � ������� ����� - �� 1
	switch (meth_code)
	{
		case JSonMeth::CompareCellChange:
		{
			column = param.InsIndex();
		}
		case JSonMeth::CompareRow:
		case JSonMeth::CompareCell:
		case JSonMeth::InsertRowCompare:
		{
			column = param.DstIndex();
		}
		default:
		{
			TLog log("��������� �����: ", "TJsonReport::procFindCell");
			log << TConstJSMeth::asStr(meth_code) << " �� ��������������!\n";
			log.toErrBuff();
			return false;
		}
	}
	if (column == TIndex::EmptyIndex)
		return false;
	return ColoringRowCell(curRow, column, fing_flg, true);
}


bool TJsonReport::CellHasChanged(const TExtendSheetReport& srcSheet, const NS_Tune::TCellData& param,
	size_t dstRow, size_t srcRow) const noexcept(true)
{
	using NS_Excel::TExcelCell;
	//��������� ������ ���������:
	size_t tmpCol = param.InsIndex();
	TExcelCell dstCell(dstRow, tmpCol, false);
	//��������� ������ ���������:
	tmpCol = param.SrcVal();
	TExcelCell srcCell(srcRow, tmpCol, false);
	return srcSheet.NotEquality(sheet, dstCell, srcCell);
}

bool TJsonReport::InsertDstCellBySrcCell(const TExtendSheetReport& srcSheet, const NS_Tune::TCellData& param,
	size_t dstRow, size_t srcRow) noexcept(true)
{
	using NS_Excel::TExcelCell;
	size_t tmpCol = param.InsIndex();
	//������ ���������:
	TExcelCell DstCell(dstRow, tmpCol, false);
	//������ ���������
	tmpCol = param.SrcVal();
	TExcelCell SrcCell(srcRow, tmpCol, false);
	//��������� ��������� ������ � ������ ��������� �� ������ �������� ���������:
	return srcSheet.setDstCellBySrcCell(sheet, DstCell, SrcCell);
}

bool TJsonReport::procFindCell(const TExtendSheetReport& srcSheet, const NS_Tune::TCellData& param,
	size_t dstRow, size_t srcRow) noexcept(true)
{
	using NS_Const::JSonMeth;
	using NS_Const::TConstJSMeth;
	//���� ��������� ��� ������� ������ � ������ �� ���������:
	if (param.EmptyInsIndx() or param.EmptySrcVal())
	{
		TLog("������ ��������� ��� ������� ������ � ������!", "TJsonReport::procFindCell").toErrBuff();
		return false;
	}
	bool color_flg = true;
	switch (meth_code)
	{
		case JSonMeth::CompareCellChange:
		{
			if (CellHasChanged(srcSheet, param, dstRow, srcRow) == false)	break;
		}
		case JSonMeth::CompareRow:
		case JSonMeth::CompareCell:
		case JSonMeth::InsertRowCompare:
		{
			//������� ������:
			return InsertDstCellBySrcCell(srcSheet, param, dstRow, srcRow);
		}
		default:
		{
			TLog log("��������� �����: ", "TJsonReport::procFindCell");
			log << TConstJSMeth::asStr(meth_code) << " �� ��������������!\n";
			log.toErrBuff();
		}
	}
	return false;
}

bool TJsonReport::procFindRow(const TExtendSheetReport& srcSheet, const CellDataArr& params,
	size_t dstRow, size_t srcRow) noexcept(true)
{
	using NS_Tune::TCellData;
	using NS_Excel::TExcelCell;
	//��� ������: ������ ��� ��������� - 1 ��������, 2 ��������
	using POutParams = std::vector<const TCellData*>;
	//��������� ������ �� �������� ���������� ����� �����-���������
	POutParams outArr;
	for (const TCellData& param : params)
	{
		if (param.EmptyInsIndx() or param.EmptySrcVal()) continue;
		TExcelCell tmpCell(srcRow, param.SrcVal(), false);
		if (srcSheet.isEmptyCell(tmpCell) == false)
			outArr.push_back(&param);
	}
	//���� �������� ���������� ��� -�����:
	if (outArr.empty()) return true;
	//��������� ����� ������:
	if (InsNewRow(dstRow, dstRow) == true)
		dstRow = dstRow + 1;
	else
		return false;
	size_t cnt = 0;
	for (const TCellData* param : outArr)
	{
		if (procFindCell(srcSheet, *param, dstRow, srcRow) == true)
			cnt++;
	}
	//��������� ������� ������
	bool flg = (cnt == outArr.size()) ? true : false;
	//����������� ������ ������������� � ����� ������
	ColoringRowCell(dstRow, getColID(), flg, true);
	return flg;
}

bool TExtendSheetReport::getDstCell_In_SrcCell(const TExtendSheetReport& srcSheet, 
	const NS_Excel::TExcelCell& DstCell, const NS_Excel::TExcelCell& SrcCell,
	bool NoSpaceNoCase) const noexcept(true)
{
	using NS_Excel::TExcelCell;
	try
	{
		//��������� ���� �� ������ � ��������� ������:
		if (sheet.isEmptyCell(DstCell)) return false;
		//��������� ������ �� ������ ���������:
		return srcSheet.CheckInCell(sheet, DstCell, SrcCell, NoSpaceNoCase);
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("�� ������������ ������ ��� ������ ������:", "Get_DstCell_In_SrcRow");
		log << DstCell.getName() << " � ������ ���������: " << SrcCell.getName() << '\n';
		log.toErrBuff();
	}
	return false;
}

size_t TExtendSheetReport::getSrcRow_By_Cell(const TExtendSheetReport& srcSheet, NS_ExcelReport::TRowsFlag& srcRows,
	const NS_Excel::TExcelCell& DstCell, size_t SrcCol, bool NoSpaceNoCase) const noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_Tune::TIndex;
	//��������� ���� �� ������ � ��������� ������:
	if (sheet.isEmptyCell(DstCell)) return TIndex::EmptyIndex;
	//��������� ����� ������ �� ����� ���������:
	return srcSheet.CheckOnSheet(sheet, DstCell, SrcCol, srcRows, NoSpaceNoCase);
}

size_t TExtendSheetReport::getSrcRow_By_Params(const TExtendSheetReport& srcSheet, NS_ExcelReport::TRowsFlag& srcRows,
	const NS_Tune::CellDataArr& params, size_t curRow, size_t& param_index, bool NoSpaceNoCase) const noexcept(true)
{
	using NS_Tune::TCellData;
	using NS_Excel::TExcelCell;
	using NS_Tune::TIndex;
	//�������� �� ���������� � ��������� ����� ������(���������� ��� ��������� ������ ����������):
	for (; param_index < params.size(); param_index++)
	{
		//���� ��� ������ �� �������� �� ������ ���������/��������� - ��������� � ��������� ������
		if (params[param_index].EmptyDstIndx() and params[param_index].EmptySrcParam()) continue;
		//������������� ������ ���������:
		TExcelCell DstCell(curRow, params[param_index].DstIndex(), false);
		//��������� ����� ������ � ��������� ��� ������ ���������:
		return getSrcRow_By_Cell(srcSheet, srcRows, DstCell, params[param_index].SrcParam(), NoSpaceNoCase);
	}
	return TIndex::EmptyIndex;
}

size_t TExtendSheetReport::getSrcRow_By_Dest_Params(const TExtendSheetReport& srcSheet,
	NS_ExcelReport::TRowsFlag& srcRows, const NS_Tune::CellDataArr& params, size_t curRow,
	bool NoSpaceNoCase) const noexcept(true)
{
	using NS_Tune::CellDataArr;
	using NS_Tune::TCellData;
	using NS_Excel::TExcelCell;
	using NS_Tune::TIndex;
	using NS_ExcelReport::TRowsFlag;
	//������ ������� ��������� ��� ������
	size_t param_index = 0;
	//��������� ����� ������ ��������� �� ������ ������ ���������
	size_t srcRow = getSrcRow_By_Params(srcSheet, srcRows, params, curRow, param_index, NoSpaceNoCase);
	//������� ���� ������ �� �������
	if (srcRow == TIndex::EmptyIndex) return TIndex::EmptyIndex;
	//���� ������ ������� - ���������� ������ ��� ����� ������ ��������� � �������� ������ ���������
	for (++param_index; param_index < params.size(); param_index++)
	{
			//�������������� ������ ���������
			TExcelCell DstCell(curRow, params[param_index].DstIndex(), false);
			//������������� ������ ���������
			TExcelCell SrcCell(srcRow, params[param_index].SrcParam(), false);
			//���� �� ������� ���� ���� ������ ������: ������ ��������� - �� �������
			if (getDstCell_In_SrcCell(srcSheet, DstCell, SrcCell, NoSpaceNoCase) == false)
				return TIndex::EmptyIndex;
	}
	return srcRow;
}

bool TJsonReport::Proc_DstRowCells_In_SrcSheet(const TExtendSheetReport& srcSheet, NS_ExcelReport::TRowsFlag& srcRows,
	const CellDataArr& params, size_t curRow, bool NoSpaceNoCase) noexcept(true)
{
	using NS_Tune::CellDataArr;
	using NS_Tune::TCellData;
	using NS_Excel::TExcelCell;
	using NS_Tune::TIndex;
	using NS_ExcelReport::TRowsFlag;
	//������� �� ��������� �����:
	int failCnt = 0, col_cnt = 0, ChngCnt = 0;
	//��� ������ ������ � ����� ��������� - ������������ ��������� � json-����� �������
	for (const TCellData& param : params)
	{
		//���� ��� ������ �� �������� �� ������ ���������/��������� - ��������� � ��������� ������
		if (param.EmptyDstIndx() and param.EmptySrcParam()) continue;
		//������������ ������ ���������
		size_t tmpCol = param.DstIndex();
		TExcelCell dstCell(curRow, tmpCol, false);
		//���� ������ � ��������� ��� ��������� - ������ - ���� ������
		if (sheet.isEmptyCell(dstCell)) continue;
		col_cnt++;
		//��������� ����� �� ����� ���������:
		tmpCol = param.SrcParam();
		size_t srcRow = srcSheet.CheckOnSheet(sheet, dstCell, tmpCol, srcRows, NoSpaceNoCase);
		//���� �������� �� ������ � ������ ���������:
		if (srcRow == TIndex::EmptyIndex)
		{
			failCnt++;
			//������� ������� �����, ���� ������ �� ��������
			ColoringRowCell(dstCell, false, true);
		}
		else
		{
			//�������� ��������� ������ �� ������� ����� ���������:
			srcRows[srcRow] = false;//srcRows.erase(srcRow);
			//������������ ��������� ������ �  �������:
			bool procFlg = procFindCell(srcSheet, param, curRow, srcRow);
			//���� �������������� ������ ������� ����������
			//����������� ������� ���������� �����
			if (procFlg) ChngCnt++;
			//������� ������ � ����������� �� ����������� ������:
			ColoringRowCell(dstCell, true, procFlg);
		}
	}
	//��������� ������ � ����������� �� ������ ���������:
	bool FindFlg = cells_data.getMethod().isSuccess(col_cnt, failCnt);;
	//�������� ������:
	ColoringRowByFlg(curRow, FindFlg, ChngCnt > 0);
	return FindFlg;
}

bool TJsonReport::Execute_Seek_Dst_Cell_In_Src_Sht(const TExtendSheetReport& srcSheet, TRowsFlag& DstRows,
	TRowsFlag& SrcRows, const CellDataArr& params, bool NoSpaceNoCase) noexcept(true)
{
	using NS_ExcelReport::TRowFlag;
	using NS_Excel::TExcelCell;
	using NS_ExcelReport::TCellData;
	size_t lastRow = DstRows.rbegin()->first;
	//�������� �� ������ ������ �����-���������:
	for (const TRowFlag& index : DstRows)
	{
		//�� ������������ ������, ������� ���� ���������� �����
		if (index.second == false) continue;
		//������� ������ ��������� � ���������
		TLog log("���� ���������: ");
		log << index.first << " ������� �� " << lastRow << " �����";
		if (Proc_DstRowCells_In_SrcSheet(srcSheet, SrcRows, params, index.first, NoSpaceNoCase) == true)
			DstRows[index.first] = false;//��� �������� �� ������, ���� ��������� ������ � ����� ���������
	}
	TLog("��������� ���������!").toErrBuff();
	return true;
}

bool TJsonReport::Inverse_Dst2Src_Param(const NS_Tune::CellDataArr& old_params, NS_Tune::CellDataArr& new_param) 
	noexcept(false)
{
	using NS_Tune::TCellData;
	if (old_params.empty()) return false;
	try
	{
		for (const TCellData& param : old_params)
		{
			//������� ������ ����������
			if (param.isEmpty()) continue;
			//������������� ���������
			new_param.push_back(param.Inverse_Src2Dst());
		}
		if (new_param.size() > 0) return true;
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ��� �������� ����������!", "Inverse_Dst2Src_Param").toErrBuff();
	}
	return false;
}

size_t TJsonReport::get_SrcRow_By_Dst_Params(const TJsonReport& Dst, const TExtendSheetReport& Src,
	TRowsFlag& DstRows, TRowsFlag& SrcRows, const NS_Tune::CellDataArr& original_params,
	bool NoSpaceNoCase) noexcept(false)
{
	using NS_ExcelReport::TRowsFlag;
	using NS_ExcelReport::TRowFlag;
	using NS_Tune::TIndex;
	using NS_Tune::CellDataArr;
	//���������� ������������ �������� ��������� � ���������
	//���� ����� ����� � ��������� ������, ��� � ��������� - ������ �� �������
	bool ReverseFlg = SrcRows.size() < DstRows.size();
	//����� �������� �� �������� �� �������� � ��������
	const TExtendSheetReport* DstPtr = ReverseFlg ? &Src : &Dst;
	const TExtendSheetReport* SrcPtr = ReverseFlg ? &Dst : &Src;
	//���������� ������� ����� ��� ��������� � ���������:
	TRowsFlag* srcRowsPtr = ReverseFlg ? &DstRows : &SrcRows;
	TRowsFlag* dstRowsPrt = ReverseFlg ? &SrcRows : &DstRows;
	//��������� �������� ���������� ������ ������ ��� ��������� � ���������:
	CellDataArr params;
	if (ReverseFlg)
		Inverse_Dst2Src_Param(original_params, params);
	else
		params = original_params;
	//���������� ������ ������
	//������� ����� ����� � ���������:
	size_t count = 0;
	//����������� ����� �����, ��������� ��������(������ ��������������� �������):
	size_t lastRow = dstRowsPrt->rbegin()->first;
	//��������� ������ �� ������ ������ �������� �������:
	for (const TRowFlag& index : *dstRowsPrt)
	{
		//������� ����� ������������ �����
		if (index.first == false) continue;
		//������� ������ � ������ ����������� ����� � ��������:
		size_t curRow = index.first + count;
		TLog log("���� ���������:");
		log << curRow << " ������ �� " << lastRow << " �����";
		//���������� ������ ������ ��������� � ���������:
		size_t srcRow = DstPtr->getSrcRow_By_Dest_Params(*SrcPtr, *srcRowsPtr, params, curRow, NoSpaceNoCase);
		if (srcRow == TIndex::EmptyIndex) continue;
		//���� ������ ������� �� ���������� ����������:

		return srcRow;
	}
	return TIndex::EmptyIndex;
}


bool TJsonReport::Execute_Seek_Dst_Row_In_Src_Sht(const TExtendSheetReport& srcSheet, TRowsFlag& DstRows,
	TRowsFlag& SrcRows, const CellDataArr& params, bool NoSpaceNoCase) noexcept(false)
{
	using NS_Const::JSonMeth;
	using NS_ExcelReport::TRowsFlag;
	using NS_ExcelReport::TRowFlag;
	using NS_Tune::TIndex;
	using NS_Tune::CellDataArr;
	//���������� ������������ �������� ��������� � ���������
	//���� ����� ����� � ��������� ������, ��� � ��������� - ������ �� �������
	bool ReverseFlg = SrcRows.size() < DstRows.size();
	//����� �������� �� �������� �� �������� � ��������
	const TExtendSheetReport* DstPtr = ReverseFlg ? &srcSheet : this;
	const TExtendSheetReport* SrcPtr = ReverseFlg ? this : &srcSheet;
	//���������� ������� ����� ��� ��������� � ���������:
	TRowsFlag* dstRowsPrt = ReverseFlg ? &SrcRows : &DstRows;
	TRowsFlag* srcRowsPtr = ReverseFlg ? &DstRows : &SrcRows;
	//��������� �������� ���������� ������ ������ ��� ��������� � ���������:
	CellDataArr tmp_params;
	if (ReverseFlg)
		Inverse_Dst2Src_Param(params, tmp_params);
	else
		tmp_params = params;
	//���������� ������ ������
	//������� ����� ����� � ���������:
	size_t count = 0;
	size_t line = 0;
	//����������� ����� �����, ��������� ��������(������ ��������������� �������):
	size_t lastRow = dstRowsPrt->rbegin()->first;
	//��������� ������ �� ������ ������ �������� �������:
	for (const TRowFlag& index : *dstRowsPrt)
	{
		//������� ����� ������������ �����
		if (index.first == false) continue;
		line++;
		//������� ������ � ������ ����������� ����� � ��������:
		size_t curRow = index.first;
		//���� �������� �� ���� - ������������ ���� � ������� ��������� ������
		//�������������� ����������� ������� �� ����� ����������� �����
		if (!ReverseFlg) curRow += count;
		TLog log("���� ���������: ");
		log << line << " ������ (" << curRow << '\\' << lastRow << ")";
		log.toErrBuff();
		//���������� ������ ������ ��������� � ���������:
		size_t srcRow = DstPtr->getSrcRow_By_Dest_Params(*SrcPtr, *srcRowsPtr, tmp_params, curRow, NoSpaceNoCase);
		if (srcRow == TIndex::EmptyIndex)
		{
			TLog log("������ ");
			log << curRow << " �� ������� � ����� ���������!";
			log.toErrBuff();
			continue;
		}
		else
		{
			//���� ������ ������� �� ���������� ����������:
			//� ����������� �� ����� �������� ��������� � ���������:
			//�������������� ��������� � ������� ������ ��� ��������� � ���������
			size_t DstCurRow = ReverseFlg ? srcRow : curRow;
			size_t SrcCurRow = ReverseFlg ? curRow : srcRow;
			//������� ������ ��������� �� ����������� ������:
			SrcRows[SrcCurRow] = false;
			//������� ������ ��������� �� ���������:
			DstRows[DstCurRow] = false;
			//��������� ��������� ������ ��������� - ����� ��� ���� ������� ������:
			if (procFindRow(srcSheet, params, DstCurRow, SrcCurRow))
				//����������� ������� ������������ �����
				count++;
			else
			{
				TLog log("��� ��������� ");
				log << DstCurRow << " ������ ��������� ��� " << SrcCurRow << " ������ ��������� ��������� ������!";
				log.toErrBuff();
			}
		}
	}
	return true;
}

bool TJsonReport::Search_DestData_In_SrcSheet(TRowsFlag& DstRows, 
	const TExtendSheetReport& srcSheet, bool NoSpaceNoCase) noexcept(true)
{
	using NS_ExcelReport::TRowsFlag;
	using NS_Const::JSonMeth;
	using NS_Const::TConstJSMeth;
	try
	{
		//�������� ������ ������������ ����������:
		CellDataArr cellArr = cells_data.getCellDataArr();
		if (cellArr.empty())
			throw TLog("������ ������� ������� ��� ���������!", "TJsonReport::Search_DestData_In_SrcSheet");
		//��������� ������ ����� ��������� ��� ��������� - ��������� �� 1:
		TRowsFlag srcRows = srcSheet.setFiltredRowsArr();
		if (srcRows.empty())
			throw TLog("�� �������� ����� ��������� ��� ������ ���������� ��� �������!",
				"TJsonReport::Search_DestData_In_SrcSheet");
		//� ����������� �� ������ �������� ������ ����� ������:
		switch (meth_code)
		{
		case JSonMeth::CompareCell:
		case JSonMeth::CompareCellChange:
		{
			//����� ������ ��������� ������ ��������� �� �������� ����� ���������
			Execute_Seek_Dst_Cell_In_Src_Sht(srcSheet, DstRows, srcRows, cellArr, NoSpaceNoCase);
			break;
		}
		case JSonMeth::CompareRow:
		case JSonMeth::InsertRowCompare:
		{
			Execute_Seek_Dst_Row_In_Src_Sht(srcSheet, DstRows, srcRows, cellArr, NoSpaceNoCase);
			break;
		}
		default:
		{
			TLog log("��������� �����(", "Search_DestRow_In_SrcSheet");
			log << TConstJSMeth::asStr(meth_code) << ") �� ��������������!";
			throw log;
		}
		}
		return true;
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ������ ������ �� ��������-��������� � ��������-���������!",
			"TJsonReport::Search_DestRow_In_SrcSheet").toErrBuff();
	}
	return false;
}

void TJsonReport::Compare_Excel_Sheets(bool NoSpaceNoCase) noexcept(false)
{
	using NS_ExcelReport::TRowsFlag;
	using NS_Tune::TShareData;
	//�������� ���������� excel-�����
	if (!book.isValid()) throw TLog("����� �� ����������������!", "TJsonReport::Compare_Excel_Sheets");
	//��������� ������ �� ������������ �����, ������� ������������� �������� ����������:
	//��������� ����� ���� � ������� ����� - �� 1
	TRowsFlag DestRows = TExtendSheetReport::setFiltredRowsArr();
	if (DestRows.empty()) throw TLog("�� ����� �� ������� ����� ��������������� �������!", "TJsonReport::Compare_Excel_Sheets");
	//�������� ����� ������ � ����� ���������:
	size_t srcPageCnt = 0;
	//������ � ����� ���������:
	const TShareData* src_file_data = cells_data.getSrcFilRef();
	if (!src_file_data) throw TLog("�� ������� ������ � ����� ���������!", "TJsonReport::Compare_Excel_Sheets");
	srcPageCnt = src_file_data->getPageCnt();
	//��������� ����� �����-���������
	string src_name = src_file_data->getName();
	//������������� ����� ��� �����-���������
	TExcelBook src_book(src_name);
	//�������� �� ������� ����� ����� ���������:
	for (size_t i = 0; i < srcPageCnt; i++)
	{
		//������������� ����� �����-���������
		TExtendSheetReport src(src_book, *src_file_data, i);
		//��������� ����� ������ ��� ������ �����-��������� �� ����� �����-���������:
		if (Search_DestData_In_SrcSheet(DestRows, src, NoSpaceNoCase) == false)
		{
			TLog("��� ������ �������� ���������: " + getSheetName() + " � �������� ���������: " +
				src.getSheetName() + " ��������� ������!", "TJsonReport::Compare_Excel_Sheets").toErrBuff();
		}
	}
}

bool TJsonReport::crtSheetReport() noexcept(true)
{
	using NS_Const::JSonMeth;
	try
	{
		//���������� ���������� �������� � ����������� �� ������ ���������:
		switch (meth_code)
		{
		case JSonMeth::CompareRow:
		case JSonMeth::CompareCell:
		case JSonMeth::InsertRowCompare:
		case JSonMeth::CompareCellChange:
			Compare_Excel_Sheets();
			break;
		case JSonMeth::GetFromDB:
		case JSonMeth::GetRowIDByDB:
		case JSonMeth::SendToDB:
		{
			ProcessSheetDataWithDB();
			break;
		}
		}
		return true;
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("�������������� ������ ��� ������������ �������� ������ �����!", 
			"TJsonReport::crtSheetReport").toErrBuff();
	}
	return false;
}

std::string TJsonReport::getServerNameByTuneIndex(size_t val) const noexcept(true)
{
	using NS_Tune::TUserTune;
	using NS_Const::TuneField;
	vector<TUserTune> arr = cells_data.getDBTuneArr();
	if (arr.size() > val) return arr[val].getFieldValueByCode(TuneField::TNS);
	return string();
}

bool TJsonMarkReport::CorrectFilter(size_t cur_row) const noexcept(true)
{
//�������� ����� ����� �������� ������ ������ ������� json-�����
//	if (getMethCode() != NS_Const::JSonMeth::GetRowIDByDB and procRows.second.size() > 0)
//		return true;
	return TJsonReport::CorrectFilter(cur_row);
}

bool TJsonMarkReport::checkINDataBase(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false)
{
	//���� ������ ������ ������
	bool f = TJsonReport::checkINDataBase(rs, curRow);
	TRowFlag row_flg = TRowFlag(curRow, f);
	procRows.second.insert(row_flg);
	return f;
}

void TJsonMarkReport::setExcelDataByDB(TStatement& query, size_t& rowFrom) noexcept(false)
{
	using NS_ExcelReport::TRowFlag;
	//��������� ����� �������� � ��:
	//TDataBaseSheetReport::setMaxIterationCnt(query, getMaxIterationCnt());
	if (procRows.second.empty())
		TJsonReport::setExcelDataByDB(query, rowFrom);
	else
	{
		for (const TRowFlag& row : procRows.second)
			if (row.second == flg) TJsonReport::setExcelRowDataByBD(query, row.first);
	}
}

void TJsonMarkReport::UpdExcelDataByDB(NS_Oracle::TDBConnect& db, const NS_Tune::TUserTune& tune, size_t& rowFrom) noexcept(false)
{
	using NS_Const::TuneField;
	if (tune.getFieldValueByCode(TuneField::TNS) == procRows.first)
		flg = true;
	else
		flg = false;
	TJsonReport::UpdExcelDataByDB(db, tune, rowFrom);
}

TJsonMarkReport::TJsonMarkReport(TExcelBook& book_ref, const TExcelProcData& json_tune, size_t page, 
	NS_ExcelReport::TRowsTNS& Rows) :	TJsonReport(book_ref, json_tune, page), procRows(Rows), flg(false)
{
	//���� ��� ������ ��� ���������� - �������� ��� ������� � �������� ��� ����������
	if (isFilterObject())	procRows.first = getServerNameByTuneIndex(0);
}

bool TJsonMarkReport::isFilterObject() const noexcept(true)
{
	if (getTuneFilesCnt() == 1 and procRows.second.empty())
		return true;
	return false;

}

std::string TJsonMarkReport::getFlgName() const noexcept(true)
{
	if (isFilterObject())
		return getServerNameByTuneIndex(0);
	return string();
}

TSheetTuneReport::TSheetTuneReport(NS_Excel::TExcelBook& book_link, const NS_Tune::TUserTune& config): 
	TBaseSheetReport(book_link, nullptr), tune(config)
{
	SetSheetByTune();
}

TSheetTuneReport::TSheetTuneReport(TExcelBook& ref_book, const NS_Excel::TExcelBookSheet& ref_sheet, const NS_Tune::TUserTune& ref_tune):
	TBaseSheetReport(ref_book, ref_sheet), tune(ref_tune)
{
	//���� �������� ������:
	if (!ref_sheet.isValid())
		//������� ��
		SetSheetByTune();
}

bool TSheetTuneReport::SetSheetByTune(const string& name) noexcept(true)
{
	using NS_Excel::TStrArr;
	const bool active_sheet = true;
	//�������� ������������� ����������
	if (tune.Empty())
	{
		TLog("������ ������ �� ����� ��������!", "TSheetTuneReport::SetSheetTune").toErrBuff();
		return false;
	}
	//��������� ������ �� ������
	string tmp_val = tune.getTemplateFile();
	string sh_name = name;
	if (sh_name.empty()) sh_name = tune.getOutSheet();
	//���� ������ �� ������ - ������� ��������
	if (tmp_val.empty())
	{
		//��������� ������������ 
		TStrArr head = tune.getColumns();
		book.setHeaderByStrArr(head, false, sh_name);
		sheet = book.getActiveSheet();
	}
	//���� ������ ������ - ������ ���
	else
	{
		if (book.setSheetByTemplate(tmp_val, sh_name, DEF_TEMPL_SH_INDX, active_sheet))
		{
			//������������� �������� �����:
			sheet = book.getActiveSheet();
			//������������� �������� ����� ������ ����� �������:
			book.setAsTemplate(true);
			//������������� �������� ����� �������:
			initRowFormat();
		}
	}
	if (!sheet.isValid())
		return false;
	return true;
}

bool TSheetTuneReport::CreateNewPage(size_t cur_val, bool byRows)
{ 
	//�� ����� �������� ����� �������� �� ���������!!!
	if (NeedNewPage(cur_val, byRows))
	{
		int indx = book.getActiveSheetIndx() + 1;
		string sh_delimeter = NS_Const::TConstExclTune::asStr(NS_Const::TExclBaseTune::PageDelimiter);
		//������ ��� ����� ��������
		string new_name = sheet.getName() + sh_delimeter + NS_Converter::toStr(indx+1);
		//���������� ��������
		SetSheetByTune(new_name);
		return true;
	}
	return false;
}

TConnectParam TDataBaseInterface::getConnectParam(const NS_Tune::TUserTune& param, int prefetch) noexcept(false)
{
	TConnectParam result;
	if (param.Empty()) throw TLog("��������� ������ �� ���������!", "TDataBaseSheetReport::getConnectParam()");
	result.username = param.getFieldValueByCode(TuneField::UserName);
	result.password = param.getFieldValueByCode(TuneField::Password);
	result.tns_name = param.getFieldValueByCode(TuneField::TNS);
	result.auto_commit = false;
	result.prefetch_rows = prefetch;
	return result;
}

void TDataBaseInterface::setSqlParamByTune(NS_Oracle::TStatement& sql, const NS_Tune::TSubParam& param, bool use_raise) noexcept(false)
{
	using NS_Tune::DataType;
	using NS_Converter::toType;
	using NS_Converter::toDblType;
	using NS_Converter::toBoolType;
	const DataType& data_type = param.DataType();
	//���� �������� ��������� - ������:
	if (param.Value().empty())
	{
		sql.setNullVal(param.ID(), TDataBaseInterface::convertDataType(data_type));
		return;
	}
	//��������� ������� �� ������ ��� ��������:
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
			toDblType(param.Value(), &val);
			sql.setDoubleVal(par_id, val);
			return;
		}
		case DataType::Date:
			sql.setDateAsStringVal(par_id, param.Value());
			return;
		case DataType::String:
			sql.setStringVal(par_id, param.Value());
			return;
		case DataType::SQL_String:
			sql.setSqlStringVal(par_id, param.Value());
			return;
		default: 
			throw TLog("��������� ��� ������ " + NS_Const::TConstType(param.DataType()).toStr() + " �� ��������������!", "TDataBaseSheetReport::setSqlParamByTune");
		}
	}
	catch (const oracle::occi::SQLException& err)
	{
		TLog log("������ ��������� ��������: " + param.Value() + " ��� ���������: " + param.Comment() + TLog::NL, "TDataBaseSheetReport::setSqlParamByTune");
		log << err.what() << TLog::NL;
		raise_app_err(log, use_raise);
	}
	catch (const TLog& er)
	{
		raise_app_err(er, use_raise);
	}
	catch (...)
	{
		TLog log("�������������� ������ ��������� ��������: " + param.Value() + " ��� ���������: " + param.Comment() + TLog::NL);
		raise_app_err(log, use_raise);
	}
}

void TDataBaseInterface::setSqlParamsByTune(NS_Oracle::TStatement& sql, const NS_Tune::TUserTune& param) noexcept(false)
{
	//���� ������ sql - �����
	if (!sql.isValid()) throw TLog("�� �������� sql-�������: " + sql.getSQL(), "setSqlParamsByTunes");
	string sql_text = sql.getSQL();
	if (sql_text.empty()) throw TLog("������ ����� sql-�������", "setSqlParamsByTunes");
	//��������� ���������� ����������:
	for (const NS_Tune::TSubParam& p : param.getParams())
		setSqlParamByTune(sql, p);
}

NS_Const::DataType TDataBaseInterface::convertOraType(const NS_Oracle::TType& type) noexcept(true)
{
	using NS_Oracle::TType;
	using NS_Const::DataType;
	using NS_Const::TConstType;
	switch (type)
	{
	//����� � ��������� ������:
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
	case TType::OCCISTRING:
		return DataType::String;
	case TType::OCCIBOOL:
		return DataType::Boolean;
	case TType::OCCIDATE:
	case TType::OCCI_SQLT_DATE:
	case TType::OCCI_SQLT_DAT:
		return DataType::Date;
	}
	return DataTypeError;
}

NS_Oracle::TType TDataBaseInterface::convertDataType(const NS_Const::DataType& type) noexcept(true)
{
	using NS_Oracle::TType;
	using NS_Const::DataType;
	using NS_Const::TConstType;
	using std::make_pair;
	switch (type)
	{
	//����� � ��������� ������:
	case DataType::Integer:
		return TType::OCCIINT;
	case DataType::Double:
		return TType::OCCIDOUBLE;
	case DataType::String:
		//return TType::OCCI_SQLT_STR;
		return TType::OCCISTRING;
	case DataType::Boolean:
		return TType::OCCIBOOL;
	case DataType::Date:
		return TType::OCCIDATE;
	}
	return OraTypeError;
}

void TDataBaseSheetReport::setCellByResultSet(const NS_Oracle::TResultSet& rs, const NS_Excel::TExcelCell& cell) noexcept(false)
{
	using NS_Const::DataType;
	using NS_Excel::FormatPtr;
	//��-�� ������������ ���������� ������� � oracle � excel
	size_t resultSetCol = cell.getCol() + 1;
	//��������� ���� ���� ������ ��� ������ � ������:
	DataType code = convertOraType(rs.getColumnType(resultSetCol));
	//��������� ������ �� ������ ������:
	FormatPtr format = getCellFormatPtr(cell);
	TDataBaseInterface::setCellByResultSet(book, sheet, code, rs, resultSetCol, cell, format);
}

bool TDataBaseSheetReport::WriteFromResultSet(NS_Oracle::TResultSet& rs) noexcept(true)
{
	using NS_Oracle::UInt;
	using NS_Excel::TExcelCell;
	try
	{
		UInt col_cnt = rs.getColumnsCnt();
		//���� ����� ������� ��������� ����������� excel - �������
		CreateNewPage(col_cnt, false);
		size_t row = LastRow();
		while (rs.Next())
		{
			for (UInt i = 1; i <= col_cnt; i++)
			{
				TExcelCell cell(row, i, false);
				setCellByResultSet(rs, cell);
			}
			row++;
			//���� ��������� ����� ����� �� ��������
			//������� ����� ��������
			if (CreateNewPage(row, true)) 
				row = LastRow() + 1;
			//��������� �������� ��� ����� ����� ������:
			setRowCellsFormat(row);
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
		TLog("������������� ������ ������ ������ �� ���� �� ���� excel-���������!", "TDataBaseSheetReport::WriteFromResultSet").toErrBuff();
	}
	return false;
}

void TDataBaseSheetReport::FillSheetBySql(NS_Oracle::TDBConnect& db, const string& sql_txt, int prefetch)
{
	if (sql_txt.empty())
		throw TLog("������ ������������ ������ - ������ ����� �������!", "TDataBaseSheetReport::FillSheetBySql");
	if (!db.isValid())
		throw TLog("������ ������������ ������ - ������ ��� ����������� � ��!", "TDataBaseSheetReport::FillSheetBySql");
	//�������� sql-�������:
	TStatement st(db, sql_txt, prefetch);
	//������������� ��������� �������:
	setSqlParamsByTune(st, tune);
	//��������� ������:
	TResultSet rs(st);
	//������� ������ ������ � excel �� ���������� �������:
	WriteFromResultSet(rs);
	//��������� �������� ������ �������:
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

/*
string TDataBaseInterface::getSqlByTune(bool use_parse, bool by_str, const string& str) noexcept(true)
{
	using NS_Tune::TUserTune;
	string sql = TUserTune::getSqlString(use_parse, by_str, str);
	return sql;
}
/**/

string TDataBaseInterface::getSqlByTune(const NS_Tune::TUserTune& tune, bool asDQL) noexcept(true)
{
	using NS_Const::TuneField;
	using NS_Tune::TSimpleTune;
	//���� ��������� ������ - �������
	if (tune.Empty()) return string();
	bool parserFlg = false;
	bool byFileFlg = false;
	parserFlg = tune.useFlag(TuneField::UseSqlParser);
	string str;
	if (asDQL)
		str = tune.getDQLText();
	else
		str = tune.getDMLText();
	return str;
}


void TDataBaseSheetReport::CrtBySqlLine(NS_Oracle::TDBConnect& db, const string& sql_line, int prefetch) noexcept(false)
{
	using NS_Tune::TUserTune;
	bool use_parse = useSqlParse();
	string sql = TUserTune::getSqlString(use_parse, true, sql_line);
	//������������ ������ ��� ��������
	FillSheetBySql(db, sql, prefetch);
}

void TDataBaseSheetReport::CrtBySqlFiles(NS_Oracle::TDBConnect& db, int prefetch) noexcept(false)
{
	using std::vector;
	using NS_Tune::TUserTune;
	vector<string> sql_lst = tune.getDQLFileLst();
	if (sql_lst.size() < 1)
		throw TLog("������ ����� sql-�������� � ����������!", "TDataBaseSheetReport::CrtBySqlFiles");
	bool use_parse = useSqlParse();
	//���������� ������� ������� �� ������:
	for (const string& sql_file : sql_lst)
	{
		string sql = TUserTune::getSqlString(use_parse, false, sql_file);
		//���������� DQL-������� � ���������� ������� �����
		FillSheetBySql(db, sql, prefetch);
	}
}

size_t TDataBaseInterface::executeDML(NS_Oracle::TDBConnect& db, const NS_Tune::TUserTune& param,
	const string& dml, bool use_comit) noexcept(false)
{
	//��������� ������ �������:
	//�������� dml-�������:
	TStatement st(db, dml);
	//������������� ��������� �������:
	setSqlParamsByTune(st, param);
	//���������� �������:
	size_t result = 0;
	result = st.executeDML();
	if (result > 0 and use_comit)
	{
		TLog log("������ ", "executeDML");
		log << dml << "\n��������� " << result << " �����!\n";
		log.toErrBuff();
		st.Commit();
	}
	return result;
}

size_t TDataBaseInterface::runDML4Directory(NS_Oracle::TDBConnect& db, const NS_Tune::TUserTune& param,
	bool use_comit) noexcept(false)
{
	using std::vector;
	using NS_Tune::TUserTune;
	//��������� ������ ������:
	vector<string> dml_lst = param.getDMLFileLst();
	if (dml_lst.size() < 1)
		throw TLog("������ ����� dml-������ � ����������!", "TDataBaseSheetReport::runDML4Directory");
	//������� ������ ����� ������������ �����:
	size_t cnt = 0;
	//���������� ������� ������� �� ������:
	for (const string& dml : dml_lst)
	{
		size_t tmp = 0;
		//��������� ������ dml-�������
		string dml_txt = TUserTune::getSqlString(false, false, dml);
		//���������� dml-�������
		tmp = executeDML(db, param, dml_txt, use_comit);
		if (tmp == 0)
			TLog("������ " + dml + " �� ������ ����������!", "TDataBaseSheetReport::runDML4Directory").toErrBuff();
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
			throw TLog("������ ������������ ������ - ������ ���� ��������!", "TDataBaseSheetReport::crtReport");
		if (!sheet.isValid())
			throw TLog("������ ������������ ������ - �� ������� �������� ��� ������!", "TDataBaseSheetReport::crtReport");
		if (tune.isDQLEmpty())
			throw TLog("�� ������� �� ������ SQL-�������!", "TDataBaseSheetReport::crtReport");
		//��������� �������� ��� ����������� � ��:
		TConnectParam cp = getConnectParam(tune, prefetch_rows);
		//������������ � ��:
		TDBConnect db(cp);
		bool dql_first = isDQLFirst();
		//���� ������ ���� ��������� DML-�������
		if (!dql_first) runDML(db, tune);
		//��������� ����������� �� sql-������� �� �����
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
		TLog("����� �� �����������! �� ������������ ������!", "TDataBaseSheetReport::crtReport").toErrBuff();
	}
	return false;
}

//���������� DML �������:
size_t TDataBaseInterface::runDML(NS_Oracle::TDBConnect& db, const NS_Tune::TUserTune& param, bool use_comit) noexcept(true)
{
	try
	{
		//�������� ���������� � ����� ��������
		if (!db.isValid()) throw TLog("����������� � �� �� �������!", "TDataBaseSheetReport::runDML");
		if (param.Empty()) throw TLog("������ ���� ��������!", "TDataBaseSheetReport::runDML");
		if (param.isDMLEmpty()) throw TLog("��� DML-������ ��� ����������!", "TDataBaseSheetReport::runDML");
		//��������� dml-������ �������:
		string dml = param.getFieldValueByCode(TuneField::DMLText);
		//��������� ������� �� ����� ������� � ������:
		size_t result = 0;
		if (!dml.empty())
			//���������� dml-������� �� ������:
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
		TLog("�� ������������ ������ ��� ���������� DML-�������!", "TDataBaseSheetReport::runDML");
	}
	return 0;
}

void TDataBaseInterface::setCellByResultSet(NS_Excel::TExcelBook& book, NS_Excel::TExcelBookSheet& sheet, 
	const NS_Const::DataType& dt, const NS_Oracle::TBaseSet& bs, size_t resultSetCol, 
	const NS_Excel::TExcelCell& cell, const NS_Excel::FormatPtr format) noexcept(false)
{
	using NS_Oracle::TType;
	using NS_Oracle::UInt;
	using NS_Oracle::SQLException;
	using NS_Oracle::TDate;
	using NS_Excel::TExcelDate;
	using NS_Const::DataType;
	using NS_Excel::FormatPtr;
	if (!bs.isValid()) throw TLog("�� �������� ������ BaseSet!", "TDataBaseSheetReport::setCellByResultSet");
	//���������� ��������� �� ������ �������� ������ � �������:
	if (bs.isNullVal(resultSetCol))
	{
		sheet.setBlank(cell);
		return;
	}
	try
	{
		switch (dt)
		{
		case DataType::Integer:
			sheet.WriteAsNumber(cell, bs.getIntVal(resultSetCol), format);
			break;
		case DataType::Double:
			sheet.WriteAsNumber(cell, bs.getDoubleVal(resultSetCol), format);
			break;
		case DataType::String:
			sheet.WriteAsString(cell, bs.getStringVal(resultSetCol));
			break;
		case DataType::Boolean:
			sheet.WriteAsBool(cell, bs.getIntVal(resultSetCol), format);
			break;
		case DataType::Date:
			if (sheet.isDate(cell))
			{
				TDate date = bs.getDateVal(resultSetCol);
				TExcelDate tmp;
				date.getDate(tmp.year, tmp.month, tmp.day, tmp.hour, tmp.minute, tmp.sec);
				double dbl_date = book.Date2Double(tmp);
				sheet.WriteAsNumber(cell, dbl_date, format);
			}
			else
				sheet.WriteAsString(cell, bs.getDateAsStrVal(resultSetCol));
			break;
		default:
			TLog log("��������� ��� ������ � ", "TDataBaseSheetReport::setCellByResultSet");
			log << resultSetCol << " ������� - �� ��������������!" << TLog::NL;
			log.toErrBuff();
			break;
		}
	}
	catch (const SQLException& err)
	{
		TLog log("������ ���������� ", "TDataBaseSheetReport::setCellByResultSet");
		log << NS_Const::TConstType(dt).toStr() << " ������ �� ������� :";
		log << resultSetCol << ", ������: " << cell.getRow() << TLog::NL;
		log << err.what() << TLog::NL;
		log.toErrBuff();
	}
	catch (...)
	{
		TLog log("�� ������������ ������ ������ ������ �� �������: ", "TDataBaseSheetReport::setCellByResultSet");
		log << resultSetCol << ", ������: " << cell.getRow() << TLog::NL;
		log.toErrBuff();
	}
}

bool TDataBaseInterface::setMaxIterationCnt(NS_Oracle::TStatement& query, size_t cnt) noexcept(true)
{
	try
	{
		if (query.isValid() and query.getState() == NS_Oracle::TSQLState::UNPREPARED)
		{
			query.setMaxIterationCnt(cnt);
			return true;
		}
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (const NS_Oracle::SQLException& err)
	{
		TLog log("������ ��������� ������������� ����� �������� ��� �������: " + query.getSQL() + "\n", 
			"TDataBaseInterface::setMaxIterationCnt");
		log << err.what() << '\n';
		log.toErrBuff();
	}
	catch (...)
	{
		TLog log("�� ������������ ������ ��������� ������������� ����� �������� ��� �������: " + query.getSQL() + "\n",
			"TDataBaseInterface::setMaxIterationCnt");
		log.toErrBuff();
	}
	return false;
}

bool TDataBaseInterface::addQueryIteration(TStatement& query) noexcept(true)
{
	try
	{
		if (query.isValid())
		{
			if (query.getMaxIterationCnt() > query.getCurIteration() + 1)
			{
				query.addIteration();
				return true;
			}
		}
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (const NS_Oracle::SQLException& err)
	{
		TLog log("������ ���������� �������� ��� �������:\n" + query.getSQL() + "\n",
			"TDataBaseInterface::addQueryIteration");
		log << err.what() << '\n';
		log.toErrBuff();
	}
	catch (...)
	{
		TLog log("�� ������������ ������ ���������� �������� ��� �������: " + query.getSQL() + "\n",
			"TDataBaseInterface::addQueryIteration");
		log.toErrBuff();
	}
	return false;
}

void TReport::saveReport(TExcelBook& book, const string& file_name) const noexcept(false)
{
	//�������� ������������� ����������:
	string path = config.getOutPath();
	if (TSharedTune::CheckPath(path, true))
		//���������� �����:
		book.SaveToFile(file_name);
	else
		throw TLog("��������� ����������: " + path + " �� �������!\n����� �� ��������!",
			"TReport::One_Report_For_Each_Config");
}

void TReport::One_Report_For_Each_Config() const noexcept(false)
{
	using NS_Const::TuneField;
	using std::vector;
	using NS_Logger::TLog;
	using NS_Excel::TExcelBook;
	using StrArr = vector<string>;
	//��������� ������ ���������������� ������
	StrArr conf_lst = config.getConfFileLst(true);
	if (conf_lst.empty())
		throw TLog("������ ������ ���������������� ������!", "TReport::One_Report_For_Each_Config");
	//��������� ����� ��������� �����:
	string tmp = config.getOutFile();
	//��� ������� ����� ��������� ������������ ������ ��� �����
	for (const string& sub_tune : conf_lst)
	{
		//������������ ����� ��������:
		TUserTune tune(config, sub_tune);
		if (!tune.Empty())
		{
			//�������� excel-���������
			TExcelBook book(tmp);
			//��������� ����� ��� config-�����:
			TDataBaseSheetReport sh(book, tune);
			//���� ����� �� �����������:
			if (!sh.crtSheet())
			{
				TLog log("����� ��� ����� " + sub_tune + " �� �����������!", "TReport::One_Report_For_Each_Config");
				log.toErrBuff();
			}
			//����� ��������� ����:
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
	//��������� ������ ���������������� ������
	StrArr conf_lst = config.getConfFileLst(true);
	if (conf_lst.empty())
		throw TLog("������ ������ ���������������� ������!", "TReport::One_Sheet_By_One_Config");
	//��������� ���� ��������� �����:
	string tmp = config.getOutFile();
	//�������� excel-���������
	TExcelBook book(tmp);
	size_t fail_cnt = 0;
	//��� ������� ����� ��������� ������������ ������ ��� �����
	for (const string& sub_tune : conf_lst)
	{
		//������������ �������� ������:
		TUserTune tune(config, sub_tune);
		//��������� ����� ��� config-�����:
		TDataBaseSheetReport sh(book, tune);
		//���� ����� �� �����������:
		if (!sh.crtSheet())
		{
			TLog log("����� ��� ����� " + sub_tune + " �� �����������!", "TReport::One_Sheet_By_One_Config");
			log.toErrBuff();
			fail_cnt++;
		}
	}
	if (fail_cnt >= conf_lst.size())
		throw TLog("��� ������������ ������ �������� ������!", "TReport::One_Sheet_By_One_Config");
	else//���������� �����, ���� ���� ��� ���������
		saveReport(book);
}

//����� ���� ������, ��� ��� ������ ����� �������� ����� ���� ��������� ��������
//����� ���������� ���������� �������� � ������� �� ���������� ������
void TReport::One_Sheet_By_Many_Statement() const noexcept(false)
{
	using NS_Const::TuneField;
	using std::vector;
	using NS_Logger::TLog;
	using NS_Excel::TExcelBook;
	using StrArr = vector<string>;
	//��������� ������ ���������������� ������
	StrArr conf_lst = config.getConfFileLst(true);
	if (conf_lst.empty())
		throw TLog("������ ������ ���������������� ������!", "TReport::One_Sheet_By_Many_Statement");
	//��������� ���� ��������� �����:
	string tmp = config.getOutFile();
	//�������� excel-���������
	TExcelBook book(tmp);
	//�������������� ��������, ������� ����� �������:
	TExcelBookSheet sheet(nullptr);
	//����� ������ ��� ������������ ������:
	size_t fail_cnt = 0;
	for (const string& sub_tune: conf_lst)
	{
		//�������� ����� ��������:
		TUserTune tune(config, sub_tune);
		//��������� ����� ��� config-�����:
		TDataBaseSheetReport sh(book, sheet, tune);
		//���� ����� �� �����������:
		if (!sh.crtSheet())
		{
			TLog log("����� ��� ����� " + sub_tune + " �� �����������!", "TReport::One_Sheet_By_Many_Statement");
			log.toErrBuff();
			fail_cnt++;
		}
		if (!sheet.isValid())	sheet = book.getActiveSheet();
	}
	if (fail_cnt >= conf_lst.size())
		throw TLog("��� ������������ ������ �������� ������!", "TReport::One_Sheet_By_Many_Statement");
	else//���������� �����, ���� ���� ��� ���������
		saveReport(book);
}

size_t TReport::runDML_By_Tune(bool use_comit) const noexcept(false)
{
	using std::vector;
	using NS_Logger::TLog;
	using StrArr = vector<string>;
	//��������� ������ ���������������� ������
	StrArr conf_lst = config.getConfFileLst(true);
	if (conf_lst.empty())
		throw TLog("������ ������ ���������������� ������!", "TReport::One_Sheet_By_Many_Statement");
	//����� ������������ ����� dml-��������
	size_t result = 0;
	for (const string& sub_tune : conf_lst)
	{
		//��������� ���������� �� ����� ��������
		TUserTune tune(config, sub_tune);
		//��������� ���������� �����������:
		TConnectParam cp = TDataBaseSheetReport::getConnectParam(tune);
		//��� ������ �������� ��������� ����������� � ��:
		TDBConnect db(cp);
		//��������� dml-�������:
		size_t cnt = 0;
		cnt = TDataBaseSheetReport::runDML(db, tune, use_comit);
		if (cnt > 0) result += cnt;
	}
	return result;
}

bool TReport::ProcessExcelFileByJson(TExcelBook& book, const string& js_file, 
	std::vector<NS_ExcelReport::TRowsTNS>& Rows) const noexcept(true)
{
	using NS_Logger::TLog;
	using NS_Tune::TExcelProcData;
	using NS_ExcelReport::TJsonMarkReport;
	using NS_ExcelReport::TRowsFlag;
	using NS_ExcelReport::TRowsTNS;
	try
	{
		//�������������� ��������� ��� json-�����:
		TExcelProcData proc_data(js_file, &config);
		//�������� ������� excel-����� ���������:
		if (proc_data.isDstFileEmpty()) throw TLog("�� ������ excel-���� ��������!", "ProcessExcelFileByJson");
		size_t pageCnt = proc_data.getProcPagesCnt();
		if (pageCnt == 0) throw TLog("�� ������� ��������� ������� ��� excel-����� ���������!", "ProcessExcelFileByJson");
		if (Rows.size() == 0) Rows.resize(pageCnt);
		//��������� ��������� ��� ������� ����� � js-����� ��������
		for (size_t i = 0; i < pageCnt; i++)
		{
			//������������� ������ ��� i-�� ��������:
			TJsonMarkReport report(book, proc_data, i, Rows[i]);
			//������������ ������ ��� i-�� ��������:
			report.crtSheetReport();
		}
		return true;
	}
	catch (TLog& err)
	{
		err << "������ ��� ��������� js-�����: " << js_file << '\n';
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ��� ������ � js-������: " + js_file, "ProcessExcelFileByJson").toErrBuff();
	}
	return false;
}

void TReport::Json_One_Row_One_DBQuery() const noexcept(false)
{
	using NS_Logger::TLog;
	using NS_Tune::TExcelProcData;
	using NS_Excel::TExcelBook;
	using NS_Tune::StrArr;
	using NS_ExcelReport::TRowsTNS;
	//��������� ������ ������ �������� �� config
	StrArr js_files = config.getConfFileLst();
	if (js_files.empty()) throw TLog("������ ���������� � js-������� ��������!", "Json_One_Row_One_DBQuery");
	//������� excel-����:
	string book_name = config.getOutFile();
	TExcelBook book(book_name);
	//������������ ������� �������������� �����
	//����������� ��� ������������� ��������������� ���������� ������ ��� ������ � ������ ��:
	vector<TRowsTNS> rows;
	//���������� ������� �� ��������������:
	for (const string& js : js_files)
	{
		if (js.empty()) continue;
		ProcessExcelFileByJson(book, js, rows);
	}
	//��������� ���������
	if (!book.isEmpty())
		saveReport(book);
}

bool TReport::Json_Report_By_File_Compare(const string& js_file) const noexcept(true)
{
	using NS_Logger::TLog;
	using NS_Tune::TExcelProcData;
	using NS_Excel::TExcelBook;
	if (js_file.empty()) return false;
	try
	{
		//������������ json-��������:
		TExcelProcData js_tune(js_file, &config);
		//�������� ������� excel-����� ���������:
		if (js_tune.isDstFileEmpty()) throw TLog("�� ������ excel-���� ��������!", "Json_Report_By_File_Compare");
		//��������� ����� �������:
		size_t pageCnt = js_tune.getProcPagesCnt();
		if (pageCnt == 0) throw TLog("�� ������� ��������� ������� ��� excel-����� ���������!", "Json_Report_By_File_Compare");
		//�������������� excel-����� ������:
		string book_name = config.getOutFile();
		TExcelBook book(book_name);
		//������� ������
		size_t errCnt = 0;
		//��������� ��������� ��� ������ �������� ����� ���������:
		for (size_t i = 0; i < pageCnt; i++)
		{
			//������������� ������ ��� ������
			TJsonReport page(book, js_tune, i);
			if (page.NoSheet())
			{
				TLog log("������ ��� �������� ��������: ", "Json_Report_By_File_Compare");
				log << i << '\n';
				log.toErrBuff();
				errCnt++;
				continue;
			}
			//������������ ������
			if (page.crtSheetReport() == false)
			{
				TLog log("������ ������������ ������ ��� ��������: ", "Json_Report_By_File_Compare");
				log << i << '\n';
				log.toErrBuff();
				errCnt++;
			}
		}
		//���� ����� ������������ �������� ������ ����� ������
		if (errCnt < pageCnt)
		{
			saveReport(book);
			return true;
		}
	}
	catch (TLog& err)
	{
		err << "������ ��� ��������� �����: " << js_file << ":\n";
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ��� ������������ ������ ��� �����: " + js_file,
			"Json_Report_By_File_Compare").toErrBuff();
	}
	return false;
}

void TReport::Json_Report_By_Files_Compare() const noexcept(true)
{
	using NS_Logger::TLog;
	using NS_Tune::StrArr;
	//������������ ������ json-������:
	StrArr js_files = config.getConfFileLst();
	//������ �� ������� json-�����
	for (const string& file : js_files)
	{
		//������������ ������ ��� �����:
		if (Json_Report_By_File_Compare(file) == false)
			TLog("����: " + file + " �� ���������!", "Json_Report_By_Files_Compare").toErrBuff();
	}
}

bool TReport::Json_SubTune_File_Run(NS_Excel::TExcelBook& book, const string& js_file) const noexcept(true)
{
	using NS_Tune::TSharedTune;
	using NS_Tune::TExcelProcData;
	using NS_Tune::TShareData;
	using NS_Tune::TProcCell;
	using NS_Const::JsonParams;
	using NS_Const::TConstJson;
	using boost::property_tree::ptree;
	using boost::property_tree::read_json;
	using boost::property_tree::json_parser_error;
	try
	{
		//������������� json-�����
		ptree js;
		//���������� ������ � json-����
		read_json(js_file, js);
		//�������������� ��������� ��� ��������� ����� ���������(�������� ���)
		TShareData DestFile(js);
		//���� ��� �� ������� - ����� ��� �� ��������� �����:
		if (DestFile.isEmptyName())	DestFile.setName(book.getFileName());
		//���� ���� ��������������� � �������� - �������
		if (DestFile.isEmpty()) return false;
		//��������� ����� �������
		size_t pageCnt = DestFile.getPageCnt();
		//���� ������� ��� - ��������� ����
		if (pageCnt < 1) return false;
		//��������� ������ �������� ��� �����:
		
		//����� ������, ��� ����� ����� ������������������ �� ������ json-�����
		//�������������� find ���� ������ �� ��������� json
		ptree cell_node = js.get_child(TConstJson::asStr(JsonParams::Cells));
		
		//��������� ������ ��� ��������� �����:
		TProcCell cells(cell_node, &config);
		//������������ ������ �������� �����-���������:
		for (size_t i = 0; i < pageCnt; i++)
		{
			//������������� ������:
			TJsonReport report(book, DestFile, cells, i);
			//������������ ������:
			return report.crtSheetReport();
		}
	}
	catch (const json_parser_error& err)
	{
		TLog(err.what()).toErrBuff();
	}
	catch (const std::exception& err)
	{
		TLog(err.what()).toErrBuff();
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ���� ��������� ��� ����� ��������: " + js_file, "Json_SubTune_File_Run").toErrBuff();
	}
	return false;
}

void TReport::SubConfig_Json_UpdOutExlFile() const noexcept(true)
{
	using NS_Tune::StrArr;
	using NS_Excel::TExcelBook;
	try
	{
		//��������� ����� ����� ��� �������������:
		//�� ������ ����� ����� ��� ������ ���� �����������!
		string name = config.getOutFile();
		//��������� ������� ����� ��� ��������� �� 2�� ����:
		if (TSharedTune::CheckPath(name, false) == false)
			throw TLog("��������� ����: " + name + " �� ������! ���� ��������� �� ���������!", "TReport::SubConfig_Stage");
		//������������� excel-����� ���������:
		TExcelBook book(name);
		//��������� ������ ������ ��������:
		StrArr js_files = config.getSubConfigFileLst();
		//������� �������� ���������:
		size_t sucCnt = 0;
		//������������ ������ ���� � ����������� ���� ��������� ��������:
		for (const string& file : js_files)
		{
			if (Json_SubTune_File_Run(book, file) == true) sucCnt++;
		}
		//���� ���� ���� ���� �������� ��������� - ��������� ���������
		if (sucCnt > 0)
			saveReport(book, name);
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ��� ���������� ����� ���� ��������� ������!", "TReport::SubConfig_Stage").toErrBuff();
	}
}

void TReport::SubConfig_IniFile_Execute() const noexcept(true)
{
	using NS_Tune::StrArr;
	try
	{
		//��������� ������ ini-������:
		StrArr config_lst;
	}
	catch (const TLog)
	{
	}
	catch (...)
	{
		TLog("�� ������������ ������ ��� ���������� ini-�����!", "TReport::SubConfig_IniFile_Execute").toErrBuff();
	}
}

bool TReport::loadFromJson(const string& js_file) const noexcept(true)
{
	using NS_Excel::TExcelBook;
	using NS_Tune::TProcCell;
	using NS_Tune::TExcelProcData;
	using NS_Tune::TSimpleTune;
	using NS_Tune::StrArr;
	try
	{
		//������������� �������� js-�����:
		TExcelProcData json(js_file, &config);
		//�������� ���������� �������
		if (json.isDstFileEmpty())
			throw TLog("������ �������� excel-���� ��� ����������!", "loadFromJson");
		if (json.isCellsEmpty())
			throw TLog("������ ������ ��� ���������!", "loadFromJson");
		//��������� ���������� ����������� excel-������:
		string directory = json.getDstFile().getName();
		StrArr filesLst = TSimpleTune::getFileLst(directory);
		for (const string& name : filesLst)
		{
			TLog log("���������� �������� ������ �� �����: " + name, "loadFromJson");
			//������������� excel-�����:
			TExcelBook book(name);
			//��������� ����� ����� �.�. ��� ������� ���� ����������:
			json.setDstFileName(name);
			//��������� ����� �������������� �������:
			size_t pageCnt = json.getProcPagesCnt();
			size_t errCnt = 0;//������� ������
			//��������� ������ �������� excel-�����:
			for (size_t i = 0; i < pageCnt; i++)
			{
				log << " ��� " << i << " ��������!\n";
				log.toErrBuff();
				//������������� ����������� excel-�����
				TJsonReport report(book, json, i);
				//������������ ������:
				if (report.crtSheetReport() == false)
					errCnt++;
			}
		}
	}
	catch (const TLog& log)
	{
		log.toErrBuff();
	}
	catch (...)
	{
		TLog log("�� ������������ ������ ��� �������� ������ �� �����: " + js_file, "loadFromJson");
		log.toErrBuff();
	}
	return false;
}

void TReport::load2DBFromExcel() const noexcept(false)
{
	using NS_Logger::TLog;
	using NS_Tune::TExcelProcData;
	using NS_Excel::TExcelBook;
	using NS_Tune::StrArr;
	using NS_ExcelReport::TRowsTNS;
	//���������� �������� json-������:
	//��������� ������ json-������ �������� �� config
	StrArr js_files = config.getConfFileLst();
	if (js_files.empty())
		throw TLog("������ ���������� � js-������� ��������!", "load2DBFromExcel");
	//���������� ������� �� ������� �����:
	for (const string& js : js_files)
	{
		//���� ���� ������ - ����������:
		if (js.empty()) continue;
		loadFromJson(js);
	}
/**/
}

void TReport::Create_Report_By_Code(const NS_Const::ReportCode& code) const
{
	using NS_Const::ReportCode;
	using NS_Const::TConstReportCode;
	using NS_Logger::TLog;
	//������������ ���� ������ � �� ������������� ������:
	auto raise_err = [](const ReportCode& rp)->TLog 
	{
		TLog log("����� ", "TReport::Create_Report_By_Code");
		log << TConstReportCode(rp).getName() << " �� ����������!" << TLog::NL;
		return log;
	};
	switch (code)
	{
		//������ ���������� �� ���������� ���������� sql-������� �� ����:
	case ReportCode::BALANCE_LIST:
	case ReportCode::REPAYMENT_FOR_DATE:
	case ReportCode::DOCS_MF_SF_FOR_PERIOD:
	case ReportCode::LOTS:
		One_Sheet_By_One_Config();
		break;
		//case ReportCode::DOCS_MF_SF_FOR_PERIOD:
		//���� ���� ������ ��� ������ config-�����
	case ReportCode::RIB_DOCS_FOR_PERIOD:
	case ReportCode::POTREB_CRED_BY_FILE:
	case ReportCode::CRED_CASE_MF:
		One_Report_For_Each_Config();
		break;
		//����� ������� �� ������ ����������� ��������� �������� �� ���� �������� � ���� ����:
	case ReportCode::NBKI_NP:
	case ReportCode::NBKI_JP:
	case ReportCode::BALANCE_SUA:
		One_Sheet_By_Many_Statement();
		break;
		//���������� �������� ���������:
	case ReportCode::NBKI_APPLY:
	case ReportCode::CLOSE_DAY:
		runDML_By_Tune(true);
		break;
		//������ ��������� �������� + ����������� � excel-������
	case ReportCode::FULL_CRED_REPORT:
	{
		//������������ ������ �� ��
		//One_Sheet_By_Many_Statement();
		//��������� excel-������
		SubConfig_Json_UpdOutExlFile();
		break;
	}
	//������ � ��������� �������� ��� ���(������� ������ �� ���� � excel)
	//�� ������������
	case ReportCode::FULL_CRED_REPORT_SUA:
	case ReportCode::EXCEL_SET_DATA_FROM_BASE:
		Json_One_Row_One_DBQuery();
		break;
		//�������� ������ � oracle �� excel
	case ReportCode::LOAD_FROM_FILE:
	case ReportCode::EXCEL_PAY_LOAD_MF:
	case ReportCode::EXCEL_PAY_LOAD_SF:
	case ReportCode::EXCEL_DOC_LOAD:
		load2DBFromExcel();
		break;
		//��������� ������ excel
	case ReportCode::FILE_COMPARE_RIB:
	case ReportCode::FILE_COMPARE_RTBK:
		Json_Report_By_Files_Compare();
		break;
	case ReportCode::QUIT_REPORT:
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
	using std::cout;
	using std::cin;
	try
	{
		//��������� �������������� ���� ������:
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
		TLog("�� ������������ ������ ��� ������������ ������!", "TReport::Execute").toErrBuff();
	}
	return false;
}

TReport::TReport(const string& config_file): config(config_file)
{
	try
	{
		if (!config.Empty()) Execute();
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ��� ������������� ������ " + config.getMainCode() +
			" ��� ����� ��������: " + config_file, "TReport::TReport");
	}
}


void TReport::Smolevich_Sld_Report(const string& path, const string& out_file_name)
{
	using NS_SMLVCH_IMP::TAccount;
	using NS_SMLVCH_IMP::TImportAccount;
	using NS_SMLVCH_IMP::TAccounts;
	using NS_Tune::TSimpleTune;
	using NS_Tune::StrArr;
	using NS_Excel::TExcelCell;
	const string tmpl_name = "F:\\Projects\\SomeThing\\TypicalReport\\��������\\���������\\template\\template.xlt";
	//������ ����� � ����������:
	StrArr files = TSimpleTune::getFileLst(path);
	if (files.empty())
	{
		TLog log("����� �� �����������! �� ���������� ����: ", "Smolevich_Sld_Report");
		log << path << " ������ �� ����������!\n";
		log.toErrBuff();
		return;
	}
	//������� excel-����
	TExcelBook book(out_file_name);
	//��������� ������� �����:
	for (const string& name : files)
	{
		TImportAccount accs(name);
		//��������� ������ ��� �������� �� �����:
		if (book.setSheetByTemplate(tmpl_name, accs.getName(), DEF_TEMPL_SH_INDX, true))
		{
			//������������� ��������
			TExcelBookSheet sheet = book.getActiveSheet();
				/*
				//��������� �� �������� � ��������� �����
				TAccounts act_acc;//������ �������� ������
				TAccounts pas_acc;//������ ��������� ������
				for (size_t i = 0; i < accs.Count(); i++)
				{
					if (accs[i].isActive())
						act_acc.push_back(accs[i]);
					else
						pas_acc.push_back(accs[i]);
				}
				/**/
				//������ ������ � ����:
				size_t row = 1;
				double sum_sld = 0;
				//������� "�����"
				sheet.WriteAsString(TExcelCell(row++, 0), "�����");
				for (size_t i = 0; i < accs.Count(); i++)
				{
					//������ � ���� �������� ������
					if (accs[i].isActive() == false) continue;
					size_t col = 0;
					sheet.WriteAsString(TExcelCell(row, col++), accs[i].Account());
					sheet.WriteAsString(TExcelCell(row, col++), accs[i].Name());
					double sld = 0;
					NS_Converter::toDblType(accs[i].SldRub(), &sld);
					sum_sld += sld;
					sheet.WriteAsNumber(TExcelCell(row, col++), sld);
					if (accs[i].Account().substr(5, 3) != "810")
						sheet.WriteAsNumber(TExcelCell(row, col++), 0);
					else
						sheet.WriteAsNumber(TExcelCell(row, col++), sld);
					sheet.WriteAsString(TExcelCell(row, col++), accs[i].LastOperDate());
					row++;
				}
				//������ ������ �� ������:
				sheet.WriteAsString(TExcelCell(row, 0), "����� �� �������� ������: ");
				sheet.WriteAsNumber(TExcelCell(row, 2), sum_sld);
				sheet.WriteAsNumber(TExcelCell(row, 3), sum_sld);
				book.SaveToFile();
				//������ ��������� ������:
				row++;
				sum_sld = 0;
				//������� "������"
				sheet.WriteAsString(TExcelCell(row++, 0), "������");
				for (size_t i = 0; i < accs.Count(); i++)
				{
					//������ � ���� ��������� ������
					if (accs[i].isActive()) continue;
					size_t col = 0;
					sheet.WriteAsString(TExcelCell(row, col++), accs[i].Account());
					sheet.WriteAsString(TExcelCell(row, col++), accs[i].Name());
					double sld = 0;
					NS_Converter::toDblType(accs[i].SldRub(), &sld);
					sheet.WriteAsNumber(TExcelCell(row, col++), sld);
					sum_sld += sld;
					if (accs[i].Account().substr(5, 3) != "810")
						sheet.WriteAsNumber(TExcelCell(row, col++), 0);
					else
						sheet.WriteAsNumber(TExcelCell(row, col++), sld);
					sheet.WriteAsString(TExcelCell(row, col++), accs[i].LastOperDate());
					row++;
				}
				//������ ������ �� ������:
				sheet.WriteAsString(TExcelCell(row, 0), "����� �� ��������� ������: ");
				sheet.WriteAsNumber(TExcelCell(row, 2), sum_sld);
				sheet.WriteAsNumber(TExcelCell(row, 3), sum_sld);
				book.SaveToFile();
		}
	}
}

void NS_ExcelReport::TReport::Smolevich_Imp_Docs(const string& path, const string& out_file) noexcept(true)
{
	using NS_SMLVCH_IMP::TRSBankDoc;
	using NS_SMLVCH_IMP::TRSBankDocs;
	using NS_SMLVCH_IMP::TRSBankImp;
	using NS_Tune::TSimpleTune;
	using NS_Tune::StrArr;
	using NS_Excel::TExcelBook;
	using NS_Excel::TExcelBookSheet;
	using NS_Excel::TExcelBookFormat;
	using NS_Excel::TExcelCell;
	using NS_Const::TConstExclTune;
	TLog log("", "Smolevich_Imp_Docs");
	//������������ ����� ������� �������:
	string imp_template = "F:\\Projects\\SomeThing\\TypicalReport\\��������\\����� ����������\\template\\template.txt";
	//�������� ����� ������
	if (path.empty())
		log << "�� ������ ���� � ����� ��� ����������\n";
	if (out_file.empty())
		log << "�� ������� ��� ��������� �����!\n";
	if (!log.isEmpty())
	{
		log.toErrBuff();
		return;
	}
	//��������� excel-������ ��� ���������� ����������:
	//������ ����� � ����������:
	StrArr files = TSimpleTune::getFileLst(path);
	if (files.empty())
	{
		log << "����� �� �����������! �� ���������� ����: " << path << " ������ �� ����������!";
		log.toErrBuff();
		return;
	}
	//������������� ��������� ���������� ��� �������:
	TRSBankImp docs(imp_template);
	//��������� �����������, ���� �� �� ����������:
	if (docs.NoDelimeter()) docs.setDelimiter('^');
	//��������� ������� excel-�����:
	for (const string& name : files)
	{
		//��������� ���������� ������ - ��������� ������ excel:
		if (!TConstExclTune::isValidFileByExtension(name))
		{
			log << "���������� ����� " << name << " �� ��������������!";
			log.toErrBuff();
			log.clear();
			continue;
		}
		//������������� excel-�����
		TExcelBook book(name);
		book.load(name);
		//��������� ����� ������� � �����:
		size_t pages = book.SheetCount();
		//��������� ������ �������� �����:
		for (size_t i = 0; i < pages; i++)
		{
			//������������� �������� �����:
			TExcelBookSheet sheet = book.getSheetByIndex(i);
			//���� �� �������� ��� ������ - ������������ ��������� ��������
			if (sheet.hasNoData()) continue;
			//!!!��������� ��� ���������� ����� ���� �� 2�� ������:
			size_t curRow = 2;
			size_t lastRow = sheet.getLastRow();
			//������������ ���������� ���������:
			for (; curRow <= lastRow; curRow++)
			{
				//!!!��������� ���������� ��������� ���������� �� 2�� ������
				size_t curCell = 1;
				//size_t lastCell = 8;
				TRSBankDoc doc;
				//���������� ����� �������:
				//������������ ������ ��� ����������:
				TExcelCell cell(curRow, curCell, false);
				doc.UsrCode = sheet.ReadAsString(cell, book);
				cell.getNextColCell();
				//���� ���������
				doc.Date = sheet.ReadAsString(cell, book);
				cell.getNextColCell();
				//��� ����� ���������!!!! ���� �� ��������
				doc.Num = sheet.ReadAsString(cell, book);
				cell.getNextColCell();
				doc.payer.AccNum = sheet.ReadAsString(cell, book);
				doc.payer.Account = doc.payer.AccNum;
				cell.getNextColCell();
				doc.recipient.AccNum = sheet.ReadAsString(cell, book);
				doc.recipient.Account = doc.recipient.AccNum;
				cell.getNextColCell();
				doc.Summa = sheet.ReadAsString(cell, book);
				cell.getNextColCell();
				doc.Note = sheet.ReadAsString(cell, book);
				//���������� ��������� � ������
				docs.AddDoc(doc);
			}
		}
	}
	docs.CreateFile4Import(out_file);
}