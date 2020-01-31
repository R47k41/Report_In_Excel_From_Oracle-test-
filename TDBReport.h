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
	using NS_Tune::TExcelProcData;
	using NS_Tune::TShareData;
	using NS_Tune::TProcCell;
	using NS_Tune::TCellData;
	using NS_Tune::TFilterData;
	using NS_Tune::TCellMethod;
	using DBUserTuneArr = vector<TUserTune>;
	using CellDataArr = vector<TCellData>;
	using TRowFlag = std::pair<size_t, bool>;
	using TRowsFlag = std::vector<TRowFlag>;
	using TRowsTNS = std::pair<std::string, TRowsFlag>;

	//����� ����� ����������� �� ���� ��������� � ��
	const int PrefetchRowsCnt = 200;

	//����� ����������� ��� ������������ ������ ��� ��������:
	class TBaseSheetReport
	{
	protected:
		TExcelBook& book;//������ �� excel-����
		TExcelBookSheet sheet;//���� excel-�����(��������� ������ ���� �� 0!!!)
		//�������� �� ������������� �������� ����� ��������:
		virtual bool NeedNewPage(size_t item_cnt, bool byRows = true) const noexcept(false);
		//������� ��������� ������/��������� ������ ��������:
		virtual size_t getRow(bool first) const noexcept(false) = 0;
		//������� �������� ��������� ����� �� ��������� ��������:
		virtual bool OpenBookSheet(const string& srcName, size_t page) noexcept(true);
	public:
		//������������� ������� �� ����� � ������� �� �������� ������ �����:
		explicit TBaseSheetReport(TExcelBook& book_ref, const TExcelBookSheet& sheet_ref) :
			book(book_ref), sheet(sheet_ref) {}
		//������������� ����� ��� ������ �������� ������ �����:
		//��������� �������� ���������� �������� ��� ���������:
		TBaseSheetReport(TExcelBook& book_ref, const string& src_file, size_t page_index);
		virtual ~TBaseSheetReport() {}
		//������� ��������� ��������� ������ ������:
		virtual size_t FirstRow() const noexcept(false) { return getRow(true); }
		//������� ��������� ��������� ������ ������:
		virtual size_t LastRow() const noexcept(false) { return getRow(false); }
		//������� �������� ��������� ������ � ��������:
		virtual bool inRange(size_t row, size_t col) const noexcept(false);
		//������� �������� ������ ������ �� excel-��������
		bool isEmptySheetData() const noexcept(true) { return sheet.getFirstRow() == sheet.getLastRow(); }
		//����������� ���� ������ excel � ��� ������ �� NS_Const::DataType
		static NS_Const::DataType convertExcelType(const NS_Excel::TDataType& dt, bool isDate = false) noexcept(true);
		static NS_Excel::TDataType convertDataType(const NS_Const::DataType& dt) noexcept(true);
	};

	//����� ��� ��������� excel-������ �� ��������� json-��������
	class TJsonReport : public TBaseSheetReport
	{
	private:
		size_t begin_row;//��������� ������ ���������
		size_t end_row;//�������� ������ ���������
		vector<TFilterData> filters;//������� ��� ������ ��������
		const TProcCell& cells_data;//���������� �� �������
		//������������� ����� ��������� �� ��������:
		void InitDstFile(const TShareData& dstFile, size_t page) noexcept(false);
		//������� ��������� ��������� ������� ��� ������:
		void setDQLParamByCell(TStatement& query, const NS_Excel::TExcelCell& cell,
			const TCellData& value) const noexcept(false);
		//������� ���������� ������ �� excel-����� ��������� ��� ������ excel-����� ���������:

	protected:
		//������� ��������� ������ �� ��������� excel-������ � ������ ���������� � excel-������ �����
		virtual void writeExcelFromDB(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false);
		//������� �������� ������������� ������ � ��(��� ����� ������):
		virtual bool checkINDataBase(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false);
		//������� ������ ������ � ��:
		virtual void insertToDataBase(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false);
		//������� ���������� ��������� � ����������� �� ������:
		bool ProcessByDataBase(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false);
		//������� ���������� ������� ��� ������:
		virtual bool runQuery(NS_Oracle::TStatement& query, size_t curRow) noexcept(true);
		//������� ��������� ������ �� �������� excel-�����:
		virtual bool setExcelRowDataByBD(NS_Oracle::TStatement& query, size_t curRow) noexcept(true);
		//������� ��������� ������ ��� �������/���������� � �������� excel-����� ��� ���� �����:
		virtual void setExcelDataByDB(TStatement& query, size_t& rowFrom) noexcept(false);
		//������� ���������� ���������� �� excel-�����
		virtual void UpdExcelDataByDB(TDBConnect& db, const TUserTune& tune, size_t& rowFrom) noexcept(false);
		//������� �������� ���������� ������� ���������� ��� ��������� ������:
		virtual bool CorrectFilter(size_t cur_row) const noexcept(true);
		//������� ��������� ���������� � sql-������ ��� ��������� ������:
		bool setSelectParams(TStatement& query, size_t curRow) const noexcept(true);
		//������� ��������� ������/��������� ������ ��������:
		virtual size_t getRow(bool first) const noexcept(false) { return first ? begin_row : end_row; }
		//������� ���������� ������ ��� �������� �� ���� ������:
		void ProcessSheetDataWithDB() noexcept(false);
		//������� ������� ������ �� excel-����� � ���� ������:

		//������� ������� ������ �� ������� excel-�����:

		//������� ��������� Excel-������

		//������� ��������� ������������ ������� �� ���������������� ������:
		std::string getServerNameByTuneIndex(size_t val) const noexcept(true);
		//������� ��������� ����� ���������������� ������:
		size_t getTuneFilesCnt() const noexcept(true) { return cells_data.getDBTuneArr().size(); }
		//������� ��������� ���� ������ ���������:
		NS_Const::JSonMeth getMethCode() const noexcept(true) { return cells_data.getMethodCode(); }
	public:
		//������������� ��� ������ �������� �� json-�����
		TJsonReport(TExcelBook& book_ref, const TExcelProcData& json_tune, size_t page);
		//���������������:
		virtual ~TJsonReport() {}
		//������� ��������� ������ ������ ���������:
		virtual size_t FirstRow() const noexcept(true);
		//������� ��������� ��������� ������ � ���������:
		virtual size_t LastRow() const noexcept(true);
		//������� ������������ ������
		virtual bool crtSheetReport() noexcept(true);
		//������� �������� �������������:
		bool preProcFlg() const noexcept(true) { return cells_data.getMethodCode() == NS_Const::JSonMeth::GetRowIDByDB; }

	};

	//����� ������������ ����� excel-������ �� ��������� js-����� ��������:
	class TJsonMarkReport: public TJsonReport
	{
	private:
		TRowsTNS& procRows;//������ ����� ��� ���������
		bool flg;//������� ���������� �� ����� ���������
	protected:
		//������� �������� ���������� ������� ���������� ��� ��������� ������:
		virtual bool CorrectFilter(size_t cur_row) const noexcept(true);
		//������� �������� ������������� ������ � ��:
		bool checkINDataBase(NS_Oracle::TResultSet& rs, size_t curRow) noexcept(false);
		//������� ��������� ������ ��� �������/���������� � �������� excel-����� ��� ���� �����:
		virtual void setExcelDataByDB(TStatement& query, size_t& rowFrom) noexcept(false);
		//������� ���������� ���������� �� excel-�����
		virtual void UpdExcelDataByDB(TDBConnect& db, const TUserTune& tune, size_t& rowFrom) noexcept(false);
	public:
		//������������� �������
		TJsonMarkReport(TExcelBook& book_ref, const TExcelProcData& json_tune, size_t page, TRowsTNS& Rows);
		//���������������:
		virtual ~TJsonMarkReport() {}
		//������� ���. ������� ������ �����:
		bool NoFltrRows() const noexcept(true) { return procRows.second.empty(); }
		//���������� ������ ������������ ����� ��� ��������:
		TRowsTNS& getProcRows() const noexcept(true) { return procRows; }
		//������� ��������� ������������ ������� ������������ ��� ������� ������:
		std::string getFlgName() const noexcept(true);
		//�������� ��������� �� ������ ����� ���������� ������:
		bool isFilterObject() const noexcept(true);
	};

	//����� ��� ������������ ������� �� ��������� ������ ��������
	//���������� ���� �� �� �
	class TSheetTuneReport: public TBaseSheetReport
	{
	protected:
		const TUserTune& tune;//��������� ��� ����� ������
		//������� ��������� ������/��������� ������ ��������:
		virtual size_t getRow(bool first) const noexcept(false) { return TBaseSheetReport::getRow(first); }
		//������� ������������� �������� �� ����������:
		virtual bool SetSheetByTune(const string& name = "") noexcept(true);
		//������� ���������� �������� ��� ������, ���� ������ ��������� �����������:
		virtual bool CreateNewPage(size_t val, bool byRows = true) noexcept(false);
	public:
		//�������� �������� ������ �� ����� � ������ �� ���� ���������
		TSheetTuneReport(TExcelBook& book_link, const TUserTune& config);
		//������������� ���������: 
		TSheetTuneReport(TExcelBook& ref_book, const TExcelBookSheet& ref_sheet, const TUserTune& ref_tune);
		//����������:
		virtual ~TSheetTuneReport() {}
		//������� ���������� �������� ������
		virtual bool crtSheet() noexcept(true) = 0;
	};

	//����� ��� �������������� � ��:
	struct TDataBaseInterface
	{
		static const  NS_Const::DataType DataTypeError = NS_Const::DataType::ErrorType;
		static const NS_Oracle::TType OraTypeError = NS_Oracle::TType::OCCIANYDATA;
		//������������ ���������� �����������:
		static TConnectParam getConnectParam(const TUserTune& param, int prefetch = 1) noexcept(false);
		//��������� �������� ��������� sql-������� �� ��������:
		static void setSqlParamByTune(TStatement& sql, const NS_Tune::TSubParam& param, bool use_raise = false) noexcept(false);
		//������� �������������� ����� ������ �� TOracle(TType) � ��� ������ TConstants(DataType)
		static NS_Const::DataType convertOraType(const NS_Oracle::TType& type) noexcept(true);
		//������� �������������� �� ����� ������ excel � ���� ������ oracle:
		static NS_Oracle::TType convertDataType(const NS_Const::DataType& type) noexcept(true);
		//��������� �������� ���������� sql-������� �� ����������:
		static void setSqlParamsByTune(TStatement& sql, const TUserTune& param) noexcept(false);
		//��������� ������ sql-������� � ��������� ��� ���:
		static string getSqlByTune(bool use_parse, bool by_str, const string& str) noexcept(true);
		//��������� ������ sql-������� �� ��������:
		static string getSqlByTune(const TUserTune& tune) noexcept(true);
		//��������� ������ ������� �� ������ ���� �� ��������� ��������, ���� ������ �����
		template <typename T>
		static string getSqlText(bool by_str, const string& str) noexcept(false);
		//����� ���������� ��������� dml-�������:
		static size_t executeDML(TDBConnect& db, const TUserTune& param, const string& dml, bool use_comit = true) noexcept(false);
		//����� dml-������� ��� ������:
		static size_t runDML4Directory(TDBConnect& db, const TUserTune& param, bool use_comit = true) noexcept(false);
		//���������� DML �������(����� ��������� ��� �������� � ������):
		static size_t runDML(TDBConnect& db, const TUserTune& param, bool use_comit = true) noexcept(true);
		//��������� �������� ���� �� ���� ������ �� ������� ������ sql-������
		static void setCellByResultSet(TExcelBook& book, TExcelBookSheet& sheet, 
			const TResultSet& rs, size_t resultSetCol, const NS_Excel::TExcelCell& cell) noexcept(false);
		//��������� ����� �������� ��� �������:
		static bool setMaxIterationCnt(TStatement& query, size_t cnt) noexcept(true);
		//���������� �������� ��� �������:
		static bool addQueryIteration(TStatement& query) noexcept(true);
	};

	//����� ��������� ������ ��� �������� �� ��
	class TDataBaseSheetReport: public TSheetTuneReport, public TDataBaseInterface
	{
	private:
		//��������� � ����������� ��������:
		enum class SqlType { DQL, DML };
		int prefetch_rows;//����� ����������� ����� �� ���� ��������� � ��
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
	protected:
		//������������ �������� ������ �� ����f�� �������:
		void CrtBySqlLine(TDBConnect& db, const string& sql_line, int prefetch = 1) noexcept(false);
		//������������ �������� ������ �� ������ sql-��������:
		void CrtBySqlFiles(TDBConnect& db, int prefetch = 1) noexcept(false);
	public:
		//������������� ������ � ������� ��������:
		TDataBaseSheetReport(TExcelBook& pbook, const TUserTune& config, int prefetch = PrefetchRowsCnt) : 
			TSheetTuneReport(pbook, config), prefetch_rows(prefetch) {}
		//������������� ������, ��������� � ������ ��������:
		TDataBaseSheetReport(TExcelBook& ref_book, const TExcelBookSheet& ref_sheet, const TUserTune& ref_tune,
			int prefetch = PrefetchRowsCnt) : TSheetTuneReport(ref_book, ref_sheet, ref_tune), prefetch_rows(prefetch) {}
		//������������ �������� ������:
		virtual bool crtSheet() noexcept(true);
	};

	class TReport
	{
	private:
		//�������� ��������� ��� ������������ ������
		TSharedTune config;
		//������� ���������� ������:
		void saveReport(NS_Excel::TExcelBook& book, const string& file_name = "") const noexcept(false);
		//������� ��������� ������ excel-����� �� ��������� js-����� ��������
		bool ProcessExcelFileByJson(TExcelBook& book, const string& js_file, 
			std::vector<NS_ExcelReport::TRowsTNS>& Rows) const noexcept(true);
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
		//��������� �� ��������� js-������:
		void run_JS_Excel_Report() const noexcept(false);
		//����� �������, ����������� ����� �� ����:
		void Create_Report_By_Code(const NS_Const::ReportCode& code) const;
		//������������ ������� ������ �� ����� main_config:
		bool Execute() const noexcept(true);
	};
}
#endif