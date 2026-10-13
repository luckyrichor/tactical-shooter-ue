param([switch]$ReuseCapture)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
Set-Location $root
if(-not $ReuseCapture){powershell -NoProfile -ExecutionPolicy Bypass -File "$PSScriptRoot\check.ps1";if($LASTEXITCODE -ne 0){throw 'Capture check failed'}}
$j=Get-Content .local\network-summary.json -Raw|ConvertFrom-Json
$trace="$root\.local\$($j.trace_file)"
$labArguments="-OpenTraceFile=$trace -AutoQuit -NoUI -ExecOnAnalysisCompleteCmd=`"TimingInsights.ExportTimingEvents $root\.local\lab-timing.csv -columns=TimerName,StartTime,EndTime,Duration -timers=Lab_*`" -ABSLOG=$root\.local\insights-export.log"
$p=Start-Process 'D:\UE\UE_5.6\Engine\Binaries\Win64\UnrealInsights.exe' -ArgumentList $labArguments -Wait -PassThru
if($p.ExitCode -ne 0){throw 'Unreal Insights failed'}
$rows=@(Import-Csv .local\lab-timing.csv)
$stats=@{}
foreach($name in @('Lab_AI','Lab_Fire','Lab_Dash','Lab_Pulse')){
 $values=@($rows|Where-Object {$_.TimerName -eq $name}|ForEach-Object {[double]::Parse($_.Duration,[Globalization.CultureInfo]::InvariantCulture)*1000000}|Sort-Object)
 if($values.Count -lt 1){throw "No events for $name"}
 $n=$values.Count
 $median=if($n%2 -eq 1){$values[[int][Math]::Floor($n/2)]}else{($values[$n/2-1]+$values[$n/2])/2}
 $stats[$name]=@{samples=$n;median_us=$median;max_us=$values[-1]}
}
$summary=@{engine='UE 5.6.1';insights_exit=$p.ExitCode;trace_sha256=(Get-FileHash $trace -Algorithm SHA256).Hash.ToLower();event_csv_sha256=(Get-FileHash .local\lab-timing.csv -Algorithm SHA256).Hash.ToLower();scopes=$stats;boundary='One local null RHI capture; inclusive CPU durations; small skill sample sizes; no GPU/FPS/optimization benefit claim'}
[IO.File]::WriteAllText("$root\.local\profile-summary.json",($summary|ConvertTo-Json -Depth 6),(New-Object Text.UTF8Encoding($false)))
Write-Output ($summary|ConvertTo-Json -Depth 6)