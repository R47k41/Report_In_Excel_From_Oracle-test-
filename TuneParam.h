//#pragma once
#ifndef TUNE_PARAM_H_
#define TUNE_PARAM_H_
//модуль предназначен для описания класса работающего с файлом настроек
#include <string>
//#include <vector>
#include <map>
#include <utility>

//обаласти пространства имен для работы с настройками
namespace NS_Tune
{
	using std::string;
//	using std::ifstream;
	using std::pair;
//	using std::vector;
	using std::map;
	using find_fnc = size_t (string::*)(const string& s, size_t pos) const;
	//Класс для наименования полей настроек(константы)
	//порядок указан такой же, как и файле настроек
	enum class TuneField {Empty = 0, DataRange,
		DataBase, Report, Columns,
		UserName, Password, TNS, OutFile, SqlFile, SqlText,	Column};
	//преобразование в строку:
	string TuneFieldToStr(const TuneField& val);
	//преобразование в число:
	inline int TuneFieldToInt(const TuneField& val) { return int(val); };
	inline TuneField operator+(const TuneField& val, int x) { return TuneField(TuneFieldToInt(val) + x); };
	inline TuneField operator+=(const TuneField& val, int x) { return val + x; };
	bool operator==(const TuneField& val, const string& str);
	size_t get_pos_in_str(const string& str, const TuneField& substr, const size_t beg = 0, find_fnc = &string::find);
	string Get_TuneFiel_Val_From_Str(const TuneField& f, const string& str);
	
	//класс для данных полей настроек???
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

	//Класс - данные для подключения:
	class TUserData
	{
	private:
		TFields fields;
		//фукнция установки значений по умолчанию
		void set_default_fields_val();
		//чтение настроек из файла:
		void ReadFromFile(const string& file);
		TUserData(const TFields& v);
		TUserData(const TUserData& v);
		TUserData& operator=(const TUserData& v);
	public:
		//список настроек формируется из файла
		TUserData(const string& tunefile);
		~TUserData() {};
		//проверка на пустоту:
		bool Empty(void) const { return fields.empty(); };
		//получение значения:
		string operator[](const TuneField& code) const;
		string getValue(const TuneField& code) const;
		//установка занчения:
		void operator()(const TuneField& code, const string& val);
		void setValue(const TuneField& code, const string& val);
		//получение списка колонок:
		//vector<string> getColumnsTitle(void) const;
	};

}


#endif TUNE_PARAM_H_