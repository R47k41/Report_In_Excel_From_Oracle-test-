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
		MainPath, ConfigPath, ConfigFileExt, SqlPath, SqlFileExt, TemplatePath, TemplateFileExt,
		OutDirectory, OutFileName, 
		End_Shared_Index,
		Start_Unq_Tune,
		//���� ������
		UserName, Password, TNS,
		//�����
		SheetName, TemplateName,
		//������
		SqlFile, SqlText, SqlParam, Column,
		SqlParamQuane, SqlParamType, SqlParamNote, SqlParamValue, UseSqlParser,
		End_Unq_Tune,
		Last
	};

	//���� ������ ��� ���������� � ��������:
	enum class DataType { ErrorType = 0, String, Integer, Double, Date, Boolean, Last };

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
		dies_comment, minus_comment, dash_comment,
		Last };

	//��������� ��� ������ ������:
	enum class ReportCode: int
	{
		Empty = EmptyType,
		RIB_DOCS_FOR_PERIOD = 0,//������� ���������� �� ���� ��� (�������)
		DOCS_MF_SF_FOR_PERIOD,//������� ���������� ��� �� � �� ������� (����������)
		REPAYMENT_FOR_DATE,//������� �������� �� + �� (��������)
		POTREB_CRED_BY_FILE,//��������������� ������� �� �� ����� (��������)
		CRED_CASE_MF,//��������� �������� �� (��������)
		NBKI,//������ ��� �������� � ���� (��������) ��� � ���� ����
		CLOSE_DAY,//�������� �������/������
		NBKI_APPLY,//���������� ������ �� ���� (��������) ����� ������ ������ � 3 �� 0
		BALANCE_LIST,//��������� �������� �� (��������)
		FULL_CRED_REPORT,//������ ��������� �������� (��������) ������� ����� �� �����
		LOAD_FROM_FILE,//�������� ���������� �� excel/xml/txt-�����
		FILE_COMPARE,//��������� ������ excel
		Last
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


	//���� �� ����� ��������:
	class TConstField : public TF_Const
	{
	public:
		explicit TConstField(const TuneField& x) : TF_Const(x) { }
		explicit TConstField(int x) : TF_Const(x) { }
		//�������� ���������� ��������� � ������:
		virtual bool StrInclude(const string& str) const;
		//������� �������� � ������
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
		virtual string toStr() const;
		//�������� ����������:
		TConstType& operator=(const DataType& x) { DF_const::operator=(x); return *this; }
	};

	/*
	class TConstTag : public Tag_Const
	{
	public:
		explicit TConstTag(const Tags& x) : Tag_Const(x) { }
		explicit TConstTag(int x) : Tag_Const(x) { }
		//������� �������� � ������
		virtual string toStr() const;
		//�������� ����������:
		TConstTag& operator=(const Tags& x) { Tag_Const::operator=(x); return *this; }
	};
	/**/
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
		//�������� ���������� ���������� ��� excel-�����:
		static bool isValidExtensions(const string& val) noexcept(true);
	};

	//����� ��� ������ � ������������ ������:
	class TConstReportCode : public RC_Const
	{
	public:
		explicit TConstReportCode(const ReportCode& x) : RC_Const(x) { }
		explicit TConstReportCode(int x) : RC_Const(x) { }
		TConstReportCode& operator=(const ReportCode& x) { RC_Const::operator=(x); return *this; }
		//��������� ���� ������:
		string toStr() const;
		//��������� ������������ ������:
		string getName() const;
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

}

#endif // ! TCONSTANTS_H_

