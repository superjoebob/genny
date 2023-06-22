SETLOCAL enableextensions
IF ERRORLEVEL 1 (
@ECHO Unable to enable command extensions
goto :error 1
)

set arg1=%1
set arg2=%2

IF NOT DEFINED versionnumber (
	set versionnumber = 100
)


IF "%arg1%" == "-v" (
	set versionnumber=%arg2%
)


"C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv.com" Genny.sln /build "Release|Win32" || goto :error
"C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv.com" Genny.sln /build "Release|x64" || goto :error
"C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv.com" Genny.sln /build "ReleaseFL|Win32" || goto :error
"C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv.com" Genny.sln /build "ReleaseFL|x64" || goto :error

supercopy "Genny\bin\ReleaseFL\x64\GennyFL_x64.dll" "Genny\dist\genny\fl\GennyFL\GennyFL_x64.dll" || goto :copyerror
supercopy "Genny\bin\ReleaseFL\x86\GennyFL.dll" "Genny\dist\genny\fl\GennyFL\GennyFL.dll" || goto :copyerror

supercopy "Genny\bin\Release\x64\Genny_x64.dll" "Genny\dist\genny\vst\Genny_x64.dll" || goto :copyerror
supercopy "Genny\bin\Release\x86\Genny.dll" "Genny\dist\genny\vst\Genny.dll" || goto :copyerror


supercopy "MegaMIDI-master\.pio\build\teensy2pp\firmware.hex" "Genny\dist\genny\megamidi_firmware\firmware.hex" || goto :copyerror


cd /D "Genny\dist\genny"
"..\..\..\zipper\7z.exe" a "..\genny_all_v%versionnumber%.zip" "*" -r  || goto :ziperror
cd /D "fl"
"..\..\..\..\zipper\7z.exe" a "..\..\genny_fl_v%versionnumber%.zip" "*" "..\genny_documentation.pdf" -r  || goto :ziperror
cd /D "..\vst"
"..\..\..\..\zipper\7z.exe" a "..\..\genny_vst_v%versionnumber%.zip" "*" "..\genny_documentation.pdf" -r  || goto :ziperror

cd /D "..\megamidi_firmware"
"..\..\..\..\zipper\7z.exe" a "..\..\genny_megamidi_firmware_v%versionnumber%.zip" "*" -r  || goto :ziperror

exit /b

:error
@ECHO Build FAILED with error code %ERRORLEVEL%
exit /b %ERRORLEVEL%
:copyerror
@ECHO supercopy FAILED with error code %ERRORLEVEL%
exit /b %ERRORLEVEL%
:ziperror
@ECHO 7Zip FAILED with error code %ERRORLEVEL%
exit /b %ERRORLEVEL%