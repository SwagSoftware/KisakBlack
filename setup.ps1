# setup.ps1 - point the built game at your Call of Duty: Black Ops install. Nothing of the install is copied (a few MB
# of DLLs aside) and the install is only ever read, never changed.
#
#   powershell -NoProfile -ExecutionPolicy Bypass -File setup.ps1 [-GameDir "D:\Games\Call of Duty Black Ops"] [-Config Release|Debug]
#
# Without -GameDir the folder is taken from gamedir.txt (written by an earlier run), then looked up in Steam
# (registry + library folders) and the usual install paths.
#
# Layout (build\<Config> is where the exe runs, and fs_b):
#   zone\        junction to <install>\zone       the exe loads zone\ relative to its own folder (db_registry.cpp)
#   main\video\  junction to <install>\main\video the movies load from <working folder>\main\video (r_cinematic.cpp)
#   main\*.iwd   not linked: the game reads them from the install with +set fs_cdpath <install>
#                (Play.bat passes it; the Visual Studio debugger gets it from CMakeLists.txt; both read gamedir.txt)
#   *.dll, localization.txt, codlogo.bmp   a few MB, copied
# Junctions need no admin rights and work across NTFS drives. To undo: delete the two junctions with `rmdir <link>`
# (never `Remove-Item -Recurse`, which can follow a link into the install) and delete gamedir.txt.
param(
    [string]$GameDir = '',
    [ValidateSet('Release', 'Debug')][string]$Config = 'Release'
)
$ErrorActionPreference = 'Stop'
$cfg = Join-Path $PSScriptRoot 'gamedir.txt'

function Test-BlackOpsDir([string]$dir) {
    $dir -and (Test-Path (Join-Path $dir 'zone\Common')) -and (Test-Path (Join-Path $dir 'main\iw_00.iwd'))
}

function Find-SteamLibraries {
    $steam = (Get-ItemProperty 'HKCU:\Software\Valve\Steam' -ErrorAction SilentlyContinue).SteamPath
    if (-not $steam) { $steam = (Get-ItemProperty 'HKLM:\SOFTWARE\WOW6432Node\Valve\Steam' -ErrorAction SilentlyContinue).InstallPath }
    if (-not $steam) { return }
    $steam = $steam -replace '/', '\'
    $steam
    $vdf = Join-Path $steam 'steamapps\libraryfolders.vdf'
    if (Test-Path $vdf) {
        foreach ($m in [regex]::Matches((Get-Content $vdf -Raw), '"path"\s+"([^"]+)"')) { $m.Groups[1].Value -replace '\\\\', '\' }
    }
}

if (-not $GameDir -and (Test-Path $cfg)) { $GameDir = (Get-Content $cfg -TotalCount 1).Trim() }
if (-not $GameDir) {
    $candidates = @(Find-SteamLibraries | ForEach-Object { Join-Path $_ 'steamapps\common\Call of Duty Black Ops' }) +
        'C:\Program Files (x86)\Steam\steamapps\common\Call of Duty Black Ops'
    $GameDir = $candidates | Where-Object { Test-BlackOpsDir $_ } | Select-Object -First 1
}
if (-not (Test-BlackOpsDir $GameDir)) {
    throw "No Black Ops install found$(if ($GameDir) { " at '$GameDir'" }). Pass -GameDir with the folder that holds BlackOps.exe, zone\ and main\ (main\iw_00.iwd must exist)."
}
$GameDir = (Resolve-Path $GameDir).Path.TrimEnd('\')
"game install: $GameDir"
Set-Content -Path $cfg -Value $GameDir -Encoding ASCII

$dst = Join-Path $PSScriptRoot "build\$Config"
New-Item -ItemType Directory -Force (Join-Path $dst 'main') | Out-Null

# $link -> $target. An existing junction to somewhere else is repointed; a real folder or file is never touched.
function Set-Junction([string]$link, [string]$target) {
    $item = Get-Item $link -Force -ErrorAction SilentlyContinue
    if ($item) {
        if ($item.LinkType -ne 'Junction') {
            throw "'$link' exists and is not a junction (a manual copy of the install?). Delete it yourself, then run setup.ps1 again."
        }
        if ((@($item.Target)[0]).TrimEnd('\') -ieq $target.TrimEnd('\')) { "ok       $link"; return }
        [IO.Directory]::Delete($link)   # removes the link only, never the target's contents
    }
    New-Item -ItemType Junction -Path $link -Target $target | Out-Null
    "junction $link -> $target"
}
Set-Junction (Join-Path $dst 'zone') (Join-Path $GameDir 'zone')
if (Test-Path (Join-Path $GameDir 'main\video')) { Set-Junction (Join-Path $dst 'main\video') (Join-Path $GameDir 'main\video') }
else { 'note: no main\video in the install, so no movies' }

# the engine's own DLLs (the exe is built against this Steam SDK, Bink and libvpx)
Copy-Item (Join-Path $PSScriptRoot 'src\binklib\binkw32.dll') $dst -Force
Copy-Item (Join-Path $PSScriptRoot 'src\libs\libvpx-1.5.0\vpx.dll') $dst -Force
Copy-Item (Join-Path $PSScriptRoot 'src\steam\redistributable_bin\*.dll') $dst -Force
# small loose files the exe reads from its working folder (localization.txt is required: the game asserts without it)
foreach ($f in 'localization.txt', 'codlogo.bmp', 'cod.bmp') {
    $src = Join-Path $GameDir $f
    if (Test-Path $src) { Copy-Item $src $dst -Force } elseif ($f -eq 'localization.txt') { throw "$src is missing" }
}
# DirectX 9 runtime (d3dx9_43, xinput1_3) out of the game's own Redist\DirectX, so nothing has to be installed.
# (The exe is linked /MT, so it needs no Visual C++ runtime. The Debug build's d3dx9d_43.dll only ships with the DirectX SDK.)
$dx = Join-Path $GameDir 'Redist\DirectX'
if (Test-Path $dx) {
    expand.exe (Join-Path $dx 'Jun2010_d3dx9_43_x86.cab') "-F:d3dx9_43.dll" $dst | Out-Null
    expand.exe (Join-Path $dx 'APR2007_xinput_x86.cab') "-F:xinput1_3.dll" $dst | Out-Null
    foreach ($f in 'd3dx9_43.dll', 'xinput1_3.dll') { if (-not (Test-Path (Join-Path $dst $f))) { throw "could not extract $f from $dx" } }
} else { "note: no Redist\DirectX in the install; d3dx9_43.dll and xinput1_3.dll must come from an installed DirectX runtime" }
if ($Config -eq 'Debug') {
    $sdk = 'C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Developer Runtime\x86\d3dx9d_43.dll'
    if (Test-Path $sdk) { Copy-Item $sdk $dst -Force } else { 'note: d3dx9d_43.dll (DirectX SDK June 2010, Developer Runtime) not found; the Debug build needs it' }
}
"ready in $dst (game files are read from $GameDir; start with Play.bat)"
