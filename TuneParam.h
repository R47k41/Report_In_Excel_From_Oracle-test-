//#pragma once
#ifndef TUNE_PARAM_H_
#define TUNE_PARAM_H_
//модуль предназначен для описания класса работающего с файлом настроек
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <utility>

//обаласти пространства имен для работы с настройками
namespace NS_Tune
{
	using std::string;
	using std::ifstream;
	using std::pair;
	using std::vector;
	using std::set;
	using find_fnc = size_t (string::*)(const string& s, size_t pos) const;

	//Поля из файла настроек:
	//порядок указан такой же, как и файле настроек
	enum class TuneField {
		Empty = 0,
		DataBase, Report, SqlParams, Columns, Block_End,
		UserName, Password, TNS, TemplateName, OutFileName, SqlFile, SqlText, SqlParam, Column,
		SqlParamQuane, SqlParamType, SqlParamNote, SqlParamValue,
		Last
	};

	//Типы данных для параметров в запросах:
	enum class DataType { ErrorType = 0, String, Integer, Double, Date, Last };

	//Типы разделителей значений:
	enum class Tags {Null = '\0', colon = ':', quotes = '\"', dash = '\\', rangle = '>', langle = '<',
		quane = '=', semicolon = ';',
		//составные теги:
		open_param_tag=1000, close_param_tag,
		Last};

	template <typename T>
	void Str2Type(const string& str, T& val) noexcept(false);

	template <typename T>
	string Type2Str(T val) noexcept(true);

	//базовый интерфейс для константных значений:
	template <typename T>
	class TConstant
	{
	
	private:
		int val;//значение параметра
		//суммирование:
		TConstant& operator+=(int x) { val += x; return *this; }
		//инициализация
		virtual void Init(int x);
	protected:
		//нахождение в диапазоне
		virtual bool inRange(const T& a, const T& b) const noexcept(true) { return (Value() > a and Value() < b); };
		//проверка валидности
		virtual bool isValid(const T& a, const T& b, bool exit_on_error = false) const noexcept(false);
		//установка значения из типа:
		void setValue(const T& x);
	public:
		explicit TConstant(int x) { Init(x); }
		explicit TConstant(const T& x);
		//проверка валидности:
		virtual bool isValid(bool exit_on_error = false) const noexcept(false) = 0;
		//перевод значения в строку
		virtual string toStr() const = 0;
		//перевод значения в число
		virtual int toInt() const { return val; }
		//полечение значения нужного типа:
		T Value() const { return T(val); }
		//сравнение значения со строкой
		virtual bool operator==(const string& str) const { return toStr() == str; }
		//получение следующего элемента:
		virtual TConstant& Next(bool exit_on_err = true) noexcept(false);
		//проверка на пустоту:
		virtual bool isEmpty() const = 0;
		//опреция присвоения:
		TConstant& operator=(const T& x);
		//оператор сравнения с типом:
		bool operator<(const T& x) const { return Value() < x; }
		bool operator>(const T& x) const { return !(operator==(x) or operator<(x)); }
		virtual bool operator==(const T& x) const { return Value() == x; }
		virtual bool operator!=(const T& x) const { return !operator==(x); }
		//операция сравнения между собой:
		bool operator<(const TConstant& x) const { return val < x.val; }
	};

	class TConstField : public TConstant<TuneField>
	{
	public:
		explicit TConstField(const TuneField& x) : TConstant<TuneField>(x) {}
		explicit TConstField(int x) : TConstant<TuneField>(x) {}
		virtual bool isValid(bool exit_on_error = false) const noexcept(false)
			{	return TConstant<TuneField>::isValid(TuneField::Empty, TuneField::Last, exit_on_error); }
		//проверка содержания параметра в строке:
		virtual bool StrInclude(const string& str) const;
		//перевод значения в строку
		virtual string toStr() const;
		//проверка на пустоту:
		virtual bool isEmpty() const { return Value() == TuneField::Empty; }
		//операция присвоения:
		TConstField& operator=(const TuneField& x) { TConstant<TuneField>::operator=(x); return *this; }
	};

	class TConstType : public TConstant<DataType>
	{
	public:
		explicit TConstType(const DataType& x) : TConstant<DataType>(x) {}
		explicit TConstType(int x) : TConstant<DataType>(x) {};
		explicit TConstType(const string& str);
		bool isValid(bool exit_on_error = false) const noexcept(false)
			{ return TConstant<DataType>::isValid(DataType::ErrorType, DataType::Last, exit_on_error); }
		//перевод значения в строку
		virtual string toStr() const;
		//проверка на пустоту:
		virtual bool isEmpty() const { return Value() == DataType::ErrorType; }
		//операция присвоения:
		TConstType& operator=(const DataType& x) { TConstant<DataType>::operator=(x); return *this; }
	};

	class TConstTag : public TConstant<Tags>
	{
	protected:
		//нахождение в диапазоне
		virtual bool inRange() const noexcept(true)
			{ return TConstant<Tags>::inRange(Tags::Null, Tags::Last); }
	public:
		explicit TConstTag(const Tags& x) : TConstant<Tags>(x) {};
		explicit TConstTag(int x) : TConstant<Tags>(x) {};
		bool isValid(bool exit_on_error = false) const noexcept(false)
		{ return TConstant<Tags>::isValid(Tags::Null, Tags::Last, exit_on_error); }
		//перевод значения в строку
		virtual string toStr() const;
		//проверка на пустоту:
		virtual bool isEmpty() const { return Value() == Tags::Null; }
		//операция присвоения:
		TConstTag& operator=(const Tags& x) { TConstant<Tags>::operator=(x); return *this; }
	};

	class TBaseParam
	{
	public:
	private:
		string src_data;//исходные данные
	protected:
		string value;//значение параметра
		size_t get_pos_in_src(const string& substr, const size_t beg = 0, find_fnc = &string::find) const;
		size_t get_pos_in_src(const TConstField& substr, const size_t beg = 0, find_fnc = &string::find) const;
		size_t get_pos_in_src(const Tags& substr, const size_t beg = 0, find_fnc = &string::find) const;
		//по умолчанию выполняет последовательный поиск в строке:
		virtual string Get_TuneField_Val(const TConstField& param, const Tags& b_delimeter, const Tags& e_delimeter) const;
		void setSrcData(const string& val) { src_data = val; };
		virtual void setValue(const TConstField&, const Tags& open_val, const Tags& close_val) = 0;
	public:
		explicit TBaseParam(const string& str) : src_data(str) {}
		TBaseParam(const TBaseParam& x) : src_data(x.src_data), value(x.value) {}
		virtual bool isEmpty() const { return src_data.empty(); }
		virtual string toStr() const { return src_data; }
		virtual string Value() const { return value; }
		size_t srcSize() const { return src_data.size(); }
		string srcSubStr(size_t posb, size_t pose) const { return src_data.substr(posb, pose - posb); }
		size_t srcFind(const string& substr, size_t pos) const { return src_data.find(substr, pos); }
	};

	//класс параметров из строки:
	class TStringParam: public TBaseParam
	{
	private:
		TConstField param;//параметр
	protected:
		//закрывающий тег ищет с конца строки:
		virtual string Get_TuneField_Val(const TConstField& param, const Tags& b_delimeter, const Tags& e_delimeter) const;
		virtual void setValue(const TConstField&, const Tags& open_val, const Tags& close_val);
	public:
		TStringParam(const string& full_data, const TuneField& tune_field,
			const Tags& open_val = Tags::quotes, const Tags& close_val = Tags::quotes);
		virtual bool isEmpty() const { return (param.isEmpty() or value.empty()); }
		virtual string ParamName() const { return param.toStr(); }
		TuneField Param() const { return param.Value(); }
		bool operator<(const TuneField& x) const { return param < x; }
		bool operator>(const TuneField& x) const { return param > x; }
		bool operator==(const TuneField& x) const { return param == x; }
		string toStr(bool use_quotes = false) const;
		bool operator<(const TStringParam& x) const { return param < x.param; }
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
		virtual void setValue(const TConstField&, const Tags&, const Tags&);
	public:
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
	using TFields = std::set<TStringParam>;
	using StrArr = std::vector<string>;
	using ParamArr = std::set<TSubParam>;

	//Класс - данные для подключения:
	class TUserData
	{
	private:
		TFields fields;
		StrArr cols;
		ParamArr params;
		//функция считывания потоковых параметров:
		void Read_StreamData_Val(ifstream& file, const TuneField& stream_title, StrArr& str_arr);
		//функция считывания данных о параметрах запроса:
		void Read_Param_Val(ifstream& file);
		//функция чтения значений колонок
		void Read_Col_Val(ifstream& file);
		//функция чтения занчений настроки из файла:
		void Read_Tune_Val(ifstream& file);
		//чтение настроек из файла:
		void ReadFromFile(const string& file);
		TUserData(const TFields& v);
		TUserData(const TUserData& v);
		TUserData& operator=(const TUserData& v);
		//шаблон функции для получения значения поля по его уникальному коду/ID
		template <typename KeyType, typename ValType>
		const ValType& getValueByID(const KeyType& par_ID, const set<ValType>& arr) const noexcept(false);
	public:
		//список настроек формируется из файла
		explicit TUserData(const string& tunefile);
		~TUserData() {};
		//проверка на пустоту для полей:
		bool EmptyFields(void) const { return fields.empty(); }
		//проверка пустой ли список колонок
		bool EmptyColumns(void) const { return cols.empty(); }
		//проверка на наличие параметров:
		bool EmptyParams(void) const { return params.empty(); }
		//проверка на пустоту для продолжения работы:
		bool Empty() const { return EmptyFields() && EmptyColumns(); }
		//получение значения:
		string getFieldByCode(const TuneField& code, bool exit_on_er = true) const noexcept(false);
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
		//установка занчения(не определена - удалить?):
		//void setValue(const TuneField& code, const string& val);
		//функция отображения списка настроек:
		void show_tunes(void) const;
		//функция отображения списка колонок:
		void show_columns(void) const;
		//функция отображения параметров:
		void show_params() const;
		//функция получения числа колонок:
		size_t ColumnsCnt() const { return cols.size(); }
		//функция получения числа параметров:
		size_t ParamsCnt() const { return params.size(); }
	};
}

#endif TUNE_PARAM_H_