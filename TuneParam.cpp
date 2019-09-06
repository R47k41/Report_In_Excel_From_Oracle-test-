#include <ios>
#include "TuneParam.h"

using std::string;

string NS_Tune::TuneFieldToStr(const TuneField& val)
{
	switch (val)
	{
	case TuneField::DataRange: return "\"";
	case TuneField::DataBase: return "[DATA BASE]";
	case TuneField::UserName: return "UserName";
	case TuneField::Password: return "Password";
	case TuneField::TNS: return "TNS";
	case TuneField::Report: return "[REPORT]";
	case TuneField::OutFile: return "OutFileName";
	case TuneField::SqlFile: return "SQLFile";
	case TuneField::SqlText: return "SQLText";
	case TuneField::Columns: return "[COLUMNS]";
	case TuneField::Column: return "Column";
	default: return string();
	}
};

bool NS_Tune::operator==(const TuneField& val, const string& str)
{
	if (TuneFieldToStr(val) == str) return true;
	return false;
};

bool NS_Tune::operator<(const TuneField& v1, const TuneField& v2)
{
	return TuneFieldToInt(v1) < TuneFieldToInt(v2);
};