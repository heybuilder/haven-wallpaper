# Haven v0.4.0 - Windows early release

A calmer desktop with live video wallpapers and transparent productivity readouts.

Included:
- Local video wallpaper playback and preview, monitor selection, tray controls and normal-wallpaper restoration.
- Saved tasks/goals, timer, stopwatch and configurable Pomodoro.
- Active-only translucent corner overlays and soft sage/rose/lavender themes.
- YouTube import support through separately installed official helpers, with 1080p/1 GB limits and import diagnostics.

## Downloads
- **Haven-v0.4.0-windows-x64.zip**: extract all files, then open Start Haven.cmd.
- **Haven-v0.4.0-source.zip**: Haven source, tests, build instructions and notices.
- **Haven-v0.4.0-library-sources.zip**: matching Qt/FFmpeg source archives and build/SBOM information for the bundled dynamic libraries.
- **SHA256SUMS.txt**: checksums for release downloads.

No personal tasks, settings or video files are included. YouTube helper binaries are not redistributed; see tools/README.md inside the app download.

Tested: Release compilation, deterministic clock/import tests, overlay integration checks, YouTube import of the reported video, video decoding, desktop attachment and restoration. Broader testing on clean Windows PCs is still needed.

This is unsigned early software. Known limitations include undocumented Explorer attachment, one active wallpaper/monitor, no automatic fullscreen/battery pause, and no installer or auto-updater.
