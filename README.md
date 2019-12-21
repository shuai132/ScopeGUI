# ScopeGUI

[![Build Status](https://github.com/shuai132/ScopeGUI/workflows/Windows/badge.svg)](https://github.com/shuai132/ScopeGUI/actions?workflow=Windows)
[![Build Status](https://github.com/shuai132/ScopeGUI/workflows/macOS/badge.svg)](https://github.com/shuai132/ScopeGUI/actions?workflow=macOS)
[![Build Status](https://github.com/shuai132/ScopeGUI/workflows/Ubuntu/badge.svg)](https://github.com/shuai132/ScopeGUI/actions?workflow=Ubuntu)

Oscilloscope GUI for [ScopeMCU](https://github.com/shuai132/ScopeMCU)

Powered by [Dear ImGui](https://github.com/ocornut/imgui) and [GLFW](https://github.com/glfw)

[截图说明见wiki](https://github.com/shuai132/ScopeGUI/wiki)

[二进制releases](https://github.com/shuai132/ScopeGUI/releases)

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
