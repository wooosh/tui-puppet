# tuitest

`tuitest` runs a program inside an emulated terminal (xterm/vt100 compatible)
and reads a command file that is capable of sending input, resizing, and
capturing the screen state as a hash. These hashes can then be compared to
ensure your tui program renders consistently.

```
usage: tuitest [-r] <command_file> <executable> <arguments to executable>
  -r | show the terminal state on stderr
     |   this is designed to be used with xterm-truecolor with the following
     |   set in .Xresources:
     |
     |    *VT100.allowWindowOps: true 
```

The command file format is relatively simple. Each line contains a single
command.

Commands:
```
keystream [key] ...
  Sends the stream of keys to stdin. The stream of keys is space separated. if
  a key is not found in the special keys list or parsed as a keybind, the full
  string will be written to stdin.

  example:
    keystream hello <space> world <enter>

  special key names:
    <space>
    <enter>
    <tab>

    <esc>

    <up>, <down>, <right>, <left>
    
    <backspace>
      NOTE: uses ascii delete, not ctrl-h, which is used in some terminals
    <delete>

    <ctrl-X> 
      X can be any one byte character
  keynames to be added:
    <f1> through <f12>
    <ctrl-up,down,right,left>
    <ctrl-shift-x>

  Please create an issue if you need keys other than these.

hash
  Prints the hashed terminal state to stdout.
```

Commands to be added:
```
#
  Does nothing and ignores the rest of the line. A space is required after #

resize <width <height>
  Resizes the terminal to width x height. The terminal is 132x42 by default.

quit
  Exits the program.
```

# Installation

Run dependencies:
- libvterm
- libcrypto (part of openssl)

Build dependencies:
- pkgconfig
- c compiler
- make

```
make
sudo make install
```

# todo

- [ ] handle ctrl-c properly
- [ ] pause on quit in show terminal mode
- [ ] configurable poll timeout
- [ ] sleep command
- [ ] clean up code
- [ ] resizing
- [ ] quit command
- [ ] full keybind parser
- [ ] comments in commandfile
- [ ] hash color info
- [ ] hash cursor position
- [ ] export proper terminal variable
- [ ] tests
