#include "devDB.h"
#include "devSettings.h"

#include <exception>

#include <memory>//[TEST]
#include <mutex>

#include <iomanip>
#include <sstream>
#include <utility>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace dev
{
	namespace db
	{

struct tMYSQL
{
	mutable std::recursive_mutex Mtx;
	MYSQL MySQL{};
	std::string DB;

	std::uint8_t UpdateID = 0xFF;
	std::uint8_t RcvID = 0xFF;
}g_MySQL;

typedef std::lock_guard<std::recursive_mutex> tLockGuard;

typedef std::pair<std::string, std::string> tSQLQueryParamPair;
typedef std::vector<tSQLQueryParamPair> tSQLQueryParam;

my_ulonglong Insert(const std::string& table, const tSQLQueryParam& prm);

std::string GetErrorMsg(MYSQL* mysql)
{
	std::stringstream SStream;
	SStream << "MYSQL Error(" << mysql_errno(mysql) << ") [" << mysql_sqlstate(mysql) << "] \"" << mysql_error(mysql) << "\"";
	return SStream.str();
}

std::string GetErrorMsg(MYSQL_STMT* stmt)
{
	std::stringstream SStream;
	SStream << "MYSQL Error(" << mysql_stmt_errno(stmt) << ") [" << mysql_stmt_sqlstate(stmt) << "] \"" << mysql_stmt_error(stmt) << "\"";
	return SStream.str();
}

template<typename T>
std::string ToString(T value)
{
	std::stringstream Stream;

	if (std::is_floating_point<T>::value)
	{
		Stream << value;
	}
	else//std::is_integral<T>::value
	{
		Stream << (unsigned int)value;
	}

	return Stream.str();
}

std::string ToString(const std::tm& time)
{
	std::stringstream Stream;
	Stream << std::put_time(&time, "%F %T");
	return Stream.str();
}

std::string ToString(const std::time_t& timestamp)
{
	return ToString(*std::gmtime(&timestamp));
}

void Init(const std::string& host, const std::string& user, const std::string& passwd, const std::string& db, unsigned int port)
{
	tLockGuard Lock(g_MySQL.Mtx);

	g_MySQL.DB = db;

	mysql_init(&g_MySQL.MySQL);

	if (&g_MySQL.MySQL == nullptr)
		throw std::runtime_error{ GetErrorMsg(&g_MySQL.MySQL) };

	mysql_real_connect(&g_MySQL.MySQL, host.c_str(), user.c_str(), passwd.c_str(), "", port, "", 0);//DB is not specified intentionally

	if (mysql_errno(&g_MySQL.MySQL))
		throw std::runtime_error{ GetErrorMsg(&g_MySQL.MySQL) };
}

void Create()
{
	tLockGuard Lock(g_MySQL.Mtx);

	if (mysql_query(&g_MySQL.MySQL, std::string("CREATE DATABASE IF NOT EXISTS " + g_MySQL.DB).c_str()))
		throw std::runtime_error{ GetErrorMsg(&g_MySQL.MySQL) };

	if (mysql_query(&g_MySQL.MySQL, std::string("USE " + g_MySQL.DB).c_str()))
		throw std::runtime_error{ GetErrorMsg(&g_MySQL.MySQL) };

	const std::vector<std::string> ReqList
	{
		"CREATE TABLE sys (update_id INT(2) NOT NULL AUTO_INCREMENT, version VARCHAR(20) NOT NULL DEFAULT '', PRIMARY KEY(update_id));",
		"INSERT INTO sys (version) VALUE('" DEV_DB_VERSION "');",
		"CREATE TABLE rcv (rcv_id INT(2) NOT NULL AUTO_INCREMENT, timestamp DATETIME NOT NULL, model VARCHAR(50) NOT NULL DEFAULT '', id VARCHAR(50) NOT NULL DEFAULT '', PRIMARY KEY(rcv_id), UNIQUE INDEX(id));",
		"CREATE TABLE pos (pos_id INT(10) NOT NULL AUTO_INCREMENT, timestamp DATETIME NOT NULL, gnss INT(2), date_time DATETIME, valid BOOLEAN, latitude DOUBLE, longitude DOUBLE, altitude DOUBLE, speed FLOAT, course FLOAT, rcv_id INT(2) NOT NULL, update_id INT(2) NOT NULL, PRIMARY KEY(pos_id), INDEX(timestamp));",
		"CREATE TABLE pos_sat (pos_id INT(10) NOT NULL, sat_id INT(3) NOT NULL, elevation INT(2), azimuth INT(3), snr INT(2), PRIMARY KEY(pos_id, sat_id));",
		"CREATE TABLE sat (sat_id INT(3) NOT NULL, gnss VARCHAR(50) NOT NULL DEFAULT '', description VARCHAR(50) NOT NULL DEFAULT '', PRIMARY KEY(sat_id));",
	};

	for (auto& i : ReqList)
	{
		if (mysql_query(&g_MySQL.MySQL, i.c_str()))
		{
			if (mysql_errno(&g_MySQL.MySQL) == ER_TABLE_EXISTS_ERROR)
				return;

			throw std::runtime_error{ GetErrorMsg(&g_MySQL.MySQL) };
		}
	}

	boost::property_tree::ptree PTree;
	boost::property_tree::xml_parser::read_xml(g_Settings.GetConfigFileName(), PTree);

	if (auto Value = PTree.get_child_optional("App.DB_Init.Table"))
	{
		std::string TableID;

		for (auto i : *Value)
		{
			if (i.first == "<xmlattr>")
			{
				TableID = i.second.get<std::string>("ID");
			}
			else if (i.first == "Row")
			{
				if (TableID == "sat")
				{
					if (auto Attr = i.second.get_child_optional("<xmlattr>"))
					{
						std::string Sat_ID = Attr->get<std::string>("ID");
						std::string GNSS = Attr->get<std::string>("GNSS");
						std::string Descript = Attr->get<std::string>("Description");

						const tSQLQueryParam Query
						{
							{"sat_id", Sat_ID},
							{"gnss", GNSS},
							{"description", Descript},
						};

						Insert("sat", Query);
					}
				}
			}
		}
	}
}

void Open()
{
	tLockGuard Lock(g_MySQL.Mtx);

	Init(g_Settings.DB.Host, g_Settings.DB.User, g_Settings.DB.Passwd, g_Settings.DB.DB, g_Settings.DB.Port);

	if (mysql_query(&g_MySQL.MySQL, std::string("USE " + g_MySQL.DB).c_str()))
	{
		const unsigned int Cerr = mysql_errno(&g_MySQL.MySQL);

		if (Cerr != ER_BAD_DB_ERROR)
			throw std::runtime_error{ GetErrorMsg(&g_MySQL.MySQL) };
	}

	Create();

	tTable TableSys = GetTableSys();

	if (TableSys.size() == 1 && TableSys.front().size() == 2 && TableSys.front()[1] == DEV_DB_VERSION)
	{
		g_MySQL.UpdateID = utils::Read<std::uint8_t>(TableSys.front()[0].cbegin(), TableSys.front()[0].cend(), utils::tRadix_10);
	}
	else
	{
		throw std::runtime_error{ "Wrong DEV_DB_VERSION" };
	}

	for (int i = 0; i < 2; ++i)//if the receiver is not on the list - insert it and repeate the search
	{
		tTable TableRCV = GetTableRcv();

		for (tTableRow& row : TableRCV)
		{
			if (row.size() != 4)
				throw std::runtime_error{ "Wrong TableRCV rows qty" };

			if (row[2] == g_Settings.Main.Model && row[3] == g_Settings.Main.ID)
			{
				g_MySQL.RcvID = utils::Read<std::uint8_t>(row[0].cbegin(), row[0].cend(), utils::tRadix_10);
				return;
			}
		}

		InsertTableRcv();
	}

	if (mysql_errno(&g_MySQL.MySQL))
		throw std::runtime_error{ GetErrorMsg(&g_MySQL.MySQL) };
}

my_ulonglong Insert(const std::string& table, const tSQLQueryParam& prm)
{
	std::stringstream SStream;

	SStream << "INSERT INTO " << table << " (";

	for (std::size_t i = 0; i < prm.size(); ++i)
	{
		SStream << prm[i].first;

		if (i < prm.size() - 1)
		{
			SStream << ',';
		}
	}

	SStream << ") VALUE(";

	for (std::size_t i = 0; i < prm.size(); ++i)
	{
		SStream << "'" << prm[i].second << "'";

		if (i < prm.size() - 1)
		{
			SStream << ',';
		}
	}

	SStream << ");";

	const std::string ReqStr = SStream.str();

	tLockGuard Lock(g_MySQL.Mtx);

	if (mysql_real_query(&g_MySQL.MySQL, ReqStr.c_str(), static_cast<unsigned long>(ReqStr.size())))//Contrary to the mysql_query() function, mysql_real_query is binary safe.
		throw std::runtime_error{ GetErrorMsg(&g_MySQL.MySQL) };

	return mysql_insert_id(&g_MySQL.MySQL);
}

my_ulonglong InsertTablePos(const std::string& timestamp, char gnss, const std::string& dateTime, bool valid, double latitude, double longitude, double altitude, double speed, double course)
{
	const tSQLQueryParam Query
	{
		{"timestamp", timestamp},
		{"gnss", ToString(gnss)},
		{"date_time", dateTime},
		{"valid", ToString(valid)},
		{"latitude", ToString(latitude)},
		{"longitude", ToString(longitude)},
		{"altitude", ToString(altitude)},
		{"speed", ToString(speed)},
		{"course", ToString(course)},
		{"rcv_id", ToString(g_MySQL.RcvID)},
		{"update_id", ToString(g_MySQL.UpdateID)},
	};

	return Insert("pos", Query);
}

//my_ulonglong InsertTablePosSat(int pos_id, int sat_id, int elevation, int azimuth, int snr, int& cerr)
//{
//	const tSQLQueryParam Query
//	{
//		{"pos_id", ToString(pos_id)},
//		{"sat_id", ToString(sat_id)},
//		{"elevation", ToString(elevation)},
//		{"azimuth", ToString(azimuth)},
//		{"snr", ToString(snr)},
//	};
//
//	return Insert("pos_sat", Query, cerr);
//}

void InsertTablePosSatBulk(tTableSatBulk& table)
{
	tLockGuard Lock(g_MySQL.Mtx);

	MYSQL_STMT* Stmt = mysql_stmt_init(&g_MySQL.MySQL);

	if (Stmt == nullptr)
		throw std::runtime_error{ GetErrorMsg(Stmt) };

	const unsigned int ColumnQty = 5;

	if(mysql_stmt_prepare(Stmt, "INSERT INTO pos_sat VALUES (?,?,?,?,?)", -1))//[TBD] - columns
		throw std::runtime_error{ GetErrorMsg(Stmt) };

	std::unique_ptr<MYSQL_BIND> Bind{ new MYSQL_BIND[ColumnQty] };

	auto InsertBulk = [=, &table](MYSQL_BIND* bind, std::size_t& count) -> void
	{
		if (count >= table.size())
			return;//No Error at least from DB

		const std::size_t RowSize = sizeof(tTableSatBulkRow);
		const std::size_t ArraySize = table.size();
		//std::size_t ArraySize = table.size() - count;
		//if (ArraySize > 16)
		//{
		//	ArraySize = 16;
		//}

		memset(bind, 0, sizeof(MYSQL_BIND) * ColumnQty);

		bind[0].buffer = &table[count].pos_id;
		bind[0].buffer_type = MYSQL_TYPE_LONG;
		bind[0].u.indicator = &table[count].pos_id_ind;

		bind[1].buffer = &table[count].sat_id;
		bind[1].buffer_type = MYSQL_TYPE_LONG;
		bind[1].u.indicator = &table[count].sat_id_ind;

		bind[2].buffer = &table[count].elevation;
		bind[2].buffer_type = MYSQL_TYPE_LONG;
		bind[2].u.indicator = &table[count].elevation_ind;

		bind[3].buffer = &table[count].azimuth;
		bind[3].buffer_type = MYSQL_TYPE_LONG;
		bind[3].u.indicator = &table[count].azimuth_ind;

		bind[4].buffer = &table[count].snr;
		bind[4].buffer_type = MYSQL_TYPE_LONG;
		bind[4].u.indicator = &table[count].snr_ind;

		if (mysql_stmt_attr_set(Stmt, STMT_ATTR_ARRAY_SIZE, &ArraySize))
			throw std::runtime_error{ GetErrorMsg(Stmt) };

		if (mysql_stmt_attr_set(Stmt, STMT_ATTR_ROW_SIZE, &RowSize))
			throw std::runtime_error{ GetErrorMsg(Stmt) };

		if (mysql_stmt_bind_param(Stmt, bind))
			throw std::runtime_error{ GetErrorMsg(Stmt) };

		if (mysql_stmt_execute(Stmt))
			throw std::runtime_error{ GetErrorMsg(Stmt) };

		count += ArraySize;

		return;
	};

	std::size_t Count = 0;
	while (Count < table.size())
	{
		InsertBulk(Bind.get(), Count);
	}

	mysql_stmt_close(Stmt);
}

my_ulonglong InsertTableRcv()
{
	std::string Timestamp = GetTimestamp(std::time(nullptr));

	const tSQLQueryParam Query
	{
		{"timestamp", Timestamp},
		{"model", g_Settings.Main.Model},
		{"id", g_Settings.Main.ID},
	};

	return Insert("rcv", Query);
}

tTable GetTable(const std::string& table)
{
		tTable List;

		tLockGuard Lock(g_MySQL.Mtx);

		std::string Query("SELECT * FROM ");
		Query += table + ';';

		if (mysql_query(&g_MySQL.MySQL, Query.c_str()))
			throw std::runtime_error{ GetErrorMsg(&g_MySQL.MySQL) };

		MYSQL_RES* Res = mysql_use_result(&g_MySQL.MySQL);

		if (Res == nullptr)
			throw std::runtime_error{ GetErrorMsg(&g_MySQL.MySQL) };

		unsigned int Count = mysql_num_fields(Res);

		while (true)//[#] true or [limit]
		{
			tTableRow Row;

			MYSQL_ROW AB = mysql_fetch_row(Res);

			if (AB == nullptr)
				break;

			for (unsigned int i = 0; i < Count; ++i)
			{
				Row.push_back(AB[i]);
			}

			List.push_back(std::forward<tTableRow>(Row));
		}

		mysql_free_result(Res);

		return List;
}

tTable GetTablePos()
{
	return GetTable("pos");
}

tTable GetTableRcv()
{
	return GetTable("rcv");
}

tTable GetTableSat()
{
	return GetTable("sat");
}

tTable GetTableSys()
{
	return GetTable("sys");
}

void Drop()
{
	tLockGuard Lock(g_MySQL.Mtx);

	if (mysql_query(&g_MySQL.MySQL, std::string("DROP DATABASE IF EXISTS " + g_MySQL.DB).c_str()))
		throw std::runtime_error{ GetErrorMsg(&g_MySQL.MySQL) };
}

void Clear()
{
	Close();
	Drop();
	Open();
}

void Close()
{
	tLockGuard Lock(g_MySQL.Mtx);

	mysql_close(&g_MySQL.MySQL);
}

std::string GetTimestamp(const std::time_t& timestamp)
{
	return ToString(timestamp);
}

std::string GetTimestamp(unsigned char year, unsigned char month, unsigned char day, unsigned char hour, unsigned char minute, unsigned char second)
{
	std::tm Time{};
	Time.tm_year = year + 100;
	Time.tm_mon = month - 1;
	Time.tm_mday = day;
	Time.tm_hour = hour;
	Time.tm_min = minute;
	Time.tm_sec = second;

	return ToString(Time);
}

std::string GetTimestampNow()
{
	return GetTimestamp(std::time(nullptr));
}

	}
}
