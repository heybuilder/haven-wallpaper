# Third-party components

Haven's own source code is MIT licensed. That license does not replace the licenses of dynamically linked or separately downloaded components.

| Component | Version in this release | Terms / origin |
|---|---|---|
| Qt Core, GUI, Widgets, Network, SVG and Multimedia, related plugins/translations | 6.12.0 | LGPL v3 option for the included modules; bundled third-party components retain their licenses. https://www.qt.io/development/open-source-lgpl-obligations |
| FFmpeg shared libraries | 9.0.1 | The shipped avcodec library reports LGPL 2.1 or later. https://ffmpeg.org/legal.html |
| GCC libstdc++ / libgcc | MinGW GCC 13.1 | GPL v3 with the GCC Runtime Library Exception. https://www.gnu.org/licenses/gcc-exception-3.1.html |
| MinGW-w64 / winpthreads runtime | supplied with the matching Qt compiler kit | Permissive runtime notices; see included licenses. https://www.mingw-w64.org/ |

The Windows ZIP includes `licenses/`, SBOM/build information and a file manifest. Matching Qt/FFmpeg source archives are available in `Haven-v0.4.0-library-sources.zip` on the same release page, together with the configuration information supplied by the Qt SDK and reported by FFmpeg. These libraries have not been modified by Haven. Compatible dynamic libraries can be replaced; the application does not prohibit reverse engineering for debugging modifications to those libraries.

The release omits Qt's optional software-OpenGL fallback DLL and Microsoft's D3D compiler redistribution DLL; it relies on the Windows system graphics components instead.

## Optional YouTube helpers - not bundled

The setup script obtains pinned official releases directly from GitHub and checks SHA-256 before installation:

- yt-dlp 2026.08.19: https://github.com/yt-dlp/yt-dlp/releases/tag/2026.08.19 . Project source is Unlicense; the standalone Windows executable and its bundled dependencies have additional licenses documented upstream.
- Deno 2.9.7: https://github.com/denoland/deno/releases/tag/v2.9.7 . MIT and upstream third-party notices.

Consult each upstream project for its complete licenses and sources. Neither helper executable is redistributed in Haven's public ZIP or source repository.
