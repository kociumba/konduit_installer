# konduit_installer

## dependencies

general:

- cmake (with ninja or make)

compiler:

- linux:
    - gcc 15+
    - clang 19+
- mac-os:
    - clang 19+
- windows:
    - mingw64/gcc 15+
    - msvc compiles but produces broken binaries

libs:

- linux:
    - libx11-dev
    - libxrandr-dev
    - libxinerama-dev
    - libxcursor-dev
    - libxi-dev
- mac-os:
    - xcode env (recommended but might work with system default)
- windows:
    - mingw64 env (if using gcc)
    - msvc