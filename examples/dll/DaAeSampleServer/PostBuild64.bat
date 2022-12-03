IF "%~1" == "" goto default

if exist ..\..\..\src\Technosoftware\Server\ClassicServer\x64\DLL\Release\OpcDllDaAeServer.exe copy ..\..\..\src\Technosoftware\Server\ClassicServer\x64\DLL\Release\OpcDllDaAeServer.exe OpcDllDaAeServer.exe
if exist ..\..\..\src\Technosoftware\Server\ClassicServer\x64\DLL\Debug\OpcDllDaAeServer.exe copy ..\..\..\src\Technosoftware\Server\ClassicServer\x64\DLL\Debug\OpcDllDaAeServer.exe OpcDllDaAeServer.exe

if exist OpcDllDaAeServer.exe copy OpcDllDaAeServer.exe %1\OpcDllDaAeServer.exe
goto done

:default
if exist ..\..\..\src\Technosoftware\Server\ClassicServer\x64\DLL\Release\OpcDllDaAeServer.exe copy ..\..\..\src\Technosoftware\Server\ClassicServer\x64\DLL\Release\OpcDllDaAeServer.exe OpcDllDaAeServer.exe
if exist ..\..\..\src\Technosoftware\Server\ClassicServer\x64\DLL\Debug\OpcDllDaAeServer.exe copy ..\..\..\src\Technosoftware\Server\ClassicServer\x64\DLL\Debug\OpcDllDaAeServer.exe OpcDllDaAeServer.exe

if exist OpcDllDaAeServer.exe copy OpcDllDaAeServer.exe x64\Debug

:done