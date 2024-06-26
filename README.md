# pulsar

> my eecs 498 game engine

[wip] a fast, cross-platform distributable 2D game engine. 
with c++20, perf engineering, box2d, sdl, lua middleware and modular ECS.

---

### Quick Start

Having all [requirements](README.md#requirements) set, here you can find how to quickly build and run the application.

#### Table of contents

- [pulsar](#pulsar)
    - [Quick Start](#quick-start)
      - [Table of contents](#table-of-contents)
      - [Build](#build)
      - [Execute](#execute)
        - [macOS](#macos)
        - [Windows](#windows)
        - [Linux](#linux)
      - [Distribution](#distribution)
      - [Tests](#tests)
      - [Preview](#preview)
    - [credits](#credits)

#### Build

Usually available build modes are `Debug`, `Release`, and `RelWithDebInfo`.

To run a **debug** build:

```shell
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -B build/debug
cmake --build build/debug
```

To run a **release** build:

```shell
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -B build/release
cmake --build build/release
```

On macOS Xcode should be used as generator via `-GXcode`. For example creating a release build with XCode.

```shell
# Using Xcode
cmake -GXcode -DCMAKE_BUILD_TYPE=Release -B build/xcode
cmake --build build/xcode
```

#### Execute

When not running through an [IDE like CLion](https://www.jetbrains.com/clion), the built application can be run by
directly executing the generated binary.

##### macOS

To run a **debug** build:

```shell
./build/debug/src/app/App.app/Contents/MacOS/App
```

To run a **release** build:

```shell
./build/release/src/app/App.app/Contents/MacOS/App
```

To run a **debug** build created **with Xcode**:

```shell
./build/xcode/src/app/Debug/App.app/Contents/MacOS/App
```

To run a **release** build created **with Xcode**:

```shell
./build/xcode/src/app/Release/App.app/Contents/MacOS/App
```

##### Windows

To run a **debug** build:

```shell
build/debug/src/app/App.exe
```

To run a **release** build:

```shell
build/release/src/app/App.exe
```

##### Linux

To run a **debug** build:

```shell
./build/debug/src/app/App
```

To run a **release** build:

```shell
./build/release/src/app/App
```

#### Distribution

To bundle the application and create a distribution package CPack is used. Before executing CPack
a [release build needs to be generated](#build).

```shell
cpack --config build/release/CPackConfig.cmake
```

#### Tests

On any [generated build](#build) tests can be executed by using CTest, e.g. a Debug build:

```shell
ctest --test-dir build/debug
```

#### Preview

---

### credits
[Martin Fieber](https://martin-fieber.de/blog/basic-cpp-setup-with-dependency-management)'s C++20 project template with SDL2 and imgui. Lot of project installation, setup, distribution docs come from there.

Also, open source font [Manrope](https://manropefont.com).
