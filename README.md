![app](https://github.com/EgorOrachyov/X11HelloWorld/blob/main/docs/pictures/application.gif?raw=true)

# X11 Hello World!

[![Ubuntu](https://github.com/EgorOrachyov/X11HelloWorld/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/EgorOrachyov/X11HelloWorld/actions/workflows/ubuntu.yml)

**X11 windowing system and GLX** based C++ application with basic colored
triangle following the mouse position, which is rendered with modern OpenGL graphics.

### Technologies

- X11 library for native OS windowing and input handling
- GLX for OS specific OpenGL context setup
- OpenGL for graphics
- C++11 language
- CMake for build configuration

### Dependencies

- [glm](https://github.com/g-truc/glm) for 3d vector math and transformations
- [glew](https://github.com/Perlmint/glew-cmake) for OpenGL functions and extensions loading

> These dependencies are stored as project submodules.
> No extra installation step is required.

### Platforms

- Linux-based OS with X11 and GLX support

## Get started

### Requirements

- Linux-based OS (tested on Ubuntu 20.04)
- GCC compiler for C++11 (tested on 8.4.0)
- CMake 3.11 or greater (tested on 3.16.3)

### Configuration

Library uses `X11/Xlib.h` and `X11/Xutil.h` header files and `X11` cmake package dependency.
So, the complete set of x11 tools can be installed as follows (may be too excessive):

```shell script
$ sudo apt-get update -y
$ sudo apt-get install -y build-essential
$ sudo apt-get install -y libgl1-mesa-dri libgl1-mesa-glx libgl1-mesa-dev
$ sudo apt-get install -y libxmu-dev libxi-dev libgl-dev libglx-dev
$ sudo apt-get install -y libx11-dev
$ sudo apt-get install -y xorg-dev
```

### Get the source code

```shell script
$ git clone https://github.com/EgorOrachyov/X11HelloWorld.git
$ cd X11HelloWorld
$ git submodule update --init --recursive
```

### Configure and build

```shell script
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
```

### Run application

```shell script
./x11helloworld
```

## License

This project is licensed under MIT license. The license text can be found at 
[link](https://github.com/EgorOrachyov/X11HelloWorld/blob/main/LICENSE.md).

## Also

If you have any questions, feel free to contact me at `egororachyov@gmail.com`.
You can also create an issue or a pull request if you want!
