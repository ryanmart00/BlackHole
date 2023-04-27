# BlackHole

<p style="text-align: center;"><img src="/builds/Black_Hole_Clip.gif" width="400"></p>

This project aims to simulate the visual aspect of being near a Black Hole according to Einstein's theory of General Relativity. As an interactive experience the hope is that users will be able to better intuit the dynamics than if just shown pictures. In particular, the project uses the geodesics for light in the Schwartzschild (black hole) Geometry to render the scene instead of the usual straight lines we are used to. 

Currently, time has been eliminated from the simulation, but the plan is to incorporate time into the simulation by adding interactable clocks which will tick according to their experience of time. Further extensions would be to allow the black hole to ``spin,'' which is called the Kerr Geometry.

All of the theory for this can be found in ``Semi-Riemannian Geometry With Applications to Relativity'' By Barrett O'Neill.

The project uses GLFW, GLM, and Freetype 2, all used in accordance with their respective licencing agreements. Links to their websites are given below:

[GLFW](https://github.com/glfw/glfw)

[GLM](https://github.com/g-truc/glm)

[Freetype](https://www.freetype.org/index.html)


## Pre-built Binaries
<a id="raw-url" href="https://raw.githubusercontent.com/ryanmart00/BlackHole/master/builds/Win64/BlackHole.zip"> Windows 64-bit</a>

<a id="raw-url" href="https://raw.githubusercontent.com/ryanmart00/BlackHole/master/builds/Linux/BlackHole.zip"> Linux 64-bit</a>

<a id="raw-url" href="https://raw.githubusercontent.com/ryanmart00/BlackHole/master/builds/MacOS/BlackHole.zip"> Mac OSX 64-bit</a>

## Building from Source
Clone this repository in the terminal using
```
git clone --recurse-submodules https://github.com/ryanmart00/BlackHole.git
```
Then 
```
cd BlackHole
```
and 
```
mkdir bin; cd bin
```
Next we'll want to build the repo using cmake. If you don't have cmake install it using a normal package manager,
details per os are below.
Finally 
```
cmake .. ; make
```
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
```
brew install cmake
```
This should also install make and clang++ but if not you may install clang++ from llvm:
```
brew install llvm
```
To point your computer to llvm run 
```
echo 'export PATH="/usr/local/opt/llvm/bin:$PATH"' >> ~/.bash_profile ; source ~/.bash_profile
```


