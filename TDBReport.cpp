//модуль определения функционала для TDBReport
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
			TLog log("Превышено число столбцов в отчете на одном листе!\n Число столбцов в выборке: ", "WriteFromResultSet");
			log << item_cnt << TLog::NL << "\nЧисло столбцов в excel-книге: " << max_val;
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
		throw TLog("Страница для отчета: " + book.getFileName() + 
			" не инициализирована!", "TBaseSheetReport::FirstRow");
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
		//если книга пустая - загружаем книгу
		if (book.isEmpty() and !book.load(srcName)) return false;
		//устанавливаем страницу с которой будем работать
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

bool TBaseSheetReport::setCellFormat(size_t Row, size_t Column, NS_Excel::TExcelBookFormat& format) noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_Excel::TExcelBookFormat;
	//если указан недопустимый цвет
	if (!format.isValid()) return false;
	TExcelCell cell(Row, Column, false);
	//если указана пустая ячейка
	if (cell.isEmpty()) return false;
	try
	{
		bool rslt = sheet.setCellFormat(cell, format);
		if (rslt == false)
		{
			TLog log("Ошибка установки формата для ячейки: ", "TExtendSheetReport::setCellColor");
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
		TLog log("Не обработанная ошибка при окраске ячейки:", "TExtendSheetReport::setCellColor");
		log << cell.getName() << '\n';
		log.toErrBuff();
	}
	return false;
}

bool TBaseSheetReport::EqualCellsType(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
	const NS_Excel::TExcelCell& srcCell) const noexcept(false)
{
	using NS_Excel::TDataType;
	//получение типа данных в ячейке источнике:
	TDataType srcType = sheet.getCellType(srcCell);
	//получение типа данных в ячейке приемнике:
	TDataType dstType = dstSheet.getCellType(dstCell);
	return srcType == dstType;
}

bool TBaseSheetReport::checkCellStrVal(const NS_Excel::TExcelCell& cell, const string& val,
		bool showLog) const noexcept(false)
{
	using NS_Const::Trim;
	using NS_Const::LowerCase;
	try
	{
		//формирование ячейки для проверки значения:
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
			if (showLog)
			{
				TLog log("Значение в ячейке: ", "TBaseSheetReport::checkCellStrVal");
				log << tmp << " не соответствует указанному значению: " << str << '\n';
				log << "Ячейка: " << cell.getName() << '\n';
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
		TLog log("Не обработанная ошибка при сравнениии значений ячейки:", "TBaseSheetReport::checkCellStrVal");
		log << cell.getName() << " и значения: " << val << '\n';
		log.toErrBuff();
	}
	return false;
}

bool TBaseSheetReport::InsNewRow(size_t curRow, size_t newRow) noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_Excel::TExcelRange;
	if (curRow == newRow) return true;
	try
	{
		//формируем диапазон строк для вставки:
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
		TLog log("Не обработанная ошибка при добавлении новой строки: ", "TBaseSheetReport::InsNewRow");
		log << newRow << " полсе " << curRow << " строки\n";
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
	//открываем указанный excel-файл приемник:
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
		TLog log("Не обработанная ошибка при проверке данных в ячейке: ", "TExtendSheetReport::isEmptyCell");
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
	//если фильтра нет - условия истины
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
	//заносим все строки
	for (; i <= size; i++) 
	{
		if (all_flg or isCorrectFilter(i))
			rows.push_back(TRowFlag(i, true));
	}
	return rows;
}

bool TExtendSheetReport::dropFromFiltredRows(NS_ExcelReport::TRowsFlag& RowsArr, size_t row) noexcept(true)
{
	using NS_ExcelReport::TRowFlag;
	if (row >= RowsArr.size()) return false;
	for (TRowFlag& cur_row : RowsArr)
	{
		if (cur_row.first == row) cur_row.second = false;
		return true;
	}
	return false;
}

bool TExtendSheetReport::Compare_Cells(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
	const NS_Excel::TExcelCell& srcCell, bool NoSpaceNoCase) const noexcept(true)
{
	using NS_Excel::TDataType;
	using NS_Excel::TExcelDate;
	using std::stringstream;
	try
	{
		//if (QualCellsType(dstSheet, dstCell, srcCell) == false) return false;
		//получение типа данных для ячейки с которой сравниваем:
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
			//считывание данных в источнике:
			double srcVal = sheet.ReadAsNumber(srcCell);
			//проверяем являются ли данные в ячейке - датой:
			if (sheet.isDate(srcCell))
			{
				//если в приемнике не дата:
				if (dstSheet.isDate(dstCell) == false)
				{
					//сравниваем как строки:
					string dstVal = dstSheet.ReadAsString(dstCell);
					TExcelDate srcDate;
					book.Double2Date(srcVal, srcDate);
					throw TLog("Не реализовано сравнение даты со строкой!", "TExtendSheetReport::Compare_Cells");
					//return dstVal == srcDate;
				}
			}
			double dstVal = dstSheet.ReadAsNumber(dstCell);
			return dstVal == srcVal;
		}
		case TDataType::CELLTYPE_STRING: 
		//case TDataType::CELLTYPE_EMPTY:
		{
			string dstVal = dstSheet.ReadAsString(dstCell);
			if (NoSpaceNoCase)
				return checkCellStrVal(srcCell, dstVal, false);
			else
			{
				string srcVal = sheet.ReadAsString(srcCell);
				return dstVal == srcVal;
			}
		}
		default: 
		{
			TLog log("Тип данных с ID: ", "TExtendSheetReport::Compare_Cells");
			log << src_dt << " не обрабатывается!\n";
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
		TLog log("Ошибка сравнения данных в ячейки источника: ", "TExtendSheetReport::Compare_Cells");
		log << srcCell.getName() << " и ячейки приемника: " << dstCell.getName() << '\n';
		if (!tmp.empty()) log << tmp << '\n';
		log.toErrBuff();
	}
	return false;
}

bool TExtendSheetReport::CheckInCell(const NS_Excel::TExcelBookSheet& dstSheet, 
	const NS_Excel::TExcelCell& dstCell, const NS_Excel::TExcelCell& srcCell,
	bool NoSpaceNoCase) const noexcept(true)
{
	//если в ячейке нет данных - выходим
	if (sheet.isEmptyCell(srcCell)) return false;
	//проверяем совпадение:
	return Compare_Cells(dstSheet, dstCell, srcCell, NoSpaceNoCase);
}

bool TExtendSheetReport::CheckInCell(const NS_Excel::TExcelBookSheet& dstSheet, 
	const NS_Excel::TExcelCell& dstCell, size_t srcRow, size_t srcCol, 
	bool NoSpaceNoCase) const noexcept(true)
{
	using NS_Excel::TExcelCell;
	//формирование ячейки источника:
	TExcelCell srcCell(srcRow, srcCol, false);
	return CheckInCell(dstSheet, dstCell, srcCell, NoSpaceNoCase);
}

size_t TExtendSheetReport::CheckOnSheet(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
	size_t srcCol, bool NoSpaceNoCase) const noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_Tune::TIndex;
	size_t curRow = FirstRow();
	size_t lastRow = LastRow();
	//проходим по строкам листа:
	for (; curRow <= lastRow; curRow++)
	{
		//проверяем условия фильтра:
		if (isCorrectFilter(curRow))
		{
			//ищем данные в строке:
			if (CheckInCell(dstSheet, dstCell, curRow, srcCol, NoSpaceNoCase))
				//выходим
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
	//если массив значений пуст - выход
	if (RowsArr.empty()) return TIndex::EmptyIndex;
	//признак использования фильтрации по умолчанию:
	//проходим по строкам листа:
	for (TRowFlag& curRow: RowsArr)
	{
		//если фильтрация для строки прошла успешно:
		if (curRow.second == true)
		{
			//ищем данные в строке:
			if (CheckInCell(dstSheet, dstCell, curRow.first, srcCol, NoSpaceNoCase))
			{
				//убираем строку из массива просматриваемых строк
				curRow.second = false;
				//выходим
				return curRow.first;
			}
		}
	}
	return TIndex::EmptyIndex;
}

bool TExtendSheetReport::NotEquality(const NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
	const NS_Excel::TExcelCell& srcCell, bool NoCaseNoSpace) const noexcept(true)
{
	bool flg = Compare_Cells(dstSheet, dstCell, srcCell, NoCaseNoSpace);
	return !flg;
}

bool TExtendSheetReport::setDstCellBySrcCell(NS_Excel::TExcelBookSheet& dstSheet, const NS_Excel::TExcelCell& dstCell,
	const NS_Excel::TExcelCell& srcCell) const noexcept(true)
{
	using NS_Excel::TDataType;
	try
	{
		//если данные в ячейке источнике - пустые:
		if (sheet.isBlank(srcCell))
		{
			//ставим пустые данные
			if (dstSheet.isBlank(dstCell) == false)
				dstSheet.setBlank(dstCell);
			return true;
		}
		//если типы данных в ячейках не совпадают:
		//if (QualCellsType(dstSheet, dstCell, srcCell) == false) return false;
		//получение типа данных в ячейке источнике:
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
			//если значения отличаются - заменяем
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
			TLog log("Указанный тип данных: ", "TExtendSheetReport::setDstCellBySrcCell");
			log << srcType << " не обрабатывается\n";
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
		TLog log("Ошибка добавления данных в ячейку: ", "TExtendSheetReport::setDstCellBySrcCell");
		log << dstCell.getName() << " листа: " << dstSheet.getName() << " из ячейки: " << srcCell.getName()
			<< " листа: " << sheet.getName() << '\n';
		if (!tmp.empty()) log << tmp << '\n';
		log.toErrBuff();
	}
	return false;
}

TJsonReport::TJsonReport(NS_Excel::TExcelBook& book_ref, const TShareData& DstFile, const TProcCell& cell_arr, size_t page):
	TExtendSheetReport(book_ref, DstFile, page), cells_data(cell_arr), meth_code(NS_Const::JSonMeth::Null)
{
	meth_code = cells_data.getMethodCode();
	//формирование форматов закраски:
	crtCellFillFormatArr();
}

TJsonReport::TJsonReport(NS_Excel::TExcelBook& book_ref, const TExcelProcData& json_tune, size_t page) :
	TExtendSheetReport(book_ref, json_tune.getDstFile(), page), cells_data(json_tune.getCellsData()),
	meth_code(NS_Const::JSonMeth::Null)
{
	meth_code = cells_data.getMethodCode();
	//формирование форматов закраски:
	crtCellFillFormatArr();
}

bool TJsonReport::CorrectFilter(size_t cur_row) const noexcept(true)
{
	return TExtendSheetReport::isCorrectFilter(cur_row);
}

void TJsonReport::setDMLOutParam(NS_Oracle::TStatement& query, const NS_Tune::TCellData& param) noexcept(false)
{
	NS_Oracle::TType out_type = TDataBaseInterface::convertDataType(param.getOutType());
	if (out_type == TDataBaseInterface::OraTypeError)
		throw TLog("Ошибка преобразования данных в тип данных Oracle!", "TJsonReport::setDMLOutParam");
	query.registerOutParam(param.SrcParam(), out_type);
}

void TJsonReport::setDQLParamByCell(TStatement& query, const NS_Tune::TCellData& value, size_t curRow) const noexcept(false)
{
	using NS_Excel::TExcelCell;
	using NS_Const::DataType;
	using NS_Const::TConstType;
	using NS_Oracle::TDate;
	using NS_Excel::TExcelDate;
	const DataType& data_type = value.getInType();
	//инициализируем ячейку
	TExcelCell cell(curRow, value.DstIndex(), false);
	//если ячейка пустая
	if (sheet.isBlank(cell))
	{
		query.setNullVal(value.SrcParam(), TDataBaseInterface::convertDataType(data_type));
		return;
	}
	//если там есть данные
	switch (data_type)
	{
		case DataType::String:
		{
			//считываем из excel-файла
			string tmp = sheet.ReadAsString(cell);
			if (tmp.empty())
				query.setNullVal(value.SrcParam(), TDataBaseInterface::convertDataType(DataType::String));
			else
				//вписываем в параметр запроса
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
			TLog log("Указанный тип данных: ", "TJsonReport::setDQLParamByCell");
			log << TConstType::asStr(value.getInType()) << " не обрабатывается!\n";
			throw log;
	}
}

bool TJsonReport::setStatementParam(NS_Oracle::TStatement& query, const NS_Tune::TCellData& value, size_t Row) const noexcept(true)
{
	using NS_Const::DataType;
	//переменная отслеживания ошибки при установке i-ого параметра
	size_t ColIndx = 0;
	try
	{
		//если параметр пустой/ошибочный - продолжаем
		if (value.isEmpty())
			return true;
		//установка выходного параметра:
		if (value.isOutParam())
		{
			ColIndx = value.SrcParam();
			//установка выходного параметра:
			setDMLOutParam(query, value);
		}
		//установка входного параметра
		else
		{
			ColIndx = value.DstIndex();
			//установка параметра запроса для данной ячейки:
			setDQLParamByCell(query, value, Row);
		}
		return true;
	}
	catch (TLog& err)
	{
		err << "Ошибка при установке параметра: " << ColIndx << '\n';
		err.toErrBuff();
	}
	catch (const NS_Oracle::SQLException& err)
	{
		TLog log("Ошибка установки параметра: ", "TJsonReport::setStatementParam");
		log << ColIndx << '\n' << err.what() << '\n';
		log.toErrBuff();
	}
	catch (...)
	{
		TLog log("Не обработанная ошибка при установке параметра: ", "TJsonReport::setStatementParam");
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
		//получение массива параметров:
		vector<TCellData> params = cells_data.getCellDataArr();
		//если параметры пустые - устанавливать ни чего не надо
		if (params.empty()) return true;
		//проходим по всем параметрам и устанавливаем их в запрос:
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
		TLog log("Не обработанная ошибка при установке параметров запроса для строки: ", "TJsonReport::setSelectParams");
		log << curRow << '\n';
		log.toErrBuff();
	}
	return false;
}

void TJsonReport::WriteOutParamToExcel(NS_Oracle::TBaseSet& bs, const NS_Const::DataType& type_code,
	const NS_Tune::TCellData& param, size_t OutParamIndex, size_t Row) noexcept(false)
{
	using NS_Excel::TExcelCell;
	//если индекс для вставки пустой - продолжаем:
	//if (param.EmptyInsIndx()) return;
	//формируем колонку:
	TExcelCell cell(Row, param.InsIndex(), false);
	//вставляем данные в ячейку
	TDataBaseInterface::setCellByResultSet(book, sheet, type_code, bs, OutParamIndex, cell);
}

void TJsonReport::writeExcelFromDB(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false)
{
	using NS_Excel::TExcelCell;
	using NS_Tune::TIndex;
	using NS_Const::DataType;
	//получение массива параметров:
	vector<TCellData> params = cells_data.getCellDataArr();
	//если параметры пустые - устанавливать ни чего не надо
	if (params.empty()) return;
	//получение числа колонок запроса:
	//size_t colCnt = rs.getColumnsCnt();
	//проходим по результату запроса:
	while (rs.Next())
	{
		//проходим по всем параметрам и заполняем их:
		for (const TCellData& cd : params)
		{
			size_t OutParamIndx = cd.SrcVal();
			//получение типа данных для ячейки:
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

void TJsonReport::insertToDataBase(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false)
{
	throw TLog("Функция вставки данных в БД НЕ реализована!", "TJsonReport::insertToDataBase");
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
		case JSonMeth::SendToDB:
		{
			insertToDataBase(rs, curRow);
			return true;
		}
	}
	return false;
}

void TJsonReport::ProcessByStatement(NS_Oracle::TStatement& query, size_t curRow) noexcept(false)
{
	using NS_Excel::TExcelCell;
	using NS_Const::DataType;
	//получение массива параметров:
	vector<TCellData> params = cells_data.getCellDataArr();
	//если параметры пустые - устанавливать ни чего не надо
	if (params.empty()) return;
	//проходим по всем параметрам и заполняем их:
	for (const TCellData& cd : params)
	{
		if (cd.EmptyInsIndx()) continue;
		size_t OutParamIndx = cd.SrcParam();
		//получение типа данных для ячейки:
		DataType col_type = cd.getOutType();
		WriteOutParamToExcel(query, col_type, cd, OutParamIndx, curRow);
	}
}

bool TJsonReport::runQuery(NS_Oracle::TStatement& query, size_t curRow) noexcept(true)
{
	using NS_Oracle::TSQLState;
	try
	{
		//выолняем запрос:
		if (query.ExecuteSQL() == false) return false;
		//получаем статус запроса:
		TSQLState state = query.getState();
		switch (state)
		{
			//обрабатываем данные из результатов запроса
			case TSQLState::RESULT_SET_AVAILABLE:
			{
				//получение ссылки на ResultSet
				NS_Oracle::ResultSetPtr rsp = query.getResultSetVal();
				//выполнение запроса
				TResultSet rs(rsp);
				//установка значений из запроса в excel-файл
				bool rslt = ProcessByResultSet(rs, curRow);
				//закрываем полученный набор
				query.closeResultSet(rsp);
				return rslt;
			}
			//обрабатываем данные после выполнения хранимой процедуры
			case TSQLState::UPDATE_COUNT_AVAILABLE:
			{
				ProcessByStatement(query, curRow);
				return true;
			}
			default:
			{
				TLog log("Указанное состояние ", "TJsonReport::runQuery");
				log << state << " не обрабатывается!\n";
				throw log;
			}
		}
	}
	catch (const NS_Oracle::SQLException& err)
	{
		TLog log("Ошибка выполнения запроса: ", "TJsonReport::runQuery");
		log << query.getSQL() << "\nДля строки: " << curRow << '\n' << err.what() << '\n';
		log.toErrBuff();
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("Не обработанная ошибка при выполнении запроса: ", "TJsonReport::runQuery");
		log << query.getSQL() << "\nДля строки: " << curRow << '\n';
		log.toErrBuff();
	}
	return false;
}

bool TJsonReport::setExcelRowDataByBD(NS_Oracle::TStatement& query, size_t curRow) noexcept(true)
{
	//проверяем условие для фильтрации строк в excel-файле:
	if (CorrectFilter(curRow))
	{
		//установка параметров запроса:
		if (setSelectParams(query, curRow))
		{
			//выполняем запрос:
			bool find_flg = runQuery(query, curRow);
			//окраска ячейки-идентификатора
			ColoringRowCell(curRow, getColID(), 0, find_flg);
			return find_flg;
		}
	}
	return false;
}

void TJsonReport::setExcelDataByDB(NS_Oracle::TStatement& query, size_t& rowFrom) noexcept(false)
{
	using NS_Tune::TIndex;
	if (rowFrom <= 0)
	{
		TLog log("Не верно указана начальная строка обработки: ", "TJsonReport::setExcelDataByDB");
		log << rowFrom << '\n';
		throw log;
	}
	//получение номера последней обрабатываемой строки
	size_t rowTo = LastRow();
	//считывание строк excel-файла
	for (rowFrom; rowFrom <= rowTo; rowFrom++)
	{
		setExcelRowDataByBD(query, rowFrom);
	}
}

void TJsonReport::UpdExcelDataByDB(NS_Oracle::TDBConnect& db, const NS_Tune::TUserTune& tune, size_t& rowFrom) noexcept(false)
{
	using NS_Tune::TIndex;
	using NS_Oracle::TStatement;
	//если строка от которой идет считывание не установлена - берем ее из настроек
	if (rowFrom == TIndex::EmptyIndex)
		rowFrom = FirstRow();
	//получение текста запроса:
	string sql = TDataBaseInterface::getSqlByTune(tune);
	//формирование запроса к БД:
	TStatement query(db, sql, 1);
	//установка постоянных параметров:
	TDataBaseInterface::setSqlParamsByTune(query, tune);
	setExcelDataByDB(query, rowFrom);
	query.close();
}

void TJsonReport::ProcessSheetDataWithDB() noexcept(false)
{
	if (!book.isValid()) throw TLog("Книга не инициализирована!", "TJsonReport::SheetDataFromDataBase");
	//получение первой строки для обработки
	size_t row = FirstRow();
	//для каждой настройки подключения выполняем:
	for (const TUserTune& config : cells_data.getDBTuneArr())
	{
		//получение параметров подключения к БД
		NS_Oracle::TConnectParam cp = TDataBaseInterface::getConnectParam(config);
		//создаем подключение к БД:
		TDBConnect db(cp);
		if (!db.isValid()) throw TLog("Ошибка подключения к БД: " + cp.tns_name, "TJsonReport::SheetDataFromDataBase");
		try
		{
			//обновляем данные в excel-файле:
			UpdExcelDataByDB(db, config, row);
		}
		catch (TLog& err)
		{
			err << "Последняя обработанная строка: " << row << '\n';
			err.toErrBuff();
		}
		catch (...)
		{
			TLog log("Не обработанная ошибка при работе со строкой: ", "TJsonReport::SheetDataFromDataBase");
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
	//если формат не валидет - выходим
	if (init_format.isValid())
	{
		TColor color = TColor::COLOR_NONE;
		//получение цвета заливки из настроек:
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
				//меняем цвет шрифта
				TExcelBookFont font = result.getFont();
				font.setColor(color);
				result.setFont(font);
			}
			else
			{
				//меняем цвет ячейки
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
	//получение формата текущей ячейки
	TExcelCell cell(Row, Col, false);
	TExcelBookFormat cur_format = sheet.getCellFormat(cell);
	//формирование новых форматов для текущей ячейки
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
	//добавление формата для ячейки-идентификатора:
	bool colID_flg = fill_code == JsonCellFill::ID_And_CurCell;
	if (colID_flg)	crtCellFillFormat(Row, getColID(), false);
	//получение ссылки на массив обрабатываемых колонок
	const CellDataArr& arr = cells_data.getCellDataArr();
	//установка формата для всех указанных в паратрах ячеек:
	for (const TCellData& param : arr)
	{
		//если параметр пустой - берем следующий
		if (param.isEmpty()) continue;
		//если ячейка параметра совпадает с ячейкой-идентификатором - пропускаем данный шаг
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
		//проверяем наличие обрабатываемых ячеек:
		if (cells_data.CellCnt() <= 0) return;
		//получение ссылки на метод обработки:
		const TCellMethod& meth = cells_data.getMethod();
		//если не указан метод заливки или цвета пустые - выходи
		if (meth.isEmptyColor()) return;
		//получение ссылки на строку
		size_t Row = FirstRow();
		const NS_Const::JsonCellFill fill_code = meth.getFillType();
		//формируем массив форматов ячеек:
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
		TLog("Не обработанная ошибка при установке формата!\n", "TJsonReport::crtCellFillFormatArr").toErrBuff();
	}
	frmt_arr.clear();
}

bool TJsonReport::ColoringRowCell(size_t Row, size_t Col, size_t frmt_index, bool find_flg) noexcept(true)
{
	using NS_Excel::TExcelBookFormat;
	using NS_Excel::TExcelCell;
	if (frmt_index >= frmt_arr.size()) return false;
	//получение формата ячейки из списка форматов:
	try
	{
		TFillFormat fill = frmt_arr[frmt_index];
		int FormatIndex = find_flg ? fill.first : fill.second;
		FormatIndex--;//т.к. в excel индексы от 0
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
		TLog log("Не обработанная ошибка при установке формата ячейке: ", "TJsonReport::procRowCell");
		log << cell.getName() << '\n';
		log.toErrBuff();
	}
	return false;
}

bool TJsonReport::ColoringRowCell(const NS_Excel::TExcelCell& cell, size_t frmt_index, bool find_flg) noexcept(true)
{
	return ColoringRowCell(cell.getRow(false), cell.getCol(false), frmt_index, find_flg);
}

bool TJsonReport::ColoringCellByParam(const NS_Tune::TCellData& param, size_t curRow, size_t frmt_index, bool fing_flg) noexcept(true)
{
	using NS_Const::JSonMeth;
	using NS_Const::TConstJSMeth;
	using NS_Tune::TIndex;
	using NS_Excel::TExcelCell;
	size_t column = 0;
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
			TLog log("Указанный метод: ", "TJsonReport::procFindCell");
			log << TConstJSMeth::asStr(meth_code) << " не обрабатывается!\n";
			log.toErrBuff();
			return false;
		}
	}
	if (column == TIndex::EmptyIndex)
		return false;
	//формируем колонку:
	TExcelCell cell(column, curRow, false);
	return ColoringRowCell(cell, frmt_index, fing_flg);
}


bool TJsonReport::CellHasChanged(const TExtendSheetReport& srcSheet, const NS_Tune::TCellData& param,
	size_t dstRow, size_t srcRow) const noexcept(true)
{
	using NS_Excel::TExcelCell;
	//формируем ячейки приемника:
	size_t tmpCol = param.InsIndex();
	TExcelCell dstCell(dstRow, tmpCol, false);
	//формируем ячейку источника:
	tmpCol = param.SrcVal();
	TExcelCell srcCell(srcRow, tmpCol, false);
	return srcSheet.NotEquality(sheet, dstCell, srcCell);
}

bool TJsonReport::InsertDstCellBySrcCell(const TExtendSheetReport& srcSheet, const NS_Tune::TCellData& param,
	size_t dstRow, size_t srcRow) noexcept(true)
{
	using NS_Excel::TExcelCell;
	size_t tmpCol = param.InsIndex();
	//ячейка приемника:
	TExcelCell DstCell(dstRow, tmpCol, false);
	//ячейка источника
	tmpCol = param.SrcVal();
	TExcelCell SrcCell(srcRow, tmpCol, false);
	//вставляем требуемые данные в ячейку приемника из ячейки страницы источника:
	return srcSheet.setDstCellBySrcCell(sheet, DstCell, SrcCell);
}

bool TJsonReport::procFindCell(const TExtendSheetReport& srcSheet, const NS_Tune::TCellData& param,
	size_t dstRow, size_t srcRow) noexcept(true)
{
	using NS_Const::JSonMeth;
	using NS_Const::TConstJSMeth;
	//если параметры для вставки данных в ячейку не заполнены:
	if (param.EmptyInsIndx() or param.EmptySrcVal())
	{
		TLog("Пустые параметры для вставки данных в ячейки!", "TJsonReport::procFindCell").toErrBuff();
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
			//закраска ячейки:
			//ColoringCellByParam(param, dstRow, true);
			//вставка данных:
			return InsertDstCellBySrcCell(srcSheet, param, dstRow, srcRow);
		}
		default:
		{
			TLog log("Указанный метод: ", "TJsonReport::procFindCell");
			log << TConstJSMeth::asStr(meth_code) << " не обрабатывается!\n";
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
	//тип данных: ячейка для обработки - 1 приемник, 2 источник
	using POutParams = std::vector<const TCellData*>;
	//формируем массив из выходных параметров ячеек файла-источника
	POutParams outArr;
	for (const TCellData& param : params)
	{
		if (param.EmptyInsIndx() or param.EmptySrcVal()) continue;
		TExcelCell tmpCell(srcRow, param.SrcVal());
		if (srcSheet.isEmptyCell(tmpCell) == false)
			outArr.push_back(&param);
	}
	//если выходных параметров нет -выход:
	if (outArr.empty()) return true;
	//добавляем новую строку:
	if (InsNewRow(dstRow, dstRow + 1) == true)
		dstRow = dstRow + 1;
	else
		return false;
	size_t cnt = 0;
	for (const TCellData* param : outArr)
	{
		if (procFindCell(srcSheet, *param, dstRow, srcRow) == true)
			cnt++;
	}
	//выполняем окраску строки
	bool flg = (cnt == outArr.size()) ? true : false;
	TExcelCell DstCell(dstRow, getColID(), false);
	ColoringRowCell(DstCell, 0, flg);
	return flg;
}

bool TJsonReport::DstCell_In_SrcCell(const TExtendSheetReport& srcSheet, 
	const NS_Excel::TExcelCell& DstCell, const NS_Excel::TExcelCell& SrcCell,
	bool NoSpaceNoCase) noexcept(true)
{
	using NS_Excel::TExcelCell;
	try
	{
		//проверяем есть ли данные в указанной ячейке:
		if (sheet.isEmptyCell(DstCell)) return false;
		//формируем данные по строке источника:
		return srcSheet.CheckInCell(sheet, DstCell, SrcCell, NoSpaceNoCase);
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("Не обработанная ошибка при поиске ячейки:", "Get_DstCell_In_SrcRow");
		log << DstCell.getName() << " в ячейке источника: " << SrcCell.getName() << '\n';
		log.toErrBuff();
	}
	return false;
}

size_t TJsonReport::getSrcRow_By_DstCell(const TExtendSheetReport& srcSheet, NS_ExcelReport::TRowsFlag& srcRows,
	const NS_Excel::TExcelCell& DstCell, size_t SrcCol, bool NoSpaceNoCase) noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_Tune::TIndex;
	//проверяем есть ли данные в указанной ячейке:
	if (sheet.isEmptyCell(DstCell)) return TIndex::EmptyIndex;
	//выполняем поиск ячейки на листе источника:
	return srcSheet.CheckOnSheet(sheet, DstCell, SrcCol, srcRows, NoSpaceNoCase);
}

size_t TJsonReport::DstRow_In_SrcSheet(const TExtendSheetReport& srcSheet, 
	NS_ExcelReport::TRowsFlag& srcRows, const CellDataArr& params, size_t curRow,
	bool NoSpaceNoCase) noexcept(true)
{
	using NS_Tune::CellDataArr;
	using NS_Tune::TCellData;
	using NS_Excel::TExcelCell;
	using NS_Tune::TIndex;
	using NS_ExcelReport::TRowsFlag;
	size_t srcRow = TIndex::EmptyIndex;
	size_t cntFail = 0;
	for (const TCellData& param : params)
	{
		//если нет данных по колонкам из строки приемника/источника - переходим к следующей строке
		if (param.EmptyDstIndx() and param.EmptySrcParam()) continue;
		//формирование ячейки приемника:
		size_t tmpCol = param.DstIndex();
		TExcelCell DstCell(curRow, tmpCol, false);
		tmpCol = param.SrcParam();
		//если это первый попавшийся параметр
		if (srcRow == TIndex::EmptyIndex)
		{
			//выполняем поиск на совпадение для первого параметра/колонки приемника:
			srcRow = getSrcRow_By_DstCell(srcSheet, srcRows, DstCell, tmpCol, NoSpaceNoCase);
			//если для указанной ячейки не нашлось строки источника - данные не найдены
			if (srcRow == TIndex::EmptyIndex) return TIndex::EmptyIndex;
		}
		else
		{
			//формирование ячейки источника для указанной строки
			TExcelCell SrcCell(srcRow, tmpCol, false);
			//если не найдена хоть одна ячейка строки: строка приемника - не найдена
			if (DstCell_In_SrcCell(srcSheet, DstCell, SrcCell, NoSpaceNoCase) == false)
				return TIndex::EmptyIndex;
		}
	}
	return srcRow;
}

bool TJsonReport::DstRowCells_In_SrcSheet(const TExtendSheetReport& srcSheet, NS_ExcelReport::TRowsFlag& srcRows,
	const NS_Tune::CellDataArr& params, size_t curRow, bool NoSpaceNoCase) noexcept(true)
{
	using NS_Tune::CellDataArr;
	using NS_Tune::TCellData;
	using NS_Excel::TExcelCell;
	using NS_Tune::TIndex;
	using NS_ExcelReport::TRowsFlag;
	//счетчик не найденных ячеек:
	int failCnt = 0, col_cnt = 0;
	//для каждой строки в файле приемнике - обрабатываем указанные в json-файле колонки
	for (const TCellData& param : params)
	{
		//если нет данных по колонкам из строки приемника/источника - переходим к следующей строке
		if (param.EmptyDstIndx() and param.EmptySrcParam()) continue;
		//формирование ячейки приемника
		size_t tmpCol = param.DstIndex();
		TExcelCell dstCell(curRow, tmpCol, false);
		//если ячейка в приемнике для сравнения - пустая - идем дальше
		if (sheet.isEmptyCell(dstCell)) continue;
		col_cnt++;
		//выполняем поиск на листе источника:
		tmpCol = param.SrcParam();
		size_t srcRow = srcSheet.CheckOnSheet(sheet, dstCell, tmpCol, srcRows, NoSpaceNoCase);
		//если параметр не найден в строке источника:
		if (srcRow == TIndex::EmptyIndex)
		{
			failCnt++;
			//окраска текущей ячейи, если данные не найденны
			ColoringRowCell(dstCell, col_cnt, false);
		}
		else
		{
			//обрабатываем найденные данные в  ячейках:
			bool flg = procFindCell(srcSheet, param, curRow, srcRow);
			//
			if (WithChangeMeth() and flg == false) continue;
			//окраска ячейки в зависимости от вставляемых данных:
			ColoringRowCell(dstCell, col_cnt, true);
		}
	}
	//результат поиска в зависимости от метода обработки:
	bool FindFlg = cells_data.getMethod().isSuccess(col_cnt, failCnt);
	return FindFlg;
}

bool TJsonReport::Search_DestData_In_SrcSheet(TRowsFlag& DstRows, 
	const TExtendSheetReport& srcSheet, bool NoSpaceNoCase) noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_ExcelReport::TRowFlag;
	using NS_Const::JSonMeth;
	using NS_Const::TConstJSMeth;
	using NS_Tune::TIndex;
	try
	{
		//получаем список сравниваемых аттрибутов:
		CellDataArr cellArr = cells_data.getCellDataArr();
		if (cellArr.empty()) 
			throw TLog("Пустые индексы колонок для сравнения!", "TJsonReport::Search_DestData_In_SrcSheet");
		//формируем массив строк источника для обработки:
		TRowsFlag srcRows = srcSheet.setFiltredRowsArr();
		if (srcRows.empty())
			throw TLog("На странице файла источника нет данных подходящих под условия!",
				"TJsonReport::Search_DestData_In_SrcSheet");
		//проходим по каждой строке листа-приемника:
		for (size_t index = 0; index < DstRows.size(); index++)
		{
			TLog log("Идет обработка: ");
			log << index << " сткроки из " << DstRows.size() << " строк";
			log.toErrBuff();
			//не обрабатываем ячейки, которые были обработаны ранее
			if (DstRows[index].second == false) continue;
			bool flg = false;
			//выполняем проверку наличия ячеек строки файла-приемника в каждой строке файла-источника
			switch (meth_code)
			{
			case JSonMeth::CompareCell:
			case JSonMeth::CompareCellChange:
			{
				flg = DstRowCells_In_SrcSheet(srcSheet, srcRows, cellArr, DstRows[index].first, NoSpaceNoCase);
				//исключение строки приемника из списка обработки
				DstRows[index].second = !flg;
				if (WithChangeMeth() and flg) continue;
				break;
			}
			case JSonMeth::CompareRow:
			case JSonMeth::InsertRowCompare:
			{
				size_t srcRow = DstRow_In_SrcSheet(srcSheet, srcRows, cellArr, DstRows[index].first, NoSpaceNoCase);
				if (srcRow == TIndex::EmptyIndex) break;
				//обработка строки:
				procFindRow(srcSheet, cellArr, DstRows[index].first, srcRow);
				flg = true;
				//только для метода вставки новой строки для сравнения
				if (meth_code == JSonMeth::InsertRowCompare)
				{
					//убираем текущую строку из фильтрации
					DstRows[index].second = false;
					//пересчитать значения в массиве строк приемника - т.к. добавили строку:
					index++;
					TRowFlag NewRowFlg = DstRows[index];
					NewRowFlg.first++;
					DstRows.push_back(NewRowFlg);
					//убираем добавленную строку
					DstRows[index].second = false;
				}
				break;
			}
			default:
			{
				TLog log("Указанный метод(", "Search_DestRow_In_SrcSheet");
				log << TConstJSMeth::asStr(meth_code) << ") не обрабатывается!";
				throw log;
			}
			}
			//окраска ячейки-идентификатора:
			ColoringRowCell(DstRows[index].first, getColID(), 0, flg);
		}
		return true;
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка поиска данных из страницы-приемника в странице-источнике!", 
			"TJsonReport::Search_DestRow_In_SrcSheet").toErrBuff();
	}
	return false;
}

void TJsonReport::Compare_Excel_Sheets(bool NoSpaceNoCase) noexcept(false)
{
	using NS_ExcelReport::TRowsFlag;
	using NS_Tune::TShareData;
	//проверка валидности excel-книги
	if (!book.isValid()) throw TLog("Книга не инициализирована!", "TJsonReport::Compare_Excel_Sheets");
	//формируем массив из сравниваемых строк, которые удовлетворяют условиям фильтрации:
	TRowsFlag DestRows = TExtendSheetReport::setFiltredRowsArr();
	if (DestRows.empty()) throw TLog("На листе не найдено строк удовлетворяющих фильтру!", "TJsonReport::Compare_Excel_Sheets");
	//получаем число листов в файле источнике:
	size_t srcPageCnt = 0;
	//данные о файле источнике:
	const TShareData* src_file_data = cells_data.getSrcFilRef();
	if (!src_file_data) throw TLog("Не указаны данные о файле источнике!", "TJsonReport::Compare_Excel_Sheets");
	srcPageCnt = src_file_data->getPageCnt();
	//получение имени файла-источника
	string src_name = src_file_data->getName();
	//инициализация книги для файла-источника
	TExcelBook src_book(src_name);
	//проходим по каждому листу файла источника:
	for (size_t i = 0; i < srcPageCnt; i++)
	{
		//инициализация листа файла-источника
		TExtendSheetReport src(src_book, *src_file_data, i);
		//выполняем поиск данных для строки файла-приемника на листе файла-источника:
		if (Search_DestData_In_SrcSheet(DestRows, src, NoSpaceNoCase) == false)
		{
			TLog("При сверке страницы приемника: " + getSheetName() + " и страницы источника: " +
				src.getSheetName() + " произошла ошибка!", "TJsonReport::Compare_Excel_Sheets").toErrBuff();
		}
	}
}

bool TJsonReport::crtSheetReport() noexcept(true)
{
	using NS_Const::JSonMeth;
	try
	{
		//выполнение дальнейших действий в зависимости от метода обработки:
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
		TLog("Необработанная ошибка при формировании страницы отчета файла!", 
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
//наверное имеет смысл отсекать фильтр именно внултри json-файла
//	if (getMethCode() != NS_Const::JSonMeth::GetRowIDByDB and procRows.second.size() > 0)
//		return true;
	return TJsonReport::CorrectFilter(cur_row);
}

bool TJsonMarkReport::checkINDataBase(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false)
{
	//если запрос вернул данные
	bool f = TJsonReport::checkINDataBase(rs, curRow);
	TRowFlag row_flg = TRowFlag(curRow, f);
	procRows.second.push_back(row_flg);
	return f;
}

void TJsonMarkReport::setExcelDataByDB(TStatement& query, size_t& rowFrom) noexcept(false)
{
	using NS_ExcelReport::TRowFlag;
	//установка числа итераций к БД:
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
	//если это объект для фильтрации - получаем имя сервера с которого шла фильтрация
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
	//если страница пустая:
	if (!ref_sheet.isValid())
		//создаем ее
		SetSheetByTune();
}

bool TSheetTuneReport::SetSheetByTune(const string& name) noexcept(true)
{
	using NS_Excel::TStrArr;
	const bool active_sheet = true;
	//проверка настраиваемых параметров
	if (tune.Empty())
	{
		TLog("Пустые данные из файла настроек!", "TSheetTuneReport::SetSheetTune").toErrBuff();
		return false;
	}
	//проверяем указан ли шаблон
	string tmp_val = tune.getTemplateFile();
	string sh_name = name;
	if (sh_name.empty()) sh_name = tune.getOutSheet();
	//если шаблон не указан - создаем страницу
	if (tmp_val.empty())
	{
		//получение наименование 
		TStrArr head = tune.getColumns();
		book.setHeaderByStrArr(head, false, sh_name);
		sheet = book.getActiveSheet();
	}
	//если шаблон указан - грузим его
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
	//по числу столбцов новой страницы не создается!!!
	if (NeedNewPage(cur_val, byRows))
	{
		int indx = book.getActiveSheetIndx() + 1;
		string sh_delimeter = NS_Const::TConstExclTune::asStr(NS_Const::TExclBaseTune::PageDelimiter);
		//задаем имя новой странице
		string new_name = sheet.getName() + sh_delimeter + NS_Converter::toStr(indx+1);
		//добавление страницы
		SetSheetByTune(new_name);
		return true;
	}
	return false;
}

TConnectParam TDataBaseInterface::getConnectParam(const NS_Tune::TUserTune& param, int prefetch) noexcept(false)
{
	TConnectParam result;
	if (param.Empty()) throw TLog("Параметры отчета не заполнены!", "TDataBaseSheetReport::getConnectParam()");
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
	//проверяем откуда получаем sql-текст: файл или строка
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
			throw TLog("Ошибка открытия файла: " + str, "TDataBaseSheetReport::getSqlText");
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
	//если значение параметра - пустое:
	if (param.Value().empty())
	{
		sql.setNullVal(param.ID(), TDataBaseInterface::convertDataType(data_type));
		return;
	}
	//проверяем имеется ли нужный нам параметр:
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
		case DataType::SQL_String:
			sql.setSqlStringVal(par_id, param.Value());
			return;
		default: 
			throw TLog("Указанный тип данных " + NS_Const::TConstType(param.DataType()).toStr() + " НЕ обрабатывается!", "TDataBaseSheetReport::setSqlParamByTune");
		}
	}
	catch (const oracle::occi::SQLException& err)
	{
		TLog log("Ошибка установки значения: " + param.Value() + " для параметра: " + param.Comment() + TLog::NL, "TDataBaseSheetReport::setSqlParamByTune");
		log << err.what() << TLog::NL;
		raise_app_err(log, use_raise);
	}
	catch (const TLog& er)
	{
		raise_app_err(er, use_raise);
	}
	catch (...)
	{
		TLog log("Неорпеделенная ошибка установки значения: " + param.Value() + " для параметра: " + param.Comment() + TLog::NL);
		raise_app_err(log, use_raise);
	}
}

void TDataBaseInterface::setSqlParamsByTune(NS_Oracle::TStatement& sql, const NS_Tune::TUserTune& param) noexcept(false)
{
	//если пустой sql - выход
	if (!sql.isValid()) throw TLog("Не валидная sql-команда: " + sql.getSQL(), "setSqlParamsByTunes");
	string sql_text = sql.getSQL();
	if (sql_text.empty()) throw TLog("Пустой текст sql-команды", "setSqlParamsByTunes");
	//проверяем количество параметров:
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
	//числа с плавающей точкой:
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
	//числа с плавающей точкой:
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
	//из-за несовпадения индексации колонок в oracle и excel
	size_t resultSetCol = cell.getCol() + 1;
	DataType code = convertOraType(rs.getColumnType(resultSetCol));
	TDataBaseInterface::setCellByResultSet(book, sheet, code, rs, resultSetCol, cell);
}

bool TDataBaseSheetReport::WriteFromResultSet(NS_Oracle::TResultSet& rs) noexcept(true)
{
	using NS_Oracle::UInt;
	using NS_Excel::TExcelCell;
	try
	{
		UInt col_cnt = rs.getColumnsCnt();
		//если число колонок превышает ограничения excel - выходим
		CreateNewPage(col_cnt, false);
		size_t row = LastRow()+1;
		while (rs.Next())
		{
			for (UInt i = 1; i <= col_cnt; i++)
			{
				TExcelCell cell(row, i, false);
				setCellByResultSet(rs, cell);
			}
			row++;
			//если привысили число строк на странице
			//создаем новую страницу
			if (CreateNewPage(row, true)) 
				row = LastRow() + 1;
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
		TLog("Необработання ошибка записи данных из базы на лист excel-документа!", "TDataBaseSheetReport::WriteFromResultSet").toErrBuff();
	}
	return false;
}

void TDataBaseSheetReport::FillSheetBySql(NS_Oracle::TDBConnect& db, const string& sql_txt, int prefetch)
{
	if (sql_txt.empty())
		throw TLog("Ошибка формирования отчета - пустой текст запроса!", "TDataBaseSheetReport::FillSheetBySql");
	if (!db.isValid())
		throw TLog("Ошибка формирования отчета - ошибка при подключении к БД!", "TDataBaseSheetReport::FillSheetBySql");
	//создание sql-команды:
	TStatement st(db, sql_txt, prefetch);
	//устанавливаем параметры запроса:
	setSqlParamsByTune(st, tune);
	//выполняем запрос:
	TResultSet rs(st);
	//функция записи данных в excel из результата запроса:
	WriteFromResultSet(rs);
	//закрываем источник данных запроса:
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
	//если настройки пустые - выходим
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
	//формирование данных для страницы
	FillSheetBySql(db, sql, prefetch);
}

void TDataBaseSheetReport::CrtBySqlFiles(NS_Oracle::TDBConnect& db, int prefetch) noexcept(false)
{
	using std::vector;
	vector<string> sql_lst = tune.getDQLFileLst();
	if (sql_lst.size() < 1)
		throw TLog("Пустой списк sql-запросов в директории!", "TDataBaseSheetReport::CrtBySqlFiles");
	bool use_parse = useSqlParse();
	//выполнение каждого запроса из списка:
	for (const string& sql_file : sql_lst)
	{
		string sql = getSqlByTune(use_parse, false, sql_file);
		//выполнение DQL-запроса и заполнение данными листа
		FillSheetBySql(db, sql, prefetch);
	}
}

size_t TDataBaseInterface::executeDML(NS_Oracle::TDBConnect& db, const NS_Tune::TUserTune& param,
	const string& dml, bool use_comit) noexcept(false)
{
	//выполняем данную команду:
	//создание dml-команды:
	TStatement st(db, dml);
	//устанавливаем параметры запроса:
	setSqlParamsByTune(st, param);
	//выполнение команды:
	size_t result = 0;
	result = st.executeDML();
	if (result > 0 and use_comit)
	{
		TLog log("Запрос ", "executeDML");
		log << dml << "\nОбработал " << result << " строк!\n";
		log.toErrBuff();
		st.Commit();
	}
	return result;
}

size_t TDataBaseInterface::runDML4Directory(NS_Oracle::TDBConnect& db, const NS_Tune::TUserTune& param,
	bool use_comit) noexcept(false)
{
	using std::vector;
	//получение списка файлов:
	vector<string> dml_lst = param.getDMLFileLst();
	if (dml_lst.size() < 1)
		throw TLog("Пустой списк dml-команд в директории!", "TDataBaseSheetReport::runDML4Directory");
	//счетчик общего числа обработанных строк:
	size_t cnt = 0;
	//выполнение каждого запроса из списка:
	for (const string& dml : dml_lst)
	{
		size_t tmp = 0;
		//получение текста dml-команды
		string dml_txt = getSqlByTune(false, false, dml);
		//выполнение dml-команды
		tmp = executeDML(db, param, dml_txt, use_comit);
		if (tmp == 0)
			TLog("Запрос " + dml + " не вернул результата!", "TDataBaseSheetReport::runDML4Directory").toErrBuff();
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
			throw TLog("Ошибка формирования отчета - пустой файл настроек!", "TDataBaseSheetReport::crtReport");
		if (!sheet.isValid())
			throw TLog("Ошибка формирования отчета - не создана страница для отчета!", "TDataBaseSheetReport::crtReport");
		if (tune.isDQLEmpty())
			throw TLog("Не указано ни одного SQL-запроса!", "TDataBaseSheetReport::crtReport");
		//получение настроек для подключения к БД:
		TConnectParam cp = getConnectParam(tune, prefetch_rows);
		//подключаемся к БД:
		TDBConnect db(cp);
		bool dql_first = isDQLFirst();
		//если первым надо выполнить DML-команды
		if (!dql_first) runDML(db, tune);
		//проверяем считывается ли sql-команда из файла
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
		TLog("Отчет не сформирован! Не обработанная ошибка!", "TDataBaseSheetReport::crtReport").toErrBuff();
	}
	return false;
}

//выполнение DML запроса:
size_t TDataBaseInterface::runDML(NS_Oracle::TDBConnect& db, const NS_Tune::TUserTune& param, bool use_comit) noexcept(true)
{
	try
	{
		//проверка соединения и файла настроек
		if (!db.isValid()) throw TLog("Подключение к БД не валидно!", "TDataBaseSheetReport::runDML");
		if (param.Empty()) throw TLog("Пустой файл настроек!", "TDataBaseSheetReport::runDML");
		if (param.isDMLEmpty()) throw TLog("Нет DML-команд для выполнения!", "TDataBaseSheetReport::runDML");
		//получение dml-текста команды:
		string dml = param.getFieldValueByCode(TuneField::DMLText);
		//проверяем имеется ли текст команды в строке:
		size_t result = 0;
		if (!dml.empty())
			//выполнение dml-команды из строки:
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
		TLog("Не обработанная ошибка при выполнении DML-команды!", "TDataBaseSheetReport::runDML");
	}
	return 0;
}

void TDataBaseInterface::setCellByResultSet(NS_Excel::TExcelBook& book, NS_Excel::TExcelBookSheet& sheet, 
	const NS_Const::DataType& dt, const NS_Oracle::TBaseSet& bs, size_t resultSetCol, const NS_Excel::TExcelCell& cell) noexcept(false)
{
	using NS_Oracle::TType;
	using NS_Oracle::UInt;
	using NS_Oracle::SQLException;
	using NS_Oracle::TDate;
	using NS_Excel::TExcelDate;
	using NS_Const::DataType;
	if (!bs.isValid()) throw TLog("Не валдиный объект BaseSet!", "TDataBaseSheetReport::setCellByResultSet");
	//необходимо проверить на пустое значение данных в столбце:
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
			sheet.WriteAsNumber(cell, bs.getIntVal(resultSetCol));
			break;
		case DataType::Double:
			sheet.WriteAsNumber(cell, bs.getDoubleVal(resultSetCol));
			break;
		case DataType::String:
			sheet.WriteAsString(cell, bs.getStringVal(resultSetCol));
			break;
		case DataType::Boolean:
			sheet.WriteAsBool(cell, bs.getIntVal(resultSetCol));
			break;
		case DataType::Date:
			if (sheet.isDate(cell))
			{
				TDate date = bs.getDateVal(resultSetCol);
				TExcelDate tmp;
				date.getDate(tmp.year, tmp.month, tmp.day, tmp.hour, tmp.minute, tmp.sec);
				double dbl_date = book.Date2Double(tmp);
				sheet.WriteAsNumber(cell, dbl_date);
			}
			else
				sheet.WriteAsString(cell, bs.getDateAsStrVal(resultSetCol));
			break;
		default:
			TLog log("Указанный тип данных в ", "TDataBaseSheetReport::setCellByResultSet");
			log << resultSetCol << " колонке - НЕ обрпбатывается!" << TLog::NL;
			log.toErrBuff();
			break;
		}
	}
	catch (const SQLException& err)
	{
		TLog log("Ошибка считывания ", "TDataBaseSheetReport::setCellByResultSet");
		log << NS_Const::TConstType(dt).toStr() << " данных из колонки :";
		log << resultSetCol << ", строки: " << cell.getRow() << TLog::NL;
		log << err.what() << TLog::NL;
		log.toErrBuff();
	}
	catch (...)
	{
		TLog log("Не обработанная ошибка записи строки из колонки: ", "TDataBaseSheetReport::setCellByResultSet");
		log << resultSetCol << ", строки: " << cell.getRow() << TLog::NL;
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
		TLog log("Ошибка установки максимального числа итераций для запроса: " + query.getSQL() + "\n", 
			"TDataBaseInterface::setMaxIterationCnt");
		log << err.what() << '\n';
		log.toErrBuff();
	}
	catch (...)
	{
		TLog log("Не обработанная ошибка установки максимального числа итераций для запроса: " + query.getSQL() + "\n",
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
		TLog log("Ошибка добавления итерации для запроса:\n" + query.getSQL() + "\n",
			"TDataBaseInterface::addQueryIteration");
		log << err.what() << '\n';
		log.toErrBuff();
	}
	catch (...)
	{
		TLog log("Не обработанная ошибка добавления итерации для запроса: " + query.getSQL() + "\n",
			"TDataBaseInterface::addQueryIteration");
		log.toErrBuff();
	}
	return false;
}

void TReport::saveReport(TExcelBook& book, const string& file_name) const noexcept(false)
{
	//проверка существования директории:
	string path = config.getOutPath();
	if (TSharedTune::CheckPath(path, true))
		//сохранение книги:
		book.SaveToFile(file_name);
	else
		throw TLog("Указанная директория: " + path + " не найдена!\nОтчет не сохранен!",
			"TReport::One_Report_For_Each_Config");
}

void TReport::One_Report_For_Each_Config() const noexcept(false)
{
	using NS_Const::TuneField;
	using std::vector;
	using NS_Logger::TLog;
	using NS_Excel::TExcelBook;
	using StrArr = vector<string>;
	//получение списка конфигурационных файлов
	StrArr conf_lst = config.getConfFileLst(true);
	if (conf_lst.empty())
		throw TLog("Пустой список конфигурационных файлов!", "TReport::One_Report_For_Each_Config");
	//получение имени выходного файла:
	string tmp = config.getOutFile();
	//для каждого файла выполняем формирование отчета для листа
	for (const string& sub_tune : conf_lst)
	{
		//формирование файла настроек:
		TUserTune tune(config, sub_tune);
		if (!tune.Empty())
		{
			//создание excel-документа
			TExcelBook book(tmp);
			//формируем отчет для config-файла:
			TDataBaseSheetReport sh(book, tune);
			//если отчет не сформирован:
			if (!sh.crtSheet())
			{
				TLog log("Отчет для файла " + sub_tune + " не сформирован!", "TReport::One_Report_For_Each_Config");
				log.toErrBuff();
			}
			//иначе сохраняем файл:
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
	//получение списка конфигурационных файлов
	StrArr conf_lst = config.getConfFileLst(true);
	if (conf_lst.empty())
		throw TLog("Пустой список конфигурационных файлов!", "TReport::One_Sheet_By_One_Config");
	//получение пути выходного файла:
	string tmp = config.getOutFile();
	//создание excel-документа
	TExcelBook book(tmp);
	size_t fail_cnt = 0;
	//для каждого файла выполняем формирование отчета для листа
	for (const string& sub_tune : conf_lst)
	{
		//формирование настроек отчета:
		TUserTune tune(config, sub_tune);
		//формируем отчет для config-файла:
		TDataBaseSheetReport sh(book, tune);
		//если отчет не сформирован:
		if (!sh.crtSheet())
		{
			TLog log("Отчет для файла " + sub_tune + " не сформирован!", "TReport::One_Sheet_By_One_Config");
			log.toErrBuff();
			fail_cnt++;
		}
	}
	if (fail_cnt >= conf_lst.size())
		throw TLog("При формировании отчета возникли ошибки!", "TReport::One_Sheet_By_One_Config");
	else//сохранение файла, если есть что сохранять
		saveReport(book);
}

//здесь надо учесть, что для одного файла настроек может быть несколько запросов
//также необходимо передавать страницу в функцию по заполнению данных
void TReport::One_Sheet_By_Many_Statement() const noexcept(false)
{
	using NS_Const::TuneField;
	using std::vector;
	using NS_Logger::TLog;
	using NS_Excel::TExcelBook;
	using StrArr = vector<string>;
	//получение списка конфигурационных файлов
	StrArr conf_lst = config.getConfFileLst(true);
	if (conf_lst.empty())
		throw TLog("Пустой список конфигурационных файлов!", "TReport::One_Sheet_By_Many_Statement");
	//получение пути выходного файла:
	string tmp = config.getOutFile();
	//создание excel-документа
	TExcelBook book(tmp);
	//инициализируем страницу, которая будет создана:
	TExcelBookSheet sheet(nullptr);
	//число ошибок при формировании отчета:
	size_t fail_cnt = 0;
	for (const string& sub_tune: conf_lst)
	{
		//создание файла настроек:
		TUserTune tune(config, sub_tune);
		//формируем отчет для config-файла:
		TDataBaseSheetReport sh(book, sheet, tune);
		//если отчет не сформирован:
		if (!sh.crtSheet())
		{
			TLog log("Отчет для файла " + sub_tune + " не сформирован!", "TReport::One_Sheet_By_Many_Statement");
			log.toErrBuff();
			fail_cnt++;
		}
		if (!sheet.isValid())	sheet = book.getActiveSheet();
	}
	if (fail_cnt >= conf_lst.size())
		throw TLog("При формировании отчета возникли ошибки!", "TReport::One_Sheet_By_Many_Statement");
	else//сохранение файла, если есть что сохранять
		saveReport(book);
}

size_t TReport::runDML_By_Tune(bool use_comit) const noexcept(false)
{
	using std::vector;
	using NS_Logger::TLog;
	using StrArr = vector<string>;
	//получение списка конфигурационных файлов
	StrArr conf_lst = config.getConfFileLst(true);
	if (conf_lst.empty())
		throw TLog("Пустой список конфигурационных файлов!", "TReport::One_Sheet_By_Many_Statement");
	//число обработанных строк dml-командой
	size_t result = 0;
	for (const string& sub_tune : conf_lst)
	{
		//получение параметров из файла настроек
		TUserTune tune(config, sub_tune);
		//получение параметров подключения:
		TConnectParam cp = TDataBaseSheetReport::getConnectParam(tune);
		//для данных настроек формируем подлюечение к БД:
		TDBConnect db(cp);
		//выполняем dml-команду:
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
		//инициализируем настройки для json-файла:
		TExcelProcData proc_data(js_file, &config);
		//проверка наличия excel-файла приемника:
		if (proc_data.isDstFileEmpty()) throw TLog("Не указан excel-файл приемник!", "ProcessExcelFileByJson");
		size_t pageCnt = proc_data.getProcPagesCnt();
		if (pageCnt == 0) throw TLog("Не указаны параметры страниц для excel-файла приемника!", "ProcessExcelFileByJson");
		if (Rows.size() == 0) Rows.resize(pageCnt);
		//выполняем обработку для каждого листа в js-файле настроек
		for (size_t i = 0; i < pageCnt; i++)
		{
			//инициализация отчета для i-ой страницы:
			TJsonMarkReport report(book, proc_data, i, Rows[i]);
			//формирование отчета для i-ой страницы:
			report.crtSheetReport();
		}
		return true;
	}
	catch (TLog& err)
	{
		err << "Ошибка при обработке js-файла: " << js_file << '\n';
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка при работе с js-файлом: " + js_file, "ProcessExcelFileByJson").toErrBuff();
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
	//формируем список файлов настроек из config
	StrArr js_files = config.getConfFileLst();
	if (js_files.empty()) throw TLog("Пустая директория с js-файлами настроек!", "ProcessExcelFileByJson");
	//создаем excel-файл:
	string book_name = config.getOutFile();
	TExcelBook book(book_name);
	//формирование массива обрабатываемых строк
	//учитывается при использовании предварительной фильтрации данных для отбора в разных БД:
	vector<TRowsTNS> rows;
	//выполнение прохода по всфайламфайлам:
	for (const string& js : js_files)
	{
		if (js.empty()) continue;
		ProcessExcelFileByJson(book, js, rows);
	}
	//сохраняем наработки
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
		//формирование json-настроек:
		TExcelProcData js_tune(js_file, &config);
		//проверка наличия excel-файла приемника:
		if (js_tune.isDstFileEmpty()) throw TLog("Не указан excel-файл приемник!", "Json_Report_By_File_Compare");
		//получение числа страниц:
		size_t pageCnt = js_tune.getProcPagesCnt();
		if (pageCnt == 0) throw TLog("Не указаны параметры страниц для excel-файла приемника!", "Json_Report_By_File_Compare");
		//инициализируем excel-книгу отчета:
		string book_name = config.getOutFile();
		TExcelBook book(book_name);
		//счетчик ошибок
		size_t errCnt = 0;
		//выполняем обработку для каждой страницы файла приемника:
		for (size_t i = 0; i < pageCnt; i++)
		{
			//инициализация данных для отчета
			TJsonReport page(book, js_tune, i);
			if (page.NoSheet())
			{
				TLog log("Ошибка при загрузке страницы: ", "Json_Report_By_File_Compare");
				log << i << '\n';
				log.toErrBuff();
				errCnt++;
				continue;
			}
			//формирование отчета
			if (page.crtSheetReport() == false)
			{
				TLog log("Ошибка формирования отчета для страницы: ", "Json_Report_By_File_Compare");
				log << i << '\n';
				log.toErrBuff();
				errCnt++;
			}
		}
		//если число обработаннах страницы больше числа ошибок
		if (errCnt < pageCnt)
		{
			saveReport(book);
			return true;
		}
	}
	catch (TLog& err)
	{
		err << "Ошибка при обработке файла: " << js_file << ":\n";
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка при формировании отчета для файла: " + js_file,
			"Json_Report_By_File_Compare").toErrBuff();
	}
	return false;
}

void TReport::Json_Report_By_Files_Compare() const noexcept(true)
{
	using NS_Logger::TLog;
	using NS_Tune::StrArr;
	//формирование списка json-файлов:
	StrArr js_files = config.getConfFileLst();
	//проход по каждому json-файлу
	for (const string& file : js_files)
	{
		//формирование отчета для файла:
		if (Json_Report_By_File_Compare(file) == false)
			TLog("Файл: " + file + " не обработан!", "Json_Report_By_Files_Compare").toErrBuff();
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
		//инициализация json-файла
		ptree js;
		//считывание данных в json-файл
		read_json(js_file, js);
		//инициализируем настройки для обработки файла приемника(выходной фал)
		TShareData DestFile(js);
		//если имя не указано - берем его из выходного файла:
		if (DestFile.isEmptyName())	DestFile.setName(book.getFileName());
		//если файл инициализирован с ошибками - выходим
		if (DestFile.isEmpty()) return false;
		//получение числа страниц
		size_t pageCnt = DestFile.getPageCnt();
		//если страниц нет - следующий файл
		if (pageCnt < 1) return false;
		//получение дерева настроек для ячеек:
		
		//Нужно учесть, что метод может инициализироваться из поного json-файла
		//соответственно find надо делать из дочернего json
		ptree cell_node = js.get_child(TConstJson::asStr(JsonParams::Cells));
		
		//формируем данные для обработки ячеек:
		TProcCell cells(cell_node, &config);
		//обрабатываем каждую страницу файла-приемника:
		for (size_t i = 0; i < pageCnt; i++)
		{
			//инициализация отчета:
			TJsonReport report(book, DestFile, cells, i);
			//формирование отчета:
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
		TLog("Не обработанная ошибка пост обработки для файла настроек: " + js_file, "Json_SubTune_File_Run").toErrBuff();
	}
	return false;
}

void TReport::SubConfig_Stage() const noexcept(true)
{
	using NS_Tune::StrArr;
	using NS_Excel::TExcelBook;
	try
	{
		//получение имени файла для постобработки:
		//на данном этапе отчет уже должен быть сформирован!
		string name = config.getOutFile();
		//проверяем наличие файла для обработки на 2ой фазе:
		if (TSharedTune::CheckPath(name, false) == false)
			throw TLog("Указанный файл: " + name + " не найден! Пост обработка не выполнена!", "TReport::SubConfig_Stage");
		//инициализация excel-файла приемника:
		TExcelBook book(name);
		//получение списка файлов настроек:
		StrArr js_files = config.getSubConfigFileLst();
		//счетчик успешных обработок:
		size_t sucCnt = 0;
		//обрабатываем каждый файл с настройками пост обработки отдельно:
		for (const string& file : js_files)
		{
			if (Json_SubTune_File_Run(book, file) == true) sucCnt++;
		}
		//если была хоть одна успешная обработка - сохраняем изменения
		if (sucCnt > 0)
			saveReport(book, name);
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка при выполнении файзы пост обработки отчета!", "TReport::SubConfig_Stage").toErrBuff();
	}
}

void TReport::Create_Report_By_Code(const NS_Const::ReportCode& code) const
{
	using NS_Const::ReportCode;
	using NS_Const::TConstReportCode;
	using NS_Logger::TLog;
	//формирование лога ошибки о не реализованном отчете:
	auto raise_err = [](const ReportCode& rp)->TLog 
	{
		TLog log("Отчет ", "TReport::Create_Report_By_Code");
		log << TConstReportCode(rp).getName() << " не реализован!" << TLog::NL;
		return log;
	};
	switch (code)
	{
	//отчеты основанные на выполнении одиночного sql-запроса на лист:
	case ReportCode::BALANCE_LIST:
	case ReportCode::REPAYMENT_FOR_DATE:
	case ReportCode::DOCS_MF_SF_FOR_PERIOD:
		One_Sheet_By_One_Config();
		break;
	//case ReportCode::DOCS_MF_SF_FOR_PERIOD:
	//один файл отчета для одного config-файла
	case ReportCode::RIB_DOCS_FOR_PERIOD:
	case ReportCode::POTREB_CRED_BY_FILE:
	case ReportCode::CRED_CASE_MF:
		One_Report_For_Each_Config();
		break;
	//отчет основан на записи результатов различных запросов на одну страницу в один файл:
	case ReportCode::NBKI_NP:
	case ReportCode::NBKI_JP:
	case ReportCode::BALANCE_SUA:
		One_Sheet_By_Many_Statement();
		break;
	//выполнение хранимой процедуры:
	case ReportCode::NBKI_APPLY:
	case ReportCode::CLOSE_DAY:
		runDML_By_Tune(true);
		break;
	//полный кредитный портфель + манипуляция с excel-файлом
	case ReportCode::FULL_CRED_REPORT:
	{
		One_Sheet_By_Many_Statement();
		SubConfig_Stage();
		break;
	}
	//данные о кредитном портфеле для СУА
	case ReportCode::FULL_CRED_REPORT_SUA:
	case ReportCode::EXCEL_SET_DATA_FROM_BASE:
		Json_One_Row_One_DBQuery();
		break;
	//загрузка данных в oracle из excel
	case ReportCode::LOAD_FROM_FILE:
		throw raise_err(code);
		break;
	//сравнение файлов excel
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
		//получение идентификатора кода отчета:
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
		TLog("Не обработанная ошибка при формировании отчета!", "TReport::Execute").toErrBuff();
	}
	return false;
}

TReport::TReport(const string& config_file): config(config_file)
{
	try
	{
		if (!config.Empty())
			Execute();
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка при формаировании отчета " + config.getMainCode() +
			" для файла настроек: " + config_file, "TReport::TReport");
	}
}