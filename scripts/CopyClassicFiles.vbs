Option Explicit

Dim shell, fso
Dim projectPath, rootDir, platformName, dotnetVersion, srcFile, dstFile, configFile, genericServerFile, licenseFile

Set shell = WScript.CreateObject("WScript.Shell")

If WScript.Arguments.Count > 0 Then
    projectPath = WScript.Arguments(0)
    WScript.Echo "Project Path: " & projectPath 
End If

If WScript.Arguments.Count > 1 Then
    rootDir = WScript.Arguments(1)
    WScript.Echo "Root Dir: " & rootDir 
End If

WScript.Echo "" 

Set fso = CreateObject("Scripting.FileSystemObject")

genericServerFile = rootDir & platformName & "\OpcNetDaAeServer.exe"

WScript.Echo "Generic Server File: " & genericServerFile
  
If fso.FileExists(genericServerFile) Then

  Set srcFile = fso.GetFile(genericServerFile)
  srcFile.Copy projectPath   

  WScript.Echo "Copied: " & genericServerFile

End If
