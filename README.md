# BalaBoy 
A Gameboy emulator written in C which runs under linux (ubuntu) with libsdl2.
I've always been fascinated by emulators so I decided to code one in order to understand how old video game hardwares used to work. 

### Screenshots:
![screenshot](https://raw.githubusercontent.com/louvetr/balaboy/master/screenshot/balaboy_01.png "balaboy_01")
![screenshot](https://raw.githubusercontent.com/louvetr/balaboy/master/screenshot/balaboy_02.png "balaboy_02")
![screenshot](https://raw.githubusercontent.com/louvetr/balaboy/master/screenshot/balaboy_03.png "balaboy_03")

# How to play
Controls:
- up arrow:             up
- down arrow:           down
- left arrow:           left
- right arrow:          right
- w:                    A button
- x:                    B button
- control (left):       select
- alt:                  start

### Required packets to install:
sudo apt install libsdl2-dev

### Compilation
gcc balaboy.c cpu.c memory.c gpu.c time.c input.c -o balaboy -lSDL2 -lSDL2_image

### Execution
./balaboy <rom full path> <option: screen scaling>
examples:
./balaboy ./Tetris.gb
./balaboy ./Tetris.gb 3

### Status
What is working:
    - runs Tetris smoothly
    - all cpu instruction implemented

What remains to do:
    - support other games (Video RAM issue ?)
    - check cartridge with ROM & RAM banks are supported (MBC1 type)
    - sound support

### Documentation
The Ultimate Game Boy Talk (33c3)
https://www.youtube.com/watch?v=HyzD8pNlpwI

Game CPU Manual
http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf

Imran Nazar's blog article on gameboy emulation
http://imrannazar.com/GameBoy-Emulation-in-JavaScript

Table describing all CPU instructions 
https://www.pastraiser.com/cpu/gameboy/gameboy_opcodes.html