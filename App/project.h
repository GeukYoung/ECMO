/****************************************************************************/
#ifndef   __PROJECT_H__
#define   __PROJECT_H__

#include    <stdio.h>
#include    <string.h>
#include    <ctype.h>
#include    <stdlib.h>
#include    <string.h>
#include    <math.h>
#include    <intrinsics.h>
#include    <stdbool.h>

typedef	unsigned char           U8;
typedef	signed char             S8;
typedef	unsigned short          U16;
typedef	signed short            S16;
typedef	unsigned long           U32;
typedef	signed long             S32;        
typedef	unsigned long long	U64;
typedef	signed long long        S64;
typedef float                   F32;
typedef double                  F64;

typedef	unsigned char   BOOL;

#ifndef ON  
  #define ON		        (U8)1
#endif

#ifndef OFF  
  #define OFF		        (U8)0
#endif

#ifndef true  
  #define true		        (U8)1
#endif

#ifndef false  
  #define false		        (U8)0
#endif

#ifndef TRUE  
  #define TRUE		        (U8)1
#endif

#ifndef FALSE  
  #define FALSE		        (U8)0
#endif

#ifndef NULL
  #define NULL (void *)0
#endif


#define	WORDSIZEOF(x)	(sizeof(x))
#define	BYTESIZEOF(x)	(sizeof(x)<<1)
#define	LOBYTE(w)	((w) & 0x00FF)
#define	HIBYTE(w)	(LOBYTE((w)>>8))
#define MAKEWORD(h, l)	((U16)(((h) & 0x00ff)<<8) | (U16)((l) & 0x00ff))
#define MAKEBYTE(h, l)	((U16)(((h) & 0x000f)<<4) | (U16)((l) & 0x000f))

#define	GET_BIT(x, n)	((x >> n) & 0x1)

#define	NTOHS(x)		(((x<<8) & 0xFF00) | ((x>>8) & 0x00FF))
#define	HTONS(x)		NTOHS(x)
#define	NTOHL(x)		(((x<<24) & 0xFF000000L) | ((x<<8) & 0x00FF0000L) | ((x>>8) & 0x0000FF00L) | ((x>>24) & 0x000000FFL))
#define	HTONL(x)		NTOHL(x)

#define	PI		        (3.141592653589793)
#define	SIN60			(0.866025403784439)
#define	SIN30			(0.5)
#define	COS60			(0.5)
#define	COS30			(0.866025403784439)
#define	SQRT2			(1.414213562373095)
#define	SQRT3			(1.732050807568877)
#define	ISQRT2			(0.707106781186548)
#define	ISQRT3			(0.577350269189626)

#define	ABS(x)			(((x) >= 0) ? (x) : -(x))
#define	MAX(x,y)		(((x) >= (y)) ? (x) : (y))
#define	MIN(x,y)		(((x) <= (y)) ? (x) : (y))
#define	SQUARE(x)		((x)*(x))
#define	DEG2RAD(x)		((x)*0.01745329252)
#define	RAD2DEG(x)		((x)*57.2957795131)
#define	RPM2FRQ(x)		((x)/60.)
#define	RPM2ANGFRQ(x)	((x)*0.10471975512)		//((x)*2.*PI/60.) RPM -> rad/sec

#define	SIN(degree)		(sin(DEG2RAD(degree)))
#define	COS(degree)		(cos(DEG2RAD(degree)))

#define	MSEC(sec)		((sec)/1000.)
#define	USEC(sec)		((sec)/1000000.)

#define	ROUND(x)		((Int32)(x + 0.5))

#define SetBit(val,bit)      ( (val) = (val) | (1<<bit) )
#define ClearBit(val,bit)    ( (val) = (val) &~(1<<bit) )
#define ToggleBit(val,bit)   ( (val) = (val) ^ (1<<bit) )
#define GetBit(val,bit)      ( ((val)>>bit) & 1 )

#define RANGE_CONTAIN(var, minval, maxval)  (( (minval <= var) && (var <= maxval) ) ? true : false)
#define IS_ARRAY_ACCESS(var, maxval) (0 <= var && var < maxval) ? true : false
#define	ARRAY_SIZE(x)	        ( sizeof((x))/sizeof((x)[0]) ) 
#define	UPCASE(c)		( ( (c)>='a' && (c)<='z') ? (c)-('a'-'A') : (c) )
#define LOWCASE(c)		( ( (c)>='A' && (c)<='Z') ? (c)+('a'-'A') : (c) )	

#define FOREACH(x,n)  for(int x=0;x<(n);x++)
#define FOR(x,st,ed)  for(int x=st;x<(ed);x++)

/****************************************************************************/

#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_it.h"
#include "cmsis_os.h"
#include "util.h"

typedef union
{
    F32 _float;
    S32 _int;
    U32 _uint;
    U16 _short[2];
    U8  _byte[4];
}uIntConverter_t;

#define FREQ_50HZ       (1)
#define FREQ_60HZ       (0)

#include "AppParam.h"
#include "AppData.h"
#include "arm_math.h"
#include "LED.h"
#include "DIO.h"
#endif