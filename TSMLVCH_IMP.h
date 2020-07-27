//файл используется для импорта данных из файла,
//формируемого RS-Банком Смоленвича в файл excel
#ifndef TSMLVCH_IMP_H_
#define TSMLVCH_IMP_H_
#include <string>
#include <vector>
#include <iostream>
#include "TConstants.h"
#include "TuneParam.h"

namespace NS_SMLVCH_IMP
{
	using NS_Const::DataType;
	using NS_Const::TConstType;
	using std::string;
	using std::vector;
	using std::istream;
	using std::ostream;
	using StrArr = vector<string>;
	using IntArr = vector<size_t>;

	//структура индексов

	//класс для описания счетов:
	class TAccount
	{
	public:
	private:
		//массив индексов полей в строковом массиве:
		enum FieldIndexs { Account, Name, Saldo_Active, Saldo_Passive, Last_Op_Date };
		string acc;//номер счета
		string name;//наименование счета
		double sld_rub;//остаток в рублях
		string last_op_date;//дата последнего движения
		bool active_flg;//признак активного счета
		//функция установки поля объекта в зависимости от индекса:
		void setFieldByIndex(const StrArr& arr, size_t index) noexcept(true);
	public:
		//инициализация стандартная
		TAccount(const string& acc_num, const string& acc_name, double sld_rur, const string& last_date, 
			bool isActive) :	acc(acc_num), name(acc_name), sld_rub(sld_rur), last_op_date(last_date), active_flg(isActive) {}
		//инициализация объектом:
		TAccount(const TAccount& x): acc(x.acc), name(x.name), sld_rub(x.sld_rub), last_op_date(x.last_op_date),
			active_flg(x.active_flg) {}
		//инициализация массивом строк и параметрами из настроек
		TAccount(const StrArr& fields, const NS_Tune::CellDataArr& params);
		//функции для редактирования:
		void setAcc(const string& val) { acc = val; }
		void setName(const string& val);
		void setSldRub(double val) { sld_rub = val; }
		void setOpDate(const string& val) { last_op_date = val; }
		void setActive(bool val) { active_flg = val; }
		//функция добавления данных к имени:
		void addName(const string& val, const NS_Const::CtrlSym& delimeter = NS_Const::CtrlSym::Space);
		//функции для просмотра:
		string getAccount(void) const { return acc; }
		string getName(void) const { return name; }
		double getSldRub(void) const { return sld_rub; }
		string getLastOperDate(void) const { return last_op_date; }
		bool isActive(void) const { return active_flg; }
		//функция отображения данных о счете:
		void show(std::ostream& stream) const;
		//проверка пустоты:
		bool isEmpty() const { return acc.empty() || sld_rub <= 0; }
		//функция получения кода валюты:
		string getCurrencyCode() const noexcept(true) { return acc.substr(5, 3); }
		//функция проверки валидности списка строковых значений:
		static bool isValidStrArr(const StrArr& arr) noexcept(true);
		//функция получения имени счета из массива:
		static string getNameByArr(const StrArr& arr) noexcept(true) { return arr[FieldIndexs::Name]; }
		//функция получения индекса поля приемника в файле настроек для идентификации в параметрах отчета:
		static constexpr size_t AccountIndex() noexcept(true) { return FieldIndexs::Account; }
		static constexpr size_t NametIndex() noexcept(true) { return FieldIndexs::Name; }
		static constexpr size_t SldRubIndex() noexcept(true) { return FieldIndexs::Saldo_Active; }
		static constexpr size_t DateIndex() noexcept(true) { return FieldIndexs::Last_Op_Date; }

	};
	
	using TAccounts = vector<TAccount>;

	//класс для импорта данных о счетах:
	class TImportBalance
	{
		private:
			enum Fields { COND_NAME_INDX = 0, OEM_MAX_LEN = 256};
			enum Delimiters {Internal = 0, External = 1};
			string name;//наименование группы счетов
			TAccounts active;//массив активных счетов
			TAccounts passive;//массив пассивных счетов
			//функция разбиения строки на колонки(для счетов)
			static string divide_str(const string& str, char delimeter, size_t& pos) noexcept(false);
			//функция разбиение строки на массив строковых значений:
			static StrArr divide_by_cols(const string& str, char delimeter) noexcept(true);
			//фукнция конвертирования OEM-файла:
			static StrArr ConvertOEM2ANSI(const string& file) noexcept(true);
			//конверстирование OEM-файла в ANSI файл
			static bool ConvertOEM2File(const string& file, const NS_Tune::TConditionValue& condition, 
				const NS_Const::CtrlSym& delimeter) noexcept(true);
			//установка имени группы счетов по массиву строк файла
			static string getNameByArr(const StrArr& Rows, const NS_Tune::TConditionValue& condition) noexcept(true);
			//функция инициализации массива счетов:
			void InitAccountsByParams(const StrArr& Rows, const NS_Tune::CellDataArr& params, char delimeter) noexcept(true);
			//функция инициализации данных объекта из настроек:
			void InitByTune(const string& file, const NS_Tune::TBalanceTune& tune) noexcept(true);
	public:
		//импорт данных из текстового файла
		TImportBalance(const string& file, const NS_Tune::TBalanceTune& tune);
		//функция отображения числа активных счетов
		size_t AciveCount() const noexcept(true) { return active.size(); }
		//функция отображения числа пассивных счетов:
		size_t PassiveCount() const noexcept(true) { return passive.size(); }
		//функция отображения числа счетов:
		size_t Count() const { return AciveCount() + PassiveCount(); }
		//функция отображения массива счетов:
		void show(std::ostream& stream) const;
		//функция проверки массива счетов на пустоту:
		bool isEmpty(bool ActiveFlg) const noexcept(true) { return ActiveFlg ? active.empty() : passive.empty(); }
		//функция проверки на пустоту:
		bool isEmpty() const { return isEmpty(true) and isEmpty(false); }
		//имя массива
		string getName() const { return name; }
		//получение массива счетов:
		const TAccounts& getAccounts(bool activeFlg) const { return activeFlg ? active: passive; }
		//получение счета из массива по индексу:
		//const TAccount& operator[](int x) const { return accs[x]; }
	};

	//Структура реквизитов Плательщик/Получаетль:
	struct TRSBankClient
	{
		string Account;//счет клиента
		string AccNum;//номер счета(по сути он дублируется)
		string BIK;//БИК плательщика/получателя
		string CorAcc;//корреспондентский счет
		string Inn;//ИНН клиента
		string KPP;//ИНН под клиента
		string Name;//наименование клиента
		string Bank;//Банк клиента
		//функция проверки на пустоту данных по счету:
		bool isEmptyAcc() const noexcept(true) { return Account.empty() and AccNum.empty() && Inn.empty(); }
		//функция проверки на пустоту данных по клиенту:
		bool isEmptyClient() const noexcept(true) { return Name.empty(); }
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
		//поля документа при импорте(начинаются с 1):
		enum class RSDocField { Empty = 0, 
			Numb_Doc,//номер документа 
			DocDate,//дата документа
			ShifrOper,//шифр операции(Спаравочник Шифры операций)
			KindOper,//вид операции (Справочник Виды документа)
			BIK_Payer,//БИК плательщика
			Acc_Payer,//счет плательщика
			DT_Acc,//счет Дт
			CorAccPayer,//корреспондентский счет банка плательщика
			INN_Payer,//ИНН плательщика
			KPP_Payer,//КПП плательщика
			BIK_Receiver,//БИК банка получателя
			Acc_Receiver,//счет получателя
			KT_Acc,//счет Кт
			CorAccReceiver,//корреспондентский счет банка получателя
			INN_Receiver,//ИНН получателя
			KPP_Receiver,//КПП получателя
			Summa,//сумма документа
			Pachka,//пачка
			Currency,//код валюты
			Symbol_Cach,//символ кассы
			Payment,//очередность
			Payer,//Наименование плательщика
			Bank_Payer,//Банк плательщика
			Receiver,//Наименование получателя
			Bank_Receiver,//Банк получателя
			TypeDoc,//тип документа
			UsrDocType,//пользовательский тип документа
			Depart,//филиал
			Dispatch,//способ отправки
			Rslt_Carry,//тип операции
			CompState,//статус составителя
			BudjCode,//код бюджетной классификации
			OKATO,//окато
			TaxNote,//основание налогового платежа
			TaxPeriod,//налоговый период
			TaxNum,//номер налогового документа
			TaxDate,//дата налогового документа
			TaxType,//тип налогового платежа
			UsrF1,//пользовательское поле 1
			UsrF2,//пользовательское поле 1
			UsrF3,//пользовательское поле 1
			UsrF4,//пользовательское поле 1
			UsrCode,//код пользователя
			Note//назначение платежа
		};
		const size_t FIRST_EMPTY_BLOCK = 3;//первый блок пустых аттрибутов - от Получатель.Банк до Вид передачи
		const size_t SECOND_EMPTY_BLOCK = 13;//второй блок пустых аттрибутов - от Вид передачи до Назначения
		string Num;//номер документа
		string Date;//дата документа
		string Shifr;//Вид документа 09
		string KindOp;//Шифр документа 1
		string CreaterState;//Статус составителя - пустой
		TRSBankClient payer;//плательщик
		string payerBIK_Filial;//БИК филиала Банка плательщика(для межфилиальных расчетов)
		TRSBankClient recipient;//получатель
		string Summa;//сумма документа - в рублях, разделитель точка
		string Pachka;//номер пачки - 0
		string CurCode;//Код валюты
		string SymKas;//символ кассы - 0
		string Queue;//очередность - 5
		string Filial;//филиал
		string Dispatch;//метод получения - электронно
		string RsltCarry;//тип операции
		string UsrCode;//код пользователя
		string Note;//назначение платежа
		//функция инициализации аттрибутов по массиву строки:
		bool InitByStrArr(const StrArr& arr, const IntArr& indxs) noexcept(true);
		//функция проверки на пустоту атрибутов документа:
		bool isEmptyDocAtr() const noexcept(true);
		//функция проверки на пустоту/валидность атрибутов - без них документ не импортировать:
		bool isEmpty() const noexcept(true);
		//функция валидности основных атрибутов для создания документа:
		bool isValid() const noexcept(true);
		//функция формирования строки пустых аттрибутов:
		static string getEmptyAttr(char delimeter, size_t count) noexcept(true);
		//функция вывода аттрибутов документа строкой(НЕ используется):
		string getAtrStr(char delimiter) const noexcept(true);
		//функция получения реквизитов документа строкой:
		string getDocReq(char delimeter) const noexcept(true);
		//функция инициализации умолчательных атрибутов документа:
		void setDefaultAtr(const TRSBankDoc& dflt) noexcept(true);
		//функция установки значения аттрибу документа:
		bool setField(const string& val, size_t index) noexcept(true);
		//функция получения значения аттрибута документа по индексу:
		string getField(size_t indx, char delimeter = '\0') const  noexcept(true);
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
		bool InitDefaultAttr(const string& file, const IntArr& indxs) noexcept(true);
	public:
		//инициализация структуры по умолчанию - считывание из файла-шаблона:
		explicit TRSBankImp(const string& file, const IntArr& indxs);
		//инициализация пустого объекта:
		TRSBankImp(): delimeter('\0') {}
		//функция формировани файла для импорта:
		bool CreateFile4Import(const string& file, const std::ios_base::openmode& mode = std::ios_base::out) const noexcept(true);
		//функция установки разделителя:
		void setDelimiter(char val) noexcept(true) { delimeter = val; }
		//функция добавления документа в массив:
		bool AddDoc(TRSBankDoc& doc) noexcept(true);
		//функция получения разделителя
		char getDelimeter() const { return delimeter; }
		//функция проверки наличия разделителя:
		bool NoDelimeter() const { return delimeter == '\0'; }
		//функция установки умолчательных аттрибутов по файлу:
		bool setDefaultAttrByFile(const string& template_file, const IntArr& indxs, 
			char def_div = '\0', bool clr_DocsArr = false) noexcept(true);
	};

}

#endif // !TSMLVCH_IMP_H_

