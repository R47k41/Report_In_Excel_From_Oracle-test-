#ifndef TORACLE_H_
#define TORACLE_H_
//Заголовочный файл описывающий интерфейс для работы с БД Oracle
// с использованием OCCI библиотеки
//FAQ по OCCI:
// https://docs.oracle.com/en/database/oracle/oracle-database/19/lncpp/introduction-to-occi.html#GUID-99077609-BFCC-4FF7-9D23-C45623DDD465
//описание типов данных:
//https://docs.oracle.com/en/database/oracle/oracle-database/19/sqlrf/Data-Types.html#GUID-7B72E154-677A-4342-A1EA-C74C1EA928E6
#include <string>
#include "occi.h"

namespace NS_Oracle
{
	
	//using namespace oracle::occi;
	using std::string;
	using oracle::occi::Environment;
	using oracle::occi::Connection;
	using oracle::occi::Statement;
	
	//Класс-интерфейс Запрос:
	class TStatement
	{
		private:
		public:
	};
	//класс-интерфейс Соединение
	class TConnection
	{
		private:
			Connection* cnct;//соединение
		public:
			TStatement(Connection* )

	};

	//класс интерфейса для взаимодействия с БД oracle
	class TOracle
	{
	private:
		Environment* env;//переменная окружения
		Connection* connect;//текущее соединение
	public:
		TOracle(const string& uname = "", const string& pass = "", const string& connect_str = "");
		virtual ~TOracle();
	};
}


#endif