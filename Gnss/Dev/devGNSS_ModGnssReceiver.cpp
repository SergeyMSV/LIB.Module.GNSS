#include "devGNSS.h"
#include "devDataSet.h"
#include "devDB.h"
#include "devSettings.h"

#include <cstdio>
#include <fstream>

namespace dev
{

tGNSS::tModGnssReceiver::tModGnssReceiver(tGNSS* obj)
	:mod::tGnssReceiver(obj->m_pLog), m_pObj(obj), m_Board(this, *obj->m_pIO)
{

}

tGNSS::tModGnssReceiver::~tModGnssReceiver()
{

}

mod::tGnssTaskScript tGNSS::tModGnssReceiver::GetTaskScript(const std::string& id, bool userTaskScript)
{
	return g_Settings.GetTaskScript(id, userTaskScript);
}

mod::tGnssSettingsNMEA tGNSS::tModGnssReceiver::GetSettingsNMEA()
{
	return g_Settings.GetSettingsNMEA();
}

void tGNSS::tModGnssReceiver::OnChanged(const mod::tGnssDataSet& value)
{
	const std::string FileName = g_Settings.Output.Path + "/" + g_Settings.Output.FileName;
	const std::string FileNameTemp = FileName + ".tmp";
	std::fstream File = std::fstream(FileNameTemp, std::ios::out);
	if (File.is_open())
	{
		File << value.ToJSON();
		File.close();
	}
	std::remove(FileName.c_str());
	std::rename(FileNameTemp.c_str(), FileName.c_str());

	std::string Timestamp = db::GetTimestampNow();
	std::string DateTime = db::ToString(value.GetDateTime());

	my_ulonglong Res = db::InsertTablePos(Timestamp, static_cast<char>(value.GNSS), DateTime, value.Valid, value.Latitude, value.Longitude, value.Altitude, value.Speed, value.Course);

	db::tTableSatBulk TableSat(value.Satellite.size());

	auto SatIter = value.Satellite.cbegin();
	for (std::size_t i = 0; i < TableSat.size() && SatIter != value.Satellite.cend(); ++i, ++SatIter)
	{
		TableSat[i].pos_id = Res;
		TableSat[i].sat_id = SatIter->ID.Value;
		TableSat[i].elevation = SatIter->Elevation.Value;
		TableSat[i].azimuth = SatIter->Azimuth.Value;
		TableSat[i].snr = SatIter->SNR.Value;
	}

	db::InsertTablePosSatBulk(TableSat);

	m_pObj->m_pLog->WriteLine(true, utils::tLogColour::LightYellow, value.ToJSON());
}

void tGNSS::tModGnssReceiver::Board_PowerSupply(bool state)
{
	std::stringstream SStream;
	SStream << "Board_PowerSupply: " << state;
	m_pObj->m_pLog->WriteLine(true, utils::tLogColour::LightMagenta, SStream.str());
}

void tGNSS::tModGnssReceiver::Board_Reset(bool state)
{
	std::stringstream SStream;
	SStream << "Board_Reset:       " << state;
	m_pObj->m_pLog->WriteLine(true, utils::tLogColour::LightMagenta, SStream.str());
}

bool tGNSS::tModGnssReceiver::Board_Send(const utils::tVectorUInt8& data)
{
	return m_Board.Send(data);
}

void tGNSS::tModGnssReceiver::OnReceived(utils::tVectorUInt8& data)
{
	Board_OnReceived(data);
}

}
