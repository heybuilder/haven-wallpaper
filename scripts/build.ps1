param(
    [Parameter(Mandatory=$true)][string]$QtRoot,
    [Parameter(Mandatory=$true)][string]$CompilerBin,
    [string]$CMake='cmake',
    [string]$NinjaBin='',
    [string]$BuildDirectory=''
)
$ErrorActionPreference='Stop'
$projectRoot=Split-Path -Parent $PSScriptRoot
if(-not $BuildDirectory){$BuildDirectory=Join-Path $projectRoot 'build'}
$CMake=(Get-Command $CMake -ErrorAction Stop).Source
$env:PATH="$CompilerBin;$QtRoot/bin;$NinjaBin;$env:PATH"
& $CMake -S $projectRoot -B $BuildDirectory -G Ninja "-DCMAKE_PREFIX_PATH=$QtRoot" -DCMAKE_BUILD_TYPE=Release
if($LASTEXITCODE -ne 0){throw 'CMake configuration failed'}
& $CMake --build $BuildDirectory
if($LASTEXITCODE -ne 0){throw 'Build failed'}
& (Join-Path (Split-Path $CMake) 'ctest.exe') --test-dir $BuildDirectory --output-on-failure
if($LASTEXITCODE -ne 0){throw 'Tests failed'}
Write-Output "Built $BuildDirectory/LiveWallpaper.exe"

