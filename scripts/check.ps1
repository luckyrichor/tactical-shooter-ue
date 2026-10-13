param([switch]$SkipNetwork)
$ErrorActionPreference='Stop'
$ProgressPreference='SilentlyContinue'
$root=Split-Path $PSScriptRoot -Parent
Set-Location $root
New-Item .local -ItemType Directory -Force | Out-Null
$engine='D:\UE\UE_5.6\Engine'
$p=Start-Process "$engine\Build\BatchFiles\Build.bat" -ArgumentList "CombatLabEditor Win64 Development $root\CombatLab.uproject -WaitMutex" -Wait -PassThru -RedirectStandardOutput .local\build-final.log -RedirectStandardError .local\build-final-error.log
if($p.ExitCode -ne 0) {throw 'Build failed'}
$exe="$engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$p=Start-Process $exe -ArgumentList "$root\CombatLab.uproject -unattended -nosplash -nosound -nullrhi -NoUdpMessaging -ExecCmds=`"Automation RunTests CombatLab`" -TestExit=`"Automation Test Queue Empty`" -ReportExportPath=$root\.local\automation-final -abslog=$root\.local\automation-final.log" -Wait -PassThru
$j=Get-Content .local\automation-final\index.json -Raw | ConvertFrom-Json
if($p.ExitCode -ne 0 -or $j.failed -ne 0 -or ($j.succeeded+$j.succeededWithWarnings) -ne 4) {throw 'Automation failed or missing tests'}
Write-Output "Automation: $($j.succeeded) clean successes, $($j.succeededWithWarnings) successes with warnings"
if($SkipNetwork){exit 0}
$tag=Get-Date -Format yyyyMMdd-HHmmss
$trace="$root\.local\server-$tag.utrace"
$common='-game -unattended -nosplash -nosound -nullrhi -NoUdpMessaging -stdout -FullStdOutLogOutput'
foreach($name in @('server.log','client-a.log','client-b.log','network-summary.json')){Remove-Item ("$root\.local\"+$name) -ErrorAction SilentlyContinue}
$server=Start-Process $exe -ArgumentList "$root\CombatLab.uproject /Engine/Maps/Entry?listen -port=7777 $common -LabNetworkProbe -LabExitSeconds=45 -trace=cpu,frame,bookmark,log -tracefile=$trace -ExecCmds=`"t.MaxFPS 60,csvprofile start`" -abslog=$root\.local\server.log" -PassThru -RedirectStandardOutput .local\server-stdout.log -RedirectStandardError .local\server-stderr.log
$null=$server.Handle
$clients=@()
try {
 $ready=$false
 for($n=0;$n -lt 90;$n++){Start-Sleep -Seconds 1;if($server.HasExited){throw 'Server died before listen'};if((Test-Path .local\server.log) -and (Select-String .local\server.log -Pattern 'GameNetDriver.*listening|IpNetDriver.*listening' -Quiet)){$ready=$true;break}}
 if(-not $ready){throw 'Server never listened'}
 foreach($name in @('a','b')) {$clientProcess=Start-Process $exe -ArgumentList "$root\CombatLab.uproject 127.0.0.1:7777 $common -LabAutoPlayer -LabExitSeconds=25 -ExecCmds=`"t.MaxFPS 60`" -abslog=$root\.local\client-$name.log" -PassThru -RedirectStandardOutput ".local\$name-stdout.log" -RedirectStandardError ".local\$name-stderr.log";$null=$clientProcess.Handle;$clients+=$clientProcess}
 foreach($p in $clients){$null=$p.Handle;if(-not $p.WaitForExit(90000)){throw 'Client timeout'};if($p.ExitCode -ne 0){throw "Client failed exit=$($p.ExitCode)"}}
 if(-not $server.WaitForExit(90000)){throw 'Server timeout'}
 if($server.ExitCode -ne 0){throw 'Server failed'}
 $serverText=[IO.File]::ReadAllText("$root\.local\server.log")
 $joins=([regex]::Matches($serverText,'Join succeeded:')).Count
 if($joins -ne 2 -or $serverText -notmatch 'LAB_STATE net=2' -or $serverText -notmatch 'LAB_DASH' -or $serverText -notmatch 'LAB_PULSE'){throw 'Missing real joins/server RPC skill execution'}
 function LastStates([string]$path){$map=@{};foreach($line in [IO.File]::ReadAllLines($path)){if($line -match 'LAB_STATE net=\d id=(\d+) actor=\S+ ammo=(\d+) health=(\d+) stamina=(\d+) ai=(\w+)'){$map[$Matches[1]]=@{ammo=[int]$Matches[2];health=[int]$Matches[3];stamina=[int]$Matches[4];ai=$Matches[5]}}};return $map}
 $serverStates=LastStates "$root\.local\server.log";$checked=0
 foreach($name in @('a','b')) {
  $text=[IO.File]::ReadAllText("$root\.local\client-$name.log")
  if($text -notmatch 'LAB_STATE net=3' -or $text -notmatch 'LAB_CUE') {throw 'Client is not networked or missed cues'}
  $states=LastStates "$root\.local\client-$name.log"
  foreach($id in $states.Keys){if($id -eq '0' -or -not $serverStates.ContainsKey($id)){continue};if(($states[$id]|ConvertTo-Json -Compress) -ne ($serverStates[$id]|ConvertTo-Json -Compress)){throw "Final state mismatch $name/$id"};$checked++}
 }
 if($checked -lt 4){throw 'Too few replicated final states'}
 $csv=Get-ChildItem "$root\Saved\Profiling\CSV" -Filter '*.csv' | Sort-Object LastWriteTime -Descending | Select-Object -First 1
 if(-not $csv -or $csv.Length -lt 100 -or $csv.LastWriteTime -lt $server.StartTime){throw 'Missing fresh CSV capture'}
 $traceFile=Get-Item $trace
 if($traceFile.Length -lt 100000){throw 'Missing trace capture'}
 $summary=@{date='2026-10-13';joins=$joins;server_exit=$server.ExitCode;client_exits=@($clients|ForEach-Object {$_.ExitCode});verified_final_actor_states=$checked;server_mode='listen';client_modes=3;skill_rpc=$true;client_cues=$true;trace_file=$traceFile.Name;trace_bytes=$traceFile.Length;csv_file=$csv.Name;csv_bytes=$csv.Length;scenario='loopback, null RHI, t.MaxFPS=60; network probe places enemy out of sight'}
 [IO.File]::WriteAllText("$root\.local\network-summary.json",($summary|ConvertTo-Json -Depth 5),(New-Object Text.UTF8Encoding($false)))
 Write-Output ($summary|ConvertTo-Json -Depth 5)
} finally {foreach($p in @($server)+$clients){if(-not $p.HasExited){$p.Kill()}}}
