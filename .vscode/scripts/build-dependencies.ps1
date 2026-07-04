<#
    build-dependencies.ps1

    Builds the third-party dependencies (zlib, libpng, libviso2) from source for a
    given target platform and deploys the resulting artifacts into
    libs/bin/<Platform> and libs/lib/<Platform>, with canonical names the root
    CMakeLists expects (zlib.dll/.lib, libpng14.dll/.lib, libviso2.dll/.lib).

    - png++ is header-only (nothing to build).
    - Win32 artifacts are committed, so by default this is a no-op for Win32
      (skip-if-present). Use -Force to rebuild regardless.
    - libpng depends on zlib (find_package(ZLIB)); zlib is built first and wired in.
#>
param(
    [Parameter(Mandatory = $false)]
    [ValidateSet('Win32', 'x64', 'ARM64')]
    [string]$Platform = 'Win32',

    [Parameter(Mandatory = $false)]
    [switch]$Force
)

$ErrorActionPreference = 'Stop'

# --- Paths -----------------------------------------------------------------
$repo   = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$incDir = Join-Path $repo 'libs\include'
$binOut = Join-Path $repo "libs\bin\$Platform"
$libOut = Join-Path $repo "libs\lib\$Platform"

$zlibSrc = Join-Path $incDir 'zlib'
$pngSrc  = Join-Path $incDir 'png'
$visoSrc = Join-Path $incDir 'libviso2'

# --- Host arch (for the native toolset host, independent of the target) ----
if (-not [string]::IsNullOrEmpty($env:PROCESSOR_ARCHITEW6432)) { $hostArch = $env:PROCESSOR_ARCHITEW6432 }
else { $hostArch = $env:PROCESSOR_ARCHITECTURE }

# --- Locate CMake via vswhere ----------------------------------------------
$cmake = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" `
    -latest -products * -requires Microsoft.VisualStudio.Component.VC.CMake.Project `
    -find **\cmake.exe | Select-Object -First 1
if (-not $cmake) { Write-Error 'CMake not found'; exit 1 }

# --- Skip-if-present --------------------------------------------------------
$expected = @(
    (Join-Path $binOut 'zlib.dll'),      (Join-Path $libOut 'zlib.lib'),
    (Join-Path $binOut 'libpng14.dll'),  (Join-Path $libOut 'libpng14.lib'),
    (Join-Path $binOut 'libviso2.dll'),  (Join-Path $libOut 'libviso2.lib')
)
if (-not $Force -and -not ($expected | Where-Object { -not (Test-Path $_) })) {
    Write-Host "Dependencies for '$Platform' already present -> skipping (use -Force to rebuild)."
    exit 0
}

New-Item -ItemType Directory -Force -Path $binOut, $libOut | Out-Null
Write-Host "Building dependencies for platform '$Platform' (host toolset: $hostArch)"

# --- Helpers ----------------------------------------------------------------
function Invoke-CMakeBuild([string]$srcDir, [string[]]$extraConfigureArgs) {
    $buildDir = Join-Path $srcDir "build\$Platform"
    if ($Force -and (Test-Path $buildDir)) { Remove-Item -Recurse -Force $buildDir }
    New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

    # NOTE: pipe CMake output to Out-Host so it is displayed but does NOT become
    # part of the function's return value (otherwise $buildDir would be polluted).
    $cfgArgs = @('-S', $srcDir, '-B', $buildDir, '-A', $Platform, '-T', "host=$hostArch") + $extraConfigureArgs
    & $cmake @cfgArgs | Out-Host
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed for '$srcDir'" }
    & $cmake --build $buildDir --config Release | Out-Host
    if ($LASTEXITCODE -ne 0) { throw "CMake build failed for '$srcDir'" }
    return $buildDir
}

# Copy the newest artifact matching one of $names from under $buildDir to $dest.
function Publish-Artifact([string]$buildDir, [string[]]$names, [string]$dest) {
    $found = $null
    foreach ($n in $names) {
        $found = Get-ChildItem $buildDir -Recurse -Filter $n -ErrorAction SilentlyContinue |
                 Sort-Object LastWriteTime -Descending | Select-Object -First 1
        if ($found) { break }
    }
    if (-not $found) { throw "Artifact not found under '$buildDir' (looked for: $($names -join ', '))" }
    Copy-Item $found.FullName $dest -Force
    Write-Host "  deployed $($found.Name) -> $($dest.Replace($repo + '\', ''))"
}

# ============================================================================
# 1) zlib  (no dependencies)
#    zlib's CMake renames the source-tree zconf.h to zconf.h.included on
#    out-of-source builds; back it up and restore it so the repo stays clean.
# ============================================================================
Write-Host "[1/3] zlib"
$zconf       = Join-Path $zlibSrc 'zconf.h'
$zconfBackup = Join-Path $env:TEMP "zconf.h.libviso2gui.bak"
$zconfSaved  = Test-Path $zconf
if ($zconfSaved) { Copy-Item $zconf $zconfBackup -Force }
try {
    $zlibBuild = Invoke-CMakeBuild $zlibSrc @()
}
finally {
    # Restore the pristine source zconf.h regardless of build outcome.
    if ($zconfSaved) { Copy-Item $zconfBackup $zconf -Force }
    $included = Join-Path $zlibSrc 'zconf.h.included'
    if (Test-Path $included) { Remove-Item $included -Force }
}
# On Win32 zlib emits zlib1.dll; on x64/ARM64 it is zlib.dll. Normalize to zlib.dll.
Publish-Artifact $zlibBuild @('zlib.dll', 'zlib1.dll') (Join-Path $binOut 'zlib.dll')
Publish-Artifact $zlibBuild @('zlib.lib')              (Join-Path $libOut 'zlib.lib')

# ============================================================================
# 2) libpng  (needs zlib: wire find_package(ZLIB) to what we just built)
# ============================================================================
Write-Host "[2/3] libpng"
$pngArgs = @(
    "-DZLIB_INCLUDE_DIR=$zlibSrc",
    "-DZLIB_LIBRARY=$(Join-Path $libOut 'zlib.lib')",
    '-DPNG_TESTS=OFF'
)
$pngBuild = Invoke-CMakeBuild $pngSrc $pngArgs
Publish-Artifact $pngBuild @('libpng14.dll') (Join-Path $binOut 'libpng14.dll')
Publish-Artifact $pngBuild @('libpng14.lib') (Join-Path $libOut 'libpng14.lib')

# ============================================================================
# 3) libviso2  (self-contained; SSE bridged to NEON via sse2neon on ARM64)
# ============================================================================
Write-Host "[3/3] libviso2"
$visoBuild = Invoke-CMakeBuild $visoSrc @()
Publish-Artifact $visoBuild @('libviso2.dll') (Join-Path $binOut 'libviso2.dll')
Publish-Artifact $visoBuild @('libviso2.lib') (Join-Path $libOut 'libviso2.lib')

Write-Host "Dependencies for '$Platform' built and deployed to libs/bin/$Platform and libs/lib/$Platform."
