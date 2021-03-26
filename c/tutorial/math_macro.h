#ifndef MATH_MACRO_H_
#define MATH_MACRO_H_

//helpfull math macros

#define MAX(a, b)               (((a) > (b)) ? (a) : (b))
#define MIN(a, b)               (((a) < (b)) ? (a) : (b))
#define SQ(a)                   ((a) * (a))
#define ABS(a)                  ( ((a) < 0) ? -(a) : (a) )
#define CLIP(a, l, h)  			( MAX((MIN((a), (h))), (l)) )
#define SIGN(a)                 ( ((a) > 0) ? 1 : (((a) == 0) ? 0 : -1) )


#endif
