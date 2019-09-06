//#pragma once
#ifndef TUNE_PARAM_H_
#define TUNE_PARAM_H_
//������ ������������ ��� �������� ������ ����������� � ������ ��������
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <ctype.h>
#include <iterator>

//������� ��������� ��� ����������� ��������:
namespace Constant_Data
{
	using std::string;
	const char Space = ' ';
	const string delimeters(" \n\t,;");
	//��������� SQL �������
	namespace SQL_OPERATOR
	{
		const string Select = "select";
		const string From = "from";
		const string Where = "where";
		const string Order = "order by";
		const string As = "as";
		const string EndCommand = ";";
		const string And = "and";
		const string Or = "or";
		const string ColumnDelim = ",";
	}
	//��������� ����� ��������
	namespace Config_Param
	{
		const string DataDevider = "=";//����������� ������ �� ������������
		const string DataBase("[DATA BASE]");
		const string ReportParam("[REPORT]");
		const string ColumnsParam("[COLUMNS]");
		const string UserName("UserName");
		const string Pass("Password");
		const string ConnectionStr("TNS");
		const string ColumnName("Column");
	}
}

//������� ��������� ��� ������� ������ �� ��������:
namespace String_Fnc {};

//�������� ������������ ���� ��� ������ � �����������
namespace NS_Tune
{
	using std::string;
	using std::ifstream;
	using std::pair;
	using std::vector;

	//����� - ������ ��� �����������:
	class UserData
	{
	private:
		string name;
		string pass;
		string ConnStr;
	public:
		UserData(const string& par_name, const string& par_pass, const string& par_DB) :
			name(par_name), pass(par_pass), ConnStr(par_DB) {};
		//��������� ��������
		void setName(const string& val) { name = val; };
		void setPass(const string& val) { pass = val; };
		void setConnStr(const string& val) { ConnStr = val; };
		//����������� ��������
		string getName() const { return name; };
		string getPass() const { return pass; };
		string getConnStr() const { return ConnStr; };
	};

	class TColumn;
	class TRow;
	//��� ������ �������
	using TRowIterator = vector<TColumn>::iterator;
	using TRowConstIterator = vector<TColumn>::const_iterator;


	//������������ ����� ��� ������� � ������
	class TSection
	{
		private:
			string name;
		public:
			TSection(const string& val) : name(val) {};
			string toString() const { return name; };
	};

	//�������� ������ �������
	class TColumn
	{
		private:
			string title;
			string name;
		public:
			TColumn(const string& p_title = "", const string& p_name = "") : title(p_title), name(p_name) {};
			TColumn(const std::pair<string, string>& p) : title(p.first), name(p.second) {};
			std::pair<string, string> getPair() const { return std::make_pair(title, name); };
			bool isEmpty() const { return title.empty() && name.empty(); };
			string getName() const { return name; };
			string getTitle() const { return title; };
			string toString(const string& d) const;
			string toString(const char d) const { return toString(std::string{ d }); };
	};

	//�������� ������ ������:
	class TRow
	{
		private:
			string name;
			vector<TColumn> row;
			void InitRow(const string::iterator& first, const string::iterator& second);
		public:
			TRow(): name(), row() {};
			//������������� ����� ���������� ������
			TRow(const string& bblock, const string& eblock);
			void AddCol(const TColumn& col) { row.push_back(col); };
			void Clear() { row.clear(); };
			bool isEmpty() const { return row.empty(); };
			TRowConstIterator Begin() const { return row.begin(); };
			TRowConstIterator End()	const { return row.end(); };
			string toString(const string& row_delimeter, const string& col_delimeter) const;
			string toString(const char& row_delimeter, const char& col_delimeter) const { return toString(string{ row_delimeter }, string{ col_delimeter }); };
	};

	//����� ��� ������ � SQL ��������(��� ��������� ��������!):
	class SQL_Text
	{
	private:
		TRow slct_sec;//������ � ��������� ������� SELECT
		TRow frm_sec;//������ � �������� FROM
		string whr_sec;//������ ������� WHERE
		string ord_sec;//������ ���������� ORDER
		//�������� ������ �� �������
		//static string TRow
		//������������� �������
		void init_by_str(const string& str);
	public:
		//�������� ��� ������� where
		SQL_Text(const string& str = "");
		//���������� ������ �������
		TRow getColumns() const { return slct_sec; };
		//���������� ������ ������
		TRow getTables() const { return frm_sec; };
		//���������� ������ �������
		string getWhereClause() const { return whr_sec; };
		//���������� ������ ����������:
		string getOrderClause() const { return ord_sec; };
		//���������� ������� ��� �������:
		void AddWhereCond(const string& val, const string& opr);
		//�������� ������� ��� �������
		void ClearWhereSec() { whr_sec = ""; };
		//���������� ������� ��� ����������
		void AddOrderVal(const string& val);
		//�������� ������� ��� ����������:
		void ClearOrderSec() { ord_sec = ""; };
		//������������� sql-������� � ���� ������
		string ToString() const;
	};

	//����� ������ � ������ ��������
	class Tune
	{
	private:
		//������������ ����� � ������� ��������
		string src_file;
		//������������ ��������� �����
		string out_file;
		//������ ������������ ��� �����������
		UserData* usr_data;
		//����� ������� ��� ������
		SQL_Text* rep_SQL;
		//������ ��������� �������� ��� ������
		TRow* rep_cols;
		//������� ������������� ������ �� ����� ��������
		void read_tune_file(const string& file_name) noexcept(false);
	public:
		//����������� �� ���������
		Tune(const string& file_name = "");
		//����������
		~Tune();
		//��������� ����� �����:
		string getSrcName() const { return src_file; };
		//��������� ������ ��� �������
		string getSQL(void) const { return rep_SQL->ToString(); };
		//��������� ������� ��� �������
	};

}

namespace String_Fnc
{
	using std::string;
	using NS_Tune::TRow;
	using TStrings = std::vector<string>;
	using TStringsIterator = TStrings::iterator;
	using TStringsConstIterator = TStrings::const_iterator;
	using TInterval = std::pair<TStrings::iterator, TStrings::iterator>;
	//������� �������������� ������ � ������ ����� �� ���� ���������� ����� �� ��������� ��������:
	TStrings splitStr(const string& str, const string& delimen = Constant_Data::delimeters);
	//������� ��������� ��������� ��� �����������:
	TInterval getInterval(const TStringsIterator& ibeg, const TStringsIterator& iend,
		const string& op_beg, const string& op_end);
	//������� ���������� �������/�����
	TRow fill_sec(const TInterval& range);
	//��������� ������ �� ��������� ������
	string fill_str(const TInterval& range);
}

#endif TUNE_PARAM_H_