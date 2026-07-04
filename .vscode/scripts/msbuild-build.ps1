param(
    [Parameter(Mandatory = $true)]
    [string]$SolutionPath
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

# Map architecture to MSBuild paths
switch ($hostArch) {
    'x86' { $findString = 'MSBuild\**\bin\MSBuild.exe' }
    'AMD64' { $findString = 'MSBuild\**\amd64\MSBuild.exe' }
    'ARM64' { $findString = 'MSBuild\**\arm64\MSBuild.exe' }
    default { $findString = "MSBuild\**\$hostArch\MSBuild.exe" } # Fallback
}

Write-Host "Detected host architecture: $hostArch"

# Try to find architecture-specific MSBuild first
$msbuildPath = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -find ${findString} | Select-Object -First 1

# If not found, fall back to default path
if (-not $msbuildPath) {
    Write-Host "Architecture-specific MSBuild not found, trying default location..."
    $msbuildPath = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -find "MSBuild\**\bin\MSBuild.exe" | Select-Object -First 1
}

if ($msbuildPath) {
    Write-Host "Using MSBuild from: $msbuildPath"
    
    # Run MSBuild with the correct host architecture preference
    & $msbuildPath $SolutionPath "/p:PreferredToolArchitecture=$hostArch"
    
    # Return the exit code from MSBuild
    exit $LASTEXITCODE
}
else { 
    Write-Error "MSBuild not found"
    exit 1
}