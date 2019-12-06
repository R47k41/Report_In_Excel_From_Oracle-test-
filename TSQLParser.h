#ifndef  TSQL_PARSER_H_
#define TSQL_PARSER_H_
#include <string>
#include <vector>
#include <iterator>
#include <ostream>
#include <utility>
#include "TConstants.h"

namespace NS_Sql
{
	using std::string;
	using std::vector;
	using std::pair;
	using NS_Const::TSql;
	using NS_Const::CtrlSym;
	using NS_Const::TConstSql;
	using NS_Const::TConstCtrlSym;
	using NS_Const::TSymGroup;

	//������� ����� ��� �������� sql-��������:
	class TSimpleSql
	{
	private:
		string text;//����� �������
	protected:
		//������� ��� ���������� �� ��������:
		enum class Side { Left, Right, Full };
		enum class KeyCase {Lower, Upper};
		//��������� ��������� ��� sql-�������
		void set_data(const string& sql, bool use_trim = true);
		//��������� ������ � sql-�������:
		string get_data() const noexcept(true) { return text; }
		//������� ������ ��������� ����� � ������:
		static std::size_t find_word_pos(const string& str, const string& key, const size_t n, bool must_find = false);
		//�������� �������� �� ������ �����������
		static bool isCommentLine(const string& str) noexcept(true);
	public:
		//��������� �������������
		TSimpleSql() :text() {}
		//������������� �������
		explicit TSimpleSql(const string& txt, bool use_trim = false);
		//������������� ������
		explicit TSimpleSql(std::istream& file);
		TSimpleSql(const TSimpleSql& sql) : text(sql.text) {}
		//�������� �� �������:
		virtual bool Empty() const { return text.empty(); }
		//��������� ������ �������:
		virtual void Data(const string& val) { set_data(val); }
		//��������� ������ �������:
		virtual string Data() const noexcept(true) { return get_data(); }
		//������� ������:
		virtual void clear() { text.clear(); }
		//������� ������� �� ����� :
		void Trim(const Side& flg = Side::Full);
		//������ ������� ������:
		string toCase(const KeyCase& flg) const;
		//����������:
		TSimpleSql& operator=(const string& val) { set_data(val); }
		//������� ������� ���������� � �������:
		bool hasParams() const;
		//������� ���������� sql-���� � �� �����:
		//������� ����������� �������� ������ ��� �������� �����������
		//���������� ������ ���������� � ������ ������
		static string read_sql_file(std::istream& file, bool skip_coment = true) noexcept(false);
		//������� �������������� � ������:
		friend string AsString(const TSimpleSql& sql);
	};

	//����� ����������� ������� �������� with/select/from/where/order by/group by
	class TSection: public TSimpleSql
	{
	private:
		TConstSql name;//������������ �����
		//������� ������������ ������ ����� ��������� �������:
		static string get_data_by_key_range(const string& str, size_t& pos, const TConstSql& br,
			const TConstSql& er) noexcept(false);
		//������� ���������� ���� � ������:
		void add_field_to_data(const string& str, const string& ch, bool use_brkt) noexcept(false);
		//������ ��������� ����� �� ������ �� ���� data:
		void set_fields() noexcept(true);
	protected:
		//��������� ������ ��� ���������� sql-������������ ��� ������(select: pair("select", "from"))
		inline string get_first_range_val(void) const { return name.toStr(); };
		inline string get_second_range_val(void) const { return TConstSql(name+1).toStr(); };
	public:
		//������������� � ������� sql-�������
		explicit TSection(const TSql& title, const string& str = "");
		explicit TSection(const TSql& title, const TSimpleSql& slq);
		explicit TSection(const TConstSql& title, const string& str = "");
		//����� �������� ����������� �������������
		TSection(const TSection& sect) : TSimpleSql(sect.Data()), name(sect.name) {};
		//����� �������� ����������� �����������:
		TSection& operator=(const TSection& s);
		//����������
		~TSection(void) {};
		bool operator<(const TSection& val) const { return name < val.Title(); };
		//��������� �� �������:
		bool Empty(void) const { return TSimpleSql::Empty() || name.isEmpty(); };
		//���������� ���� ������:
		//str - ������ ������, ch - ����������� ��� ����� ������,
		//use_brkt - ������������� ������ ��� ��������� ������ ����� ������ �� �����
		void AddField(const string& str, const TConstSql& ch, bool use_brkt = false) noexcept(false);
		TConstSql Title(void) const { return name; };
		string Name(void) const { return name.toStr(); };
		//�������������� � ������
		string to_Str(void) const;
		//������� ������� ������:
		void clear() { TSimpleSql::clear(); name = TSql::Empty; }
		friend vector<string> getColumnValue(const TSection& section);
	};
	
	//����� ��� ������ � sql-�������
	class TText
	{
	public:
		using TSectIndex = vector<TSection>::iterator;
		using TConstSectIndex = vector<TSection>::const_iterator;
	private:
		vector<TSection> sect;
		//������� ������������� �������� �� ������:
		void Init_Sectors(const string& str);
		//������� ���������/������ ����� ������� �� ����� select/from/where
		TSectIndex operator[](const TConstSql& title);
	public:
		//������������� �������
		TText(const string& str="");
		//������������� �� ������ ������
		TText(std::istream& stream);
		TText(const vector<TSection>& val) : sect(val) {};
		TText(const TText& val) : sect(val.sect) {};
		//TText& operator=(const TText& val);
		//���������������
		~TText() {};
		//�������� �� �������:
		bool isEmpty() const { return sect.empty(); };
		//������� ��������� ������ � ������:
		TSection operator[](const TConstSql& title) const;
		//�������� ����� �������:
		string toStr(bool use_eoc = false) const;
		//������� ���������� ����� �������(+):
		void AddSection(const TSql& title, const string& str);
		void AddSection(const TSection& sect);
		//������� �������� ����� �������(+):
		bool DelSection(const TSql& title) noexcept(true);
		//������� ���������� ������ � ���� �������(+):
		bool AddField2Section(const TSql& title, const string& str,
			const TSql& delimeter = TSql::Empty, bool brkt = false) noexcept(false);
		//������� �������������� � ������:
		friend string AsString(const TText& sql);
	};

	//�������������� ������������� ������ � ������:
	std::string AsString(const TText& sql);
	//��������� ������ �� ������:
	std::string AsString(const TSimpleSql& slq);
}
#endif // ! TSQL_PARSER_H_

