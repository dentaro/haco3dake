#include <Arduino.h>
#include "string.h"

// kaihen.mml
#define TRACK	3
#define	TEMPO	120
#define	REST	1
#define	RE5	2
#define	MI5	3
#define	FA5	4
#define	RA5	5
#define	DO6	6
#define	SO5	7
#define	DO5	8
#define	DO4	9
#define	FA4	10
#define	RA4	11
#define	L8	0
#define	L2	32
#define	L1	64
#define	L4	96
uint8_t len[]={4,16,32,8,0};
#define SQUARE_WAVE
uint16_t C[]={0,0,15289,10204,9091,11454,12135,13621,30578,16198,18182,22908,20408,0};
const char part1[] PROGMEM={
	RE5|L8,MI5|L8,FA5|L8,RA5|L8,
	RE5|L8,MI5|L8,FA5|L8,RA5|L8,
	RE5|L8,MI5|L8,FA5|L8,RA5|L8,
	RA5|L8,RA5|L8,DO6|L8,FA5|L8,
	RE5|L8,FA5|L8,RA5|L8,SO5|L8,
	RE5|L8,FA5|L8,RA5|L8,SO5|L8,
	RE5|L8,MI5|L8,FA5|L8,RA5|L8,
	RE5|L8,MI5|L8,FA5|L8,RA5|L8,
	RE5|L8,MI5|L8,FA5|L8,RA5|L8,
	RA5|L8,RA5|L8,DO6|L8,FA5|L8,
	RE5|L8,FA5|L8,RA5|L8,SO5|L8,
	RE5|L8,FA5|L8,RA5|L8,SO5|L8,
	RE5|L8,MI5|L8,FA5|L8,DO5|L8,REST|L8,
	0};
const char part2[] PROGMEM={
	DO4|L2,FA4|L2,DO4|L2,FA4|L2,
	DO4|L2,FA4|L2,
	DO4|L2,FA4|L2,
	DO4|L2,FA4|L2,
	DO4|L2,FA4|L2,
	0};
const char part3[] PROGMEM={
	FA4|L1,
	REST|L4,
	FA4|L1,
	REST|L4,
	RA4|L1,
	REST|L4,
	FA4|L1,
	REST|L4,
	FA4|L1,
	REST|L4,
	0};