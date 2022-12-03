IF "%~1" == "" goto default

if exist ..\..\..\src\Technosoftware\Server\ClassicServer\Win32\DLL\Release\OpcDllDaAeServer.exe copy ..\..\..\src\Technosoftware\Server\ClassicServer\Win32\DLL\Release\OpcDllDaAeServer.exe OpcDllDaAeServer.exe
if exist ..\..\..\src\Technosoftware\Server\ClassicServer\Win32\DLL\Debug\OpcDllDaAeServer.exe copy ..\..\..\src\Technosoftware\Server\ClassicServer\Win32\DLL\Debug\OpcDllDaAeServer.exe OpcDllDaAeServer.exe

if exist OpcDllDaAeServer.exe copy OpcDllDaAeServer.exe %1\OpcDllDaAeServer.exe
goto done

:default
if exist ..\..\..\src\Technosoftware\Server\ClassicServer\Win32\DLL\Release\OpcDllDaAeServer.exe copy ..\..\..\src\Technosoftware\Server\ClassicServer\Win32\DLL\Release\OpcDllDaAeServer.exe OpcDllDaAeServer.exe
if exist ..\..\..\src\Technosoftware\Server\ClassicServer\Win32\DLL\Debug\OpcDllDaAeServer.exe copy ..\..\..\src\Technosoftware\Server\ClassicServer\Win32\DLL\Debug\OpcDllDaAeServer.exe OpcDllDaAeServer.exe

if exist OpcDllDaAeServer.exe copy OpcDllDaAeServer.exe Win32\Debug

:done