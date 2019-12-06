//#pragma once
#ifndef TUNE_PARAM_H_
#define TUNE_PARAM_H_
//модуль предназначен для описания класса работающего с файлом настроек
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <utility>
#include "TConstants.h"

//обаласти пространства имен для работы с настройками
namespace NS_Tune
{
	using std::string;
	using std::ifstream;
	using std::pair;
	using std::vector;
	using std::set;
	using find_fnc = size_t (string::*)(const string& s, size_t pos) const;
	using NS_Const::TuneField;
	using NS_Const::DataType;
	using NS_Const::CtrlSym;
	using NS_Const::TConstField;
	using NS_Const::TConstType;
	using NS_Const::TConstCtrlSym;


	class TBaseParam
	{
	private:
		string src_data;//исходные данные
	protected:
		string value;//значение параметра
		size_t get_pos_in_src(const string& substr, const size_t beg = 0, find_fnc ff = &string::find) const;
		size_t get_pos_in_src(const TConstField& substr, const size_t beg = 0, find_fnc ff = &string::find) const;
		size_t get_pos_in_src(const CtrlSym& substr, const size_t beg = 0, find_fnc ff = &string::find) const;
		//получение значения из строки данных:
		virtual string Get_Str_Val(size_t pos, const CtrlSym& b_delimeter, const CtrlSym& e_delimeter,
			bool from_end = false) const;
		//по умолчанию выполняет последовательный поиск в строке:
		virtual string Get_TuneField_Val(const TConstField& param, const CtrlSym& b_delimeter,
			const CtrlSym& e_delimeter, bool pose_from_end = false) const;
		void setSrcData(const string& val) { src_data = val; };
		virtual void setValue(const TConstField&, const CtrlSym& open_val, const CtrlSym& close_val) = 0;
	public:
		explicit TBaseParam(const string& full_str) : src_data(full_str) {}
		TBaseParam(const TBaseParam& x) : src_data(x.src_data), value(x.value) {}
		virtual bool isEmpty() const { return src_data.empty(); }
		virtual string toStr() const { return src_data; }
		virtual string Value() const { return value; }
		size_t srcSize() const { return src_data.size(); }
		string srcSubStr(size_t posb, size_t pose) const { return src_data.substr(posb, pose - posb); }
		size_t srcFind(const string& substr, size_t pos) const { return src_data.find(substr, pos); }
		//прямая установка значения
		virtual void Value(const string& val) { value = val; }
	};

	//класс параметров из строки:
	class TStringParam: public TBaseParam
	{
	private:
		TConstField param;//параметр
	protected:
		virtual void setValue(const TConstField&, const CtrlSym& open_val, const CtrlSym& close_val);
	public:
		TStringParam() : TBaseParam(string()), param(TuneField::Empty) {};
		explicit TStringParam(const TConstField& val) : TBaseParam(string()), param(val) {}
		//прямая инициализация значениями
		TStringParam(const TuneField& tune_field, const string& str, bool parse_val = true);
		//инициализация строкой с извлечением значения параметра
		TStringParam(const string& full_data, const TuneField& tune_field,
			const CtrlSym& open_val = CtrlSym::quotes, const CtrlSym& close_val = CtrlSym::quotes);
		TStringParam(const TStringParam& val): TBaseParam(val), param(val.param) {}
		virtual bool isEmpty() const { return (param.isEmpty() or value.empty()); }
		virtual string ParamName() const { return param.toStr(); }
		TuneField Param() const { return param.Value(); }
		bool operator<(const TuneField& x) const { return param < x; }
		bool operator>(const TuneField& x) const { return param > x; }
		bool operator==(const TuneField& x) const { return param == x; }
		string toStr(bool use_quotes = false) const;
		bool operator<(const TStringParam& x) const { return param < x.param; }
		bool operator==(const TStringParam& x) const { return operator==(x.Param()); }
		//формирование пары для заполнения массива
		//virtual pair<TuneField, TStringParam> toPair() const { return std::make_pair(param.Value(), *this); };
	};

	//класс для вложенных параметров:
	class TSubParam: public TBaseParam
	{
	private:
		const int EmptyID = 0;
		int id;//номер параметра
		TConstType type;//тип параметра
		string comment;//коментарий
	protected:
		virtual void setValue(void);
		virtual void setValue(const TConstField&, const CtrlSym&, const CtrlSym&);
	public:
		explicit TSubParam(int val) : TBaseParam(string()), id(val), type(DataType::ErrorType), comment() {}
		explicit TSubParam(const string& str);
		TSubParam(const TSubParam& x) : TBaseParam(x), id(x.id), type(x.type), comment(x.comment) {}
		//получение ID параметра
		int ID() const { return id; }
		//получение типа данных в виде строки
		string Type() const { return type.toStr(); }
		//получение типа данных в виде объекта
		DataType DataType() const { return type.Value(); }
		//получение коментария
		string Comment() const { return comment; }
		//получение кода параметра:
		string getCode() const;
		//установка значения пользователем:
		bool setValByUser();
		//значение строкой:
		void show() const;
		//оператор < для сортировки:
		bool operator<(const TSubParam& val) const { return id < val.id; }
		bool operator<(const int& x) const { return id < x; }
		bool operator>(const int& x) const { return id > x; }
		bool operator==(const int& x) const { return id == x; }
	};

	using TSubParamType = int;

	using TField = std::pair<TuneField, TStringParam>;
	using TParam = std::pair<int, TSubParam>;
	using StrArr = std::vector<string>;
	using FileParam = std::pair<string, string>;
	using TFields = std::vector<TStringParam>;
	using ParamArr = std::vector<TSubParam>;
	using TuneRange = std::pair<TuneField, TuneField>;
	using RangeField = std::set<TConstField>;

	//Настройки пользователя:
	//базовый класс
	class TSimpleTune
	{
	protected:
		TFields fields;//поля настроек с их значениями
		enum class Types { Config, Sql, Template, OutPath, OutName, OutSheet };
		enum class TRead { Section, TuneVal };
	private:
		TSimpleTune& operator=(const TSimpleTune& v);
	protected:
		//функция получения списка файлов в каталоге:
		static vector<string> getFileLst(const string& file_dir, const string& file_ext = "", bool use_sort = true) noexcept(false);
		//функция добавления разделителя к имени:
		static void AddDelimeter(string& str, const char delim) noexcept(false);
		//функция получения пути/директории из настроек:
		string getPathByCode(const Types& code) const noexcept(true);
		//функция получения пути по коду:
		FileParam getFileParamByCode(const Types& code) const noexcept(true);
		//функция получения списка файлов в каталоге из настроек:
		vector<string> getFileLst(const Types& code, bool use_sort = true) const noexcept(false);
		//установка формата для вывода даты в потоке:
		//основано на библиотеки boost\date_time при использовании date_facet
		//соответствия форматов смотри 
		//https://www.boost.org/doc/libs/1_49_0/doc/html/date_time/date_time_io.html
		static bool set_date_format(std::ostream& stream, const string& fromat) noexcept(true);
		static string cur_date_to_string_by_format(const string& format) noexcept(false);
		//функция получения имени по коду настроек(использование формата даты)
		string AddCurDate2Name(const Types& code) const noexcept(false);
		//функция добавления поля настройки:
		bool AddField(const TStringParam& val) noexcept(true);
		//функция установки значения параметра:
		bool setFieldVal(const TuneField& key, const string& val) noexcept(false);
		//функция получения ограничений на получение списка искомых параметров:
		virtual TuneRange getTuneRange(const TRead& x) const noexcept(true) = 0;
		//функция получения списка параметров у указанном диапазоне
		static RangeField getTuneField4Range(const TuneRange& range);
		//пропуск секции - считывание файла до позиции [END]:
		static void skip_block(ifstream& file, const string& end_block);
		//функция считывания секции настроек из файла по коду
		virtual void Read_Section(ifstream& file, const string& code) = 0;
		//функция чтения занчений настроек из файла:
		virtual void Read_Tune_Val(ifstream& file);
		//чтение настроек из файла:
		virtual void ReadFromFile(const string& file);
		//template <typename KeyType, typename ValType>
		//ValType& getValueByID(const KeyType& par_ID, set<ValType>& arr) noexcept(false);
		//шаблон функции для получения значения поля по его уникальному коду/ID
		template <typename KeyType, typename ValType>
		static ValType& getElementByID(const KeyType& par_ID, vector<ValType>& arr)  noexcept(false);
		template <typename KeyType, typename ValType>
		static const ValType& getConstElementByID(const KeyType& par_ID, const vector<ValType>& arr)  noexcept(false);
		/*
		template <typename KeyType, typename ValType>
		static const ValType& NS_Tune::TSimpleTune::getValueByID(const KeyType& par_ID, const set<ValType>& arr) noexcept(false);
		/**/
		//функция получения значения для параметра:
		//получение имени исходящего файла/страницы/пути:
		string getNameByCode(const Types& code) const noexcept(true);
		//получение полного имени файла по коду:
		string getFullFileName(const Types& code, bool only_path = false) const noexcept(true);
	public:
		TSimpleTune() {}
		//инициализация другой структурой
		explicit TSimpleTune(const TSimpleTune& v) : fields(v.fields) {}
		explicit TSimpleTune(const TFields& v) : fields(v) {}
		~TSimpleTune() {}
		//проверка на пустоту для продолжения работы:
		virtual bool Empty() const { return fields.empty(); }
		//получение значения:
		string getFieldValueByCode(const TuneField& code, bool exit_on_er = true) const noexcept(true);
		//функция отображения списка настроек:
		virtual void show_tunes(void) const;
		//если данного параметра не найдено берет наименование страницы по умолчанию
		string getOutSheet() const noexcept(false) { return getFullFileName(Types::OutSheet); }
	};
	
	//общие настройки пользователя:
	class TSharedTune : public TSimpleTune
	{
	private:
		string  main_code;//код основной настройки
		TSharedTune& operator=(const TSharedTune& v);
		//получение имени секции индивидуальных настроек отчета
		string getSectionName() const noexcept(true);
		virtual TuneRange getTuneRange(const TRead& x) const noexcept(true);
		//считывание секции
		virtual void Read_Section(ifstream& file, const string& code);
	public:
		//инициализация
		TSharedTune(const string& file, const string& code);
		TSharedTune(const TFields& v, const string& file, const string& code);
		TSharedTune(const TSharedTune& v, const string& file, const string& code);
		~TSharedTune() {}
		//код основной настройки
		string getMainCode(bool use_brkt = true) const;
		//получение значения основной настройки:
		string getMainCodeVal() const { return getFieldValueByCode(TuneField::MainPath); }
		//проверка пустоты:
		bool Empty() const { return TSimpleTune::Empty(); }
		//получение списка файлов:
		vector<string> getConfFileLst(bool use_sort = true) const noexcept(false) { return getFileLst(Types::Config, use_sort); }
		vector<string> getSqlFileLst(bool use_sort = true) const noexcept(false) { return getFileLst(Types::Sql, use_sort); }
		vector<string> getTemplFileLst(bool use_sort = true) const noexcept(false) { return getFileLst(Types::Template, use_sort); }
		//получение пути к файлу настроек:
		string getConfigPath() const noexcept(false) { return getFullFileName(Types::Config, true); }
		//получение имени исходящего файла:
		string getOutPath() const noexcept(false) { return getFullFileName(Types::OutPath, true); }
		string getOutFile() const noexcept(false) { return getFullFileName(Types::OutName, false); }
		//проверка существования директории - если не существует - создаем:
		static bool CheckPath(const string& dir, bool crt_if_not_found = true);
	};

	//Настройки пользователя для страницы отчета
	class TUserTune: public TSimpleTune
	{
	private:
		StrArr cols;
		ParamArr params;
		virtual TuneRange getTuneRange(const TRead& x) const noexcept(true);
		//функция считывания секции настроек из файла по коду
		void Read_Section(ifstream& file, const string& code);
		//функция считывания потоковых параметров:
		void Read_StreamData_Val(ifstream& file, const TuneField& stream_title, StrArr& str_arr);
		//функция считывания данных о параметрах запроса:
		void Read_Param_Val(ifstream& file);
		//функция чтения значений колонок
		void Read_Col_Val(ifstream& file);
		TUserTune(const TFields& v);
		TUserTune(const TUserTune& v);
		TUserTune& operator=(const TUserTune& v);
	public:
		//список настроек формируется из файла
		explicit TUserTune(const string& tunefile);
		//инициализация структурой настроек и новым файлом
		explicit TUserTune(const TSimpleTune& tune, const string& file);
		~TUserTune() {};
		//проверка на пустоту для полей:
		virtual bool EmptyFields(void) const { return TSimpleTune::Empty(); }
		//проверка пустой ли список колонок
		bool EmptyColumns(void) const { return cols.empty(); }
		//проверка на наличие параметров:
		bool EmptyParams(void) const { return params.empty(); }
		//проверка на пустоту для продолжения работы:
		bool Empty() const { return EmptyFields() && EmptyColumns(); }
		//получение значения для параметра по ID:
		string getParamValByID(int par_id, bool exit_on_er = true) const noexcept(false);
		//получение параметра:
		TSubParam getParamByID(int par_id, bool exit_on_er = true) const noexcept(false);
		//получение массива параметров:
		ParamArr getParams() const { return params; }
		//функция ролучения значения для колонки с указанным индексом:
		string getColValByIndx(int indx) const { return cols[indx]; }
		//получение списка колонок:
		StrArr getColumns(void) const { return cols; }
		//функция отображения списка колонок:
		void show_columns(void) const;
		//функция отображения параметров:
		void show_params() const;
		//функция получения числа колонок:
		size_t ColumnsCnt() const { return cols.size(); }
		//функция получения числа параметров:
		size_t ParamsCnt() const { return params.size(); }
		//получение имени файла запросов:
		string getSqlFile() const noexcept(false) { return getFullFileName(Types::Sql, false); }
		//получение имени шаблонного файла:
		string getTemplateFile() const noexcept(false) { return getFullFileName(Types::Template, false); }
	};
}

#endif TUNE_PARAM_H_