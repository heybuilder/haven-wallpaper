# Haven

A calm Windows desktop companion: video wallpapers, small intentions, and transparent focus tools.

**Early release v0.5.0 - Windows x64**

## Download and run

1. Download `Haven-Setup-v0.5.0.exe` from [Releases](https://github.com/heybuilder/haven-wallpaper/releases).
2. Run the installer, then open Haven from the Start menu.
3. Select a local video and choose **Set as wallpaper**.

A portable ZIP is also available: extract it completely and open **Start Haven.cmd**.

Qt and the C++ runtime are included. No Qt installation or administrator account is required. The app is unsigned. Windows may display its usual reputation notice; verify the release source and checksum before deciding whether to run it.

## A space that feels like home

- Loop a local video behind desktop icons; preview before applying.
- Choose a monitor, pause playback, adjust volume, and restore your normal wallpaper.
- Manage tasks and goals with locally saved completion states.
- Use an independent timer, stopwatch, or customizable Pomodoro.
- Show only active readouts as small translucent desktop overlays, with optional task/goal lists and four corner positions.
- Choose sage, rose, or lavender colors for the control panel.
- Keep the app in the tray while wallpaper or timers run.

Nothing applies automatically at startup. Timers reopen paused. Overlays are ordinary desktop windows and can be covered by other apps; they are not part of the image or video file.

## Optional YouTube import

Click **Enable YouTube imports** inside Haven once. The app downloads the optional helpers from their official GitHub releases and verifies their pinned SHA-256 checksums. Then paste a public YouTube video link and click **Import YouTube**. Internet access is required for setup and imports. Imports are cached locally, prefer MP4, and are limited to 1080p / 1 GB. Restricted or unavailable videos can fail. Only import content you are permitted to use.

## Privacy

Tasks, goals, preferences, and imported clips are saved on your computer. Haven does not send task or goal data to a server and has no analytics or account requirement. YouTube imports contact YouTube through the optional downloader; helper installation contacts GitHub. Your settings and videos are not included in release downloads. See [Privacy](PRIVACY.md).

## Build

Requires Windows, C++17, CMake 3.21+, and Qt 6.5+ Widgets / Network / Multimedia / MultimediaWidgets. Use a compiler that matches your Qt kit.

```powershell
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="C:/Qt/6.12.0/mingw_64" -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Run from the kit's build environment, including its compiler, Ninja and Qt DLL paths. See [Development guide](docs/DEVELOPMENT.md) and [Build script](scripts/build.ps1).

## Status and limitations

This is an early development release tested on the development machine, not a broadly certified Windows product. Explorer desktop attachment uses undocumented Windows behavior and may need reapplying after Explorer or display changes. One wallpaper/monitor is active at a time. There is no signing, auto-update, start-with-Windows feature, or automatic fullscreen/battery pause yet.

## License

Haven's own code is [MIT](LICENSE). Qt, FFmpeg and compiler runtimes retain their own terms. The portable download includes dependency notices and a companion source archive is supplied with the release. See [Third-party components](THIRD_PARTY.md). You may replace the dynamically linked libraries with compatible builds. No restriction is imposed on reverse engineering for debugging changes to those libraries.
