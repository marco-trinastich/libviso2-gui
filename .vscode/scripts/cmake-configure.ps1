param(
    [Parameter(Mandatory=$true)]
    [string]$SourceDir,

    # Target platform passed to the Visual Studio generator (-A).
    # Defaults to Win32 to preserve the existing behavior.
    [Parameter(Mandatory=$false)]
    [ValidateSet('Win32', 'x64', 'ARM64')]
    [string]$Platform = 'Win32'
)

# Detect host architecture from environment variables
$hostArch = 'unknown'

# Architecture detection
if (-not [string]::IsNullOrEmpty($env:PROCESSOR_ARCHITEW6432)) {
    # Running 32-bit process on 64-bit OS
    $hostArch = $env:PROCESSOR_ARCHITEW6432
}
else {
    $hostArch = $env:PROCESSOR_ARCHITECTURE
}

Write-Host "Detected host architecture: $hostArch"

# Find CMake using vswhere
$cmakePath = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.CMake.Project -find **\cmake.exe | Select-Object -First 1

if ($cmakePath) {
    Write-Host "Using CMake from: $cmakePath"
    Write-Host "Target platform: $Platform"

    # -A selects the TARGET platform (Win32/x64/ARM64); -T host=... keeps the
    # native host toolchain for fast compilation, independent of the target.
    $cmakeArgs = @(
        $SourceDir,
        "-A", $Platform,
        "-T", "host=$hostArch"
    )
    
    # Run CMake with the correct configuration
    & $cmakePath $cmakeArgs
    
    # Return the exit code from CMake
    exit $LASTEXITCODE
} else { 
    Write-Error "CMake not found"
    exit 1
}