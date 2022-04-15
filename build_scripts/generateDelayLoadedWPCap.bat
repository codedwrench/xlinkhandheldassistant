@ECHO OFF
if not exist ..\Objects (mkdir ..\Objects)
cd ..\Objects

if not defined DLLPath (
  set DLLPath=%WINDIR%\System32\Npcap
)

gendef %DLLPath%\wpcap.dll
dlltool -dllname %WINDIR%\System32\Npcap\wpcap.dll --def wpcap.def --output-delaylib wpcap.a
cd ..