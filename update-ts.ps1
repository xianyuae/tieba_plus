$ErrorActionPreference = 'Stop'
$lu = 'C:\QtSDK\Desktop\Qt\4.7.4\mingw\bin\lupdate.exe'
$files = Get-ChildItem 'C:\Users\xianyuae\Desktop\tieba\qml\tieba' -Recurse -File | Where-Object {$_.Extension -in '.qml','.js'} | ForEach-Object {$_.FullName}
foreach ($ts in @('tieba_zh_CN','tieba_zh_TW','tieba_ja','tieba_ko','tieba_en')) {
    $tsPath = "C:\Users\xianyuae\Desktop\tieba\i18n\$ts.ts"
    & $lu @files -codecfortr UTF-8 -no-obsolete -ts $tsPath 2>&1
}
