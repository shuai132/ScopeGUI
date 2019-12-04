# ScopeGUI

[![Build Status](https://github.com/shuai132/ScopeGUI/workflows/build/badge.svg)](https://github.com/shuai132/ScopeGUI/actions?workflow=build)

Oscilloscope GUI for [ScopeMCU](https://github.com/shuai132/ScopeMCU)

Powered by [Dear ImGui](https://github.com/ocornut/imgui) and [GLFW](https://github.com/glfw)

[截图和二进制文件](https://github.com/shuai132/ScopeGUI/wiki)

## Features

* 全平台支持: macOS、Linux、Windows，也可移植到Android/iOS。
* Fast: 60Fps
* FFT频谱分析
* 自动识别设备

## Dependencies

* C++11
* OpenGL

### Install Dependencies

* macOS  
Nothing todo

* Ubuntu
```bash
sudo apt-get update
sudo apt-get install -y libsdl2-dev
```

* Windows(MinGW)  
Nothing todo

## Build

```bash
mkdir build && cd build && cmake .. && make ScopeGUI
```
