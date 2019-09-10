//#pragma once
#ifndef TUNE_PARAM_H_
#define TUNE_PARAM_H_
//������ ������������ ��� �������� ������ ����������� � ������ ��������
#include <fstream>
#include <string>
//#include <vector>
#include <map>
#include <utility>

//�������� ������������ ���� ��� ������ � �����������
namespace NS_Tune
{
	using std::string;
	using std::ifstream;
	using std::pair;
//	using std::vector;
	using std::map;
	using find_fnc = size_t (string::*)(const string& s, size_t pos) const;
	//����� ��� ������������ ����� ��������(���������)
	//������� ������ ����� ��, ��� � ����� ��������
	enum class TuneField {Empty = 0, DataRange,
		DataBase, Report, Columns,
		UserName, Password, TNS, OutFile, SqlFile, SqlText,	Column};
	//�������������� � ������:
	string TuneFieldToStr(const TuneField& val);
	//�������������� � �����:
	inline int TuneFieldToInt(const TuneField& val) { return int(val); };
	inline TuneField operator+(const TuneField& val, int x) { return TuneField(TuneFieldToInt(val) + x); };
	inline TuneField operator+=(TuneField& val, int x) { return val = val + x; };
	bool operator==(const TuneField& val, const string& str);
	size_t get_pos_in_str(const string& str, const TuneField& substr, const size_t beg = 0, find_fnc = &string::find);
	string Get_TuneFiel_Val_From_Str(const TuneField& f, const string& str);
	
	using TField = std::pair<TuneField, std::string>;
	using TFields = std::map<TuneField, string>;

	//����� - ������ ��� �����������:
	class TUserData
	{
	private:
		TFields fields;
		std::vector<string> cols;
		//������� ��������� �������� �� ���������
		void set_default_fields_val();
		//������� ������ �������� �������
		void Read_Col_Val(ifstream& file);
		//������� ������ �������� �������� �� �����:
		void Read_Tune_Val(ifstream& file);
		//������ �������� �� �����:
		void ReadFromFile(const string& file);
		TUserData(const TFields& v);
		TUserData(const TUserData& v);
		TUserData& operator=(const TUserData& v);
		//��������� ������ �� ��������:
		string& operator[](const TuneField& code) noexcept(false) { return fields.at(code); };
		string operator[](const TuneField& code) const noexcept(false) { return fields.at(code); };
	public:
		//������ �������� ����������� �� �����
		TUserData(const string& tunefile);
		~TUserData() {};
		//�������� �� �������:
		bool EmptyFields(void) const { return fields.empty(); };
		//�������� ������ �� ������ �������
		bool EmptyColumns(void) const { return cols.empty(); };
		//��������� ��������:
		string getValue(const TuneField& code) const;
		//��������� ��������:
		void operator()(const TuneField& code, const string& val);
		void setValue(const TuneField& code, const string& val) { operator()(code, val); };
		//������� ����������� ������ ��������:
		void show_tunes(void) const;
		//������� ����������� ������ �������:
		void show_columns(void) const;
		//��������� ������ �������:
		//vector<string> getColumnsTitle(void) const;
	};

}


#endif TUNE_PARAM_H_