$ErrorActionPreference='Stop'
if(Get-Process -Name Haven,LiveWallpaper -ErrorAction SilentlyContinue){throw 'Quit Haven before installing the optional helpers.'}
$downloads=@(
    @{Url='https://github.com/yt-dlp/yt-dlp/releases/download/2026.08.19/yt-dlp.exe';Name='yt-dlp.exe';Hash='66674953FE251B89F4D08C5F0E35E0728679BD67AB3D7D05C0562AF101DD3E7A'},
    @{Url='https://github.com/denoland/deno/releases/download/v2.9.7/deno-x86_64-pc-windows-msvc.zip';Name='deno-runtime.zip';Hash='A0C3101B4158D1DFB7D6A78A7BF0F3DE80C96BB423C152BEEC8BEB22786F2238'}
)
foreach($download in $downloads){
    $temporary=Join-Path $PSScriptRoot ($download.Name+'.download')
    Invoke-WebRequest -UseBasicParsing -Uri $download.Url -OutFile $temporary -TimeoutSec 120
    if((Get-FileHash -LiteralPath $temporary -Algorithm SHA256).Hash -ne $download.Hash){throw "Checksum did not match for $($download.Name). The file was not installed."}
    Move-Item -LiteralPath $temporary -Destination (Join-Path $PSScriptRoot $download.Name) -Force
}
$archive=Join-Path $PSScriptRoot 'deno-runtime.zip'
Expand-Archive -LiteralPath $archive -DestinationPath $PSScriptRoot -Force
Remove-Item -LiteralPath $archive
Write-Output 'YouTube helpers installed. Open Haven again.'
