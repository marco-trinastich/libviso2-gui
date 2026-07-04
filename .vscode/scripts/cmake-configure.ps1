param(
    [Parameter(Mandatory=$true)]
    [string]$SourceDir
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
    
    # Additional CMake arguments to specify the host architecture
    $cmakeArgs = @(
        $SourceDir,
        "-A", "Win32",
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