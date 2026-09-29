@echo off
rem Builds Toggle-AlphaRing.exe using the C# compiler that ships with Windows.
"%WINDIR%\Microsoft.NET\Framework64\v4.0.30319\csc.exe" /nologo /target:winexe /platform:x64 /optimize+ ^
  /win32manifest:"%~dp0app.manifest" /r:System.Windows.Forms.dll ^
  /out:"%~dp0Toggle-AlphaRing.exe" "%~dp0ToggleAlphaRing.cs"
