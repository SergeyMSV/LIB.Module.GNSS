#include "devSettings.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace dev
{

tSettings g_Settings;

tSettings::tSettings(const std::string& fileName)
	:m_ConfigFileName(fileName)
{
	boost::property_tree::ptree PTree;
	boost::property_tree::xml_parser::read_xml(m_ConfigFileName, PTree);

	if (auto Value = PTree.get_child_optional("App.Settings.OutputFile"))
	{
		auto ValueIter = (*Value).begin();

		if (ValueIter->first == "<xmlattr>")
		{
			Output.Path = ValueIter->second.get<std::string>("Path");
			Output.FileName = ValueIter->second.get<std::string>("FileName");
		}
	}

	if (auto Value = PTree.get_child_optional("App.Settings.GNSS_Receiver"))
	{
		auto ValueIter = (*Value).begin();

		if (ValueIter->first == "<xmlattr>")
		{
			Main.Model = ValueIter->second.get<std::string>("Model");
			Main.ID = ValueIter->second.get<std::string>("ID");
		}
	}

	if (auto Value = PTree.get_child_optional("App.Settings.SerialPort"))
	{
		auto ValueIter = (*Value).begin();

		if (ValueIter->first == "<xmlattr>")
		{
			SerialPort.ID = ValueIter->second.get<std::string>("ID");
			SerialPort.BR = ValueIter->second.get<std::uint32_t>("BR");
		}
	}
}

mod::tGnssTaskScript tSettings::GetTaskScript(const std::string& id, bool userTaskScript) const
{
	boost::property_tree::ptree PTree;
	boost::property_tree::xml_parser::read_xml(m_ConfigFileName, PTree);

	mod::tGnssTaskScript Script;

	std::string Path = "App.Settings.GNSS_Receiver.";
	Path += (userTaskScript ? "UserTaskScript." : "TaskScript.");

	if (auto Value = PTree.get_child_optional(Path + id))
	{
		for (auto i : *Value)
		{
			if (i.first == "REQ")
			{
				if (auto Attr = i.second.get_child_optional("<xmlattr>"))
				{
					auto Value = std::make_unique<mod::tGnssTaskScriptCmdREQ>();
					Value->Msg = Attr->get<std::string>("Msg");
					Value->RspHead = Attr->get<std::string>("RspHead");
					Value->RspBody = Attr->get<std::string>("RspBody");
					Value->CaseRspWrong = Attr->get<std::string>("CaseRspWrong");
					Value->RspWait_us = Attr->get<decltype(Value->RspWait_us)>("RspWait_us");
					Value->Pause_us = Attr->get<decltype(Value->Pause_us)>("Pause_us");
					Script.push_back(std::move(Value));
				}
			}
			else if (i.first == "GPI")
			{
				if (auto Attr = i.second.get_child_optional("<xmlattr>"))
				{
					auto Value = std::make_unique<mod::tGnssTaskScriptCmdGPI>();
					Value->ID = Attr->get<std::string>("ID");
					Value->State = Attr->get<decltype(Value->State)>("State");
					Value->Wait_us = Attr->get<decltype(Value->Wait_us)>("Wait_us");
					Script.push_back(std::move(Value));
				}
			}
			else if (i.first == "GPO")
			{
				if (auto Attr = i.second.get_child_optional("<xmlattr>"))
				{
					auto Value = std::make_unique<mod::tGnssTaskScriptCmdGPO>();
					Value->ID = Attr->get<std::string>("ID");
					Value->State = Attr->get<decltype(Value->State)>("State");
					Value->Pause_us = Attr->get<decltype(Value->Pause_us)>("Pause_us");
					Script.push_back(std::move(Value));
				}
			}
		}
	}

	return Script;
}

mod::tGnssSettingsNMEA tSettings::GetSettingsNMEA() const
{
	boost::property_tree::ptree PTree;
	boost::property_tree::xml_parser::read_xml(m_ConfigFileName, PTree);

	if (auto Value = PTree.get_child_optional("App.Settings.GNSS_Receiver.NMEA"))
	{
		auto ValueIter = (*Value).begin();

		mod::tGnssSettingsNMEA Settings;

		if (ValueIter->first == "<xmlattr>")
		{
			Settings.PeriodMax = ValueIter->second.get<std::uint32_t>("PeriodMAX_us");
		}

		for (auto i : *Value)
		{
			if (i.first != "<xmlcomment>")
			{
				Settings.MsgLast = i.first;
			}
		}

		return Settings;
	}

	return {};
}

}