What could you possibly do on a rainy Saturday afternoon?

Why, make a Forth interpreter/compiler from scratch, of course :-)
With the intent to put it up on a 1.5$ microcontroller (Blue Pill)
or even an Arduino, with its extremely tiny 2K RAM!

*I haven't done anything remotely close to this in decades...
Loved it.*

Current status, after 4h of hacking: basic arithmetic,
"functions" (Forth words), literals, constants, and
also variables:

    3 4 + .
    7 OK
    : pi 355 * 113 / ;
     OK
    10000 pi .
    31415 OK
    : x2 2 * ;
     OK
    : p4 4 + ;
     OK
    10 x2 p4 . 
    24 OK
    : yes pi p4 ;
     OK
    10 yes .
    35 OK
    123 variable foo
     OK
    foo @ .
    123 OK
    42 constant lfe
     OK
    lfe foo !
     OK
    foo @
     OK
    .         
    42 OK
    $11 foo !
    foo @ .

**UPDATE**: Porting to the Blue Pill completed! I placed the ported code
in a [separate branch](https://github.com/ttsiodras/MiniForth/tree/BluePill-STM32F103C).

![The 1.5$ 'Beast'](contrib/BluePill.jpg "The 1.5$ 'Beast'")

I first burned the [stm32duino](https://github.com/rogerclarkmelbourne/STM32duino-bootloader)
bootloader on the BluePill, so I could easily program it
in subsequent iterations via the USB connection (and a simple `make upload`)
and immediately afterwards interact with it over the same connection,
used as a serial port.

And indeed, as you can see below, it works!

On the left pane of `tmux` is the output from `make upload`;
and on the right one, I use `picocom` to interact with my mini-Forth
over the serial port.

![Compiling, uploading and testing](contrib/itworks.jpg "Compiling, uploading and testing")

Not bad for a weekend of hacking, methinks :-)

**FINAL UPDATE**: And the ultimate challenge was met!
I fitted it all [inside the tiny brain of an Arduino UNO (2K
RAM)](https://github.com/ttsiodras/MiniForth/tree/Arduino-UNO).

[![Recording of building and uploading on an Arduino UNO](https://asciinema.org/a/422952.svg)](https://asciinema.org/a/422952?autoplay=1)

I had to create my own [list](https://github.com/ttsiodras/MiniForth/tree/Arduino-UNO/src/mini_stl.h)
and `vector`-like C++ templates, since the ArduinoSTL wasted space...
And just as important, it made the build 20x slower.
 Not good for rapid iterations!

It was fun, re-inventing a tiny C++ STL :-)

I also moved all strings to Flash at compile-time (with some nifty macro-ing).

Finally, I also made the code compile and work under x86 - which allows me
to easily debug the logic in modern machines. But I also used sim-avr's
`simduino` to easily simulate execution (interactively!) on the final target.

The final form of my `Makefile` has many rules - e.g. `make arduino-sim`.
Here's what they do:

- **arduino**: Builds src/tmp/myforth.ino.{elf,hex}

- **arduino-sim**: After building, launches the compiled HEX in `simduino`.

- **upload**: After building, uploads to an Arduino attached to `/dev/ttyUSB0`.

- **terminal**: After uploading, launches a `picocom` terminal with
	        all appropriate settings to interact with my Forth.

- **x86**: Builds for x86. Actually, it will build for any native target (ARM, etc).

- **test**: Uses the x86 binary to test the code, executing all steps
	    of the scenario shown above.
