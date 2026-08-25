$base = 'C:\Users\xianyuae\Desktop\tieba\_pkg_deb_20260815'
if (Test-Path "$base\debian\tieba") { Remove-Item -Recurse -Force "$base\debian\tieba" }
if (Test-Path "$base\tieba") { Move-Item "$base\tieba" "$base\debian\tieba" }
if (Test-Path "$base\tieba.aegis") { Copy-Item "$base\tieba.aegis" "$base\debian\tieba.aegis" }
