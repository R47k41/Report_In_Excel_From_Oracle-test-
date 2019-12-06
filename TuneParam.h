//#pragma once
#ifndef TUNE_PARAM_H_
#define TUNE_PARAM_H_
//������ ������������ ��� �������� ������ ����������� � ������ ��������
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <utility>
#include "TConstants.h"

//�������� ������������ ���� ��� ������ � �����������
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
		string src_data;//�������� ������
	protected:
		string value;//�������� ���������
		size_t get_pos_in_src(const string& substr, const size_t beg = 0, find_fnc ff = &string::find) const;
		size_t get_pos_in_src(const TConstField& substr, const size_t beg = 0, find_fnc ff = &string::find) const;
		size_t get_pos_in_src(const CtrlSym& substr, const size_t beg = 0, find_fnc ff = &string::find) const;
		//��������� �������� �� ������ ������:
		virtual string Get_Str_Val(size_t pos, const CtrlSym& b_delimeter, const CtrlSym& e_delimeter,
			bool from_end = false) const;
		//�� ��������� ��������� ���������������� ����� � ������:
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
		//������ ��������� ��������
		virtual void Value(const string& val) { value = val; }
	};

	//����� ���������� �� ������:
	class TStringParam: public TBaseParam
	{
	private:
		TConstField param;//��������
	protected:
		virtual void setValue(const TConstField&, const CtrlSym& open_val, const CtrlSym& close_val);
	public:
		TStringParam() : TBaseParam(string()), param(TuneField::Empty) {};
		explicit TStringParam(const TConstField& val) : TBaseParam(string()), param(val) {}
		//������ ������������� ����������
		TStringParam(const TuneField& tune_field, const string& str, bool parse_val = true);
		//������������� ������� � ����������� �������� ���������
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
		//������������ ���� ��� ���������� �������
		//virtual pair<TuneField, TStringParam> toPair() const { return std::make_pair(param.Value(), *this); };
	};

	//����� ��� ��������� ����������:
	class TSubParam: public TBaseParam
	{
	private:
		const int EmptyID = 0;
		int id;//����� ���������
		TConstType type;//��� ���������
		string comment;//����������
	protected:
		virtual void setValue(void);
		virtual void setValue(const TConstField&, const CtrlSym&, const CtrlSym&);
	public:
		explicit TSubParam(int val) : TBaseParam(string()), id(val), type(DataType::ErrorType), comment() {}
		explicit TSubParam(const string& str);
		TSubParam(const TSubParam& x) : TBaseParam(x), id(x.id), type(x.type), comment(x.comment) {}
		//��������� ID ���������
		int ID() const { return id; }
		//��������� ���� ������ � ���� ������
		string Type() const { return type.toStr(); }
		//��������� ���� ������ � ���� �������
		DataType DataType() const { return type.Value(); }
		//��������� ����������
		string Comment() const { return comment; }
		//��������� ���� ���������:
		string getCode() const;
		//��������� �������� �������������:
		bool setValByUser();
		//�������� �������:
		void show() const;
		//�������� < ��� ����������:
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

	//��������� ������������:
	//������� �����
	class TSimpleTune
	{
	protected:
		TFields fields;//���� �������� � �� ����������
		enum class Types { Config, Sql, Template, OutPath, OutName, OutSheet };
		enum class TRead { Section, TuneVal };
	private:
		TSimpleTune& operator=(const TSimpleTune& v);
	protected:
		//������� ��������� ������ ������ � ��������:
		static vector<string> getFileLst(const string& file_dir, const string& file_ext = "", bool use_sort = true) noexcept(false);
		//������� ���������� ����������� � �����:
		static void AddDelimeter(string& str, const char delim) noexcept(false);
		//������� ��������� ����/���������� �� ��������:
		string getPathByCode(const Types& code) const noexcept(true);
		//������� ��������� ���� �� ����:
		FileParam getFileParamByCode(const Types& code) const noexcept(true);
		//������� ��������� ������ ������ � �������� �� ��������:
		vector<string> getFileLst(const Types& code, bool use_sort = true) const noexcept(false);
		//��������� ������� ��� ������ ���� � ������:
		//�������� �� ���������� boost\date_time ��� ������������� date_facet
		//������������ �������� ������ 
		//https://www.boost.org/doc/libs/1_49_0/doc/html/date_time/date_time_io.html
		static bool set_date_format(std::ostream& stream, const string& fromat) noexcept(true);
		static string cur_date_to_string_by_format(const string& format) noexcept(false);
		//������� ��������� ����� �� ���� ��������(������������� ������� ����)
		string AddCurDate2Name(const Types& code) const noexcept(false);
		//������� ���������� ���� ���������:
		bool AddField(const TStringParam& val) noexcept(true);
		//������� ��������� �������� ���������:
		bool setFieldVal(const TuneField& key, const string& val) noexcept(false);
		//������� ��������� ����������� �� ��������� ������ ������� ����������:
		virtual TuneRange getTuneRange(const TRead& x) const noexcept(true) = 0;
		//������� ��������� ������ ���������� � ��������� ���������
		static RangeField getTuneField4Range(const TuneRange& range);
		//������� ������ - ���������� ����� �� ������� [END]:
		static void skip_block(ifstream& file, const string& end_block);
		//������� ���������� ������ �������� �� ����� �� ����
		virtual void Read_Section(ifstream& file, const string& code) = 0;
		//������� ������ �������� �������� �� �����:
		virtual void Read_Tune_Val(ifstream& file);
		//������ �������� �� �����:
		virtual void ReadFromFile(const string& file);
		//template <typename KeyType, typename ValType>
		//ValType& getValueByID(const KeyType& par_ID, set<ValType>& arr) noexcept(false);
		//������ ������� ��� ��������� �������� ���� �� ��� ����������� ����/ID
		template <typename KeyType, typename ValType>
		static ValType& getElementByID(const KeyType& par_ID, vector<ValType>& arr)  noexcept(false);
		template <typename KeyType, typename ValType>
		static const ValType& getConstElementByID(const KeyType& par_ID, const vector<ValType>& arr)  noexcept(false);
		/*
		template <typename KeyType, typename ValType>
		static const ValType& NS_Tune::TSimpleTune::getValueByID(const KeyType& par_ID, const set<ValType>& arr) noexcept(false);
		/**/
		//������� ��������� �������� ��� ���������:
		//��������� ����� ���������� �����/��������/����:
		string getNameByCode(const Types& code) const noexcept(true);
		//��������� ������� ����� ����� �� ����:
		string getFullFileName(const Types& code, bool only_path = false) const noexcept(true);
	public:
		TSimpleTune() {}
		//������������� ������ ����������
		explicit TSimpleTune(const TSimpleTune& v) : fields(v.fields) {}
		explicit TSimpleTune(const TFields& v) : fields(v) {}
		~TSimpleTune() {}
		//�������� �� ������� ��� ����������� ������:
		virtual bool Empty() const { return fields.empty(); }
		//��������� ��������:
		string getFieldValueByCode(const TuneField& code, bool exit_on_er = true) const noexcept(true);
		//������� ����������� ������ ��������:
		virtual void show_tunes(void) const;
		//���� ������� ��������� �� ������� ����� ������������ �������� �� ���������
		string getOutSheet() const noexcept(false) { return getFullFileName(Types::OutSheet); }
	};
	
	//����� ��������� ������������:
	class TSharedTune : public TSimpleTune
	{
	private:
		string  main_code;//��� �������� ���������
		TSharedTune& operator=(const TSharedTune& v);
		//��������� ����� ������ �������������� �������� ������
		string getSectionName() const noexcept(true);
		virtual TuneRange getTuneRange(const TRead& x) const noexcept(true);
		//���������� ������
		virtual void Read_Section(ifstream& file, const string& code);
	public:
		//�������������
		TSharedTune(const string& file, const string& code);
		TSharedTune(const TFields& v, const string& file, const string& code);
		TSharedTune(const TSharedTune& v, const string& file, const string& code);
		~TSharedTune() {}
		//��� �������� ���������
		string getMainCode(bool use_brkt = true) const;
		//��������� �������� �������� ���������:
		string getMainCodeVal() const { return getFieldValueByCode(TuneField::MainPath); }
		//�������� �������:
		bool Empty() const { return TSimpleTune::Empty(); }
		//��������� ������ ������:
		vector<string> getConfFileLst(bool use_sort = true) const noexcept(false) { return getFileLst(Types::Config, use_sort); }
		vector<string> getSqlFileLst(bool use_sort = true) const noexcept(false) { return getFileLst(Types::Sql, use_sort); }
		vector<string> getTemplFileLst(bool use_sort = true) const noexcept(false) { return getFileLst(Types::Template, use_sort); }
		//��������� ���� � ����� ��������:
		string getConfigPath() const noexcept(false) { return getFullFileName(Types::Config, true); }
		//��������� ����� ���������� �����:
		string getOutPath() const noexcept(false) { return getFullFileName(Types::OutPath, true); }
		string getOutFile() const noexcept(false) { return getFullFileName(Types::OutName, false); }
		//�������� ������������� ���������� - ���� �� ���������� - �������:
		static bool CheckPath(const string& dir, bool crt_if_not_found = true);
	};

	//��������� ������������ ��� �������� ������
	class TUserTune: public TSimpleTune
	{
	private:
		StrArr cols;
		ParamArr params;
		virtual TuneRange getTuneRange(const TRead& x) const noexcept(true);
		//������� ���������� ������ �������� �� ����� �� ����
		void Read_Section(ifstream& file, const string& code);
		//������� ���������� ��������� ����������:
		void Read_StreamData_Val(ifstream& file, const TuneField& stream_title, StrArr& str_arr);
		//������� ���������� ������ � ���������� �������:
		void Read_Param_Val(ifstream& file);
		//������� ������ �������� �������
		void Read_Col_Val(ifstream& file);
		TUserTune(const TFields& v);
		TUserTune(const TUserTune& v);
		TUserTune& operator=(const TUserTune& v);
	public:
		//������ �������� ����������� �� �����
		explicit TUserTune(const string& tunefile);
		//������������� ���������� �������� � ����� ������
		explicit TUserTune(const TSimpleTune& tune, const string& file);
		~TUserTune() {};
		//�������� �� ������� ��� �����:
		virtual bool EmptyFields(void) const { return TSimpleTune::Empty(); }
		//�������� ������ �� ������ �������
		bool EmptyColumns(void) const { return cols.empty(); }
		//�������� �� ������� ����������:
		bool EmptyParams(void) const { return params.empty(); }
		//�������� �� ������� ��� ����������� ������:
		bool Empty() const { return EmptyFields() && EmptyColumns(); }
		//��������� �������� ��� ��������� �� ID:
		string getParamValByID(int par_id, bool exit_on_er = true) const noexcept(false);
		//��������� ���������:
		TSubParam getParamByID(int par_id, bool exit_on_er = true) const noexcept(false);
		//��������� ������� ����������:
		ParamArr getParams() const { return params; }
		//������� ��������� �������� ��� ������� � ��������� ��������:
		string getColValByIndx(int indx) const { return cols[indx]; }
		//��������� ������ �������:
		StrArr getColumns(void) const { return cols; }
		//������� ����������� ������ �������:
		void show_columns(void) const;
		//������� ����������� ����������:
		void show_params() const;
		//������� ��������� ����� �������:
		size_t ColumnsCnt() const { return cols.size(); }
		//������� ��������� ����� ����������:
		size_t ParamsCnt() const { return params.size(); }
		//��������� ����� ����� ��������:
		string getSqlFile() const noexcept(false) { return getFullFileName(Types::Sql, false); }
		//��������� ����� ���������� �����:
		string getTemplateFile() const noexcept(false) { return getFullFileName(Types::Template, false); }
	};
}

#endif TUNE_PARAM_H_