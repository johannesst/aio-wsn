#include "display.h"

static int l_attr, l_fg, l_bg;

void gotoXY(char x, char y)
{
	printf("%c[%d;%dH", 0x1B, y, x);
}

void setColor(int fg, int bg, int attr)
{	
	if(attr == l_attr && fg == l_fg && bg == l_bg)
		return;
	
	/* Command is the control command to the terminal */
	printf("%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
	l_attr = attr;
	l_fg = fg;
	l_bg = bg;
}

void clearScreen()
{
	printf("%c[%dJ", 0x1B, 2);
	gotoXY(1,1);
}

char rowIsLine[50];


char columnWidth = 12;
char rowHeight = 1;
char columns = 5;
char rows = 15;
char xo = 1;
char yo = 3;

void drawTable()
{
	rowIsLine[2] = 1;
	rowIsLine[10] = 1;

	clearScreen();

	char x,y,xi,yi;

	setColor(WHITE, BLACK, NORMAL);
	
	for(x = 0; x < columns; ++x)
		for(y = 0; y < rows; ++y)
		{
			gotoXY(x * columnWidth + xo, y + yo);
			if(rowIsLine[y])
			{
				if(x == 0)
					printf(" ");
				else
					printf("┼");
				for(xi = 0; xi < columnWidth; ++xi)
					printf("─");
			}
			else
			{
				if(x == 0)
					printf(" ");
				else
					printf("│");
			}
			
		}

	setColor(WHITE, BLACK, BRIGHT);


	writeTableCell(0,0,"Role");
	writeTableCell(0,1,"Address");
	
	for(y = 0; y < 7; y++)
	{
		writeTableCellInt(0,y+3,y + 123);
		
	}

	for(x = 0; x < 4; x++)
	{
		setColor(WHITE, BLACK, BRIGHT);
		if(x == 0)
			writeTableCell(x+1,0,"Master");
		else
			writeTableCell(x+1,0,"Slave");
		
		writeTableCell(x+1,1,"0x1234");
		for(y = 0; y < 7; y++)
		{
			if((x * 3 + y * 7) % 5 == 2 )
			{
				setColor(GREEN, BLACK, BRIGHT);
				writeTableCell(x+1,y+3,"BEEP");
			}
			else
			{
				setColor(WHITE, BLACK, NORMAL);
				writeTableCellInt(x+1,y+3,(x * 117 + y * 73) % 2124);
			}
		}
	}
}

void writeTableCell(char x, char y, char* text)
{
	char len = strlen(text);
	gotoXY(x * columnWidth + xo + 2, y + yo);
	printf("%s",text);
	if(len < columnWidth - 4)
	{
		int i;
		for(i = 0; i < columnWidth - 4 - len; ++i)
			printf(" ");
	}
}

void writeTableCellInt(char x, char y, int text)
{
	gotoXY(x * columnWidth + xo + 2, y + yo);
	printf("%9i",text);
}
