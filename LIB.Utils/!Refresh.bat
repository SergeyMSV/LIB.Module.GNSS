del /Q *.cpp
del /Q *.h

set PATH_SOURCE=..\..\LIB.Utils\Lib.Utils

xcopy /Y %PATH_SOURCE%\utilsBase.*
xcopy /Y %PATH_SOURCE%\utilsCRC.*
xcopy /Y %PATH_SOURCE%\utilsLog.*
xcopy /Y %PATH_SOURCE%\utilsPacket.*
xcopy /Y %PATH_SOURCE%\utilsPacketNMEA.*
xcopy /Y %PATH_SOURCE%\utilsPacketNMEAPayload.*
rem xcopy /Y %PATH_SOURCE%\utilsPacketNMEAPayloadPMTK.*
rem xcopy /Y %PATH_SOURCE%\utilsPacketNMEAPayloadPTWS.*
xcopy /Y %PATH_SOURCE%\utilsPacketNMEAType.*
rem xcopy /Y %PATH_SOURCE%\utilsPatternCommand.*
xcopy /Y %PATH_SOURCE%\utilsPatternState.*
xcopy /Y %PATH_SOURCE%\utilsShell.*

pause
