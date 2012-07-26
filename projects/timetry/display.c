#include "display.h"
#include "time_master.h"
#include <string.h>
#include <list.h>

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
//	printf("%c[%dn", 0x1B, 6);
//	int n,m;
//	scanf("[%d;%dR", 0x1B, n, m);
//	printf("Got position %i, %i\n",n,m);
	gotoXY(1,1);
}


void saveLocation()
{
	printf("%c[s", 0x1B);
}

void restoreLocation()
{
	printf("%c[u", 0x1B);
}


char rowIsLine[50];


char columnWidth = 12;
char rowHeight = 1;
char columns = 5;
char rows = 15;
char xo = 1;
char yo = 3;

void drawTable(list_t slave_list) 
{
	saveLocation();
	char nodeCount =  list_length(slave_list);
	columns = nodeCount  + 1;

	rowIsLine[2] = 1;
	rowIsLine[10] = 1;

	//clearScreen();
	gotoXY(80, rows);
	printf("%c[%dJ", 0x1B, 1);


	char x,y,xi;

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

	gotoXY(1,22);
	printf("Habe %i Knoten." , nodeCount);
	restoreLocation();
}

void fillTable(list_t slave_list) 
{	
	saveLocation();
	char x,y,xi;


	char nodeCount =  list_length(slave_list);

	for(y = 0; y < 7; y++)
	{
		writeTableCellInt(0,y+3,y + 123);
		
	}

	struct slave_list_struct *tmp_slave = list_head(slave_list);
	char string[20];
	
	for(x = 0; x < nodeCount; x++)
	{
		setColor(WHITE, BLACK, BRIGHT);
		if(x == 0)
			writeTableCell(x+1,0,"Master");
		else
			writeTableCell(x+1,0,"Slave");
		
		snprintf(string, 9, "%x-%x", tmp_slave->slaveAddr.u8[1],tmp_slave->slaveAddr.u8[0]);
		writeTableCell(x+1,1,string);
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
		tmp_slave = list_item_next(tmp_slave);
	}
	gotoXY(1,22);
	printf("Habe %i Knoten." , nodeCount);
	restoreLocation();
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
