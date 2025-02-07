# sim-finance-cpp

## Goals

1. template
2. conan + cmake

## Set up conan (ubuntu)

```shell
sudo apt install python3-pip -y

# ubuntu
sudo apt-get install pipx -y
pipx ensurepath
# Success! Added /home/xcheng85/.local/bin to the PATH environment variable.
source .bashrc
pipx install conan

# conan profile

conan profile detect --force

detect_api: Found cc=gcc- 13.3.0
detect_api: gcc>=5, using the major as version
detect_api: gcc C++ standard library: libstdc++11

# Detected profile:
# [settings]
# arch=x86_64
# build_type=Release
# compiler=gcc
# compiler.cppstd=gnu17
# compiler.libcxx=libstdc++11
# compiler.version=13
# os=Linux

# WARN: This profile is a guess of your environment, please check it.
# WARN: The output of this command is not guaranteed to be stable and can change in future Conan versions.
# WARN: Use your own profile files for stability.
# Saving detected profile to /home/xcheng85/.conan2/profiles/default

# edit cpp standard
compiler.cppstd=gnu17
build_type=Release

# check default conan profile
conan config home

```

## bootstrap cmake project with conan

```shell
# root dir
mkdir -p expression-templates
cd expression-templates && touch CMakeLists.txt conanfile.txt

# directory: build
# -s = settings
conan install . -s build_type=Debug --output-folder=build --build=missing --profile=default

# use eigen as shared library


--options=zlib/1.2.11:shared=True



# cmake build

cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=./build/Debug/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
./Debug/

```

## bootstrap p2p

### build
```shell

# root dir
mkdir -p p2p
cd p2p && touch CMakeLists.txt conanfile.txt

# directory: build
# -s = settings
conan install . -s build_type=Debug --output-folder=build --build=missing --profile=default

cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=./build/Debug/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build .
./Debug/


```

### Run Locally

```shell
# websocket golang server

```