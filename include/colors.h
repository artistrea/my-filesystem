#ifndef COLORS_H
#define COLORS_H


#define CLR_RESET		0
#define CLR_BRIGHT 		1
#define CLR_DIM		2
#define CLR_UNDERLINE 	3
#define CLR_BLINK		4
#define CLR_REVERSE		7
#define CLR_HIDDEN		8
#define CLR_CLEAR -1

#define CLR_BLACK 		0
#define CLR_RED		1
#define CLR_GREEN		2
#define CLR_YELLOW		3
#define CLR_BLUE		4
#define CLR_MAGENTA		5
#define CLR_CYAN		6
#define	CLR_WHITE		7

void textcolor(int attr, int fg);

#endif // COLORS_H
