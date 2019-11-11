#include <iostream>
#include <sstream>
#include <math.h>
#include "TExcel.h"
#include "Logger.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using NS_Logger::TLog;


string NS_Excel::TExcelParam::getColumnName(int indx) const noexcept(true)
{
	if (indx >= 0 and indx <= ColumnsCnt())
	{
		return header[indx];
	}
	else
		return string();
}

void NS_Excel::TExcelBookFont::initFont(BookPtr book) noexcept(false)
{
	if (book)
	{
		pfont = book->addFont();
		if (!pfont) throw string(book->errorMessage());
	}
}

bool NS_Excel::TExcelBookFont::isFontType(const TFontType& ft) const
{
	if (!isValid()) return false;
	switch (ft)
	{
	case TFontType::Bold: return pfont->bold();
	case TFontType::Italic: return pfont->italic();
	case TFontType::StrikeOut: return pfont->strikeOut();
	case TFontType::UnderLine: return pfont->underline();
	case TFontType::Script: return pfont->script();
	}
	return false;
}

void NS_Excel::TExcelBookFont::setScript(const TScriptFontType& sft)
{
	if (isValid()) pfont->setScript(sft);
}

NS_Excel::TScriptFontType NS_Excel::TExcelBookFont::isScript() const
{
	return pfont->script();
}

void NS_Excel::TExcelBookFont::setUnderLine(const TUnderLineFontType& uft)
{
	pfont->setUnderline(uft);
}

NS_Excel::TUnderLineFontType NS_Excel::TExcelBookFont::isUnderLine() const
{
	return pfont->underline();
}

bool NS_Excel::TExcelBookFont::setColor(const TColor& c)
{
	if (isValid())
	{
		pfont->setColor(c);
		return true;
	}
	return false;
}

NS_Excel::TColor NS_Excel::TExcelBookFont::getColor() const
{
	return pfont->color();
}

bool NS_Excel::TExcelBookFont::setName(const string& s)
{
	if (isValid()) pfont->setName(s.c_str());
	return false;
}

bool NS_Excel::TExcelBookFont::setFontType(const TFontType& ft)
{
	if (!isValid()) return false;
	switch (ft)
	{
	case TFontType::Bold:
		pfont->setBold();
		return true;
	case TFontType::Italic:
		pfont->setItalic();
		return true;
	case TFontType::Script:
		pfont->setStrikeOut();
		return true;
	}
	return false;
}

void NS_Excel::TExcelBookFormat::initFormat(BookPtr book) noexcept(false)
{
	if (book)
	{
		pformat = book->addFormat();
		if (!pformat) TLog(book->errorMessage()).raise(true, "TExcelBookFormat::initFormat");
	}
}

NS_Excel::TExcelBookFont NS_Excel::TExcelBookFormat::getFont() const noexcept(false)
{
	if (isValid())
	{
		FontPtr pfont = pformat->font();
		return TExcelBookFont(pfont);
	}
	TLog("Формат не валиден! Получить шрифт невозможно!").raise(true, "TExcelBookFormat::getFont");
}

bool NS_Excel::TExcelBookFormat::setFont(TExcelBookFont& fnt) noexcept(false)
{
	if (isValid() and fnt.isValid())
		return pformat->setFont(fnt.pfont);
	return false;
}

int NS_Excel::TExcelBookFormat::getNumFormat() const
{
	if (isValid()) return pformat->numFormat();
	return EmptyType;
}

void NS_Excel::TExcelBookFormat::setNumFormat(TExcelBookFormat::TNumFormat val)
{
	if (isValid()) pformat->setNumFormat(val);
}

void NS_Excel::TExcelBookFormat::setBorderStyle(TBorderStyle style, const TBorderSide& side)
{
	if (!isValid()) return;
	switch (side)
	{
	case TBorderSide::Full:
		pformat->setBorder(style);
		break;
	case TBorderSide::Left:
		pformat->setBorderLeft(style);
		break;
	case TBorderSide::Right:
		pformat->setBorderRight(style);
		break;
	case TBorderSide::Top:
		pformat->setBorderTop(style);
		break;
	case TBorderSide::Bottom:
		pformat->setBorderBottom(style);
		break;
	case TBorderSide::Diagonal:
		pformat->setBorderDiagonalStyle(style);
		break;
	default: return;
	}
}

NS_Excel::TExcelBookFormat::TBorderStyle NS_Excel::TExcelBookFormat::getBorderStyle(const TBorderSide& side) const noexcept(true)
{
	if (!isValid()) return TBorderStyle::BORDERSTYLE_NONE;
	switch (side)
	{
	case TBorderSide::Left:
		return pformat->borderLeft();
	case TBorderSide::Right:
		return pformat->borderRight();
	case TBorderSide::Top:
		return pformat->borderTop();
	case TBorderSide::Bottom:
		return pformat->borderBottom();
	case TBorderSide::Diagonal:
		return pformat->borderDiagonalStyle();
	}
	return TBorderStyle::BORDERSTYLE_NONE;
}

void NS_Excel::TExcelBookFormat::setBorderColor(const TColor& c, const TBorderSide& side) noexcept(false)
{
	if (!isValid()) return;
	switch (side)
	{
	case TBorderSide::Full:
		pformat->setBorderColor(c);
		break;
	case TBorderSide::Left:
		pformat->setBorderLeftColor(c);
		break;
	case TBorderSide::Right:
		pformat->setBorderRightColor(c);
		break;
	case TBorderSide::Top:
		pformat->setBorderTopColor(c);
		break;
	case TBorderSide::Bottom:
		pformat->setBorderBottomColor(c);
		break;
	case TBorderSide::Diagonal:
		pformat->setBorderDiagonalColor(c);
		break;
	case TBorderSide::Background:
		pformat->setPatternBackgroundColor(c);
		break;
	case TBorderSide::Foreground:
		pformat->setPatternForegroundColor(c);
		break;
	}
}

NS_Excel::TColor NS_Excel::TExcelBookFormat::getBorderColor(const TBorderSide& side) const noexcept(false)
{
	if (!isValid()) TLog("Формат не валиден!").raise(true, "TExcelBookFormat::getBorderColor");
	switch (side)
	{
	case TBorderSide::Left:
		return pformat->borderLeftColor();
	case TBorderSide::Right:
		return pformat->borderRightColor();
	case TBorderSide::Top:
		return pformat->borderTopColor();
	case TBorderSide::Bottom:
		return pformat->borderBottomColor();
	case TBorderSide::Diagonal:
		return pformat->borderDiagonalColor();
	case TBorderSide::Background:
		return pformat->patternBackgroundColor();
	case TBorderSide::Foreground:
		return pformat->patternForegroundColor();
	default:
		TLog("указанная сторона объекта - не обрабатывается!").raise(true, "TExcelBookFormat::getBorderColor");
	}
	TLog("Ошибка получения цвета!").raise(true, "TExcelBookFormat::getBorderColor");
}

NS_Excel::TBaseObj::TBaseObj(int par_val, int par_min, int par_max): min_val(par_min), max_val(par_max), val(par_val)
{
	if (!isValid()) val = EmptyType;
}

bool NS_Excel::TBaseObj::isValid() const
{
	if (isEmpty()) return false;
	if (min_val <= val)
	{
		if (max_val == 0)
			return true;
		else
			return val <= max_val;
	}
	return false;
}

int NS_Excel::TBaseObj::Next()
{
	val++;
	if (!isValid()) val--;
	return val;
}

int NS_Excel::TBaseObj::Prev()
{
	val--;
	if (!isValid()) val++;
	return val;
}

NS_Excel::TBaseObj& NS_Excel::TBaseObj::operator=(const TBaseObj& v)
{
	if (this == &v) return *this;
	val = v.val;
	return *this;
}

NS_Excel::TBaseObj& NS_Excel::TBaseObj::operator=(int x)
{
	if (TBaseObj(x).isValid())
		val = x;
	return *this;
}

string NS_Excel::TBaseObj::getName(bool row) const
{
	if (!isEmpty())
	{
		std::stringstream ss;
		if (row)
			ss << "R";
		else
			ss << "C";
		ss << val;
		return ss.str();
	}
	return string();
}

bool NS_Excel::TExcelRange::inRange(int val, const TObjType& t) const
{
	switch (t)
	{
	case TObjType::Row: return (val >= first.getRow() and val <= last.getRow());
	case TObjType::Col: return (val >= first.getCol() and val <= last.getCol());
	}
	return false;
}

bool NS_Excel::TExcelRange::inRange(const TExcelCell& val) const
{
	return (inRange(val.getRow(), TObjType::Row) and inRange(val.getCol(), TObjType::Col));
}

NS_Excel::TExcelCell NS_Excel::TExcelRange::operator()(int row_indx, int col_indx) const
{
	TExcelCell cell(row_indx, col_indx);
	if (inRange(cell)) return cell;
	TLog("Ячейка " + cell.getName() + " вне диапазона: " + getName()).raise(true, "TExcelRange::operator()");
}

void NS_Excel::TExcelBookSheet::initSheet(BookPtr book, const string& name, bool active_flg) noexcept(false)
{
	if (book)
	{
		sheet = book->addSheet(name.c_str());
		if (sheet and active_flg)
		{
			int indx = book->sheetCount();
			book->setActiveSheet(indx);
		}
	}
}

NS_Excel::TExcelBookSheet::TExcelBookSheet(BookPtr book, const string& name, bool as_active): sheet(nullptr)
{
	initSheet(book, name, as_active);
}

bool NS_Excel::TExcelBookSheet::WriteAsString(const TExcelCell& cell, const string& val, FormatPtr format, const TDataType& type)
{
	if (cell.isValid())
	{
		return sheet->writeStr(cell.getRow(), cell.getCol(), val.c_str(), format, type);
	}
	return false;
}

std::string NS_Excel::TExcelBookSheet::ReadAsString(const TExcelCell& cell) noexcept(false)
{
	if (isValid())
	{
//		FormatPtr* format = new FormatPtr;
		 const char* val = sheet->readStr(cell.getRow(), cell.getCol());
//		delete format;
		if (val) return string(val);
	}
	TLog("Объект не валиден - лист не создан!").raise(true, "TExcelBookSheet::ReadAsString");
}

double NS_Excel::TExcelBookSheet::ReadAsNumber(const TExcelCell& cell) const
{
	if (isValid())
	{
//		FormatPtr* format = new FormatPtr;
		double result = sheet->readNum(cell.getRow(), cell.getCol());
/*
		int ft = (*format)->numFormat();
		delete format;
		if ((ft >= TExcelBookFormat::TNumFormat::NUMFORMAT_DATE
			and ft <= TExcelBookFormat::TNumFormat::NUMFORMAT_CUSTOM_MDYYYY_HMM) or
			ft >= TExcelBookFormat::TNumFormat::NUMFORMAT_CUSTOM_MMSS
			and ft <= TExcelBookFormat::TNumFormat::NUMFORMAT_CUSTOM_MMSS0)
			throw string("Ячейка содержит данные типа Дата!");
		if (ft == TExcelBookFormat::TNumFormat::NUMFORMAT_TEXT)
			throw string("Ячейка содержит строку!");
/**/		
		return result;
	}
	TLog("Страница не создана!").raise(true, "TExcelBookSheet::ReadAsNumber");
}

bool NS_Excel::TExcelBookSheet::WriteAsNumber(const TExcelCell& cell, double val, FormatPtr format)
{
	if (isValid())
	{
		return sheet->writeNum(cell.getRow(), cell.getCol(), val, format);
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::ReadAsBool(const TExcelCell& cell) const
{
	if (isValid())
	{
		return sheet->readBool(cell.getRow(), cell.getCol());
	}
	TLog("Страница не создана!").raise(true, "TExcelBookSheet::ReadAsBool");
}

bool NS_Excel::TExcelBookSheet::WriteAsBool(const TExcelCell& cell, bool val, FormatPtr format) const
{
	if (isValid())
	{
		return sheet->writeBool(cell.getRow(), cell.getCol(), val, format);
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::isBlank(const TExcelCell& cell) const
{
	if (isValid())
	{
		//FormatPtr *format = new FormatPtr;
		bool result = sheet->readBlank(cell.getRow(), cell.getCol(), nullptr);
		//delete format;
		return result;
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::setBlank(const TExcelCell& cell, FormatPtr format) const
{
	if (isValid())
	{
		return sheet->writeBlank(cell.getRow(), cell.getCol(), format);
	}
	return false;

}

std::string NS_Excel::TExcelBookSheet::ReadFormula(const TExcelCell& cell)
{
	if (isValid())
	{
		const char* val = sheet->readFormula(cell.getRow(), cell.getCol());
		return string(val);
	}
	TLog("Страница не создана!").raise(true, "TExcelBookSheet::ReadFormula");
}

bool NS_Excel::TExcelBookSheet::WirteFormula(const TExcelCell& cell, const string& formula, FormatPtr format)
{
	if (isValid() and !formula.empty())
	{
		return sheet->writeFormula(cell.getRow(), cell.getCol(), formula.c_str(), format);
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::WirteFormulaBoolAsDef(const TExcelCell& cell, const string& formula, bool def_val, FormatPtr format)
{
	if (isValid() and !formula.empty())
	{
		return sheet->writeFormulaBool(cell.getRow(), cell.getCol(), formula.c_str(), def_val, format);
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::WirteFormulaNumAsDef(const TExcelCell& cell, const string& formula, double def_val, FormatPtr format)
{
	if (isValid() and !formula.empty())
	{
		return sheet->writeFormulaNum(cell.getRow(), cell.getCol(), formula.c_str(), def_val, format);
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::WirteFormulaStrAsDef(const TExcelCell& cell, const string& formula, const string& def_val, FormatPtr format)
{
	if (isValid() and !formula.empty())
	{
		return sheet->writeFormulaBool(cell.getRow(), cell.getCol(), formula.c_str(), def_val.c_str(), format);
	}
	return false;
}

string NS_Excel::TExcelBookSheet::getComment(const TExcelCell& cell) const noexcept(false)
{
	if (isValid())
	{
		const char* tmp = sheet->readComment(cell.getRow(), cell.getCol());
		if (tmp) return string(tmp);
	}
	TLog("Страница не создана!").raise(true, "TExcelBookSheet::getComment");
}

bool NS_Excel::TExcelBookSheet::setComment(const TExcelCell& cell, const string& val, const string& author, int width, int height) noexcept(false)
{
	if (isValid() and !val.empty())
	{
		sheet->writeComment(cell.getRow(), cell.getCol(), val.c_str(), author.c_str(), width, height);
		return true;
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::delComment(const TExcelCell& cell) noexcept(false)
{
	if (isValid())
	{
		sheet->removeComment(cell.getRow(), cell.getCol());
		return true;
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::setErrorType(const TExcelCell& cell, const TErrorType& er, FormatPtr format)
{
	if (isValid())
	{
		sheet->writeError(cell.getRow(), cell.getCol(), er, format);
		return true;
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::setColsParams(const TExcelRange& r, double width, bool hide, FormatPtr format) noexcept(true)
{
	if (isValid() and r.isValid())
	{
		return sheet->setCol(r.getFirst().getCol(), r.getLast().getCol(), width, format, hide);
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::setRowHide(int row_indx, bool hide)
{
	if (isValid())
	{
		return sheet->setRowHidden(row_indx, hide);
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::setColHide(int col_indx, bool hide)
{
	if (isValid())
	{
		return sheet->setColHidden(col_indx, hide);
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::inMergeRange(const TExcelCell& cell, TExcelRange& result_range) noexcept(true)
{
	result_range.clear();
	if (isValid())
	{
		int first_row = 0, first_col = 0, last_row = 0, last_col = 0;
		bool reslt = sheet->getMerge(cell.getRow(), cell.getCol(), &first_row, &last_row, &first_col, &last_col);
		if (first_row + first_col + last_row + last_col == 0 or !reslt)
			return false;
		result_range = TExcelRange(first_row, first_col, last_row, last_col);
		return reslt;
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::setMergeRange(const TExcelRange& range) noexcept(true)
{
	if (isValid())
	{
		return sheet->setMerge(range.getFirst().getRow(), range.getLast().getRow(), 
			range.getFirst().getCol(), range.getLast().getCol());
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::delMerge(const TExcelCell& cell) noexcept(true)
{
	if (isValid())
	{
		return sheet->delMerge(cell.getRow(), cell.getCol());
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::getMergeRangeByIndx(int indx, TExcelRange& result_range) noexcept(true)
{
	result_range.clear();
	if (isValid())
	{
		int first_row = 0, first_col = 0, last_row = 0, last_col = 0;
		bool reslt = sheet->merge(indx, &first_row, &last_row, &first_col, &last_col);
		if (first_row + first_col + last_row + last_col == 0 or !reslt)
			return false;
		result_range = TExcelRange(first_row, first_col, last_row, last_col);
		return reslt;
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::setSplit(const TExcelCell& cell)
{
	if (isValid())
	{
		sheet->split(cell.getRow(), cell.getCol());
		return false;
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::getSplit(TExcelCell& cell) const
{
	cell.clear();
	if (isValid())
	{
		int row = 0, col = 0;
		bool result = sheet->splitInfo(&row, &col);
		if (row + col == 0 or !result) return false;
		cell = TExcelCell(row, col);
		return result;
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::clear(const TExcelRange& range) noexcept(true)
{
	if (isValid() and range.isValid())
	{
		sheet->clear(range.getFirst().getRow(), range.getLast().getRow(),
			range.getFirst().getCol(), range.getLast().getCol());
		return true;
	}
	return false;
}

//bool NS_Excel::TExcelBookSheet::

bool NS_Excel::TExcelBookSheet::RangeOperation(const TExcelRange& range, bool insrt, 
	bool asRow, bool updNameRange) noexcept(false)
{
	if (isValid() and range.isValid())
	{
		bool tmp = false;
		if (insrt)
		{
			if (asRow)
				tmp = sheet->insertRow(range.getFirst().getRow(), range.getLast().getRow(), updNameRange);
			else
				tmp = sheet->insertCol(range.getFirst().getCol(), range.getLast().getCol(), updNameRange);
		}
		else
		{
			if (asRow)
				tmp = sheet->removeRow(range.getFirst().getRow(), range.getLast().getRow(), updNameRange);
			else
				tmp = sheet->removeCol(range.getFirst().getCol(), range.getLast().getCol(), updNameRange);
		}
		if (!tmp)
		{
			TLog log("Ошибка выполнения операции ");
			log << (insrt ? "вставки" : "удаления") << " для ";
			log += (asRow ? "строки" : "столбца");
			log.raise(true, "TExcelBookSheet::RangeOperation");
		}
		return tmp;
	}
	return false;
}

string NS_Excel::TExcelBookSheet::getName() const
{
	if (isValid())
	{
		const char* ch = sheet->name();
		return string(ch);
	}
	TLog("Лист не инициализирован!").raise(true, "TExcelBookSheet::getName");
}

NS_Excel::TExcelBookFormat NS_Excel::TExcelBookSheet::getCellFormat(const TExcelCell& cell) const
{
	if (isValid())
	{
		FormatPtr format = sheet->cellFormat(cell.getRow(), cell.getCol());
		return TExcelBookFormat(format);
	}
	TLog("Лист не инициализирован!").raise(true, "TExcelBookSheet::getCellFormat");
}

bool NS_Excel::TExcelBookSheet::setCellFormat(const TExcelCell& cell, TExcelBookFormat& format) noexcept(false)
{
	if (isValid())
	{
		sheet->setCellFormat(cell.getRow(), cell.getCol(), format.pformat);
		return true;
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::setCellColor(const TExcelCell& cell, const TColor& color) noexcept(true)
{
	if (isValid())
	{
		try
		{
			//получение формата ячейки:
			TExcelBookFormat format = getCellFormat(cell);
			format.setBorderColor(color, TExcelBookFormat::TBorderSide::Foreground);
			//не известно надо ли:
			setCellFormat(cell, format);
			return true;
		}
		catch (const string& s)
		{
			cerr << "При установки цвета ячейки " << cell.getName() << "произошла ошибка: " << s << endl;
			return false;
		}
		catch (...)
		{
			cerr << "Ошибка при установке цвета ячейки: " << cell.getName() << endl;
			return false;
		}
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::setRangeColor(const TExcelRange& range, const TColor& color) noexcept(true)
{
	if (range.isValid())
	{
		bool flg = false;
		int first_row = range.getFirst().getRow(), last_row = range.getLast().getRow();
		int first_col = range.getFirst().getCol(), last_col = range.getLast().getCol();
		for (int i = first_row; i <= last_row; i++)
			for (int j = first_col; j <= last_col; j++)
			{
				TExcelCell cell(i, j);
				setCellColor(cell, color);
			}
		return true;
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::setRowColor(int row_indx, const TColor& color) noexcept(true)
{
	TExcelRange range(row_indx, row_indx, 0, getLastCol());
	return setRangeColor(range, color);
}

bool NS_Excel::TExcelBookSheet::setColColor(int col_indx, const TColor& color) noexcept(true)
{
	TExcelRange range(getFirstRow(), getLastRow(), col_indx, col_indx);
	return setRangeColor(range, color);
}

bool NS_Excel::TExcelBookSheet::setCellTextColor(const TExcelCell& cell, const TColor& color) noexcept(true)
{
	if (isValid())
	{
		try
		{
			//получение формата ячейки:
			TExcelBookFormat format = getCellFormat(cell);
			TExcelBookFont font(format.getFont());
			font.setColor(color);
			//не известно надо ли:
			format.setFont(font);
			setCellFormat(cell, format);
			return true;
		}
		catch (const string& s)
		{
			cerr << "При установки цвета текста ячейки " << cell.getName() << "произошла ошибка: " << s << endl;
			return false;
		}
		catch (...)
		{
			cerr << "Ошибка при установке цвета текста ячейки: " << cell.getName() << endl;
			return false;
		}
	}
	return false;
}

string NS_Excel::TExcelBook::getError() const noexcept(true)
{
	if (isValid())
		return string(book->errorMessage());
	return string("No error!");
}

void NS_Excel::TExcelBook::CrtBook(int header_row) noexcept(true)
{
	//ключи лицензии:
	string name = "GCCG";
	string key = "windows-282123090cc0e6036db16b60a1o3p0h9";
	book = xlCreateBook();
	HeaderRow = header_row;
	book->setKey(name.c_str(), key.c_str());
}

string NS_Excel::TExcelBook::getDefaultSheetName() const noexcept(true)
{
	std::stringstream name;
	//NS_Const::TConstExclTune t(NS_Const::TExclBaseTune::DefSh);
	//name << t.toStr();
	if (isValid())
		name << " " << SheetCount() + 1;
	return name.str();
}

bool NS_Excel::TExcelBook::loadFromFile(const TLoadParam& param, const LoadType& lt, bool raise_err) noexcept(false)
{
	if (param.file.empty())
	{
		TLog("Указан пустой файл!").raise(raise_err, "TExcelBook::loadFromFile");
		return false;
	}
	if (!isValid())
	{
		TLog("Объект excel-книга - не валиден!").raise(raise_err, "TExcelBook::loadFromFile");
		return false;
	}
	bool result = false;
	switch (lt)
	{
	case LoadType::Full: 
		result = book->load(param.file.c_str(), param.tmp_file.c_str());
		break;
	case LoadType::Sheet:
		result = book->loadSheet(param.file.c_str(), param.Indx, param.tmp_file.c_str());
		break;
	case LoadType::Rows:
		result = book->loadPartially(param.file.c_str(), param.Indx, param.first, param.last, param.tmp_file.c_str());
		break;
	}
	if (!result) TLog(book->errorMessage()).raise(raise_err, "TExcelBook::loadFromFile");
	return result;
}

bool NS_Excel::TExcelBook::isValid(bool raise_err) const noexcept(false)
{
	if (book) return true;
	TLog("Рабочая книга не инициализирована!").raise(raise_err, "TExcelBook::isValid");
	return false;
}

void NS_Excel::TExcelBook::setHeaderByStrArr(const TStrArr& arr) noexcept(true)
{
	if (arr.empty()) return;
	//создаем страницу в отчете:
	string sh_name = getDefaultSheetName();
	TExcelBookSheet sheet = AddSheet(sh_name, true);
	//задаем формат b шрифт заголовка:
	TExcelBookFont font = AddFont();
	TExcelBookFormat format = AddFormat();
	//формируем формат
	if (format.isValid())
	{
		format.setAlignH(TExcelBookFormat::TAlignHType::ALIGNH_CENTER);
		format.setAlignV(TExcelBookFormat::TAlignVType::ALIGNV_CENTER);
		format.setBorderStyle(TExcelBookFormat::TBorderStyle::BORDERSTYLE_MEDIUM,
			TExcelBookFormat::TBorderSide::Full);
		format.setPatternFill(TExcelBookFormat::TFill::FILLPATTERN_SOLID);
		format.setBorderColor(TColor::COLOR_GRAY25, TExcelBookFormat::TBorderSide::Foreground);
		//format.setBorderColor(TColor::COLOR_YELLOW, TExcelBookFormat::TBorderSide::Background);
		//формируем шрифт
		if (font.isValid())
		{
			font.setBold();
			format.setFont(font);
		}
	}
	for (size_t i = 0; i < arr.size(); i++)
	{
		TExcelCell cell(HeaderRow, i);
		sheet.WriteAsString(cell, arr[i], format.pformat);
	}
}

bool NS_Excel::TExcelBook::initByParam(const TExcelParam& param) noexcept(true)
{
	//если указан шаблон - грузим файл шаблона и выходим
	if (!param.getTemplateName().empty())
		return load(param.getTemplateName(), false);
		//если шаблон не указан
		//в созданную книгу добавляем наименования колонок из массива
		//выделяем данную строку в качестве заголовка
	setHeaderByStrArr(param.getHeader());
	return true;
}

bool NS_Excel::TExcelBook::load(const string& file, bool raise_err, const string& tmp_file) noexcept(false)
{
	TLoadParam param;
	param.file = file;
	param.tmp_file = tmp_file;
	return loadFromFile(param, LoadType::Full, raise_err);
}

bool NS_Excel::TExcelBook::loadSheetOnly(const string& file, int sheet_indx,
	bool raise_err, const string& tmp_file) noexcept(false)
{
	TLoadParam param;
	param.file = file;
	param.Indx = sheet_indx;
	param.tmp_file = tmp_file;
	return loadFromFile(param, LoadType::Sheet, raise_err);
}

bool NS_Excel::TExcelBook::loadSheetRowsOnly(const string& file, int sheet_indx, int first_row, int last_row,
	bool raise_err, const string& tmp_file)
{
	TLoadParam param;
	param.file = file;
	param.tmp_file = tmp_file;
	param.Indx = sheet_indx;
	param.first = first_row;
	param.last = last_row;
	return loadFromFile(param, LoadType::Rows, raise_err);
}

bool NS_Excel::TExcelBook::SaveToFile(const string& file, bool use_tmp, bool raise_err) noexcept(false)
{
	if (isValid())
	{
		bool result = book->save(file.c_str(), use_tmp);
		if (!result)
			if (raise_err)
				throw getError();
			else
				cerr << getError() << endl;
		return result;
	}
	return false;
}

NS_Excel::TExcelBookSheet NS_Excel::TExcelBook::AddSheet(const string& name, bool asActive) noexcept(false)
{
	if (isValid(true))
	{
		string tmp_name(name);
		if (tmp_name.empty())
			tmp_name = getDefaultSheetName();
		TExcelBookSheet sh(book, tmp_name, asActive);
		if (!sh.isValid()) TLog(getError()).raise(true, "TExcelBook::AddSheet");
		return sh;
	}
	TLog("Книга не валидна!").raise(true, "TExcelBook::AddSheet");
}

NS_Excel::TExcelBookSheet NS_Excel::TExcelBook::InsertSheet(int index, const string& name, SheetPtr initSheet) noexcept(false)
{
	if (isValid(true))
	{
		SheetPtr sh = book->insertSheet(index, name.c_str(), initSheet);
		if (sh == nullptr) TLog(getError()).raise(true, "TExcelBook::InsertSheet");
		return TExcelBookSheet(sh);
	}
	TLog("Книга не валидна!").raise(true, "TExcelBook::InsertSheet");
}

NS_Excel::TExcelBookSheet NS_Excel::TExcelBook::getSheetByIndex(int index) const noexcept(false)
{
	if (isValid(true))
	{
		SheetPtr sheet = book->getSheet(index);
		if (!sheet) TLog(getError()).raise(true, "TExcelBook::getSheetByIndex");
		return TExcelBookSheet(sheet);
	}
	TLog("Книга не инициализирована!").raise(true, "TExcelBook::getSheetByIndex");
}

string NS_Excel::TExcelBook::getSheetNameByIndex(int index) const noexcept(false)
{
	if (isValid(true))
	{
		string s = book->getSheetName(index);
		if (s.empty()) TLog(getError()).raise(true, "TExcelBook::getSheetNameByIndex");
		return s;
	}
	TLog("Книга не инициализирована!").raise(true, "TExcelBook::getSheetNameByIndex");
}

bool NS_Excel::TExcelBook::MoveSheetByIndex(int old_indx, int new_indx) noexcept(false)
{
	if (isValid())
	{
		bool tmp = book->moveSheet(old_indx, new_indx);
		if (tmp == false) TLog(getError()).raise(true, "TExcelBook::MoveSheetByIndex");
		return tmp;
	}
	return false;
}

bool NS_Excel::TExcelBook::DelSheetByIndex(int index) noexcept(false)
{
	if (isValid())
	{
		bool tmp = book->delSheet(index);
		if (tmp == false) TLog(getError()).raise(true, "TExcelBook::DelSheetByIndex");
		return tmp;
	}
	return false;
}

NS_Excel::TExcelBookFormat NS_Excel::TExcelBook::AddFormat(FormatPtr initFormat) noexcept(false)
{
	if (isValid(true))
	{
		FormatPtr format = book->addFormat(initFormat);
		if (!format) TLog(getError()).raise(true, "TExcelBook::AddFormat");
		return TExcelBookFormat(format);
	}
	TLog("Книга не инициализирована!").raise(true, "TExcelBook::AddFormat");
}

NS_Excel::TExcelBookFont NS_Excel::TExcelBook::AddFont(FontPtr initFont) noexcept(false)
{
	if (isValid(true))
	{
		FontPtr font = book->addFont(initFont);
		if (!font) TLog(getError()).raise(true, "TExcelBook::AddFont");
		return TExcelBookFont(font);
	}
	TLog("Книга не инициализирована!").raise(true, "TExcelBook::AddFont");
}

NS_Excel::TExcelBookFormat NS_Excel::TExcelBook::getFormatByIndex(int index) noexcept(false)
{
	if (isValid(true))
	{
		FormatPtr format = book->format(index);
		if (!format) TLog(getError()).raise(true, "TExcelBook::getFormatByIndex");
		return TExcelBookFormat(format);
	}
	TLog("Книга не инициализирована!").raise(true, "TExcelBook::getFormatByIndex");
}

NS_Excel::TExcelBookFont NS_Excel::TExcelBook::getFontByIndex(int index) noexcept(false)
{
	if (isValid(true))
	{
		FontPtr font = book->font(index);
		if (!font) TLog(getError()).raise(true, "TExcelBook::getFontByIndex");
		return TExcelBookFont(font);
	}
	TLog("Книга не инициализирована!").raise(true, "TExcelBook::getFontByIndex");
}

bool NS_Excel::TExcelBook::setActiveSheet(int index) noexcept(true)
{
	if (isValid())
	{
		book->setActiveSheet(index);
		return true;
	}
	return false;
}

bool NS_Excel::TExcelBook::setAsTemplate(bool flg) noexcept(true)
{
	if (isValid())
	{
		book->setTemplate(flg);
		return true;
	}
	return false;
}

bool NS_Excel::TExcelBook::setLocale(const string& locale) noexcept(true)
{
	if (isValid())
	{
		return book->setLocale(locale.c_str());
	}
	return false;
}

bool NS_Excel::TExcelBook::setRxCxRef(bool flg) noexcept(true)
{
	if (isValid())
	{
		book->setRefR1C1(flg);
		return true;
	}
	return false;
}

void NS_Excel::TExcelBook::DefultFont(string& font_name, int& size) noexcept(false)
{
	if (isValid(true))
	{
		const char* ch = book->defaultFont(&size);
		if (!ch) 
			TLog(getError()).raise(true, "TExcelBook::DefultFont");
		else
			font_name = string(ch);
	}
}

bool NS_Excel::TExcelBook::setDefaultFont(const string& font_name, int size) noexcept(true)
{
	if (isValid())
	{
		book->setDefaultFont(font_name.c_str(), size);
		return true;
	}
	return false;
}

double NS_Excel::TExcelBook::Date2Double(const TExcelDate& date) noexcept(false)
{
	if (isValid())
		return book->datePack(date.year, date.month, date.day, date.hour, date.minute, date.sec, date.msec);
	return 0;
}

bool NS_Excel::TExcelBook::Double2Date(double value, TExcelDate& date) noexcept(false)
{
	using std::abs;
	if (isValid())
	{
		int year = date.year;
		int month = date.month;
		int day = date.day;
		int hour = date.hour;
		int minute = date.minute;
		int sec = date.sec;
		int msec = date.msec;
		book->dateUnpack(value, &year, &month, &day, &hour, &minute, &sec, &msec);
		date = { year, unsigned(month), unsigned(day), unsigned(hour), unsigned(minute), unsigned(sec), unsigned(msec) };
		return true;
	}
	return false;
}