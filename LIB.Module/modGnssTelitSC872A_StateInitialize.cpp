/*
		case 0:
		{
			SendMsg("PMTK605");//Query the Firmware release information

			break;
		}
		case 2:
		{
			char Data[50];// = "PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0";//RMC, GGA [srg]2017-02-08 Size=47

			// 0 - GLL
			// 1 - RMC
			// 2 - VTG
			// 3 - GGA
			// 4 - GSA
			// 5 - GSV
			//17 - ZDA

			unsigned char Nmea_GGA = 0;
			unsigned char Nmea_GSV = 0;
			unsigned char Nmea_RMC = 0;

#ifdef LIB_MODULE_GNSS_NMEA_GGA
			Nmea_GGA = 1;
#endif//LIB_MODULE_GNSS_NMEA_GGA

#ifdef LIB_MODULE_GNSS_NMEA_GSV
			Nmea_GSV = 1;
#endif//LIB_MODULE_GNSS_NMEA_GSV

#ifdef LIB_MODULE_GNSS_NMEA_RMC
			Nmea_RMC = 1;
#endif//LIB_MODULE_GNSS_NMEA_RMC

			sprintf(Data, "PMTK314,%d,%d,%d,%d,%d,%d,0,0,0,0,0,0,0,0,0,0,0,%d,0",
				0,//p_obj->m_GnssSettings.Field.Nmea_GLL,
				Nmea_RMC,//p_obj->m_GnssSettings.Field.Nmea_RMC,
				0,//p_obj->m_GnssSettings.Field.Nmea_VTG,
				Nmea_GGA,//p_obj->m_GnssSettings.Field.Nmea_GGA,
				0,//p_obj->m_GnssSettings.Field.Nmea_GSA,
				Nmea_GSV,//p_obj->m_GnssSettings.Field.Nmea_GSV,
				0);//p_obj->m_GnssSettings.Field.Nmea_ZDA);

			SendMsg(Data);

			break;
*/