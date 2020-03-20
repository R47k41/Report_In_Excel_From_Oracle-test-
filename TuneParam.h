//#pragma once
#ifndef TUNE_PARAM_H_
#define TUNE_PARAM_H_
//модуль предназначен для описания класса работающего с файлом настроек
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <utility>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "TConstants.h"
#include "libxl.h"

//обаласти пространства имен для работы с настройками
namespace NS_Tune
{
	using std::string;
	using std::ifstream;
	using std::pair;
	using std::vector;
	using std::set;
	using std::size_t;
	using find_fnc = size_t (string::*)(const string& s, size_t pos) const;
	using TColor = libxl::Color;
	using NS_Const::TuneField;
	using NS_Const::DataType;
	using NS_Const::CtrlSym;
	using NS_Const::TConstField;
	using NS_Const::TConstType;
	using NS_Const::TConstCtrlSym;
	using NS_Const::TConstJson;
	using NS_Const::JsonParams;
	using NS_Const::JSonMeth;
	using NS_Const::JsonCellFill;
	using NS_Const::TConstJSCellFill;
	using NS_Const::TConstJSMeth;
	using NS_Const::JsonFilterOper;
	using NS_Const::TConstJSFilterOper;
	using boost::property_tree::ptree;


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
		void show(std::ostream& stream = std::cout) const;
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
		enum class Types { Config, SubConfig, SQL, DQL, DML, Template, OutPath, OutName, OutSheet };
		enum class TRead { Section, TuneVal };
	private:
		TSimpleTune& operator=(const TSimpleTune& v);
	protected:
		//функция добавления разделителя к имени:
		static void AddDelimeter(string& str, const char delim) noexcept(false);
		//функция получения пути/директории из настроек:
		string getPathByCode(const Types& code) const noexcept(true);
		//функция получения пути по коду:
		FileParam getFileParamByCode(const Types& code) const noexcept(true);
		//функция получения списка файлов в каталоге из настроек:
		vector<string> getFileLst(const Types& code, bool use_sort = true) const noexcept(false);
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
		//функция получения значения для параметра:
		//получение имени исходящего файла/страницы/пути:
		string getNameByCode(const Types& code) const noexcept(true);
		//получение полного имени файла по коду:
		string getFullFileName(const Types& code, bool only_path = false) const noexcept(true);
		//инициализация настроек по имени файла:
		void Initialize(const string& file = string()) noexcept(true);
	public:
		TSimpleTune() {}
		//инициализация другой структурой
		explicit TSimpleTune(const TSimpleTune& v) : fields(v.fields) {}
		explicit TSimpleTune(const TFields& v) : fields(v) {}
		~TSimpleTune() {}
		//проверка на пустоту для продолжения работы:
		virtual bool Empty() const { return fields.empty(); }
		//получение значения:
		string getFieldValueByCode(const TuneField& code) const noexcept(true);
		//получение целочисленного значения по коду параметра:
		//если вернуло false - произошла ошибка при получении значения
		bool FieldValueAsInt(const TuneField& code, int& val) const noexcept(false);
		//функция отображения списка настроек:
		virtual void show_tunes(std::ostream& stream = std::cout) const;
		//если данного параметра не найдено берет наименование страницы по умолчанию
		string getOutSheet() const noexcept(false) { return getFullFileName(Types::OutSheet); }
		//функция получения имени выходного файла на основании страницы:
		string getOutFileBySheet() const noexcept(false);
		//функция проверки значения bool-параметра:
		bool useFlag(const TuneField& code) const noexcept(true);
		//функция получения списка файлов в каталоге:
		static vector<string> getFileLst(const string& file_dir, const string& file_ext = "", bool use_sort = true) noexcept(false);
	};
	
	//общие настройки пользователя:
	class TSharedTune : public TSimpleTune
	{
	private:
		string  main_code;//код основной настройки
		TSharedTune& operator=(const TSharedTune& v);
		//получение кода отчета от пользователя:
		static string getCodeFromtUI() noexcept(true);
		//получение имени секции индивидуальных настроек отчета
		string getSectionName() const noexcept(true);
		virtual TuneRange getTuneRange(const TRead& x) const noexcept(true);
		//считывание секции
		virtual void Read_Section(ifstream& file, const string& code);
	public:
		//инициализация
		explicit TSharedTune(const string& file, const string& code = string());
		TSharedTune(const TFields& v, const string& file, const string& code);
		TSharedTune(const TSharedTune& v, const string& file, const string& code);
		~TSharedTune() {}
		//код основной настройки
		string getMainCode(bool use_brkt = true) const;
		//получение значения основной настройки:
		string getMainPathVal() const { return getFieldValueByCode(TuneField::MainPath); }
		//проверка пустоты:
		bool Empty() const { return TSimpleTune::Empty(); }
		//получение списка файлов:
		vector<string> getConfFileLst(bool use_sort = true) const noexcept(false) { return getFileLst(Types::Config, use_sort); }
		vector<string> getSubConfigFileLst(bool use_sort = true) const noexcept(true) { return getFileLst(Types::SubConfig, use_sort); }
		vector<string> getSqlFileLst(bool use_sort = true) const noexcept(false) { return getFileLst(Types::DQL, use_sort); }
		vector<string> getTemplFileLst(bool use_sort = true) const noexcept(false) { return getFileLst(Types::Template, use_sort); }
		//получение пути к файлу настроек:
		string getConfigPath() const noexcept(false) { return getFullFileName(Types::Config, true); }
		//функция получения пути к файлам поднастроек:
		string getSubConfigPath() const noexcept(true) { return getFullFileName(Types::SubConfig, true); }
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
		TUserTune& operator=(const TUserTune& v);
		//получение массива sql-команд для выполнения:
		vector<string> getSQLFileLst(const Types& kind, bool use_sort = true) const noexcept(false);
		bool isEmptyByCode(const Types& kind) const noexcept(false);
	public:
		//список настроек формируется из файла
		explicit TUserTune(const string& tunefile);
		//инициализация структурой настроек и новым файлом
		explicit TUserTune(const TSimpleTune& tune, const string& file);
		TUserTune(const TUserTune& tune) : TSimpleTune(tune), cols(tune.cols), params(tune.params) {}
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
		void show_columns(std::ostream& stream = std::cout) const;
		//функция отображения параметров:
		void show_params(std::ostream& stream = std::cout) const;
		//функция получения числа колонок:
		size_t ColumnsCnt() const { return cols.size(); }
		//функция получения числа параметров:
		size_t ParamsCnt() const { return params.size(); }
		//получение имени файла запросов/команд:
		string getDQLFile() const noexcept(false) { return getFullFileName(Types::DQL, false); }
		string getDMLFile() const noexcept(false) {return getFullFileName(Types::DML, false); }
		//получение списка файлов sql-запросов:
		vector<string> getDQLFileLst(bool use_sort = true) const noexcept(false) { return getSQLFileLst(Types::DQL); }
		vector<string> getDMLFileLst(bool use_sort = true) const noexcept(false) { return getSQLFileLst(Types::DML); }
		//проверка указания параметров:
		bool isDQLEmpty() const noexcept(true) { return isEmptyByCode(Types::DQL); }
		bool isDMLEmpty() const noexcept(true) { return isEmptyByCode(Types::DML); }
		//получение имени шаблонного файла:
		string getTemplateFile() const noexcept(false) { return getFullFileName(Types::Template, false); }
	};

	//настройки для сравнения excel-файлов
	//настройки храняться в формате json-текста
	//описание класса для обработки данных в ячейках excel-файлов:
	//обработка будет происходить с использованием boost для json(он тупо быстрее и проще пишется)
	//ссыль: https://www.boost.org/doc/libs/1_52_0/doc/html/property_tree.html
	//базовый класс для индексов:
	class TIndex
	{
	private:
		size_t index;
		//установка индекса из узла json-дерева по указанному тегу/строке
		void setIndexFromJSon(const ptree::value_type& parent_node, const string& tagStr) noexcept(false);
	public:
		static const size_t EmptyIndex = 0;
		//получение строкового значения из json-файла:
		static string getStrValue(const ptree::value_type& parent_node, const JsonParams& tag) noexcept(true);
		static string getStrValue(const ptree& parent_node, const JsonParams& tag) noexcept(true);
		static bool setStrValue(const ptree& parent_node, const JsonParams& tag, const string& val) noexcept(true);
		static TColor getColorValue(const ptree::value_type& parent_node, const JsonParams& tag) noexcept(true);
		static JsonFilterOper getOperationCode(const ptree::value_type& parent_node, 
			const JsonParams& tag) noexcept(true);
		TIndex(size_t indx = EmptyIndex) : index(indx) {}
		TIndex(const TIndex& x) : index(x.index) {}
		TIndex(const ptree::value_type& parent_node, const string& tagStr) : index(EmptyIndex) { setIndexFromJSon(parent_node, tagStr); }
		TIndex(const ptree::value_type& parent_node, const NS_Const::JsonParams& tag) : index(EmptyIndex) { setByJSon(parent_node, tag); }
		inline bool isEmpty() const noexcept(true) { return index == EmptyIndex; }
		inline size_t get() const noexcept(true) { return index; }
		inline void set(size_t val) noexcept(false) { index = val; }
		void setByJSon(const ptree::value_type& parent_node, const NS_Const::JsonParams& tag) noexcept(true);
		bool operator==(size_t val) const noexcept(true) { return isEmpty() ? false: index == val; }
		bool operator==(const TIndex& val) const noexcept(false) { return isEmpty() ? false : index == val.index; }
		void show(std::ostream& stream = std::cout, const string& front_msg = "") const noexcept(false);
};

	//класс для обработки параметров листа excel-файла
	class TSheetData
	{
	private:
		TIndex index;//индекс листа excel-файла
		TIndex col_id;//колонка идентификатор данных
		TIndex first_row;//строка с которой начинается обработка
		TIndex last_row;//строка на которой заканчивается обработка
		//установка значений по json-дереву:
		void setData(const ptree::value_type& parent_node,
			const JsonParams& indx_tag = JsonParams::list_index,
			const JsonParams& col_tag = JsonParams::col_id,
			const JsonParams& first_row_tag = JsonParams::first_row,
			const JsonParams& last_row_tag = JsonParams::last_row) noexcept(true);
	public:
		//инициализация
		TSheetData(const ptree::value_type& parent_node);
		//деинициализация:
		~TSheetData() {}
		//провекра на пустоту:
		inline bool isEmpty() const noexcept(true) { return index.isEmpty(); }
		inline bool NoColID() const noexcept(true) { return col_id.isEmpty(); }
		inline bool NoFirstRowIndex() const noexcept(true) { return first_row.isEmpty(); }
		inline bool NoLastRowIndex() const noexcept(true) { return last_row.isEmpty(); }
		//установка и получение значений:
		inline size_t getListIndex() const noexcept(true) { return index.get(); }
		inline void setListIndex(size_t val) noexcept(true) { index.set(val); }
		//получение данных о колонке-идентификаторе:
		inline size_t getColID() const noexcept(true) { return NoColID() ? 1 : col_id.get(); }
		inline void setColID(size_t val) noexcept(true) { col_id.set(val); }
		//получение индекса строки отсчета:
		inline size_t getStartRow() const noexcept(true) { return NoFirstRowIndex() ? 1 : first_row.get(); }
		inline void setStartRow(size_t val) noexcept(true) { first_row.set(val); }
		//получение последней строки на странице
		inline size_t getLastRow() const noexcept(true) { return last_row.get(); }
		inline void setLastRow(size_t val) noexcept(true) { last_row.set(val); }
		//отображение значений:
		void show(std::ostream& stream = std::cout) const noexcept(true);

	};

	//класс для обработки значений колонок фильтрации:
	class TFilterData
	{
	private:
		TIndex col_indx;//индекс колонки, где проверяется условие
		TConstJSFilterOper operation;//операция при фильтрации
		string value;//условие/значение фильтрации
		bool str_flg;//признак, что значение - строка
		//установка признака значение - строка
		void setStrFlg() noexcept(true);
		//установка значений фильмтра из узла json-файла
		void setData(const ptree::value_type& parent_node,
			const JsonParams& col_tag = JsonParams::column_index,
			const JsonParams& oper_tag = JsonParams::operation,
			const JsonParams& val_tag = JsonParams::value) noexcept(true);
		//функция проверки является ли данное значение числом:
		bool isNumberValue(const char separate) const noexcept(true);
	public:
		//конструктор
		//TFilterData(std::pair<size_t, string> val) : col_indx(val.first), value(val.second) {}
		TFilterData(size_t indx, const NS_Const::JsonFilterOper& oper, const string& val): 
			col_indx(indx), operation(oper), value(val), 
			str_flg(true) {	setStrFlg(); }
		explicit TFilterData(const ptree::value_type& parent_node): 
			col_indx(TIndex::EmptyIndex), operation(NS_Const::JsonFilterOper::Equal), 
			value(), str_flg(true) { setData(parent_node); }
		//деструктор
		~TFilterData() {}
		//проверка пустого значения
		bool isEmpty() const noexcept(true) { return col_indx.isEmpty(); }
		//получение значение индекса
		size_t getColIndx() const noexcept(true) { return col_indx.get(); }
		//утсановка индекса
		void setColIndx(size_t val) noexcept(true) { col_indx.set(val); }
		//получение кода операции:
		NS_Const::JsonFilterOper getOperationCode() const noexcept(true) { return operation.Value(); }
		//получение наименования операции:
		string getOperationName() const noexcept(true) { return operation.toStr(); }
		//установка кода операции:
		void setOperationCode(const NS_Const::JsonFilterOper& code) noexcept(false) { operation = code; }
		//получение значения фильтра строкой
		inline string getValue() const noexcept(true) { return value; }
		//получение признака значения, как строки:
		inline bool ValueIsString() const noexcept(true) { return str_flg; }
		//установка значения для фильтра
		inline void setValue(const string& val) noexcept(true) { value = val; }
		//проверка прохождения фильмтра(возможно не нужна - будем сверять на месте):
		//bool operator==(const string& val) const noexcept(true);
		//bool operator==(const TFilterData& src) const noexcept(true) { return operator==(src.value); }
		//получение значений колонка/значение в виде пары:
		//pair<size_t, string> getPair() const noexcept(true) { return std::make_pair(getColIndx(), getValue()); }
		//вывод данных в поток:
		void show(std::ostream& stream = std::cout) const noexcept(true);
		//функция сравнения значений:(Не понятно почему, но VS выдает ошибки на шаблоны!!!)
		bool isFiltredDblValue(const double& val) const noexcept(true);
		bool isFiltredBoolValue(const bool& val) const noexcept(true);
		bool isFiltredIntValue(const int& val) const noexcept(true);
		bool isFiltredStrValue(const string& val) const noexcept(true);
	};

	using FilterArr = vector<TFilterData>;
	
	//структура общих данных для файлов сверки:
	class TShareData
	{
	private:
		string name;//имя файла
		vector<TSheetData> sh_params;//список параметров страницы
		FilterArr fltr;//данные для фильтрации
		//функция установки значений параметров по json-узлу
		//заполнение массива значений:
		template <typename Type>
		static void setArrayByJson(const ptree::value_type& node, const JsonParams& tag, 
			vector<Type>& arr) noexcept(false);
		void setData(const ptree::value_type& parent_node,
			const JsonParams& name_tag = JsonParams::name,
			const JsonParams& sheet_tag = JsonParams::Sheet,
			const JsonParams& fltr_tag = JsonParams::filter) noexcept(true);
	public:
		//инициализация именем файла(для открытия файла excel), номером листа(соответтствует ограничениям excel),
		//номер строки отсчета(сопостовимо с форматом excel) и массив фильтров значений
		//считывание из основного дерева:
		explicit TShareData(ptree& main_node, const string& main_path = "");
		//считывание из поддерева:
		explicit TShareData(const ptree::value_type& parent_node, const string& main_path = "");
		//деструктор
		~TShareData() {}
		//проверка на пустоту:
		inline bool isEmptyName() const noexcept(true) { return name.empty(); }
		inline bool isEmpty() const noexcept(true) { return isEmptyName() || sh_params.empty(); }
		//получение имени файла:
		inline string getName() const noexcept(true) { return name; }
		inline void setName(const string& val) noexcept(true) { name = val; }
		//проверка пустоты фильтра:
		inline bool isEmptyFilter() const noexcept(true) { return fltr.empty(); }
		//получение массива параметров страниц:
		vector<TSheetData> getSheetParams() const noexcept(true) { return sh_params; }
		const TSheetData& getSheetParam(size_t page) const noexcept(false) { return sh_params[page]; }
		//получение массива фильтрации:
		FilterArr getFilterLst() const noexcept(true) { return fltr; }
		//получение значения фильтра по индексу в массиве:
		TFilterData getFilterByIndex(size_t index) const noexcept(true) { return fltr[index]; }
		void setFilter(const FilterArr& filter) noexcept(true) { fltr = filter; }
		//функция получения числа обрабатываемых страниц:
		size_t getPageCnt() const noexcept(true) { return sh_params.size(); }
		void show(std::ostream& stream = std::cout) const noexcept(true);
	};

	//класс описываюийя обрабатываемые поля в пределах ячейки в файле:
	class TCellData
	{
	private:
		TIndex dst_indx;//индекс поля в приемнике
		TIndex dst_ins_indx;//индекс поля в приемнике, куда вставляются данные
		TIndex src_param_indx;//индекс поля-параметра в источнике
		TIndex src_val_indx;//индекс поля-значения в источнике
		DataType in_data_type;//тип данных на входе из ячейки dst_index
		DataType out_data_type;//тип данных на выходе из ячейки src_val_indx;
		//функция получения типа данных из json-дерева по тегу:
		static DataType getTypeFromJsonByCode(const ptree::value_type& node, const JsonParams& tag) noexcept(true);
	protected:
		//инициализация по умолчанию:
		TCellData(size_t idst = TIndex::EmptyIndex, size_t ins_dst = TIndex::EmptyIndex, 
			size_t isrc = TIndex::EmptyIndex, size_t val_src = TIndex::EmptyIndex, 
			const DataType& in_type = DataType::ErrorType, 
			const DataType& out_type = DataType::ErrorType) :
			dst_indx(idst), dst_ins_indx(ins_dst), src_param_indx(isrc), src_val_indx(val_src),
			in_data_type(in_type), out_data_type(out_type) {}
		//установка значений:
		TCellData& setData(size_t dst, size_t ins, size_t src_param, size_t val) noexcept(true);
		TCellData& setData(const ptree::value_type& parent_node,
			const JsonParams& dst_tag = JsonParams::dst_index,
			const JsonParams& dst_ins_tag = JsonParams::dst_insert_index,
			const JsonParams& src_param_tag = JsonParams::src_param_index,
			const JsonParams& src_val_tag = JsonParams::src_val_index,
			const JsonParams& in_type = JsonParams::in_data_type,
			const JsonParams& out_type = JsonParams::out_data_type
			) noexcept(true);
	public:
		//инициализация из ссылки на json-объект
		explicit TCellData(const ptree::value_type& parent_node) { setData(parent_node); }
		//деструктор ни чего не делает
		~TCellData() {}
		//проверка на пустоту индексов
		bool EmptyDstIndx() const noexcept(true) { return dst_indx.isEmpty(); }
		bool EmptyInsIndx() const noexcept(true) { return dst_ins_indx.isEmpty(); }
		bool EmptySrcParam() const noexcept(true) { return src_param_indx.isEmpty(); }
		bool EmptySrcVal() const noexcept(true) { return src_val_indx.isEmpty(); }
		bool isEmpty() const noexcept(true) { return EmptyDstIndx() and EmptySrcParam() and EmptyInsIndx(); }
		//признак выходного параметра:
		bool isOutParam() const noexcept(true) { return !dst_ins_indx.isEmpty() and !src_param_indx.isEmpty() and dst_indx.isEmpty(); }
		//вывод данных:
		inline size_t DstIndex() const noexcept(true) { return dst_indx.get(); }
		inline size_t InsIndex() const noexcept(true) { return dst_ins_indx.get(); }
		inline size_t SrcParam() const noexcept(true) { return src_param_indx.get(); }
		inline size_t SrcVal() const noexcept(true) { return src_val_indx.get(); }
		//оператор присвоения:
		inline void setDstIndex(size_t val) noexcept(true) { dst_indx.set(val); }
		inline void setInsIndex(size_t val) noexcept(true) { dst_ins_indx.set(val); }
		inline void setSrcParam(size_t val) noexcept(true) { src_param_indx.set(val); }
		inline void setSrcVal(size_t val) noexcept(true) { src_val_indx.set(val); }
		//функция получения типа для входного параметра:
		DataType getInType() const noexcept(true) { return in_data_type; }
		//фукнция получения типа для выходного параметра:
		DataType getOutType() const noexcept(true) { return out_data_type; }
		//присвоение
		TCellData& operator=(const TCellData& cd) noexcept(true);
		//отображение:
		void show(std::ostream& stream = std::cout) const noexcept(true);
	};

	//класс Тип заливки:
	class TCellFillType
	{
	private:
		TConstJSCellFill code;//тип заливки ячеек
		TColor color_if_found;//цвет выделения, если данные совпали
		TColor color_not_found;//цвет выделения, если данные не совпали
		//установка значений:
		void setFillType(size_t code, const TColor& color_find, const TColor& color_nfind) noexcept(false);
		void setFillType(const ptree::value_type& node, const JsonParams& code_tag = JsonParams::code,
			const JsonParams& color_find_tag = JsonParams::color_if_found,
			const JsonParams& color_not_found_tag = JsonParams::color_not_found);
	public:
		//отстутствие цвета
		static const TColor NoColor = TColor::COLOR_NONE;
		//инициализация
		TCellFillType(const JsonCellFill& ftype = JsonCellFill::Null,
			const TColor& find_color = TColor::COLOR_NONE, const TColor& not_find_color = NoColor) :
			code(ftype), color_if_found(find_color), color_not_found(not_find_color) {}
		explicit TCellFillType(ptree& parent_node): code(JsonCellFill::Null), 
			color_if_found(NoColor), color_not_found(NoColor) { setByJsonNode(parent_node); }
		explicit TCellFillType(const ptree::value_type& sub_node) : code(),
			color_if_found(NoColor), color_not_found(NoColor)	{ setFillType(sub_node); }
		//проверка на пустоту
		bool isEmpty() const noexcept(true) { return code.isEmpty() or code.isValid(true) == false; }
 		bool isEmptyColorFind() const noexcept(true) { return color_if_found == NoColor; }
		bool isEmptyColorNFind() const noexcept(true) { return color_not_found == NoColor; }
		bool isEmptyColor() const noexcept(true) { return isEmptyColorFind() and isEmptyColorNFind(); }
		//получение значений:
		JsonCellFill getCellFillType() const noexcept(true) { return code.Value(); }
		TColor getFindColor() const noexcept(true) { return color_if_found; }
		TColor getNFindColor() const noexcept(true) { return color_not_found; }
		//установка значений:
		void setCellFillType(const JsonCellFill& ftype) noexcept(true) { code = ftype; }
		void setFindColor(const TColor& val) noexcept(true) { color_if_found = val; }
		//установка значений из json-узла:
		bool setByJsonNode(ptree& parent_node, const JsonParams& type_tag = JsonParams::fill_type) noexcept(true);
		//присвоение
		TCellFillType& operator=(const TCellFillType& ftype) noexcept(false);
		//отображение информации:
		void show(std::ostream& stream = std::cout) const noexcept(true);
		//получение признака выполнения условия в зависимости от числа элементов, числа ошибок и кода:
		bool isSuccess(size_t cnt, size_t fail) const noexcept(true);
		//признак применения цветного шрифта при закраске ячейки:
		bool useFont() const noexcept(true);
	};

	//класс Метод обработки ячеек
	class TCellMethod
	{
	private:
		TConstJSMeth code;//код метода обработки
		TCellFillType fill_type;//метод заливки
		//установка значения:
		//установка данных из узла JSon-файла
		void setMethod(ptree::value_type& node,	const JsonParams& code_tag = JsonParams::code) noexcept(true);
	public:
		//инициализация по умолчанию
		TCellMethod(const JSonMeth& meth = JSonMeth::Null, const TCellFillType& ftype = TCellFillType()) :
			code(meth), fill_type(ftype) {}
		//инициализация
		explicit TCellMethod(ptree& parent_node, const JsonParams& parent_tag = JsonParams::Cells,
			const JsonParams& tag_meth = JsonParams::Method);
		explicit TCellMethod(ptree::value_type& sub_node) : code(JSonMeth::Null), fill_type() { setMethod(sub_node); }
		~TCellMethod() {}
		//проверка на пустоту:
		bool isEmpty() const noexcept(true) { return code == JSonMeth::Null; }
		bool isEmptyIncludeColor() const noexcept(true) { return fill_type.isEmptyColorFind(); }
		bool isEmptyExcludeColor() const noexcept(true) { return fill_type.isEmptyColorNFind(); }
		bool isEmptyColor() const noexcept(true) { return fill_type.isEmpty() or fill_type.isEmptyColor(); }
		//получение метода обработки
		JSonMeth getMethod() const noexcept(true) { return code.Value(); }
		//получение цвета заливки при выполнении условия:
		TColor getIncludeColor() const noexcept(true) { return fill_type.getFindColor(); }
		//получение цвета заливки при не выполнении условия:
		TColor getExcludeColor() const noexcept(true) { return fill_type.getNFindColor(); }
		//получение метода заливки:
		JsonCellFill getFillType() const noexcept(true) { return fill_type.getCellFillType(); }
		//установка метода:
		void setMethod(const JSonMeth& val) noexcept(true) { code = val; }
		//признак наличния секции SrcFile:
		bool isSrcFileSection() const noexcept(false) { return code.HasSrcFileObj(); }
		//уставнока вида заливки:
		void setCellFillType(const TCellFillType& ftype) noexcept(true) { fill_type = ftype; }
		//вывод данных в поток:
		void show(std::ostream& stream = std::cout) const noexcept(true);
		//функция определения удачного выполнения по коду заливки:
		bool isSuccess(size_t cnt, size_t fail) const noexcept(true);
		//функция определения признака использования цветного шрифта:
		bool useFont() const noexcept(true) { return fill_type.useFont(); }
	};

	using CellDataArr = vector<TCellData>;

	//структура обрабатываемых ячеек:
	class TProcCell
	{
	private:
		TCellMethod meth;//метод обработки
		TShareData* SrcFile;//файл-источник
		vector<TUserTune> db_tune;//настроки для соединения с БД
		CellDataArr cel_arr;//индексы обрабатываемх колонок
		//инициализация файла-источника
		void InitSrcFile(ptree& node, const JsonParams& tag = JsonParams::SrcFile,
			const string& main_path = "") noexcept(true);
		//деинициализация файла-источника
		void DeInitSrcFile() noexcept(false) { if (SrcFile) delete SrcFile; SrcFile = nullptr; }
		//инициализация настроек соединения с БД
		void InitDBTune(const ptree& node, const TSimpleTune* tune_ref,
			const JsonParams& tag = JsonParams::DB_Config) noexcept(true);
		//инициализация массива данных о колонках/столюцах:
		void InitCellData(ptree& node, const JsonParams& tag = JsonParams::DataArr) noexcept(true);
		//инициализация в зависимости от метода:
		void InitByMethod(ptree& node, const TSimpleTune* tune_ref = nullptr) noexcept(true);
	public:
		//инициализация
		explicit TProcCell(ptree& parent_node, const TSimpleTune* tune_ref = nullptr);
		//деинициализация
		~TProcCell() { DeInitSrcFile(); }
		//проверка на пустоту:
		bool isEmpty() const noexcept(true) { return meth.isEmpty() or cel_arr.empty(); }
		//данные получаемые из метода:
		JSonMeth getMethodCode() const noexcept(true) { return meth.getMethod(); }
		//получение ссылки на метод обработки:
		const TCellMethod& getMethod() const noexcept(true) { return meth; }
		//получение данных из файла источника:
		bool NoSrcFile() const noexcept(true) { return SrcFile == nullptr or SrcFile->isEmpty(); }
		const TShareData* getSrcFilRef() const noexcept(true) { return SrcFile; }
		string SrcFileName() const noexcept(true) { return NoSrcFile() ? string() : SrcFile->getName(); }
		size_t TuneCnt() const noexcept(true) { return db_tune.size(); }
		size_t CellCnt() const noexcept(true) { return cel_arr.size(); }
		FilterArr SrcFileFilters() const noexcept(true) { return NoSrcFile() ? FilterArr() : SrcFile->getFilterLst(); }
		//получение массива данных для ячеек:
		const CellDataArr& getCellDataArr() const noexcept(true) { return cel_arr; }
		//получение настроек для соединения с БД
		vector<TUserTune> getDBTuneArr() const noexcept(true) { return db_tune; }
		//отображение данных:
		void show(std::ostream& stream = std::cout) const noexcept(true);
	};

	//структура данных на сравниваемых листах источника и приемника
	class TExcelProcData
	{
	private:
		TShareData* DstFile;//файла приемника
		TProcCell* cells;//колонки обрабатываемых значений
		//инициализация файла-приемника:
		void InitDstFile(const ptree::value_type& node, const string& main_path) noexcept(true);
		//деинииализация файла-приемника
		void DeInitDstFile() noexcept(true);
		//инициализация данных о ячейках:
		void InitCells(ptree::value_type& node, const TSimpleTune* tune_ref = nullptr) noexcept(true);
		//деинициализация данных ячеек
		void DeInitCells() noexcept(true);
		//инициализация объекта из json-файла по узловому тегу:
		bool InitObjByTag(ptree& json, const JsonParams& tag, const TSimpleTune* tune_ref = nullptr) noexcept(false);
		//инициализация параметров объекта:
		void InitExcelProcData(const string& json_file, const TSimpleTune* tune_ref = nullptr) noexcept(true);
	public:
		//инициализация json-файлом
		explicit TExcelProcData(const string& json_file, const TSimpleTune* tune_ref = nullptr);
		//деинициализация
		~TExcelProcData();
		//проверка на пустоту:
		bool isDstFileEmpty() const noexcept(true) { return DstFile == nullptr or DstFile->isEmpty(); }
		bool isCellsEmpty() const noexcept(true) { return cells == nullptr or cells->isEmpty(); }
		bool isEmpty() const noexcept(true) { return isDstFileEmpty() or isCellsEmpty(); }
		//получение кода метода обработки:
		JSonMeth MethodCode() const noexcept(true) { return cells ? cells->getMethodCode() : JSonMeth::Null; }
		//получение ссылки на файл-приемник:
		TShareData& getDstFile() const noexcept(false) { return *DstFile; }
		//получение ссылки на данные по колонкам:
		TProcCell& getCellsData() const noexcept(false) { return *cells; }
		//получение числа обрабатываемых страниц:
		size_t getProcPagesCnt() const noexcept(true) { return DstFile ? DstFile->getPageCnt() : 0; }
		//отображение данных:
		void show(std::ostream& stream = std::cout) const noexcept(true);
	};
/**/
}

#endif TUNE_PARAM_H_