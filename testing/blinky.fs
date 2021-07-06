; \ Start from scratch, even if Forth was left waiting for a ';'
RESET
$24 constant DDRB
$25 constant PORTB
%00100000 constant PB5
." Create a delay function... "
0 variable dummy
: MS 7 * 0 DO dummy @ 1 + dummy ! LOOP ;
." Create a function setting led GPIO as OUTPUT... "
: ENABLE_LED PB5 DDRB ! ;
." Call it... "
ENABLE_LED
." Create a function turning the led on... "
: LEDON PB5 $25 ! ;
." Create a function turning the led off... "
: LEDOFF 0 $25 ! ;
." Create a function heartbeat-ing the led... "
: BLINK 0 DO LEDON 100 MS LEDOFF 100 MS LEDON 100 MS LEDOFF 700 MS LOOP ;
." Call it 10 times "
10 BLINK
