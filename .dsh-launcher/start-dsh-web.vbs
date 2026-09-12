' Launch the DSH Web service without a console window and open the browser.
' The desktop shortcut may point at this file: double-click = service + Web UI.
' ASCII only, so any system codepage parses it identically.
'
' Optional diagnostics (not needed for normal use):
'   wscript start-dsh-web.vbs 3099              -> serve on another port
'   wscript start-dsh-web.vbs 3099 nobrowser    -> do not open a browser
Option Explicit

Dim fso, shell, scriptDir, ps1, powershell, cmd, rc, extra, argCount
Set fso = CreateObject("Scripting.FileSystemObject")
Set shell = CreateObject("WScript.Shell")

scriptDir = fso.GetParentFolderName(WScript.ScriptFullName)
ps1 = fso.BuildPath(scriptDir, "start-dsh-web.ps1")

If Not fso.FileExists(ps1) Then
    MsgBox "Launcher script not found:" & vbCrLf & ps1, 16, "DSH Web"
    WScript.Quit 1
End If

' Prefer PowerShell 7 when present, otherwise Windows PowerShell 5.1.
powershell = shell.ExpandEnvironmentStrings("%ProgramFiles%") & "\PowerShell\7\pwsh.exe"
If Not fso.FileExists(powershell) Then
    powershell = shell.ExpandEnvironmentStrings("%SystemRoot%") & "\System32\WindowsPowerShell\v1.0\powershell.exe"
End If

extra = ""
argCount = WScript.Arguments.Count
If argCount >= 1 Then extra = extra & " -Port " & WScript.Arguments(0)
If argCount >= 2 Then
    If LCase(WScript.Arguments(1)) = "nobrowser" Then extra = extra & " -NoBrowser"
End If

cmd = """" & powershell & """ -NoLogo -NoProfile -ExecutionPolicy Bypass -File """ & ps1 & """" & extra
rc = shell.Run(cmd, 0, False)
WScript.Quit rc
