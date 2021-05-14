///////////////////////////////////////////////////////////////////////////////////////////////////
// devDB.h
//
// Standard ISO/IEC 114882, C++17
//
// |   version  |    release    | Description
// |------------|---------------|---------------------------------
// |      1     |   2020 03 31  |
// |            |               | 
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <devConfig.h>

#include <mysql.h>
#include <mysqld_error.h>

#include <list>
#include <string>
#include <vector>
#include <utility>

#include <ctime>

namespace dev
{
	namespace db
	{

std::string ToString(const std::tm& time);

void Open();

my_ulonglong InsertTablePos(const std::string& timestamp, char gnss, const std::string& dateTime, bool valid, double latitude, double longitude, double altitude, double speed, double course);
//my_ulonglong InsertTablePosSat(int pos_id, int sat_id, int elevation, int azimuth, int snr, int& cerr);

struct tTableSatBulkRow
{
	my_ulonglong pos_id = 0;
	char pos_id_ind = STMT_INDICATOR_NONE;
	int sat_id = 0;
	char sat_id_ind = STMT_INDICATOR_NONE;
	int elevation = 0;
	char elevation_ind = STMT_INDICATOR_NONE;
	int azimuth = 0;
	char azimuth_ind = STMT_INDICATOR_NONE;
	int snr = 0;
	char snr_ind = STMT_INDICATOR_NONE;
};

typedef std::vector<tTableSatBulkRow> tTableSatBulk;

void InsertTablePosSatBulk(tTableSatBulk& table);

my_ulonglong InsertTableRcv();

typedef std::vector<std::string> tTableRow;
typedef std::list<tTableRow> tTable;

tTable GetTablePos();
tTable GetTableRcv();
tTable GetTableSat();
tTable GetTableSys();

void Drop();
void Clear();
void Close();

std::string GetTimestamp(const std::time_t& timestamp);
std::string GetTimestamp(unsigned char year, unsigned char month, unsigned char day, unsigned char hour, unsigned char minute, unsigned char second);
std::string GetTimestampNow();

	}
}