What could you possibly do in rainy Saturday afternoon?

Why, make a Forth interpreter/compiler from scratch, of course :-)

I haven't done anything remotely close to this in decades...
But hey, I can still do it!

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

Loved hacking this.
