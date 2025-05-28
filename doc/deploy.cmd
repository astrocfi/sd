if not exist "C:\Sd" mkdir "C:\Sd"
copy sd.exe C:\sd\sd.exe
copy sdtty.exe C:\sd\sdtty.exe
copy mkcalls.exe C:\sd\mkcalls.exe
copy sd_calls.txt C:\sd\sd_calls.txt
copy sd_calls.dat C:\sd\sd_calls.dat
copy sdtty.ico C:\sd\sdtty.ico
copy SD.lnk C:\sd\SD.lnk
copy SDTTY.lnk C:\sd\SDTTY.lnk
copy "Edit sd.ini.lnk" "C:\sd\Edit sd.ini.lnk"
copy sample1.ini C:\sd\sample1.ini
copy sample2.ini C:\sd\sample2.ini
copy sample3.ini C:\sd\sample3.ini
copy readme C:\sd\readme
if not exist "%USERPROFILE%\Start Menu\Programs\Sd" mkdir "%USERPROFILE%\Start Menu\Programs\Sd"
copy SD.lnk "%USERPROFILE%\Start Menu\Programs\Sd\SD.lnk"
copy SDTTY.lnk "%USERPROFILE%\Start Menu\Programs\Sd\SDTTY.lnk"
copy "Edit sd.ini.lnk" "%USERPROFILE%\Start Menu\Programs\Sd\Edit sd.ini.lnk"
%SystemRoot%\explorer.exe "%USERPROFILE%\Start Menu\Programs\Sd"
