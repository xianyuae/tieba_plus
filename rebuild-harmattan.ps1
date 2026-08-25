$ErrorActionPreference = 'Continue'
$env:PATH = 'C:\QtSDK\Madde\targets\harmattan_10.2011.34-1_rt1.2\bin;C:\QtSDK\Madde\bin;C:\QtSDK\Madde\madbin;' + $env:PATH
$buildDir = 'C:\Users\ae\Desktop\tieba\build-check-harmattan'
if (Test-Path $buildDir) { Remove-Item -Recurse -Force $buildDir }
New-Item -ItemType Directory -Path $buildDir | Out-Null
Set-Location $buildDir
& 'C:\QtSDK\Madde\targets\harmattan_10.2011.34-1_rt1.2\bin\qmake.exe' 'C:\Users\ae\Desktop\tieba\tieba.pro' -o Makefile
Write-Output "qmake exit: $LASTEXITCODE"
& 'C:\QtSDK\Madde\bin\make.exe' -j2 2>&1 | Select-Object -Last 40
Write-Output "make exit: $LASTEXITCODE"
