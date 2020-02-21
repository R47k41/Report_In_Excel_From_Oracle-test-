//������ ����������� ����������� ��� TDBReport
#include <iostream>
#include <fstream>
#include "TDBReport.h"
#include "TSQLParser.h"
#include "TConverter.h"
#include "Logger.hpp"


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

const int DEF_TEMPL_SH_INDX = 0;

template string TDataBaseInterface::getSqlText<NS_Sql::TText>(bool by_str, const string& str) noexcept(false);
template string TDataBaseInterface::getSqlText<NS_Sql::TSimpleSql>(bool by_str, const string& str) noexcept(false);

void raise_app_err(const TLog& log, bool as_raise = true);

void raise_app_err(const TLog& log, bool as_raise)
{
	as_raise ? throw log : log.toErrBuff();
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
		return TDataType::CELLTYPE_NUMBER;
	case DataType::String:
		return TDataType::CELLTYPE_STRING;
	case DataType::Boolean:
		return TDataType::CELLTYPE_BOOLEAN;
	}
	return TDataType::CELLTYPE_ERROR;
}

bool TBaseSheetReport::setCellFormat(size_t Row, size_t Column, NS_Excel::TExcelBookFormat& format) noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_Excel::TExcelBookFormat;
	//���� ������ ������������ ����
	if (!format.isValid()) return false;
	TExcelCell cell(Row, Column, false);
	//���� ������� ������ ������
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

bool TBaseSheetReport::checkCellStrVal(const NS_Excel::TExcelCell& cell, const string& val) const noexcept(false)
{
	using NS_Const::Trim;
	using NS_Const::LowerCase;
	try
	{
		//������������ ������ ��� �������� ��������:
		string tmp = sheet.ReadAsString(cell);
		if (tmp.empty()) return false;
		Trim(tmp);
		tmp = LowerCase(tmp);
		string str = LowerCase(val);
		Trim(str);
		if (tmp == str)
			return true;
		else
		{
			TLog log("�������� � ������: ", "TBaseSheetReport::checkCellStrVal");
			log << tmp << " �� ������������� ���������� ��������: " << str << '\n';
			log << "������: " << cell.getName() << '\n';
			throw log;
		}
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("�� ������������ ������ ��� ���������� �������� ������:", "TBaseSheetReport::checkCellStrVal");
		log << cell.getName() << " � ��������: " << val << '\n';
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

bool TExtendSheetReport::noDataInColID(size_t Row) const noexcept(false)
{
	using NS_Excel::TExcelCell;
	TExcelCell cell(Row, colID, false);
	try
	{
		return sheet.isBlank(cell);
	}
	catch (...)
	{
		string s = book.getError();
		TLog log("�� ������������ ������ ��� �������� ������ � ������: ", "TExtendSheetReport::noDataInColID");
		log << cell.getName() << "\n";
		if (!s.empty())
			log << s << '\n';
		throw log;
	}
}

size_t TExtendSheetReport::FirstRow() const noexcept(true)
{
	size_t row = getRow(true);
	return row == NS_Tune::TIndex::EmptyIndex ? sheet.getFirstRow() : row;
}

size_t TExtendSheetReport::LastRow() const noexcept(true)
{
	size_t row = getRow(false);
	return row == NS_Tune::TIndex::EmptyIndex ? sheet.getLastRow() : row;
}

bool TExtendSheetReport::isCorrectFilter(size_t curRow) const noexcept(true)
{
	using NS_Tune::TFilterData;
	using NS_Excel::TExcelCell;
	//���� ������� ��� - ������� ������
	if (filters.empty())
	{
		if (noColID()) return true;
		TExcelCell cell(curRow, getColID(), false);
		if (sheet.isEmptyCell(cell)) return false;
		return true;
	}
	for (const TFilterData& fltr : filters)
	{
		TExcelCell cell(curRow, fltr.getColIndx(), false);
		if (checkCellStrVal(cell, fltr.getValue()) == false)
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
	for (; i <= size; i++) 
	{
		if (all_flg or isCorrectFilter(i))
			rows.push_back(TRowFlag(i, true));
	}
	return rows;
}

bool TExtendSheetReport::Compare_Cells(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
	const NS_Excel::TExcelCell& srcCell) const noexcept(true)
{
	using NS_Excel::TDataType;
	try
	{
		//�������� �� ������� ������ ���������:
		if (sheet.isEmptyCell(srcCell))	return false;
		//if (QualCellsType(dstSheet, dstCell, srcCell) == false) return false;
		//��������� ���� ������ ��� ������ � ������� ����������:
		TDataType src_dt = sheet.getCellType(srcCell);
		switch (src_dt)
		{
		case TDataType::CELLTYPE_BOOLEAN: 
		{
			bool srcVal = sheet.ReadAsBool(srcCell);
			bool dstVal = dstSheet.ReadAsBool(dstCell);
			return dstVal == srcVal;
		}
		case TDataType::CELLTYPE_NUMBER: 
		{
			double srcVal = sheet.ReadAsNumber(srcCell);
			double dstVal = dstSheet.ReadAsNumber(dstCell);
			return dstVal == srcVal;
		}
		case TDataType::CELLTYPE_STRING: 
		//case TDataType::CELLTYPE_EMPTY:
		{
			string srcVal = sheet.ReadAsString(srcCell);
			string dstVal = dstSheet.ReadAsString(dstCell);
			return dstVal == srcVal;
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

size_t TExtendSheetReport::CheckOnSheet(const NS_Excel::TExcelBookSheet& dstSheet,
	const NS_Excel::TExcelCell& dstCell, size_t srcCol, TRowsFlag* RowsArr) const noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_Tune::TIndex;
	//������� ������������� ���������� �� ���������:
	bool use_fltr = true;
	size_t curRow = 0;
	size_t lastRow = 0;
	if (RowsArr)
	{
		use_fltr = false;
		curRow = (*RowsArr)[0].first;
		lastRow = (*RowsArr).size() - 1;
	}
	else
	{
		curRow = FirstRow();
		lastRow = LastRow();
	}
	//�������� �� ������� �����:
	for (; curRow <= lastRow; curRow++)
	{
		bool filtred = false;
		//��������� ������� �������:
		if (use_fltr == false)
			filtred = (*RowsArr)[curRow].second;
		else
			filtred = isCorrectFilter(curRow);
		//���� ���������� ��� ������ ������ �������:
		if (filtred)
		{
			//������������ ������ ���������:
			TExcelCell srcCell(curRow, srcCol, false);
			//���� ������ �������:
			if (Compare_Cells(dstSheet, dstCell, srcCell))
			{
				//��������� ������ ��������� �� ������� �����:
				if (RowsArr) (*RowsArr)[curRow].second = false;
				//�������
				return curRow;
			}
		}
	}
	return TIndex::EmptyIndex;
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

TJsonReport::TJsonReport(TExcelBook& book_ref, const TExcelProcData& json_tune, size_t page) :
	TExtendSheetReport(book_ref, json_tune.getDstFile(), page), cells_data(json_tune.getCellsData())
{
	//������������ �������� ��������:
	crtCellFillFormatArr();
}

bool TJsonReport::CorrectFilter(size_t cur_row) const noexcept(true)
{
	return TExtendSheetReport::isCorrectFilter(cur_row);
}

void TJsonReport::setDQLParamByCell(TStatement& query, const NS_Excel::TExcelCell& cell, const NS_Tune::TCellData& value) const noexcept(false)
{
	using NS_Excel::TExcelCell;
	using NS_Const::DataType;
	using NS_Const::TConstType;
	using NS_Oracle::TDate;
	using NS_Excel::TExcelDate;
	const DataType& data_type = value.getInType();
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
		//����� ����������� ��������� ��� ������ � ������� ��� ������???
		if (sheet.isDate(cell))
		{
			double tmp = sheet.ReadAsNumber(cell);
			TExcelDate exl_date;
			if (book.Double2Date(tmp, exl_date))
			{
				TDate ora_date;
				ora_date.setDate(exl_date.year, exl_date.month, exl_date.day,
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

bool TJsonReport::setSelectParams(TStatement& query, size_t curRow) const noexcept(true)
{
	using NS_Tune::TCellData;
	using NS_Excel::TExcelCell;
	using NS_Const::DataType;
	//���������� ������������ ������ ��� ��������� i-��� ���������
	size_t ColIndx = 0;
	try
	{
		//��������� ������� ����������:
		vector<TCellData> params = cells_data.getCellDataArr();
		//���� ��������� ������ - ������������� �� ���� �� ����
		if (params.empty()) return true;
		//�������� �� ���� ���������� � ������������� �� � ������:
		for (const TCellData& cd : params)
		{
			//���� �������� ������/��������� - ����������
			if (cd.EmptyDstIndx() or cd.EmptySrcParam() or cd.getInType() == DataType::ErrorType) continue;
			ColIndx = cd.DstIndex();
			//������� �������������� ������
			TExcelCell cell(curRow, ColIndx, false);
			//��������� ��������� ������� ��� ������ ������:
			setDQLParamByCell(query, cell, cd);
		}
		return true;
	}
	catch (TLog& err)
	{
		err << "������ ��� ��������� ���������: " << ColIndx << " ��� �������: " << query.getSQL() << '\n';
		err.toErrBuff();
	}
	catch (const NS_Oracle::SQLException& err)
	{
		TLog log("������ ��������� ���������: ", "TJsonReport::setSelectParams");
		log << ColIndx << "\n��� �������:\n" << query.getSQL() << '\n' << err.what() << '\n';
		log.toErrBuff();
	}
	catch (...)
	{
		TLog log("�� ������������ ������ ��� ��������� ���������� ������� ��� ������: ", "TJsonReport::setSelectParams");
		log << curRow << '\n';
		log.toErrBuff();
	}
	return false;
}

void TJsonReport::writeExcelFromDB(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false)
{
	using NS_Excel::TExcelCell;
	using NS_Tune::TIndex;
	//��������� ������� ����������:
	vector<TCellData> params = cells_data.getCellDataArr();
	//���� ��������� ������ - ������������� �� ���� �� ����
	if (params.empty()) return;
	//��������� ����� ������� �������:
	size_t colCnt = rs.getColumnsCnt();
	//�������� �� ���������� �������:
	while (rs.Next())
	{
		//�������� �� ���� ���������� � ��������� ��:
		for (const TCellData& cd : params)
		{
			//���� ������ ��� ������� ������ - ����������:
			if (cd.EmptyInsIndx()) continue;
			//��������� �������:
			TExcelCell cell(curRow, cd.InsIndex(), false);
			//��������� ������ � ������
			TDataBaseInterface::setCellByResultSet(book, sheet, rs, cd.SrcVal(), cell);
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

void TJsonReport::insertToDataBase(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false)
{
	throw TLog("������� ������� ������ � �� �� �����������!", "TJsonReport::insertToDataBase");
}

bool TJsonReport::ProcessByDataBase(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false)
{
	using NS_Const::JSonMeth;
	const JSonMeth& m = cells_data.getMethodCode();
	switch (m)
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
	case JSonMeth::SendToDB:
	{
		insertToDataBase(rs, curRow);
		return true;
	}
	}
	return false;
}

bool TJsonReport::runQuery(NS_Oracle::TStatement& query, size_t curRow) noexcept(true)
{
	NS_Oracle::ResultSetPtr rsp = query.executeQuery();
	try
	{
		//���������� �������
		TResultSet rs(rsp);
		//��������� �������� �� ������� � excel-����
		ProcessByDataBase(rs, curRow);
		//��������� ResultSet
		return true;
	}
	catch (TLog& err)
	{
		err << "������ ���������� ������� ��� ������: " << curRow << '\n';
		err.toErrBuff();
	}
	catch (const NS_Oracle::SQLException& err)
	{
		TLog log("������ ���������� ������� ��� ������: ", "TJsonReport::runQuery4Row");
		log << curRow << '\n' << err.what() << '\n';
		log.toErrBuff();
	}
	catch (...)
	{
		TLog log("�� ������������ ������ ���������� �������:\n", "TJsonReport::runQuery4Row");
		log << query.getSQL() << "\n��� ������: " << curRow << '\n';
		log.toErrBuff();
	}
	query.closeResultSet(rsp);
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
			return runQuery(query, curRow);
		}
	}
	return false;
}

void TJsonReport::setExcelDataByDB(NS_Oracle::TStatement& query, size_t& rowFrom) noexcept(false)
{
	using NS_Tune::TIndex;
	if (rowFrom <= 0)
	{
		TLog log("�� ����� ������� ��������� ������ ���������: ", "TJsonReport::setExcelDataByDB");
		log << rowFrom << '\n';
		throw log;
	}
	//��������� ������ ��������� �������������� ������
	size_t rowTo = LastRow();
	//���������� ����� excel-�����
	for (rowFrom; rowFrom <= rowTo; rowFrom++)
	{
		setExcelRowDataByBD(query, rowFrom);
	}
}

void TJsonReport::UpdExcelDataByDB(NS_Oracle::TDBConnect& db, const NS_Tune::TUserTune& tune, size_t& rowFrom) noexcept(false)
{
	using NS_Tune::TIndex;
	using NS_Oracle::TStatement;
	//���� ������ �� ������� ���� ���������� �� ����������� - ����� �� �� ��������
	if (rowFrom == TIndex::EmptyIndex)
		rowFrom = FirstRow();
	//��������� ������ �������:
	string sql = TDataBaseInterface::getSqlByTune(tune);
	//������������ ������� � ��:
	TStatement query(db, sql, 1);
	//��������� ���������� ����������:
	TDataBaseInterface::setSqlParamsByTune(query, tune);
	setExcelDataByDB(query, rowFrom);
	query.close();
}

void TJsonReport::ProcessSheetDataWithDB() noexcept(false)
{
	if (!book.isValid()) throw TLog("����� �� ����������������!", "TJsonReport::SheetDataFromDataBase");
	//��������� ������ ������ ��� ���������
	size_t row = FirstRow();
	//��� ������ ��������� ����������� ���������:
	for (const TUserTune& config : cells_data.getDBTuneArr())
	{
		//��������� ���������� ����������� � ��
		NS_Oracle::TConnectParam cp = TDataBaseInterface::getConnectParam(config);
		//������� ����������� � ��:
		TDBConnect db(cp);
		if (!db.isValid()) throw TLog("������ ����������� � ��: " + cp.tns_name, "TJsonReport::SheetDataFromDataBase");
		try
		{
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

size_t TJsonReport::crtFillFormat(NS_Excel::TExcelBookFormat& init_format, bool find_flg, bool font_flg) noexcept(false)
{
	using NS_Excel::TExcelBookFormat;
	using NS_Excel::TExcelBookFont;
	using NS_Excel::TColor;
	using NS_Tune::TCellMethod;
	const int NoColor = 0;
	//���� ������ �� ������� - �������
	if (init_format.isValid())
	{
		TColor color = TColor::COLOR_NONE;
		//��������� ����� ������� �� ��������:
		const TCellMethod& meth = cells_data.getMethod();
		if (find_flg)
			color = meth.getIncludeColor();
		else
			color = meth.getExcludeColor();
		if (color == TColor::COLOR_NONE) return NoColor;
		TExcelBookFormat result = book.AddFormat(init_format);
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
			return book.FormatCount();
		}
	}
	return NoColor;
}

void TJsonReport::crtCellFillFormat(size_t Row, size_t Col, bool font_flg) noexcept(false)
{
	using NS_Excel::TExcelCell;
	using NS_Excel::TExcelBookFormat;
	//��������� ������� ������� ������
	TExcelCell cell(Row, Col, false);
	TExcelBookFormat cur_format = sheet.getCellFormat(cell);
	//������������ ����� �������� ��� ������� ������
	TFillFormat format;
	format.first = crtFillFormat(cur_format, true, font_flg);
	format.second = crtFillFormat(cur_format, false, font_flg);
	frmt_arr.push_back(format);
}

void TJsonReport::crtRowFillFormat(size_t Row, const NS_Const::JsonCellFill& fill_code) noexcept(true)
{
	using NS_Tune::TCellData;
	using NS_Tune::CellDataArr;
	using  NS_Const::JsonCellFill;
	using NS_ExcelReport::TFillFormat;
	using NS_ExcelReport::TFillFrmts;
	//���������� ������� ��� ������-��������������:
	bool colID_flg = fill_code == JsonCellFill::ID_And_CurCell;
	if (colID_flg)	crtCellFillFormat(Row, getColID(), false);
	//��������� ������ �� ������ �������������� �������
	const CellDataArr& arr = cells_data.getCellDataArr();
	//��������� ������� ��� ���� ��������� � �������� �����:
	for (const TCellData& param : arr)
	{
		//���� �������� ������ - ����� ���������
		if (param.isEmpty()) continue;
		//���� ������ ��������� ��������� � �������-��������������� - ���������� ������ ���
		if (colID_flg and param.DstIndex() == getColID()) continue;
		crtCellFillFormat(Row, param.DstIndex(), true);
	}
}

void TJsonReport::crtCellFillFormatArr() noexcept(true)
{
	using NS_Tune::TCellMethod;
	using NS_Tune::TCellData;
	using NS_Tune::CellDataArr;
	using NS_Const::JsonCellFill;
	using NS_Tune::TCellFillType;
	try
	{
		//��������� ������� �������������� �����:
		if (cells_data.CellCnt() <= 0) return;
		//��������� ������ �� ����� ���������:
		const TCellMethod& meth = cells_data.getMethod();
		//���� �� ������ ����� ������� ��� ����� ������ - ������
		if (meth.isEmptyColor()) return;
		//��������� ������ �� ������
		size_t Row = FirstRow();
		const NS_Const::JsonCellFill fill_code = meth.getFillType();
		//��������� ������ �������� �����:
		switch (fill_code)
		{
		case JsonCellFill::CurCell:
		case JsonCellFill::ID_And_CurCell:
		{
			crtRowFillFormat(Row, fill_code);
			break;
		}
		case JsonCellFill::ID_All_Find:
		case JsonCellFill::ID_More_One_Find:
		{
			crtCellFillFormat(Row, getColID(), false);
			break;
		}
		}
		return;
	}
	catch (TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("�� ������������ ������ ��� ��������� �������!\n", "TJsonReport::crtCellFillFormatArr").toErrBuff();
	}
	frmt_arr.clear();
}

bool TJsonReport::procRowCell(size_t Row, size_t Col, size_t index, bool find_flg) noexcept(true)
{
	using NS_Excel::TExcelBookFormat;
	using NS_Excel::TExcelCell;
	if (index >= frmt_arr.size()) return false;
	//��������� ������� ������ �� ������ ��������:
	try
	{
		TFillFormat fill = frmt_arr[index];
		int FormatIndex = find_flg ? fill.first : fill.second;
		FormatIndex--;//�.�. � excel ������� �� 0
		if (FormatIndex >= 0)
		{
			TExcelBookFormat format = book.getFormatByIndex(FormatIndex);
			TBaseSheetReport::setCellFormat(Row, Col, format);
			return true;
		}
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TExcelCell cell(Row, Col, false);
		TLog log("�� ������������ ������ ��� ��������� ������� ������: ", "TJsonReport::procRowCell");
		log << cell.getName() << '\n';
		log.toErrBuff();
	}
	return false;
}

bool TJsonReport::procRowCell(const NS_Excel::TExcelCell& cell, size_t index, bool find_flg) noexcept(true)
{
	return procRowCell(cell.getRow(false), cell.getCol(false), index, find_flg);
}

bool TJsonReport::Search_DstRow_In_SrcSheet(const TExtendSheetReport& srcSheet, const NS_Tune::CellDataArr& cols, 
	size_t curRow) noexcept(true)
{
	using NS_Tune::CellDataArr;
	using NS_Tune::TCellData;
	using NS_Excel::TExcelCell;
	using NS_Tune::TIndex;
	using NS_ExcelReport::TRowsFlag;
	//������� �� ��������� �����:
	int failCnt = 0, col_cnt = 0;
	//��������� ������ ����� ��� ���������:
	TRowsFlag srcRows = srcSheet.setFiltredRowsArr();
	//��� ������ ������ � ����� ��������� - ������������ ��������� � json-����� �������
	for (const TCellData& indx : cols)
	{
		//���� ��� ������ �� �������� �� ������ ���������/��������� - ��������� � ��������� ������
		if (indx.EmptyDstIndx() and indx.EmptySrcParam()) continue;
		size_t dstCol = indx.DstIndex();
		size_t srcCol = indx.SrcParam();
		//��������� ������ � ������� ����� ������������ ������:
		TExcelCell dstCell(curRow, dstCol, false);
		//���� ������ � ��������� ��� ��������� - ������ - ���� ������
		if (sheet.isEmptyCell(dstCell)) continue;
		col_cnt++;
		//��������� ����� �� ����� ���������:
		size_t srcRow = srcSheet.CheckOnSheet(sheet, dstCell, srcCol, &srcRows);
		//���� ������ �� �������:
		if (srcRow == TIndex::EmptyIndex)
		{
			failCnt++;
			//������� ������� �����, ���� ������ �� ��������
			procRowCell(dstCell, col_cnt, false);
		}
		else
		{
			//������ ���������:
			TExcelCell dCell(curRow, indx.InsIndex(), false);
			//������ ���������
			TExcelCell sCell(srcRow, indx.SrcVal(), false);
			//��������� ��������� ������ � ������ ��������� �� ������ �������� ���������:
			srcSheet.setDstCellBySrcCell(sheet, dCell, sCell);
			//������� ������� �����, ���� ������ ��������
			procRowCell(dstCell, col_cnt, true);
		}
		//book.SaveToFile("D:\\������\\2020\\02\\test.xlsx");
	}
	//��������� � ������� ������-��������������:
	bool FindFlg = cells_data.getMethod().isSuccess(col_cnt, failCnt);
	procRowCell(curRow, getColID(), 0, FindFlg);
	return FindFlg;
}

bool TJsonReport::Search_Dest_Data_In_Src_Sheet(TRowsFlag& DstRows, const TExtendSheetReport& srcSheet) noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_ExcelReport::TRowFlag;
	try
	{
		//�������� ������ ������������ ����������:
		CellDataArr cellArr = cells_data.getCellDataArr();
		if (cellArr.empty()) 
			throw TLog("������ ������� ������� ��� ���������!", "TJsonReport::Search_Dest_Data_In_Src_Sheet");
		//�������� �� ������ ������ �����-���������:
		for (size_t row = 0; row < DstRows.size(); row++)
		{
			//���� ������ ������� - ��������� �� �� ������:
			if (Search_DstRow_In_SrcSheet(srcSheet, cellArr, DstRows[row].first) == true)
				DstRows[row].second = false;
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
			"TJsonReport::Search_Dest_Data_In_Src_Sheet").toErrBuff();
	}
	return false;
}

void TJsonReport::Compare_Excel_Sheets() noexcept(false)
{
	using NS_ExcelReport::TRowsFlag;
	using NS_Tune::TShareData;
	//�������� ���������� excel-�����
	if (!book.isValid()) throw TLog("����� �� ����������������!", "TJsonReport::Compare_Excel_Sheets");
	//��������� ������ �� ������������ �����, ������� ������������� �������� ����������:
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
		//��������� ����� ������ ��� ������ ������ �����-��������� �� ����� �����-���������:
		if (Search_Dest_Data_In_Src_Sheet(DestRows, src) == false)
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
		switch (cells_data.getMethodCode())
		{
		case JSonMeth::CompareColor:
			throw TLog("������� ��������� �� �����������!", "ProcessExcelFileByJson");
			break;
		case JSonMeth::CompareIns:
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
	procRows.second.push_back(row_flg);
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
			sheet = book.getActiveSheet();
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

template <typename T>
string TDataBaseInterface::getSqlText(bool by_str, const string& str) noexcept(false)
{
	using std::ifstream;
	//��������� ������ �������� sql-�����: ���� ��� ������
	if (by_str)
	{
		T sql(str);
		return NS_Sql::AsString(sql);
	}
	else
	{
		ifstream sql_txt_file(str, std::ios_base::in);
		if (!sql_txt_file.is_open())
		{
			throw TLog("������ �������� �����: " + str, "TDataBaseSheetReport::getSqlText");
		}
		T sql(sql_txt_file);
		sql_txt_file.close();
		return NS_Sql::AsString(sql);
	}
	return string();
}

void TDataBaseInterface::setSqlParamByTune(NS_Oracle::TStatement& sql, const NS_Tune::TSubParam& param, bool use_raise) noexcept(false)
{
	using NS_Tune::DataType;
	using NS_Converter::toType;
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
			toType(param.Value(), &val);
			sql.setDoubleVal(par_id, val);
			return;
		}
		case DataType::Date:
			sql.setDateAsStringVal(par_id, param.Value());
			return;
		case DataType::String:
			sql.setStringVal(par_id, param.Value());
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
	switch (type)
	{
	//����� � ��������� ������:
	case DataType::Integer:
		return TType::OCCINUMBER;
	case DataType::Double:
		return TType::OCCI_SQLT_NUM;
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
	//��-�� ������������ ���������� ������� � oracle � excel
	size_t resultSetCol = cell.getCol() + 1;
	TDataBaseInterface::setCellByResultSet(book, sheet, rs, resultSetCol, cell);
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
				TExcelCell cell(row, i-1);
				setCellByResultSet(rs, cell);
			}
			row++;
			//���� ��������� ����� ����� �� ��������
			//������� ����� ��������
			if (CreateNewPage(row, true)) 
				row = LastRow();
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

string TDataBaseInterface::getSqlByTune(bool use_parse, bool by_str, const string& str) noexcept(true)
{
	string sql;
	if (use_parse)
		sql = getSqlText<NS_Sql::TText>(by_str, str);
	else
		sql = getSqlText<NS_Sql::TSimpleSql>(by_str, str);
	return sql;
}

string TDataBaseInterface::getSqlByTune(const NS_Tune::TUserTune& tune) noexcept(true)
{
	using NS_Const::TuneField;
	//���� ��������� ������ - �������
	if (tune.Empty()) return string();
	bool parserFlg = false;
	bool byFileFlg = false;
	parserFlg = tune.useFlag(TuneField::UseSqlParser);
	string str = tune.getFieldValueByCode(TuneField::SqlFile);
	if (str.empty())
	{
		byFileFlg = false;
		str = tune.getFieldValueByCode(TuneField::SqlText);
	}
	else
	{
		byFileFlg = true;
		str = tune.getDQLFile();
	}
	return getSqlByTune(parserFlg, !byFileFlg, str);
}


void TDataBaseSheetReport::CrtBySqlLine(NS_Oracle::TDBConnect& db, const string& sql_line, int prefetch) noexcept(false)
{
	bool use_parse = useSqlParse();
	string sql = getSqlByTune(use_parse, true, sql_line);
	//������������ ������ ��� ��������
	FillSheetBySql(db, sql, prefetch);
}

void TDataBaseSheetReport::CrtBySqlFiles(NS_Oracle::TDBConnect& db, int prefetch) noexcept(false)
{
	using std::vector;
	vector<string> sql_lst = tune.getDQLFileLst();
	if (sql_lst.size() < 1)
		throw TLog("������ ����� sql-�������� � ����������!", "TDataBaseSheetReport::CrtBySqlFiles");
	bool use_parse = useSqlParse();
	//���������� ������� ������� �� ������:
	for (const string& sql_file : sql_lst)
	{
		string sql = getSqlByTune(use_parse, false, sql_file);
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
		string dml_txt = getSqlByTune(false, false, dml);
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
	const NS_Oracle::TResultSet& rs, size_t resultSetCol, const NS_Excel::TExcelCell& cell) noexcept(false)
{
	using NS_Oracle::TType;
	using NS_Oracle::UInt;
	using NS_Oracle::SQLException;
	using NS_Oracle::TDate;
	using NS_Excel::TExcelDate;
	using NS_Const::DataType;
	if (!rs.isValid()) throw TLog("�� �������� ������ ResultSet!", "TDataBaseSheetReport::setCellByResultSet");
	//���������� ��������� �� ������ �������� ������ � �������:
	if (rs.isNullVal(resultSetCol))
	{
		sheet.setBlank(cell);
		return;
	}
	//��-�� ������������ ������ ������� ��� excel � oracle
	//UInt i = cell.getCol() + 1;
	const DataType dt = convertOraType(rs.getColumnType(resultSetCol));
	try
	{
		switch (dt)
		{
		case DataType::Integer:
		case DataType::Double:
			sheet.WriteAsNumber(cell, rs.getDoubleVal(resultSetCol));
			break;
		case DataType::String:
			sheet.WriteAsString(cell, rs.getStringVal(resultSetCol));
			break;
		case DataType::Boolean:
			sheet.WriteAsBool(cell, rs.getIntVal(resultSetCol));
			break;
		case DataType::Date:
			if (sheet.isDate(cell))
			{
				TDate date = rs.getDateVal(resultSetCol);
				TExcelDate tmp;
				date.getDate(tmp.year, tmp.month, tmp.day, tmp.hour, tmp.minute, tmp.sec);
				double dbl_date = book.Date2Double(tmp);
				sheet.WriteAsNumber(cell, dbl_date);
			}
			else
				sheet.WriteAsString(cell, rs.getDateAsStrVal(resultSetCol));
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
			TExcelBook book(tune.getOutFileBySheet());
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
	if (js_files.empty()) throw TLog("������ ���������� � js-������� ��������!", "ProcessExcelFileByJson");
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
		pageCnt = 1;
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
	//!!!!!   case ReportCode::FULL_CRED_REPORT:
	//������ � ��������� �������� ��� ���
	case ReportCode::FULL_CRED_REPORT_SUA:
	case ReportCode::EXCEL_SET_DATA_FROM_BASE:
		Json_One_Row_One_DBQuery();
		break;
	//�������� ������ � oracle �� excel
	case ReportCode::LOAD_FROM_FILE:
		throw raise_err(code);
		break;
	//��������� ������ excel
	case ReportCode::FILE_COMPARE_RIB:
		Json_Report_By_Files_Compare();
		break;
	case ReportCode::FILE_COMPARE_RTBK:
		Json_Report_By_Files_Compare();
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