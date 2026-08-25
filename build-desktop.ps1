$ErrorActionPreference = 'Stop'
$qmake = 'C:\QtSDK\Desktop\Qt\4.7.4\mingw\bin\qmake.exe'
$make = 'C:\QtSDK\mingw\bin\mingw32-make.exe'
$env:PATH = 'C:\QtSDK\mingw\bin;C:\QtSDK\Desktop\Qt\4.7.4\mingw\bin;' + $env:PATH
$buildDir = Join-Path $PSScriptRoot 'build-desktop'
if (Test-Path $buildDir) { Remove-Item -Recurse -Force $buildDir }
New-Item -ItemType Directory -Path $buildDir | Out-Null
Set-Location $buildDir
& $qmake (Join-Path $PSScriptRoot 'tieba.pro') -spec win32-g++ 2>&1
& $make -j4 2>&1 | Select-Object -Last 15
Write-Output "Build output: $buildDir\debug\tieba.exe"
