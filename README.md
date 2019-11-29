# ScopeGUI

[![Build Status](https://github.com/shuai132/ScopeGUI/workflows/build/badge.svg)](https://github.com/shuai132/ScopeGUI/actions?workflow=build)

Oscilloscope GUI for [ScopeMCU](https://github.com/shuai132/ScopeMCU)

Powered by [Dear ImGUI](https://github.com/ocornut/imgui) and [GLFW](https://github.com/glfw)

## Features

* 全平台支持: macOS、Linux、Windows，也可移植到Android/iOS。
* Fast: 60Fps
* FFT频谱分析

## Install Dependencies

* macOS
```bash
brew install glfw3
brew install sdl2
```

* Ubuntu
```bash
sudo apt-get update
sudo apt-get install -y libglfw3-dev libsdl2-dev
```

* Windows(TODO)  
MinGW

## Build

```bash
mkdir build && cd build && cmake .. && make ScopeGUI
```
