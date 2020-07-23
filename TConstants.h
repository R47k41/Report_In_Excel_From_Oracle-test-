#ifndef  TCONSTANTS_H_
//� ������ ����� ����������� ��� ����� ��������� ��� ������ ���������:
#define TCONSTANTS_H_

#include <string>
#include <vector>
#include <utility>

//������� ��������� ��� ����������� ��������� ���� enum class:
namespace NS_Const
{
	using std::string;

	const int EmptyType = -1;

	//������� ������ ����� ��, ��� � ����� ��������
	enum class TuneField {
		Empty = 0,
		//��������� ������ ��������
		Shared, Paths, DataBase, Report, SqlParams, Columns, Block_End,
		//������ ��� ������ ����������:
		Start_Shared_Index,
		//����� ��������� �������:
		AddDateToOutFileName, AddDateToSheetName, AddDateToOutPath,
		//�������� ����� ������������ ������
		MainPath, ConfigPath, ConfigFileExt, SubTunePath, SubTuneFileExt, SqlPath, SqlFileExt, TemplatePath, TemplateFileExt,
		OutDirectory, OutFileName, 
		End_Shared_Index,
		Start_Unq_Tune,
		//���� ������
		UserName, Password, TNS,
		//�����
		SheetName, TemplateName,
		//������
		SqlFirst, SqlFile, SqlText, DMLFile, DMLText, ClearSLQText, ClearSQLFile, 
		SqlParam, Column,	SqlParamQuane, SqlParamType, SqlParamNote, SqlParamValue, UseSqlParser,
		End_Unq_Tune,
		Last
	};

	//��������� ��� ��������� excel-������(��� ���������/�������)
	enum class JsonParams {Null, False, True,
		//���� ��� �������� ��������
		ObjBegin, DstFile, Sheet, Cells, SrcFile, DataArr, Method, DB_Config, ObjEnd,
		//���� ��� ������ � ������� ����
		FileBegin, name, list_index, col_id, first_row, last_row, filter, FileEnd,
		//���� ��� ������ � ������� ������
		FilterBegin, column_index, operation, value, FilterEnd,
		//���� ��� ������ ������� �����
		MethodBegin, code, color_if_found, color_not_found, fill_type, MethodEnd,
		//���� ������ ��� ������� �������
		CellsBegin, dst_index, dst_insert_index, src_param_index, src_val_index, 
		in_data_type, out_data_type, CellsEnd,
		//���� ���������� ��� ����������:
		//����� �� ���������
		SM_Balance_Begin, iftrue, iffalse, currency, rates, SM_Balance_End,
		//������ ����������:
		SM_Imp_Begin, pattern, fields, empty_block, SM_Imp_End,
		Last
	};

	//������ ��������� excel-������ �� ��������� Json-����������
	enum class JSonMeth { Null, CompareRow, CompareCell, GetFromDB, SendToDB, GetRowIDByDB, InsertRowCompare, CompareCellChange, Last };
	//��� �������� � ������� ������� ��� json-�����
	enum class JsonFilterOper {Null, Equal, NotEqual, MoreThan, MoreEqualThan, LessThan, LessEqualThan, Like, 
		LikeNoCase, NotLike, StrEqualNoCase, isEmpty, NotEmpty, Last};
	//���� �������� ����� excel ��� ������� json-���������
	//������������ ��� �������� �� ������ ������ �� ������(�.�. ������� �� ��� ������ ������ ��� ������ ���������)
	enum class JsonCellFill {Null, CurCell, ID_All_Find, ID_More_One_Find, ID_And_CurCell, Last};

	//���� ������ ��� ���������� � ��������:
	enum class DataType { ErrorType = 0, String, Integer, Double, Date, Boolean, SQL_String, Last };

	//��������� ��� ������� ������ excel
	enum class TExclBaseTune { Empty = EmptyType, xlt, xls, xlsx, DefExt, DefName, DefSh, PageDelimiter, Last };

	//��������� ��� ����������� excel-����������
	//http://www.excelworld.ru/publ/help/char_and_rest/sheet_char_and_rest/37-1-0-99
	enum TExclConstraint {
		xls_max_col = 256,
		xlsx_max_col = 16385,
		xls_max_row = 65536,
		xlsx_max_row = 1048577
	};

	//��������� sql:
	enum class TSql { Empty = EmptyType, With, Select, From, Where, Order, Group, As, And, Or, EOC, D4L, Last };

	//����������� �������:
	enum class CtrlSym { 
		Empty = EmptyType, EOL, Null, Space, NL, EndCommand, semicolon, colon,  EndCol, point, 
		lbkt, rbkt, qlbkt, qrbkt, crwn, quotes, Tab, dash, quane, rangle, langle,
		//������������ ��� ����������� ����������
		dies_comment, minus_comment, dash_comment, Last };

	//��������� ��� ������ ������:
	enum class ReportCode: int
	{
		Empty = 0,
		RIB_DOCS_FOR_PERIOD,//������� ���������� �� ���� ��� (�������)
		DOCS_MF_SF_FOR_PERIOD,//������� ���������� ��� �� � �� ������� (����������)
		REPAYMENT_FOR_DATE,//������� �������� �� + �� (��������)
		POTREB_CRED_BY_FILE,//��������������� ������� �� �� ����� (��������)
		CRED_CASE_MF,//��������� �������� �� (��������)
		NBKI_NP,//������ �� ���. �����(��������)
		NBKI_JP,//������ �� ��. �����(��������)
		CLOSE_DAY,//�������� �������/������
		NBKI_APPLY,//���������� ������ �� ���� (��������) ����� ������ ������ � 3 �� 0
		BALANCE_LIST,//��������� �������� �� (��������)
		BALANCE_SUA,//��������� �������� ��� �������� � ���(��������)
		FULL_CRED_REPORT,//������ ��������� �������� (��������) ������� ����� �� �����
		FULL_CRED_REPORT_SUA,//������ �������� ��� ���(��������)
		LOAD_FROM_FILE,//�������� ���������� �� excel/xml/txt-����� (���������� �������� insert � ������� �� ��� ���� ���������)
		FILE_COMPARE_RIB,//��������� ������ excel
		FILE_COMPARE_RTBK,
		EXCEL_SET_DATA_FROM_BASE,//���������� ����� excel ����� �� ��
		EXCEL_DOC_LOAD,//�������� ���������� �� excel-�����(��� ���������)
		EXCEL_PAY_LOAD_SF,//�������� ������ �� excel-�����(��� ���������)
		EXCEL_PAY_LOAD_MF,//�������� ������ �� excel-�����(��� ���������)		
		ACCOUNT_BALANCE,//������� �� �����
		ACCOUNT_BALANCE_STREAM,//��������� �������(��������� excel-���� � �� ��� ��������� ��������� ������� �� ������� �����)
		LOTS,//����� �� ����� ��� ����������� �����
		SMLVCH_BALANCE,//��������� ����� ��������� ��������
		SMLVCH_IMP,//�������� ������ ����������
		QUIT_REPORT,//����� �� ������ ������ �������
		Last
		};

	//������� �������� �� ��������:
	//�������������� � ������ �������:
	string LowerCase(const string& str);
	string UpperCase(const string& str);
	//������� �� ������ ��������� �������:
	//void DeleteServiceSymb(string& str);
	//������� ��������� ������� �� ������ � ����� ������:
	void Trim_Left(string& str);
	void Trim_Right(string& str);
	void Trim(string& str);
	//������� ��������� �������� ��������� ����� � ������� ����� �����:
	char getNLSNumPoint() noexcept(true);
	//������� ���������� ��������� �������� �� ������� �����:
	double Round(double x, int sz = 100) noexcept(true);
	//����� ������� ������ � �����:
	namespace DateInteface
	{
		//��������� ������� ��� ������ ���� � ������:
		//�������� �� ���������� boost\date_time ��� ������������� date_facet
		//������������ �������� ������ 
		//https://www.boost.org/doc/libs/1_49_0/doc/html/date_time/date_time_io.html
		bool set_stream_date_format(std::ostream& stream, const string& fromat) noexcept(true);
		//�������������� ������� ���� � ������:
		string cur_date_to_string_by_format(const string& format) noexcept(false);
		//�������������� ����/������/��� � ������ �� ��������
		string from_date(int yy, size_t mm, size_t dd, const string& format = "%d.&m.%Y") noexcept(true);
		string from_date(double date_as_dbl, const string& format = "%d.&m.%Y") noexcept(true);
	};


	//������� ��������� ��� ����������� ��������:
	template <typename T, T min_val, T max_val>
	class TConstant
	{
	private:
		int val;//�������� ���������
		//������������:
		TConstant<T, min_val, max_val>& operator+=(int x) { val += x; return *this; }
	protected:
		//�������������:
		virtual void Init(const T& def_val = min_val) noexcept(true);
		//���������� � ���������
		virtual bool inRange(const T& a = min_val, const T& b = max_val) const noexcept(true) { return (Value() > a and Value() < b); };
		//�������� ����������
		virtual bool isValid(const T& a = min_val, const T& b = max_val, bool exit_on_error = false) const noexcept(false);
		//��������� �������� �� ����:
		void setValue(const T& x);
		//�������������
		void setValue(int x);
	public:
		explicit TConstant<T, min_val, max_val>(int x) { setValue(x); Init(); }
		explicit TConstant<T, min_val, max_val>(const T& x = min_val) { setValue(x); Init(); }
		//�������� ����������:
		virtual bool isValid(bool exit_on_error = false) const noexcept(false) { return isValid(min_val, max_val, exit_on_error); }
		//������� �������� � ������
		virtual string toStr() const = 0;
		//������� �������� � �����
		virtual int toInt() const { return val; }
		//��������� �������� ������� ����:
		T Value() const { return T(val); }
		//��������� �������� �� �������
		virtual bool operator==(const string& str) const { return toStr() == str; }
		//��������� ���������� ��������:
		virtual TConstant<T, min_val, max_val>& Next(bool exit_on_err = true) noexcept(false);
		//�������� �� �������:
		virtual inline bool isEmpty() const { return Value() == min_val; };
		//������� ����������:
		TConstant<T, min_val, max_val>& operator=(const T& x);
		//�������� ��������� � �����:
		virtual bool operator<(const T& x) const { return Value() < x; }
		virtual bool operator>(const T& x) const { return !(operator==(x) or operator<(x)); }
		virtual bool operator==(const T& x) const { return Value() == x; }
		virtual bool operator!=(const T& x) const { return !operator==(x); }
		virtual bool operator<=(const T& x) const { return operator<(x) or operator==(x); }
		//�������� ��������� ����� �����:
		bool operator<(const TConstant<T, min_val, max_val>& x) const { return val < x.val; }
	};

	//���������� �����:
	using TF_Const = TConstant<TuneField, TuneField::Empty, TuneField::Last>;
	using DF_const = TConstant<DataType, DataType::ErrorType, DataType::Last>;
	using EBT_Const = TConstant<TExclBaseTune, TExclBaseTune::Empty, TExclBaseTune::Last>;
	using RC_Const = TConstant<ReportCode, ReportCode::Empty, ReportCode::Last>;
	using SQL_Const = TConstant<TSql, TSql::Empty, TSql::Last>;
	using CS_Const = TConstant<CtrlSym, CtrlSym::Empty, CtrlSym::Last>;
	using JS_Const = TConstant<JsonParams, JsonParams::Null, JsonParams::Last>;
	using JS_Meth = TConstant<JSonMeth, JSonMeth::Null, JSonMeth::Last>;
	using JS_CellFill = TConstant<JsonCellFill, JsonCellFill::Null, JsonCellFill::Last>;
	using JS_FilterOper = TConstant<JsonFilterOper, JsonFilterOper::Null, JsonFilterOper::Last>;

	//���� �� ����� ��������:
	class TConstField : public TF_Const
	{
	public:
		explicit TConstField(const TuneField& x) : TF_Const(x) { }
		explicit TConstField(int x) : TF_Const(x) { }
		//�������� ���������� ��������� � ������:
		virtual bool StrInclude(const string& str) const;
		//������� �������� � ������
		static string description(const TuneField& code) noexcept(true);
		static string asStr(const TuneField& code);
		virtual string toStr() const { return asStr(Value()); };
		//��������� �������������� ��������� �� �� ����:
		static TuneField getIDByCode(const string& code, const TuneField& bval, const TuneField& eval);
		//�������� ����������:
		TConstField& operator=(const TuneField& x) { TF_Const::operator=(x); return *this; }
		friend bool operator==(const string& str, const TConstField& val) { val.operator==(str); }
	};

	//���� ����� ������:
	class TConstType : public DF_const
	{
	public:
		explicit TConstType(const DataType& x) : DF_const(x) {}
		explicit TConstType(int x) : DF_const(x) {};
		explicit TConstType(const string& str);
		//������� �������� � ������
		static string asStr(const DataType& code) noexcept(true);
		virtual string toStr() const noexcept(true) { return asStr(Value()); }
		//������� ��������� ���� ���� ������ �� ������������:
		static DataType getCodeByName(const string& name) noexcept(true);
		//�������� ����������:
		TConstType& operator=(const DataType& x) { DF_const::operator=(x); return *this; }
	};

	//���� ��� ������ � Excel:
	//����� ��� ������ � ����������� �����:
	class TConstExclTune : public EBT_Const
	{
	public:
		//�������������
		explicit TConstExclTune(const TExclBaseTune& x) : EBT_Const(x) { }
		explicit TConstExclTune(int x) : EBT_Const(x) { }
		//�������������� � ������
		static string asStr(const TExclBaseTune& val) noexcept(true);
		//�������������� � ������
		virtual string toStr() const { return asStr(Value()); };
		//�������� ����������
		TConstExclTune& operator=(const TExclBaseTune& x) { EBT_Const::operator=(x); return*this; }
		//��������� ���������� �����:
		static string getFileExtention(const string& val) noexcept(true);
		//������� ����������� �������� �� ���� - �������:
		static bool isTemplate(const TExclBaseTune& val) noexcept(true);
		//������� ��������� ���� ���������� �� ��� ����������� �������������:
		static TExclBaseTune getFileExtCode(const string& ext) noexcept(true);
		//�������� ���������� ���������� ��� excel-�����:
		static bool isValidExtensions(const string& val) noexcept(true);
		static bool isValidFileByExtension(const string& name) noexcept(true);
	};

	//����� ��� ������ � ������������ ������:
	class TConstReportCode : public RC_Const
	{
	public:
		explicit TConstReportCode(const ReportCode& x) : RC_Const(x) { }
		explicit TConstReportCode(int x) : RC_Const(x) { }
		//��������� ���� ������:
		string toStr() const;
		//��������� ������������ ������:
		string getName() const;
		//����������� ��������� �������:
		static void show() noexcept(true);
		//�������� ����������:
		TConstReportCode& operator=(int val) noexcept(true) { RC_Const::setValue(val); return *this; }
		TConstReportCode& operator=(const ReportCode& val) noexcept(false) { RC_Const::setValue(val); return *this; }
		//��������� �������������� ������ �� ����:
		static ReportCode getIDByCode(const string& code, const ReportCode& bval, const ReportCode& eval);
		friend bool operator==(const string& str, const TConstReportCode& val) { return val.operator==(str); }
	};

	//����� ������ ��������
	class TSymGroup
	{
	private:
		using Syms = std::pair<char, char>;
		using SymsArr = std::vector<Syms>;
		SymsArr arr;//������ ������� ��� �������� ������������ �� ��������
	public:
		//�������������
		explicit TSymGroup(const SymsArr& val) : arr(val) {};
		TSymGroup(bool set_default);
		//���������� ������� � ������:
		TSymGroup& operator+(const Syms& pair_val) noexcept(false) { arr.push_back(pair_val); return *this; };
		//�������� �� ������������ ��������� ������:
		bool IsCorrectSym(const std::string& str, std::size_t pos, const std::size_t cnt = 1) const noexcept(false);
	};

	//����� ��������� ��������� ��������:
	class TConstCtrlSym : CS_Const
	{
	public:
		explicit TConstCtrlSym(const CtrlSym& x) : CS_Const(x) { }
		explicit TConstCtrlSym(int x) : CS_Const(x) { }
		TConstCtrlSym& operator=(const CtrlSym& x) { CS_Const::operator=(x); return *this; }
		//����� �������������� � ������
		static string asStr(const CtrlSym& val);
		//����� �������������� � ������
		static char asChr(const CtrlSym& val) { return asStr(val)[0]; };
		//�������������� � ������
		string toStr() const { return asStr(Value()); };
		//�������������� � ������
		char toChar() const { return toStr()[0]; }
		string operator+(const string& str) const noexcept(true);
		//������������� ������� ������ �� ��������:
		friend string operator+(const string& str, const TConstCtrlSym& ch);
		friend string operator+(const TConstCtrlSym& ch, const string& str);
	};

	//����� ��� �������� sql-������:
	class TConstSql : public SQL_Const
	{
	public:
		explicit TConstSql(const TSql& x) :SQL_Const(x) { }
		explicit TConstSql(int x) : SQL_Const(x) { }
		TConstSql& operator=(const TSql& x) { SQL_Const::operator=(x); return *this; }
		string toStr() const;
		bool MustFound() const;
		//������� ��������� ����������� �� ������������
		TSql GetDelimeter() const noexcept(false);
		TConstSql GetDelimeterAsObj() const noexcept(false) { return TConstSql(GetDelimeter()); };
		//������� �������� ������������ ���������� �����������:
		bool CorrectDelimeter(const std::string& d) const noexcept(true);
		//������� ����������� ����������� ������������� ������ � �������:
		bool CanUseBrkt() const noexcept(true) { return Value() == TSql::Where; };
		//������� ������������ ����������� ������� �� �������:
		string getClosedElem() const noexcept(false);
		//�������� ��������� �� �������:
		bool operator==(const string& str) const noexcept(true) { return toStr() == str; };
		bool operator==(const TConstSql& val) const noexcept(true) { return Value() == val.Value(); }
		TConstSql operator+(int x) const noexcept(true);
		//����� ������ TCtrlSym � �����
		friend std::ostream& operator<<(std::ostream& stream, const TConstSql& val) noexcept(false);
	};

	//����� ��� ��������� �� �������� � �������� ���������:
	class TTrimObj
	{
	private:
		string symb;
	protected:
		string Symbols() const noexcept(true) { return symb; }
	public:
		TTrimObj(const string& arr) : symb(arr) {};
		TTrimObj(const TConstSql& title);
		//����������� � ���������:
		bool operator()(const char& ch) const;
	};

	string& operator<<(string& str, const JsonParams& param);
	
	//����� ��� ��������� Json-��������:
	class TConstJson : public JS_Const
	{
	private:
		static bool inRange(const JsonParams& val, const JsonParams& b, const JsonParams& e) noexcept(true);
	public:
		//�������������
		explicit TConstJson(const JsonParams& x) : JS_Const(x) {}
		explicit TConstJson(int x) : JS_Const(x) {}
		//����������:
		TConstJson& operator=(const JsonParams & x) { JS_Const::operator=(x); return *this; }
		//��������� ���������� �������� �� �������������:
		static string asStr(const JsonParams& val) noexcept(true);
		static string description(const JsonParams& val) noexcept(true);
		static bool isObjectTag(const JsonParams& val) noexcept(true) { return inRange(val, JsonParams::ObjBegin, JsonParams::ObjEnd); }
		static bool isFileTag(const JsonParams& val) noexcept(true) { return inRange(val, JsonParams::FileBegin, JsonParams::FileEnd); }
		static bool isFilterTag(const JsonParams& val) noexcept(true) { return inRange(val, JsonParams::FilterBegin, JsonParams::FilterEnd); }
		static bool isCellTag(const JsonParams& val) noexcept(true) { return inRange(val, JsonParams::CellsBegin, JsonParams::CellsEnd); }
		static bool isMethTag(const JsonParams& val) noexcept(true) { return inRange(val, JsonParams::MethodBegin, JsonParams::MethodEnd); }
		static bool isTag(const JsonParams& val) noexcept(true);
		static bool isBalanceTag(const JsonParams& val) noexcept(true) { return inRange(val, JsonParams::SM_Balance_Begin, JsonParams::SM_Balance_End); }
		static bool isImpDocsTag(const JsonParams& val) noexcept(true) { return inRange(val, JsonParams::SM_Imp_Begin, JsonParams::SM_Imp_End); }
		static string Concate(const std::vector<JsonParams>& arr) noexcept(true);
		string toStr() const noexcept(true) { return asStr(Value()); }
		//���������:
		bool operator==(const string& str) const noexcept(true) { return toStr() == str; }
		bool operator==(const JsonParams& val) const noexcept(true) { return Value() == val; }
		friend string& operator<<(string& str, const JsonParams& param);
	};

	//����� ��� �������� ������� ��������� ������
	class TConstJSMeth : public JS_Meth
	{
	public:
		explicit TConstJSMeth(const JSonMeth& x) : JS_Meth(x) {}
		explicit TConstJSMeth(int x) : JS_Meth(x) {}
		static string asStr(const JSonMeth& val) noexcept(true);
		string toStr() const noexcept(true) { return asStr(JS_Meth::Value()); };
		TConstJSMeth& operator=(const JSonMeth& x) noexcept(true) { JS_Meth::operator=(x); return *this; }
		TConstJSMeth& operator=(size_t x) noexcept(true) { JSonMeth val = JSonMeth(x); return operator=(val); }
		//�������� �� ������� ������� SrcFile � json-����������
		bool HasSrcFileObj() const noexcept(false);
		};

	//����� ��� �������� ����� ������� �����
	class TConstJSCellFill : public JS_CellFill
	{
	public:
		//�������������
		TConstJSCellFill(const JsonCellFill& x = JsonCellFill::Null) : JS_CellFill(x) {}
		explicit TConstJSCellFill(const int x) : JS_CellFill(x) {}
		explicit TConstJSCellFill(size_t x) : JS_CellFill(JsonCellFill(x)) {}
		//����������� �������
		static string asStr(const JsonCellFill& val) noexcept(true);
		string toStr() const noexcept(true) { return asStr(JS_CellFill::Value()); }
		//����������
		TConstJSCellFill& operator=(const JsonCellFill& x) noexcept(true) { JS_CellFill::setValue(x); return *this; }
		TConstJSCellFill& operator=(size_t x) noexcept(true) { JsonCellFill val = JsonCellFill(x); return operator=(val); }
	};

	//����� ��� �������� �������� ����������
	class TConstJSFilterOper: public JS_FilterOper
	{
		protected:
/*
			//������� ���������� ������� ��������:
			template <typename Type>
			static bool runBaseOperation(const Type& val1, const Type& val2,
				const NS_Const::JsonFilterOper& oper_code) noexcept(true);
			//������� ���������� �������� ��� ����� ����������:
			template <typename Type>
			static bool RunOperation(const Type& val1, const Type& val2,
				const NS_Const::JsonFilterOper& oper_code) noexcept(true);
/**/
		public:
			//�������������
			TConstJSFilterOper(const JsonFilterOper& x = JsonFilterOper::Null) : JS_FilterOper(x) {}
			explicit TConstJSFilterOper(const int x) : JS_FilterOper(x) {}
			explicit TConstJSFilterOper(size_t x) : JS_FilterOper(JsonFilterOper(x)) {}
			//����������� �������
			static string asStr(const JsonFilterOper& val) noexcept(true);
			string toStr() const noexcept(true) { return asStr(JS_FilterOper::Value()); }
			//����������
			TConstJSFilterOper& operator=(const JsonFilterOper& x) noexcept(true) { JS_FilterOper::setValue(x); return *this; }
			TConstJSFilterOper& operator=(size_t x) noexcept(true) { JsonFilterOper val = JsonFilterOper(x); return operator=(val); }
			static bool DoubleBaseOperation(double val1, double val2, const NS_Const::JsonFilterOper& oper_code)
				noexcept(true);
			static bool BoolBaseOperation(bool val1, bool val2, const NS_Const::JsonFilterOper& oper_code)
				noexcept(true);
			static bool IntBaseOperation(int val1, int val2, const NS_Const::JsonFilterOper& oper_code)
				noexcept(true);
			static bool StringBaseOperation(const string& val1, const string& val2, 
				const NS_Const::JsonFilterOper& oper_code) noexcept(true);
	};

}

#endif // ! TCONSTANTS_H_

