# konduit_installer

## dependencies

general:

- cmake
- internet connection to fetch raylib and miniz (or those libs available in the env)

compiler:

- linux:
    - gcc 15+
    - clang 19+
- mac-os:
    - clang 19+
- windows:
    - mingw64/gcc 15+
    - msvc *NOT SUPPORTED* (lacks support for C23 `#embed`)

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
    - mingw64 env (recommend using msys2)