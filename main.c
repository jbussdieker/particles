#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ESCAPE 27
#define UP    119
#define DOWN  115
#define LEFT  97
#define RIGHT 100

int window; 
float xrot, yrot, zrot;

GLfloat LightAmbient[]=  { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat LightDiffuse[]=  { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat LightPosition[]= { 2.0f, 2.0f, 5.0f, 1.0f };
GLfloat LightColor[]= { 0.5f, 0.5f, 0.5f };

const float PARTICLE_SIZE = 0.01f;
const int NUM_PARTICLES = 10000;
const int INITIAL_PARTICLE_SPREAD = 200;
const float SPEED_DECAY = 0.0000005f;

long Time1,Time2;
long Ticks = 1;

GLfloat distance;
GLfloat pitch;

typedef struct SpriteInfo {
	float xPos, yPos, zPos;
	float xVec, yVec, zVec;
	float r, g, b, life;
} SpriteInfo;

SpriteInfo Spr[10000];

void InitParticles() {
	int Index;
	for (Index = 0; Index != NUM_PARTICLES; Index++) {
		Spr[Index].life = 0.0f;
		Spr[Index].r = 1.0f;
		Spr[Index].b = 0.0f;
	}
}

void InitGL(int Width, int Height)
{
    glClearColor(0.13f, 0.13f, 0.13f, 0.0f);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
    glShadeModel(GL_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT1, GL_POSITION,LightPosition);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, LightColor);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);
    glMatrixMode(GL_MODELVIEW);

	InitParticles();
}

void ReSizeGLScene(int Width, int Height)
{
    if (Height==0)
	Height=1;
    glViewport(0, 0, Width, Height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,1000.0f);
    glMatrixMode(GL_MODELVIEW);
}

void CalcParticleMovement() {
	int Index;
	for (Index = 0; Index != NUM_PARTICLES; Index++) {
		if (Spr[Index].life > 0.0f) {
			Spr[Index].xPos += (Spr[Index].xVec * Ticks);
			Spr[Index].yPos += (Spr[Index].yVec * Ticks);
			Spr[Index].zPos += (Spr[Index].zVec * Ticks);
			Spr[Index].yVec -= (SPEED_DECAY * Ticks);

			if(Spr[Index].xPos > -10.0f && Spr[Index].xPos < 10.0f &&
				Spr[Index].zPos > -10.0f && Spr[Index].zPos < 10.0f) {
				if(Spr[Index].yPos < PARTICLE_SIZE) {
					Spr[Index].yPos = PARTICLE_SIZE;
					Spr[Index].life -= 0.01f;
					Spr[Index].yVec *= -0.6f;
				}
			}
			Spr[Index].life -= (0.0001f * Ticks);
		} else {
			Spr[Index].xPos = 0.0f;
			Spr[Index].yPos = PARTICLE_SIZE;
			Spr[Index].zPos = 0.0f;

			// Get a random spread and direction
			int MaxSpread = 5;
			float Spread=(float)(rand()%MaxSpread)/10000.0f;
			float Angle=(float)(rand()%157)/100.0f; // Quarter circle

			// Calculate X and Z vectors
			Spr[Index].xVec=cos(Angle)*Spread;
			Spr[Index].zVec=sin(Angle)*Spread;

			// Randomly reverse X and Z vector to complete the circle
			if(rand()%2)
				Spr[Index].xVec= - Spr[Index].xVec;
			if(rand()%2)
				Spr[Index].zVec= - Spr[Index].zVec;

			// Get a random initial speed
			Spr[Index].yVec=(float)(rand()%500)/10000.0f;

			// Get a random life and 'yellowness'
			Spr[Index].life=(float)(rand()%100)/100.0f;
			Spr[Index].g=0.2f+((float)(rand()%50)/100.0f);
		}
	}
}

void DrawParticles() {
	//glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);

	// Draw the particles
	int Index;
	int MaxParticles = 500;
	for(Index=0;Index!=MaxParticles;Index++) {
		glPushMatrix();

		// Place the quad and rotate to face the viewer
		glColor4f(Spr[Index].r,Spr[Index].g,Spr[Index].b,Spr[Index].life);
		glTranslatef(Spr[Index].xPos,Spr[Index].yPos,Spr[Index].zPos);
 
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f,0.0f); glVertex3f(-PARTICLE_SIZE, PARTICLE_SIZE,0.0f);
			glTexCoord2f(0.0f,1.0f); glVertex3f(-PARTICLE_SIZE,-PARTICLE_SIZE,0.0f);
			glTexCoord2f(1.0f,1.0f); glVertex3f( PARTICLE_SIZE,-PARTICLE_SIZE,0.0f);
			glTexCoord2f(1.0f,0.0f); glVertex3f( PARTICLE_SIZE, PARTICLE_SIZE,0.0f);
		glEnd();

		glPopMatrix();
	}

	glDepthMask(GL_TRUE);
}

void DrawGLScene()
{
	// Move
	CalcParticleMovement();

	// Draw
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

	// View
	glTranslatef(0.0f,0.0f,-5.0f+distance);
	glRotatef(-65.0f+pitch, 1.0f, 0.0f, 0.0f);

	// Scene
	DrawParticles();

    glutSwapBuffers();
}

void keyPressed(unsigned char key, int x, int y) 
{
    usleep(100);

    if (key == ESCAPE) { 
		glutDestroyWindow(window); 
		exit(0);                   
    }
    if (key == UP) { 
		distance -= 0.1f;
	}
    if (key == DOWN) { 
		distance += 0.1f;
	}
    if (key == LEFT) { 
		pitch -= 1.0f;
	}
    if (key == RIGHT) { 
		pitch += 1.0f;
	}
}

int main(int argc, char **argv) 
{  
    glutInit(&argc, argv);  
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);  
    glutInitWindowSize(1920, 1200);  
    glutInitWindowPosition(0, 0);  
    window = glutCreateWindow("Planets");  
    glutDisplayFunc(&DrawGLScene);  
    glutFullScreen();
    glutIdleFunc(&DrawGLScene);
    glutReshapeFunc(&ReSizeGLScene);
    glutKeyboardFunc(&keyPressed);
    InitGL(1920, 1200);
    glutMainLoop();  
    return 1;
}

