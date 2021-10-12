del /Q *.cpp
del /Q *.h

set PATH_SOURCE=..\..\LIB.Utils\Lib.Utils

xcopy /Y %PATH_SOURCE%\utilsBase.*
xcopy /Y %PATH_SOURCE%\utilsCRC.*
xcopy /Y %PATH_SOURCE%\utilsLog.*
xcopy /Y %PATH_SOURCE%\utilsPacket.*
xcopy /Y %PATH_SOURCE%\utilsPacketNMEA.*
xcopy /Y %PATH_SOURCE%\utilsPacketNMEAPayload.*
xcopy /Y %PATH_SOURCE%\utilsPacketNMEAType.*
xcopy /Y %PATH_SOURCE%\utilsShell.*

pause
