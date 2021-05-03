# BlackHole

## Pre-built Binaries
<a id="raw-url" href="https://raw.githubusercontent.com/ryanmart00/BlackHole/master/builds/Win64/BlackHole.zip"> Windows 64-bit</a>

<a id="raw-url" href="https://raw.githubusercontent.com/ryanmart00/BlackHole/master/builds/Linux/BlackHole.zip"> Linux 64-bit</a>

<a id="raw-url" href="https://raw.githubusercontent.com/ryanmart00/BlackHole/master/builds/MacOS/BlackHole.zip"> Mac OSX 64-bit</a>

## Building from Source
Clone this repository in the terminal using
```git clone --recurse-submodules https://github.com/ryanmart00/BlackHole.git```
Then 
```cd BlackHole```
and 
```mkdir bin; cd bin```
Next we'll want to build the repo using cmake. If you don't have cmake install it using a normal package manager,
details per os are below.
Finally ```cmake .. ; make```
If everything went well you should have an executable called ```BlackHole```
in the bin directory which you can execute as you see fit.

## OS Specific Instructions

### Linux
Required libraries 
```
libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```
### MacOS
First we'll want to install cmake
```brew install cmake```
This should also install make and clang++ but if not you may install clang++ from llvm:
```brew install llvm```
To point your computer to llvm run 
```echo 'export PATH="/usr/local/opt/llvm/bin:$PATH"' >> ~/.bash_profile ; source ~/.bash_profile```


