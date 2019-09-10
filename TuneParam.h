//#pragma once
#ifndef TUNE_PARAM_H_
#define TUNE_PARAM_H_
//������ ������������ ��� �������� ������ ����������� � ������ ��������
#include <string>
//#include <vector>
#include <map>
#include <utility>

//�������� ������������ ���� ��� ������ � �����������
namespace NS_Tune
{
	using std::string;
//	using std::ifstream;
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
	inline TuneField operator+=(const TuneField& val, int x) { return val + x; };
	bool operator==(const TuneField& val, const string& str);
	size_t get_pos_in_str(const string& str, const TuneField& substr, const size_t beg = 0, find_fnc = &string::find);
	string Get_TuneFiel_Val_From_Str(const TuneField& f, const string& str);
	
	//����� ��� ������ ����� ��������???
	class TTuneField
	{
		private:
			TuneField title;
			string value;
		public:
			TTuneField(const TuneField& code, const string& val) : title(code), value(val) {};

	};

	//using TField = std::pair<std::string, std::string>;
	using TFields = std::map<TuneField, string>;

	//����� - ������ ��� �����������:
	class TUserData
	{
	private:
		TFields fields;
		//������� ��������� �������� �� ���������
		void set_default_fields_val();
		//������ �������� �� �����:
		void ReadFromFile(const string& file);
		TUserData(const TFields& v);
		TUserData(const TUserData& v);
		TUserData& operator=(const TUserData& v);
	public:
		//������ �������� ����������� �� �����
		TUserData(const string& tunefile);
		~TUserData() {};
		//�������� �� �������:
		bool Empty(void) const { return fields.empty(); };
		//��������� ��������:
		string operator[](const TuneField& code) const;
		string getValue(const TuneField& code) const;
		//��������� ��������:
		void operator()(const TuneField& code, const string& val);
		void setValue(const TuneField& code, const string& val);
		//��������� ������ �������:
		//vector<string> getColumnsTitle(void) const;
	};

}


#endif TUNE_PARAM_H_