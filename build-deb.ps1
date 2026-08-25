$ErrorActionPreference = 'Stop'
$stage = 'C:/Users/ae/Desktop/tieba/_pkg_deb'
if (Test-Path $stage) { Remove-Item -Recurse -Force $stage }
New-Item -ItemType Directory -Force -Path $stage | Out-Null
$env:PATH = 'C:\QtSDK\Madde\targets\harmattan_10.2011.34-1_rt1.2\bin;C:\QtSDK\Madde\bin;C:\QtSDK\Madde\madbin;' + $env:PATH
Set-Location 'C:\Users\ae\Desktop\tieba\build-check-harmattan'
& 'C:\QtSDK\Madde\bin\make.exe' install "INSTALL_ROOT=$stage/debian/tieba" 2>&1 | Out-Null
New-Item -ItemType Directory -Force -Path "$stage/debian" | Out-Null
Copy-Item 'C:\Users\ae\Desktop\tieba\qtc_packaging\debian_harmattan\control' "$stage/debian/control"
Copy-Item 'C:\Users\ae\Desktop\tieba\qtc_packaging\debian_harmattan\compat' "$stage/debian/compat"
New-Item -ItemType Directory -Force -Path "$stage/debian/tieba/DEBIAN" | Out-Null
Copy-Item 'C:\Users\ae\Desktop\tieba\qtc_packaging\debian_harmattan\changelog' "$stage/debian/tieba/DEBIAN/"
Copy-Item 'C:\Users\ae\Desktop\tieba\qtc_packaging\debian_harmattan\copyright' "$stage/debian/tieba/DEBIAN/"
$binControl = "$stage/debian/tieba/DEBIAN/control"
Get-Content 'C:\Users\ae\Desktop\tieba\qtc_packaging\debian_harmattan\control' | Where-Object { $_ -notmatch '^Architecture:' -and $_ -notmatch '^Standards-Version:' } | Set-Content $binControl
Add-Content $binControl 'Version: 0.0.2'
Add-Content $binControl 'Architecture: armel'
Copy-Item 'C:\Users\ae\Desktop\tieba\qtc_packaging\debian_harmattan\manifest.aegis' "$stage/debian/tieba.aegis"
# sanity checks: launcher must not use the declarative booster
Select-String -Path "$stage/debian/tieba/usr/share/applications/tieba_harmattan.desktop" -Pattern "Exec"
if ((Test-Path "$stage/debian/tieba/usr/share/applications/tieba_harmattan.desktop") -eq $false) { throw 'desktop file missing from staging!' }
$env:SYSROOT_DIR = 'C:\QtSDK\Madde\sysroots\harmattan_sysroot_10.2011.34-1_slim'
Set-Location $stage
bash -c "export SYSROOT_DIR=/c/QtSDK/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim && export PERL5LIB=/c/QtSDK/Madde/madlib/perl5 && dpkg-deb --build debian/tieba . 2>&1; echo exit=`$?"
Get-ChildItem "$stage/*.deb" | Select-Object Name,Length
