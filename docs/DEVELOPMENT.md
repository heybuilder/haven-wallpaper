# Haven - a desktop space that feels like home

C++17 / Qt 6 desktop application. This is a working development build, not yet a production release.

## Your day - a softer space to focus

Use **Your space** for wallpapers and **Your day** for productivity. Choose **Sage morning**, **Rose linen**, or **Lavender dusk** in the header; the palette is saved for your next launch.

- **Tasks and goals:** add intentions in separate tabs, check them off, and see your progress. Select an entry to remove it. Entries and completion states save locally as you work. Goals are a checklist rather than numeric targets.
- **Pomodoro:** adjustable focus, short rest, and long rest durations (defaults 25/5/15 minutes). Every fourth completed focus earns a long rest. Each phase waits for you to begin; skipping does not earn a completed session. Reset returns to a fresh focus while keeping the completed-session total.
- **Timer:** an independent countdown from 1 to 180 minutes, with start, pause, resume, and reset.
- **Stopwatch:** count upward independently, pause, resume, or reset.
- **Desktop tools:** running timers, stopwatches, and Pomodoro sessions appear automatically as small translucent readouts in a desktop corner. There is no title bar, toolbar, or full editor on the wallpaper. Pausing a clock hides its readout. In **Your day**, optionally enable **Tasks** or **Goals** to show up to four open entries each; click their checkboxes to complete them. Choose one of four corners or drag a readout within the current session. Use **Show active tools on wallpaper** to hide all readouts without stopping any clocks. Manage everything else in the main app.

The desktop tools and main panel share the same data and work with normal wallpapers too. Closing the main panel while a clock or overlay is active leaves the app in the system tray; **Quit** exits. Overlays remain independent of the main window when it is minimized. They use normal window stacking so other applications can cover them. Timing uses elapsed time, rather than counted UI ticks. Idle clocks do not run a polling timer.

Data saves on edits, every 30 seconds during timing, and on a normal exit. After restarting, clocks restore paused. Timers do not run or notify while the app is closed. Tasks and goals stay until you remove them; there is no automatic midnight deletion. Completed Pomodoro sessions are a running total.

Completion appears in the app and as a Windows notification, following your Windows notification settings. No alarm audio is played by the app.

Productivity data is in `intentions.json` under the application's local data folder, using atomic saves. An unreadable file is backed up before a new one is saved. Duplicate-instance protection prevents competing app copies. A forced shutdown may lose up to 30 seconds of running-clock progress.

## Run

Open `preview/LiveWallpaper.exe`. Keep the preview and tools folders together.

1. Click **+ Local video** to add one or more local files, or paste a YouTube URL and click **Import YouTube**.
2. Select an entry in the library.
3. Click **Preview** to watch it in the control panel.
4. Select your monitor and click **Set as wallpaper** to play behind desktop icons.
5. Use **Play / pause**, volume, or **Remove wallpaper** to control it.

Adding or importing a video never applies it automatically. A YouTube import downloads a local copy first; progress and failures appear in the status line. Cancel import stops the download. Imports are limited to 1 GB. The importer prefers an MP4 with audio and falls back to a video-only MP4 when needed. Restricted, unavailable, live, or unsupported videos can fail. Imported clips loop locally without a permanent network connection.

The library is saved between launches. Local files stay in their original location, so moving them requires adding them again. Removing a library entry does not delete its video. **Open downloaded videos folder** opens the cache so you can manage disk space, including partial downloads after cancellation.

YouTube imports select formats up to **1080p** and filter known sizes against the **1 GB** limit, so a long video does not automatically select a multi-gigabyte 4K version. MP4 is preferred, with WebM fallback. Short/Shorts/embed links are normalized to individual videos; tracking and playlist parameters are dropped. UTF-8 output preserves international video titles and filenames. **Import details** shows the full current-session transcript and process result. Cancel and exit clean up the downloader process tree.

Closing the control panel while a wallpaper is active leaves the wallpaper running. Double-click the tray icon to reopen the panel, or use its menu to pause, remove the wallpaper, or quit. Quitting restores the normal desktop. The app does not start with Windows or automatically apply a wallpaper at launch.

## Build and learn

Open `CMakeLists.txt` in Qt Creator and select a matching Qt desktop/compiler kit. Requires Qt 6.5+ Widgets, Multimedia and MultimediaWidgets, a C++17 compiler, and CMake 3.21+. Tested with Qt 6.12.0 and MinGW GCC 13.1 on Windows.

Read the files in this order:

- `src/main.cpp`: application startup, duplicate-instance lock, event loop.
- `src/PlayerWindow.h` and `.cpp`: control panel, saved library, playback, tray and asynchronous imports.
- `src/DesktopHost.h` and `.cpp`: Windows monitor lookup and desktop attachment.
- `src/ProductivityModel.h` and `.cpp`: saved intentions and clocks shared between views.
- `src/FocusClock.h`: deterministic elapsed-time clock engine.
- `src/ProductivityPanel.h` and `.cpp`: task/goal interface and transparent active-tool overlays.
- `src/CalmTheme.h` and `.cpp`: shared sage, rose, and lavender styling.
- `src/ImportSupport.h` and `.cpp`: YouTube link normalization, format limits, completed-file validation and errors.
- `src/ImportJob.h` and `.cpp`: Windows downloader process-tree lifetime management.
- `CMakeLists.txt`: executable sources and linked dependencies.

Desktop attachment relies on Explorer's undocumented WorkerW behavior. The module handles classic and nested WorkerW hosts, reports unsupported layouts, and checks for a lost desktop parent. After an Explorer restart or monitor removal, reapply the wallpaper. Only one monitor/wallpaper is active at a time. Automatic pause for games/fullscreen apps, installer, signed releases, automatic updates, richer thumbnails and broad Windows-version testing remain future work.

## Bundled tools

The tools folder contains official release executables downloaded from:

- yt-dlp: https://github.com/yt-dlp/yt-dlp (Unlicense; bundled dependencies have their own notices)
- Deno: https://github.com/denoland/deno (MIT)

Qt runtime files are deployed by Qt's windeployqt. For redistribution, audit and include all required Qt, FFmpeg, MinGW and tool dependency notices and license obligations. This folder is a local development build; distribution packaging has not been completed.

## Verification

- Release build completed with compiler warnings enabled.
- Public YouTube video imported successfully through the app's QProcess importer.
- Preview decoded frames and stayed in PlayingState.
- Wallpaper attached to the desktop host, decoded frames and stayed in PlayingState.
- Removal detached the wallpaper successfully.
- Control panel visually inspected using the computer-use skill.
- Deterministic tests cover delayed timer ticks, pause/resume, stopwatch reset, Pomodoro phases, long rests, skipped sessions, persistence, restart behavior, and themes.
- UI integration checks independent clocks, active-only overlays, saved data, frameless windows, transparent rendering, and hiding readouts without stopping clocks.

Run `ctest --test-dir YOUR_BUILD_FOLDER --output-on-failure` from the configured Qt build environment for deterministic tests. `--day-test ABSOLUTE_REPORT_PATH` runs the productivity UI integration check with isolated test data and exits automatically.

Automated integration checks are available as `--smoke-test LOCAL_VIDEO ABSOLUTE_REPORT_PATH` and `--import-test YOUTUBE_URL ABSOLUTE_REPORT_PATH`. The desktop smoke test briefly applies the test video and restores the desktop before exiting. Run desktop tests in an ordinary desktop session; the execution sandbox cannot access the real Explorer wallpaper host.

