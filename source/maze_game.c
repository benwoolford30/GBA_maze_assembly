
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <maxmod.h>		// maxmod library
#include <math.h>
#include "soundbank.h"		// created by building project
#include "soundbank_bin.h"	// created by building project

//extern const unsigned int charData[2048]

extern void generatemazeasm(struct cell* maze, unsigned short* layertodrawtiles);	//initialise the maze in assembly
extern void drawtileasm(struct cell* space, unsigned short* layertodrawtiles);		//function to draw maze in assembly
extern unsigned int charData[];			//tile data using tile converter
#define MAXSIZE 60

// need "extern" because the function body is in an external .s file

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
struct fourway		//struct to store wether theres a gap with a neighbour
{
	int up;
	int left;
	int right;
	int down;
};
struct cell			//struct to hold data about each cell of the maze
{
	int x;			//x position of maze
	int y;			//y position of maze
	int visited;		// has the cell been visited by the recursive backtracker algorithm
	int current;		//current cell being used in the recursive backtracker algorithm
	struct cell* neighbour[4];	//pointers to 4 adjacent neighbours of this cell (if they exist)
	/*
	0= left
	1= up
	2= right
	3=down
	*/
	struct fourway wall;		//struct to hold whether there is a gap to the left,right,up or down of this cell
	
};
typedef enum Gamestates {	//enumerator for the different states of the game
	menu = (1),	
	pause = (2),
	Gameover = (3),
	Maingame =(4),
	instructionspage =(5),
	highscorepage = (6),
	highscoreenter =(7),
	levelintro = (8),
	leveloutro = (9),
	Quit = (10),
} Gamestates;
struct cell maze[9][9];		//2d array of maze cells
void initialisepalettes(unsigned short* palleteaddress,int noofpalettes) //set up palettes in game
{
	unsigned short* newcolour;	
	int blueoffset=60;
	int redoffset=120;
	int greenoffset=0;
	for (int i=1;i<=noofpalettes;i++) //initialise background palette with random colours
	{
		newcolour=(unsigned short*)palleteaddress+(i*16);
		blueoffset+=60;
		redoffset+=20;
		greenoffset+=45;
		newcolour[0]=RGB8(200+redoffset,191+blueoffset,231+greenoffset);	
		newcolour[1] = RGB8(255+redoffset, 127+blueoffset, 39+greenoffset);
		newcolour[2] = RGB8(255+redoffset, 242+blueoffset, 0+greenoffset);
		newcolour[3] = RGB8(34+redoffset, 177+blueoffset, 76+greenoffset);
		newcolour[4] = RGB8(0+redoffset, 162+blueoffset, 232+greenoffset);
		newcolour[5] = RGB8(163+redoffset, 73+blueoffset, 164+greenoffset);
		newcolour[6] = RGB8(255+redoffset, 255+blueoffset, 255+greenoffset);
		newcolour[7] = RGB8(153+redoffset, 217+blueoffset, 234+greenoffset);
		newcolour[8] = RGB8(181+redoffset, 230+blueoffset, 29+greenoffset);
		newcolour[9] = RGB8(0+redoffset, 0+blueoffset, 0+greenoffset);
		newcolour[10] = RGB8(128+redoffset, 64+blueoffset, 0+greenoffset);
		newcolour[11] = RGB8(64+redoffset, 128+blueoffset ,128+greenoffset);
		newcolour[12] = RGB8(255+redoffset, 0+blueoffset, 128+greenoffset);
		newcolour[13] = RGB8(237+redoffset, 28+blueoffset, 36+greenoffset);
		newcolour[14] = RGB8(128+redoffset, 0+blueoffset, 255+greenoffset);
		newcolour[15] = RGB8(0+redoffset, 64+blueoffset, 128+greenoffset);
	}
	
}
void drawtext(char* string,int arraysize,int xcoord,int ycoord)	//draw a letter tile on the screen 
{
	unsigned short* layer1=(unsigned short*) 0x06003800;
	for (int i=0;i<arraysize;i++)	//iterate through size of string/char*
	{
		int tilehouse=string[i]-86;		
		layer1[(ycoord * 32) + xcoord+i] = (tilehouse) | (15 << 12);		//draw letter to bg1	
	}
}
void drawtile(int x,int y)	//draw cell from maze in game 
{
	
	/* x*=2;
	y*=2;
	x+=5;
	y+=1;
	
	/*
		x= 10 start
		y=4 start
		x=21 end width ==11
		y=17 end yheight 13
		width 6
		height 7
	
	 
	 gamecell.x=x;
	 gamecell.y=y;
	 gamecell.visited=false;
	 gamecell.current=false;
	 gamecell.wall.up=false;
	 gamecell.wall.down=false;
	 gamecell.wall.left=false;
	 gamecell.wall.right=false;	
	 gamecell.neighbour[0]=NULL; //set neighbours to null
	 gamecell.neighbour[1]=NULL;
	 gamecell.neighbour[2]=NULL;
	 gamecell.neighbour[3]=NULL; */

	unsigned short* layer1=(unsigned short*) 0x06003800; // get background 1 and assign maze to it
	layer1[(y * 32) + x] = (6) | (0 << 12);
	layer1[(y * 32) + x+1] = (8) | (0 << 12);
	layer1[((y+1) * 32) + x] = (9) | (0 << 12);
	layer1[((y+1) * 32) + x+1] = (10) | (0 << 12);
	/*
		initialise the tile to be
		 ---
		|	|
		|	|
		 ---
		 
	*/
}
void redrawtile(struct cell gamecell)	//draw appropriate grpahical representation of tile based on the walls it has
{
	int x=gamecell.x;
	int y=gamecell.y;
	
	int tiles[4]={6,8,9,10};	//by default set to be a fully closed tile 
	/*
		 ---
		|	|
		|	|
		 ---
		 
	*/
	unsigned short* layer1=(unsigned short*) 0x06003800;	//drawing to bg layer 1
	/*
		1 is left wall
		2 is top wall
		3 is right wall
		4 is bottom wall
		*/
	if (gamecell.wall.left && gamecell.wall.up) //there are neighbours left and up
	{		
		tiles[0]=0;
	}
	if (gamecell.wall.left && !gamecell.wall.up)//there are neighbours left but not up
	{
		
		tiles[0]=2;
	}
	if (!gamecell.wall.left && gamecell.wall.up)//there are neighbours left but not up
	{		
		tiles[0]=1;
	}
	if (gamecell.wall.left && gamecell.wall.down)//there are neighbours left and down
	{		
		tiles[2]=0;
	}
	if (gamecell.wall.left && !gamecell.wall.down)//there are neighbours left but not down
	{		
		tiles[2]=4;
	}
	if (!gamecell.wall.left && gamecell.wall.down)//there are neighbours left but not up
	{
		
		tiles[2]=1;
	}
	if (gamecell.wall.right && gamecell.wall.up) //there are neighbours left and up
	{
		
		tiles[1]=0;
	}
	if (gamecell.wall.right && !gamecell.wall.up)//there are neighbours left but not up
	{
		
		tiles[1]=2;
	}
	if (!gamecell.wall.right && gamecell.wall.up)//there are neighbours left but not up
	{
		
		tiles[1]=3;
	}
	if (gamecell.wall.right && gamecell.wall.down)//there are neighbours left and down
	{		
		tiles[3]=0;
	}
	if (gamecell.wall.right && !gamecell.wall.down)//there are neighbours left but not down
	{		
		tiles[3]=4;
	}
	if (!gamecell.wall.right && gamecell.wall.down)//there are neighbours left but not up
	{		
		tiles[3]=3;
	}
	layer1[(y * 32) + x] = (tiles[0]) | (0 << 12);
	layer1[(y * 32) + x+1] = (tiles[1]) | (0 << 12);
	layer1[((y+1) * 32) + x] = (tiles[2]) | (0 << 12);
	layer1[((y+1) * 32) + x+1] = (tiles[3]) | (0 << 12);
	/*
		if there are gaps to the left and bottom the tile would be drawn like this
		----
		   |
		   |
	*/
	
}
struct mystack // own stack implementation	for some reason the stack header didn't work so improvised 
{
	int top;	//top of stack
	struct cell* stack[MAXSIZE];	//stack holds cells 
};
struct mystack mystackconstructor(struct mystack* thestack) //constructor for stack
{
	thestack->top=-1;	//initilaise top value to be -1
}
struct cell* top(struct mystack* thestack) // check top value of stack
{
	struct cell* specificspot=NULL;		//initialise prt to null 
	if (thestack->top==-1)	//check stack isn't empty 
	{				
		return specificspot;
	}
	else
	{
		specificspot=thestack->stack[thestack->top];		
	}
	return specificspot;
}
void push(struct mystack* thestack,struct cell* space) // put cell space onto stack
{
	if (thestack->top>MAXSIZE) //check stack size is beyond limit
	{
		return;
	}
	else
	{
		thestack->top+=1;
		thestack->stack[thestack->top]=space; // increment size and store space in stack
	}
	return;
}
struct cell* pop(struct mystack* thestack) // pop cell space from stack
{
	struct cell* specificspot; 
	if (thestack->top==-1) // check stack isn't empty
	{
		
		specificspot->x=-999;
		specificspot->y=-999;
		
		return specificspot;// return an impossible value to inform that the stack is empty
	}
	else
	{
		specificspot=thestack->stack[thestack->top]; // get space from stack
		thestack->top-=1; //decrement stack size
	}
	return specificspot;
}

struct cell* generatemaze()	//functiopn to create a procedurally generated maze
{	
	unsigned short* layer1=(unsigned short*) 0x06003800;	//assign layer 1 to draw maze to
	generatemazeasm(&maze[0][0],layer1);
	/*
		this generatemazeasm function replaces the code commented out underneath
	*/
	/* for (int i=0;i<9;i++)
	{
		for (int i2 =0;i2<9;i2++)	//reset the screen with empty tiles and initialises values
		{
			//layer1[(i2 * 32) + i] = (47) | (0 << 12);
			/*  maze[i][i2].x=i;
			maze[i][i2].y=i2;
			drawtileasm(&maze[i][i2],layer1);  
			drawtile(maze[i][i2].x,maze[i][i2].y);		
			//maze[i][i2]=drawtile(i,i2);				
		}
	} */
	for (int i=0;i<9;i++)
	{
		for (int i2 =0;i2<9;i2++)	//Set neighbours
		{			
			if (i-1>=0) //left neighbour
				maze[i][i2].neighbour[0]=&maze[i-1][i2];
			if (i2-1>=0)//up neighbour 
				maze[i][i2].neighbour[1]=&maze[i][i2-1];
			if (i+1<9)//right neighbour
				maze[i][i2].neighbour[2]=&maze[i+1][i2];
			if (i2+1<9)//down neighbour
				maze[i][i2].neighbour[3]=&maze[i][i2+1];
				maze[i][i2].neighbour[2]->wall.right=false;
		}
	}
	struct mystack gamestack;	//declare own implementation of stack
	mystackconstructor(&gamestack);	//"constructor"
	bool unvisited=false;	
	

	struct cell* current=&maze[2][5];	//start recursive backtracking algorithm at this spot of the maze
	current->visited=true;
	struct cell* unvisneigh[4];	
	struct cell* chosenneigh;
	int numofneigh=0;	//track how many adjacent cells there're availible 

	unsigned short* layer0=(unsigned short*) 0x06005000;	
	while (unvisited==false) //all spaces have been connected for maze
	{				
		numofneigh=0;
		for (int i=0; i<4;i++)	//go through left,right,up,down neighbours
		{
			if (current->neighbour[i]!=NULL) //check that there is a neighbour
			{
				if (current->neighbour[i]->visited!=true) //check that it has been visited
				{
					unvisneigh[numofneigh]=current->neighbour[i]; //assign neighbour to array
					numofneigh+=1;
					
				}				
			}
		}	
		if (numofneigh>0) //check if there are any cells left
		{	
			chosenneigh=unvisneigh[0+((rand())%numofneigh)]; //assign a random neighbour
			for (int i=0; i<4;i++)
			{
				if (current->neighbour[i]!=NULL) //check that there is a neighbour
				{
					
					if (current->neighbour[i]==chosenneigh) //check that it has been visited
					{
						
						switch (i) //find out which direction the neighbour is
						{
							case 0:{ //left
								current->wall.left=true;
								chosenneigh->wall.right=true;
							}break;
							case 1:{ //up
								current->wall.up=true;
								chosenneigh->wall.down=true;
							}break;
							case 2:{ //right
								current->wall.right=true;
								chosenneigh->wall.left=true;
							}break;
							case 3:{ //down
								current->wall.down=true;
								chosenneigh->wall.up=true;
							}break;
						}
						push(&gamestack,current);	//push current cell to stack
						current=chosenneigh;	//assign new neighbour
						current->visited=true;	//set visited to true
						numofneigh=0;	//reset neigh num to 0						
						break; //we've found the neighbour so no need to do rest of loop
					}				
				}
			}		
		}
		else	//no cells left
		{
			if (gamestack.top>-1) //check if stack is empty
			{	
				current=pop(&gamestack);	//pop a cell from stack 
				current->visited=true;	//set visited to true
			}
			else
			{
				bool nomorespaces=true;	//check that all cells have been used 
				for (int i=0;i<9;i++)
				{
					for (int i2 =0;i2<9;i2++)	//iterate through whole 2d array
					{
						if (maze[i][i2].visited==false)	//check there aren't any remaining cells left to apply recursive backtracker to 
						{
							current=&maze[i][i2];	//assign found unused cell to current
							current->visited=true;
							nomorespaces=false;						//set value to false		
						}
						if (nomorespaces==false)
						{
							break;
						}	
					}
					if (nomorespaces==false)
					{
						break;
					}						
				}
				if (nomorespaces==true)
					unvisited=true;	//set value to leave loop to true
			}
		}
	}
	layer0[((current->y) * 32) + current->x+1] = (72) | (0 << 12);	//set goal for player to reach-orange tree
	for (int i=0;i<9;i++)
	{
		for (int i2 =0;i2<9;i2++)
		{
			redrawtile(maze[i][i2]);	//graphically display new maze to player
		}
	}
	
	return current;	// return target area cell
}
void clearscreen()	//clear the different layers with tile 0 (blank)
{
	unsigned short* layer0=(unsigned short*) 0x06005000;
	unsigned short* layer1=(unsigned short*) 0x06003800;
	unsigned short* layer2=(unsigned short*) 0x06002000;
	unsigned short* layer3=(unsigned short*) 0x06006000;
	for (int i=0;i<=31;i++)
	{
		for (int i2 =0;i2<=31;i2++)	//reset the screen with empty tiles
		{
			layer1[(i2 * 32) + i] = (0) | (0 << 12);
			layer2[(i2 * 32) + i] = (0) | (0 << 12);
			layer3[(i2 * 32) + i] = (0) | (0 << 12);
			layer0[(i2 * 32) + i] = (0) | (0 << 12);
		}					
	}
}
struct cell* initialisegamestate(int Gstate)	//set the game graphically depending on game state in
{
	unsigned short* layer1=(unsigned short*) 0x06003800;	//drawing on layer 1
	switch (Gstate)	//check which game state in
	{
		case instructionspage:{
		
		}break;
		case highscorepage:{
			
		}break;
		case highscoreenter:{
			
		}break;
		case levelintro:{
							
		}break;
		case leveloutro:{
							
		}break;
		case Quit:{
			clearscreen();
		}break;
		case pause:{
		
		}break;
		case Gameover:{
			
		}break;
		case Maingame:{	//maze 
			clearscreen();		
			setRepeat(1, 2);	//set key repeat to allow a less jittery feel for movement 
			char string[5]= {'s','c','o','r','e'};
			drawtext(string,5,24,2);	//display score on screen
			layer1[(((2*3)+0) * 32) + 24] = (37+(((int)(10)))) | (0 << 12);	//set initial score to 0
			return generatemaze();	//generate maze
		}break;
		case menu:{	//game menu
			clearscreen();	//clear screen
			int ymenuset=-4;	//set up in game menu to bg1
			char string[5]= {'s','t','a','r','t'};
			drawtext(string,5,12,8+ymenuset);	
			char stringa[10]= {'t','i','m','e','t','t','r','i','a','l'};	
			drawtext(stringa,10,10,11+ymenuset);
			char stringd[4]= {'q','u','i','t'};	
			drawtext(stringd,4,12,14+ymenuset);
			layer1[(4 * 32) + 8] = (37) | (0 << 12);	//set star tile to first option
			setRepeat(1, 20);	//set key down repeat to every 20  frames to allow ease of navigating menu
		}break;
	}
	return NULL;
}
int main(void) {
//---------------------------------------------------------------------------------
	int seed=0;		//set seed of srand 	
	irqInit();
	irqSet(IRQ_VBLANK, mmVBlank);
	irqEnable(IRQ_VBLANK);
	mmInitDefault((mm_addr)soundbank_bin, 8 ); //set up sound 
	int Gamestate=menu;	//set first gamestate to menu	
	unsigned int* tilechar=(unsigned int*)0x6000900; //rewriteing tile
	tilechar[0]= ((3<<0)|(0<<4)|(0<<8)|(2<<12)|(2<<16)|(0<<20)|(0<<24)|(3<<28));
	tilechar[1]= ((0<<0)|(1<<4)|(0<<8)|(2<<12)|(2<<16)|(0<<20)|(0<<24)|(0<<28));
	tilechar[2]= ((0<<0)|(0<<4)|(1<<8)|(1<<12)|(1<<16)|(1<<20)|(0<<24)|(0<<28));
	tilechar[3]= ((1<<0)|(0<<4)|(0<<8)|(1<<12)|(1<<16)|(0<<20)|(0<<24)|(1<<28));
	tilechar[4]= ((0<<0)|(1<<4)|(0<<8)|(1<<12)|(1<<16)|(0<<20)|(1<<24)|(0<<28));
	tilechar[5]= ((0<<0)|(0<<4)|(1<<8)|(1<<12)|(1<<16)|(1<<20)|(0<<24)|(0<<28));
	tilechar[6]= ((0<<0)|(0<<4)|(0<<8)|(1<<12)|(1<<16)|(0<<20)|(0<<24)|(0<<28));
	tilechar[7]= ((3<<0)|(0<<4)|(1<<8)|(1<<12)|(1<<16)|(1<<20)|(0<<24)|(3<<28));	
	unsigned int* objchar = (unsigned int*)0x06010000; //obj sprite
	objchar[562 * 8 + 0] = ((0 << 0) | (0 << 4) | (0 << 8) | (2 << 12) | (2 << 16) | (0 << 20) | (0 << 24) | (3 << 28) | (3<<32));
	objchar[562 * 8 + 1] = ((0 << 0) | (1 << 4) | (0 << 8) | (2 << 12) | (2 << 16) | (0 << 20) | (0 << 24) | (0 << 28));
	objchar[562 * 8 +2] = ((0 << 0) | (0 << 4) | (1 << 8) | (1 << 12) | (1 << 16) | (1 << 20) | (0 << 24) | (0 << 28));
	objchar[562 * 8 + 3] = ((1 << 0) | (0 << 4) | (0 << 8) | (1 << 12) | (1 << 16) | (0 << 20) | (0 << 24) | (1 << 28));
	objchar[562 * 8 + 4] = ((0 << 0) | (1 << 4) | (0 << 8) | (1 << 12) | (1 << 16) | (0 << 20) | (1 << 24) | (0 << 28));
	objchar[562 * 8 + 5] = ((0 << 0) | (0 << 4) | (1 << 8) | (1 << 12) | (1 << 16) | (1 << 20) | (0 << 24) | (0 << 28));
	objchar[562 * 8 + 6] = ((0 << 0) | (0 << 4) | (0 << 8) | (1 << 12) | (1 << 16) | (0 << 20) | (0 << 24) | (0 << 28));
	objchar[562 * 8 + 7] = ((3 << 0) | (0 << 4) | (1 << 8) | (1 << 12) | (1 << 16) | (1 << 20) | (0 << 24) | (3 << 28));	// 0= black 1== white 2==green
	unsigned short* newcolour=(unsigned short*)0x050001E4;
	newcolour[0]=(0<<0)|(10<<5)|(0<<10); 		//red,blue,green 
	newcolour=(unsigned short*)0x050001E6;
	newcolour[0]=RGB8(231,242,78);//(16<<0)|(5<<5)|(27<<10);
	newcolour=(unsigned short*)0x05000000;
	//definiing palette for char data array
	newcolour[0]=RGB8(200,191,231);	
	newcolour[1] = RGB8(255, 127, 39);
	newcolour[2] = RGB8(255, 242, 0);
	newcolour[3] = RGB8(34, 177, 76);
	newcolour[4] = RGB8(0, 162, 232);
	newcolour[5] = RGB8(163, 73, 164);
	newcolour[6] = RGB8(255, 255, 255);
	newcolour[7] = RGB8(153, 217, 234);
	newcolour[8] = RGB8(181, 230, 29);
	newcolour[9] = RGB8(0, 0, 0);
	newcolour[10] = RGB8(128, 64, 0);
	newcolour[11] = RGB8(64, 128 ,128);
	newcolour[12] = RGB8(255, 0, 128);
	newcolour[13] = RGB8(237, 28, 36);
	newcolour[14] = RGB8(128, 0, 255);
	newcolour[15] = RGB8(0, 64, 128);
	

	initialisepalettes((unsigned short*)0x05000220,13);	//initialise sprite palette
	initialisepalettes((unsigned short*)0x05000020,12);
	
	int i = 0;
	unsigned int* tileset1 = (unsigned int*)0x6004000;	//char base 1
	int count = 0;
	int i2 = 8;
	for (i = 8; i < 2040; i++)//iterate through array ignoring first 8 rows
	{
		if (count==8)	//check next row
		{ 
			count = 0;
			i += 1;
		}		
		tileset1[i2] = charData[i];	//assign pixel from bmp to gba char base 1
		count++;
		i2++;

	}
	newcolour=(unsigned short*)0x05000200;	
	for (i=0;i<15;i++)
	newcolour[i]=RGB8(0,((16-i)*12),0);
	newcolour[0] = RGB8(((16 - 0) * 12), 0, 0); 
	/*
		bit 0 = graphics mode = (0-5)	(1<<0)
		bit 6 = obj data mode on = 1 	(1<<5)
		bit 8 = bg0 1 = on 				(1<<8)
		bit 9 = bg1 1 = on 				(1<<9)
		bit 10 = bg2 1 = on 			(1<<10)
		bit 11 = bg3 1 = on 			(1<<11)
		bit 12 = obj on off 			(1<<12)
	*/
	unsigned short* displaycnt=(unsigned short*)0x04000000;
	*displaycnt = ((0<<0)|(1<<6)|(1<<8)|(1<<9)|(1<<10)|(1<<11)|(1<<12));
	//initialise the backgrounds of the game
	unsigned short* Bg1=(unsigned short*)0x04000008;
	Bg1[0]= ((0<<0)|(0<<2)|(0<<7)|(10<<8)|(0<<14));
	unsigned short* Bg2=(unsigned short*)0x0400000A;
	*Bg2= ((1<<0)|(1<<2)|(0<<7)|(7<<8)|(0<<14));
	unsigned short* Bg3=(unsigned short*)0x0400000C;
	*Bg3= ((2<<0)|(1<<2)|(0<<7)|(4<<8)|(0<<14));
	unsigned short* Bg4=(unsigned short*)0x0400000E;
	*Bg4= ((3<<0)|(1<<2)|(0<<7)|(12<<8)|(0<<14));	
	//screen is 240x160 
	int a=40;
	int b=12;	
	unsigned short* layer0=(unsigned short*) 0x06005000;
	unsigned short* layer1=(unsigned short*) 0x06003800;
	unsigned short* layer2=(unsigned short*) 0x06002000;
	unsigned short* layer3=(unsigned short*) 0x06006000;
	//base_block_address [(y*32) + x] layer (coords)		
	setRepeat(1, 20);	//set keydown repeat to register a held down every 20 frames
	clearscreen();	//clear screen for all layers
	int ymenuset=-4;		//initialise menu/draw it
	char string[5]= {'s','t','a','r','t'};
	drawtext(string,5,12,8+ymenuset);	
	char stringa[10]= {'t','i','m','e','t','t','r','i','a','l'};	
	drawtext(stringa,10,10,11+ymenuset);
	char stringd[4]= {'q','u','i','t'};	
	drawtext(stringd,4,12,14+ymenuset);
	layer1[(4 * 32) + 8] = (37) | (0 << 12);	//set star to menu option
	int palettetouse=3;	// set scrolling stars to different palettes
	for (int i=1;i<=7;i++)
	{		
		layer2[((4*i) * 32) + 5] = (37) | (palettetouse << 12);		//set a different palette
		layer3[((4*i) * 32) + 25] = (37) | (palettetouse << 12);
		palettetouse+=1;
	}
	int option=0;	//menuoption
	unsigned short* obj_oam = (unsigned short*)0x07000000;	//oam/player
	int movspeed=2;	//player movespeed
	int countdown=22*59.73;	//timer- gba has a fixed frame rate of 57.73 frames per second
	bool horileft=false;
	bool vertdown=false;	//flipping the oam horizontally or vertically based on input
	int xscroll=0;
	int yscroll=0;		//x and y scroll of screen
	unsigned short* bg1xscroll = (unsigned short*)0x04000014;
	unsigned short* bg1yscroll = (unsigned short*)0x04000016;
	unsigned short* bg0xscroll = (unsigned short*)0x04000010;
	unsigned short* bg0yscroll = (unsigned short*)0x04000012; 
	unsigned short* bg2yscroll = (unsigned short*)0x0400001A;	//memory address for scrolling different layers
	unsigned short* bg3yscroll = (unsigned short*)0x0400001E;	
	int targetx=-100;
	int targety=-100;	//dummy values for player goal
	struct cell* target=NULL;	//target cell players has to reach
	int geta= 0;	//x of oam as a tile
	int getb= 0;	//y of oam as a tile
	int tilex=0;	//the i in the maze array 
	int tiley=0;	//the i2 in the maze array
	int score=0;	//player score
	bool timetrial=false;	//is game in timetrial mode
	while (1) // main game loop
	{
		
		seed+=1;	//increment seed every frame- alternative to srand(time(NULL))
		mmStart( MOD_CT5PROGC_SND, MM_PLAY_LOOP );	//play sound
		scanKeys();	//refresh flags for keys 
		unsigned short k = keysDownRepeat();	//check for keys
		int oldgamestate=Gamestate;		//assign current frame gamestate to old gamestate
		switch (Gamestate)	//check which gamestate we're in 
		{
			case menu:{		//menu 		
				for (int i=1;i<=7;i++)	//palette swap scrolling stars
				{					
					layer2[((4*i) * 32) + 5] = (37) | (palettetouse << 12);					
					layer3[((4*i) * 32) + 25] = (37) | (palettetouse << 12);
					palettetouse+=1;
				}
				yscroll-=1;
				*bg2yscroll=(yscroll<<1);	//scroll layer 2 up
				*bg3yscroll=(yscroll*-1<<1);	//scroll layer 3 down
				if (k != 0)	//if there is input
				{					
					layer1[(((option*3)+4) * 32) + 8] = (0) | (0 << 12); //previous position is replaced with blank
					if (k & KEY_UP) //if there is a up arrow press
					{ 						 
						if (option>0)
							option-=1;	//decrement option- go up in menu
					}
					if (k & KEY_DOWN) //if there is a down arrow press
					{ 					
						if (option<2)
							option+=1;	//increment option- go down in menu			
					}
					if (k & KEY_START)	//if key start is pressed
					{
						switch (option)	//check which menu option is selected
						{
							case 0:{	//normal mode
								a=40;
								b=12;
								obj_oam[(1*4)]= ((b<< 0) | (0 << 8) | (0 << 9) | (0 << 14));
								obj_oam[(1 * 4)+1] = ((a << 0) | (0 << 12) | (0 << 13) | (2 << 14));	
								obj_oam[(1 * 4)+2] = (562<<0) | (0<<10) | (0<<12);
							
								Gamestate=Maingame;	//set state to maze 
								timetrial=false;	//set timetrial to false
								srand (seed);	//set the seed
								for (int i=0;i<10;i++)
									rand();	//read up from sources that first few rands produce similar results-to get different patterns call rand a few times
							}break;
							case 1:{	//time trial mode							
								a=40;
								b=12;
								obj_oam[(1*4)]= ((b<< 0) | (0 << 8) | (0 << 9) | (0 << 14));
								obj_oam[(1 * 4)+1] = ((a << 0) | (0 << 12) | (0 << 13) | (2 << 14));	
								obj_oam[(1 * 4)+2] = (562<<0) | (0<<10) | (0<<12);								
								Gamestate=Maingame;
								timetrial=true;	//turn on timetrial
								srand (seed);
								for (int i=0;i<10;i++)
									rand();
							}break;
							case 2:{//quit						
								Gamestate=Quit;
							}break;
						}	
					}
					layer1[(((option*3)+4) * 32) + 8] = (37) | (0 << 12);	//draw new option tile 
				}
			}break;
			case pause:{
				
			}break;
			case Maingame:{ //maze game state
				
				
				if (timetrial==true)	//is there a time trial
				{
					countdown--;	//decrement 
					if (countdown<1) //reset game
					{
						Gamestate=menu;	//go back to menu
						countdown=22*59.73;	//reset timer
						option=0;	//set menu option to 0
						b=550;
						a=550;	//move oam beyond screen base block
						obj_oam[(1*4)]= ((b<< 0) | (0 << 8) | (0 << 9) | (0 << 14));
						obj_oam[(1 * 4)+1] = ((a << 0) | (horileft << 12) | (vertdown << 13) | (0 << 14));
						//draw new oam
					}
					if ((int)(countdown/59.73)>9)	//is the time remaining more than 10 seconds
					{
						
						layer1[(((10)+4) * 15) + 9] = (37+((int)(countdown/59.73)/10)%10) | (0 << 12);
						//draw first digit of timer value
						layer1[(((10)+4) * 15) + 10] = (38+(((int)(countdown/59.73))%10)) | (0 << 12);						
						//draw second digit of timer value
					}
					else
					{	
						layer1[(((10)+4) * 15) + 9] = (0) | (0 << 12);
						//draw blank where first digit is
						layer1[(((10)+4) * 15) + 10] = (38+(int)(countdown/59.73)) | (0 << 12);
						//draw single digit 
					}	
				}
				
				if (k != 0)//if a key is pressed
				{					
					geta= ((int)floor(a/8));	//convert oam x to tile x
					getb= ((int)floor(b/8));	//convert oam y to tile y
					tilex=(int)round((geta-5)/2);	//convert to i of maze array 
					tiley=(int)round((getb-1)/2);	//convert to i2 of maze array 
					if (k & KEY_LEFT) //if the left key is pressed
					{ 						
						
							if (a>1)	//if a is not on the leftside of the screen
							{								
								a -= movspeed; 	//decrement a														
							}
							if (maze[tilex][tiley].wall.left==false && (maze[tilex][tiley].x*8)>a)
							{	//check if there is a gap on the tile the oam is at 
								//if there isn't then check if the oam is intersecting with the tile
								a+=movspeed; //remove decrement of a 								
							}
							 horileft=true; //set oam to face the left
						
					}
					if (k & KEY_RIGHT) //check if right press is applied
					{
						if (a<231)	//check if oam is beyond right side of screen
						{
							a += movspeed;	//increment a											
						} 							
						if (maze[tilex][tiley].wall.right==false && (maze[tilex][tiley].x*8)+8<a)
						{//check if there is a gap on the tile the oam is at 
						//if there isn't then check if the oam is intersecting with the tile		
							a-=movspeed; //remove increment of a						
						}
						horileft=false;	//set oam to be facing right			
					}
					if (k & KEY_UP) //check if key up is pressed 
					{ 
						if (b>1)	//check the oam is not above the screen
						{
							b -= movspeed;	//decrement b							
						}
						if (maze[tilex][tiley].wall.up==false && (maze[tilex][tiley].y*8)>b)
						{//check if there is a gap on the tile the oam is at 
						//if there isn't then check if the oam is intersecting with the tile		
							b+=movspeed; //remove decrement of b							
						}
						vertdown=false; //have oam face upwards
					}
					if (k & KEY_DOWN) 
					{ 						
						if (b<151)
						{
							b += movspeed; //increment b						
						}
						if (maze[tilex][tiley].wall.down==false && (maze[tilex][tiley].y*8)+8<b)
						{//check if there is a gap on the tile the oam is at 
						//if there isn't then check if the oam is intersecting with the tile		
							b-=movspeed;//remove increment of b						
						}
						vertdown=true;//have oam face downwards
					}	
					int aa=a*0.1223628691983122;
					int bb=b*0.130718954248366;
					if ((aa>(targetx-1)) && (aa<(targetx+1)) && (bb>(targety-1)) && (bb<(targety+1)))
					{	//bounding box of oam with orange tree tile 
						
						b=550;	//move oam beyond screen
						a=550;
						
						layer0[((target->y) * 32) + target->x+1] = (0) | (0 << 12); //remove tree
						Gamestate=leveloutro;	//change state to level outro
						score+=1;
						if (score>5)	//check if player has done more than 5 levels 
						{
							Gamestate=menu;	//go back to menu
							option=0;	//set option back to 0
							score=0;	//set score back to 0 
						}
					}
					obj_oam[(1*4)]= ((b<< 0) | (0 << 8) | (0 << 9) | (0 << 14));
					obj_oam[(1 * 4)+1] = ((a << 0) | (horileft << 12) | (vertdown << 13) | (0 << 14));
					//re draw oam
					if (score>0)	//check score is more than 0
						layer1[(((2*3)+0) * 32) + 24] = (37+(((int)(score)))) | (0 << 12);
					else
						layer1[(((2*3)+0) * 32) + 24] = (37+(((int)(10)))) | (0 << 12);
					//draw score			
				}
			}break;
			case instructionspage:{
				
			}break;
			case highscorepage:{
				
			}break;
			case highscoreenter:{
				
			}break;
			case levelintro:{
								
			}break;
			case leveloutro:{	//transitioning between levels
				xscroll+=1;	//increment x scroll
				if (xscroll>128)	//check we haven't reached A full scroll
				{
					xscroll=0;	//reset scroll
					Gamestate=Maingame;	//go back to maze state
					a=40;
					b=12;	//reset oam position to top left corner
					obj_oam[(1*4)]= ((b<< 0) | (0 << 8) | (0 << 9) | (0 << 14));
					obj_oam[(1 * 4)+1] = ((a << 0) | (horileft << 12) | (vertdown << 13) | (0 << 14));						
					//draw oam
					countdown=22*59.73; //set timer 
				}
				*bg1xscroll=(xscroll<<1);	//scroll layer 1
				
			}break;
			case Quit:{	//if quit state
				
				char stringf[5]= {'c','l','o','s','e'};	
				drawtext(stringf,5,12,7);
				char stringg[6]= {'s','c','r','e','e','n'};	
				drawtext(stringg,6,12,10);	//draw close screen 			
			}break;
		}
		if (oldgamestate!=Gamestate)	//check that the state has changed from last frame
		{
			target=NULL;
			target=initialisegamestate(Gamestate);	//initalise new game state 
			if (target!=NULL)	//if target isn't null a new maze was generated
			{				 
				 targetx=target->x;
				 targety=target->y;//set new goal position				
			}
		}	
		mmFrame();	//sound 
		VBlankIntrWait();	//vblank
	}
}