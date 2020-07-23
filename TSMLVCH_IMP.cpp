//определение функционала по импорту из Смолевича
#include "TSMLVCH_IMP.h"
#include <sstream>
#include <fstream>
#include <string.h>
#include "Logger.hpp"

using NS_Logger::TLog;
using std::string;

void NS_SMLVCH_IMP::TAccount::setFieldByIndex(const StrArr& arr, size_t index) noexcept(true)
{
	using NS_Converter::toDblType;
	switch (index)
	{
	case FieldIndexs::Account:
		acc = arr[index];
		break;
	case FieldIndexs::Name:
		name = arr[index];
		break;
	//остатки на счете:
	case FieldIndexs::Saldo_Active:
	{
		//если остаток не установлен - то это активный счет
		if (sld_rub == 0)
		{
			toDblType(arr[index], &sld_rub);
			active_flg = true;
		}
		break;
	}
	case FieldIndexs::Saldo_Passive:
	{
		//если остаток не установлен - пассивный счет
		if (sld_rub == 0)
		{
			toDblType(arr[index], &sld_rub);
			active_flg = false;
		}
		break;
	}
	case FieldIndexs::Last_Op_Date:
		last_op_date = arr[index];
		break;
	default:
		{
			TLog log("Индекс ", "TAccount::setBFieldByIndex");
			log << index << " не обрабатывается!";
			log.toErrBuff();
		}
	}
}

NS_SMLVCH_IMP::TAccount::TAccount(const StrArr& fields, const NS_Tune::CellDataArr& params): sld_rub(0)
{
	using NS_Tune::TCellData;
	//если не заполнены данные по параметрам или полям - выход
	if (fields.empty() || params.empty()) return;
	//проходим по всем параметрам и выставляем данные из колонок основываясь на параметрах:
	for (const TCellData& param : params)
	{
		//индекс параметра массива должен быть заполнен
		if (param.EmptySrcParam()) continue;
		//т.к. индексы параметров нумеруются от 1 - вычитаем 1
		setFieldByIndex(fields, param.SrcParam(false));
	}
}

void NS_SMLVCH_IMP::TAccount::show(std::ostream& stream) const
{
	using std::endl;
	if (stream)
	{
		stream << "Счет: " << getAccount() << endl
			<< "Наименование счета:" << getName() << endl
			<< "Остаток в рублях: " << getSldRub() << endl
			<< "Дата последней операции: " << getLastOperDate() << endl
			<< (active_flg ? "Активный" : "Пассивный") << endl;
	}
}

void NS_SMLVCH_IMP::TAccount::setName(const string& val)
{
	using NS_Const::Trim;
	string str(val);
	//получение строки без пробелов в начале и конце
	Trim(str);
	//присвоение
	name = str;
}

void NS_SMLVCH_IMP::TAccount::addName(const string& val, const NS_Const::CtrlSym& delimeter)
{
	using NS_Const::TConstCtrlSym;
	if (val.empty()) return;
	name += TConstCtrlSym::asStr(delimeter) + val;
}

bool NS_SMLVCH_IMP::TAccount::isValidStrArr(const StrArr& arr) noexcept(true)
{
	if (arr.empty() || arr[FieldIndexs::Account].empty())
		return false;
	return true;
}

string NS_SMLVCH_IMP::TImportBalance::divide_str(const string& str, char delimeter, size_t& pos) noexcept(false)
{
	//pos - начинается от позиции данных подстроки(без разделителя)
	//позиция окончания подстроки:
	int posb = pos + 1;
	pos = str.find_first_of(delimeter, posb);
	if (pos == string::npos)
		return string();
	//возвращаем значение между ограничителями
	return str.substr(posb, pos - posb);
}

 NS_SMLVCH_IMP::StrArr NS_SMLVCH_IMP::TImportBalance::divide_by_cols(const string& str, char delimeter) noexcept(true)
{
	using NS_Const::Trim;
	StrArr arr;
	size_t pos = 0;
	while (pos < str.size())
	{
		string tmp = divide_str(str, delimeter, pos);
		if (pos == string::npos) continue;
		//необходимо проверить является ли первое считанное поле корректным:
		//номер счета может быть пустым либо числовым значением
		Trim(tmp);
		//если считывается номер счета
		//поле может быть либо пустым либо числовым значением
		if (arr.size() == 0 and tmp.empty() == false and isdigit(unsigned char(tmp[0])) == 0) break;
		arr.push_back(tmp);
	}
	return arr;
}

NS_SMLVCH_IMP::StrArr NS_SMLVCH_IMP::TImportBalance::ConvertOEM2ANSI(const string& file) noexcept(true)
{
	using NS_Const::TConstCtrlSym;
	using std::ifstream;
	using std::getline;
	using NS_Converter::OEM2Char;
	StrArr Text;
	try
	{
		//инициализация входного файла
		ifstream txt(file, std::ios_base::in);
		if (txt.is_open() == false)
			throw TLog("Ошибка при открытии файла: " + file, "TImportAccount::ConvertOEMFile");
		int line = 0;//счетчик строк файла
		//char* buf = new char[OEM::MAX_LEN];//бкфер для кодировки
		string tmp;//буфер для считываемой строки
		while (txt)
		{
			//считываение строки
			getline(txt, tmp);
			//если строка не пустая:
			if (tmp.empty() == false)
			{
				tmp = OEM2Char(tmp, Fields::OEM_MAX_LEN);
				if (!tmp.empty())
					Text.push_back(tmp);
			}
			line++;
		}
		txt.close();
		TLog log("Обработано ", "TImportAccount::ConvertOEMFile");
		log << line << " строк исходного файла!\n";
		log.toErrBuff();
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (const std::exception& err)
	{
		TLog log("Ошибка при конвертации файла: ", "TImportAccount::ConvertOEMFile");
		log << file << '\n' << err.what() << '\n';
		log.toErrBuff();
	}
	catch (...)
	{
		TLog log("Не обработанная ошибка при конвертации файла: ", "TImportAccount::ConvertOEMFile");
		log << file << '\n';
		log.toErrBuff();
	}
	return Text;
}

string NS_SMLVCH_IMP::TImportBalance::getNameByArr(const StrArr& Rows, const NS_Tune::TConditionValue& condition) noexcept(true)
{
	using NS_Tune::TConditionValue;
	using NS_Tune::ConditionValues;
	//если нет данных для определения имени группа
	if (condition.isEmpty()) return string();
	size_t index = condition.getColIndx();
	if (index > Rows.size() or index < 0)
	{
		TLog("Указан не верный индекс! Имя файла не сформировано", "TImportAccount::setNameByArr").toErrBuff();
		return string();
	}
	return condition.getResultValue(Rows[index]);
}

bool NS_SMLVCH_IMP::TImportBalance::ConvertOEM2File(const string& file, const NS_Tune::TConditionValue& condition,
	const NS_Const::CtrlSym& delimeter) noexcept(true)
{
	using NS_Const::TConstCtrlSym;
	using std::ofstream;
	//получение строк файла
	StrArr arr = ConvertOEM2ANSI(file);
	if (arr.empty()) return false;
	string name = getNameByArr(arr, condition);
	try
	{
	ofstream out_file(name, std::ios_base::out);
	if (out_file.is_open())
		for (const string& s : arr) out_file << s << std::endl;
	else
	{
		TLog log("Не удалось открыть файл: ", "TImportAccount::ConvertOEMFile");
		log << name << " для записи!\n";
		throw log;
	}
	out_file.close();
	}
	catch (const TLog& err)
	{
		err.toErrBuff();
	}
	catch (const std::exception& err)
	{
		TLog log("Ошибка при конвертации файла: ", "TImportAccount::ConvertOEMFile");
		log << file << '\n' << err.what() << '\n';
		log.toErrBuff();
	}
	catch (...)
	{
		TLog log("Не обработанная ошибка при конвертации файла: ", "TImportAccount::ConvertOEMFile");
		log << file << '\n';
		log.toErrBuff();
	}
	return false;
}

void NS_SMLVCH_IMP::TImportBalance::InitAccountsByParams(const StrArr& Rows, const NS_Tune::CellDataArr& params, char delimeter) noexcept(true)
{
	using NS_Const::TConstCtrlSym;
	using NS_Const::Trim;
	//разделитель для конца файла
	//const string external_d = tune.getDelimeter(1);
	//признак типа счета - актив/пассив:
	bool typeFlg = false;
	//проходим по каждой строке и формируем запись для TAccount
	for (const string& str : Rows)
	{
		if (str[0] != delimeter) continue;
		//столбцы строки
		StrArr fields = divide_by_cols(str, delimeter);
		if (fields.empty()) continue;
		//если полученные поля валидны:
		if (TAccount::isValidStrArr(fields))
		{
			//инициализация счета:
			TAccount acc(fields, params);
			typeFlg = acc.isActive();
			if (typeFlg)
				active.push_back(acc);
			else
				passive.push_back(acc);
		}
		//если поля не валидны:
		else
		{
			//проверяем является ли данный массив продолжением информации по предыдущему счету:
			string note = TAccount::getNameByArr(fields);
			if (note.empty()) continue;
			//добавление информации к предыдущему значению:
			if (typeFlg)
				active[active.size() - 1].addName(note);
			else
				passive[passive.size() - 1].addName(note);
		}
	}
}

void NS_SMLVCH_IMP::TImportBalance::InitByTune(const string& file, const NS_Tune::TBalanceTune& tune) noexcept(true)
{
	using NS_Tune::TSimpleTune;
	using NS_Tune::TConditionValue;
	//если настройки не заполнены или пустое имя файла - выход
	if (tune.isEmpty() || file.empty() || tune.NoDelimeters()) return;
	//конвертируем входной файл в кодировку ANSI в виде списка строк:
	StrArr fields = ConvertOEM2ANSI(file);
	//установка имени группы ипортируемых данных:
	const TConditionValue& condition = tune.getCondition(Fields::COND_NAME_INDX);
	name = getNameByArr(fields, condition);
	//если имя пустое вставляем туда имя файла:
	if (name.empty()) name = TSimpleTune::getOnlyName(file);
	//инициалиазция массивов групп счетов:
	const NS_Tune::CellDataArr& params = tune.getParams();
	char delimeter = tune.getDelimeter(Delimiters::Internal)[0];
	InitAccountsByParams(fields, params, delimeter);
}

NS_SMLVCH_IMP::TImportBalance::TImportBalance(const string& file, const NS_Tune::TBalanceTune& tune)
{
	InitByTune(file, tune);
}

string NS_SMLVCH_IMP::TRSBankClient::getAccData(char delimeter) const noexcept(true)
{
	using std::stringstream;
	stringstream ss;
	ss << Account << delimeter << AccNum << delimeter << Code << delimeter << Inn << delimeter << InnSub << delimeter;
	return ss.str();
}

string NS_SMLVCH_IMP::TRSBankClient::getOEMStr(const string& str, char delimeter) noexcept(true)
{
	using NS_Converter::ANSI2OEMStr;
	string tmp = delimeter ? str + delimeter : str;
	if (tmp.size() > 1)
		tmp = ANSI2OEMStr(tmp);
	return tmp;
}

string NS_SMLVCH_IMP::TRSBankClient::getNameData(char delimeter) const noexcept(true)
{
	using std::stringstream;
	stringstream ss;
	ss << getOEMStr(Name, delimeter) << getOEMStr(Bank, delimeter);
	return ss.str();
}

void NS_SMLVCH_IMP::TRSBankClient::setByDefault(const TRSBankClient& dflt) noexcept(true)
{
	if (Inn.empty()) Inn = dflt.Inn;
	if (InnSub.empty()) InnSub = dflt.InnSub;
	if (Name.empty()) Name = dflt.Name;
	if (Bank.empty()) Bank = dflt.Bank;
}

bool NS_SMLVCH_IMP::TRSBankDoc::InitByStrArr(const StrArr& arr) noexcept(true)
{
	try
	{
		TypeDoc = arr[2];
		Shifr = arr[3];
		payer.Inn = arr[8];
		payer.InnSub = arr[9];
		recipient.Inn = arr[14];
		recipient.InnSub = arr[15];
		HardFlag = arr[17];
		Pachka = arr[18];
		SymKas = arr[19];
		queue = arr[20];
		payer.Name = arr[21];
		payer.Bank = arr[22];
		recipient.Name = arr[23];
		recipient.Bank = arr[24];
		GetMeth = arr[28];
		return true;
	}
	catch (...)
	{
		TLog("Ошибка при установке атрибутов из шаблона!", "InitByStrArr").toErrBuff();
	}
	return false;
}

bool NS_SMLVCH_IMP::TRSBankDoc::isEmptyDocAtr() const noexcept(true)
{
	TLog log("", "TRSBankDoc::isEmptyDocAtr");
	if (Num.empty()) log << "Не указан номер документа!\n";
	if (Date.empty()) log << "Не указана дата документа!\n";
	if (Summa.empty()) log << "Не указана сумма документа!\n";
	if (Note.empty()) log << "Не указано Назначение платежа\n";
	if (log.isEmpty()) return false;
	log.toErrBuff();
	return true;
}

bool NS_SMLVCH_IMP::TRSBankDoc::isEmpty() const noexcept(true)
{
	if (isEmptyDocAtr()) return true;
	TLog log("", "TRSBankDoc::isEmpty");
	if (payer.isEmptyAcc()) log << "Не заполнен счет плательщика!\n";
	if (recipient.isEmptyAcc()) log << "Не заполнен счет получателя!\n";
	if (log.isEmpty()) return false;
	log.toErrBuff();
	return true;
}

bool NS_SMLVCH_IMP::TRSBankDoc::isValid() const noexcept(true)
{
	if (isEmpty()) return false;
	TLog log("", "TRSBankDoc::isValid");
	if (TypeDoc.empty()) log << "Не указан Тип документа!\n";
	if (Shifr.empty()) log << "Не указан Шифр документа!\n";
	if (queue.empty()) log << "Не указана Очередность платежа!\n";
	if (GetMeth.empty()) log << "Не указан Метод передачи документа!\n";
	if (log.isEmpty()) return true;
	log.toErrBuff();
	return false;
}

string NS_SMLVCH_IMP::TRSBankDoc::getAtrStr(char delimeter) const noexcept(true)
{
	using std::stringstream;
	if (isValid() == false) return string();
	stringstream ss;
	ss << Num << delimeter << Date << delimeter << TypeDoc << delimeter;
	ss << Shifr << delimeter << CreaterState << delimeter << payer.getAccData(delimeter);
	ss << payerBIK_Filial << delimeter << recipient.getAccData(delimeter) << Summa << delimeter;
	ss << HardFlag << delimeter << Pachka << delimeter << SymKas << delimeter << queue << delimeter;
	ss << payer.getNameData(delimeter) << recipient.getNameData(delimeter);
	string tmp = getEmptyAttr(delimeter, FIRST_EMPTY_BLOCK);
	ss << tmp << TRSBankClient::getOEMStr(GetMeth, delimeter);
	tmp = getEmptyAttr(delimeter, SECOND_EMPTY_BLOCK);
	ss << tmp << UsrCode << delimeter << TRSBankClient::getOEMStr(Note, '\0');
	return ss.str();
}

void NS_SMLVCH_IMP::TRSBankDoc::setDefaultAtr(const TRSBankDoc& dflt) noexcept(true)
{
	//проверяем основные аттрибуты документа:
	//Вид документа:
	if (TypeDoc.empty()) TypeDoc = dflt.TypeDoc;
	//Шифр документа:
	if (Shifr.empty()) Shifr = dflt.Shifr;
	//Информация о плательщике:
	payer.setByDefault(dflt.payer);
	//Информация о получателе:
	recipient.setByDefault(dflt.recipient);
	//сложная проводка
	HardFlag = dflt.HardFlag;
	//пачка
	if (Pachka.empty()) Pachka = dflt.Pachka;
	//кассовый символ
	SymKas = dflt.SymKas;
	//очередность
	queue = dflt.queue;
	//метод получения
	GetMeth = dflt.GetMeth;
}

string NS_SMLVCH_IMP::TRSBankDoc::getEmptyAttr(char delimeter, size_t count) noexcept(true)
{
	string str;
	for (size_t i = 0; i < count; i++) str+= delimeter;
	return str;
}

bool NS_SMLVCH_IMP::TRSBankImp::InitDefaultAttr(const string& file) noexcept(true)
{
	using std::ifstream;
	if (file.empty())
	{
		TLog("Не задано имя файла шаблона! Значения по умолчанию не установлены!", "InitDefaultAttr").toErrBuff();
		return false;
	}
	//инициализация файла:
	ifstream tmpl(file, std::ios_base::in);
	if (!tmpl.is_open())
	{
		TLog log("Ошибка открытия файла:", "InitDefaultAttr");
		log << file << '!';
		log.toErrBuff();
		return false;
	}
	//получение первого символа из файла - это и есть разделитель
	setDelimiter(tmpl.get());
	//массив параметров шаблона:
	StrArr attrs(1);
	string str;
	//считывание только первой строки до Enter
	while (tmpl)
	{
		char ch = tmpl.get();
		if (ch == delimeter)
		{
			attrs.push_back(str);//добавление строки в массив
			str.clear();
		}
		else
			str += ch;//формирование строки
	}
	//закрываем файл
	tmpl.close();
	//производим считывание шаблонной строки:
	if (tmp_doc.InitByStrArr(attrs))
	{
		TLog("Шаблон для формирования документов заполнен успешно!", "TRSBankImp::InitDefaultAttr").toErrBuff();
		return true;
	}
	TLog("Шаблон для формирования документов не заполнен!", "TRSBankImp::InitDefaultAttr").toErrBuff();
	return false;
}

NS_SMLVCH_IMP::TRSBankImp::TRSBankImp(const string& file): delimeter('\0')
{
	if (file.empty()) return;
	InitDefaultAttr(file);
}

bool NS_SMLVCH_IMP::TRSBankImp::CreateFile4Import(const string& file) const noexcept(true)
{
	using std::ofstream;
	//using NS_Converter::ANSI2OEMStr;
	TLog log("", "TRSBankImp::CreateFile4Import");
	//не указан фала - выход
	if (file.empty())
	{
		log << "Не указано имя файла для импорта!";
		log.toErrBuff();
		return false;
	}
	//нет документов для импорта - выход
	if (docs.empty())
	{
		log << "Нет документов для импорта!";
		log.toErrBuff();
		return false;
	}
	//инициализация файла для записи:
	ofstream out(file, std::ios_base::out);
	if (!out.is_open())
	{
		log << "Не удалось открыть файл " << file << " для записи!";
		log.toErrBuff();
		return false;
	}
	//записываем реквизиты документов в файл:
	size_t count = 0;
	for (const TRSBankDoc& doc : docs)
	{
		string str = doc.getAtrStr(delimeter);
		//преобразование строки в OEM-кодировку
		if (!str.empty())
		{
			out << str << std::endl;
			count++;
		}
	}
	out.close();
	if (count > 0)
	{
		log << "Запись в файл успешно произведена!\n";
		log << "Записано " << count << " строк!";
		log.toErrBuff();
		return true;
	}
	log << "В файл ничего не записано!";
	log.toErrBuff();
	return false;
}

bool NS_SMLVCH_IMP::TRSBankImp::AddDoc(TRSBankDoc& doc) noexcept(true)
{
	//если реквизиты документа пустые - выход
	if (doc.isEmpty()) return false;
	//заполнение для документа аттрибутов по умолчанию:
	doc.setDefaultAtr(tmp_doc);
	if (doc.isValid() == false) return false;
	docs.push_back(doc);
	return true;
}