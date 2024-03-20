# callidaria's yomisensei

## RUN

<br>

### linux
```
./yomisensei
```
or press [r] key in cascabel console

<br>

### windows
double click yomisensei.exe \
or press [r] key in cascabel console

### DISCLAIMER
*building is not required to run the game!*

<br>

***

<br>

## BUILD

<br>

### linux
```
# to open cascabel console
./ccb_console
```
select BUILD ALL COMPONENTS with arrow keys and run with [SPACE] to build libraries \
then, press [b] key to build executable

<br>

### windows
run ccb_console.exe \
select BUILD ALL COMPONENTS with [w] & [s] and run with [SPACE] to build libraries \
then, press [b] key to build executable

<br>

### DISCLAIMER
*certain libraries are required to satisfy the compiler:*
- SDL2
- SDL2_net
- GLEW
- GLM
- SOIL
- OpenAL

<br>

To see the full documentation of engine go [here](doc/index.md)

<br>

***

<br>

## v0.0.7 DEV

### optimizations:
- bitwise operations replacing some division and modulo operators
- removed unnecessary calculations from montecarlo shader

## v0.0.6 [UNX]

### features:
- render to multiple subscreens within the main window
- dynamic player readable identification of requested buttons based on input mapping
- automatic recognition of main input device between game controller and keyboard
- improved menu navigation with instructional annotations
- improved menu splash animations
- improved title speedup effect

### optimizations:
- refactor/rewrite of main menu code (also menu listing and menu dialogue component)
- optimization of globe preview render
- writing geometry directly to vertex array, instead of copying from respective object pre-load
- minimized on/off switching of depth testing
- active texture reset call reduction by uniformly switching back after render
- using inlines for small functions with trivial implementation and straight forward usage
- optimized structure for input mapping

### fixes:
- FIXED: globe location preview, faulty render & rotation changes
- FIXED: vsync refresh rate inaccuracies

Worktime: 320h 17min

<br>

## v0.0.5

### features:
- import & interpolate mesh animations
- new physical based lighting system
- deferred shading with support for transparent objects
- loading feedback screen
- custom byte format savefiles
- particle emission & management system
- documented assembly translation analysis
- automatic compilation of all files using the originally updated source (upon request)
- dynamic world loading
- finding all FIXMEs, TODOs & DEPRECATEDs in source code and show in ccb console by path

### optimizations:
- hotplugging controllers
- colour correction: high dynamic range
- colour correction: gamma correction
- switching dynamically between input methods
- constant face culling, no more disable/enable calls
- improved runtime debug output
- improving code quality/portability by substituting pragma once usage
- dynamic directory/file finder for line counter
- autoremoving all libraries when the whole project gets rebuilt
- system to remove world objects should they have been deemed obsolete

### fixes:
- FIXED: shadow transition acne on heavily sloped surfaces
- FIXED: double main loop
- FIXED: memory leaks
- FIXED: inconsitent animation timing results on different machines
- FIXED: memory issues with 3D renderer

Worktime: 322h 51min

<br>

## v0.0.4

### features:
- individual rotations of instanced objects
- individual animations of instanced objects
- system for interactable playing card games
- communication system with conversation tree interpretation

### fixes:
- FIXED: despawned directed projectiles sometimes appear in view
- FIXED: negative jet tilt according to movement input
- FIXED: hitbox indicator over player character somehow gets covered by character model
- FIXED: bullet system renders spawned bullets twice with a slight offset
- FIXED: texture issues with opponent's flyfighter object
- FIXED: lined & misplaced rendering of emitted projectiles

Worktime: 174h 35min

<br>

## v0.0.3 [WIN]

### features:
- preview practice & arcade position in menu
- spawn player controlled bullets
- health bar system
- engine build system
- action menu for system and informations
- time delta disconnected from frames
- most basic healthbar style

### optimizations:
- improved documentation
- structured buffer creation
- removing all planning and progression scripts from repo
- reconceptualizing the basic menu colour scheme
- new dynamic shader program concept
- new renderer code structure
- readable menu code
- simplifications of vertex loading
- 3D protagonists placeholders dpilot
- sorted script source and header file

### fixes:
- FIXED: title animation leaks into camera view when not desired
- FIXED: autocreation of initialization file, if not present
- FIXED: title splash geometry gets modified by sepia vignette
- FIXED: standard splash's idle geometry gets modified by selection
- FIXED: menu dialogue has deprecated functions
- FIXED: collision sphere center off object center
- FIXED: scuffed geometry of globe preview
- FIXED: missing anti aliasing on menu selection splash
- FIXED: unused bullets get rendered at origin position
- FIXED: memory issues with font - text relationship

Worktime: 167h 51min

<br>

## v0.0.2
- Build in Dev Mode & Engine Terminal
- Dynamic Entity Loader & Interpreter
- Windows Port
- Test Builds
- Main Menu Prototype
- Options Configuration Save
- First Player Movement Prototype

Worktime: 120h

<br>

***

<br>

## Trivia
- Total Worktime: 1105h 34min
- Worktime v0.0.7:
  + 281: 30min
  + 258: 29min
  + 253: 16min
- Total Code Lines: 17.485
