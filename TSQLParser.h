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


	//����� ����������� ������� �������� with/select/from/where/order by/group by
	class TSection
	{
	private:
		//�� ����� �� ����������� �������� ������ ������� ������ � ������
		//using find_function = std::size_t (std::string::*)(const string& s, const size_t pos) const;
		TConstSql name;//������������ �����
		string data;//������ ������� - ������ sql-������
		//������� ������ ��������� ����� � ������:
		static std::size_t find_word_pos(const string& str, const string& key, const size_t n, bool must_find = false);
		//������� ������������ ������ ����� ��������� �������:
		static string get_data_by_key_range(const string& str, size_t& pos, const TConstSql& br,
			const TConstSql& er) noexcept(false);
		//������� ���������� ���� � ������:
		void add_field_to_data(const string& str, const string& ch, bool use_brkt) noexcept(false);
		//������� ��������� ������ ��� ���� data
		void set_data(const string& str);
		//������ ��������� ����� �� ������ �� ���� data:
		void set_fields(const string& str) noexcept(true);
		void clear(void);
	protected:
		//��������� ������ ��� ���������� sql-������������ ��� ������(select: pair("select", "from"))
		inline string get_first_range_val(void) const { return name.toStr(); };
		inline string get_second_range_val(void) const { return TConstSql(name+1).toStr(); };
	public:
		//������������� � ������� sql-�������
		explicit TSection(const TSql& title, const string& str = "");
		explicit TSection(const TConstSql& title, const string& str = "");
		//����� �������� ����������� �������������
		TSection(const TSection& sect) : name(sect.name), data(sect.data) {};
		//����� �������� ����������� �����������:
		TSection& operator=(const TSection& s);
		//����������
		~TSection(void) {};
		bool operator<(const TSection& val) const { return name < val.Title(); };
		//��������� �� �������:
		bool Empty(void) const { return name.isEmpty() || data.empty(); };
		//���������� ���� ������:
		//str - ������ ������, ch - ����������� ��� ����� ������,
		//use_brkt - ������������� ������ ��� ��������� ������ ����� ������ �� �����
		void AddField(const string& str, const TConstSql& ch, bool use_brkt = false) noexcept(false);
		TConstSql Title(void) const { return name; };
		string Name(void) const { return name.toStr(); };
		string Data(void) const { return data; };
		void Data(const string& str) { set_data(str); }
		//�������������� � ������
		string to_Str(void) const;
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
	};
}
#endif // ! TSQL_PARSER_H_

