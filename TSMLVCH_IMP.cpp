//определение функционала по импорту из Смолевича
#include "TSMLVCH_IMP.h"
#include <sstream>
#include <fstream>
#include <string.h>
#include "Logger.hpp"

using NS_Logger::TLog;
using std::string;

void NS_SMLVCH_IMP::TAccount::show(std::ostream& stream) const
{
	using std::endl;
	if (stream)
	{
		stream << "Счет: " << Account() << endl
			<< "Наименование счета:" << Name() << endl
			<< "Остаток в рублях: " << SldRub() << endl
			<< "Дата последней операции: " << LastOperDate() << endl
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
	name.setValue(str);
}

void NS_SMLVCH_IMP::TAccount::addName(const string& val, const NS_Const::CtrlSym& delimeter)
{
	using NS_Const::TConstCtrlSym;
	string str(name.ValueStr());
	setName(val);
	str += TConstCtrlSym::asStr(delimeter) + name.ValueStr();
	name.setValue(str);
}

string NS_SMLVCH_IMP::TImportAccount::divide_str(const string& str, char delimeter, size_t& pos)
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

NS_SMLVCH_IMP::StrArr NS_SMLVCH_IMP::TImportAccount::ConvertOEM2Arr(const string& file) noexcept(true)
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
				tmp = OEM2Char(tmp, OEM::MAX_LEN);
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

void NS_SMLVCH_IMP::TImportAccount::setNameByArr(const StrArr& Rows, size_t indx_row, 
	const NS_SMLVCH_IMP::TImportAccount::TConditionStr& condition)
{
	if (indx_row > Rows.size() or indx_row < 0)
	{
		TLog("Указан не верный индекс! Имя файла не сформировано", "TImportAccount::setNameByArr").toErrBuff();
		return;
	}
	if (Rows[indx_row].find(condition.first) != string::npos)
		name = condition.second.first;
	else
		name = condition.second.second;
}

bool NS_SMLVCH_IMP::TImportAccount::ConvertOEM2File(const string& file, const NS_Const::CtrlSym& delimeter) noexcept(true)
{
	using NS_Const::TConstCtrlSym;
	using std::ofstream;
	//получение строк файла
	StrArr Text = ConvertOEM2Arr(file);
	if (Text.empty()) return false;
	string name;
	if (Text[4].find_first_of("Внеб"))
			name = "unbalance.txt";
		else
			name = "balance.txt";
	try
	{
	ofstream out_file(name, std::ios_base::out);
	if (out_file.is_open())
		for (const string& s : Text) out_file << s << std::endl;
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

void NS_SMLVCH_IMP::TImportAccount::LoadFromOEMFile(const string& file, const NS_Const::CtrlSym& delimeter,
	const NS_Const::CtrlSym& last_line)
{
	using NS_Const::TConstCtrlSym;
	using NS_Const::Trim;
	//!!!!Здесь нужно избавиться от номеров в массиве и использовать тип Настрока
	//формируем массив из строк файла
	StrArr Fields = ConvertOEM2Arr(file);
	setNameByArr(Fields, 4, TConditionStr(std::make_pair("Внеб", std::make_pair("Внебалансовые счета", "Балансовые счета"))));
	const char internal_d = TConstCtrlSym::asChr(delimeter);
	const string external_d = TConstCtrlSym::asStr(last_line);
	bool prev_Rec = false;
	//проходим по каждой строке и формируем запись для TAccount
	for (const string& str : Fields)
	{
		//столбцы строки
		StrArr fields;
		//позиция отсчета:
		size_t pos = 0;
		//проходим по полям строки
		while (pos < str.size())
		{
			//!!!!!!!!!Здесь можно доработать, чтобы отсеивать текстовые строки:
			//типа итоги и заголовки
			string tmp = divide_str(str, internal_d, pos);
			if (pos == string::npos) continue;
			Trim(tmp);
			fields.push_back(tmp);
		}
		//если есть поля под запись:
		if (!fields.empty())
		{
			//проверяем поле "Номер счета":
			if (isdigit(unsigned char(fields[0][0])) > 0)
			{
				//формирование записи о счете:
				bool isActive = false;
				double sld = 0;
				string saldo = fields[2];
				NS_Converter::toDblType(saldo, &sld);
				if (sld > 0)
					isActive = true;
				else
					saldo = fields[3];
				TAccount account(fields[0], fields[1], saldo, fields[4], isActive);
				accs.push_back(account);
				prev_Rec = true;
				continue;
			}
			//если "Номер счета" - пуст, но это продолжение предыдущей записи:
			if (fields[0].empty() and prev_Rec and !fields[1].empty())
			{
				accs[accs.size() - 1].addName(fields[1]);
				continue;
			}
			prev_Rec = false;
		}
	}
}

NS_SMLVCH_IMP::TImportAccount::TImportAccount(const string& txt_file, const NS_Const::CtrlSym& delimeter,
	const NS_Const::CtrlSym& last_line)
{
	LoadFromOEMFile(txt_file, delimeter, last_line);
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
	string tmp = str + delimeter;
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
	ss << tmp << UsrCode << delimeter << TRSBankClient::getOEMStr(Note, delimeter);
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