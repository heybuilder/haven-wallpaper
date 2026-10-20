# Validation history

# Haven v0.6.2 — playback, startup and desktop tools fixes

Download **Haven-Setup-v0.6.2.exe**, quit the old Haven through its tray menu, and install. Qt and C++ libraries are included.

- Video decoding runs in a separate process, keeping the control panel responsive if decoding hangs. Wallpaper attachment waits for a valid frame; playback failures remove the desktop surface.
- YouTube imports use H.264 MP4 for compatibility. Reimport old failing videos. Compatibility playback can be enabled for older GPUs; restart Haven after changing it.
- Optional sign-in startup and last-wallpaper restoration. Enable both options to restore wallpaper after restarting Windows.
- Saved task/goal tiles restore without starting a clock. Clock snapshots reopen paused.
- Smaller windows scroll vertically and horizontally to reach all controls.
- Adding tasks/goals or starting a clock reveals the corresponding desktop tile. Show desktop tools minimizes the control panel when tools are active.
- Save/load safeguards preserve unreadable task data and unavailable library entries.

Validation: automated clock/import/persistence checks; small-window and actual control-action checks; isolated playback, desktop attachment/removal and saved overlay restoration checks. User-reported checks passed for reboot restoration and playback on Windows 11 with a GT 710. Testing coverage remains limited.

A portable ZIP, Haven source and SHA-256 checksums accompany the installer. The Qt/FFmpeg dependencies are unchanged; matching sources remain available as [Haven-v0.5.0-library-sources.zip](https://github.com/heybuilder/haven-wallpaper/releases/download/v0.5.0/Haven-v0.5.0-library-sources.zip).

This is an unsigned early release. One active wallpaper/monitor is supported. Explorer attachment uses undocumented Windows behavior. There is no auto-updater or automatic fullscreen/battery pause. Personal tasks, settings and videos are not included in these downloads.
