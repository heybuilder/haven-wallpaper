# Optional YouTube tools

Local wallpapers, tasks and timers work without these helpers. For YouTube imports, quit Haven and install the official yt-dlp and Deno executables here.

## Setup script

Review `Install-YouTube-Tools.ps1`, then run it from PowerShell using your normal script policy:

```powershell
& './tools/Install-YouTube-Tools.ps1'
```

If your policy does not allow local scripts, use the manual downloads below instead. The script downloads from official GitHub releases and checks pinned SHA-256 hashes before using the files. It does not ask for an administrator account or change execution/security policy.

## Manual setup

1. Download yt-dlp.exe from https://github.com/yt-dlp/yt-dlp/releases/tag/2026.08.19 and put it in this tools folder.
2. Download deno-x86_64-pc-windows-msvc.zip from https://github.com/denoland/deno/releases/tag/v2.9.7, extract deno.exe, and put it in this tools folder.
3. Restart Haven, paste a public video link, and click Import YouTube.

The files should be `tools/yt-dlp.exe` and `tools/deno.exe`, next to the sibling `app` folder. Standalone development builds use the sibling `preview` folder instead.

These executables are obtained from their publishers, not redistributed by Haven. Their own licenses and third-party notices apply. Helper updates may be necessary when YouTube changes.
