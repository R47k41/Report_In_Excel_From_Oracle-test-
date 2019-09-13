#ifndef TEXCEL_H_
#define TEXCEL_H_
#include <string>
#include <vector>
#include "Logger.h"
#include "libxl.h"

namespace NS_Excel
{
	using std::string;
	using libxl::Book;
	using libxl::Font;
	using libxl::Format;
	using libxl::Sheet;
	enum class TFormatTune {xlt, xls, xlsx, DefExt, DefName, DefSh};
	//преобразование расширения в строку
	string getTuneFormat(const TFormatTune& val);
	//проверка является ли файл шаблоном:
	bool isTemplate(const string& str);
	//функция записи данных из контейнера для страницы(если не указан индекс - берем активную страницу):
	//template <template <class TItem> class TArr>
	//bool WriteData<TItem>(const TArr<TItem>& arr);

	using TDataType = libxl::CellType;
	//enum class TDataFormat { Default = 0, AsString, AsInteger, AsDate, AsBool };

	//класс-надстройка для создания интерфейса для работы с excel-файлом
	class TExcel
	{
	private:
		enum DefVal {Empty = -1, MainIndx = 0, TitleIndx = 1,
			FontSize = 10, };
		Book* file;//excel-файл
		Sheet* active_sh;//активный рабочий лист
		//Format* main_frmt;//основной формат
		string name;//имя выходного файла
		std::vector<TDataType> OrdColumnsFormat;
		//запрещаем инициализацию и присвоение
		TExcel(const TExcel& exl);
		TExcel& operator=(const TExcel& exl);
		//функция полуения имени выходного файла по умолчанию(чтобы можно было менять на что-то более разумное):
		virtual void setDefOutFileName(void);
		//функция создания шрифта по умолчанию:
		virtual void setDefFont(void) noexcept(false);
		//функция создания формата по умолчанию:
		virtual void setDefFormat(void) noexcept(false);
		//функция установки шрифта для заголовка:
		virtual void setTitleFont(Font* src = 0);
		//функция установки формата для заголовка:
		virtual void setTitleFormat(Format* src = 0);
		//функция очистки данных по excel-файлу
		virtual void clear(void);
		//функция получения первой/последней строки с данными:
		int getUsedRow(Sheet* sh, bool last = false) const;
		//функция получения первой/последней ячейки с данными
		int getUsedCell(Sheet* sh, bool last = false) const;
		//функция заполнения формата полей по шаблону(В шаблоне формат должен быть заполнен корректно):
		//указывается лист и строка заголовка, после которой считываем формат ячеек:
		void FillColumnsFormat(int TitleRow = 1, int ShIndex = DefVal::Empty);
	public:
		//инициализация - как открытие файла, если файла нет - создаем
		//для инициализации используется шаблон отчета,
		//чтобы распознать форматы строк и вставляемые в них значения
		//если файл не указан - вставкаданных идет по умолчанию, как Общий формат
		TExcel(const string& tmp_name = "", const string& out_name = "", bool crt_active_sh = false);
		//деинициализация
		virtual ~TExcel(void) { clear(); };
		//является ли книга шаблоном:
		bool isTemplate(void) const { return file->isTemplate(); };
		//копирование для шрифта
		virtual void copyFont(int index, Font* src);
		//копирование для формата:
		virtual void copyFormat(int index, Format* src);
		//функция получения числа страниц:
		int SheetCount(void) const { return (file ? file->sheetCount() : DefVal::Empty); }
		//функция добавления страницы и делаем ее активной:
		bool AddSheet(const string& name, bool set_as_active = false);
		//функция получения активной страницы:
		int ActiveSheet(void) const { return file ? file->activeSheet() : DefVal::Empty; }
		//функция установки активной страницы:
		void setActiveSheet(int index);
		//функция получения первой заполненой строки:
		int getFirstUsedfRow(int SheetIndex) const { return getUsedRow(active_sh, false); };
		//функция получения последней заполненой строки:
		int getLastUsedRow(int SheetIndex) const { return getUsedRow(active_sh, true); };
		//функция получения первой ячейки с данными в строке
		int getFirstUsedCell(int SheetIndex) const { return getUsedCell(active_sh, false); };
		//функция получения последней ячейки с данными в строке
		int getLastUsedCell(int SheetIndex) const { return getUsedCell(active_sh, true); };
		inline bool WriteBool(bool val, int row, int col) { return active_sh->writeBool(row, col, val); };
		inline bool WriteBlank(int row, int col) { return active_sh->writeBlank(row, col, file->format(DefVal::MainIndx)); };
		inline bool WriteNum(double val, int row, int col) { return active_sh->writeNum(row, col, val); };
		inline bool WriteSrt(const string& str, int row, int col) { return active_sh->writeStr(row, col, str.c_str()); };
		//функция чтения данных:
		inline bool ReadBool(int row, int col) const { return active_sh->readBool(row, col); };
		inline double ReadNum(int row, int col) const { return active_sh->readNum(row, col); };
		inline string ReadStr(int row, int col) const { return string(active_sh->readStr(row, col)); };
	};
};
#endif