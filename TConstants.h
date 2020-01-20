#ifndef  TCONSTANTS_H_
//В данном файле указываются все общие константы для работы программы:
#define TCONSTANTS_H_

#include <string>
#include <vector>
#include <utility>

//область видимости для константных выражений типа enum class:
namespace NS_Const
{
	using std::string;

	const int EmptyType = -1;

	//порядок указан такой же, как и файле настроек
	enum class TuneField {
		Empty = 0,
		//заголовки нгрупп настроек
		Shared, Paths, DataBase, Report, SqlParams, Columns, Block_End,
		//отсчет для начала параметров:
		Start_Shared_Index,
		//общие настройки отчетов:
		AddDateToOutFileName, AddDateToSheetName, AddDateToOutPath,
		//настроки путей расположения файлов
		MainPath, ConfigPath, ConfigFileExt, SqlPath, SqlFileExt, TemplatePath, TemplateFileExt,
		OutDirectory, OutFileName, 
		End_Shared_Index,
		Start_Unq_Tune,
		//база данных
		UserName, Password, TNS,
		//отчет
		SheetName, TemplateName,
		//запрос
		SqlFirst, SqlFile, SqlText, DMLFile, DMLText, SqlParam, Column,
		SqlParamQuane, SqlParamType, SqlParamNote, SqlParamValue, UseSqlParser,
		End_Unq_Tune,
		Last
	};

	//параметры для настройки excel-файлов(для сравнения/импорта)
	enum class JsonParams {Null, False, True,
		//блок для основных объектов
		ObjBegin, DstFile, Cells, SrcFile, DataArr, Method, DB_Config, ObjEnd,
		//блок для данных в объекте Файл
		FileBegin, name, list_index, start_index, last_index, filter, FileEnd,
		//блок для данных в объекте Фильтр
		FilterBegin, column_index, value, FilterEnd,
		//блок для данных объекта Метод
		MethodBegin, code, color_if_found, color_not_found, MethodEnd,
		//блок данных для объекта Колонка
		CellsBegin, dst_index, dst_insert_index, src_param_index, src_val_index, CellsEnd,
		Last
	};

	//методы обработки excel-файлов на основании Json-параметров
	enum class JSonMeth { Null, CompareColor, CompareIns, GetFromDB, SendToDB, Last };

	//Типы данных для параметров в запросах:
	enum class DataType { ErrorType = 0, String, Integer, Double, Date, Boolean, Last };

	//константы для формата файлов excel
	enum class TExclBaseTune { Empty = EmptyType, xlt, xls, xlsx, DefExt, DefName, DefSh, PageDelimiter, Last };

	//константы для ограничений excel-документов
	//http://www.excelworld.ru/publ/help/char_and_rest/sheet_char_and_rest/37-1-0-99
	enum TExclConstraint {
		xls_max_col = 256,
		xlsx_max_col = 16385,
		xls_max_row = 65536,
		xlsx_max_row = 1048577
	};

	//операторы sql:
	enum class TSql { Empty = EmptyType, With, Select, From, Where, Order, Group, As, And, Or, EOC, D4L, Last };

	//контрольные символы:
	enum class CtrlSym { 
		Empty = EmptyType, EOL, Null, Space, NL, EndCommand, semicolon, colon,  EndCol, point, 
		lbkt, rbkt, qlbkt, qrbkt, crwn, quotes, Tab, dash, quane, rangle, langle,
		//используется для определения комнтариев
		dies_comment, minus_comment, dash_comment,
		Last };

	//константы для выбора отчета:
	enum class ReportCode: int
	{
		Empty = EmptyType,
		RIB_DOCS_FOR_PERIOD = 0,//выборка документов из базы РИБ (Фоминых)
		DOCS_MF_SF_FOR_PERIOD,//выборка документов для МФ и СФ ОраБанк (Кривошеева)
		REPAYMENT_FOR_DATE,//гашение крелитов СФ + МФ (Скачкова)
		POTREB_CRED_BY_FILE,//потребительские крелиты МФ по файлу (Борисова)
		CRED_CASE_MF,//кредитный портфель МФ (Ермакова)
		NBKI_NP,//данные по Физ. лицам(Борисова)
		NBKI_JP,//данные по Юр. лицам(Борисова)
		CLOSE_DAY,//закрытие баланса/месяца
		NBKI_APPLY,//обновление данных по НБКИ (Борисова) когда меняем статус с 3 на 0
		BALANCE_LIST,//ведомость остатков МФ (Ермакова)
		FULL_CRED_REPORT,//полный кредитный портфель (Ермакова) большой отчет по файлу
		LOAD_FROM_FILE,//загрузка документов из excel/xml/txt-файла
		FILE_COMPARE,//сравнение файлов excel
		Last
		};

	//базовые операции со строками:
	//преобразование в нижний регистр:
	string LowerCase(const string& str);
	string UpperCase(const string& str);
	//убираем из строки служебные символа:
	//void DeleteServiceSymb(string& str);
	//функция убирающая пробелы из начали и конца строки:
	void Trim_Left(string& str);
	void Trim_Right(string& str);
	void Trim(string& str);

	//базовый интерфейс для константных значений:
	template <typename T, T min_val, T max_val>
	class TConstant
	{
	private:
		int val;//значение параметра
		//суммирование:
		TConstant<T, min_val, max_val>& operator+=(int x) { val += x; return *this; }
	protected:
		//Инициализация:
		virtual void Init(const T& def_val = min_val) noexcept(true);
		//нахождение в диапазоне
		virtual bool inRange(const T& a = min_val, const T& b = max_val) const noexcept(true) { return (Value() > a and Value() < b); };
		//проверка валидности
		virtual bool isValid(const T& a = min_val, const T& b = max_val, bool exit_on_error = false) const noexcept(false);
		//установка значения из типа:
		void setValue(const T& x);
		//инициализация
		void setValue(int x);
	public:
		explicit TConstant<T, min_val, max_val>(int x) { setValue(x); Init(); }
		explicit TConstant<T, min_val, max_val>(const T& x = min_val) { setValue(x); Init(); }
		//проверка валидности:
		virtual bool isValid(bool exit_on_error = false) const noexcept(false) { return isValid(min_val, max_val, exit_on_error); }
		//перевод значения в строку
		virtual string toStr() const = 0;
		//перевод значения в число
		virtual int toInt() const { return val; }
		//полечение значения нужного типа:
		T Value() const { return T(val); }
		//сравнение значения со строкой
		virtual bool operator==(const string& str) const { return toStr() == str; }
		//получение следующего элемента:
		virtual TConstant<T, min_val, max_val>& Next(bool exit_on_err = true) noexcept(false);
		//проверка на пустоту:
		virtual inline bool isEmpty() const { return Value() == min_val; };
		//опреция присвоения:
		TConstant<T, min_val, max_val>& operator=(const T& x);
		//оператор сравнения с типом:
		virtual bool operator<(const T& x) const { return Value() < x; }
		virtual bool operator>(const T& x) const { return !(operator==(x) or operator<(x)); }
		virtual bool operator==(const T& x) const { return Value() == x; }
		virtual bool operator!=(const T& x) const { return !operator==(x); }
		virtual bool operator<=(const T& x) const { return operator<(x) or operator==(x); }
		//операция сравнения между собой:
		bool operator<(const TConstant<T, min_val, max_val>& x) const { return val < x.val; }
	};

	//псевдонимы типов:
	using TF_Const = TConstant<TuneField, TuneField::Empty, TuneField::Last>;
	using DF_const = TConstant<DataType, DataType::ErrorType, DataType::Last>;
	using EBT_Const = TConstant<TExclBaseTune, TExclBaseTune::Empty, TExclBaseTune::Last>;
	using RC_Const = TConstant<ReportCode, ReportCode::Empty, ReportCode::Last>;
	using SQL_Const = TConstant<TSql, TSql::Empty, TSql::Last>;
	using CS_Const = TConstant<CtrlSym, CtrlSym::Empty, CtrlSym::Last>;
	using JS_Const = TConstant<JsonParams, JsonParams::Null, JsonParams::Last>;
	using JS_Meth = TConstant<JSonMeth, JSonMeth::Null, JSonMeth::Last>;


	//Поля из файла настроек:
	class TConstField : public TF_Const
	{
	public:
		explicit TConstField(const TuneField& x) : TF_Const(x) { }
		explicit TConstField(int x) : TF_Const(x) { }
		//проверка содержания параметра в строке:
		virtual bool StrInclude(const string& str) const;
		//перевод значения в строку
		static string asStr(const TuneField& code);
		virtual string toStr() const { return asStr(Value()); };
		//получение идентификатора настройки по ее коду:
		static TuneField getIDByCode(const string& code, const TuneField& bval, const TuneField& eval);
		//операция присвоения:
		TConstField& operator=(const TuneField& x) { TF_Const::operator=(x); return *this; }
		friend bool operator==(const string& str, const TConstField& val) { val.operator==(str); }
	};

	//Поля типов данных:
	class TConstType : public DF_const
	{
	public:
		explicit TConstType(const DataType& x) : DF_const(x) {}
		explicit TConstType(int x) : DF_const(x) {};
		explicit TConstType(const string& str);
		//перевод значения в строку
		virtual string toStr() const;
		//операция присвоения:
		TConstType& operator=(const DataType& x) { DF_const::operator=(x); return *this; }
	};

	/*
	class TConstTag : public Tag_Const
	{
	public:
		explicit TConstTag(const Tags& x) : Tag_Const(x) { }
		explicit TConstTag(int x) : Tag_Const(x) { }
		//перевод значения в строку
		virtual string toStr() const;
		//операция присвоения:
		TConstTag& operator=(const Tags& x) { Tag_Const::operator=(x); return *this; }
	};
	/**/
	//Поля для работы с Excel:
	//класс для работы с настройками файла:
	class TConstExclTune : public EBT_Const
	{
	public:
		//инициализация
		explicit TConstExclTune(const TExclBaseTune& x) : EBT_Const(x) { }
		explicit TConstExclTune(int x) : EBT_Const(x) { }
		//преобразование в строку
		static string asStr(const TExclBaseTune& val) noexcept(true);
		//преобразование в строку
		virtual string toStr() const { return asStr(Value()); };
		//операция присвоения
		TConstExclTune& operator=(const TExclBaseTune& x) { EBT_Const::operator=(x); return*this; }
		//получение расширения файла:
		static string getFileExtention(const string& val) noexcept(true);
		//проверка валидности расширения для excel-файла:
		static bool isValidExtensions(const string& val) noexcept(true);
	};

	//класс для работы с константными полями:
	class TConstReportCode : public RC_Const
	{
	public:
		explicit TConstReportCode(const ReportCode& x) : RC_Const(x) { }
		explicit TConstReportCode(int x) : RC_Const(x) { }
		TConstReportCode& operator=(const ReportCode& x) { RC_Const::operator=(x); return *this; }
		//получение кода отчета:
		string toStr() const;
		//получение наименования отчета:
		string getName() const;
		//получение идентификатора отчета по коду:
		static ReportCode getIDByCode(const string& code, const ReportCode& bval, const ReportCode& eval);
		friend bool operator==(const string& str, const TConstReportCode& val) { return val.operator==(str); }
	};

	//класс группы символов
	class TSymGroup
	{
	private:
		using Syms = std::pair<char, char>;
		using SymsArr = std::vector<Syms>;
		SymsArr arr;//парные символы для проверки корректности их закрытия
	public:
		//инициализация
		explicit TSymGroup(const SymsArr& val) : arr(val) {};
		TSymGroup(bool set_default);
		//добавление символа в группу:
		TSymGroup& operator+(const Syms& pair_val) noexcept(false) { arr.push_back(pair_val); return *this; };
		//проверка на корректность найденных данных:
		bool IsCorrectSym(const std::string& str, std::size_t pos, const std::size_t cnt = 1) const noexcept(false);
	};

	//класс обработки служебных символов:
	class TConstCtrlSym : CS_Const
	{
	public:
		explicit TConstCtrlSym(const CtrlSym& x) : CS_Const(x) { }
		explicit TConstCtrlSym(int x) : CS_Const(x) { }
		TConstCtrlSym& operator=(const CtrlSym& x) { CS_Const::operator=(x); return *this; }
		//общее преобразование в строку
		static string asStr(const CtrlSym& val);
		//общее преобразование в символ
		static char asChr(const CtrlSym& val) { return asStr(val)[0]; };
		//преобразование в строку
		string toStr() const { return asStr(Value()); };
		//преобразование в символ
		char toChar() const { return toStr()[0]; }
		string operator+(const string& str) const noexcept(true);
		//дружественные функции работы со строками:
		friend string operator+(const string& str, const TConstCtrlSym& ch);
		friend string operator+(const TConstCtrlSym& ch, const string& str);
	};

	//класс для обрабоки sql-команд:
	class TConstSql : public SQL_Const
	{
	public:
		explicit TConstSql(const TSql& x) :SQL_Const(x) { }
		explicit TConstSql(int x) : SQL_Const(x) { }
		TConstSql& operator=(const TSql& x) { SQL_Const::operator=(x); return *this; }
		string toStr() const;
		bool MustFound() const;
		//функция получения разделителя по наименованию
		TSql GetDelimeter() const noexcept(false);
		TConstSql GetDelimeterAsObj() const noexcept(false) { return TConstSql(GetDelimeter()); };
		//функция проверки корректности указанного разделителя:
		bool CorrectDelimeter(const std::string& d) const noexcept(true);
		//функция проверяющая возможность использования скобок в запросе:
		bool CanUseBrkt() const noexcept(true) { return Value() == TSql::Where; };
		//функция определяющая закрывающий элемент по команде:
		string getClosedElem() const noexcept(false);
		//оператор сравнения со строкой:
		bool operator==(const string& str) const noexcept(true) { return toStr() == str; };
		bool operator==(const TConstSql& val) const noexcept(true) { return Value() == val.Value(); }
		TConstSql operator+(int x) const noexcept(true);
		//вывод данныъ TCtrlSym в поток
		friend std::ostream& operator<<(std::ostream& stream, const TConstSql& val) noexcept(false);
	};

	//класс для сравнения со строками в качестве предиката:
	class TTrimObj
	{
	private:
		string symb;
	protected:
		string Symbols() const noexcept(true) { return symb; }
	public:
		TTrimObj(const string& arr) : symb(arr) {};
		TTrimObj(const TConstSql& title);
		//манипуляция с символами:
		bool operator()(const char& ch) const;
	};

	string& operator<<(string& str, const JsonParams& param);
	
	//класс для обработки Json-констант:
	class TConstJson : public JS_Const
	{
	private:
		static bool inRange(const JsonParams& val, const JsonParams& b, const JsonParams& e) noexcept(true);
	public:
		//инициализация
		explicit TConstJson(const JsonParams& x) : JS_Const(x) {}
		explicit TConstJson(int x) : JS_Const(x) {}
		//присвоение:
		TConstJson& operator=(const JsonParams & x) { JS_Const::operator=(x); return *this; }
		//получение строкового значения по идентификтору:
		static string asStr(const JsonParams& val) noexcept(true);
		static bool isObjectTag(const JsonParams& val) noexcept(true) { return inRange(val, JsonParams::ObjBegin, JsonParams::ObjEnd); }
		static bool isFileTag(const JsonParams& val) noexcept(true) { return inRange(val, JsonParams::FileBegin, JsonParams::FileEnd); }
		static bool isFilterTag(const JsonParams& val) noexcept(true) { return inRange(val, JsonParams::FilterBegin, JsonParams::FilterEnd); }
		static bool isCellTag(const JsonParams& val) noexcept(true) { return inRange(val, JsonParams::CellsBegin, JsonParams::CellsEnd); }
		static bool isMethTag(const JsonParams& val) noexcept(true) { return inRange(val, JsonParams::MethodBegin, JsonParams::MethodEnd); }
		static bool isTag(const JsonParams& val) noexcept(true);
		static string Concate(const std::vector<JsonParams>& arr) noexcept(true);
		string toStr() const noexcept(true) { return asStr(Value()); }
		//сравнение:
		bool operator==(const string& str) const noexcept(true) { return toStr() == str; }
		bool operator==(const JsonParams& val) const noexcept(true) { return Value() == val; }
		friend string& operator<<(string& str, const JsonParams& param);
	};

	class TConstJSMeth : public JS_Meth
	{
	public:
		explicit TConstJSMeth(const JSonMeth& x) : JS_Meth(x) {}
		explicit TConstJSMeth(int x) : JS_Meth(x) {}
		static string asStr(const JSonMeth& val) noexcept(true);
		string toStr() const noexcept(true) { return asStr(JS_Meth::Value()); };
		TConstJSMeth& operator=(const JSonMeth& x) noexcept(true) { JS_Meth::operator=(x); return *this; }
/*
		TConstJSMeth& operator=(int x) noexcept(true) { JS_Meth::setValue(x); return *this; }
/**/

		};

}

#endif // ! TCONSTANTS_H_

