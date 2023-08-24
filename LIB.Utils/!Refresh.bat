del /Q *.cpp
del /Q *.h

xcopy /Y %LIB_UTILS%\utilsBase.*
xcopy /Y %LIB_UTILS%\utilsCRC.*
xcopy /Y %LIB_UTILS%\utilsFile.*
xcopy /Y %LIB_UTILS%\utilsLog.*
xcopy /Y %LIB_UTILS%\utilsPacket.*
xcopy /Y %LIB_UTILS%\utilsPacketNMEA.*
xcopy /Y %LIB_UTILS%\utilsPacketNMEAPayload.*
xcopy /Y %LIB_UTILS%\utilsPacketNMEAType.*
xcopy /Y %LIB_UTILS%\utilsPath.*
xcopy /Y %LIB_UTILS%\utilsSerialPort.*
xcopy /Y %LIB_UTILS%\utilsShell.*

pause
