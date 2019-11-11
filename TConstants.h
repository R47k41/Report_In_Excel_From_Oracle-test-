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
		DataBase, Report, SqlParams, Columns, Block_End,
		//база данных
		UserName, Password, TNS,
		//отчет
		OutDirectory, OutFileName, SheetName, AddDateToName, TemplateName,
		//запрос
		SqlFile, SqlText, SqlParam, Column,
		SqlParamQuane, SqlParamType, SqlParamNote, SqlParamValue,
		Last
	};

	//Типы данных для параметров в запросах:
	enum class DataType { ErrorType = 0, String, Integer, Double, Date, Last };

	//константы для формата файлов excel
	enum class TExclBaseTune { Empty = EmptyType, xlt, xls, xlsx, DefExt, DefName, DefSh, Last };

	//операторы sql:
	enum class TSql { Empty = EmptyType, With, Select, From, Where, Order, Group, As, And, Or, EOC, D4L, Last };

	//контрольные символы:
	enum class CtrlSym { 
		Empty = EmptyType, EOL, Null, Space, NL, EndCommand, semicolon, colon,  EndCol, 
		lbkt, rbkt, crwn, quotes, Tab, dash, quane, rangle, langle,
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
		NBKI,//данные для отправки в НБКИ (Борисова) все в один файл
		NBKI_APPLY,//обновление данных по НБКИ (Борисова) когда меняем статус с 3 на 0
		BALANCE_LIST,//ведомость остатков МФ (Ермакова)
		FULL_CRED_REPORT,//полный кредитный портфель (Ермакова) большой отчет по файлу
		LOAD_FROM_FILE,//загрузка документов из excel/xml/txt-файла
		FILE_COMPARE,//сравнение файлов excel
		Last
		};

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


	//Поля из файла настроек:
	class TConstField : public TF_Const
	{
	public:
		explicit TConstField(const TuneField& x) : TF_Const(x) { }
		explicit TConstField(int x) : TF_Const(x) { }
		//проверка содержания параметра в строке:
		virtual bool StrInclude(const string& str) const;
		//перевод значения в строку
		virtual string toStr() const;
		//операция присвоения:
		TConstField& operator=(const TuneField& x) { TF_Const::operator=(x); return *this; }
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
		virtual string toStr() const;
		TConstExclTune& operator=(const TExclBaseTune& x) { EBT_Const::operator=(x); return*this; }
	};

	//класс для работы с константными полями:
	class TConstReportCode : public RC_Const
	{
	public:
		explicit TConstReportCode(const ReportCode& x) : RC_Const(x) { }
		explicit TConstReportCode(int x) : RC_Const(x) { }
		TConstReportCode& operator=(const ReportCode& x) { RC_Const::operator=(x); return *this; }
		string toStr() const;
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
		string toStr() const;
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

}

#endif // ! TCONSTANTS_H_

