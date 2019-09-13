#ifndef TEXCEL_H_
#define TEXCEL_H_
#include <string>
#include <vector>
#include "Logger.h"
#include "libxl.h"

namespace NS_Excel
{
	using std::string;
	using libxl::Book;
	using libxl::Font;
	using libxl::Format;
	using libxl::Sheet;
	enum class TFormatTune {xlt, xls, xlsx, DefExt, DefName, DefSh};
	//�������������� ���������� � ������
	string getTuneFormat(const TFormatTune& val);
	//�������� �������� �� ���� ��������:
	bool isTemplate(const string& str);
	//������� ������ ������ �� ���������� ��� ��������(���� �� ������ ������ - ����� �������� ��������):
	//template <template <class TItem> class TArr>
	//bool WriteData<TItem>(const TArr<TItem>& arr);

	using TDataType = libxl::CellType;
	//enum class TDataFormat { Default = 0, AsString, AsInteger, AsDate, AsBool };

	//�����-���������� ��� �������� ���������� ��� ������ � excel-������
	class TExcel
	{
	private:
		enum DefVal {Empty = -1, MainIndx = 0, TitleIndx = 1,
			FontSize = 10, };
		Book* file;//excel-����
		Sheet* active_sh;//�������� ������� ����
		//Format* main_frmt;//�������� ������
		string name;//��� ��������� �����
		std::vector<TDataType> OrdColumnsFormat;
		//��������� ������������� � ����������
		TExcel(const TExcel& exl);
		TExcel& operator=(const TExcel& exl);
		//������� �������� ����� ��������� ����� �� ���������(����� ����� ���� ������ �� ���-�� ����� ��������):
		virtual void setDefOutFileName(void);
		//������� �������� ������ �� ���������:
		virtual void setDefFont(void) noexcept(false);
		//������� �������� ������� �� ���������:
		virtual void setDefFormat(void) noexcept(false);
		//������� ��������� ������ ��� ���������:
		virtual void setTitleFont(Font* src = 0);
		//������� ��������� ������� ��� ���������:
		virtual void setTitleFormat(Format* src = 0);
		//������� ������� ������ �� excel-�����
		virtual void clear(void);
		//������� ��������� ������/��������� ������ � �������:
		int getUsedRow(Sheet* sh, bool last = false) const;
		//������� ��������� ������/��������� ������ � �������
		int getUsedCell(Sheet* sh, bool last = false) const;
		//������� ���������� ������� ����� �� �������(� ������� ������ ������ ���� �������� ���������):
		//����������� ���� � ������ ���������, ����� ������� ��������� ������ �����:
		void FillColumnsFormat(int TitleRow = 1, int ShIndex = DefVal::Empty);
	public:
		//������������� - ��� �������� �����, ���� ����� ��� - �������
		//��� ������������� ������������ ������ ������,
		//����� ���������� ������� ����� � ����������� � ��� ��������
		//���� ���� �� ������ - ������������� ���� �� ���������, ��� ����� ������
		TExcel(const string& tmp_name = "", const string& out_name = "", bool crt_active_sh = false);
		//���������������
		virtual ~TExcel(void) { clear(); };
		//�������� �� ����� ��������:
		bool isTemplate(void) const { return file->isTemplate(); };
		//����������� ��� ������
		virtual void copyFont(int index, Font* src);
		//����������� ��� �������:
		virtual void copyFormat(int index, Format* src);
		//������� ��������� ����� �������:
		int SheetCount(void) const { return (file ? file->sheetCount() : DefVal::Empty); }
		//������� ���������� �������� � ������ �� ��������:
		bool AddSheet(const string& name, bool set_as_active = false);
		//������� ��������� �������� ��������:
		int ActiveSheet(void) const { return file ? file->activeSheet() : DefVal::Empty; }
		//������� ��������� �������� ��������:
		void setActiveSheet(int index);
		//������� ��������� ������ ���������� ������:
		int getFirstUsedfRow(int SheetIndex) const { return getUsedRow(active_sh, false); };
		//������� ��������� ��������� ���������� ������:
		int getLastUsedRow(int SheetIndex) const { return getUsedRow(active_sh, true); };
		//������� ��������� ������ ������ � ������� � ������
		int getFirstUsedCell(int SheetIndex) const { return getUsedCell(active_sh, false); };
		//������� ��������� ��������� ������ � ������� � ������
		int getLastUsedCell(int SheetIndex) const { return getUsedCell(active_sh, true); };
		inline bool WriteBool(bool val, int row, int col) { return active_sh->writeBool(row, col, val); };
		inline bool WriteBlank(int row, int col) { return active_sh->writeBlank(row, col, file->format(DefVal::MainIndx)); };
		inline bool WriteNum(double val, int row, int col) { return active_sh->writeNum(row, col, val); };
		inline bool WriteSrt(const string& str, int row, int col) { return active_sh->writeStr(row, col, str.c_str()); };
		//������� ������ ������:
		inline bool ReadBool(int row, int col) const { return active_sh->readBool(row, col); };
		inline double ReadNum(int row, int col) const { return active_sh->readNum(row, col); };
		inline string ReadStr(int row, int col) const { return string(active_sh->readStr(row, col)); };
	};
};
#endif