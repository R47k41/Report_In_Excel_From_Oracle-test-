//���� ������������ ��� ������� ������ �� �����,
//������������ RS-������ ���������� � ���� excel
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

	//����� �������� ����� �������:
	class TImpField
	{
		private:
			string value;//��������� �������� �� �����
			NS_Const::TConstType dt;//��� ������
		public:
			//�������������
			TImpField(const string& val, const NS_Const::DataType& val_type) : value(val), dt(val_type) {}
			TImpField(const TImpField& val) : value(val.value), dt(val.dt) {}
			//��������� ��������
			void setValue(const string& str) { value = str; }
			//��������� ��������:
			string ValueStr() const { return value; }
			//��������� ���� ������:
			NS_Const::DataType getType() const { return dt.Value(); }
			string getTypeStr() const { return dt.toStr(); }
			bool isEmpty() const { return value.empty(); }
	};
	//����� ��� �������� ������:
	class TAccount
	{
	private:
		TImpField acc;//����� �����
		TImpField name;//������������ �����
		TImpField sld_rub;//������� � ������
		TImpField last_op_date;//���� ���������� ��������
		bool active_flg;//������� ��������� �����
	public:
		//������������� �����������
		TAccount(const string& acc_num, const string& acc_name, const string& sld_rur, const string& last_date, bool isActive) :
			acc(acc_num, NS_Const::DataType::String), name(acc_name, NS_Const::DataType::String), 
			sld_rub(sld_rur, NS_Const::DataType::Double), last_op_date(last_date, NS_Const::DataType::Date), 
			active_flg(isActive) {}
		TAccount(const TAccount& x): acc(x.acc), name(x.name), sld_rub(x.sld_rub), last_op_date(x.last_op_date),
			active_flg(x.active_flg) {}
		//������� ��� ��������������:
		void setAcc(const string& val) { acc.setValue(val); }
		void setName(const string& val);
		void setSldRub(const string& val) { sld_rub.setValue(val); }
		void setOpDate(const string& val) { last_op_date.setValue(val); }
		void setActive(bool val) { active_flg = val; }
		//������� ���������� ������ � �����:
		void addName(const string& val, const NS_Const::CtrlSym& delimeter = NS_Const::CtrlSym::Space);
		//������� ��� ���������:
		string Account(void) const { return acc.ValueStr(); }
		string Name(void) const { return name.ValueStr(); }
		string SldRub(void) const { return sld_rub.ValueStr(); }
		string LastOperDate(void) const { return last_op_date.ValueStr(); }
		bool isActive(void) const { return active_flg; }
		//������� ����������� ������ � �����:
		void show(std::ostream& stream) const;
		//�������� �������:
		bool isEmpty() const { return acc.isEmpty(); }
		//TAccount operator=(const TAccount& x) { *this = x; return *this; }
	};
	
	using TAccounts = vector<TAccount>;
	using StrArr = vector<string>;

	//����� ��� ������� ������ � ������:
	class TImportAccount
	{
		private:
			using TConditionStr = std::pair<string, std::pair<string, string> >;
			enum OEM{ MAX_LEN = 256};
			string name;//������������ ������ ������
			TAccounts accs;//������ ������
			//������� ��������� ������ �� �������(��� ������)
			static string divide_str(const string& str, char delimeter, size_t& pos);
			//������� ��������������� OEM-�����:
			static StrArr ConvertOEM2Arr(const string& file) noexcept(true);
			static bool ConvertOEM2File(const string& file, const NS_Const::CtrlSym& delimeter) noexcept(true);
			void LoadFromOEMFile(const string& file, const NS_Const::CtrlSym& delimeter = NS_Const::CtrlSym::txt_delimeter,
				const NS_Const::CtrlSym& last_line = NS_Const::CtrlSym::txt_tbl_range);
			void setNameByArr(const StrArr& Rows, size_t indx_row, const TConditionStr& condition);
	public:
			//������ ������ �� ���������� �����
		TImportAccount(const string& txt_file, const NS_Const::CtrlSym& delimeter = NS_Const::CtrlSym::txt_delimeter,
			const NS_Const::CtrlSym& last_line = NS_Const::CtrlSym::txt_tbl_range);
			//������� ����������� ����� ������:
			int Count() const { return accs.size(); }
			//������� ����������� ������� ������:
			void show(std::ostream& stream) const;
			//������� �������� �� �������:
			bool isEmpty() const { return accs.empty(); }
			//��� �������
			string getName() const { return name; }
			//��������� ������� ������:
			const TAccounts& getAccounts() const { return accs; }
			//��������� ����� �� ������� �� �������:
			const TAccount& operator[](int x) const { return accs[x]; }
	};

	//��������� ���������� ����������/����������:
	struct TRSBankClient
	{
		string Account;//���� �������
		string AccNum;//����� �����(�� ���� �� �����������)
		string Code;//�����-�� ��� - �����
		string Inn;//��� �������
		string InnSub;//��� ��� �������
		string Name;//������������ �������
		string Bank;//���� �������
		//������� �������� �� ������� ������ �� �����:
		bool isEmptyAcc() const noexcept(true) { return Account.empty() and AccNum.empty() && Inn.empty(); }
		//������� �������� �� ������� ������ �� �������:
		bool isEmptyClient() const noexcept(true) { return Name.empty() && Bank.empty(); }
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
		const size_t FIRST_EMPTY_BLOCK = 3;//������ ���� ������ ���������� - �� ����������.���� �� ��� ��������
		const size_t SECOND_EMPTY_BLOCK = 13;//������ ���� ������ ���������� - �� ��� �������� �� ����������
		string Num;//����� ���������
		string Date;//���� ���������
		string TypeDoc;//��� ��������� 09
		string Shifr;//���� ��������� 1
		string CreaterState;//������ ����������� - ������
		TRSBankClient payer;//����������
		string payerBIK_Filial;//��� ������� ����� �����������(��� ������������� ��������)
		TRSBankClient recipient;//����������
		string Summa;//����� ��������� - � ������, ����������� �����
		string HardFlag;//������� ������� �������� - 0
		string Pachka;//����� ����� - 0
		string SymKas;//������ ����� - 0
		string queue;//����������� - 5
		string GetMeth;//����� ��������� - ����������
		string UsrCode;//��� ������������
		string Note;//���������� �������
		//������� ������������� ���������� �� ������� ������:
		bool InitByStrArr(const StrArr& arr) noexcept(true);
		//������� �������� �� ������� ��������� ���������:
		bool isEmptyDocAtr() const noexcept(true);
		//������� �������� �� �������/���������� ��������� - ��� ��� �������� �� �������������:
		bool isEmpty() const noexcept(true);
		//������� ���������� �������� ��������� ��� �������� ���������:
		bool isValid() const noexcept(true);
		//������� ������������ ������ ������ ����������:
		static string getEmptyAttr(char delimeter, size_t count) noexcept(true);
		//������� ������ ���������� ��������� �������:
		string getAtrStr(char delimiter) const noexcept(true);
		//������� ������������� ������������� ��������� ���������:
		void setDefaultAtr(const TRSBankDoc& dflt) noexcept(true);
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
		bool InitDefaultAttr(const string& file) noexcept(true);
	public:
		//������������� ��������� �� ��������� - ���������� �� �����-�������:
		TRSBankImp(const string& file);
		//������� ����������� ����� ��� �������:
		bool CreateFile4Import(const string& file) const noexcept(true);
		//������� ��������� �����������:
		void setDelimiter(char val) noexcept(true) { delimeter = val; }
		//������� ���������� ��������� � ������:
		bool AddDoc(TRSBankDoc& doc) noexcept(true);
		//������� ��������� �����������
		char getDelimeter() const { return delimeter; }
		//������� �������� ������� �����������:
		bool NoDelimeter() const { return delimeter == '\0'; }
	};

}

#endif // !TSMLVCH_IMP_H_

