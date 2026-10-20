# Privacy

Haven v0.5.0 stores task and goal entries, completion states, clock snapshots, preferences, and imported video files locally. These are not bundled with the app when it is shared.

Haven has no account system, telemetry, or analytics. It does not upload your task and goal data. Local file selections refer to files on your own computer.

Optional YouTube import invokes yt-dlp with the video URL. That program contacts YouTube and its media servers. Its official Deno runtime can execute YouTube challenge-solving scripts as part of extraction. Public downloads omit both helper executables; the in-app Enable YouTube imports button (or optional setup script) downloads them from the official GitHub release URLs and verifies pinned SHA-256 checksums.

Import details show the current session's downloader output, which can contain a video URL, video title and local file path. Review it before sharing it in a public issue.

Data is stored under the application's Windows local-data directory, normally `%LOCALAPPDATA%\LiveWallpaperLearning\Live Wallpaper`, and preferences use the current user's Qt settings. Exiting the app stops timing. Deleting a library entry does not delete its underlying video.
