param(
    [Parameter(Mandatory=$false)]
    [string]$BuildDir = ".",
    
    [Parameter(Mandatory=$false)]
    [string]$Configuration = "Debug"
)

# Find CMake using vswhere
$cmakePath = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.CMake.Project -find **\cmake.exe | Select-Object -First 1

if ($cmakePath) {
    Write-Host "Using CMake from: $cmakePath"
    
    # Build the project
    & $cmakePath --build $BuildDir --config $Configuration --verbose
    
    exit $LASTEXITCODE
} else { 
    Write-Error "CMake not found"
    exit 1
}