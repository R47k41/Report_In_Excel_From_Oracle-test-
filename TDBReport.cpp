//модуль определения функционала для TDBReport
#include <iostream>
#include <fstream>
#include <math.h>
#include "TDBReport.h"
#include "TSQLParser.h"
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
using NS_ExcelReport::TCellFormatIndex;

void raise_app_err(const TLog& log, bool as_raise = true);

void raise_app_err(const TLog& log, bool as_raise)
{
	as_raise ? throw log : log.toErrBuff();
}

//функция получения числа для 10 в степени x:
long get10(int val = 0) noexcept(true);

//округление числа до указанной точности:
double Round(double x, int sz = 0) noexcept(true);

long get10(int val) noexcept(true)
{
	long x10 = 1;
	while (val-- > 0) x10 *= 10;
	return x10;
}

double Round(double x, int sz) noexcept(true)
{
	using std::floor;
	using std::ceil;
	try
	{

		if (x == 0) return x;
		long d = get10(sz);
		if (x > 0)
			return floor((x * d) + 0.5) / d;
		else
			return ceil((x * d) - 0.5) / d;
	}
	catch (...)
	{
		TLog log("Не обработаная ошибка: ", "Round");
		log << "Round(" << x << ", " << sz << ")!";
		log.toErrBuff();
	}
	return x;
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
	//инициализация ячейки:
	TExcelCell cell(Row, Col, false);
	//если ячейка создана не корректоно - выход
	if (!cell.isValid())
		return false;
	try
	{
		//получение формата из текущей ячейки:
		TExcelBookFormat format = sheet.getCellFormat(cell);
		//если формат валиден - добавляем
		if (format.isValid())
		{
			//получение индекса формата текущей ячейки в excel-файле:
			size_t curCellFormatIndex = book.getFormatIndex(format);
			//инициализация формата:
			TCellFormatIndex cell_format_indx(curCellFormatIndex);
			if (cell_format_indx.InitFlg == false)
			{
				TLog log("Не удалось инициализировать формат для ячейки: ", "TBaseSheetReport::addCurCellFormat");
				log << cell.getName() << '\n';
				throw log;
			}
			//дабавление его в массив
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
		TLog log("Не обработанная ошибка при добавлении формата для ячейки: ", "TBaseSheetReport::add2CellFormatArr");
		log << cell.getName() << '\n';
		if (!book.getError().empty()) log << book.getError() << '\n';
	}
	return false;
}

bool TBaseSheetReport::initRowFormat() noexcept(true)
{
	//если не валидна книга или лист:
	if (!book.isValid() or !sheet.isValid()
		or book.isEmpty()) return false;
	//получение ссылки на первую строку с данными:
	//если книга является шаблоном - то формат ячеек заполнен в первой пустой строке
	size_t curRow = LastRow();
	//проходим по каждому столбцу строки
	//т.к. ячейка берется в формтае excel - от 0
	//увеличиваем значение на 1, чтобы создавать ячейки правильно
	size_t curCol = sheet.getFirstCol() + 1;
	size_t last_col = sheet.getLastCol();
	//для каждой ячейки формируем ее текущии формат:
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
	//получение формата из книги:
	TExcelBookFormat format = book.getFormatByIndex(FormatIndex);
	//установка формата:
	return setCellFormat(cell, format);
}

bool TBaseSheetReport::setCellFormatByIndexArr(size_t Row, size_t ColIndex) noexcept(true)
{
	using NS_Excel::TExcelCell;
	TExcelCell cell(Row, ColIndex, false);
	return setCellFormatByIndexArr(cell);
}

bool TBaseSheetReport::addFillFormat(size_t init_format_index, const NS_Excel::TColor& color, 
	bool font_flg, size_t& AddedFormatIndex) noexcept(true)
{
	using NS_Excel::TExcelBookFormat;
	using NS_Excel::TExcelBookFont;
	using NS_Excel::TColor;
	try
	{
		//если цвет не задан
		if (color == TColor::COLOR_NONE) return false;
		//инициализация формата по индексу:
		TExcelBookFormat tmpFormat = book.getFormatByIndex(init_format_index);
		//если формат не валиден - выходим
		if (tmpFormat.isValid())
		{
			//создаем новый формат на основании формата текущей ячейки:
			TExcelBookFormat result = book.AddFormat(tmpFormat, false);
			//обработка шрифтов
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
			//получение индекса нового формата для возврата
			AddedFormatIndex = book.FormatCount() - 1;
			return true;
		}
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("Не обработанная ошибка при установке цвета для ячейки!", "TBaseSheetReport::addFillFormat");
		string err_exl = book.getError();
		if (!err_exl.empty()) log << '\n' << err_exl;
		log.toErrBuff();
	}
	return false;
}


bool TBaseSheetReport::setRowCellsFormat(size_t Row) noexcept(true)
{
	using NS_Excel::TExcelCell;
	if (cells_format_indexs.empty()) return true;
	size_t cnt = 0;
	//проходим по каждой ячейки для которой заполнен формат
	for (const TCellFormat& i: cells_format_indexs)
	{
		//т.к. в массиве храниться информация о колонках в формате excel - от 0
		//надо перевести данный формат в соответствие:
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
	return  first ? sheet.getFirstRow() : sheet.getLastRow();
/*
	if (book.isTemplate() == true)
		return  first ? sheet.getFirstRow() + 1 : sheet.getLastRow() - 1;
	else
		return  first ? sheet.getFirstRow() : sheet.getLastRow();
/**/
}

bool TBaseSheetReport::isDataFormatRow(size_t curRow) const noexcept(false)
{
	using NS_Excel::TExcelCell;
	//получение первой ячейки с данными:
	size_t col = sheet.getFirstCol();
	//инициализация ячейки
	TExcelCell cell(curRow, col, false);
	return sheet.isEmptyCell(cell);
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

bool TBaseSheetReport::setCellFormat(const NS_Excel::TExcelCell& cell, NS_Excel::TExcelBookFormat& format) noexcept(true)
{
	//если указан недопустимый цвет
	if (!format.isValid()) return false;
	//если указана пустая ячейка - надо ли ставить ей формат???
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

bool TBaseSheetReport::setCellFormat(size_t Row, size_t Column, NS_Excel::TExcelBookFormat& format) noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_Excel::TExcelBookFormat;
	//преобразуем индексы строки и столбца к excel-виду
	TExcelCell cell(Row, Column, false);
	return setCellFormat(cell, format);
}

bool NS_ExcelReport::TBaseSheetReport::setCellColorByFormatIndxArr(const NS_Excel::TExcelCell& cell, bool foundFlg) noexcept(true)
{
	//получение ссылки на форматы для указанной ячейки
	const TCellFormatIndex& format = getFormatIndexByColl(cell.getCol());
	if (format.InitFlg == false) return false;
	size_t formatIndex = foundFlg ? format.Found : format.NotFound;
	TExcelBookFormat cell_format = book.getFormatByIndex(formatIndex);
	return setCellFormat(cell, cell_format);
}

bool NS_ExcelReport::TBaseSheetReport::InitSheetByTemplate(const string& tmpl_name, const string& sh_name,
	bool set_as_active, size_t tmpl_sh_index) noexcept(true)
{
	try
	{
		//инициализация страницы книги по шаблону
		if (book.setSheetByTemplate(tmpl_name, sh_name, tmpl_sh_index, set_as_active))
		{
			//инициализация страницы книги последней активной страницей:
			sheet = book.getActiveSheet();
			//выставляем даннй книги признак шаблона:
			book.setAsTemplate(true);
			//инициализация форматов ячеек шаблона:
			initRowFormat();
			return sheet.isValid();
		}
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("Не обработанная ошибка инициализации шаблона:", "InitSheetByTemplate");
		log << tmpl_name;
		log.toErrBuff();
	}
	return false;
}


NS_Excel::FormatPtr TBaseSheetReport::getCellFormatPtr(const NS_Excel::TExcelCell& cell) noexcept(true)
{
	using NS_Excel::FormatPtr;
	try
	{
		//получение индекса формата ячейки из массива форматов:
		TCellFormatIndex format = getFormatIndexByColl(cell.getCol());
		//функция получения ссылки на формат в книге по индексу
		FormatPtr pformat = book.getFormatPrtByIndex(format.Current);
		return pformat;
	}
	catch (const std::exception& err)
	{
		TLog log("Ошибка при получении ссылки на формат для ячейки: ", "TBaseSheetReport::getCellFormatPtr");
		log << cell.getName() << '\n' << err.what() << '\n';
		log.toErrBuff();
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка получения формата ячейки: " + cell.getName(), 
			"TBaseSheetReport::getCellFormatPtr").toErrBuff();
	}
	return nullptr;
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

bool TBaseSheetReport::InsNewRow(size_t curRow, size_t newRow) noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_Excel::TExcelRange;
	//if (curRow == newRow) return true;
	try
	{
		//формируем диапазон строк для вставки:
		TExcelCell cellB(curRow, 0);
		TExcelCell cellE(newRow, 0);
		TExcelRange range(cellB, cellE);
		return sheet.insRows(range, false);
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
	//берем строки в формате книги - от 1
	size_t row = getRow(true);
	return row == NS_Tune::TIndex::EmptyIndex ? sheet.getFirstRow() + 1 : row;
}

size_t TExtendSheetReport::LastRow() const noexcept(true)
{
	//берем строки в формате книги - от 1
	size_t row = getRow(false);
	return row == NS_Tune::TIndex::EmptyIndex ? sheet.getLastRow() + 1 : row;
}

bool TExtendSheetReport::isCorrectFilter(size_t curRow) const noexcept(true)
{
	using NS_Tune::TFilterData;
	using NS_Excel::TExcelCell;
	//если фильтра нет - условия истины
	if (filters.empty())
	{
		if (noColID()) return true;
		return noDataInColID(curRow) == false;
	}
	for (const TFilterData& fltr : filters)
	{
		//объединение условий фильтрации по AND - если хоть один не подошел - выход
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
	//заносим все строки
	TLog log("Выполняется проверка условий отбора для ", "TExtendSheetReport::setFiltredRowsArr");
	log << size << " строк!\n";
	log.toErrBuff();
	for (; i <= size; i++) 
	{
		if (all_flg or isCorrectFilter(i))
			rows.insert(TRowFlag(i, true));
	}
	log.clear(true);
	log << "Для обработки выбрано: " << rows.size() << " строк!\n";
	log.toErrBuff();
	return rows;
}

bool NS_ExcelReport::TExtendSheetReport::copyRowsArr(NS_ExcelReport::TRowsFlag& dst, NS_ExcelReport::TRowsFlag& src,
	size_t key_to, bool only_true) noexcept(true)
{
	using NS_ExcelReport::TRowFlag;
	size_t sz = dst.size();
	//если данных в источнике нет - выход
	if (src.empty()) return false;
	//проверяем наличие заданного ключа в массиве-источнике:
	const TRowsFlag::const_iterator& indx_end = src.find(key_to);
	//если данные не найдены - выход
	if (indx_end == src.end()) return false;
	//проходим по всем значениям массива источника, до указанного элемента:
	for (TRowsFlag::const_iterator i = src.begin(); i != indx_end; i++)
	{
		if (only_true and i->second == false) continue;
		//добавление элемента в приемник
		dst.insert(*i);
		//меняем в источнике флаг параметра:
		src[i->first] = false;
	}
	//если размер массива приемника изменился - true
	return sz != dst.size();
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
		//проверка ячеек на пустоту:
		//if (sheet.isEmptyCell(srcCell) or dstSheet.isEmptyCell(dstCell)) return false;
		//проверяем совпадение типов:
		if (EqualCellsType(dstSheet, dstCell, srcCell) == false)
		{
			//сравниваем ячейки как строковые значения:
			string srcVal = sheet.ReadAsString(srcCell, book);
			string dstVal = dstSheet.ReadAsString(dstCell, book);
			return  srcVal == dstVal;
		}
		//получение типа данных для ячейки с которой сравниваем:
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
				//считывание данных в источнике:
				double srcVal = sheet.ReadAsNumber(srcCell);
				//проверяем являются ли данные в ячейке - датой!!!!:
				if (sheet.isDate(srcCell))
				{
					//если в приемнике не дата:
					if (dstSheet.isDate(dstCell) == false)
					{
						//форматы и преобразования внутри ячейки не помогли
						//надо переводить дату в строку и строку в дату
						//сравниваем как строки:
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
				//формируем код операции
				//возвращаем результат операции
				return TConstJSFilterOper::StringBaseOperation(dstVal, srcVal, operation);
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

bool TExtendSheetReport::checkByFilter(const NS_Tune::TFilterData& filter, size_t Row) const noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_Excel::TDataType;
	using NS_Const::TConstJSFilterOper;
	//если фильтр пустой - выход
	if (filter.isEmpty())
		return true;
	//формирование ячейки для фильрации:
	TExcelCell cell(Row, filter.getColIndx(), false);
	try
	{
		//если в ячейке нет данных - выход
		if (sheet.isEmptyCell(cell))
			//Отладка:
			//throw TLog("Пустая ячейка: " + cell.getName(), "TExtendSheetReport::checkCellByFilter");
			return false;
		//получаем тип данных в ячейке:
		TDataType dt = sheet.getCellType(cell);
		//считываем данные из ячейки:
		switch (dt)
		{
			case TDataType::CELLTYPE_BOOLEAN:
			{
				bool srcVal = sheet.ReadAsBool(cell);
				return filter.isFiltredBoolValue(srcVal);
			}
			case TDataType::CELLTYPE_NUMBER:
			{
				//считывание данных в источнике:
				double srcVal = sheet.ReadAsNumber(cell);
				//проверяем являются ли данные в ячейке - датой!!!:
				if (sheet.isDate(cell))
				{
					throw TLog("Не реализовано сравнение между датами!", "TExtendSheetReport::checkCellByFilter");
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
				TLog log("Тип данных с ID: ", "TExtendSheetReport::checkCellByFilter");
				log << dt << " в ячейке : " << cell.getName() << " не обрабатывается!\n";
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
		TLog log("Не обработанная ошибка при фильтрации данных в ячейке: ", "TExtendSheetReport::checkCellByFilter");
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
	//если в ячейке нет данных - выходим
	if (sheet.isEmptyCell(srcCell)) return false;
	//проверяем совпадение:
	JsonFilterOper operation = JsonFilterOper::Equal;
	if (NoSpaceNoCase) operation = JsonFilterOper::StrEqualNoCase;
	return Compare_Cells(dstSheet, dstCell, srcCell, operation);
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

size_t TExtendSheetReport::CheckOnSheet(const NS_Excel::TExcelBookSheet& dstSheet, 
	const NS_Excel::TExcelCell& dstCell, size_t srcCol, bool NoSpaceNoCase) const noexcept(true)
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
	for (const TRowFlag& curRow: RowsArr)
	{
		//если фильтрация для строки прошла успешно:
		if (curRow.second == true)
		{
			//ищем данные в строке:
			if (CheckInCell(dstSheet, dstCell, curRow.first, srcCol, NoSpaceNoCase))
			{
				//убираем строку из массива просматриваемых строк
				//RowsArr.erase(curRow.first);//RowsArr[curRow.first] = false;
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
	initRowFormat();
}

TJsonReport::TJsonReport(NS_Excel::TExcelBook& book_ref, const TExcelProcData& json_tune, size_t page) :
	TExtendSheetReport(book_ref, json_tune.getDstFile(), page), cells_data(json_tune.getCellsData()),
	meth_code(NS_Const::JSonMeth::Null)
{
	meth_code = cells_data.getMethodCode();
	//формирование форматов закраски:
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
		throw TLog("Ошибка преобразования данных в тип данных Oracle!", "TJsonReport::setDMLOutParam");
	query.registerOutParam(param.SrcParam(), out_type);
}

bool TJsonReport::isParamColumn(size_t Col) const noexcept(true)
{
	using NS_Tune::TCellData;
	using NS_Tune::CellDataArr;
	//сверка происходит в формате файла: отсчет от 1
	if (Col == getColID()) return true;
	//получение ссылки на массив параметров:
	CellDataArr arr = cells_data.getCellDataArr();
	for (const NS_Tune::TCellData& param : arr)
		//если колонка относится к вставляемому или извлекаемому индексу
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
	using NS_Excel::FormatPtr;
	//если индекс для вставки пустой - продолжаем:
	//if (param.EmptyInsIndx()) return;
	//формируем колонку:
	TExcelCell cell(Row, param.InsIndex(), false);
	//получение формата для ячейки:
	FormatPtr format = getCellFormatPtr(cell);
	//вставляем данные в ячейку
	TDataBaseInterface::setCellByResultSet(book, sheet, type_code, bs, OutParamIndex, cell, format);
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

void TJsonReport::insertToDataBase(NS_Oracle::TStatement& query, size_t curRow) noexcept(false)
{
	//выполнение DML-команды и подсчет обработанных строк:
	size_t cnt = query.getProcessedCntRows();
	if (cnt > 0)
	{
		TLog log("Строка ", "TJsonReport::insertToDataBase");
		log << curRow  << " была успешно импортирована!\n";
		log.toErrBuff();
		//query.Commit();
	}
	else
	{
		TLog log("Ошибка импорта ", "TJsonReport::insertToDataBase");
		log << curRow  << " строки!\n";
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

void TJsonReport::ProcessByStatement(NS_Oracle::TStatement& query, size_t curRow) noexcept(false)
{
	using NS_Const::JSonMeth;
	//выполняем обработку в зависимости от метода обработки:
	switch (meth_code)
	{
		//добавление данных в базу
		case JSonMeth::SendToDB:
		{
			//на данный момент запрос уже выполнен!
			insertToDataBase(query, curRow);
			break;
		}
		//по умолчанию считаем, что из базы получаем выходные паарметры
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
			ColoringRowCell(curRow, getColID(), find_flg, true);
			return find_flg;
		}
	}
	else//если условие фильтрации не выполнено:
		return true;
	return false;
}

void TJsonReport::setExcelDataByDB(NS_Oracle::TStatement& query, size_t& rowFrom) noexcept(false)
{
	using NS_Tune::TIndex;
	using NS_Const::JSonMeth;
	if (rowFrom <= 0)
	{
		TLog log("Не верно указана начальная строка обработки: ", "TJsonReport::setExcelDataByDB");
		log << rowFrom << '\n';
		throw log;
	}
	//получение номера последней обрабатываемой строки
	size_t rowTo = LastRow();
	size_t errCnt = 0;
	//считывание строк excel-файла
	for (rowFrom; rowFrom <= rowTo; rowFrom++)
	{
		bool rslt = setExcelRowDataByBD(query, rowFrom);
		if (rslt == false) errCnt++;
	}
	//если есть ошибки при импорте данных:
	if (meth_code == JSonMeth::SendToDB)
		if (errCnt > 0)
		{
			TLog log("При обработке запроса: ", "setExcelDataByDB");
			log << query.getSQL() << " выявлено: " << errCnt << " ошибок! Обработка отменена!\n";
			throw log;
		}
		else
		{
			query.Commit();
			TLog("Все строки успешно импортированы!\n", "setExcelDataByDB").toErrBuff();
		}
}

void TJsonReport::SetDBStatementData(NS_Oracle::TDBConnect& db, const NS_Tune::TUserTune& tune, 
	const string& sql, size_t& rowFrom) noexcept(false)
{
	using NS_Oracle::TStatement;
	//формирование запроса к БД:
	TStatement query(db, sql, 1);
	//установка постоянных параметров:
	TDataBaseInterface::setSqlParamsByTune(query, tune);
	//выполнение запроса:
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
	//если строка от которой идет считывание не установлена - берем ее из настроек
	if (rowFrom == TIndex::EmptyIndex)
		rowFrom = FirstRow();
	//получение текста запросов:
	StrArr sqlLst = tune.getDMLList();
	//обработка запросов
	runDBStatementLst(db, tune, sqlLst, rowFrom);
}

bool TJsonReport::ClearImportDBTbl(TDBConnect& db, const TUserTune& tune) const noexcept(true)
{
	using NS_Oracle::TStatement;
	using NS_Tune::TUserTune;
	using NS_Tune::StrArr;
	try
	{
		//проверяем наличие скрипта для очистки таблицы в БД:
		StrArr clearLst = tune.getClearList();
		for (const string& sql : clearLst)
		{
			//формирование запроса к БД:
			TStatement query(db, sql, 1);
			//установка постоянных параметров:
			//TDataBaseInterface::setSqlParamsByTune(query, tune);
			//выполнение запроса:
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
		TLog("Не обработанная ошибка очистки таблиц импорта!", "ClearImportDBTbl").toErrBuff();
	}
	return false;
}

void TJsonReport::ProcessSheetDataWithDB() noexcept(false)
{
	if (!book.isValid()) 
		throw TLog("Книга не инициализирована!", "TJsonReport::SheetDataFromDataBase");
	//получение первой строки для обработки
	size_t row = FirstRow();
	//для каждой настройки подключения выполняем:
	for (const TUserTune& config : cells_data.getDBTuneArr())
	{
		//получение параметров подключения к БД
		NS_Oracle::TConnectParam cp = TDataBaseInterface::getConnectParam(config);
		//создаем подключение к БД:
		TDBConnect db(cp);
		if (!db.isValid()) 
			throw TLog("Ошибка подключения к БД: " + cp.tns_name, "TJsonReport::SheetDataFromDataBase");
		try
		{
			//если указано - очищаем таблицы импорта:
			ClearImportDBTbl(db, config);
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

bool TJsonReport::addCellFillFormat(size_t Row, size_t Col, bool font_flg) noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_Excel::TExcelBookFormat;
	using NS_Excel::TColor;
	using NS_Tune::TCellMethod;
	using std::exception;
	//инициализация ячейки:
	TExcelCell cell(Row, Col, false);
	try
	{
		//получение формата данный ячейки из массива форматов:
		TCellFormatIndex& tmpIndex = TBaseSheetReport::getFormatIndexByColl(cell.getCol());
		if (tmpIndex.InitFlg == false)
			throw TLog("Форматы для ячейки: " + cell.getName() + " не заданы!\n", "TJsonReport::addCellFillFormat");
		//формирование новых форматов для текущей ячейки:
		//определяем цвет относительно флага:
		TColor color = TColor::COLOR_NONE;
		//получение цвета заливки из настроек:
		const TCellMethod& meth = cells_data.getMethod();
		//установка цвета для найденных данных:
		color = meth.getIncludeColor();
		addFillFormat(tmpIndex.Current, color, font_flg, tmpIndex.Found);
		//установка цвета для ненайденных данных:
		color = meth.getExcludeColor();
		addFillFormat(tmpIndex.Current, color, font_flg, tmpIndex.NotFound);
		return true;
	}
	catch (const exception& err)
	{
		TLog log("Ошибка при добавлении формата для ячейки: ", "TJsonReport::addCellFillFormat");
		log << cell.getName() << '\n' << err.what() << '\n';
		log.toErrBuff();
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка при установке формата для ячейки: " + cell.getName(), 
			"TJsonReport::addCellFillFormat").toErrBuff();
	}
	return false;
}

bool TJsonReport::addCurCellFormat(size_t Row, size_t Col) noexcept(true)
{
	using NS_Const::JsonCellFill;
	//проверяем относится ли данная ячейка к параметрам или ID-ячейки:
	//проверяем наличие обрабатываемых ячеек:
	if (isParamColumn(Col) == false) return false;
	//устанавливаем форматирование для текущей ячейки:
	bool setCurCellFormatFlg = TBaseSheetReport::addCurCellFormat(Row, Col);
	//получение ссылки на метод обработки:
	const TCellMethod& meth = cells_data.getMethod();
	//если не указан метод заливки или цвета пустые
	if (meth.isEmptyColor())
		//выставляем формат только текущей ячейки
		return setCurCellFormatFlg;
	bool use_font = false;
	//если ячейка не является ID - проверям возможность выставления цвета шрифта
	if (getColID() != Col) use_font = meth.useFont();
	return addCellFillFormat(Row, Col, use_font);
}

bool TJsonReport::initRowFormat() noexcept(true)
{
	//если параметров нет - выходим
	if (cells_data.CellCnt() <= 0) return false;
	//установка форматов только для параметризованных ячеек
	return TBaseSheetReport::initRowFormat();
}

bool TJsonReport::useColoring(bool FndFlg, bool ChngFlg) const noexcept(true)
{
	//если используется метод отслеживающий изменения и данные найдены
	if (WithChangeMeth() and FndFlg)
		//за окраску отвечает призк наличия изменений:
		return ChngFlg;
	return true;
}

bool TJsonReport::ColoringRowCell(const NS_Excel::TExcelCell& cell, bool find_flg, bool procFlg) noexcept(true)
{
	//проверка наличия массива форматов excel-книги
	if (EmptyCellsIndexFormat()) return false;
	//если используется метод отслеживания изменений и изменений не было - выход
	if (useColoring(find_flg, procFlg) == false) return false;
	try
	{
		if (setCellColorByFormatIndxArr(cell, find_flg) == false)
			throw TLog("Форматы для ячейки: " + cell.getName() + " не заданы!", "TJsonReport::addCellFillFormat");
		return true;
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("Не обработанная ошибка при установке формата ячейке: ", "TJsonReport::ColoringRowCell");
		log << cell.getName() << '\n';
		log.toErrBuff();
	}
	return false;
}

bool TJsonReport::ColoringRowCell(size_t Row, size_t Col, bool find_flg, bool procFlg) noexcept(true)
{
	using NS_Excel::TExcelCell;
	//инициализация ячейки:
	TExcelCell cell(Row, Col, false);
	return ColoringRowCell(cell, find_flg, procFlg);
}

bool TJsonReport::ColoringRowByFlg(size_t curRow, bool FndFlg, bool ChngFlg) noexcept(true)
{
	//в данной версии окраска происходит только по ColID
	//если нет ячейки идентификатора - выход
	if (noColID()) return false;
	return ColoringRowCell(curRow, getColID(), FndFlg, ChngFlg);
}

bool TJsonReport::ColoringRowByCnt(size_t curRow, size_t FindCnt, size_t FailCnt) noexcept(true)
{
	//функция отвечает за окраску строки или ее идентификационной ячейки
	//используется для общего случая окраси, когда были проверены все параметры
	//проверяем можно считать, что строка была найдена
	//в зависимости от метода поиска:
	bool flg = cells_data.getMethod().isSuccess(FindCnt, FailCnt);
	//закраска строки
	return ColoringRowByFlg(curRow, flg, true);
}


bool TJsonReport::ColoringCellByParam(const NS_Tune::TCellData& param, size_t curRow, size_t frmt_index, bool fing_flg) noexcept(true)
{
	using NS_Const::JSonMeth;
	using NS_Const::TConstJSMeth;
	using NS_Tune::TIndex;
	using NS_Excel::TExcelCell;
	size_t column = 0;
	//берем данные о колонке в формате книги - от 1
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
	return ColoringRowCell(curRow, column, fing_flg, true);
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
	size_t dstRow, size_t srcRow) noexcept(true)//!!!!, TRowsFlag& RowsArr) noexcept(true)
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
		TExcelCell tmpCell(srcRow, param.SrcVal(), false);
		if (srcSheet.isEmptyCell(tmpCell) == false)
			outArr.push_back(&param);
	}
	//если выходных параметров нет -выход:
	if (outArr.empty()) return true;
	//добавляем новую строку:
	if (InsNewRow(dstRow, dstRow) == true)
	{
		dstRow = dstRow + 1;
		//добавление в массив строк - новой строки, чтобы не сбивать порядок
		//!!!!RowsArr.insert(TRowFlag(dstRow, false));
	}
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
//закрашиваем ячейку идентификатор в новой строке
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

size_t TExtendSheetReport::getSrcRow_By_Cell(const TExtendSheetReport& srcSheet, NS_ExcelReport::TRowsFlag& srcRows,
	const NS_Excel::TExcelCell& DstCell, size_t SrcCol, bool NoSpaceNoCase) const noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_Tune::TIndex;
	//проверяем есть ли данные в указанной ячейке:
	if (sheet.isEmptyCell(DstCell)) return TIndex::EmptyIndex;
	//выполняем поиск ячейки на листе источника:
	return srcSheet.CheckOnSheet(sheet, DstCell, SrcCol, srcRows, NoSpaceNoCase);
}

size_t TExtendSheetReport::getSrcRow_By_Params(const TExtendSheetReport& srcSheet, NS_ExcelReport::TRowsFlag& srcRows,
	const NS_Tune::CellDataArr& params, size_t curRow, size_t& param_index, bool NoSpaceNoCase) const noexcept(true)
{
	using NS_Tune::TCellData;
	using NS_Excel::TExcelCell;
	using NS_Tune::TIndex;
	//проходим по параметрам и выполняем поиск строки(используем для отсечения пустых параметров):
	for (; param_index < params.size(); param_index++)
	{
		//если нет данных по колонкам из строки приемника/источника - переходим к следующей строке
		if (params[param_index].EmptyDstIndx() and params[param_index].EmptySrcParam()) continue;
		//инициализация ячейки приемника:
		TExcelCell DstCell(curRow, params[param_index].DstIndex(), false);
		//выполняем поиск строки в источнике для ячейки приемника:
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
	//индекс первого параметра для поиска
	size_t param_index = 0;
	//выполняем поиск строки источника по первой ячейки приемника
	size_t srcRow = getSrcRow_By_Params(srcSheet, srcRows, params, curRow, param_index, NoSpaceNoCase);
	//выполняем циклический поиск строки на странице
	while (srcRow != TIndex::EmptyIndex)
	{
		size_t cnt = 0, suc = 0;
		//если строка найдена - продолжаем сверку для ячеек строки приемника с ячейками строки источника
		for (++param_index; param_index < params.size(); param_index++)
		{
			cnt++;
			//инициализируем ячейку приемника
			TExcelCell DstCell(curRow, params[param_index].DstIndex(), false);
			//инициализация ячейки источника
			TExcelCell SrcCell(srcRow, params[param_index].SrcParam(), false);
			//если не найдена хоть одна ячейка строки - найденная строка не подходит
			if (getDstCell_In_SrcCell(srcSheet, DstCell, SrcCell, NoSpaceNoCase) == false)
				break;
			else
				suc++;
		}
		//если все ячейки найдены - выходим
		if (suc > 0 and cnt == suc)
			break;
		else
			//если строка не найдена - ищем  ее дальше:
			srcRow = getSrcRow_By_Params(srcSheet, srcRows, params, curRow, param_index, NoSpaceNoCase);
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
	//счетчик не найденных ячеек:
	int failCnt = 0, col_cnt = 0, ChngCnt = 0;
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
			ColoringRowCell(dstCell, false, true);
		}
		else
		{
			//удаление найденной строки из массива строк источника:
			srcRows[srcRow] = false;//srcRows.erase(srcRow);
			//обрабатываем найденные данные в  ячейках:
			bool procFlg = procFindCell(srcSheet, param, curRow, srcRow);
			//если обрабатываются данные которые изменялись
			//увеличиваем счетчик измененных ячеек
			if (procFlg) ChngCnt++;
			//окраска ячейки в зависимости от вставляемых данных:
			ColoringRowCell(dstCell, true, procFlg);
		}
	}
	//результат поиска в зависимости от метода обработки:
	bool FindFlg = cells_data.getMethod().isSuccess(col_cnt, failCnt);;
	//закраска строки:
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
	//проходим по каждой строке листа-приемника:
	for (const TRowFlag& index : DstRows)
	{
		//не обрабатываем ячейки, которые были обработаны ранее
		if (index.second == false) continue;
		//текущая строка приемника в обработке
		TLog log("Идет обработка: ");
		log << index.first << " сткроки из " << lastRow << " строк";
		log.toErrBuff();
		if (Proc_DstRowCells_In_SrcSheet(srcSheet, SrcRows, params, index.first, NoSpaceNoCase) == true)
			DstRows[index.first] = false;//это делается на случай, если несколько листов у файла источника
	}
	TLog("Обработка завершена!").toErrBuff();
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
			//пропуск пустых параметров
			if (param.isEmpty()) continue;
			//инициализация параметра
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
		TLog("Не обработанная ошибка при инверсии параметров!", "Inverse_Dst2Src_Param").toErrBuff();
	}
	return false;
}

bool TJsonReport::InsertNewRows4Books(const TExtendSheetReport& srcSheet, const NS_Tune::CellDataArr& params,
	const Rows4Insert& newRowsArr) noexcept(true)
{
	using NS_Excel::TExcelBook;
	using NS_Excel::TExcelBookSheet;
	size_t sz = newRowsArr.size();
	if (sz == 0) return false;
	TLog("Производим добавление новых строк!", "InsertNewRows4Books").toErrBuff();
	size_t cnt = 0;
	for (const Row4Insert& val : newRowsArr)
	{
		size_t dstRow = val.first + cnt;
		if (procFindRow(srcSheet, params, dstRow, val.second) == false)
		{
			TLog log("При обработке ", "Error");
			log << val.first << " строки приемника для " << val.second << " строки источника произошла ошибка!";
			log.toErrBuff();
			continue;
		}
		std::cout << "Добавлена: " << cnt++ << " строка из " << sz << std::endl;
	}
	return true;
}

bool TJsonReport::Execute_Seek_Dst_Row_In_Src_Sht(const TExtendSheetReport& srcSheet, TRowsFlag& DstRows,
	TRowsFlag& SrcRows, const CellDataArr& params, bool NoSpaceNoCase) noexcept(false)
{
	using NS_Const::JSonMeth;
	using NS_ExcelReport::TRowsFlag;
	using NS_ExcelReport::TRowFlag;
	using NS_ExcelReport::Rows4Insert;
	using NS_Tune::TIndex;
	using NS_Tune::CellDataArr;
	//переменная определяющая инверсию Источника и Приемника
	//если число строк в источнике меньше, чем в приемнике - меняем их местами
	bool ReverseFlg = SrcRows.size() < DstRows.size();
	//далее работаем со ссылками на источник и приемник
	const TExtendSheetReport* DstPtr = ReverseFlg ? &srcSheet : this;
	const TExtendSheetReport* SrcPtr = ReverseFlg ? this : &srcSheet;
	//определяем массивы строк для источника и приемника:
	TRowsFlag* dstRowsPrt = ReverseFlg ? &SrcRows : &DstRows;
	TRowsFlag* srcRowsPtr = ReverseFlg ? &DstRows : &SrcRows;
	//выполняем инверсию параметров поиска данных для источника и приемника:
	CellDataArr tmp_params;
	if (ReverseFlg)
		Inverse_Dst2Src_Param(params, tmp_params);
	else
		tmp_params = params;
	//выполнение поиска данных
	//счетчик новых строк в приемнике:
	//!!!!size_t count = 0;
	size_t line = 0;
	//определение числа строк, требующих обрботки(строки удовлетворяющие фильтру):
	size_t lastRow = dstRowsPrt->rbegin()->first;
	//инициализируем массив со строками для обработки
	//в нем будем хранить строки, которые не были обработаны
	TRowsFlag tmpRows;
	//массив новых строк:
	Rows4Insert newRowsArr;
	//выполняем проход по каждой строке меньшего массива:
	for (const TRowFlag& index : *dstRowsPrt)
	{
		//пропуск ранее обработанных строк
		if (index.second == false) continue;
		line++;
		//текущая строка с учетом добавленных строк в приемник:
		size_t curRow = index.first;
		//если инверсии не было - обрабатываем файл в который вставляем данные
		//соответственно увеличиваем счетчик на число вставленных строк
		//!!!!if (!ReverseFlg) curRow += count;
		TLog log("Идет обработка: ");
		log << line << " строки (" << curRow << '\\' << lastRow << ")";
		log.toErrBuff();
		//выполнение поиска строки приемника в источнике:
		size_t srcRow = DstPtr->getSrcRow_By_Dest_Params(*SrcPtr, *srcRowsPtr, tmp_params, curRow, NoSpaceNoCase);
		//если строка не найдена, но есть данные для сравнения в побочном массиве:
		if (srcRow == TIndex::EmptyIndex and tmpRows.empty() == false)
		{
			//поис строки в побочном массиве:
			srcRow = DstPtr->getSrcRow_By_Dest_Params(*SrcPtr, tmpRows, tmp_params, curRow, NoSpaceNoCase);
			if (srcRow == TIndex::EmptyIndex)
			{
				//если во временном массиве строк есть данные
				TLog log("Строка ");
				log << curRow << " не найдена в файле источника!";
				log.toErrBuff();
				continue;
			}
		}
		//если строка найдена ее необходимо обработать:
		//копируем во временный массив значения строк приемника, которые не обработались:
		copyRowsArr(tmpRows, *srcRowsPtr, srcRow);
		//в зависимости от флага инверсии приемника и источника:
		//инициализируем найденную и текущую строку для приемника и источника
		size_t DstCurRow = ReverseFlg ? srcRow : curRow;
		size_t SrcCurRow = ReverseFlg ? curRow : srcRow;
		//убираем строку источника из дальнейшего поиска:
		SrcRows[SrcCurRow] = false;
		//убираем строку приемника из обработки:
		DstRows[DstCurRow] = false;
		//добавление в массив новых строк:
		newRowsArr.insert(Row4Insert(DstCurRow, SrcCurRow));
		//выполняем обработку строки приемника - здесь уже идет вставка строки:
		/*//!!!!
		//!!!!if (procFindRow(srcSheet, params, DstCurRow, SrcCurRow, DstRows))
			//увеличиваем счетчик обработанных строк
			//!!!!count++;
		else
		{
			TLog log("При обработке ");
			log << DstCurRow << " строки приемника для " << SrcCurRow << " строки источника произошла ошибка!";
			log.toErrBuff();
		}
		/**/
	}
	//если массив с новыми строками для вставки заполнен:
	return InsertNewRows4Books(srcSheet, params, newRowsArr);

	//return true;
}

bool TJsonReport::Search_DestData_In_SrcSheet(TRowsFlag& DstRows, 
	const TExtendSheetReport& srcSheet, bool NoSpaceNoCase) noexcept(true)
{
	using NS_ExcelReport::TRowsFlag;
	using NS_Const::JSonMeth;
	using NS_Const::TConstJSMeth;
	try
	{
		//получаем список сравниваемых аттрибутов:
		const CellDataArr& cellArr = cells_data.getCellDataArr();
		if (cellArr.empty())
			throw TLog("Пустые индексы колонок для сравнения!", "TJsonReport::Search_DestData_In_SrcSheet");
		//формируем массив строк источника для обработки - нумерация от 1:
		TRowsFlag srcRows = srcSheet.setFiltredRowsArr();
		if (srcRows.empty())
			throw TLog("На странице файла источника нет данных подходящих под условия!",
				"TJsonReport::Search_DestData_In_SrcSheet");
		//в зависимости от метода вызываем нужный метод поиска:
		switch (meth_code)
		{
		case JSonMeth::CompareCell:
		case JSonMeth::CompareCellChange:
		{
			//вызов метода обработки ячейки приемника на странице файла источника
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
			TLog log("Указанный метод(", "Search_DestRow_In_SrcSheet");
			log << TConstJSMeth::asStr(meth_code) << ") не обрабатывается!";
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
	//нумерация строк идет в формате книги - от 1
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
	procRows.second.insert(row_flg);
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
		book.setAsTemplate(false);
		return sheet.isValid();
	}
	else
		//инициализация страницы - страницей шаблона
		return InitSheetByTemplate(tmp_val, sh_name, active_sheet);
	return false;
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

void TDataBaseInterface::setSqlParamByTune(NS_Oracle::TStatement& sql, const NS_Tune::TSubParam& param, bool use_raise) noexcept(false)
{
	using NS_Tune::DataType;
	using NS_Converter::toType;
	using NS_Converter::toDblType;
	using NS_Converter::toBoolType;
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
	using NS_Excel::FormatPtr;
	//из-за несовпадения индексации колонок в oracle и excel
	size_t resultSetCol = cell.getCol() + 1;
	//получение кода типа данных для записи в ячейку:
	DataType code = convertOraType(rs.getColumnType(resultSetCol));
	//получение ссылки на формат ячейки:
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
		//если число колонок превышает ограничения excel - выходим
		CreateNewPage(col_cnt, false);
		//берем последнюю строку с данными
		size_t row = LastRow();
		//если предыдущая строка - строка формата ячеек - пишем в нее
		if (isDataFormatRow(row-1) == true) row--;
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
			//установка форматов для ячеек новой строки:
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
	//если настройки пустые - выходим
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
	//формирование данных для страницы
	FillSheetBySql(db, sql, prefetch);
}

void TDataBaseSheetReport::CrtBySqlFiles(NS_Oracle::TDBConnect& db, int prefetch) noexcept(false)
{
	using std::vector;
	using NS_Tune::TUserTune;
	vector<string> sql_lst = tune.getDQLFileLst();
	if (sql_lst.size() < 1)
		throw TLog("Пустой списк sql-запросов в директории!", "TDataBaseSheetReport::CrtBySqlFiles");
	bool use_parse = useSqlParse();
	//выполнение каждого запроса из списка:
	for (const string& sql_file : sql_lst)
	{
		string sql = TUserTune::getSqlString(use_parse, false, sql_file);
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
	using NS_Tune::TUserTune;
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
		string dml_txt = TUserTune::getSqlString(false, false, dml);
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
	if (!bs.isValid()) throw TLog("Не валидный объект BaseSet!", "TDataBaseSheetReport::setCellByResultSet");
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

bool NS_ExcelReport::TSmlvchReport::InitSheet(const string& sh_name, bool set_as_active) noexcept(true)
{
	using NS_Tune::StrArr;
	//получение имени шаблона из настроек:
	StrArr files = config.getTemplFileLst();
	//не обрабатываем более одного шаблона:
	if (files.size() != 1)
	{
		TLog("Не реализована обработка более одного шаблона на страницу отчета!", "TSmlvchReport::InitSheet").toErrBuff();
		return false;
	}
	string name = files[0];
	//формируем новую страницу книги из шаблона для файла:
	return InitSheetByTemplate(name, sh_name, set_as_active);
}

bool NS_ExcelReport::TSmlvchBalance::setTotalFields(size_t curRow, bool active_flg, double sld_rub, 
	double sld_val, const NS_Tune::CellDataArr& params) noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_SMLVCH_IMP::TAccount;
	//установка форматов ячеек строки
	setRowCellsFormat(curRow);
	//формирование текста:
	string str = "Итого по ";
	str += active_flg ? "активным " : " пассивным ";
	str += "счетам:";
	//запись заголовка
	sheet.WriteAsString(TExcelCell(curRow, sheet.getFirstCol(), false), str);
	//поиск колонок для вставки данных в параметрах:
	for (const NS_Tune::TCellData& param : params)
	{
		//если не указано поле вставки - пропускаем
		if (param.EmptyInsIndx() || param.SrcParam(false) != TAccount::SldRubIndex()) continue;
		//инициализация ячейки
		TExcelCell cell(curRow, param.InsIndex(), false);
		//если удалось записать рублевый итог - записываем валютный
		if (sheet.WriteAsNumber(cell, sld_rub))
			return sheet.WriteAsNumber(cell.getRow(), cell.getCol() - 1, sld_val);
		else
			return false;
	}
	return true;
}

bool NS_ExcelReport::TSmlvchBalance::setAccount2Row(size_t Row, const NS_SMLVCH_IMP::TAccount& acc, 
	const NS_Tune::CellDataArr& params, const NS_Excel::TColor& color, double rate) noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_SMLVCH_IMP::TAccount;
	using std::stringstream;
	if (params.empty()) return false;
	try
	{
		//установка форматов ячеек строки:
		setRowCellsFormat(Row);
		//проходим по параметрам списка:
		for (const NS_Tune::TCellData& param : params)
		{
			//если не указан параметр колонки в которую записывается инфа - переходим к следующему
			if (param.EmptyInsIndx()) continue;
			//инициализация ячейки под запись:
			size_t Col = param.InsIndex();
			TExcelCell cell(Row, Col, false);
			//т.к. параметры берутся от 1 - вычиаем 1 из значения параметра
			size_t index = param.SrcParam(false);
			switch (index)
			{
			case TAccount::AccountIndex():
			{
				//если это валютный счет - закрасим его
				if (rate != 1) setCellColorByFormatIndxArr(cell, true);
				sheet.WriteAsString(cell, acc.getAccount());
				break;
			}
			case TAccount::NametIndex():
				sheet.WriteAsString(cell, acc.getName());
				break;
			case TAccount::SldRubIndex():
			{
				//запись остатка в рублях:
				if (sheet.WriteAsNumber(cell, acc.getSldRub()))
				{
					double sld_val = acc.getSldRub();
					//если курс не 0 и не 1 - вычисляем остаток
					if (rate > 0 and rate != 1)
					{
						sld_val /= rate;
						//округляем
						sld_val = Round(sld_val, 2);
					}
					//запись остатка в валюте счета - на ячейку меньше:
					sheet.WriteAsNumber(cell.getRow(), cell.getCol() - 1, sld_val);
				}
				break;
			}
			case TAccount::DateIndex():
				sheet.WriteAsString(cell, acc.getLastOperDate());
				break;
			default:
			{
				TLog log("Указанный SrcParamIdex: ", "setAccount2Row");
				log << index << " не обрабатывается!";
				throw log;
			}
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
		TLog log("Не обработанная ошибка при записи счета: ", "setAccount2Row");
		log << acc.getAccount();
		log.toErrBuff();
	}
	return false;
}

bool NS_ExcelReport::TSmlvchBalance::setAccounts2Sheet(size_t& curRow, const NS_SMLVCH_IMP::TAccounts& arr, 
	const NS_Tune::CellDataArr& params, const NS_Tune::TCurrencyBlock& rates, SubHeaderRows& headers, 
	const string& row_name_grp, bool last_row_as_sum) noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_SMLVCH_IMP::TAccount;
	//если счета не заполнены - выход:
	if (arr.empty()) throw TLog("Массив счетов - пуст!", "setAccounts2Sheet");
	try
	{
		//если указана строка заголовок - вставояем ее в 1ю ячейку:
		if (row_name_grp.empty() == false)
		{
			//получение первого свободного столбца листа
			size_t curCol = sheet.getFirstCol();
			if (sheet.WriteAsString(TExcelCell(curRow, curCol, false), row_name_grp))
				headers.push_back(curRow++);
		}
		//инициализация переменных для вывода итогов:
		double sum_rur = 0, sum_val = 0;
		//проходим по каждому счету и заносим его в строку excel-страницы
		for (const TAccount& acc : arr)
		{
			//пустой счет пропускаем
			if (acc.isEmpty()) continue;
			//получение курса для кода валюты:
			double rate = rates.getCurRateByCode(acc.getCurrencyCode());
			//запись данных в строку excel:
			bool flg = setAccount2Row(curRow, acc, params, rates.getColor(), rate);
			//увеличиваем данные по суммарным остаткам:
			if (last_row_as_sum and flg)
			{
				sum_rur += acc.getSldRub();
				sum_val += rate == 0 ? acc.getSldRub() : acc.getSldRub() / rate;
			}
			curRow++;
		}
		//запись итоговой ячейки:
		if (setTotalFields(curRow, arr[0].isActive(), sum_rur, sum_val, params))
			headers.push_back(curRow);
		return true;
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка при обрпботке массива счетов!", "setAccounts2Sheet").toErrBuff();
	}
	return false;
}

bool NS_ExcelReport::TSmlvchBalance::UpdFormatArrByColor(const NS_Excel::TColor& color, bool fnd_flg, 
	bool font_flg) noexcept(true)
{
	//проходим по всем форматам листа:
	for (size_t i = 0; i < cells_format_indexs.size(); i++)
	{
		//size_t init_indx = cells_format_indexs[i].Current;
		size_t& format_indx = fnd_flg ? cells_format_indexs[i].Found : cells_format_indexs[i].NotFound;
		//добавляем новый формат на лист:
		addFillFormat(cells_format_indexs[i].Current, color, font_flg, format_indx);
	}
	return true;
}

bool NS_ExcelReport::TSmlvchBalance::setSubHeadersFont(const SubHeaderRows& rows) noexcept(true)
{
	using NS_Excel::TExcelCell;
	using NS_Excel::TExcelBookFormat;
	using NS_Excel::TExcelBookFont;
	if (rows.empty()) return false;
	try
	{
		size_t i = 0;
		size_t Col = sheet.getFirstCol() + 1;
		TExcelCell cell(rows[i++], Col, false);
		//получаем формат ячейки:
		TExcelBookFormat cell_frmt = sheet.getCellFormat(cell);
		TExcelBookFormat frmt = book.AddFormat(cell_frmt, false);
		TExcelBookFont fnt = frmt.getFont();
		fnt.setBold();
		//установка формата для всех ячеек
		sheet.setCellFormat(cell, frmt);
		for (; i < rows.size(); i++)
		{
			TExcelCell tmp(rows[i], Col, false);
			sheet.setCellFormat(tmp, frmt);
		}
		return true;
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка при установке подзаголовка для массива ячеек!", "setSubHeadersFont").toErrBuff();
	}
	return false;
}

bool NS_ExcelReport::TSmlvchBalance::crtSheet(const string& imp_file, const NS_Tune::TBalanceTune& tune) noexcept(true)
{
	using NS_SMLVCH_IMP::TImportBalance;
	try
	{
		//инициализация объекта Баланс дляуказанного файла:
		TImportBalance balance(imp_file, tune);
		//если данные по счетам не загрузились - выход
		if (balance.isEmpty())
			throw TLog("Пустые данные по счетам для файла: " + imp_file, "TSmlvchBalance::crtSheet");
		//инициализация страницы для записи данных файла:
		InitSheet(balance.getName());
		//обновление форматов ячеек:
		UpdFormatArrByColor(tune.getColor(), true, false);
		//запись данных массива счетов на страницу:
		const NS_Tune::CellDataArr& params = tune.getParams();
		const NS_Tune::TCurrencyBlock& rates = tune.getCurrencyBlock();
		//инициализируем массив заголовков:
		SubHeaderRows sub_head;
		//получение строки для записи:
		size_t curRow = getRow(false);
		//проверяем не является ли предыдущая строка строкой форматов
		if (isDataFormatRow(curRow - 1) == true) curRow--;
		//запись активных счетов:
		setAccounts2Sheet(curRow, balance.getAccounts(true), params, rates, sub_head, "Актив");
		curRow++;
		//запись пассивных счетов:
		setAccounts2Sheet(curRow, balance.getAccounts(false), params, rates, sub_head, "Пассив");
		setSubHeadersFont(sub_head);
		return true;
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка формирования страницы отчета для файла: " + imp_file, "TSmlvchBalance::crtSheet").toErrBuff();
	}
	return false;
}

NS_Tune::TBalanceTune NS_ExcelReport::TSmlvchBalance::getBalanceTune() const noexcept(true)
{
	using NS_Tune::StrArr;
	using NS_Tune::TBalanceTune;
	try
	{
		//инициализация настроек отчета по балансу:
		StrArr files = config.getConfFileLst();
		if (files.size() != 1)
			throw TLog("Не реализована обработка нескольких файлов-настроек для данного отчета!", "TSmlvchBalance::getBalanceTune");
		TBalanceTune tune(files[0]);
		if (tune.isEmpty())
		{
			TLog log("Не удалось инициализировать настройки отчета по файлу: ", "TSmlvchBalance::getBalanceTune");
			log << files[0];
			throw log;
		}
		return tune;
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка при получении настроек отчета!", "TSmlvchBalance::getBalanceTune").toErrBuff();
	}
	return NS_Tune::TBalanceTune();
}

bool NS_ExcelReport::TSmlvchBalance::crtReport() noexcept(true)
{
	using NS_Tune::StrArr;
	using NS_Tune::TBalanceTune;
	try
	{
		//получение настроек для формирования отчета по Балансу:
		TBalanceTune tune = getBalanceTune();
		//получение списка файлов для импорта:
		StrArr files = tune.getImportFiles(config.getMainPathVal());
		//выполняем обработку каждого файла:
		for (const string& file : files)
		{
			//формирование страницы отчета под каждый файл импорта
			if (crtSheet(file, tune) == false)
				//если страница не загрузилась:
				throw TLog("Ошибка импорта файла: " + file, "crtReport");
		}
		return true;
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка при формировании отчета!", "TSmlvchBalance::crtReport").toErrBuff();
	}
	return false;
}

NS_ExcelReport::TSmlvchImp::TSmlvchImp(NS_Excel::TExcelBook& book_lnk, const NS_Tune::TSharedTune& tune_lnk, const string& json_file) :
	TSmlvchReport(book_lnk, tune_lnk), imp_data(), imp_tune(json_file, tune_lnk.getMainPathVal())
{
	//если настройки пустые
	if (imp_tune.isEmpty())
	{
		TLog("Не удалось инициализировать настройки импорта для файла: " + json_file, "TSmlvchImp").toErrBuff();
		return;
	}
	//получение имени шаблона:
	string name = config.getMainPathVal() + imp_tune.getTemlateName();
	//инициалиазция структуры импорта документов:
	imp_data.setDefaultAttrByFile(name, imp_tune.getTemplateFields());
}

bool NS_ExcelReport::TSmlvchImp::readRowData(size_t curRow) noexcept(true)
{
	using NS_SMLVCH_IMP::TRSBankDoc;
	using NS_Excel::TExcelCell;
	using NS_Tune::CellDataArr;
	using NS_Tune::TCellData;
	try
	{
		//получение ссылки на параметры ячеек строки:
		const CellDataArr& params = imp_tune.getParams();
		//инициализация структуры документа импорта:
		TRSBankDoc doc;
		//проходим по параметрам считывания:
		for (const TCellData& param : params)
		{
			//параметры, где не указан индекс данных источника - пропускаем:
			if (param.EmptySrcParam() || param.EmptyDstIndx()) continue;
			//инициализация ячейки
			TExcelCell cell(curRow, param.SrcParam(), false);
			//считывание данных из ячейки:
			string data = sheet.ReadAsString(cell, book);
			//если данные пустые - выход
			if (data.empty())
			{
				TLog log("Не указаны данные для обязательного поля: ", "readRowData");
				log << cell.getName();
				throw log;
			}
			doc.setField(data, param.DstIndex());
		}
		//добавление документа в массив:
		return imp_data.AddDoc(doc);
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog log("Не обработанная ошибка при считывании параметров из строки: ", "TSmlvchImp::readRowData");
		log << curRow;
		log.toErrBuff();
	}
	return false;
}

bool NS_ExcelReport::TSmlvchImp::readSheet(size_t first_row, size_t last_row) noexcept(true)
{
	try
	{
		size_t lst_row = sheet.getLastRow()+1;
		//если первая строка для считывания превышает индекс последней строки с данными на листе:
		if (first_row > lst_row)
			throw TLog("Первая строка для считывания не может превышать последнюю строку с данными!", "TSmlvchImp::readSheet");
		//определение последней строки:
		if (lst_row < last_row) lst_row = last_row;
		size_t errCnt = 0;
		//проходим по всем строкам страницы:
		for (size_t curRow = first_row; curRow < lst_row; curRow++)
		{
			if (readRowData(curRow) == false)
			{
				TLog log("Ошибка при формировании документа для строки: ", "TSmlvchImp::readSheet");
				log << curRow;
				log.toErrBuff();
				errCnt++;
			}
		}
		if (errCnt >= lst_row - first_row)
			throw TLog("Документы не импортированы!", "TSmlvchImp::readSheet");
		if (errCnt > 0)
		{
			TLog log("При импорте документов произшло: ", "TSmlvchImp::readSheet");
			log << errCnt << " ошибок!";
			throw log;
		}
		return true;
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка при чтении данных страницы!", "TSmlvchImp::readSheet").toErrBuff();
	}
	return false;
}

bool NS_ExcelReport::TSmlvchImp::readFile(const string& file) noexcept(true)
{
	using NS_Tune::TSheetData;
	using NS_Tune::SheetArr;
	if (file.empty()) return false;
	try
	{
		//если имеются настроки фильтрации данных:
		if (imp_tune.getFiltersTune().size() > 0)
			throw TLog("Функционал обработки фильтрации считываемых строк НЕ реализован!", "TSmlvchImp::readFile");
		//инициализация настроек страниц книги:
		const SheetArr& tune_shts = imp_tune.getSheetTune();
		size_t errCnt = 0;
		//проходим по всем страницам из настроек
		for (const TSheetData& sh : tune_shts)
		{
			//получение строки с которой идет считывание:
			size_t curtRow = sh.getStartRow();
			//получение строки по которую идет считывание:
			size_t lstRow = sh.getLastRow();
			//открываем книгу на заданной странице:
			if (book.loadSheetOnly(file, sh.getListIndex()-1))
			{
				//инициализация страницы
				sheet = book.getActiveSheet();
				//считывание данных из книги по заданным параметрам::
				if (readSheet(curtRow, lstRow) == true) continue;
			}
			//если страница не загрузилась или произошли ошибки при считывании страницы:
			errCnt++;
		}
		return errCnt == 0 ? true : false;
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (...)
	{
		TLog("Не обработанная ошибка загрузки файла: " + file, "readFile").toErrBuff();
	}
	return false;
}

bool NS_ExcelReport::TSmlvchImp::setDocsByFiles() noexcept(true)
{
	using NS_Tune::StrArr;
	using NS_Tune::TSimpleTune;
	try
	{
		//считываем расширение для загружаемых файлов:
		string load_ext = config.getFieldValueByCode(NS_Const::TuneField::TemplateFileExt);
		//считывание пути загрузки:
		string load_path = imp_tune.getSrc();
		//получение файлов для считывания:
		StrArr arr = TSimpleTune::getFileLst(load_path, load_ext);
		//если нет файлов для загрузки:
		if (arr.empty()) return false;
		//проходим по каждому файлу и считываем из него параметры документов:
		for (const string& file : arr)
		{
			TLog("Процесс считвания файла: " + file).toErrBuff();
			//загрузка excel-файла для считывания:
			if (readFile(file))
			{
				TLog log("Считывание файла: " + file + " прошло успешно!\nВыполняю удаление файла: " + file, "TSmlvchImp::setDocsByFiles");
				if (std::remove(file.c_str()) == 0)
					log << "\nФайл: " << file << " успешно удален!";
				else
					log << "\nОшибка удаления файла: " << file;
				log.toErrBuff();
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
		TLog("Не обработанная ошибка при загрузке данных!", "setDocsByFiles").toErrBuff();
	}
	return false;
}

bool NS_ExcelReport::TSmlvchImp::crtOutFile() const noexcept(true)
{
	using std::ifstream;
	string file = config.getOutFile();
	//проверка существования файла:
	std::ios_base::openmode mode = ifstream(file) ? std::ios_base::ate : std::ios_base::out;
	return imp_data.CreateFile4Import(file, mode);
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
	if (js_files.empty()) throw TLog("Пустая директория с js-файлами настроек!", "Json_One_Row_One_DBQuery");
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
		
		//Нужно учесть, что метод может инициализироваться из полного json-файла
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

void TReport::SubConfig_Json_UpdOutExlFile() const noexcept(true)
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

void TReport::SubConfig_IniFile_Execute() const noexcept(true)
{
	using NS_Tune::StrArr;
	try
	{
		//получение списка ini-файлов:
		StrArr config_lst;
	}
	catch (const TLog)
	{
	}
	catch (...)
	{
		TLog("Не обработанная ошибка при выполнении ini-файла!", "TReport::SubConfig_IniFile_Execute").toErrBuff();
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
		//инициализация настроек js-файла:
		TExcelProcData json(js_file, &config);
		//проверка валидности натроек
		if (json.isDstFileEmpty())
			throw TLog("Пустой исходный excel-файл для считывания!", "loadFromJson");
		if (json.isCellsEmpty())
			throw TLog("Пустые данные для обработки!", "loadFromJson");
		//получение директории загружаемых excel-файлов:
		string directory = json.getDstFile().getName();
		StrArr filesLst = TSimpleTune::getFileLst(directory);
		for (const string& name : filesLst)
		{
			TLog log("Производим загрузку данных из файла: " + name, "loadFromJson");
			//инициализация excel-файла:
			TExcelBook book(name);
			//установка имени файла т.к. там указана лишь директория:
			json.setDstFileName(name);
			//получение числа обрабатываемых страниц:
			size_t pageCnt = json.getProcPagesCnt();
			size_t errCnt = 0;//счетчик ошибок
			//обработка каждой страницы excel-файла:
			for (size_t i = 0; i < pageCnt; i++)
			{
				log << " для " << i << " страницы!\n";
				log.toErrBuff();
				//инициализация обработчика excel-файла
				TJsonReport report(book, json, i);
				//формирование отчета:
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
		TLog log("Не обработанная ошибка при загрузке данных из файла: " + js_file, "loadFromJson");
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
	//выполнение парсинга json-файлов:
	//формируем список json-файлов настроек из config
	StrArr js_files = config.getConfFileLst();
	if (js_files.empty())
		throw TLog("Пустая директория с js-файлами настроек!", "load2DBFromExcel");
	//выполнение прохода по каждому файлу:
	for (const string& js : js_files)
	{
		//если файл пустой - пропускаем:
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
	case ReportCode::LOTS:
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
		//формирование данных из БД
		One_Sheet_By_Many_Statement();
		//сравнение excel-файлов
		SubConfig_Json_UpdOutExlFile();
		break;
	}
	//данные о кредитном портфеле для СУА(вставка данных из базы в excel)
	//не используются
/*Не реализовано
	case ReportCode::FULL_CRED_REPORT_SUA:
	case ReportCode::EXCEL_SET_DATA_FROM_BASE:
		throw raise_err(code);
		//Json_One_Row_One_DBQuery();
		break;
		//загрузка данных в oracle из excel
	case ReportCode::LOAD_FROM_FILE:
	case ReportCode::EXCEL_PAY_LOAD_MF:
	case ReportCode::EXCEL_PAY_LOAD_SF:
	case ReportCode::EXCEL_DOC_LOAD:
		throw raise_err(code);
		//load2DBFromExcel();
		break;
/**/
	//сравнение файлов excel
	case ReportCode::FILE_COMPARE_RIB:
	case ReportCode::FILE_COMPARE_RTBK:
		Json_Report_By_Files_Compare();
		break;
	case ReportCode::SMLVCH_BALANCE:
		Smolevich_Balance_Report();
		break;
	case ReportCode::SMLVCH_IMP:
		Smolevich_Docs_Import();
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
		if (!config.Empty()) Execute();
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

void TReport::Smolevich_Balance_Report() const noexcept(true)
{
	using NS_Tune::StrArr;
	using NS_Excel::TExcelBook;
	//получение имени выходного файла:
	string name = config.getOutFile();
	//инициализация шаблона excel-книги
	TExcelBook book(name);
	//инициализация отчета по Балансу:
	TSmlvchBalance balance(book, config);
	balance.crtReport();
	saveReport(book);
}

bool NS_ExcelReport::TReport::Smolevich_Docs_Import() const noexcept(true)
{
	using NS_Tune::StrArr;
	using NS_Excel::TExcelBook;
	using NS_SMLVCH_IMP::TRSBankDocs;
	using NS_Tune::TImpDocsTune;
	//получение пути к файлу с настроками:
	StrArr tune_files = config.getConfFileLst();
	//инициализация путой книги:
	string exl_ext = config.getFieldValueByCode(NS_Const::TuneField::TemplateFileExt);
	TExcelBook book(exl_ext);
	//выполнение прохода по каждому json-файлу настроек импорта
	for (const string& file : tune_files)
	{
		//инициализация объекта для иморта:
		TSmlvchImp imp(book, config, file);
		//формирование документов на импорт
		if (imp.setDocsByFiles() == false)
		{
			TLog("Документы для импорта не найдены!", "Smolevich_Docs_Import").toErrBuff();
			return false;
		}
		//формирование файла импорта:
		imp.crtOutFile();
	}
	return true;
}
