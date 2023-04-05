# Async HTTP Downloader

Async file downloader from http server

> This project only tested on Linux.

## Requirements

- CMake
- GNU/Make (optional; it's needed by server for enviroment setup and running)
- Clone this repo with `--recursive` flag

## Thirdparty

- `HTTPRequest` - single-header library
- `bit7z` - library for handling different archive formats (7z, zip and etc.)
- `yaml-cpp` - for handling YAML config file

## How to build

```bash
mkdir build
cmake -B build
cmake --build build
```

To run executable:

```bash
./build/async-http-downloader <path-to-config>
```

## How to run http-server

```bash
cd ./server
make
```

