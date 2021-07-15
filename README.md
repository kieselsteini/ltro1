# LTRO-1 Fantasy Console (Lospec Jam 1)
## Overview
- Lua based "fantasy console" LTRO-1
- 240x135 pixel screen resolution
- 10 color display
- 7 button controller
- 2 channel "chip-tune" synthesizer
- graphics / sprites are simple Lua strings
- music / sound effects are simple Lua strings
- More info here https://itch.io/jam/lospec-jam-1
- integrated sprite editor
- Public Domain :)
- SDL2 is the only external dependency

## Functions
- **F1**: resume with the Lua script
- **F2**: enter sprite editor

### Lua Script
The game is programmed using the Lua (http://www.lua.org) programming language. LTRO-1 will include the newest Lua 5.4.3 as its runtime.

### Sprite Editor
There is a very simple sprite editor in LTRO-1. You can draw 12x12 sprites and export/import it to/from the clipboard. Sou you can simply paste the exported string right into your Lua script.

## Programming API
### ltro.quit()
Forces the fantasy console to quit immediately.

### ltro.btn(button)
Returns *true* if *button* is pressed, *false* otherwise. Valid strings for button are:
- **up**, **down**, **left**, **right** for d-pad
- **a**, **b** for buttons
- **start** for the extra button

```lua
if ltro.btn('a') then
    -- do something with a
end
```

### ltro.btnp(button)
Returns *true* if *button* is pressed once, *false* otherwise. Valid strings for button are:
- **up**, **down**, **left**, **right** for d-pad
- **a**, **b** for buttons
- **start** for the extra button

```lua
if ltro.btnp('a') then
    -- do something with a
end
```

### ltro.clearcolor([color])
If *color* is given, it will be set as the new default clear color. It also affects the non-drawable area around the screen, when the window has not 16:9 aspect ratio. Always returns the current clear color.

```lua
local color = ltro.clearcolor() -- get the current clear color
ltro.clearcolor(5) -- set new clear color
```

### ltro.clear([color])
Clears the scren with the given *color*. If no color is given, the default clear color will be used.
Returns nothing.

```lua
ltro.clear() -- clear with default clear color
ltro.clear(9) -- clear with color 9
```

### ltro.pixel(color, x, y)
Draw a single pixel with *color* at *x*, *y*.
Returns nothing.

### ltro.line(color, x0, y0, x1, y1)
Draw a line with *color* from *x0*, *y0* to *x1*, *y1*.
Returns nothing.

### ltro.rect(color, x0, y0, x1, y1 [, fill])
Draw a rectangle with *color* from *x0*, *y0* to *x1*, *y1*. If *fill* is not falsy it will fill the rectangle.
Returns nothing.

### ltro.circle(color, x0, y0, radius, [, fill])
Draw a circle with *color* at *x0*, *y0* in *radius* size. If *fill* is not falsy it will fill the circle.
Returns nothing.

### ltro.print(color, x, y, text)
Print the given ASCII *text* with *color* at *x*, *y*. You can use all 256 glyphs from the standard IBM-PC.
Returns nothing.

### ltro.draw(image, x, y [, mask])
Draws the given *image* string to *x*, *y*. If *mask* is given, all colors with that index will be transparent.
The image string has a special format. The first two characters denote the width of the image. The next two characters the height. Then follow all the pixels as single characters ranging from **0**-**9**.

Example:
``10010123456789``
This is an image, 10 pixels width and 1 pixel height. Containing all 10 colors.

```lua
ltro.draw('10010123456789', 0, 0) -- draw this example to the screen
```

### ltro.gain([gain])
If *gain* is given it will se the current volume for the audio output (range 0.0 - 1.0). Always returns the current audio volume.

```lua
local gain = ltro.gain() -- just query the current audio volume
ltro.gain(0.5) -- set volume to 50%
```

### ltro.play(channel, mml)
Starts the playback of the given MML (https://en.wikipedia.org/wiki/Music_Macro_Language) string on audio *channel*.
Returns nothing.

Remarks to MML:
| Command | Function |
| ------- | -------- |
| **cdefgab** | Play the corresponding note. Sharp notes are appeded with **+** or **#**, flat notes with **-**. A length value might follow. Example: *c+8* Play C# as 1/8. |
| **p**, **r** | Pause the playback |
| **o** | Set the octave (0 - 7) |
| **<**, **>** | Go one octave down or up |
| **l** | Sets the default note length |
| **t** | Sets the tempo in beats per minute |
| **m** | Set the PSG mode: **5** 50% pulse wave, **2** 25% pulse wave, **1** 12.5% pulse wave. Example: *m2* sets the playback to 25% pulse wave. |

```lua
ltro.play(1, 'cdefgab>c') -- just plays one octave :)
```

### ltro.stop(channel)
Immediately stops the playback for the given audio channel (1 - 2).
Returns nothing.

```lua
ltro.stop(1) -- stop audio channel 1
```

## Update Log

### 0.5.0
- fixed package creation for Emscripten/Windows
- added rudimentary sprite editor (can be accessed with F2)

### 0.4.0
- renamed "init.lua" to "game.lua"
- added 3 types of pulse wave generators

### 0.3.0
- added HTML5/JavaScript port (Emscripten)

### 0.2.0
- added simple game controller support
- included Windows build (MinGW 64)

### 0.1.0
- first implementation
