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
		size_t get_pos_in_src(const string& substr, const size_t beg = 0, find_fnc = &string::find) const;
		size_t get_pos_in_src(const TConstField& substr, const size_t beg = 0, find_fnc = &string::find) const;
		size_t get_pos_in_src(const CtrlSym& substr, const size_t beg = 0, find_fnc = &string::find) const;
		//�� ��������� ��������� ���������������� ����� � ������:
		virtual string Get_TuneField_Val(const TConstField& param, const CtrlSym& b_delimeter, const CtrlSym& e_delimeter) const;
		void setSrcData(const string& val) { src_data = val; };
		virtual void setValue(const TConstField&, const CtrlSym& open_val, const CtrlSym& close_val) = 0;
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

	//����� ���������� �� ������:
	class TStringParam: public TBaseParam
	{
	private:
		TConstField param;//��������
	protected:
		//����������� ��� ���� � ����� ������:
		virtual string Get_TuneField_Val(const TConstField& param, const CtrlSym& b_delimeter, const CtrlSym& e_delimeter) const;
		virtual void setValue(const TConstField&, const CtrlSym& open_val, const CtrlSym& close_val);
	public:
		TStringParam(const string& full_data, const TuneField& tune_field,
			const CtrlSym& open_val = CtrlSym::quotes, const CtrlSym& close_val = CtrlSym::quotes);
		virtual bool isEmpty() const { return (param.isEmpty() or value.empty()); }
		virtual string ParamName() const { return param.toStr(); }
		TuneField Param() const { return param.Value(); }
		bool operator<(const TuneField& x) const { return param < x; }
		bool operator>(const TuneField& x) const { return param > x; }
		bool operator==(const TuneField& x) const { return param == x; }
		string toStr(bool use_quotes = false) const;
		bool operator<(const TStringParam& x) const { return param < x.param; }
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
	using TFields = std::set<TStringParam>;
	using StrArr = std::vector<string>;
	using ParamArr = std::set<TSubParam>;

	//����� - ������ ��� �����������:
	class TUserData
	{
	private:
		TFields fields;
		StrArr cols;
		ParamArr params;
		//������� ���������� ��������� ����������:
		void Read_StreamData_Val(ifstream& file, const TuneField& stream_title, StrArr& str_arr);
		//������� ���������� ������ � ���������� �������:
		void Read_Param_Val(ifstream& file);
		//������� ������ �������� �������
		void Read_Col_Val(ifstream& file);
		//������� ������ �������� �������� �� �����:
		void Read_Tune_Val(ifstream& file);
		//������ �������� �� �����:
		void ReadFromFile(const string& file);
		TUserData(const TFields& v);
		TUserData(const TUserData& v);
		TUserData& operator=(const TUserData& v);
		//������ ������� ��� ��������� �������� ���� �� ��� ����������� ����/ID
		template <typename KeyType, typename ValType>
		const ValType& getValueByID(const KeyType& par_ID, const set<ValType>& arr) const noexcept(false);
	public:
		//������ �������� ����������� �� �����
		explicit TUserData(const string& tunefile);
		~TUserData() {};
		//�������� �� ������� ��� �����:
		bool EmptyFields(void) const { return fields.empty(); }
		//�������� ������ �� ������ �������
		bool EmptyColumns(void) const { return cols.empty(); }
		//�������� �� ������� ����������:
		bool EmptyParams(void) const { return params.empty(); }
		//�������� �� ������� ��� ����������� ������:
		bool Empty() const { return EmptyFields() && EmptyColumns(); }
		//��������� ��������:
		string getFieldByCode(const TuneField& code, bool exit_on_er = true) const noexcept(false);
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
		//��������� ��������(�� ���������� - �������?):
		//void setValue(const TuneField& code, const string& val);
		//������� ����������� ������ ��������:
		void show_tunes(void) const;
		//������� ����������� ������ �������:
		void show_columns(void) const;
		//������� ����������� ����������:
		void show_params() const;
		//������� ��������� ����� �������:
		size_t ColumnsCnt() const { return cols.size(); }
		//������� ��������� ����� ����������:
		size_t ParamsCnt() const { return params.size(); }
	};
}

#endif TUNE_PARAM_H_