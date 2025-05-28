REM Create the program folder.
if not exist "C:\Sd" mkdir "C:\Sd"
REM Save old versions.
if exist "C:\Sd\sdtty.exe" goto bad
if exist "C:\Sd\sd.exe" goto bad
if exist "C:\Sd\mkcalls.exe" goto bad
if exist "C:\Sd\sd_calls.txt" goto bad
if exist "C:\Sd\sd_calls.dat" goto bad
goto ok
:bad
cls
@echo off
echo.
echo WARNING!!!!!  A previous version of Sd/Sdtty was found.
echo   If you proceed with the installation, those files will
echo   be moved to the directory C:\sd_pre329 and the new version
echo   will be installed in their place.
echo.
echo   If you do NOT want to do this, you must stop the installation
echo   by typing control-C now, and then typing "Y".  If you type
echo   any other character, initialization will proceed.
pause
echo on
if not exist "C:\Sd_pre329" mkdir "C:\Sd_pre329"
if exist "C:\Sd\sdtty.exe" copy C:\sd\sdtty.exe C:\Sd_pre329\sdtty.exe
if exist "C:\Sd\sd.exe" copy C:\sd\sd.exe C:\Sd_pre329\sd.exe
if exist "C:\Sd\mkcalls.exe" copy C:\sd\mkcalls.exe C:\Sd_pre329\mkcalls.exe
if exist "C:\Sd\sdlib.dll" copy C:\sd\sdlib.dll C:\Sd_pre329\sdlib.dll
if exist "C:\Sd\sd_calls.txt" copy C:\sd\sd_calls.txt C:\Sd_pre329\sd_calls.txt
if exist "C:\Sd\sd_calls.dat" copy C:\sd\sd_calls.dat C:\Sd_pre329\sd_calls.dat
:ok
REM Install the program.
copy sd.exe C:\sd\sd.exe
copy sdtty.exe C:\sd\sdtty.exe
copy mkcalls.exe C:\sd\mkcalls.exe
copy sdlib.dll C:\sd\sdlib.dll
copy sd_calls.txt C:\sd\sd_calls.txt
copy sd_calls.dat C:\sd\sd_calls.dat
copy sd.ico C:\sd\sd.ico
copy sdby.ico C:\sd\sdby.ico
copy sdry.ico C:\sd\sdry.ico
copy sdtty.ico C:\sd\sdtty.ico
copy SD.lnk C:\sd\SD.lnk
copy SDTTY.lnk C:\sd\SDTTY.lnk
copy "Edit sd.ini.lnk" "C:\sd\Edit sd.ini.lnk"
copy sample1.ini C:\sd\sample1.ini
copy sample2.ini C:\sd\sample2.ini
copy sample3.ini C:\sd\sample3.ini
copy readme C:\sd\readme
REM Create Sd folder in start menu.
if not defined USERPROFILE goto try98
REM This seems to be Windows NT
if not exist "%USERPROFILE%\Start Menu\Programs\Sd" mkdir "%USERPROFILE%\Start Menu\Programs\Sd"
REM Create shortcuts in same.
copy SD.lnk "%USERPROFILE%\Start Menu\Programs\Sd\SD.lnk"
copy SDTTY.lnk "%USERPROFILE%\Start Menu\Programs\Sd\SDTTY.lnk"
copy "Edit sd.ini.lnk" "%USERPROFILE%\Start Menu\Programs\Sd\Edit sd.ini.lnk"
REM Display same in explorer.
%SystemRoot%\explorer.exe "%USERPROFILE%\Start Menu\Programs\Sd"
goto end
:try98
REM This seems to be Windows 98
if not exist "%windir%\Start Menu\Programs\Sd" mkdir "%windir%\Start Menu\Programs\Sd"
REM Create shortcuts in same.
copy SD.lnk "%windir%\Start Menu\Programs\Sd\SD.lnk"
copy SDTTY.lnk "%windir%\Start Menu\Programs\Sd\SDTTY.lnk"
copy "Edit sd.ini.lnk" "%windir%\Start Menu\Programs\Sd\Edit sd.ini.lnk"
REM Display same in explorer.
%windir%\explorer.exe "%windir%\Start Menu\Programs\Sd"
:end
