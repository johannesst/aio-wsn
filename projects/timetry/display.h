#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>
#include <list.h>

#define RESET		0
#define NORMAL		0
#define BRIGHT 		1
#define DIM		2
#define UNDERLINE 	3
#define BLINK		4
#define REVERSE		7
#define HIDDEN		8

#define BLACK 		0
#define RED		1
#define GREEN		2
#define YELLOW		3
#define BLUE		4
#define MAGENTA		5
#define CYAN		6
#define	WHITE		7

void gotoXY(char x, char y);
void setForegroundColor(char col);
void clearScreen();
void setBackgroundColor(char col);
void drawTable(list_t slave_list); 
void fillTable(list_t slave_list); 
void writeTableCell(char x, char y, char* text);
void writeTableCellInt(char x, char y, int text);
void saveLocation();
void restoreLocation();

#endif
