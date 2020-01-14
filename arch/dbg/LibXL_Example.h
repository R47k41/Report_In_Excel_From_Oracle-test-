//просто примеры от LibXL
//не участвует в программе
#pragma once
#include "libxl.h"

using namespace libxl;

class Example
{
public:

	Example()
	{
		book = xlCreateBook();
		sheet = book->addSheet("Sheet1");
	}

	~Example()
	{
		book->release();
	}

	void showH(unsigned short row, AlignH align, const char* alignStr)
	{
		Format* format = book->addFormat();
		format->setAlignH(align);
		format->setBorder();
		sheet->writeStr(row, 1, alignStr, format);
	}

	void showV(unsigned short col, AlignV align, const char* alignStr)
	{
		Format* format = book->addFormat();
		format->setAlignV(align);
		format->setBorder();
		sheet->writeStr(2, col, alignStr, format);
		sheet->setMerge(2, 6, col, col);
	}

	void showB(unsigned short row, BorderStyle style, const char* styleStr)
	{
		Format* format = book->addFormat();
		format->setBorder(style);
		sheet->writeStr(row, 1, styleStr, format);
	}

	void showC(unsigned short row, Color color, const char* colorStr)
	{
		Font* font = book->addFont();
		font->setColor(color);
		Format* format = book->addFormat();
		format->setBorder();
		format->setBorderColor(color);
		format->setFont(font);
		sheet->writeStr(row, 7, colorStr, format);
	}

	void showP(unsigned short row, Color color, FillPattern pattern = FILLPATTERN_SOLID)
	{
		Format* format = book->addFormat();
		format->setFillPattern(pattern);
		format->setPatternForegroundColor(color);
		//sheet->writeBlank(row, pattern == FILLPATTERN_SOLID ? 3 : 5, format);
		sheet->setCellFormat(row, pattern == FILLPATTERN_SOLID ? 3 : 5, format);
	}

	void run()
	{
		sheet->setDisplayGridlines(false);

		sheet->setCol(1, 1, 30);
		sheet->setCol(3, 3, 11.4);
		sheet->setCol(4, 4, 2);
		sheet->setCol(5, 5, 15);
		sheet->setCol(6, 6, 2);
		sheet->setCol(7, 7, 15.4);

		showH(2, ALIGNH_LEFT, "ALIGNH_LEFT");
		showH(4, ALIGNH_CENTER, "ALIGNH_CENTER");
		showH(6, ALIGNH_RIGHT, "ALIGNH_RIGHT");

		showV(3, ALIGNV_TOP, "ALIGNV_TOP");
		showV(5, ALIGNV_CENTER, "ALIGNV_CENTER");
		showV(7, ALIGNV_BOTTOM, "ALIGNV_BOTTOM");

		showB(12, BORDERSTYLE_MEDIUM, "BORDERSTYLE_MEDIUM");
		showB(14, BORDERSTYLE_DASHED, "BORDERSTYLE_DASHED");
		showB(16, BORDERSTYLE_DOTTED, "BORDERSTYLE_DOTTED");
		showB(18, BORDERSTYLE_THICK, "BORDERSTYLE_THICK");
		showB(20, BORDERSTYLE_DOUBLE, "BORDERSTYLE_DOUBLE");
		showB(22, BORDERSTYLE_DASHDOT, "BORDERSTYLE_DASHDOT");

		showP(12, COLOR_GREEN);
		showP(14, COLOR_BLUE);
		showP(16, COLOR_YELLOW);
		showP(18, COLOR_PINK);
		showP(20, COLOR_GREEN);
		showP(22, COLOR_GRAY25);

		showP(12, COLOR_RED, FILLPATTERN_GRAY50);
		showP(14, COLOR_BLUE, FILLPATTERN_HORSTRIPE);
		showP(16, COLOR_YELLOW, FILLPATTERN_VERSTRIPE);
		showP(18, COLOR_PINK, FILLPATTERN_REVDIAGSTRIPE);
		showP(20, COLOR_GREEN, FILLPATTERN_THINVERSTRIPE);
		showP(22, COLOR_GRAY25, FILLPATTERN_THINHORCROSSHATCH);

		showC(12, COLOR_RED, "COLOR_RED");
		showC(14, COLOR_BLUE, "COLOR_BLUE");
		showC(16, COLOR_YELLOW, "COLOR_YELLOW");
		showC(18, COLOR_PINK, "COLOR_PINK");
		showC(20, COLOR_GREEN, "COLOR_GREEN");
		showC(22, COLOR_GRAY25, "COLOR_GRAY25");

		book->save("acb.xls");
	}

private:
	Book* book;
	Sheet* sheet;
};