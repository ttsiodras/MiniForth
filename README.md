What could you possibly do on a rainy Saturday afternoon?

Why, make a Forth interpreter/compiler from scratch, of course :-)
With the intent to put it up on a 1.5$ microcontroller (Blue Pill)
or even an Arduino, with its extremely tiny 2K RAM!

*I haven't done anything remotely close to this in decades...
Loved it.*

Current status, after a week of hacking: basic arithmetic,
star-slash, "functions" (Forth words), literals, constants, variables,
string printing, reseting, comments, nested loops, comparisons
and IF/THEN...

    ." Reset... " RESET
    ." Check comments... " \ Yes, we support the new-style comments :-)
    ." Computing simple addition of 3 + 4... " 3 4 + .
    ." Is 1 = 2 ?... " 1 2 = .
    ." Is 1 > 2 ?... " 1 2 > .
    ." Is 1 < 2 ?... " 1 2 < .
    ." Define pi at double-word precision... " : pi 355 113 */ ;
    ." Use definition to compute 10K times PI... " 10000 pi .
    ." Check: 23 mod 7... " 23 7 MOD .
    ." Defining 1st level function1... " : x2 2 * ;
    ." Defining 1st level function2... " : p4 4 + ;
    ." 2nd level word using both - must print 24... " 10 x2 p4 . 
    ." Defining a variable with value 123... " 123 variable ot3
    ." Printing variable's value... " ot3 @ .
    ." Defining The Constant (TM)... " 42 constant lifeUniverse
    ." Printing The Constant (TM)... " lifeUniverse .
    ." Setting the variable to The Constant (TM)... " lifeUniverse ot3 !
    ." Printing variable's value... " ot3 @ .
    ." Setting the variable to hex 0x11... " $11 ot3 !
    ." Printing variable's value... " ot3 @ .
    ." Defining helper... " : p5 5 U.R . ;
    ." Defining 3 times loop... " : x3lp 3 0 DO I p5 LOOP ;
    ." Calling loop... " x3lp
    ." Defining loop calling loop 2 times... " : x6lp 2 0 DO x3lp LOOP ;
    ." Nested-looping 2x3 times... " x6lp
    ." Inline: " : m 3 1 DO 3 1 DO CR J p5 I p5 ." = " J I * p5 LOOP LOOP ;
    ." Use inline loops with two indexes... " m
    ." Make multiples of 7 via DUP... " : m7s 10 0 DO DUP I * . LOOP DROP ;
    ." Print them and DROP the 7... " 7 m7s
    ." Reset... " RESET
    \ Time for Turing completeness...
    ." Let's do Fizz-Buzz! " \ Turing Completeness check...
    \ fizz ( n -- 0_or_1 n )
    ." Define fizz... " : fizz DUP 3 MOD 0 = IF ." fizz " 1 ELSE 0 THEN SWAP ;
    \ buzz ( n -- 0_or_1 n )
    ." Define buzz... " : buzz DUP 5 MOD 0 = IF ." buzz " 1 ELSE 0 THEN SWAP ;
    \ emitNum ( 0_or_1 0_or_1 n -- )
    ." Define emitNum... " : emitNum ROT ROT + 0 = if . ELSE DROP THEN ;
    \ mainloop ( n -- )
    ." Define mainloop... " : mainloop fizz buzz emitNum ;
    \ fb ( -- )
    ." Define fizzbuzz... " : fb 37 1 DO I mainloop LOOP ;
    ." Run it! " fb
    ." Report memory usage... " .S

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

**FINAL UPDATE**: And the ultimate challenge was met! After lots of changes,
I fitted it all [inside the tiny brain of an Arduino UNO (2K
RAM)](https://github.com/ttsiodras/MiniForth/tree/Arduino-UNO).

[![Recording of building and uploading on an Arduino UNO](https://asciinema.org/a/423649.svg)](https://asciinema.org/a/423649?autoplay=1)

I had to create my own heap, as well as [list](https://github.com/ttsiodras/MiniForth/tree/Arduino-UNO/src/mini_stl.h)
and `string`-like C++ templates, since the ArduinoSTL wasted space...
And just as important, it made the build 20x slower.
Not good for rapid iterations!

It was fun, re-inventing a tiny C++ STL :-)

I also moved all strings to Flash at compile-time (with some nifty macro-ing;
see `dprintf` in the code).

Finally, I also made the code compile and work under x86 - which allows me
to easily debug the logic in modern machines. But I also used sim-avr's
`simduino` to easily simulate execution (interactively!) on the final target.

The final form of my `Makefile` has many rules - e.g. `make arduino-sim`.
Here's what they do:

- **arduino**: Builds src/tmp/myforth.ino.{elf,hex}

- **arduino-sim**: After building, launches the compiled HEX in `simduino`.

- **upload**: After building, uploads to an Arduino attached to the port
	      configured inside `config.mk`.

- **terminal**: After uploading, launches a `picocom` terminal with
	        all appropriate settings to interact with my Forth.

- **x86**: Builds for x86. Actually, builds for any native target (ARM, etc).

- **test-address-sanitizer**: Uses the x86 binary to test the code, executing
	all steps of the scenario shown above. The binary is built with the
	address sanitizer enabled (to detect memory issues).

- **test-valgrind**: Same, but with Valgrind.

- **test-arduino**: Sends the entire test scenario shown above to an
	            Arduino Uno connected to the port specified in `config.mk`
	            and shows the responses received over that serial port.
