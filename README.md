# Async HTTP Downloader

Async file downloader from http server

> Warning: This project only tested on Linux.

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

OR (without GNU/Make)

```bash
cd ./server
pip3 install -r ./requirements.txt
python3 ./main.py
```

## Known bugs

- Bad exception handling during `Action` execution in threads
- Can't execute program in other folder except project's root. This is because
7z's dll path is hardcoded which I'm passing to `bit7z` library (possibly just add
`ActionBuilder` and pass by cli interaface path to 7z's dll)
- Possible unsafe access to `Task` and their status in threads

