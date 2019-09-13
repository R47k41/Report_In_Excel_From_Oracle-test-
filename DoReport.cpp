//блок предназначен для сборки кода по созданию отчета:
//1. проверить наличие excel-файла
//2. проверить корректность соединения с базой данных
//3. заполняем названия колонок в excel-файле
//4. считываем данные из базы и записываем их в файл
//5. сохраняем файл

//для корректной работы в свойствах проекта - отключил UNICODE(кодировка unicode), 
//т.к. лень переписывать string на wstring
#include <iostream>
#include "Logger.h"
#include "TuneParam.h"
#include "TSQLParser.h"
#include "libxl.h"
#include "occi.h"
using std::string;
using std::cout;
using std::cerr;
using std::endl;
using namespace libxl;
using libxl::Book;
using libxl::Sheet;

using StrRow = std::vector<string>;

enum class TExctension { xls, xlsx, Default, DefName, DefSheet };

//преобразование расширения в строку
string getExctension(const TExctension& val);

//функция формирования отчета
void getReport(const string& file_name = "config.ini") noexcept(false);

//функция формирования заголовка отчета:
void SetTitleRow(Sheet* sh, const StrRow& row, Format* frmt = nullptr) noexcept(false);

string getExctension(const TExctension& val)
{
	switch (val)
	{
	case TExctension::xls:
	case TExctension::Default:
		return ".xls";
	case TExctension::xlsx: return ".xlsx";
	case TExctension::DefName: return "NoFileName.xls";
	case TExctension::DefSheet: return "Лист 1";
	}
	return string();
}


Format* SetTitileFormat(Book* file)
{
	if (file)
	{
		//создаем шрифт:
		Font* titlefont = file->addFont();
		if (titlefont)
		{
			titlefont->setSize(10);
			titlefont->setBold();
		}
		//создаем формат для данных:
		Format* titleFormat = file->addFormat();
		if (titleFormat)
		{
			//выравнивание
			titleFormat->setAlignH(ALIGNH_FILL);
			titleFormat->setAlignV(ALIGNV_CENTER);
			//шрифт:
			titleFormat->setFont(titlefont);
			//цвет:
			titleFormat->setBorder(BORDERSTYLE_MEDIUM);
			//titleFormat->setFillPattern(COLOR_GRAY80);
			titleFormat->setFillPattern(FILLPATTERN_SOLID);
			//titleFormat->setPatternBackgroundColor(COLOR_GRAY80);
			titleFormat->setPatternForegroundColor(COLOR_GRAY25);
		}
		return titleFormat;
	}
	return nullptr;
}

void SetTitleRow(Sheet* sh, const StrRow& row, Format* frmt) noexcept(false)
{
	if (row.empty()) return;
	if (sh)
	{
		int cnt = 0;
		for (size_t i = 0; i < row.size(); i++)
		{
			if (!row[i].empty())
			{
				sh->writeStr(1, i, row[i].c_str(), frmt);
				cnt++;
			}
		}
		sh->setAutoFitArea(1, 1, 1, cnt);
	}
}

void getReport(const string& file_name) noexcept(false)
{
	using libxl::Book;
	using NS_Tune::TuneField;
	using NS_Tune::TUserData;
	//using libxl::xlCreateBook;
	Book* file = xlCreateBook();
	if (file)
	{
		//формируем файл настроек:
		TUserData tune(file_name);
		string outFile = tune.getValue(TuneField::OutFile);
		if (outFile.empty()) outFile = getExctension(TExctension::DefName);
		//формируем лист на котором будут данные:
		string sheet_name = getExctension(TExctension::DefSheet);
		Format* title_frmt = SetTitileFormat(file);
		Sheet* sh = file->addSheet(sheet_name.c_str());
		//если лист создан
		if (sh)
		{
			//проверяем есть ли колонки для записи:
			if (!tune.EmptyColumns())
				SetTitleRow(sh, tune.getColumns(), title_frmt);
		}
		file->save(outFile.c_str());
		//уничтожаем файл
		file->release();
	}
}