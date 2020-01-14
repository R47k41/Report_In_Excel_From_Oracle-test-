//#pragma once
#ifndef TUNE_PARAM_H_
#define TUNE_PARAM_H_
//������ ������������ ��� �������� ������ ����������� � ������ ��������
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <utility>
#include <boost/property_tree/ptree.hpp>
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
	using boost::property_tree::ptree;


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
		enum class Types { Config, SQL, DQL, DML, Template, OutPath, OutName, OutSheet };
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
		string getFieldValueByCode(const TuneField& code) const noexcept(true);
		//��������� �������������� �������� �� ���� ���������:
		//���� ������� false - ��������� ������ ��� ��������� ��������
		bool FieldValueAsInt(const TuneField& code, int& val) const noexcept(false);
		//������� ����������� ������ ��������:
		virtual void show_tunes(void) const;
		//���� ������� ��������� �� ������� ����� ������������ �������� �� ���������
		string getOutSheet() const noexcept(false) { return getFullFileName(Types::OutSheet); }
		//������� ��������� ����� ��������� ����� �� ��������� ��������:
		string getOutFileBySheet() const noexcept(false);
		//������� �������� �������� bool-���������:
		bool useFlag(const TuneField& code) const noexcept(true);
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
		vector<string> getSqlFileLst(bool use_sort = true) const noexcept(false) { return getFileLst(Types::DQL, use_sort); }
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
		TUserTune& operator=(const TUserTune& v);
		//��������� ������� sql-������ ��� ����������:
		vector<string> getSQLFileLst(const Types& kind, bool use_sort = true) const noexcept(false);
		bool isEmptyByCode(const Types& kind) const noexcept(false);
	public:
		//������ �������� ����������� �� �����
		explicit TUserTune(const string& tunefile);
		//������������� ���������� �������� � ����� ������
		explicit TUserTune(const TSimpleTune& tune, const string& file);
		TUserTune(const TUserTune& tune) : TSimpleTune(tune), cols(tune.cols), params(tune.params) {}
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
		//��������� ����� ����� ��������/������:
		string getDQLFile() const noexcept(false) { return getFullFileName(Types::DQL, false); }
		string getDMLFile() const noexcept(false) {return getFullFileName(Types::DML, false); }
		//��������� ������ ������ sql-��������:
		vector<string> getDQLFileLst(bool use_sort = true) const noexcept(false) { return getSQLFileLst(Types::DQL); }
		vector<string> getDMLFileLst(bool use_sort = true) const noexcept(false) { return getSQLFileLst(Types::DML); }
		//�������� �������� ����������:
		bool isDQLEmpty() const noexcept(true) { return isEmptyByCode(Types::DQL); }
		bool isDMLEmpty() const noexcept(true) { return isEmptyByCode(Types::DML); }
		//��������� ����� ���������� �����:
		string getTemplateFile() const noexcept(false) { return getFullFileName(Types::Template, false); }
	};

	//��������� ��� ��������� excel-������
	//��������� ��������� � ������� json-������
	//�������� ������ ��� ��������� ������ � ������� excel-������:
	//��������� ����� ����������� � �������������� boost ��� json(�� ���� ������� � ����� �������)
	//�����: https://www.boost.org/doc/libs/1_52_0/doc/html/property_tree.html
	//������� ����� ��� ��������:
	class TIndex
	{
	private:
		size_t index;
	public:
		static const size_t EmptyIndex = 0;
		TIndex(size_t indx = EmptyIndex) : index(indx) {}
		TIndex(const TIndex& x) : index(x.index) {}
		TIndex(const ptree::value_type& json_node);
		inline bool isEmpty() const noexcept(true) { return index == EmptyIndex; }
		inline size_t get() const noexcept(true) { return index; }
		inline void set(size_t val) noexcept(false) { index = val; }
		bool operator==(size_t val) const noexcept(true) { return isEmpty() ? false: index == val; }
		bool operator==(const TIndex& val) const noexcept(false) { return isEmpty() ? false : index == val.index; }
		//��������� �������� ������� �� json-�����:
		static size_t getIndexByJson(const ptree& json, const string& parent_node,
			const NS_Const::JsonParams& child_node) noexcept(true);
		static size_t getIndexByJson(const ptree& json, const NS_Const::JsonParams& parent_node,
			const NS_Const::JsonParams& child_node) noexcept(true);
	};

	//����� ����������� �������������� ����:
	class TCellData
	{
	private:
		TIndex dst_indx;//������ ���� � ���������
		TIndex src_indx;//������ ���� � ���������
		TIndex dst_ins_indx;//������ ���� � ���������, ���� ����������� ������
	public:
		//������������� �� ���������:
		/*
		TCellData(TIndex dst = TIndex::EmptyIndex, TIndex src = TIndex::EmptyIndex, TIndex ins = TIndex::EmptyIndex) :
			dst_indx(dst), src_indx(src), dst_ins_indx(ins) {}
		/**/
		//������������� �� ������ �� json-������
		TCellData(const ptree& json);
		//���������� �� ���� �� ������
		~TCellData() {}
		//�������� �� ������� ��������
		bool EmptyDstIndx() const noexcept(true) { return dst_indx == TIndex::EmptyIndex; }
		bool EmptySrcIndx() const noexcept(true) { return src_indx == TIndex::EmptyIndex; }
		bool EmptyInsIndx() const noexcept(true) { return dst_ins_indx == TIndex::EmptyIndex; }
		//������� �� ����������� ��������� ��������:
		inline bool AnotherProcedure() const noexcept(true) { return dst_indx == dst_ins_indx && EmptySrcIndx(); }
		//����� ������:
		inline size_t DstIndex() const noexcept(true) { return dst_indx.get(); }
		inline size_t SrcIndex() const noexcept(true) { return src_indx.get(); }
		inline size_t InsIndex() const noexcept(true) { return dst_ins_indx.get(); }
		//�������� ����������:
		inline void setDstIndex(size_t val) noexcept(true) { dst_indx.set(val);}
		inline void setSrcIndex(size_t val) noexcept(true) { src_indx.set(val); }
		inline void setInsIndex(size_t val) noexcept(true) { dst_ins_indx.set(val); }
		//����������
		TCellData& setData(size_t dst, size_t src, size_t ins) noexcept(true);
		TCellData& operator=(const TCellData& cd) noexcept(true);
	};

	//����� ��� ��������� �������� ������� ����������:
	class TFilterData
	{
	private:
		TIndex col_indx;//������ �������, ��� ����������� �������
		string value;//�������/�������� ����������
	public:
		//�����������
		TFilterData(std::pair<size_t, string> val) : col_indx(val.first), value(val.second) {}
		TFilterData(size_t indx, const string& val): col_indx(indx), value(val) {}
		//����������
		~TFilterData() {}
		//�������� ������� ��������
		bool isEmpty() const noexcept(true) { return col_indx.isEmpty(); }
		//��������� �������� �������
		size_t getColIndx() const noexcept(true) { return col_indx.get(); }
		//��������� �������
		void setColIndx(size_t val) noexcept(true) { col_indx.set(val); }
		//��������� �������� ������� �������
		inline string getValue() const noexcept(true) { return value; }
		//��������� �������� ��� �������
		inline void setValue(const string& val) noexcept(true) { value = val; }
		//�������� ����������� ��������(�������� �� ����� - ����� ������� �� �����):
		bool operator==(const string& val) const noexcept(true);
		bool operator==(const TFilterData& src) const noexcept(true) { return operator==(src.value); }
		//��������� �������� �������/�������� � ���� ����:
		pair<size_t, string> getPair() const noexcept(true) { return std::make_pair(getColIndx(), getValue()); }
	};

	//��������� ����� ������ ��� ������ ������:
	class TShareData
	{
	private:
		string name;//��� �����
		size_t lst_indx;//������ ����� � �����
		size_t strt_row_indx;//������ ��������� ������
		vector<TFilterData> fltr;//������ ��� ����������
	public:
		//������������� ������ �����(��� �������� ����� excel), ������� �����(�������������� ������������ excel),
		//����� ������ �������(����������� � �������� excel) � ������ �������� ��������
		/*
		TShareData(const string& file = "", size_t ish = TIndex::EmptyIndex, size_t irow = TIndex::EmptyIndex,
			const vector<TFilterData>& filter = vector<TFilterData>()) : name(file), lst_indx(ish),
			strt_row_indx(irow), fltr(filter) {}
		/**/
		TShareData(const ptree& json, const NS_Const::JsonParams& param);
		//����������
		~TShareData() {}
		//�������� �� �������:
		inline bool isEmpty() const noexcept(true) { return name.empty() 
			|| lst_indx == TIndex::EmptyIndex || strt_row_indx == TIndex::EmptyIndex; }
		//��������� ����� �����:
		inline string getName() const noexcept(true) { return name; }
		inline void setName(const string& val) noexcept(true) { name = val; }
		//��������� ������ �������� � �����:
		inline size_t getListIndex() const noexcept(true) { return lst_indx; }
		inline void setListIndex(size_t val) noexcept(true) { lst_indx = val; }
		//��������� ������� ������ �������:
		inline size_t getFirstRow() const noexcept(true) { return strt_row_indx; }
		inline void setFirstRow(size_t val) noexcept(true) { strt_row_indx = val; }
		//�������� ������� �������:
		inline bool isEmptyFilter() const noexcept(true) { return fltr.size() == 0; }
		//��������� �������� ������� �� ������� � �������:
		vector<TFilterData> getFilter(size_t val) const noexcept(true) { return fltr; }
		void setFilter(const vector<TFilterData>& filter) noexcept(true) { fltr = filter; }
	};

	//��������� ������ �� ������������ ������ ��������� � ���������
	class TExcelCompareData
	{
	private:
		TShareData* DstFile;//����� ���������
		TShareData* SrcFile;//����� ���������
		vector<TCellData> cells;//������� �������������� ��������
		static TShareData* crtShareData(const ptree& json, const string& par_name) noexcept(false);
	public:
		//������������� json-������
		TExcelCompareData(const string& json_file);
		//���������������
		~TExcelCompareData();

	};
}

#endif TUNE_PARAM_H_