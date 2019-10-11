#include <iostream>
#include <sstream>
#include "TExcel.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

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
	pfont->script();
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
	pfont->color();
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
		if (!pformat) throw string(book->errorMessage());
	}
}

NS_Excel::TExcelBookFont NS_Excel::TExcelBookFormat::getFont() const noexcept(false)
{
	if (isValid())
	{
		FontPtr pfont = pformat->font();
		return TExcelBookFont(pfont);
	}
	throw string("‘ормат не валиден! ѕолучить шрифт невозможно!");
}

bool NS_Excel::TExcelBookFormat::setFont(TExcelBookFont& fnt) noexcept(false)
{
	if (isValid() and fnt.isValid())
		return pformat->setFont(fnt.getRef());
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

void NS_Excel::TExcelBookFormat::setBorderColor(TColor c, const TBorderSide& side) noexcept(false)
{
	if (isValid()) return;
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
	if (!isValid()) throw string("‘ормат не валиден!");
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
		throw string("указанна€ сторона объекта - не обрабатываетс€!");
	}
	throw string("ќшибка получени€ цвета!");
}

NS_Excel::TBaseObj::TBaseObj(int par_val, int par_min = 0, int par_max = 0): min_val(par_min), max_val(par_max), val(par_val)
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
	throw string("ячейка " + cell.getName() + " вне диапазона: " + getName());
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

NS_Excel::TExcelBookSheet::TExcelBookSheet(BookPtr book, const string& name, bool as_active = true): sheet(nullptr)
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
	throw string("ReadAsString");
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
			throw string("ячейка содержит данные типа ƒата!");
		if (ft == TExcelBookFormat::TNumFormat::NUMFORMAT_TEXT)
			throw string("ячейка содержит строку!");
/**/		
		return result;
	}
	throw string("ReadAsNumber");
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
	throw string("ReadAsBool");
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
	throw string("ReadFormula");
}

bool NS_Excel::TExcelBookSheet::WirteFormula(const TExcelCell& cell, const string& formula, FormatPtr format)
{
	if (isValid() and !formula.empty())
	{
		return sheet->writeFormula(cell.getRow(), cell.getCol.getCol(), formula.c_str(), format);
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::WirteFormulaBoolAsDef(const TExcelCell& cell, const string& formula, bool def_val, FormatPtr format)
{
	if (isValid() and !formula.empty())
	{
		return sheet->writeFormulaBool(cell.getRow(), cell.getCol.getCol(), formula.c_str(), def_val, format);
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::WirteFormulaNumAsDef(const TExcelCell& cell, const string& formula, double def_val, FormatPtr format)
{
	if (isValid() and !formula.empty())
	{
		return sheet->writeFormulaNum(cell.getRow(), cell.getCol.getCol(), formula.c_str(), def_val, format);
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::WirteFormulaStrAsDef(const TExcelCell& cell, const string& formula, const string& def_val, FormatPtr format)
{
	if (isValid() and !formula.empty())
	{
		return sheet->writeFormulaBool(cell.getRow(), cell.getCol.getCol(), formula.c_str(), def_val.c_str(), format);
	}
	return false;
}

bool NS_Excel::TExcelBookSheet::WirteFormulaBoolAsDef(const TExcelCell& cell, const string& formula, bool def_val, FormatPtr format)
{
	if (isValid() and !formula.empty())
	{
		return sheet->writeFormulaBool(cell.getRow(), cell.getCol.getCol(), formula.c_str(), def_val, format);
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
	throw string("getComment");
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

bool NS_Excel::TExcelBookSheet::clear(const TExcelRange& range)
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
		if (!tmp) throw string("RangeOperation");//????
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
	throw string("TExcelBookSheet::getName");
}

NS_Excel::TExcelBookFormat NS_Excel::TExcelBookSheet::getCellFormat(const TExcelCell& cell) const
{
	if (isValid)
	{
		FormatPtr format = sheet->cellFormat(cell.getRow(), cell.getCol());
		return TExcelBookFormat(format);
	}
	throw string("TExcelBookSheet::getCellFormat");
}

bool NS_Excel::TExcelBookSheet::setCellFormat(const TExcelCell& cell, TExcelBookFormat& format) noexcept(false)
{
	if (isValid())
	{
		sheet->setCellFormat(cell.getRow(), cell.getCol(), format.getRef());
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
			//получение формата €чейки:
			TExcelBookFormat format = getCellFormat(cell);
			format.setBorderColor(color, TExcelBookFormat::TBorderSide::Background);
			//не известно надо ли:
			setCellFormat(cell, format);
			return true;
		}
		catch (const string& s)
		{
			cerr << "ѕри установки цвета €чейки " << cell.getName() << "произошла ошибка: " << s << endl;
			return false;
		}
		catch (...)
		{
			cerr << "ќшибка при установке цвета €чейки: " << cell.getName() << endl;
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
			//получение формата €чейки:
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
			cerr << "ѕри установки цвета текста €чейки " << cell.getName() << "произошла ошибка: " << s << endl;
			return false;
		}
		catch (...)
		{
			cerr << "ќшибка при установке цвета текста €чейки: " << cell.getName() << endl;
			return false;
		}
	}
	return false;
}

bool NS_Excel::TExcelBook::loadFromFile(const TLoadParam& param, const LoadType& lt, bool raise_err) noexcept(false)
{
	if (param.file.empty())
	{
		if (raise_err)
			throw string("”казан пустой файл!");
		else
			cerr << "”казан пустой файл!" << endl;
		return false;
	}
	if (!isValid())
	{
		if (raise_err)
			throw string("ќбъект excel-книга - не валиден!");
		else
			cerr << "ќбъект excel-книга - не валиден!" << endl;
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
	if (!result)
		if (raise_err)
			throw string(book->errorMessage());
		else
			cerr << book->errorMessage() << endl;
	return result;
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
				throw string(book->errorMessage());
			else
				cerr << book->errorMessage() << endl;
		return result;
	}
	return false;
}

string NS_Excel::getTuneFormat(const TFormatTune& val)
{
	switch (val)
	{
	case TFormatTune::xlt: ".xlt";
	case TFormatTune::xls:
	case TFormatTune::DefExt:
		return ".xls";
	case TFormatTune::xlsx: return ".xlsx";
	case TFormatTune::DefName: return "NoFileName";
	case TFormatTune::DefSh: return "ќтчет";
	}
	return string();
}

bool NS_Excel::isTemplate(const string& str)
{
	if (str.empty()) return false;
	string s = getTuneFormat(TFormatTune::xlt);
	size_t pos = str.rfind(s);
	if (pos == string::npos or (pos + s.size() < str.size()))
		return false;
	return true;
}

void NS_Excel::TExcel::setDefOutFileName(void)
{
	name = getTuneFormat(TFormatTune::DefName);
	name += getTuneFormat(TFormatTune::DefExt);
}

void NS_Excel::TExcel::setDefFont(void) noexcept(false)
{
	//FAQ: http://www.libxl.com/font.html
	if (file)
	{
		Font* font = file->addFont();
		if (font)
		{
			//задаем умолчательные настройки дл€ шрифта:
			font->setSize(DefVal::FontSize);
			font->setName("Times New Roman");
		}
		else
			throw Logger::TLog(file->errorMessage(), nullptr);
	}
}

void NS_Excel::TExcel::setDefFormat(void) noexcept(false)
{
	//FAQ: http://www.libxl.com/format.html
	if (file)
	{
		Format* format = file->addFormat();
		if (format)
		{
			//если основной шрифт установлен - ставим его в основной формат
			if (file->fontSize() >= DefVal::MainIndx)
				format->setFont(file->font(DefVal::MainIndx));
			//выравнивание
			format->setAlignH(libxl::ALIGNH_RIGHT);
			format->setAlignV(libxl::ALIGNV_BOTTOM);
			//раст€гивание по ширине
			format->setShrinkToFit(true);
		}
		else
			throw Logger::TLog(file->errorMessage(), nullptr);
	}
}

void NS_Excel::TExcel::setTitleFont(Font* src) noexcept(false)
{
	if (file)
	{
		Font* font = (src ? file->addFont(src) : file->addFont());
		if (font)
		{
			//установки дл€ заголовка отчета:
			font->setBold();
		}
		else
			throw Logger::TLog(file->errorMessage(), nullptr);
	}
}

void NS_Excel::TExcel::setTitleFormat(Format* src) noexcept(false)
{
	if (file)
	{
		Format* format = (src ? file->addFormat(src) : file->addFormat());
		if (format)
		{
			//шрифт:
			if (file->fontSize() > DefVal::TitleIndx)
				format->setFont(file->font(DefVal::TitleIndx));
			//выравнивание
			format->setAlignH(libxl::ALIGNH_FILL);
			format->setAlignV(libxl::ALIGNV_CENTER);
			//цвет:
			format->setBorder(libxl::BORDERSTYLE_MEDIUM);
			//format->setFillPattern(COLOR_GRAY80);
			format->setFillPattern(libxl::FILLPATTERN_SOLID);
			//format->setPatternBackgroundColor(COLOR_GRAY80);
			format->setPatternForegroundColor(libxl::COLOR_GRAY25);
		}
		else
			throw Logger::TLog(file->errorMessage());
	}
}

void NS_Excel::TExcel::copyFont(int index, Font* src)
{
	if (file)
	{
		Font* font = file->font(index);
		if (font)
		{
			font = file->addFont(src);
		}
		else
			throw Logger::TLog(file->errorMessage());

	}
}

void NS_Excel::TExcel::copyFormat(int index, Format* src)
{
	if (file)
	{
		Format* format;
		format = file->format(index);
		if (format)
		{
			format = file->addFormat(src);
		}
		else
			throw Logger::TLog(file->errorMessage());
	}
}

void NS_Excel::TExcel::clear(void)
{
	if (file) file->release();
}

bool NS_Excel::TExcel::AddSheet(const string& name, bool set_as_active)
{
	//FAQ: http://www.libxl.com/spreadsheet.html
	if (file)
	{
		Sheet* sh = file->addSheet(name.c_str());
		if (sh)
		{
			//делаем ее активной
			if (set_as_active) setActiveSheet(SheetCount());
			return true;
		}
	}
	return false;
}

void NS_Excel::TExcel::setActiveSheet(int index)
{
	if (file)
	{
		//если индекс превышает число страниц в файле - ошибка
		if (SheetCount() < index)
		{
			std::stringstream ss;
			ss << "”казанный индекс " << index << " превышает число страниц!";
			throw Logger::TLog(ss.str());
		}
		active_sh = file->getSheet(index);
		file->setActiveSheet(index);
	}
}

int NS_Excel::TExcel::getUsedRow(Sheet* sh, bool last) const
{
	if (sh)
	{
		if (last)
			return sh->lastRow();
		else
			return sh->firstRow();
	}
	return DefVal::Empty;
}

int NS_Excel::TExcel::getUsedCell(Sheet* sh, bool last) const
{
	if (sh)
	{
		if (last)
			return sh->lastCol();
		else
			return sh->firstCol();
	}
	return DefVal::Empty;
}

void NS_Excel::TExcel::FillColumnsFormat(int TitleRow, int SHIndex)
{
	int oldindex = DefVal::Empty;
	//провер€ем есть ли листы в документе
	if (SheetCount() > 0 and TitleRow > 0 and isTemplate())
	{
		if (SHIndex != DefVal::Empty)
		{
			//устанавливаем активный лист
			oldindex = ActiveSheet();
			setActiveSheet(SHIndex);
		}
		if (active_sh)
		{
			//анализируем строку после строки заголовка, дл€ получени€ формата столбцов:
			std::pair<int, int> range(getUsedCell(active_sh, false), getUsedCell(active_sh, true));
			for (int i = range.first; i <= range.second; i++)
				OrdColumnsFormat.push_back(active_sh->cellType(TitleRow, i));
		}
		//возвращаем исходный активный лист:
		if (oldindex != DefVal::Empty)
			setActiveSheet(oldindex);
	}
}

NS_Excel::TExcel::TExcel(const string& tmp_name, const string& out_name, bool crt_active_sh) : name(out_name)
{
	//FAQ: http://www.libxl.com/workbook.html
	//задаем им€ выходного файла
	if (name.empty())
		setDefOutFileName();
	//формируем объект excel-файла
	file = xlCreateBook();
	if (file)
	{
		//пробуем загрузить книгу:
		if (!tmp_name.empty())
		{
			if (!file->load(tmp_name.c_str()))
				cerr << file->errorMessage() << endl;
		}
		else
			if (crt_active_sh)
				AddSheet(getTuneFormat(TFormatTune::DefSh), true);
	}
}
