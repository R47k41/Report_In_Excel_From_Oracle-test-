#ifndef  TSQL_PARSER_H_
#define TSQL_PARSER_H_
#include <string>
#include <vector>
#include <iterator>
#include <ostream>
#include <utility>

namespace NS_Sql
{
	using std::string;
	using std::vector;
	using std::pair;

	//����� ����������� �����:
	struct TCtrlGroup
	{
		vector<pair<char, char> > excluded_symb;
		//��������� sql:
		enum class TCtrlSql { Empty = 0, With, Select, From, Where, Order, Group, As, And, Or, EOC, D4L};
		//����������� �������:
		enum class TCtrlSym {EOL = '\0', Space = ' ', NL = '\n', EndCommand = ';', EndCol = ',', lbkt = '(', rbkt = ')', crwn = '"', Tab = '\t' };
		//�������������� sql ������� � ������
		static string CtrlSql2Str(const TCtrlSql& val);
		//�������������� ������������ ������� � ������:
		static inline char CtrlSym2Char(const TCtrlSym& s) { return static_cast<char>(s); };
		static string CtrlSym2Str(const TCtrlSym& ch);
		//�������� �������������� ������� ������� � �������:
		static bool MustFound(const TCtrlSql& s);
		//������� ������������ �����������
		static TCtrlGroup::TCtrlSql GetDelimeterByTitle(const TCtrlGroup::TCtrlSql& title) noexcept(false);
		//������� ��������� ����������� �� ������������
		static bool CorrectDelimeter(const TCtrlSql& title, const string& d);
		//������� ����������� ����������� ������������� ������ � �������:
		static bool CanUseBrkt(const TCtrlSql& title) { return title == TCtrlSql::Where; };
		//�������������
		TCtrlGroup();
		//�������� �� ������������ ��������� ������:
		bool IsCorrectSym(const string& str, std::size_t pos, const std::size_t cnt = 1);
		//����� ������ TCtrlSym � �����
		friend std::ostream& operator<<(std::ostream& stream, const TCtrlSym& val);
	};

	//������� ���������� ��� TCtrlSql:
	TCtrlGroup::TCtrlSql operator+(const TCtrlGroup::TCtrlSql& val, int x) noexcept(true);

	//����� ��������� ���� ���� ��� �������/������(d.ID as "ID" -> val = d.ID; delimeter = as; title = "ID")
	class TField
	{
	private:
		string val;//�������� ����
		string title;//������������ ����
		string delimeter;//����������� �����
		//������������� �� ������ � �����������
		virtual void Init_By_Str(const string& str, const string& d) noexcept(false);
	public:
		//����������� �� ��������:
		TField(const string& par_val = "", const string& par_title = "", const string& par_delimeter = "") :
			val(par_val), title(par_title), delimeter(par_delimeter) {};
		TField(const TField& f) : val(f.val), title(f.title), delimeter(f.delimeter) {};
		//������������� �� ������ � sql-�������:
		TField(const string& str, const TCtrlGroup::TCtrlSql& d) noexcept(false);
		//������������� �� ������ � �����������:
		TField(const string& str, const TCtrlGroup::TCtrlSym& ch) noexcept(false);
		string Val() const { return val; };
		string Title() const { return title; };
		string Delimeter() const { return delimeter; };
		void Val(const string& par_val) { val = par_val; };
		void Title(const string& par_title) { title = par_title; };
		void Delimeter(const string& par_delimeter) { delimeter = par_delimeter; };
		TField& operator=(const TField& f) { val = f.val; title = f.title; delimeter = f.delimeter; return *this; };
		//�������� ������ ������
		bool Empty(void) const { return val.empty() && title.empty() && delimeter.empty(); };
		~TField(void) {};
		//�������������� � ������
		string to_Str(void) const;
	};
	
	using TFields = std::vector<TField>;

	//����� ����������� ������� �������� with/select/from/where/order by/group by
	class TSection
	{
	private:
		//�� ����� �� ����������� �������� ������ ������� ������ � ������
		//using find_function = std::size_t (std::string::*)(const string& s, const size_t pos) const;
		TCtrlGroup::TCtrlSql name;//������������ �����
		string data;//������ ������� - ������ sql-������
		//������� ������ ��������� ����� � ������:
		static std::size_t find_word_pos(const string& str, const string& key, const size_t n, bool must_find = false);
		//������� ���������� ���� � ������:
		void add_field_to_data(const string& str, const string& ch, bool use_brkt) noexcept(false);
		//������� ��������� ������ ��� ���� data
		void set_data(const string& str);
		//������ ��������� ����� �� ������ �� ���� data:
		void set_fields(const TCtrlGroup::TCtrlSql& title, const string& str) noexcept(false);
		void clear(void);
	protected:
		//��������� ������������ �� ������������ sql-�����: (select -> "select")
		void set_by_sql_name(const TCtrlGroup::TCtrlSql& title) { name = title; };
		//��������� ������ ��� ���������� sql-������������ ��� ������(select: pair("select", "from"))
		inline string get_first_range_val(void) const { return TCtrlGroup::CtrlSql2Str(name); };
		inline string get_second_range_val(void) const { return TCtrlGroup::CtrlSql2Str(name + 1); };
	public:
		//������������� � ������� sql-�������
		TSection(const TCtrlGroup::TCtrlSql& title, const string& str);
		//����� �������� ����������� �������������
		TSection(const TSection& sect) : name(sect.name), data(sect.data) {};
		//����� �������� ����������� �����������:
		TSection& operator=(const TSection& s);
		//����������
		~TSection(void) {};
		bool operator<(const TSection& val) const { return name < val.Title(); };
		//��������� �� �������:
		bool Empty(void) const { return name == TCtrlGroup::TCtrlSql::Empty || data.empty(); };
		//���������� ���� ������:
		//str - ������ ������, ch - ����������� ��� ����� ������,
		//use_brkt - ������������� ������ ��� ��������� ������ ����� ������ �� �����
		void AddField(const string& str, const TCtrlGroup::TCtrlSql& ch, bool use_brkt = false) noexcept(false);
		TCtrlGroup::TCtrlSql Title(void) const { return name; };
		string Name(void) const { return TCtrlGroup::CtrlSql2Str(name); };
		string Data(void) const { return data; };
		void Data(const string& str) { set_data(str); }
		//�������������� � ������
		string to_Str(void) const;
	};
	
	//����� ��� ������ � sql-�������
	class TText
	{
	public:
		using TSectIndex = vector<TSection>::iterator;
		using TConstSectIndex = vector<TSection>::const_iterator;
	private:
		vector<TSection> sect;
		TText(const vector<TSection>& val);
		TText(const TText& val);
		TText& operator=(const TText& val);
		//������� ������������� �������� �� ������:
		void Init_Sectors(const string& str);
		//������� ���������/������ ����� ������� �� ����� select/from/where
		TSectIndex operator[](const TCtrlGroup::TCtrlSql& title);
	public:
		//������������� �������
		TText(const string& str);
		//������������� �� ������ ������
		TText(std::istream& stream);
		//���������������
		~TText() {};
		//�������� ����� �������:
		string toStr(void) const;
		//������� ���������� ����� �������(+):
		void AddSection(const TCtrlGroup::TCtrlSql& title, const string& str);
		void AddSection(const TSection& sect);
		//������� �������� ����� �������(+):
		bool DelSection(const TCtrlGroup::TCtrlSql& title) noexcept(true);
		//������� ���������� ������ � ���� �������(+):
		bool AddField2Section(const TCtrlGroup::TCtrlSql& title, const string& str,
			TCtrlGroup::TCtrlSql delimeter = TCtrlGroup::TCtrlSql::Empty, bool brkt = false) noexcept(false);
	};
}
#endif // ! TSQL_PARSER_H_

