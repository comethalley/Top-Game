#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include "latto.cpp"
#include "target.cpp"
#include "imageloader.h"
#include "vec3f.h"
//#define print printf("test\n");
using namespace std;

#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)

Latto lt;
Target tr;

Vec3f g(0.0, -1.0, 0.0);
float dott;
Vec3f gt;

int sp = 1;
int surface = 0;
int tops = 0;
int kiran = 0;
int my;

//Represents a terrain, by storing a set of heights and normals at 2D locations
class Terrain {
	private:
		int w; //Width
		int l; //Length
		float** hs; //Heights
		Vec3f** normals;
		bool computedNormals; //Whether normals is up-to-date
	public:
		Terrain(int w2, int l2) {
			w = w2;
			l = l2;

			hs = new float*[l];
			for(int i = 0; i < l; i++) {
				hs[i] = new float[w];
			}
			if(surface==0)
			{
				tops=tops+2;
				kiran=0;
			}
			normals = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals[i] = new Vec3f[w];
			}

			computedNormals = false;
		}

		~Terrain() {
			for(int i = 0; i < l; i++) {
				delete[] hs[i];
			}
			delete[] hs;

			for(int i = 0; i < l; i++) {
				delete[] normals[i];
			}
			delete[] normals;
		}

		int width() {
			return w;
		}

		int length() {
			return l;
		}

		//Sets the height at (x, z) to y
		void setHeight(int x, int z, float y) {
			hs[z][x] = y;
			computedNormals = false;
		}

		//Returns the height at (x, z)
		float getHeight(int x, int z) {
			return hs[z][x];
		}

		//Computes the normals, if they haven't been computed yet
		void computeNormals() {
			if (computedNormals) {
				return;
			}
			if(surface==0)
			{
				tops=tops+2;
				kiran=0;
			}
			//Compute the rough version of the normals
			Vec3f** normals2 = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals2[i] = new Vec3f[w];
			}

			for(int z = 0; z < l; z++) {
				for(my=0;my<100;my++)
				{
					if(kiran==1)
					{
						tops++;
					}
					else
					{
						tops = 0;
					}
				}
				for(int x = 0; x < w; x++) {
					Vec3f sum(0.0f, 0.0f, 0.0f);

					Vec3f out;
					if (z > 0) {
						out = Vec3f(0.0f, hs[z - 1][x] - hs[z][x], -1.0f);
					}
					Vec3f in;
					if (z < l - 1) {
						in = Vec3f(0.0f, hs[z + 1][x] - hs[z][x], 1.0f);
					}
					Vec3f left;
					if (x > 0) {
						left = Vec3f(-1.0f, hs[z][x - 1] - hs[z][x], 0.0f);
					}
					Vec3f right;
					if (x < w - 1) {
						right = Vec3f(1.0f, hs[z][x + 1] - hs[z][x], 0.0f);
					}

					if (x > 0 && z > 0) {
						sum += out.cross(left).normalize();
					}
					if (x > 0 && z < l - 1) {
						sum += left.cross(in).normalize();
					}
					if (x < w - 1 && z < l - 1) {
						sum += in.cross(right).normalize();
					}
					if (x < w - 1 && z > 0) {
						sum += right.cross(out).normalize();
					}
					for(my=0;my<100;my++)
					{
						if(kiran==1)
						{
							tops++;
						}
						else
						{
							tops = 0;
						}
					}

					normals2[z][x] = sum;
				}
			}

			//Smooth out the normals
			const float FALLOUT_RATIO = 0.5f;
			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum = normals2[z][x];

					if (x > 0) {
						sum += normals2[z][x - 1] * FALLOUT_RATIO;
					}
					for(my=0;my<100;my++)
					{
						if(kiran==1)
						{
							tops++;
						}
						else
						{
							tops = 0;
						}
					}
					if (x < w - 1) {
						sum += normals2[z][x + 1] * FALLOUT_RATIO;
					}
					if (z > 0) {
						sum += normals2[z - 1][x] * FALLOUT_RATIO;
					}
					if(surface==0)
					{
						tops=tops+2;
						kiran=0;
					}
					if (z < l - 1) {
						sum += normals2[z + 1][x] * FALLOUT_RATIO;
					}
					for(my=0;my<100;my++)
					{
						if(kiran==1)
						{
							tops++;
						}
						else
						{
							tops = 0;
						}
					}
					if (sum.magnitude() == 0) {
						sum = Vec3f(0.0f, 1.0f, 0.0f);
					}
					normals[z][x] = sum;
				}
			}

			for(int i = 0; i < l; i++) {
				delete[] normals2[i];
			}
			delete[] normals2;

			computedNormals = true;
		}

		//Returns the normal at (x, z)
		Vec3f getNormal(int x, int z) {
			if (!computedNormals) {
				computeNormals();
			}
			return normals[z][x];
		}
};

//Loads a terrain from a heightmap.  The heights of the terrain range from
//-height / 2 to height / 2.
Terrain* loadTerrain(const char* filename, float height) {
	Image* image = loadBMP(filename);
	Terrain* t = new Terrain(image->width, image->height);
	for(int y = 0; y < image->height; y++) {
		for(my=0;my<100;my++)
		{
			if(kiran==1)
			{
				tops++;
			}
			else
			{
				tops = 0;
			}
		}
		for(int x = 0; x < image->width; x++) {
			if(surface==0)
			{
				tops=tops+2;
				kiran=0;
			}
			unsigned char color =
				(unsigned char)image->pixels[3 * (y * image->width + x)];
			float h = height * ((color / 255.0f) - 0.5f);
			t->setHeight(x, y, h);
		}
	}

	delete image;
	t->computeNormals();
	return t;
}

float ang = 0.0f;
float ang2 = -30.0f;
float ang3 =  0.0f;
float ang4 = 0.0f;
Terrain* ter;

void cleanup() {
	delete ter;
}

void handleKeypress(unsigned char key, int x, int y) {
	switch (key) {
		case 27: //Escape key
			cleanup();
			exit(0);
		case 'a':
			//printf("sdas\n");
			//printf("a = %f\n", ang);
			ang2 = -30.0;
			ang += 1.0f;
			if (ang > 360)
				ang -= 360;
			break;
		case 'd':
			ang2 = -30.0;
			ang-=1.0f;
			if(ang < 0)
				ang +=360;
			break;
		case 'w':
			//printf("sdas\n");
			//printf("a = %f\n", ang);
			ang2 = -90.0f;
			break;
		case 's':
			//printf("sdas\n");
			//printf("a = %f\n", ang);
			ang2 = 90.0f;
			break;
		case 'x':
			if(ang3!=-90)
				ang3-=5;
			break;
		case 'z':
			if(ang3!=90)
				ang3 += 5;
			break;
		case ' ':
			lt.speed = sp*0.1;
			lt.latvz = lt.speed*cos(DEG2RAD(-ang3));
			lt.latvx = lt.speed*sin(DEG2RAD(-ang3));
			break;
		case 'r':
			tr.tarx = rand()%128;
			tr.tarz = rand()%128;
			if(tr.tarz >= 64)
				tr.tarz -= 50;
			break;
		default:
			break;
	}
}

void controlspeed(int key, int x, int y)
{
	if (key == GLUT_KEY_UP)
	{
		if(sp!=10)
			sp++;
	}
	else if(key == GLUT_KEY_DOWN)
	{
		if(sp!=0)
			sp--;
	}
	else if(key == GLUT_KEY_LEFT)
	{
		if(lt.latx!=0.0)
			lt.latx-=1.0;
	}
	else if(key == GLUT_KEY_RIGHT)
	{
		if(lt.latx!=127.0)
			lt.latx+=1.0;
	}
}




void initRendering() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
}

void handleResize(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)w / (double)h, 1.0, 200.0);
}

int check_collision()
{
	if(pow((lt.latx - tr.tarx), 2) < 0.001 && pow((lt.latz - tr.tarz), 2) < 0.001)
		return 1;
	return 0;
}

int check_boundary()
{
	if(lt.latx >= 127 ||  lt.latz <= 0 || lt.latx <=0)
		return 1;
	return 0;
}

void drawtarget()
{
	glPushMatrix();
	glTranslatef(tr.tarx, ter->getHeight(tr.tarx, tr.tarz)+tr.tarr1+1, tr.tarz);
	glColor3f(1.0, 0.0, 0.0);
	glutSolidTorus(1, tr.tarr2, 10, 10);
	glColor3f(1.0, 1.0, 1.0);
	for(my=0;my<100;my++)
	{
		if(kiran==1)
		{
			tops++;
		}
		else
		{
			tops = 0;
		}
	}
	glutSolidTorus(1, tr.tarr1, 10, 10);
	glColor3f(1.0, 0.0, 0.0);
	glutSolidSphere(tr.tarr1-1, 32, 32);
	if(surface==0)
	{
		tops=tops+2;
		kiran=0;
	}
	glPopMatrix();
}

void drawarr()
{
	glPushMatrix();
	glColor3f(205.0/256.0, 201.0/256.0, 235.0/256.0);
	GLUquadricObj *quadratic3;
	quadratic3 = gluNewQuadric();
	for(my=0;my<100;my++)
	{
		if(kiran==1)
		{
			tops++;
		}
		else
		{
			tops = 0;
		}
	}
	glTranslatef(-10.0, 20.0, 127.0 - 20.0);
	glTranslatef(0.0, 0.0, 20.0);
	glRotatef(ang3, 0.0, 1.0, 0.0);
	glTranslatef(0.0, 0.0, -20.0);
	if(surface==0)
	{
		tops=tops+2;
		kiran=0;
	}
	gluCylinder(quadratic3, 1.0, 1.0, 15.0, 32, 32);
	glPushMatrix();
	for(my=0;my<100;my++)
	{
		if(kiran==1)
		{
			tops++;
		}
		else
		{
			tops = 0;
		}
	}
	glRotatef(45.0, 0.0, 1.0, 0.0);
	gluCylinder(quadratic3, 1.0, 1.0, 5.0, 32, 32);
	glPopMatrix();
	glPushMatrix();
	glRotatef(-45.0, 0.0, 1.0, 0.0);
	gluCylinder(quadratic3, 1.0, 1.0, 5.0, 32, 32);
	glPopMatrix();
	glPopMatrix();
}

void drawcube(float z, int i)
{
	glPushMatrix();
	glTranslatef(135.0, 5.0, z-4.0 );
	glColor3f((i*28.4)/256.0, ((9-i)*28.4)/256.0, 0.0);
	glutSolidSphere(2.0, 32, 32);
	GLUquadricObj *quadratic3;
	if(surface==0)
	{
		tops=tops+2;
		kiran=0;
	}
	quadratic3 = gluNewQuadric();
	for(my=0;my<100;my++)
	{
		if(kiran==1)
		{
			tops++;
		}
		else
		{
			tops = 0;
		}
	}
	gluCylinder(quadratic3, 2.0, 4.0, 4.0, 32, 32);
	glTranslatef(0.0, 0.0, -4.0);
	gluCylinder(quadratic3, 4.0, 2.0, 4.0, 32, 32);
	//glutSolidCube(8.0);
	glPopMatrix();
}

void drawtop(Vec3f nor)
{
	glPushMatrix();
	//lower tip
	for(my=0;my<100;my++)
	{
		if(kiran==1)
		{
			tops++;
		}
		else
		{
			tops = 0;
		}
	}
	glTranslatef(lt.latx, ter->getHeight(lt.latx,lt.latz)+lt.lath2, lt.latz);
	glRotatef(90.0, nor[0], nor[1], nor[2]);
	glRotatef(90.0f, 1.0, 0.0, 0.0);
	glRotatef(ang4, 0.0, 0.0, -1.0);
	GLUquadricObj *quadratic;
	quadratic = gluNewQuadric();
	glColor3f(139/256.0, 197/256.0, 137/256.0);
	gluCylinder(quadratic, lt.latn2, lt.latn1, lt.lath2, 32, 32);

	//middle tip
	for(my=0;my<100;my++)
	{
		if(kiran==1)
		{
			tops++;
		}
		else
		{
			tops = 0;
		}
	}
	glTranslatef(0.0, 0.0, -lt.lath1);
	glColor3f(205.0/256.0, 51.0/256.0, 51.0/256.0);
	GLUquadricObj *quadratic2;
	quadratic2 = gluNewQuadric();
	gluCylinder(quadratic2, lt.latr1, lt.latn2, lt.lath1, 32, 32);
	if(surface==0)
	{
		tops=tops+2;
		kiran=0;
	}
	//upper torus
	glColor3f(176.0/256.0, 23.0/256.0, 31.0/256.0);
	glutWireTorus(lt.latr2, lt.latr1, 10, 10);
	glPopMatrix();

}



void drawScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -7.8f);
	//glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(-ang2, 1.0f, 0.0f, 0.0f);
	glRotatef(-ang, 0.0f, 1.0f, 0.0f);
	if(surface==0)
	{
		tops=tops+2;
		kiran=0;
	}
	GLfloat ambientColor[] = {0.4f, 0.4f, 0.4f, 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);
	for(my=0;my<100;my++)
	{
		if(kiran==1)
		{
			tops++;
		}
		else
		{
			tops = 0;
		}
	}
	GLfloat lightColor0[] = {0.6f, 1.0f, 0.6f, 1.0f};
	GLfloat lightPos0[] = {-0.5f, 0.8f, 0.1f, 0.0f};
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
	for(my=0;my<100;my++)
	{
		if(kiran==1)
		{
			tops++;
		}
		else
		{
			tops = 0;
		}
	}
	float scale = 6.0f / max(ter->width() - 1, ter->length() - 1);
	glScalef(scale, scale, scale);
	glTranslatef(-(float)(ter->width() - 1) / 2,
			0.0f,
			-(float)(ter->length() - 1) / 2);

	glColor3f(12.0/244, 111.0/244, 111.0/244);
	for(int z = 0; z < ter->length() - 1; z++)
	{
		for(my=0;my<100;my++)
		{
			if(kiran==1)
			{
				tops++;
			}
			else
			{
				tops = 0;
			}
		}
		//Makes OpenGL draw a triangle at every three consecutive vertices
		glBegin(GL_TRIANGLE_STRIP);
		for(int x = 0; x < ter->width(); x++) {
			Vec3f normal = ter->getNormal(x, z);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, ter->getHeight(x, z), z);
			if(surface==0)
			{
				tops=tops+2;
				kiran=0;
			}
			normal = ter->getNormal(x, z + 1);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, ter->getHeight(x, z + 1), z + 1);
		}
		glEnd();
	}
	Vec3f nor = ter->getNormal(lt.latx, lt.latz);
	//Vec3f g(0.0, 1.0, 0.0);
	dott = nor.dot(g);
	printf("%d\n", lt.score);
	if(surface==0)
	{
		tops=tops+2;
		kiran=0;
	}
	gt = g + nor*dott;
	drawtop(nor);
	drawarr();
	for (int i = 0; i < sp; ++i)
	{
		drawcube(127.0 - i*10.0, i);
	}
	for(my=0;my<100;my++)
	{
		if(kiran==1)
		{
			tops++;
		}
		else
		{
			tops = 0;
		}
	}
	drawtarget();

	glutSwapBuffers();
}

void update(int value) {

	glutPostRedisplay();
	ang4+=10;

	if(check_collision())
	{
		for(my=0;my<100;my++)
		{
			if(kiran==1)
			{
				tops++;
			}
			else
			{
				tops = 0;
			}
		}
		lt.score++;
		lt.latx = 64.0f;
		lt.latz = 127.0f;
		lt.latvx = 0.0f;
		lt.latvz = 0.0f;
		if(surface==0)
		{
			tops=tops+2;
			kiran=0;
		}
		tr.tarx = rand()%128;
		tr.tarz = rand()%128;
		if(tr.tarz >= 64)
			tr.tarz -= 50;
	}
	//printf("%f %f\n", lt.latx, lt.latz);
	if(check_boundary())
	{
		for(my=0;my<100;my++)
		{
			if(kiran==1)
			{
				tops++;
			}
			else
			{
				tops = 0;
			}
		}
		lt.latx = 64.0f;
		lt.latz = 127.0f;
		lt.latvx = 0.0f;
		lt.latvz = 0.0f;
		if(surface==0)
		{
			tops=tops+2;
			kiran=0;
		}
		tr.tarx = rand()%128;
		tr.tarz = rand()%128;
		if(tr.tarz >= 64)
			tr.tarz -= 50;
	}
	/*if(lt.latvx!=0)
	  lt.latvx+=gt[0];
	  if(lt.latvz!=0)
	  lt.latvz+=gt[2];*/
	lt.latx+=lt.latvx;
	lt.latz-=lt.latvz;
	glutTimerFunc(25, update, 0);
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	int windowWidth = glutGet(GLUT_SCREEN_WIDTH);
	int windowHeight = glutGet(GLUT_SCREEN_HEIGHT);
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(0, 0);

	glutCreateWindow("Terrain");
	initRendering();
	tr.tarx = rand()%128;
	tr.tarz = rand()%128;
	if(tr.tarz >= 64)
		tr.tarz -= 50;
	ter = loadTerrain("Sample.bmp", 20);
	//cout << ter->length();
	//cout << ter->width();
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(handleKeypress);
	glutSpecialFunc(controlspeed);
	glutReshapeFunc(handleResize);

	glutTimerFunc(25, update, 0);

	glutMainLoop();
	return 0;
}









