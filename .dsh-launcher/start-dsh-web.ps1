#Requires -Version 5.1
<#
  DSH Web one-click launcher (ASCII source: parses identically under any codepage)

  Double-clicking the desktop shortcut runs this file through start-dsh-web.vbs,
  so no console window ever appears.

  Behaviour
    - If a DSH Web service is already up, the launcher only opens the browser:
      no duplicate instance, no EADDRINUSE, no extra tab.
    - Otherwise it starts `dsh web` hidden, waits for readiness, then opens the
      default browser at the tokenized URL the service prints. That token is the
      only way in: the bare http://127.0.0.1:3080/ answers 401.
    - It then stays alive as the service's parent, so stopping the launcher stops
      the service. Signing out or rebooting ends both.

  How "already up" is decided
    1. A loopback probe of the port. This is refused inside a locked-down
       launcher context, so a negative answer is never trusted on its own.
    2. The pid record this script writes for the port, accepted only when that
       process is alive, was started before the record was written, and really is
       a dsh service bound to this port.
    3. If a start still loses the port to another process, the service itself
       reports EADDRINUSE and the launcher hands over to the existing one.

  Files (all under <launcher>\logs)
    launcher.log        this launcher's own record, append-only, all runs
    dsh-web.out.log     service stdout (holds the tokenized URL)
    dsh-web.err.log     service stderr (startup failures)
    state-<port>.json   last good URL and process id for that port
    service-<port>.pid  process id of the service this script started

  Manual use
    powershell -NoProfile -File start-dsh-web.ps1
    powershell -NoProfile -File start-dsh-web.ps1 -Port 3099 -NoBrowser
#>
[CmdletBinding()]
param(
    [string]$Workspace = 'E:\MCU_TEST_CODE',
    [int]$Port = 3080,
    [int]$WaitSeconds = 90,
    [switch]$NoBrowser
)

$ErrorActionPreference = 'Continue'
$AsciiHost = '127.0.0.1'

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$logDir = Join-Path $scriptDir 'logs'
# The launcher's record, the service's stdout and its stderr are three separate
# files: the service keeps its redirect targets open for its whole life, so one
# shared file would lock out the launcher's appends and mix output together.
$launcherLog = Join-Path $logDir 'launcher.log'
$outFile = Join-Path $logDir 'dsh-web.out.log'
$errFile = Join-Path $logDir 'dsh-web.err.log'
$stateFile = Join-Path $logDir ('state-' + $Port + '.json')
$pidFile = Join-Path $logDir ('service-' + $Port + '.pid')
$url = 'http://' + $AsciiHost + ':' + $Port

if (-not (Test-Path -LiteralPath $logDir)) {
    New-Item -ItemType Directory -Path $logDir -Force | Out-Null
}

function Write-LauncherLog {
    param([string]$Message)
    $line = '[' + (Get-Date -Format 'yyyy-MM-dd HH:mm:ss') + '] ' + $Message
    try { Add-Content -LiteralPath $launcherLog -Value $line -Encoding ASCII } catch { }
}

function Write-TextNoBom {
    param([string]$Path, [string]$Text)
    try { [System.IO.File]::WriteAllText($Path, $Text, (New-Object System.Text.ASCIIEncoding)) } catch { }
}

function Clear-LogFile {
    param([string]$Path)
    # Start-Process appends to a redirect target that already exists, so clear
    # first; otherwise an earlier run's tokenized URL would be read back as if
    # it were this run's.
    Write-TextNoBom -Path $Path -Text ''
}

function Get-LogText {
    param([string]$Path)
    try {
        if (Test-Path -LiteralPath $Path) { return [string](Get-Content -LiteralPath $Path -Raw -ErrorAction SilentlyContinue) }
    } catch { }
    return ''
}

function Test-PortListening {
    # Authoritative when it says yes; never trusted when it says no, because a
    # restricted launcher context can be refused the loopback connection.
    param([int]$TargetPort, [int]$TimeoutMs = 700)
    $client = New-Object System.Net.Sockets.TcpClient
    try {
        $iar = $client.BeginConnect($AsciiHost, $TargetPort, $null, $null)
        if ($iar.AsyncWaitHandle.WaitOne($TimeoutMs)) {
            $client.EndConnect($iar)
            return $true
        }
        return $false
    } catch {
        return $false
    } finally {
        try { $client.Close() } catch { }
    }
}

function Get-RecordedPid {
    try {
        if (Test-Path -LiteralPath $pidFile) {
            $raw = (Get-Content -LiteralPath $pidFile -Raw -ErrorAction SilentlyContinue)
            if (-not [string]::IsNullOrWhiteSpace($raw)) { return [int]$raw.Trim() }
        }
    } catch { }
    return 0
}

function Test-RecordedServiceAlive {
    # Is the process this launcher recorded for this port still the service?
    $recorded = Get-RecordedPid
    if ($recorded -le 0) { return $false }
    if ($recorded -eq $PID) { return $false }

    try { $proc = Get-Process -Id $recorded -ErrorAction Stop } catch { return $false }

    # A pid recycled long after the record was written would otherwise look like
    # a running service. The service is started before its record is written.
    try {
        $recordedAt = (Get-Item -LiteralPath $pidFile).LastWriteTime
        if ($proc.StartTime -gt $recordedAt.AddSeconds(5)) {
            Write-LauncherLog ('ignoring pid ' + $recorded + ': it started after our record was written (recycled pid)')
            return $false
        }
    } catch { }

    # And it must really be a dsh service bound to this port, not any process
    # that happens to hold the pid.
    try {
        $query = "ProcessId=$recorded"
        $cim = Get-CimInstance Win32_Process -Filter $query -ErrorAction Stop
        $cmd = [string]$cim.CommandLine
        if ($cmd -notlike '*dsh*' -or $cmd -notlike ('*' + $Port + '*')) {
            Write-LauncherLog ('ignoring pid ' + $recorded + ': its command line is not a dsh service on port ' + $Port)
            return $false
        }
    } catch { }

    return $true
}

function Get-StateCandidates {
    # This port's own state file first, then the other ports' newest first (the
    # service may have been started on a changed default or an OS-picked port).
    $candidates = @()
    if (Test-Path -LiteralPath $stateFile) { $candidates += $stateFile }
    $others = Get-ChildItem -LiteralPath $logDir -Filter 'state-*.json' -ErrorAction SilentlyContinue |
        Sort-Object LastWriteTime -Descending | ForEach-Object { $_.FullName }
    foreach ($other in $others) { if ($candidates -notcontains $other) { $candidates += $other } }
    return $candidates
}

function Get-RememberedUrl {
    foreach ($candidate in (Get-StateCandidates)) {
        try {
            $state = Get-Content -LiteralPath $candidate -Raw | ConvertFrom-Json
            if ($state -and $state.url) { return [string]$state.url }
        } catch { }
    }
    return $null
}

function Resolve-ExistingUrl {
    if (Test-PortListening -Port $Port) {
        Write-LauncherLog ('port ' + $Port + ' already answers; reusing the service on it')
        return $url
    }
    if (Test-RecordedServiceAlive) {
        $candidate = Get-RememberedUrl
        if ([string]::IsNullOrWhiteSpace($candidate)) { $candidate = $url }
        Write-LauncherLog ('port probe could not reach it, but pid ' + (Get-RecordedPid) + ' is the service; reusing ' + $candidate)
        return $candidate
    }
    return $null
}

function Open-DshUrl {
    param([string]$Target)
    if ($NoBrowser) {
        Write-LauncherLog ('browser suppressed (-NoBrowser): ' + $Target)
        return
    }
    Write-LauncherLog ('opening browser: ' + $Target)
    try { Start-Process $Target | Out-Null } catch { Write-LauncherLog ('ERROR: could not open browser: ' + $_.Exception.Message) }
}

function Find-NodeExe {
    $cmd = Get-Command node.exe -ErrorAction SilentlyContinue
    if ($cmd -and $cmd.Source) { return [string]$cmd.Source }
    $candidates = @(
        (Join-Path $env:ProgramFiles 'nodejs\node.exe'),
        "$env:LOCALAPPDATA\Programs\nodejs\node.exe",
        'D:\Program Files\nodejs\node.exe'
    )
    if (${env:ProgramFiles(x86)}) { $candidates += (Join-Path ${env:ProgramFiles(x86)} 'nodejs\node.exe') }
    foreach ($candidate in $candidates) {
        if ($candidate -and (Test-Path -LiteralPath $candidate)) { return $candidate }
    }
    return $null
}

function Find-DshEntry {
    $npxRoot = Join-Path $env:LOCALAPPDATA 'npm-cache\_npx'
    if (-not (Test-Path -LiteralPath $npxRoot)) { return $null }
    $entry = Get-ChildItem -Path $npxRoot -Directory -ErrorAction SilentlyContinue |
        Sort-Object LastWriteTime -Descending |
        ForEach-Object { Join-Path $_.FullName 'node_modules\@deepseek-ai\dsh\lib\bin.js' } |
        Where-Object { Test-Path -LiteralPath $_ } |
        Select-Object -First 1
    return $entry
}

function Remove-PidRecord {
    try { if (Test-Path -LiteralPath $pidFile) { Remove-Item -LiteralPath $pidFile -Force } } catch { }
}

function Stop-RecordedService {
    # Used only to disown a start that lost the port race.
    $recorded = Get-RecordedPid
    if ($recorded -gt 0 -and $recorded -ne $PID) {
        try {
            (Get-Process -Id $recorded -ErrorAction Stop).Kill()
            Write-LauncherLog ('stopped process ' + $recorded)
        } catch { }
    }
    Remove-PidRecord
}

function Get-FallbackUrl {
    $remembered = Get-RememberedUrl
    if ([string]::IsNullOrWhiteSpace($remembered)) { return $url }
    return $remembered
}

# ---------------------------------------------------------------------------
Write-LauncherLog ('--- launcher invoked (workspace=' + $Workspace + ', port=' + $Port + ')')

# --- 1. reuse a running service, if any ------------------------------------
$existingUrl = Resolve-ExistingUrl
if ($existingUrl) {
    Write-LauncherLog ('service already up; reusing it: ' + $existingUrl)
    Open-DshUrl -Target $existingUrl
    Start-Sleep -Seconds 2
    Write-LauncherLog 'launcher exiting (existing service left running)'
    exit 0
}

# --- 2. start the service hidden, output redirected to the log -------------
$nodeExe = Find-NodeExe
if (-not $nodeExe) {
    Write-LauncherLog 'ERROR: node.exe not found'
    Open-DshUrl -Target (Get-FallbackUrl)
    exit 1
}

$dshEntry = Find-DshEntry
if ($dshEntry) {
    $exe = $nodeExe
    $argList = @($dshEntry, 'web', '--host', $AsciiHost, '--port', "$Port")
} else {
    $npmCmd = Join-Path (Split-Path -Parent $nodeExe) 'npm.cmd'
    if (-not (Test-Path -LiteralPath $npmCmd)) { $npmCmd = 'npm.cmd' }
    $exe = $npmCmd
    $argList = @('exec', '--yes', '--', '@deepseek-ai/dsh', 'web', '--host', $AsciiHost, '--port', "$Port")
    Write-LauncherLog 'dsh package missing from the npx cache; falling back to npm exec'
}

Clear-LogFile -Path $outFile
Clear-LogFile -Path $errFile
Write-LauncherLog ('starting: ' + $exe + ' ' + ($argList -join ' '))

$proc = $null
try {
    $proc = Start-Process -FilePath $exe -ArgumentList $argList `
        -WorkingDirectory $Workspace -WindowStyle Hidden -PassThru `
        -RedirectStandardOutput $outFile -RedirectStandardError $errFile
} catch {
    Write-LauncherLog ('ERROR: failed to start service: ' + $_.Exception.Message)
    Open-DshUrl -Target (Get-FallbackUrl)
    exit 1
}

Write-TextNoBom -Path $pidFile -Text ([string]$proc.Id)
Write-LauncherLog ('service pid ' + $proc.Id)

# --- 3. wait for readiness and read the tokenized URL from stdout ----------
$deadline = (Get-Date).AddSeconds($WaitSeconds)
$readyUrl = $null
$addressInUse = $false

while ((Get-Date) -lt $deadline) {
    Start-Sleep -Milliseconds 400

    $outText = Get-LogText -Path $outFile
    $errText = Get-LogText -Path $errFile

    $match = [regex]::Match($outText, 'dsh web:\s*(http://\S+)')
    if ($match.Success) {
        $readyUrl = $match.Groups[1].Value
        break
    }

    if ($errText -match 'EADDRINUSE' -and -not $addressInUse) {
        # Another process owns the port, so this start cannot succeed. That is
        # itself proof a service is up: stop waiting and hand over to it.
        $addressInUse = $true
        Write-LauncherLog ('port ' + $Port + ' is already owned by another process; this start cannot succeed')
        break
    }

    try { $proc.Refresh() } catch { }
    if ($proc.HasExited) {
        Write-LauncherLog ('service exited early with code ' + $proc.ExitCode)
        if (-not [string]::IsNullOrWhiteSpace($errText)) {
            Write-LauncherLog ('stderr tail: ' + ($errText.Trim() -split "`n" | Select-Object -Last 3 | Out-String).Trim())
        }
        break
    }
}

if ($addressInUse) {
    Stop-RecordedService
    $served = Get-FallbackUrl
    Write-LauncherLog ('reusing the service that already owns the port: ' + $served)
    Open-DshUrl -Target $served
    Start-Sleep -Seconds 2
    Write-LauncherLog 'launcher exiting (existing service left running)'
    exit 0
}

if (-not $readyUrl) {
    if (Test-PortListening -Port $Port) {
        $readyUrl = $url
        Write-LauncherLog 'service answers on the port but printed no URL; using the bare origin'
    } elseif (-not [string]::IsNullOrWhiteSpace((Get-RememberedUrl))) {
        $readyUrl = Get-RememberedUrl
        Write-LauncherLog 'reusing the remembered URL after a failed start'
    } else {
        Write-LauncherLog ('WARNING: service not ready after ' + $WaitSeconds + ' s')
    }
}

if ($readyUrl) {
    Write-TextNoBom -Path $stateFile -Text (@{ url = $readyUrl; pid = $proc.Id; startedAt = (Get-Date).ToString('s') } | ConvertTo-Json -Compress)
    Write-LauncherLog ('ready: ' + $readyUrl)
    Open-DshUrl -Target $readyUrl
} else {
    Write-LauncherLog 'no URL available; browser not opened'
}

# --- 4. stay alive as the service's parent; exit when it does --------------
# The pid record is deliberately kept: it is what lets a later run recognise a
# service this launcher left running, even where the port probe is refused.
try { Wait-Process -Id $proc.Id -Timeout -1 } catch { }
Write-LauncherLog 'service stopped; launcher exiting'
