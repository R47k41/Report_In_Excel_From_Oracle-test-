#include <iostream>
#include <sstream>
#include "TExcel.h"
using std::string;
using std::cout;
using std::cerr;
using std::endl;
using NS_Excel::TFormatTune;

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
	if (SheetCount() > 0 and TitleRow > 0)
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

NS_Excel::TExcel::TExcel(const string& fname, bool crt_active_sh) : name(fname)
{
	//FAQ: http://www.libxl.com/workbook.html
	//задаем им€ выходного файла
	if (fname.empty())
		setDefOutFileName();
	//формируем объект excel-файла
	file = xlCreateBook();
	if (file)
	{
		//пробуем загрузить книгу:
		if (!fname.empty())
		{
			if (!file->load(fname.c_str()))
				cerr << file->errorMessage() << endl;
		}
		else
			if (crt_active_sh)
				AddSheet(getTuneFormat(TFormatTune::DefSh), true);
	}
}
