//-----------------------------------------------------------------------------
// Author: Michal Bochnak, mbochn2
// Class: CS 211, Programming Practicum
// Professor: Pat Troy
// Date: 02/20/2017
//-----------------------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

// global variable for debug mode on / off
int debugMode = FALSE;

// maze struct
typedef struct mazeStruct {
	char **arr;			// dynamic array to store maze board  
	int xsize, ysize;	// maze size
	int xstart, ystart;	// starting position
	int xend, yend;		// ending position
} maze;

// struct with solution coordinates
typedef struct nodeStruct {
	int x;				// x coordinate
	int y;				// y coordinate
	struct nodeStruct *pNext;	// pointer to the next node
} node;


//-----------------------------------------------------------------------------
// function declarations
//-----------------------------------------------------------------------------
void PrintAuthorInformation();
void DisplayMaze(maze m1);
void ReadMazeInfo(FILE *src, maze *m1);
void InitializeMaze(maze *m1);
void MarkBlockedPositions(FILE *src, maze *m1);
void Push(node **head, int x, int y);
void Pop(node **head);
void DisplayStack(node *head);
void DeallocateStack(node **head);
int InRange(int val, int lowBound, int highBound);
int UnvisitedNeighbor(maze m1, node *head);


int main(int argc, char **argv) {

	maze m1;				// struck with maze info 
	FILE *src;				// pointer to the input file
	node *head = NULL;		// head of the linked list stack
	int i = 0;				// loop counter
	int fileArgvIndex = 1;	// index of 'file name' argument from command line, 1 initially
	int endFound = 0;		// snding position found - 1, not found - 0


	PrintAuthorInformation();

	// verify the proper number of command line arguments were given
	if (argc == 1) {
		printf("Usage: %s <input file name> <optional \"-d\" for debugMode>\n", argv[0]);
		exit(-1);
	}
	else if (argc > 3) {		// too many arguments
		printf("Too many arguments were given. ");
		printf("Usage: %s <input file name> <\"-d\"optional for debugMode>\n", argv[0]);
		exit(-1);
	}
	else {						// search for '-d' flag in command line arguments
		for (i = 0; i < argc; i++) {
			if (strcmp(argv[i], "-d") == 0) {	// '-d' flag given
				debugMode = TRUE;				// set dubugMode to true
				if (i == 1)
					fileArgvIndex = 2;			// fileArgvIndex become 2
			}
		}
	}

	// Try to open the input file
	if ((src = fopen(argv[fileArgvIndex], "r")) == NULL) {	// open file failed
		printf("Can't open input file: %s\n", argv[1]);
		exit(-1);
	}

	ReadMazeInfo(src, &m1);

	// mallocate the memory for the maze
	m1.arr = (char **)malloc(sizeof(char *) * (m1.xsize + 2));
	for (i = 0; i < m1.xsize + 2; i++)
		m1.arr[i] = (char *)malloc(sizeof(char) * (m1.ysize + 2));

	// initialize the maze
	InitializeMaze(&m1);

	// mark the blocked positions in the maze with *'s
	MarkBlockedPositions(src, &m1);

	// close the inpout file
	fclose(src);

	// print out maze
	DisplayMaze(m1);

	// push start position at stack
	Push(&head, m1.xstart, m1.ystart);

	// mark starting position as visited
	m1.arr[m1.xstart][m1.ystart] = '*';


	// while loop try to find the solution by searching possible road
	// which is not blocked
	while ((head != NULL) && (!endFound)) {			// while stack not empty and end not found 
		if (head->x == m1.xend && head->y == m1.yend) {
			endFound = 1;		// end found
			break;
		}
		if (UnvisitedNeighbor(m1, head)) {		// possible move available
			if (m1.arr[(head->x) + 1][(head->y)] != '*') {		// right
				Push(&head, (head->x) + 1, head->y);			// push coordinates on the stack
				m1.arr[(head->x)][(head->y)] = '*';				// mark neighbor as visited	
			}
			else if (m1.arr[(head->x) - 1][(head->y)] != '*') {	// left
				Push(&head, (head->x) - 1, head->y);			// push coordinates on the stack
				m1.arr[(head->x)][(head->y)] = '*';				// mark neighbor as visited	
			}
			else if (m1.arr[(head->x)][(head->y) + 1] != '*') {	// down
				Push(&head, head->x, (head->y) + 1);			// push coordinates on the stack
				m1.arr[(head->x)][(head->y)] = '*';				// mark neighbor as visited	
			}
			else {												// up
				Push(&head, head->x, (head->y) - 1);			// push coordinates on the stack
				m1.arr[(head->x)][(head->y)] = '*';				// mark neighbor as visited	
			}
		}
		else {
			Pop(&head);			// all neighboors blocked, pop the last coordinates from stack
		}
	}

	if (head == NULL) {				// stack empty, no solution
		printf("Maze has no solution.\n");
	}
	else {							// solution found
		printf("Solution path: \n");
		DisplayStack(head);			// display stack with solution in reverse order
		DeallocateStack(&head);		// deallocate the memory for linked list stack
	}

	// deallocate the memory used for m1.arr
	for (i = 0; i < m1.xsize + 2; i++)
		free(m1.arr[i]);
	free(m1.arr);

	return 0;
} // end of main




  //-----------------------------------------------------------------------------
  // Functions definitions
  //-----------------------------------------------------------------------------

  // displays informations about the author
void PrintAuthorInformation() {
	printf("----------------------------------------------\n");
	printf("   Author: Michal Bochnak (mbochn2)\n");
	printf("   Class: CS 211, Programming Practicum\n");
	printf("   Professor: Pat Troy\n");
	printf("   Date: 02/06/2017\n");
	printf("----------------------------------------------\n\n");
}


// displays the maze board
void DisplayMaze(maze m1) {
	int i, j;

	// print out size, start, and end position
	printf("size: %d, %d\n", m1.xsize, m1.ysize);
	printf("start: %d, %d\n", m1.xstart, m1.ystart);
	printf("end: %d, %d\n", m1.xend, m1.yend);

	printf("\n");

	// display maze board
	for (i = 0; i < m1.xsize + 2; i++) {
		for (j = 0; j < m1.ysize + 2; j++)
			printf("%c", m1.arr[i][j]);
		printf("\n");
	}
	printf("\n");
}


// ----------------------------------------------------------------------------
// ReadMazeInfo: function with 3 helper functions, 
// verifies the correctness of input file values

// verifies if given size is valid size of maze,
// returns TRUE if valid, FALSE otherwise
int _ValidSize(maze m1) {
	if ((m1.xsize < 1) || (m1.ysize < 1))
		return FALSE;	// not valid
	else
		return TRUE;	// valid
}

// verifies if given starting position of maze is valid,
// returns TRUE if valid, FALSE otherwise
int _ValidStartPosition(maze m1) {
	if ((InRange(m1.xstart, 1, m1.xsize)) && (InRange(m1.ystart, 1, m1.ysize)))
		return TRUE;	// valid
	else
		return FALSE;	// not valid
}

// verifies if given ending position of maze is valid,
// returns TRUE if valid, FALSE otherwise
int _ValidEndPosition(maze m1) {
	if ((InRange(m1.xend, 1, m1.xsize)) && (InRange(m1.yend, 1, m1.ysize)))
		return TRUE;	// valid
	else
		return FALSE;	// not valid
}

// Read in maze info, verifies if input is correct
void ReadMazeInfo(FILE *src, maze *m1) {

	m1->xend = -1;			// check condition for correctness of input file

							// read in and verify size of maze
	fscanf(src, "%d %d", &m1->xsize, &m1->ysize);
	while ((!feof(src)) && (!(_ValidSize(*m1)))) {
		printf("%3d %3d     Invalid:  Maze sizes must be greater than 0\n", m1->xsize, m1->ysize);
		fscanf(src, "%d %d", &m1->xsize, &m1->ysize);
	}

	// read in and verify starting position
	fscanf(src, "%d %d", &m1->xstart, &m1->ystart);
	while ((!feof(src)) && (!(_ValidStartPosition(*m1)))) {
		if (!(InRange(m1->xstart, 1, m1->xsize))) {
			printf("%3d %3d     Invalid: row %d is outside of range from 1 to %d \n",
				m1->xstart, m1->ystart, m1->xstart, m1->xsize);
		}
		if (!(InRange(m1->ystart, 1, m1->ysize))) {
			printf("%3d %3d     Invalid: column %d is outside of range from 1 to %d \n",
				m1->xstart, m1->ystart, m1->ystart, m1->ysize);
		}
		fscanf(src, "%d %d", &m1->xstart, &m1->ystart);
	}

	// read in and verify ending position
	fscanf(src, "%d %d", &m1->xend, &m1->yend);
	while ((!feof(src)) && (!(_ValidEndPosition(*m1)))) {
		if (!(InRange(m1->xend, 1, m1->xsize))) {
			printf("%3d %3d     Invalid: row %d is outside of range from 1 to %d \n",
				m1->xend, m1->yend, m1->xend, m1->xsize);
		}
		if (!(InRange(m1->yend, 1, m1->ysize))) {
			printf("%3d %3d     Invalid: column %d is outside of range from 1 to %d \n",
				m1->xend, m1->yend, m1->yend, m1->ysize);
		}
		fscanf(src, "%d %d", &m1->xend, &m1->yend);
	}

	if (!(_ValidEndPosition(*m1)))
		m1->xend = -1;			// end position invalid, set back to -1

	if (m1->xend == -1) {		// not enough correct values in input file
		printf("Not Enough input values.\n");
		exit(-1);
	}
}
// ----------------------------------------------------------------------------


// Initializes maze to empty, surrounds the maze with border of asterisks ( * )
void InitializeMaze(maze *m1) {
	int i = 0, j = 0;		// loop counters

							// initialize to periods ( . )
	for (i = 0; i < m1->xsize + 2; i++)
		for (j = 0; j < m1->ysize + 2; j++)
			m1->arr[i][j] = '.';

	// mark the borders of the maze with *'s
	for (i = 0; i < m1->xsize + 2; i++) {
		m1->arr[i][0] = '*';
		m1->arr[i][m1->ysize + 1] = '*';
	}
	for (i = 0; i < m1->ysize + 2; i++) {
		m1->arr[0][i] = '*';
		m1->arr[m1->xsize + 1][i] = '*';
	}

	// mark the starting and ending positions in the maze
	m1->arr[m1->xstart][m1->ystart] = 's';
	m1->arr[m1->xend][m1->yend] = 'e';
}


// Marks blocked positions wits *'s
void MarkBlockedPositions(FILE *src, maze *m1) {
	int xpos, ypos;		// (x, y) coordinates of location in maze

	while (fscanf(src, "%d %d", &xpos, &ypos) != EOF) {		// while not end of the file
		if (m1->arr[xpos][ypos] == 's')			// starting position
			printf("%3d %3d     Invalid: attempting to block starting position\n", xpos, ypos);
		else if (m1->arr[xpos][ypos] == 'e')	// ending position
			printf("%3d %3d     Invalid: attempting to block ending position\n", xpos, ypos);
		else if ((!InRange(xpos, 1, m1->xsize)) || (!InRange(ypos, 1, m1->ysize))) {
			if (!(InRange(xpos, 1, m1->xsize))) {	// out of range
				printf("%3d %3d     Invalid: row %d is outside of range from 1 to %d \n",
					xpos, ypos, xpos, m1->xsize);
			}
			if (!(InRange(ypos, 1, m1->ysize))) {	// out of range
				printf("%3d %3d     Invalid: column %d is outside of range from 1 to %d \n",
					xpos, ypos, ypos, m1->ysize);
			}
		}
		else if (m1->arr[xpos][ypos] == '.') {		// wmpty location, mark with '*' as blocked
			if ((InRange(xpos, 1, m1->xsize)) && (InRange(ypos, 1, m1->ysize)))
				m1->arr[xpos][ypos] = '*';
		}
	}
}


// Push node onto stack ( prepend )
void Push(node **head, int x, int y) {

	// malloc the memory for new node
	node *temp = (node*)malloc(sizeof(node));

	if (debugMode)
		printf("Pushing: %d %d\n", x, y);

	if (*head == NULL) {	// empty stack
		temp->x = x;
		temp->y = y;
		temp->pNext = NULL;
		*head = temp;
		return;
	}
	else {					// stack not empty, prepend
		temp->x = x;
		temp->y = y;
		temp->pNext = *head;
		*head = temp;
	}
}


// Pop node onto stack
void Pop(node **head) {
	node *temp = *head;		// local copy of head

	if (debugMode)
		printf("Poping: %d %d\n", temp->x, temp->y);

	if (temp != NULL) {		// remove node, pop from stack
		*head = temp->pNext;
		free(temp);
	}
}


// recursively displays stack
void DisplayStack(node *head) {

	if (head == NULL)
		return;
	else {
		DisplayStack(head->pNext);
		printf("%3d %3d\n", head->x, head->y);
	}
}


// deallocates the space taken by stack of linked lists
void DeallocateStack(node **head) {
	node *prev = NULL;		// pointer to previous node
	node *cur = *head;		// pointer to current node

	while (cur != NULL) {
		prev = cur;
		cur = cur->pNext;
		free(prev);			// deallocate the memory for prev node
	}
}


// verifies if given value is in the given range,
// returns TRUE - value on range, FALSE - value not in range
int InRange(int val, int lowBound, int highBound) {
	if ((val >= lowBound) && (val <= highBound))
		return TRUE;		// in range
	else
		return FALSE;		// out of range
}


// checks if invisited neighbor exist,
// returns TRUE if exist, FALSE otherwise
int UnvisitedNeighbor(maze m1, node *head) {

	if (m1.arr[(head->x) + 1][(head->y)] != '*')
		return TRUE;	// unvisited neighboor exists
	else if (m1.arr[(head->x) - 1][(head->y)] != '*')
		return TRUE;	// unvisited neighboor exists
	else if (m1.arr[(head->x)][(head->y) + 1] != '*')
		return TRUE;	// unvisited neighboor exists
	else if (m1.arr[(head->x)][(head->y) - 1] != '*')
		return TRUE;	// unvisited neighboor exists

	return FALSE;		// unvisited neighbor does not exist
}