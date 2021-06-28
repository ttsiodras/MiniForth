What could you possibly do on a rainy Saturday afternoon?

Why, make a Forth interpreter/compiler from scratch, of course :-)
With the intent to put it up on a 1.5$ microcontroller!

I haven't done anything remotely close to this in decades...
Loved it.

Current status, after 4h of hacking: basic arithmetic,
"functions" (Forth words), literals, constants, and
also variables:

    3 4 + .
    7 OK
    : pi 355 * 113 / ;
     OK
    10000 pi .
    31415 OK
    : timesTwo 2 * ;
     OK
    : addFour 4 + ;
     OK
    10 timesTwo addFour . 
    24 OK
    : twoLevelsOfCalls pi addFour ;
     OK
    10 twoLevelsOfCalls
     OK
    .
    35 OK
    123 variable foo
     OK
    foo @ .
    123 OK
    42 constant life
     OK
    life foo !
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
