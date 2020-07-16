//файл используется для импорта данных из файла,
//формируемого RS-Банком Смоленвича в файл excel
#ifndef TSMLVCH_IMP_H_
#define TSMLVCH_IMP_H_
#include <string>
#include <vector>
#include <iostream>
#include "TConstants.h"

namespace NS_SMLVCH_IMP
{
	using NS_Const::DataType;
	using NS_Const::TConstType;
	using std::string;
	using std::vector;
	using std::istream;
	using std::ostream;

	//класс описания полей импорта:
	class TImpField
	{
		private:
			string value;//строковое значение из файла
			NS_Const::TConstType dt;//тип данных
		public:
			//инициализация
			TImpField(const string& val, const NS_Const::DataType& val_type) : value(val), dt(val_type) {}
			TImpField(const TImpField& val) : value(val.value), dt(val.dt) {}
			//установка значения
			void setValue(const string& str) { value = str; }
			//получение значений:
			string ValueStr() const { return value; }
			//получение типа записи:
			NS_Const::DataType getType() const { return dt.Value(); }
			string getTypeStr() const { return dt.toStr(); }
			bool isEmpty() const { return value.empty(); }
	};
	//класс для описания счетов:
	class TAccount
	{
	private:
		TImpField acc;//номер счета
		TImpField name;//наименование счета
		TImpField sld_rub;//остаток в рублях
		TImpField last_op_date;//дата последнего движения
		bool active_flg;//признак активного счета
	public:
		//инициализация стандартная
		TAccount(const string& acc_num, const string& acc_name, const string& sld_rur, const string& last_date, bool isActive) :
			acc(acc_num, NS_Const::DataType::String), name(acc_name, NS_Const::DataType::String), 
			sld_rub(sld_rur, NS_Const::DataType::Double), last_op_date(last_date, NS_Const::DataType::Date), 
			active_flg(isActive) {}
		TAccount(const TAccount& x): acc(x.acc), name(x.name), sld_rub(x.sld_rub), last_op_date(x.last_op_date),
			active_flg(x.active_flg) {}
		//функции для редактирования:
		void setAcc(const string& val) { acc.setValue(val); }
		void setName(const string& val);
		void setSldRub(const string& val) { sld_rub.setValue(val); }
		void setOpDate(const string& val) { last_op_date.setValue(val); }
		void setActive(bool val) { active_flg = val; }
		//функция добавления данных к имени:
		void addName(const string& val, const NS_Const::CtrlSym& delimeter = NS_Const::CtrlSym::Space);
		//функции для просмотра:
		string Account(void) const { return acc.ValueStr(); }
		string Name(void) const { return name.ValueStr(); }
		string SldRub(void) const { return sld_rub.ValueStr(); }
		string LastOperDate(void) const { return last_op_date.ValueStr(); }
		bool isActive(void) const { return active_flg; }
		//функция отображения данных о счете:
		void show(std::ostream& stream) const;
		//проверка пустоты:
		bool isEmpty() const { return acc.isEmpty(); }
		//TAccount operator=(const TAccount& x) { *this = x; return *this; }
	};
	
	using TAccounts = vector<TAccount>;
	using StrArr = vector<string>;

	//класс для импорта данных о счетах:
	class TImportAccount
	{
		private:
			using TConditionStr = std::pair<string, std::pair<string, string> >;
			enum OEM{ MAX_LEN = 256};
			string name;//наименование группы счетов
			TAccounts accs;//массив счетов
			//функция разбиения строки на колонки(для счетов)
			static string divide_str(const string& str, char delimeter, size_t& pos);
			//фукнция конвертирования OEM-файла:
			static StrArr ConvertOEM2Arr(const string& file) noexcept(true);
			static bool ConvertOEM2File(const string& file, const NS_Const::CtrlSym& delimeter) noexcept(true);
			void LoadFromOEMFile(const string& file, const NS_Const::CtrlSym& delimeter = NS_Const::CtrlSym::txt_delimeter,
				const NS_Const::CtrlSym& last_line = NS_Const::CtrlSym::txt_tbl_range);
			void setNameByArr(const StrArr& Rows, size_t indx_row, const TConditionStr& condition);
	public:
			//импорт данных из текстового файла
		TImportAccount(const string& txt_file, const NS_Const::CtrlSym& delimeter = NS_Const::CtrlSym::txt_delimeter,
			const NS_Const::CtrlSym& last_line = NS_Const::CtrlSym::txt_tbl_range);
			//функция отображения числа счетов:
			int Count() const { return accs.size(); }
			//функция отображения массива счетов:
			void show(std::ostream& stream) const;
			//функция проверки на пустоту:
			bool isEmpty() const { return accs.empty(); }
			//имя массива
			string getName() const { return name; }
			//получение массива счетов:
			const TAccounts& getAccounts() const { return accs; }
			//получение счета из массива по индексу:
			const TAccount& operator[](int x) const { return accs[x]; }
	};

	//Структура реквизитов Плательщик/Получаетль:
	struct TRSBankClient
	{
		string Account;//счет клиента
		string AccNum;//номер счета(по сути он дублируется)
		string Code;//какой-то код - пусто
		string Inn;//ИНН клиента
		string InnSub;//ИНН под клиента
		string Name;//наименование клиента
		string Bank;//Банк клиента
		//функция проверки на пустоту данных по счету:
		bool isEmptyAcc() const noexcept(true) { return Account.empty() and AccNum.empty() && Inn.empty(); }
		//функция проверки на пустоту данных по клиенту:
		bool isEmptyClient() const noexcept(true) { return Name.empty() && Bank.empty(); }
		//функция получения строки с реквизитами счета клиента:
		string getAccData(char delimeter) const noexcept(true);
		//функция получения строки с реквизитами наименования клиента:
		string getNameData(char delimeter) const noexcept(true);
		//функция заполнения информации о клиенте из шаблонна
		void setByDefault(const TRSBankClient& dflt) noexcept(true);
		//функция получения строковых данных в OEM-кодировке
		static string getOEMStr(const string& str, char delimeter) noexcept(true);
	};
	//структура документа для импорта в RS-Bank
	struct TRSBankDoc
	{
		const size_t FIRST_EMPTY_BLOCK = 3;//первый блок пустых аттрибутов - от Получатель.Банк до Вид передачи
		const size_t SECOND_EMPTY_BLOCK = 13;//второй блок пустых аттрибутов - от Вид передачи до Назначения
		string Num;//номер документа
		string Date;//дата документа
		string TypeDoc;//Вид документа 09
		string Shifr;//Шифр документа 1
		string CreaterState;//Статус составителя - пустой
		TRSBankClient payer;//плательщик
		string payerBIK_Filial;//БИК филиала Банка плательщика(для межфилиальных расчетов)
		TRSBankClient recipient;//получатель
		string Summa;//сумма документа - в рублях, разделитель точка
		string HardFlag;//признка сложной проводки - 0
		string Pachka;//номер пачки - 0
		string SymKas;//символ кассы - 0
		string queue;//очередность - 5
		string GetMeth;//метод получения - электронно
		string UsrCode;//код пользователя
		string Note;//назначение платежа
		//функция инициализации аттрибутов по массиву строки:
		bool InitByStrArr(const StrArr& arr) noexcept(true);
		//функция проверки на пустоту атрибутов документа:
		bool isEmptyDocAtr() const noexcept(true);
		//функция проверки на пустоту/валидность атрибутов - без них документ не импортировать:
		bool isEmpty() const noexcept(true);
		//функция валидности основных атрибутов для создания документа:
		bool isValid() const noexcept(true);
		//функция формирования строки пустых аттрибутов:
		static string getEmptyAttr(char delimeter, size_t count) noexcept(true);
		//функция вывода аттрибутов документа строкой:
		string getAtrStr(char delimiter) const noexcept(true);
		//функция инициализации умолчательных атрибутов документа:
		void setDefaultAtr(const TRSBankDoc& dflt) noexcept(true);
	};

	//Массив реквизитов для документов:
	using TRSBankDocs = vector<TRSBankDoc>;
	
	//структура импорта документов в RS-Банк Смоленвич:
	class TRSBankImp
	{
	private:
		char delimeter;//разделитель полей в файле
		TRSBankDoc tmp_doc;//общие шаблонные реквизиты документа
		TRSBankDocs docs;//реквизиты документов
		//функция инициализации атрибутов документа по умолчанию на основании файла шаблона
		bool InitDefaultAttr(const string& file) noexcept(true);
	public:
		//инициализация структуры по умолчанию - считывание из файла-шаблона:
		TRSBankImp(const string& file);
		//функция формировани файла для импорта:
		bool CreateFile4Import(const string& file) const noexcept(true);
		//функция установки разделителя:
		void setDelimiter(char val) noexcept(true) { delimeter = val; }
		//функция добавления документа в массив:
		bool AddDoc(TRSBankDoc& doc) noexcept(true);
		//функция получения разделителя
		char getDelimeter() const { return delimeter; }
		//функция проверки наличия разделителя:
		bool NoDelimeter() const { return delimeter == '\0'; }
	};

}

#endif // !TSMLVCH_IMP_H_

