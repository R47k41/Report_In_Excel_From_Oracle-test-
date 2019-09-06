//#pragma once
#ifndef TUNE_PARAM_H_
#define TUNE_PARAM_H_
//������ ������������ ��� �������� ������ ����������� � ������ ��������
#include "TSQLParser.h"
#include <map>

//�������� ������������ ���� ��� ������ � �����������
namespace NS_Tune
{
	using std::string;
	using std::ifstream;
	using std::pair;
	using std::vector;
	using std::map;
	
	//����� ��� ������������ ����� ��������(���������)
	//������� ������ ����� ��, ��� � ����� ��������
	enum class TuneField {Empty = 0, DataRange,
		DataBase, UserName, Password, TNS,
		Report, OutFile, SqlFile, SqlText,
		Columns, Column};
	//�������������� � ������:
	string TuneFieldToStr(const TuneField& val);
	//�������������� � �����:
	inline int TuneFieldToInt(const TuneField& val) { return int(val); };
	inline TuneField operator+(const TuneField& val, int x) { return val + x; };
	inline TuneField operator+=(const TuneField& val, int x) { return val + x; };
	bool operator==(const TuneField& val, const string& str);
	bool operator<(const TuneField& v1, const TuneField& v2);

	//using TField = std::pair<std::string, std::string>;
	using TFields = std::map<TuneField, string>;

	//����� - ������ ��� �����������:
	class TUserData
	{
	private:
		TFields fields;
	public:
		TUserData(const string& par_name = "", const string& par_pass = "", const string& par_DB = "") :
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

}


#endif TUNE_PARAM_H_