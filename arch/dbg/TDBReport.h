#ifndef TDBREPORT_H_
#define TDBREPORT_H_
//������ ��� ������������ ������ �� ���� ������
//����� ��������� � excel-��������
//���� ������ - oracle
#include <string>
#include <vector>
#include "TExcel.h"
#include "TuneParam.h"
#include "TConstants.h"
#include "TOracle.h"


namespace NS_ExcelReport
{
	using std::string;
	using std::vector;
	using NS_Tune::TSimpleTune;
	using NS_Tune::TUserTune;
	using NS_Tune::TSharedTune;
	using NS_Excel::TExcelBook;
	using NS_Excel::TExcelBookSheet;
	using NS_Oracle::TStatement;
	using NS_Oracle::TResultSet;
	using NS_Oracle::TConnectParam;
	using NS_Oracle::TDBConnect;
	
	//����� ����� ����������� �� ���� ��������� � ��
	const int PrefetchRowsCnt = 200;

	//����� ����������� ��� ������������ ������ ��� ��������:
	//������������� ���������� �������� � ���������� �����
	class TSheetReport
	{
	protected:
		TExcelBook& book;//������ �� �����
		TExcelBookSheet sheet;//���� ������
		const TUserTune& tune;//��������� ��� ����� ������
		//������� ������������� �������� �� ����������:
		virtual bool SetSheetByTune(const string& name = "") noexcept(true);
		//������� ���������� �������� ��� ������, ���� ������ ��������� �����������:
		virtual bool CreateNewPage(size_t val, bool byRows = true);
	public:
		//�������� �������� ������ �� ����� � ������ �� ���� ���������
		TSheetReport(TExcelBook& book_link, const TUserTune& config);
		//������������� ���������: 
		TSheetReport(TExcelBook& ref_book, const TExcelBookSheet& ref_sheet, const TUserTune& ref_tune);
		//����������:
		virtual ~TSheetReport() {}
		//������� ���������� �������� ������
		virtual bool crtSheet() noexcept(true) = 0;
		//��������� �������� ���� �� ���� ������ �� ������� ������
		//virtual void setDataByType(int indx) noexcept(false) = 0;
	};

	//����� ��������� ������ ��� �������� �� ��
	class TDataBaseSheetReport: public TSheetReport
	{
	private:
		//��������� � ����������� ��������:
		enum class SqlType { DQL, DML };
		int prefetch_rows;//����� ����������� ����� �� ���� ��������� � ��
		//��������� �������� ��������� sql-������� �� ��������:
		static void setSqlParamByTune(TStatement& sql, const NS_Tune::TSubParam& param, bool use_raise = false) noexcept(false);
		//������� �������������� ����� ������ �� TOracle(TType) � ��� ������ TConstants(DataType)
		static NS_Const::DataType convertOraType(const NS_Oracle::TType& type) noexcept(true);
		//��������� �������� ���������� sql-������� �� ����������:
		static void setSqlParamsByTune(TStatement& sql, const TUserTune& param) noexcept(false);
		//��������� �������� ���� �� ���� ������ �� ������� ������ sql-������
		virtual void setCellByResultSet(const TResultSet& rs, const NS_Excel::TExcelCell& cell) noexcept(false);
		virtual bool WriteFromResultSet(TResultSet& rs) noexcept(true);
		//������� ���������� ������ � ����� �� ������ sql-�������:
		void FillSheetBySql(TDBConnect& db, const string& sql_txt, int prefetch);
		//������� ������������� ��������:
		bool useSqlParse() const noexcept(false);
		//������� ���������� DQL ������� ����� DML ��������
		//��� �������, ��� ��� ��� ������� � ����� config-�����
		bool isDQLFirst() const noexcept(false);
		//��������� ������ sql-������� � ��������� ��� ���:
		static string getSqlByTune(bool use_parse, bool by_str, const string& str) noexcept(true);
	protected:
		//��������� ������ ������� �� ������ ���� �� ��������� ��������, ���� ������ �����
		template <typename T>
		static string getSqlText(bool by_str, const string& str) noexcept(false);
		//������������ �������� ������ �� ����f�� �������:
		void CrtBySqlLine(TDBConnect& db, const string& sql_line, int prefetch = 1) noexcept(false);
		//������������ �������� ������ �� ������ sql-��������:
		void CrtBySqlFiles(TDBConnect& db, int prefetch = 1) noexcept(false);
	public:
		//������������� ������ � ������� ��������:
		TDataBaseSheetReport(TExcelBook& pbook, const TUserTune& config, int prefetch = PrefetchRowsCnt) : 
			TSheetReport(pbook, config), prefetch_rows(prefetch) {}
		//������������� ������, ��������� � ������ ��������:
		TDataBaseSheetReport(TExcelBook& ref_book, const TExcelBookSheet& ref_sheet, const TUserTune& ref_tune,
			int prefetch = PrefetchRowsCnt) : TSheetReport(ref_book, ref_sheet, ref_tune), prefetch_rows(prefetch) {}
		//������������ ���������� �����������:
		static TConnectParam getConnectParam(const TUserTune& param, int prefetch = 1) noexcept(false);
		//������������ �������� ������:
		virtual bool crtSheet() noexcept(true);
		//����� ���������� ��������� dml-�������:
		static size_t executeDML(TDBConnect& db, const TUserTune& param, const string& dml, bool use_comit = true) noexcept(false);
		//����� dml-������� ��� ������:
		static size_t runDML4Directory(TDBConnect& db, const TUserTune& param, bool use_comit = true) noexcept(false);
		//���������� DML �������(����� ��������� ��� �������� � ������):
		static size_t runDML(TDBConnect& db, const TUserTune& param, bool use_comit = true) noexcept(true);
	};

	class TReport
	{
	private:
		//�������� ��������� ��� ������������ ������
		TSharedTune config;
		//������� ���������� ������:
		void saveReport(TExcelBook& book, const string& file_name = "") const noexcept(false);
	public:
		//�������������
		TReport(const string& conf_file, const string& code) : config(conf_file, code) {}
		//����������
		virtual ~TReport() {}
		//������� ������������ �����-������:
		//��� ������ ������ ����������� ����������� excel-����
		void One_Report_For_Each_Config() const noexcept(false);
		//������� ������������ ������:
		//��� ������� ����� ������� ��������� ����������� ��������
		//���������� � ����� excel-�����
		void One_Sheet_By_One_Config() const noexcept(false);
		//������� ������������ ������:
		//��� ������� ������������ �� ����� ��������
		//�������� ����������� � excel-����
		void One_Sheet_By_Many_Statement() const noexcept(false);
		//���������� dml-������(��� ������� dql-��������)
		size_t runDML_By_Tune(bool use_comit = true) const noexcept(false);
		//����� �������, ����������� ����� �� ����:
		void Create_Report_By_Code(const NS_Const::ReportCode& code) const;
		//������������ ������� ������ �� ����� main_config:
		bool Execute() const noexcept(true);
	};
}
#endif