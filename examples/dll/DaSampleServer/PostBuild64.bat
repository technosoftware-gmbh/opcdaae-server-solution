IF "%~1" == "" goto default

if exist ..\..\..\src\Technosoftware\Server\ClassicServer\x64\DLL\DAOnly\OpcDllDaServer.exe copy ..\..\..\src\Technosoftware\Server\ClassicServer\x64\DLL\DAOnly\OpcDllDaServer.exe OpcDllDaServer.exe

if exist OpcDllDaServer.exe copy OpcDllDaServer.exe %1\OpcDllDaServer.exe
goto done

:default
if exist ..\..\..\src\Technosoftware\Server\ClassicServer\x64\DLL\DAOnly\OpcDllDaServer.exe copy ..\..\..\src\Technosoftware\Server\ClassicServer\x64\DLL\DAOnly\OpcDllDaServer.exe OpcDllDaServer.exe

if exist OpcDllDaServer.exe copy OpcDllDaServer.exe x64\Debug

:done