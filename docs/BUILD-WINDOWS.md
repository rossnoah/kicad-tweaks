# Building on Windows

Two routes. MSYS2 is the quick one for a development build; MSVC + vcpkg is what upstream
KiCad ships with and is the right choice for an installer.

## MSYS2 (about two hours, no Visual Studio needed)

1. Install MSYS2 from https://www.msys2.org and open the **MSYS2 MINGW64** shell.
2. Install the toolchain and dependencies (same set as the MSYS2 `kicad` package):

   ```
   pacman -S --needed git \
     mingw-w64-x86_64-cc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja \
     mingw-w64-x86_64-pkgconf mingw-w64-x86_64-boost mingw-w64-x86_64-doxygen \
     mingw-w64-x86_64-swig mingw-w64-x86_64-wxwidgets3.2-msw \
     mingw-w64-x86_64-abseil-cpp mingw-w64-x86_64-cairo mingw-w64-x86_64-curl \
     mingw-w64-x86_64-freeglut mingw-w64-x86_64-glew mingw-w64-x86_64-glm \
     mingw-w64-x86_64-nng mingw-w64-x86_64-protobuf mingw-w64-x86_64-libgit2 \
     mingw-w64-x86_64-ngspice mingw-w64-x86_64-python mingw-w64-x86_64-opencascade \
     mingw-w64-x86_64-openssl mingw-w64-x86_64-zlib mingw-w64-x86_64-zstd \
     mingw-w64-x86_64-harfbuzz mingw-w64-x86_64-sqlite3 mingw-w64-x86_64-icu \
     mingw-w64-x86_64-gettext mingw-w64-x86_64-unixodbc
   ```

   Add `mingw-w64-x86_64-wxPython` and drop `-DKICAD_SCRIPTING_WXPYTHON=OFF` below if you
   want the Python console.

3. Clone and build. Clone into the MSYS2 home, not into `/mnt/c` or a WSL path, or the
   build will crawl.

   ```
   git clone -b worktree-tile-snap https://github.com/rossnoah/kicad-tweaks.git
   cd kicad-tweaks
   cmake -S . -B build -G Ninja \
     -DCMAKE_BUILD_TYPE=Release \
     -DKICAD_SCRIPTING_WXPYTHON=OFF \
     -DKICAD_BUILD_I18N=OFF \
     -DKICAD_BUILD_QA_TESTS=OFF \
     -DKICAD_USE_SENTRY=OFF \
     -DwxWidgets_CONFIG_EXECUTABLE="${MINGW_PREFIX}/bin/wx-config-3.2" \
     -DOCC_INCLUDE_DIR="${MINGW_PREFIX}/include/opencascade"
   cmake --build build --parallel
   ```

4. Run it from the MINGW64 shell (the MinGW DLLs are on that shell's PATH):

   ```
   ./build/pcbnew/pcbnew.exe
   ```

   To run outside the shell, `cmake --install build --prefix install` and copy the DLLs the
   executables need next to them (the GitHub workflow below does this with `ldd`).

## GitHub Actions

`.github/workflows/windows-msys2.yml` does the MSYS2 build on a hosted runner and uploads
`install/` as an artifact with the runtime DLLs bundled. It is manual: Actions tab >
"Windows build (MSYS2)" > Run workflow, pick the branch. Expect two to three hours.

## MSVC + vcpkg (upstream's route)

Install Visual Studio 2022 with the "Desktop development with C++" workload, clone
https://github.com/microsoft/vcpkg and run its `bootstrap-vcpkg.bat`, set `VCPKG_ROOT` to it,
copy `CMakePresets.json.sample` to `CMakePresets.json` and edit the `VCPKG_ROOT` value inside,
then in a Developer PowerShell:

```
cmake --preset msvc-win64-release
cmake --build --preset win64-release
```

The first configure builds every dependency (wxWidgets, OpenCASCADE, Boost, ...) from source
through vcpkg, which takes several hours; upstream CI avoids this with a binary cache that is
not available outside their GitLab. After that, incremental builds are quick.
