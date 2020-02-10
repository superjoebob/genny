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


"C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv.com" Genny.sln /build "Release|Win32"
"C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv.exe" Genny.sln /build "Release|x64"
"C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv.exe" Genny.sln /build "ReleaseFL|Win32"
"C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv.exe" Genny.sln /build "ReleaseFL|x64"
robocopy . "Genny\dist\genny" "genny_documentation.pdf"
robocopy ".\installer" "Genny\dist\genny\fl"

cd /D "Genny\dist\genny"
"..\..\..\zipper\7z.exe" a "..\genny_all_v%versionnumber%.zip" "*" -r
cd /D "fl"
"..\..\..\..\zipper\7z.exe" a "..\..\genny_fl_v%versionnumber%.zip" "*" "..\genny_documentation.pdf" -r
cd /D "..\vst"
"..\..\..\..\zipper\7z.exe" a "..\..\genny_vst_v%versionnumber%.zip" "*" "..\genny_documentation.pdf" -r