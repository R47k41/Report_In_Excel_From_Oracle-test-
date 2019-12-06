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
	using NS_Tune::TSimpleTune;
	using NS_Tune::TUserTune;
	using NS_Tune::TSharedTune;
	using NS_Excel::TExcelBook;
	using NS_Excel::TExcelBookSheet;
	using NS_Oracle::TStatement;
	using NS_Oracle::TResultSet;
	using NS_Oracle::TConnectParam;
	
	//����� ����� ����������� �� ���� ��������� � ��
	const int PrefetchRowsCnt = 200;

	//����� ����������� ��� ������������ ������ ��� ��������:
	//������������� ���������� �������� � ���������� �����
	class TSheetReport
	{
	protected:
		TExcelBook& book;//������ �� �����
		TExcelBookSheet sheet;//���� ������
		TUserTune tune;//��������� ��� ����� ������
		//������� ������������� �������� �� ����������:
		virtual bool SetSheetByTune(const string& name = "") noexcept(true);
		//������� ���������� �������� ��� ������, ���� ������ ��������� �����������:
		virtual bool CreateNewPage(size_t val, bool byRows = true);
	public:
		//�������� �������� ������ �� ����� � �����/���� ����� �������
		TSheetReport(TExcelBook& book_link, const string& config);
		//������������� �������� �� ����� � ����������:
		TSheetReport(TExcelBook& book_link, const TSimpleTune& config, const string& sub_conf_file);
		//������������� ������ � ��������� ��������� � ������ ��������:
		TSheetReport(TExcelBook& ref_book, TExcelBookSheet& ref_sheet, const TSimpleTune& ref_tune,
			const string& sub_conf_file) :
			book(ref_book), sheet(ref_sheet), tune(ref_tune, sub_conf_file) {}
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
		int prefetch_rows;//����� ����������� ����� �� ���� ��������� � ��
		//������������ ���������� �����������:
		TConnectParam getConnectParam() const noexcept(false);
		//��������� �������� ��������� sql-������� �� ��������:
		static void setSqlParamByTune(TStatement& sql, const NS_Tune::TSubParam& param, bool use_raise = false) noexcept(false);
		//������� �������������� ����� ������ �� TOracle(TType) � ��� ������ TConstants(DataType)
		static NS_Const::DataType convertOraType(const NS_Oracle::TType& type) noexcept(true);
		//��������� �������� ���������� sql-������� �� ����������:
		void setSqlParamsByTune(TStatement& sql) const noexcept(false);
		//��������� �������� ���� �� ���� ������ �� ������� ������ sql-������
		virtual void setCellByResultSet(const TResultSet& rs, const NS_Excel::TExcelCell& cell) noexcept(false);
		virtual bool WriteFromResultSet(TResultSet& rs) noexcept(true);
	protected:
		//��������� ������ sql-�������
		template <typename T>
		string getSqlText() const noexcept(false);
	public:
		//������������� ������ � ������� ��������:
		TDataBaseSheetReport(TExcelBook& pbook, const string& config, int prefetch = PrefetchRowsCnt) : 
			TSheetReport(pbook, config), prefetch_rows(prefetch) {}
		TDataBaseSheetReport(TExcelBook& pbook, const TSimpleTune& conf, const string& sub_conf,
			int prefetch = PrefetchRowsCnt) : TSheetReport(pbook, conf, sub_conf), prefetch_rows(prefetch) {}
		//�������������
		//������������ �������� ������:
		virtual bool crtSheet() noexcept(true);
	};

	class TReport
	{
	private:
		//�������� ��������� ��� ������������ ������
		TSharedTune config;
	public:
		//�������������
		TReport(const string& conf_file, const string& code) : config(conf_file, code) {}
		//����������
		virtual ~TReport() {}
		//������� ������������ �������� sql-������:
		//��� ������ ������ ����������� ����������� ��������:
		void Simple_Sql_Report() const;
		//����� �������, ����������� ����� �� ����:
		void Create_Report_By_Code(const NS_Const::ReportCode& code) const;
		//������������ ������� ������ �� ����� main_config:
		bool Execute() const noexcept(true);
	};
}
#endif