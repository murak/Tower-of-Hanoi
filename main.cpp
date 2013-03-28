/*
									6th sem VTU OpenGL Project - TOWER OF HANOI
Author: Murali K
Date: 24 Feb 2013

 */


/* Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


//Include Files
#include <iostream>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>	//To draw cilinder and disks
#include <math.h>
#include<string.h>
#include "imageloader.h"	//to load texture image


using namespace std;

//Constants
#define MAX	50			//Stack size
#define DELAY 0.01		//Delay in timer
#define TRUE 1
#define FALSE 0
int kr;

//Most usable variables
float DISK_INNER_RADIUS = 0.1;		//Inner radius of the disk
float HEIGHT_OF_DISK = 0.15;		//Thickness of the disk
float POLE_BASE = -1;				//Base position of pole (y axis)
float POLE_RADIUS = 0.04 ;			//Thickness of the pole
float POLE_INTER_SPACE = 2;			//Space between adjacent poles
float SPEED = 0.0002,sa_speed=0.9;					//Speed of disk motion
int NO_OF_DISKS = 3;				//Number of disks taken as input from user
float POLE_HEIGHT = 2 + NO_OF_DISKS * HEIGHT_OF_DISK ;		//Height of the pole;
int pop_color;						//index to colour array
//maintains the colour of the poped disk from stack

float color[8][3] =	{{1.0,0.0,1.0},{0.0,1.0,1.0},		//colour table with 8 different colours
		{0.25,0.25,0.25},{0.75,0.75,0.75},
		{0.60,0.40,0.12},{0.98,0.625,0.12},
		{0.98,0.04,0.7},{0.60,0.40,0.70}};
int ORDER = -1;


//different states in animation
int MOVE_UP = 1;			//Move up
int MOVE_DOWN = 0;			//move down
int MOVE_SIDE = 0;			//move left or right
int CHANGE_STATE = 0;		//change the stack content and update
float x_temp = 0.0;			//relative temporary x position of disk from popped stack

//Switches to turn on and off various features in opengl
bool COLOR = TRUE;			//on the color
bool LIGHT = TRUE;			//on lights
bool TEXTURE = TRUE;		//on texture
bool LIGHT0 = TRUE;			//on light for pole0 --> left most
bool LIGHT1 = TRUE;			//on light for pole1 --> middle one
bool LIGHT2 = TRUE;			//on light for pole2 --> right most
bool FULL_SCREEN = FALSE;	//off full screen
bool RESTART = FALSE;
bool EXIT = FALSE;

//To rotate whole sceen
static GLfloat xRot = 0.0f;
static GLfloat yRot = 0.0f;

//To zoom the view
float ZOOM = 0.0;

//sign board
char str1[] = "MOVE:";
char str2[] = "FROM ";
char str3[] = "TO ";
char str4[] = "Problem Solved";

//Link list to record the movement of disk between stacks
//Initially movement of disks are recorded in this list and then they
//are used to change the stack content and there by display the sceen
struct Record
{
	char src;
	char dest;
	struct Record *next;
};
struct Record *list = NULL;	//actual list
struct Record *t = NULL;	//temperory pointer to traverse the list


//stack used to help display the scene on the screen
//3 stacks are maintained which represent the 3 poles
//the content of stack is modified looking into the linked list each time
//and after eack modification the stack content is read and disks are displayed

struct Stack
{
	char ch;					//stack identifier
	int top;					//stack top pointer
	float size[MAX];			//array containing size of eack disk
	int color_index[MAX];		//array containing index to color of corresponding disks
	float x_pos;				//posiotion of stack (pole)
	float y_pos[MAX];			//position of disk in the stack (y position)
};
struct Stack from, to, aux;		//3 stacks from(A), to(C), aux(B)
struct Stack *p1;				//pointer to stack from which disk must be popped
struct Stack *p2;				//pointer to stack into which disk must be pushed

GLUquadricObj *obj;		//glu object to be drawn
GLuint _textureId1;		//The id of the texture on ground
GLuint _textureId2;		//The id of the texture on the back wall
GLuint _textureId3;
GLuint _textureId4;

//Light colours
GLfloat RED = 0.6;
GLfloat GREEN = 0.3;
GLfloat BLUE = 0.3;

//lights
GLfloat AmbientLight[] = {0.5, 0.5, 0.5, 1.0};
GLfloat SpecularLight[] = {0.5, 0.5, 0.5, 1.0};
GLfloat SpecularRef[] = {1.0, 1.0, 1.0, 1.0};
/*
GLfloat _LightColor0[] = {0.5, 0.5, 0.5, 1.0};	//light for pole on left
GLfloat _LightColor1[] = {1.0, 1.0, 1.0, 1.0};	//light for pole in the middle
GLfloat _LightColor2[] = {0.3, 0.6, 0.3, 1.0};	//light for pole on right
 */

GLfloat _LightColor0[] = {0.6, 0.3, 0.3, 1.0};	//light for pole on left
GLfloat _LightColor1[] = {0.3, 0.6, 0.3, 1.0};	//light for pole in the middle
GLfloat _LightColor2[] = {0.3, 0.3, 0.6, 1.0};	//light for pole on right


GLfloat _LightPosition0[] = {0.0, 0.0, 3.0, 1.0};	//position of light0
GLfloat _LightPosition1[] = {-20.0, 0.0, 3.0, 1.0};	//position of light1
GLfloat _LightPosition2[] = {20.0, 0.0, 3.0, 1.0};	//position of light2


//set the pointers to the stacks from which
// we need to pop and to which we need to push the disk
void SetPointers(char a, char b)
{
	//find the stack fron which to pop and assign to p1
	if(a == from.ch)
		p1 = &from;
	else if(a == to.ch)
		p1 = &to;
	else if(a == aux.ch)
		p1 = &aux;
	//FIND THE STACK TO WHICH TO PUSH AND ASSIGN TO P2
	if(b == from.ch)
		p2 = &from;
	else if(b == to.ch)
		p2 = &to;
	else if(b == aux.ch)
		p2 = &aux;
}

//pop the disk
float pop(struct Stack *p)
{
	if(p->top == 0)
	{
		cout << "\nSTACK UNDERFLOW\n";
		exit(10);
	}
	float temp = p->size[p->top];		//pop size of disk
	pop_color = p->color_index[p->top];	//pop color of disk
	p->top--;
	return temp;						//return size locally and colo globally
}

//push the disk
void push(struct Stack *p, float x)
{
	if(p->top == MAX-1)
	{
		cout << "\nSTACK OVERFLOW\n";
		exit(11);
	}
	p->top++;
	p->size[p->top] = x;						//push disk size
	p->color_index[p->top] = pop_color;			//push disk color
	p->y_pos[p->top] = HEIGHT_OF_DISK * p->top;	//st the position of disk in that stack (y position)
}

//to change the stack content by looking into the list
//this is used just to debug : just can be ommited
void ManipStack()
{
	struct Record *t = list;
	while(t != NULL)
	{
		SetPointers(t->src, t->dest);
		push(p2, pop(p1));
		t = t->next;
	}
}

//create a node in list to record each movement of the disk
void TakeAction(char a, char b)
{
	struct Record *temp = NULL;
	temp = new struct Record;

	temp->next = NULL;
	temp->src = a;
	temp->dest = b;

	if(list == NULL)
		list = temp;
	else
	{
		struct Record *p = list;
		while(p->next != NULL)
			p = p->next;
		p->next = temp;
	}
}

//Initialise the stack variables
void InitialiseStack()
{
	//set stack tops
	from.top = 0;
	to.top = 0;
	aux.top = 0;

	//set identifiers for each stack
	from.ch = 'A';
	to.ch = 'C';
	aux.ch = 'B';

	//set x positions of eack stack
	from.x_pos = -POLE_INTER_SPACE;
	to.x_pos = POLE_INTER_SPACE;
	aux.x_pos = 0.0;

	//no disk in 0th positon of stack hence make it zero
	from.y_pos[0] = 0;
	aux.y_pos[0] = 0;
	to.y_pos[0] = 0;

	//compute disk size depending on number of disks
	float size = DISK_INNER_RADIUS + NO_OF_DISKS * 0.05;
	//put the disks into the from stack initially
	for(int i = 1; i <= NO_OF_DISKS; i++)
	{
		pop_color = i % 8;							//set color of disk
		from.y_pos[i] = HEIGHT_OF_DISK * (i-1);		//set y position of disk
		push(&from,size-i*0.05);					//place it into the stack
	}

	//just to display size: ONLY FOR VERIFICATION
	for(int i = from.top; i>=1; i--)
		cout<<"\n"<<from.size[i];
}

//Tower of hanoi algorithm
void Hanoi(int n,char from, char to, char aux)
{
	if(n == 1)
	{
		TakeAction(from, to);
		return;
	}
	Hanoi(n-1, from, aux, to);
	TakeAction(from, to);
	Hanoi(n-1, aux, to, from);
}

//for debugging purpose
//displays the list contents i.e step by step movement of disks
void DisplayListContent()
{
	int kr=0;
	for(struct Record *temp = list; temp != NULL; temp = temp->next)
	{
		kr++;
		cout << "\n" << temp->src << "\t" << temp->dest;
	}
	cout<<"\nnumber of movements"<<kr;
}
////////////////////////////////////////////////////////////////////////////////////

//function to load the texture
GLuint loadTexture(Image* image)
{
	GLuint textureId;
	//Make room for our texture
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);	//Tell OpenGL which texture to edit

	//Map the image to the texture
	glTexImage2D(GL_TEXTURE_2D,					//Always GL_TEXTURE_2D
			0,								//0 for now
			GL_RGB,						//Format OpenGL uses for image
			image->width, image->height,	//Width and height
			0,								//The border of the image
			GL_RGB,						//GL_RGB, because pixels are stored in RGB format
			GL_UNSIGNED_BYTE,				//GL_UNSIGNED_BYTE, because pixels are stored
			//as unsigned numbers
			image->pixels);				//The actual pixel data
	return textureId;							//Returns the id of the texture
}

//Initialise the openGL states
void init(void)
{
	glClearColor (0.0, 0.0, 0.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glShadeModel (GL_SMOOTH);

	//load image for ground
	Image* image = loadBMP("ground.bmp");
	_textureId1 = loadTexture(image);
	delete image;

	//load image for back wall
	image = loadBMP("backwall.bmp");
	_textureId2 = loadTexture(image);
	delete image;

	image = loadBMP("brick.bmp");
	_textureId3 = loadTexture(image);
	delete image;

}

void TimerFunction(int value)
{

	/*	if(EXIT)
	{
		ORDER = (ORDER+1)%6;

	}
	else
	 */{

		 if(t == NULL)
			 return;

		 SetPointers(t->src, t->dest);


		 if(MOVE_UP && p1->y_pos[p1->top] <= POLE_HEIGHT)
		 {
			 p1->y_pos[p1->top] = p1->y_pos[p1->top] + SPEED;
			 if(p1->y_pos[p1->top] > POLE_HEIGHT)
			 {
				 MOVE_UP = 0;
				 MOVE_SIDE = 1;
			 }
		 }

		 if(MOVE_SIDE && p1->x_pos > p2->x_pos)
		 {
			 x_temp = x_temp - SPEED;
			 if(p2->x_pos >= p1->x_pos + x_temp)
			 {
				 MOVE_SIDE = 0;
				 MOVE_DOWN = 1;
			 }
		 }
		 else if(MOVE_SIDE && p1->x_pos < p2->x_pos)
		 {
			 x_temp = x_temp + SPEED;
			 if(p2->x_pos <= p1->x_pos + x_temp)
			 {
				 MOVE_SIDE = 0;
				 MOVE_DOWN = 1;
			 }
		 }

		 if(MOVE_DOWN && p1->y_pos[p1->top] > p2->y_pos[p2->top])
		 {
			 p1->y_pos[p1->top] = p1->y_pos[p1->top] - SPEED;
			 if(p1->y_pos[p1->top] <= p2->y_pos[p2->top])
			 {
				 MOVE_DOWN = 0;
				 CHANGE_STATE = 1;
			 }
		 }

		 if(CHANGE_STATE )
		 {
			 push(p2, pop(p1));
			 MOVE_UP = 1;
			 CHANGE_STATE = 0;
			 x_temp = 0.0;
			 t = t->next;
		 }
	 }
	 glutPostRedisplay();
	 glutTimerFunc(DELAY,TimerFunction, 1);

}

void SignBoard(void)
{
	if(t != NULL)
	{
		glRasterPos3f(-4.5, -3.2, 1.0);
		for(unsigned int i = 0;i<strlen(str1); i++)
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,str1[i]);


		glRasterPos3f(-4.5, -3.6, 1.0);
		for(int i = 0;i<strlen(str2); i++)
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,str2[i]);


		glRasterPos3f(-2.5, -3.6, 1.0);
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,t->src);

		glRasterPos3f(-4.5, -4.0, 1.0);
		for(int i = 0;i<strlen(str3); i++)
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,str3[i]);

		glRasterPos3f(-2.5, -4.0, 1.0);
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,t->dest);
	}
	else
	{
		glRasterPos3f(-4.5, -3.6, 1.0);
		for(unsigned int i = 0;i<strlen(str4); i++)
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,str4[i]);


	}
}

void display(void)
{


	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(COLOR)	glEnable(GL_COLOR_MATERIAL);
	else	glDisable(GL_COLOR_MATERIAL);

	if(LIGHT)	glEnable(GL_LIGHTING);
	else	glDisable(GL_LIGHTING);

	if(LIGHT0)	glEnable(GL_LIGHT0);
	else	glDisable(GL_LIGHT0);

	if(LIGHT1)	glEnable(GL_LIGHT1);
	else	glDisable(GL_LIGHT1);

	if(LIGHT2)	glEnable(GL_LIGHT2);
	else	glDisable(GL_LIGHT2);

	if(TEXTURE)	glEnable(GL_TEXTURE_2D);
	else	glDisable(GL_TEXTURE_2D);

	_LightColor0[0] = RED;
	_LightColor0[1] = GREEN;
	_LightColor0[2] = BLUE;

	_LightColor1[0] = RED;
	_LightColor1[1] = GREEN;
	_LightColor1[2] = BLUE;

	_LightColor2[0] = RED;
	_LightColor2[1] = GREEN;
	_LightColor2[2] = BLUE;


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, AmbientLight);

	glLightfv(GL_LIGHT0, GL_DIFFUSE, _LightColor0);
	glLightfv(GL_LIGHT0, GL_POSITION, _LightPosition0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, _LightColor0);

	glLightfv(GL_LIGHT1, GL_DIFFUSE, _LightColor1);
	glLightfv(GL_LIGHT0, GL_SPECULAR, _LightColor1);
	glLightfv(GL_LIGHT1, GL_POSITION, _LightPosition1);

	glLightfv(GL_LIGHT2, GL_DIFFUSE, _LightColor2);
	glLightfv(GL_LIGHT0, GL_SPECULAR, _LightColor2);
	glLightfv(GL_LIGHT2, GL_POSITION, _LightPosition2);

	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv(GL_FRONT, GL_SPECULAR, SpecularRef);
	glMateriali(GL_FRONT,GL_SHININESS,128);


	glBindTexture(GL_TEXTURE_2D, _textureId1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	glPushMatrix();
	glScalef(1+ZOOM, 1+ZOOM,  1+ZOOM);
	glRotatef(xRot,1.0,0.0,0.0);
	glRotatef(yRot,0.0,1.0,0.0);



	if(!EXIT)
	{

		glPushMatrix();
		{
			//ground or floor
			glBegin(GL_POLYGON);
			{
				glColor3f(1.0,1.0,1.0);

				glTexCoord2f(0.0f, 1.0f);
				glVertex3f(-3.5,-1.0,-2.0);

				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(-3.5,-1.0,1.0);

				glTexCoord2f(1.0f, 0.0f);
				glVertex3f(3.5,-1.0,1.0);

				glTexCoord2f(1.0f, 1.0f);
				glVertex3f(3.5,-1.0,-2.0);
			}
			glEnd();

			glBegin(GL_POLYGON);
			{
				//left
				glVertex3f(-3.5,-1.0,-2.0);
				glVertex3f(-3.5,-1.2,-2.0);
				glVertex3f(-3.5,-1.2,1.0);
				glVertex3f(-3.5,-1.0,1.0);
			}glEnd();

			glBegin(GL_POLYGON);
			{
				//right
				glVertex3f(3.5,-1.0,-2.0);
				glVertex3f(3.5,-1.2,-2.0);
				glVertex3f(3.5,-1.2,1.0);
				glVertex3f(3.5,-1.0,1.0);
			}
			glEnd();


			glBegin(GL_POLYGON);
			{
				//front
				glColor3f(0.98,0.625,0.12);
				glVertex3f(-3.5,-1.0,1.0);
				glVertex3f(-3.5,-1.2,1.0);
				glVertex3f(3.5,-1.2,1.0);
				glVertex3f(3.5,-1.0,1.0);
				glColor3f(1.0,1.0,1.0);
			}
			glEnd();

			glBegin(GL_POLYGON);
			{
				//back
				glVertex3f(-3.5,-1.0,-2.0);
				glVertex3f(-3.5,-1.2,-2.0);
				glVertex3f(3.5,-1.2,-2.0);
				glVertex3f(3.5,-1.0,-2.0);
			}
			glEnd();


			glBindTexture(GL_TEXTURE_2D, _textureId1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			//FLOOR SECOND STEP
			glBegin(GL_POLYGON);
			{
				glTexCoord2f(0.0f, 1.0f);
				glVertex3f(-3.5,-1.2,-2.0);

				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(-3.5,-1.2,1.5);

				glTexCoord2f(1.0f, 0.0f);
				glVertex3f(3.5,-1.2,1.5);

				glTexCoord2f(1.0f, 1.0f);
				glVertex3f(3.5,-1.2,-2.0);
			}
			glEnd();


			glBindTexture(GL_TEXTURE_2D, _textureId1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


			//third step ground
			glBegin(GL_POLYGON);
			{
				glTexCoord2f(0.0f, 1.0f);
				glVertex3f(-3.5,-1.4,-2.0);

				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(-3.5,-1.4,2.0);

				glTexCoord2f(1.0f, 0.0f);
				glVertex3f(3.5,-1.4,2.0);

				glTexCoord2f(1.0f, 1.0f);
				glVertex3f(3.5,-1.4,-2.0);
			}
			glEnd();


			//back wall
			glBindTexture(GL_TEXTURE_2D, _textureId2);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


			glBegin(GL_POLYGON);
			{
				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(-3.5,-1.0,-2.0);

				glTexCoord2f(1.0f, 0.0f);
				glVertex3f(3.5,-1.0,-2.0);

				glTexCoord2f(1.0f, 1.0f);
				glVertex3f(3.5,3.0,-2.0);

				glTexCoord2f(0.0f, 1.0f);
				glVertex3f(-3.5,3.0,-2.0);

			}
			glEnd();

			glBegin(GL_POLYGON);
			{
				//front
				glColor3f(0.98,0.625,0.12);
				glVertex3f(-3.5,-1.2,1.5);
				glVertex3f(-3.5,-1.4,1.5);
				glVertex3f(3.5,-1.4,1.5);
				glVertex3f(3.5,-1.2,1.5);
				glColor3f(1.0,1.0,1.0);
			}
			glEnd();

		}
		glPopMatrix();

		glColor3f (.42, .0, .35);
		glPushMatrix();
		{
			//Display First cone
			glTranslatef(-POLE_INTER_SPACE, POLE_BASE, 0.0);
			glPushMatrix();
			{
				glRotatef(-90,1,0,0);
				gluCylinder(obj, POLE_RADIUS, POLE_RADIUS, POLE_HEIGHT-0.05, 20, 1);
			}
			glPopMatrix();

			//Display second cone
			glTranslatef(POLE_INTER_SPACE,0,0);
			glPushMatrix();
			{
				glRotatef(-90,1,0,0);
				gluCylinder(obj, POLE_RADIUS, POLE_RADIUS, POLE_HEIGHT-0.05, 20, 1);
			}
			glPopMatrix();

			//Display third cone
			glTranslatef(POLE_INTER_SPACE,0,0);
			glPushMatrix();
			{
				glRotatef(-90,1,0,0);
				gluCylinder(obj, POLE_RADIUS, POLE_RADIUS, POLE_HEIGHT-0.05, 20, 1);
			}
			glPopMatrix();
		}
		glPopMatrix();

		//Display Stack contents
		glPushMatrix();
		{
			glTranslatef(0.0, POLE_BASE, 0.0);
			for(int i=1; i<=from.top; i++)
			{
				glPushMatrix();
				{
					if(i == from.top && p1 == &from)
						glTranslatef(from.x_pos + x_temp, from.y_pos[i], 0);
					else
						glTranslatef(from.x_pos, from.y_pos[i], 0);
					glRotatef(85, 1, 0, 0);
					glColor3fv(color[from.color_index[i]]);
					glutSolidTorus(DISK_INNER_RADIUS, from.size[i], 16, 20);
				}
				glPopMatrix();
			}

			for(int i=1; i<=aux.top; i++)
			{
				glPushMatrix();
				{
					if(i == aux.top && p1 == &aux)
						glTranslatef(aux.x_pos + x_temp, aux.y_pos[i], 0);
					else
						glTranslatef(aux.x_pos, aux.y_pos[i], 0);
					glRotatef(85, 1, 0, 0);
					glColor3fv(color[aux.color_index[i]]);
					glutSolidTorus(DISK_INNER_RADIUS, aux.size[i], 16, 20);
				}
				glPopMatrix();
			}

			for(int i=1; i<=to.top; i++)
			{
				glPushMatrix();
				{
					if(i == to.top && p1 == &to)
						glTranslatef(to.x_pos + x_temp, to.y_pos[i], 0);
					else
						glTranslatef(to.x_pos, to.y_pos[i], 0);
					glRotatef(85, 1, 0, 0);
					glColor3fv(color[to.color_index[i]]);
					glutSolidTorus(DISK_INNER_RADIUS, to.size[i], 16, 20);
				}
				glPopMatrix();
			}


		}
		glPopMatrix();

		glPopMatrix();
		SignBoard();

	}
	else
	{
		glClearColor(0.0,0.0,0.0,1.0);;
		glColor3f(1.0,0.0,0.0);
		glRectf(-5.9f, 1.9f, -4.f, 2.4f);//"-"
		glColor3f(0.0,1.0,0.0);
		glRectf(-5.25f,- 2.4f,-4.75f,1.9f);//"|"

		glColor3f(0.0,0.0,1.0);
		glRectf(-3.9f, -2.4f, -3.4f, 2.4f);//"|"
		glColor3f(1.0,1.0,0.0);
		glRectf(-3.4f,- 0.25f,-2.5f,0.25f);//"-"
		glColor3f(0.0,1.0,1.0);
		glRectf(-2.5f,- 2.4f,-2.0f,2.4f);//" |"

		glColor3f(0.0,0.0,1.0);
		glRectf(-1.9f,- 2.4f,-1.4f,2.4f);//"|"
		glColor3f(1.0,0.0,0.0);

		glRectf(-1.4f,1.9f,-0.5f,2.4f);//"-"
		glColor3f(1.0,0.0,0.0);
		glRectf(-0.5f,-2.4f,0.0f,2.4f);//"-"
		glColor3f(1.0,0.0,0.0);
		glRectf(-1.4f,- 0.25f,-0.5f,0.25f);//" \"

		glColor3f(0.0,1.0,1.0);
		glRectf(0.1f,- 2.4f,0.6f,2.4f);//"|"
		glColor3f(1.0,0.0,0.0);
		glRectf(0.6f,1.9f,1.4f,2.4f);//"-"
		glColor3f(1.0,0.0,0.0);
		glRectf(1.4f,- 2.4f,1.9f,2.4f);//" |"




		glRectf(2.0f,-2.4f,2.5f,2.4f);
		glColor3f(1.0,0.0,0.0);
		glBegin(GL_POLYGON);
		{
			//"/"
			glVertex2f( 2.5f,0.25f);
			glColor3f(1.0,0.0,0.0);
			glVertex2f(3.9f, 2.4f);
			glColor3f(1.0,0.0,0.0);
			glVertex2f(3.9f,1.7f);
			glColor3f(1.0,0.0,0.0);
			glVertex2f(2.5f,-0.4f);
			//"\"
			glColor3f(1.0,0.0,0.0);
			glVertex2f(2.5f,- 0.4f);
			glColor3f(1.0,0.0,0.0);
			glVertex2f(3.9f,- 2.4f);
			glColor3f(1.0,0.0,0.0);
			glVertex2f(3.9f,- 1.7f);
			glColor3f(1.0,0.0,0.0);
			glVertex2f(2.5f,0.25f);
			glColor3f(1.0,0.0,0.0);
		}

		glEnd();


		glRectf(4.3f,-2.4f,4.8f,2.4f);//"|"
		glColor3f(1.0,0.0,0.0);
		glRectf(4.8f,- 2.4f,5.4f,-1.9f);//"_"
		glColor3f(1.0,0.0,0.0);
		glRectf(5.4f,- 2.4f,5.9f,2.4f);//" |"
		glColor3f(1.0,0.0,0.0);




	}



	glutSwapBuffers();
}

void reshape (int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective(60.0, (GLfloat) w/(GLfloat) h, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt (0.0, 0.0, 8.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void Menu(int value)
{
	if(value == 1) LIGHT = !LIGHT;
	if(value == 2) COLOR = !COLOR;
	if(value == 3) TEXTURE = !TEXTURE;
	if(value == 4) LIGHT0 = !LIGHT0;
	if(value == 5) LIGHT1 = !LIGHT1;
	if(value == 6) LIGHT2 = !LIGHT2;
	if(value == 7) glutFullScreen();
	if(value == 8)
	{
		glutReshapeWindow(500,500);
		glutPositionWindow(100,100);
	}
	if(value == 9) EXIT = 1;
	if(value == 10)
	{
		InitialiseStack();
		t = list;
		glutTimerFunc(DELAY,TimerFunction, 1);
	}
	// Trigger Redraw
	glutPostRedisplay();
}

void SpecialKeys(int key, int x, int y)
{
	switch(key)
	{
	case GLUT_KEY_UP:	xRot-= 5.0f;
	break;
	case GLUT_KEY_DOWN: xRot += 5.0f;
	break;
	case GLUT_KEY_LEFT: yRot += 5.0f;
	break;
	case GLUT_KEY_RIGHT:yRot -= 5.0f;
	break;
	}

	if(key > 356.0f)
		xRot = 0.0f;

	if(key < -1.0f)
		xRot = 355.0f;

	if(key > 356.0f)
		yRot = 0.0f;

	if(key < -1.0f)
		yRot = 355.0f;

	glutPostRedisplay();
}

float save_speed = SPEED;
void MyKeys(unsigned char key, int x, int y)
{
	if(key == 's')
		SPEED = SPEED + 0.0001;
	if(key == 'S')
	{
		if(SPEED >= 0)
			SPEED = SPEED - 0.0001;
	}
	if(key == 'x')
	{
		if(SPEED != 0)
			save_speed = SPEED;
		SPEED = 0;
	}
	if(key == 'X')
		SPEED = save_speed;

	if(key == 'z')
		ZOOM = ZOOM + 0.01;

	if(key == 'Z')
		ZOOM = ZOOM - 0.01;

	if(key == 'q')
		exit(0);
	if(key == '4')
	{
		RED = RED +.1;
		if(RED > 1.0)
			RED = 1.0;
	}
	if(key == '1')
	{
		RED = RED - 0.01;
		if(RED <0.0)
			RED = 0.0;
	}
	if(key == '5')
	{
		GREEN = GREEN +0.01;
		if(GREEN > 1.0)
			GREEN = 1.0;
	}
	if(key == '2')
	{
		GREEN = GREEN -0.01;
		if(GREEN < 0.0)
			GREEN = 0.0;
	}
	if(key == '6')
	{
		BLUE = BLUE +0.01;
		if(BLUE > 1.0)
			BLUE = 1.0;
	}
	if(key == '3')
	{
		BLUE = BLUE -0.01;
		if(BLUE < 0.0)
			BLUE = 0.0;
	}
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	cout << "Enter the number of disk\t";
	cin >> NO_OF_DISKS;

	InitialiseStack();

	obj = gluNewQuadric();

	Hanoi(NO_OF_DISKS,'A','C','B');
	DisplayListContent();
	t = list;

	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize (800, 600);
	glutInitWindowPosition (100, 100);

	glutCreateWindow (argv[0]);
	init ();

	glutCreateMenu(Menu);
	glutAddMenuEntry("Light",1);
	glutAddMenuEntry("Colour", 2);
	glutAddMenuEntry("Texture", 3);
	glutAddMenuEntry("light1", 4);
	glutAddMenuEntry("light2", 5);
	glutAddMenuEntry("light3", 6);
	glutAddMenuEntry("Full Screen", 7);
	glutAddMenuEntry("Exit Full Screen", 8);
	glutAddMenuEntry("Quit", 9);
	glutAddMenuEntry("Restart", 10);
	glutAddMenuEntry("Change Back Texture", 10);

	glutAttachMenu(GLUT_RIGHT_BUTTON/**/);

	glutSpecialFunc(SpecialKeys);
	glutKeyboardFunc(MyKeys);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutTimerFunc(3000, TimerFunction, 1);

	glutMainLoop();
	return 0;
}



