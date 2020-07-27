//���� ������������ ��� ������� ������ �� �����,
//������������ RS-������ ���������� � ���� excel
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

	//��������� ��������

	//����� ��� �������� ������:
	class TAccount
	{
	public:
	private:
		//������ �������� ����� � ��������� �������:
		enum FieldIndexs { Account, Name, Saldo_Active, Saldo_Passive, Last_Op_Date };
		string acc;//����� �����
		string name;//������������ �����
		double sld_rub;//������� � ������
		string last_op_date;//���� ���������� ��������
		bool active_flg;//������� ��������� �����
		//������� ��������� ���� ������� � ����������� �� �������:
		void setFieldByIndex(const StrArr& arr, size_t index) noexcept(true);
	public:
		//������������� �����������
		TAccount(const string& acc_num, const string& acc_name, double sld_rur, const string& last_date, 
			bool isActive) :	acc(acc_num), name(acc_name), sld_rub(sld_rur), last_op_date(last_date), active_flg(isActive) {}
		//������������� ��������:
		TAccount(const TAccount& x): acc(x.acc), name(x.name), sld_rub(x.sld_rub), last_op_date(x.last_op_date),
			active_flg(x.active_flg) {}
		//������������� �������� ����� � ����������� �� ��������
		TAccount(const StrArr& fields, const NS_Tune::CellDataArr& params);
		//������� ��� ��������������:
		void setAcc(const string& val) { acc = val; }
		void setName(const string& val);
		void setSldRub(double val) { sld_rub = val; }
		void setOpDate(const string& val) { last_op_date = val; }
		void setActive(bool val) { active_flg = val; }
		//������� ���������� ������ � �����:
		void addName(const string& val, const NS_Const::CtrlSym& delimeter = NS_Const::CtrlSym::Space);
		//������� ��� ���������:
		string getAccount(void) const { return acc; }
		string getName(void) const { return name; }
		double getSldRub(void) const { return sld_rub; }
		string getLastOperDate(void) const { return last_op_date; }
		bool isActive(void) const { return active_flg; }
		//������� ����������� ������ � �����:
		void show(std::ostream& stream) const;
		//�������� �������:
		bool isEmpty() const { return acc.empty() || sld_rub <= 0; }
		//������� ��������� ���� ������:
		string getCurrencyCode() const noexcept(true) { return acc.substr(5, 3); }
		//������� �������� ���������� ������ ��������� ��������:
		static bool isValidStrArr(const StrArr& arr) noexcept(true);
		//������� ��������� ����� ����� �� �������:
		static string getNameByArr(const StrArr& arr) noexcept(true) { return arr[FieldIndexs::Name]; }
		//������� ��������� ������� ���� ��������� � ����� �������� ��� ������������� � ���������� ������:
		static constexpr size_t AccountIndex() noexcept(true) { return FieldIndexs::Account; }
		static constexpr size_t NametIndex() noexcept(true) { return FieldIndexs::Name; }
		static constexpr size_t SldRubIndex() noexcept(true) { return FieldIndexs::Saldo_Active; }
		static constexpr size_t DateIndex() noexcept(true) { return FieldIndexs::Last_Op_Date; }

	};
	
	using TAccounts = vector<TAccount>;

	//����� ��� ������� ������ � ������:
	class TImportBalance
	{
		private:
			enum Fields { COND_NAME_INDX = 0, OEM_MAX_LEN = 256};
			enum Delimiters {Internal = 0, External = 1};
			string name;//������������ ������ ������
			TAccounts active;//������ �������� ������
			TAccounts passive;//������ ��������� ������
			//������� ��������� ������ �� �������(��� ������)
			static string divide_str(const string& str, char delimeter, size_t& pos) noexcept(false);
			//������� ��������� ������ �� ������ ��������� ��������:
			static StrArr divide_by_cols(const string& str, char delimeter) noexcept(true);
			//������� ��������������� OEM-�����:
			static StrArr ConvertOEM2ANSI(const string& file) noexcept(true);
			//���������������� OEM-����� � ANSI ����
			static bool ConvertOEM2File(const string& file, const NS_Tune::TConditionValue& condition, 
				const NS_Const::CtrlSym& delimeter) noexcept(true);
			//��������� ����� ������ ������ �� ������� ����� �����
			static string getNameByArr(const StrArr& Rows, const NS_Tune::TConditionValue& condition) noexcept(true);
			//������� ������������� ������� ������:
			void InitAccountsByParams(const StrArr& Rows, const NS_Tune::CellDataArr& params, char delimeter) noexcept(true);
			//������� ������������� ������ ������� �� ��������:
			void InitByTune(const string& file, const NS_Tune::TBalanceTune& tune) noexcept(true);
	public:
		//������ ������ �� ���������� �����
		TImportBalance(const string& file, const NS_Tune::TBalanceTune& tune);
		//������� ����������� ����� �������� ������
		size_t AciveCount() const noexcept(true) { return active.size(); }
		//������� ����������� ����� ��������� ������:
		size_t PassiveCount() const noexcept(true) { return passive.size(); }
		//������� ����������� ����� ������:
		size_t Count() const { return AciveCount() + PassiveCount(); }
		//������� ����������� ������� ������:
		void show(std::ostream& stream) const;
		//������� �������� ������� ������ �� �������:
		bool isEmpty(bool ActiveFlg) const noexcept(true) { return ActiveFlg ? active.empty() : passive.empty(); }
		//������� �������� �� �������:
		bool isEmpty() const { return isEmpty(true) and isEmpty(false); }
		//��� �������
		string getName() const { return name; }
		//��������� ������� ������:
		const TAccounts& getAccounts(bool activeFlg) const { return activeFlg ? active: passive; }
		//��������� ����� �� ������� �� �������:
		//const TAccount& operator[](int x) const { return accs[x]; }
	};

	//��������� ���������� ����������/����������:
	struct TRSBankClient
	{
		string Account;//���� �������
		string AccNum;//����� �����(�� ���� �� �����������)
		string BIK;//��� �����������/����������
		string CorAcc;//����������������� ����
		string Inn;//��� �������
		string KPP;//��� ��� �������
		string Name;//������������ �������
		string Bank;//���� �������
		//������� �������� �� ������� ������ �� �����:
		bool isEmptyAcc() const noexcept(true) { return Account.empty() and AccNum.empty() && Inn.empty(); }
		//������� �������� �� ������� ������ �� �������:
		bool isEmptyClient() const noexcept(true) { return Name.empty(); }
		//������� ��������� ������ � ����������� ����� �������:
		string getAccData(char delimeter) const noexcept(true);
		//������� ��������� ������ � ����������� ������������ �������:
		string getNameData(char delimeter) const noexcept(true);
		//������� ���������� ���������� � ������� �� ��������
		void setByDefault(const TRSBankClient& dflt) noexcept(true);
		//������� ��������� ��������� ������ � OEM-���������
		static string getOEMStr(const string& str, char delimeter) noexcept(true);
	};
	
	//��������� ��������� ��� ������� � RS-Bank
	struct TRSBankDoc
	{
		//���� ��������� ��� �������(���������� � 1):
		enum class RSDocField { Empty = 0, 
			Numb_Doc,//����� ��������� 
			DocDate,//���� ���������
			ShifrOper,//���� ��������(����������� ����� ��������)
			KindOper,//��� �������� (���������� ���� ���������)
			BIK_Payer,//��� �����������
			Acc_Payer,//���� �����������
			DT_Acc,//���� ��
			CorAccPayer,//����������������� ���� ����� �����������
			INN_Payer,//��� �����������
			KPP_Payer,//��� �����������
			BIK_Receiver,//��� ����� ����������
			Acc_Receiver,//���� ����������
			KT_Acc,//���� ��
			CorAccReceiver,//����������������� ���� ����� ����������
			INN_Receiver,//��� ����������
			KPP_Receiver,//��� ����������
			Summa,//����� ���������
			Pachka,//�����
			Currency,//��� ������
			Symbol_Cach,//������ �����
			Payment,//�����������
			Payer,//������������ �����������
			Bank_Payer,//���� �����������
			Receiver,//������������ ����������
			Bank_Receiver,//���� ����������
			TypeDoc,//��� ���������
			UsrDocType,//���������������� ��� ���������
			Depart,//������
			Dispatch,//������ ��������
			Rslt_Carry,//��� ��������
			CompState,//������ �����������
			BudjCode,//��� ��������� �������������
			OKATO,//�����
			TaxNote,//��������� ���������� �������
			TaxPeriod,//��������� ������
			TaxNum,//����� ���������� ���������
			TaxDate,//���� ���������� ���������
			TaxType,//��� ���������� �������
			UsrF1,//���������������� ���� 1
			UsrF2,//���������������� ���� 1
			UsrF3,//���������������� ���� 1
			UsrF4,//���������������� ���� 1
			UsrCode,//��� ������������
			Note//���������� �������
		};
		const size_t FIRST_EMPTY_BLOCK = 3;//������ ���� ������ ���������� - �� ����������.���� �� ��� ��������
		const size_t SECOND_EMPTY_BLOCK = 13;//������ ���� ������ ���������� - �� ��� �������� �� ����������
		string Num;//����� ���������
		string Date;//���� ���������
		string Shifr;//��� ��������� 09
		string KindOp;//���� ��������� 1
		string CreaterState;//������ ����������� - ������
		TRSBankClient payer;//����������
		string payerBIK_Filial;//��� ������� ����� �����������(��� ������������� ��������)
		TRSBankClient recipient;//����������
		string Summa;//����� ��������� - � ������, ����������� �����
		string Pachka;//����� ����� - 0
		string CurCode;//��� ������
		string SymKas;//������ ����� - 0
		string Queue;//����������� - 5
		string Filial;//������
		string Dispatch;//����� ��������� - ����������
		string RsltCarry;//��� ��������
		string UsrCode;//��� ������������
		string Note;//���������� �������
		//������� ������������� ���������� �� ������� ������:
		bool InitByStrArr(const StrArr& arr, const IntArr& indxs) noexcept(true);
		//������� �������� �� ������� ��������� ���������:
		bool isEmptyDocAtr() const noexcept(true);
		//������� �������� �� �������/���������� ��������� - ��� ��� �������� �� �������������:
		bool isEmpty() const noexcept(true);
		//������� ���������� �������� ��������� ��� �������� ���������:
		bool isValid() const noexcept(true);
		//������� ������������ ������ ������ ����������:
		static string getEmptyAttr(char delimeter, size_t count) noexcept(true);
		//������� ������ ���������� ��������� �������(�� ������������):
		string getAtrStr(char delimiter) const noexcept(true);
		//������� ��������� ���������� ��������� �������:
		string getDocReq(char delimeter) const noexcept(true);
		//������� ������������� ������������� ��������� ���������:
		void setDefaultAtr(const TRSBankDoc& dflt) noexcept(true);
		//������� ��������� �������� ������� ���������:
		bool setField(const string& val, size_t index) noexcept(true);
		//������� ��������� �������� ��������� ��������� �� �������:
		string getField(size_t indx, char delimeter = '\0') const  noexcept(true);
	};

	//������ ���������� ��� ����������:
	using TRSBankDocs = vector<TRSBankDoc>;
	
	//��������� ������� ���������� � RS-���� ���������:
	class TRSBankImp
	{
	private:
		char delimeter;//����������� ����� � �����
		TRSBankDoc tmp_doc;//����� ��������� ��������� ���������
		TRSBankDocs docs;//��������� ����������
		//������� ������������� ��������� ��������� �� ��������� �� ��������� ����� �������
		bool InitDefaultAttr(const string& file, const IntArr& indxs) noexcept(true);
	public:
		//������������� ��������� �� ��������� - ���������� �� �����-�������:
		explicit TRSBankImp(const string& file, const IntArr& indxs);
		//������������� ������� �������:
		TRSBankImp(): delimeter('\0') {}
		//������� ����������� ����� ��� �������:
		bool CreateFile4Import(const string& file, const std::ios_base::openmode& mode = std::ios_base::out) const noexcept(true);
		//������� ��������� �����������:
		void setDelimiter(char val) noexcept(true) { delimeter = val; }
		//������� ���������� ��������� � ������:
		bool AddDoc(TRSBankDoc& doc) noexcept(true);
		//������� ��������� �����������
		char getDelimeter() const { return delimeter; }
		//������� �������� ������� �����������:
		bool NoDelimeter() const { return delimeter == '\0'; }
		//������� ��������� ������������� ���������� �� �����:
		bool setDefaultAttrByFile(const string& template_file, const IntArr& indxs, 
			char def_div = '\0', bool clr_DocsArr = false) noexcept(true);
	};

}

#endif // !TSMLVCH_IMP_H_

