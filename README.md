
# Volumetric Machine Firmware
## Project Description
컨베이어를 지나가는 물체의 체적을 계산하고 카메라를 찍어 관리하는 기계를 제어하는 펌웨어 프로젝트  
개인 서브 프로젝트로 라즈베리파이를 이용하여 제어

## 시작하기
## CMake install
---
### LTS version
debain/ubuntu 계통 기준
```console
sudo apt install cmake
```

### wget으로 최신버전 설치

공식 홈페이지: https://cmake.org/download

```console
wget https::/cmake.org/files/[version]/cmake-[version].tar.gz

tar -xvzf [download file].tar.gz
cd [download directory]
./bootstrap --prefix=/usr/local
make
make install
```

설치가 완료 된 후 재부팅.

## 3rd party libraries
- [wiringPi](http://wiringpi.com/)
- [libgphoto2](https://github.com/gphoto/libgphoto2)
- [jsoncpp](https://github.com/open-source-parsers/jsoncpp)

## Libraries build
---
## wiringPi
raspberry pi 2 ~ 3b
```console
sudo apt-get install wiringpi
```
raspberry pi 4 (wiringpi new Ver)  
https://github.com/wiringpi/wiringpi
```console
# 기존 구버전 wiringpi 제거
sudo apt purge wiringpi

# 새 버전 clone
git clone https://github.com/WiringPi/WiringPi.git

cd WiringPi
git pull origin
./build
```
GPIO 버전 확인
```shell
gpio -v
```

## libgphoto2
```shell
sudo apt-get install libgphoto2-6 libgphoto2-dev libgphoto2-port12
```

## jsoncpp
```shell
sudo apt-get install libjsoncpp-dev
```
### Compile
```shell
g++ -o [source ...] [source.cpp ...] -ljsoncpp
```

## Project build
---
```console
git clone https://ecremmoce@dev.azure.com/ecremmoce/Ecremmoce/_git/EcKiosk
cd EcKiosk

mkdir build

cd ./build
cmake ..
make

./EcKioskApp
```

# CMake README
## Project structure example
Example project structure is basically one master project with two static libraries.

Insight is that AppMain is some kind of project you are working on and deliver as a product.
This App product depends on two static libraries A and B which is stand alone libraries without external dependencies.

```
App
 ├── CMakeLists.txt
 ├── README.md
 ├── include
 │   └── AppSub.h
 ├── libs
 │   ├── A
 │   │   ├── CMakeLists.txt
 │   │   ├── include
 │   │   │   └── A
 │   │   │       └── A.h
 │   │   ├── src
 │   │   │   └── A.cpp
 │   │   └── test
 │   │       ├── ATests.cpp
 │   │       └── CMakeLists.txt
 │   ├── B
 │   │   ├── CMakeLists.txt
 │   │   ├── include
 │   │   │   └── B
 │   │   │       └── B.h
 │   │   ├── src
 │   │   │   └── B.cpp
 │   │   └── test
 │   │       ├── BTests.cpp
 │   │       └── CMakeLists.txt
 │   └── gtest-1.7.0
 ├── src
 │   └── AppMain.cpp
 │   └── AppSub.cpp
 └── test
```

## Running CMake
To run CMake you need to go into App directory and do these steps:

1. Create build directory `mkdir build`.
2. Go into created build directory `cd build`.
3. Run CMake to create XCode project `cmake -G Xcode ../`, but you can create any other project you want.
4. Open XCode project `open Foo.xcodeproj`.

**NOTE**: 
DO NOT make changes to generated project file, instead edit CMakeLists.txt files and regenerate project using CMake.
For example, when you want to add files to project you update CMakeLists.txt to include those files and regenerate build system using CMake.
