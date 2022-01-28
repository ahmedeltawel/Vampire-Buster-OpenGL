#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <queue>
#include <vector>
#include <Windows.h>
#include <mmsystem.h>

#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)
#define RAD2DEG(a) (a * 57.2957795131)

#define CAMERA_SPEED (3.16 / 180 * 0.2)

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

using namespace std;

int WIDTH = 800;
int HEIGHT = 600;

bool topview = false;

bool gameOver = false;

bool showStick = true;
bool showBlade1 = false;
bool showBlade2 = false;
bool showCBlade1 = true;
bool showCBlade2 = true;

GLuint tex;
char title[] = "3D Model Loader Sample";

float playerx = 0;
float playerz = 0;
float playerrot = 0;
float playeryoff = 1.8;

deque<int> playerDirection;
float dbx = 0;
float dby = 0;
float dbz = 0;
float dbr = 0;

void axis(double length);
void axis3();
void collisionDetection();
char* p0s[20];
float sunPos = -5.5f;
float sunDir = 1; // 1 for forwards , -1 backwards
float sunColorR = 1.0f;
float sunColorG = 1.0f;
float sunColorB = 1.0f;
float sunColorDir = 1;

bool showCreature1 = true;
bool oldcar1 = true;
bool showCreature21 = true;
bool showCreature22 = true;
bool showCreature23 = true;
int creature1Health = 3;
int level = 1; // can be either 1 or 2

class Vector3f
{
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator!()
	{
		return *this * -1;
	}

	Vector3f operator+(Vector3f& v)
	{
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}

	Vector3f operator-(Vector3f& v)
	{
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}

	Vector3f operator*(float n)
	{
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n)
	{
		return Vector3f(x / n, y / n, z / n);
	}

	Vector3f unit()
	{
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3f cross(Vector3f v)
	{
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

class BoundingBox
{

public:
	Vector3f a, b;
	BoundingBox(float x1, float z1, float x2, float z2)
	{
		a = Vector3f(x1, 0, z1);
		b = Vector3f(x2, 0, z2);
	}
	bool isInside(Vector3f v)
	{
		return v.x >= a.x && v.x <= b.x && v.z >= a.z && v.z <= b.z;
	}
};
class DrawableObject
{
public:
	// just a wrapper for Model_3DS, to track the position of the Model
	// only the position, not the rotations, so be careful!
	DrawableObject() {}; //constructor

	Vector3f position;
	float scale = 1.0;
	Vector3f size; // width and height of the object delta x and z not caring about the height y. this size after rotations
	Model_3DS model;
	void Load(char* filename)
	{
		// Load the 3DS file
		model.Load(filename);
	}
	void Draw()
	{
		// Draw the 3DS file
		model.Draw();
	}
	void translate(GLfloat x, GLfloat y, GLfloat z)
	{
		Vector3f v = Vector3f(x, y, z);
		position = position + v;
	}
	Vector3f getSize()
	{
		// take the abs value ?
		return Vector3f(size.x * scale, size.y * scale, size.z * scale);
	}
	bool includesPoint(Vector3f pos)
	{
		Vector3f size = getSize();
		// 		cout << "size" << size.x << " " << size.y << " " << size.z << endl;
		if (pos.x > position.x - size.x / 2 && pos.x < position.x + size.x / 2 && pos.z > position.z - size.z / 2 && pos.z < position.z + size.z / 2)
			return true;
		return false;
	};
};

class Camera
{
public:
	Vector3f eye, center, up;
	bool isFirst = true;
	int yoff = 20+dbx;	  // depends on the player hight
	float howFar = 7; // Third persion radius, used for first as well
	Camera(float eyeX = 1.0f, float eyeY = 1.0f, float eyeZ = 1.0f, float centerX = 0.0f, float centerY = 0.0f, float centerZ = 0.0f, float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f)
	{
		eye = Vector3f(eyeX, eyeY, eyeZ);
		center = Vector3f(centerX, centerY, centerZ);
		up = Vector3f(upX, upY, upZ);
	}

	void moveX(float d)
	{
		Vector3f right = up.cross(center - eye).unit();
		eye = eye + right * d;
		center = center + right * d;
	}

	void moveY(float d)
	{
		eye = eye + up.unit() * d;
		center = center + up.unit() * d;
	}

	void moveZ(float d)
	{
		Vector3f view = (center - eye).unit();
		eye = eye + view * d;
		center = center + view * d;
	}

	void mx(float d)
	{
		eye.x = eye.x + d;
		center.x = center.x + d;
	}
	void my(float d)
	{
		eye.y = eye.y + d;
		center.y = center.y + d;
	}
	void mz(float d)
	{
		eye.z = eye.z + d;
		center.z = center.z + d;
	}

	void rotateX(float a)
	{
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + up * sin(DEG2RAD(a));
		up = view.cross(right);
		center = eye + view;
	}

	void rotateY(float a)
	{
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
		right = view.cross(up);
		center = eye + view;
	}
	void resetfp()
	{
		// 	eye: -0.088161 6.233539 0.446960
		// center: -0.144583 6.233539 1.445367
		// up: 0.000000 1.000000 0.000000
		cout << "switching to first person" << endl;
		howFar = 0.1;
	}
	void resettp()
	{
		// 		eye: 0.278582 7.733539 -6.042686
		// center: 0.228309 7.279550 -5.153099
		// up: -0.025615 0.891007 0.453266
		cout << "switching to third person" << endl;
		howFar = 20;
	}

	void setDir()
	{
		int pDirection = playerDirection.front();
		float r = howFar;
		float angle = 90 * pDirection + 180;
		float z = r * cos(DEG2RAD(angle));
		float x = r * sin(DEG2RAD(angle));
		// cout << "Angle " << angle << " x= " << x << " z= " << z << endl;

		x += playerx;
		z += playerz;

		Vector3f view = Vector3f(x, yoff, z);
		view = view;
		// Vector3f pcenter = Vector3f(playerx, yoff, playerz);

		eye.x = view.x;
		eye.y = yoff;
		eye.z = view.z;
		center.x = playerx;
		center.y = yoff;
		center.z = playerz;
		// eye: 0.000000 6.000000 9.000000
		// center: -3.000000 6.000000 9.000000
		// up: 0.000000 1.000000 0.000000
		// px 0 py 6 pz 9
	}

	void moveRight()
	{
		if (isFirst)
		{
			rotateY(-0.5);
		}
	}

	void look()
	{
		// printf("eye: %f %f %f\n", eye.x, eye.y, eye.z);
		// printf("center: %f %f %f\n", center.x, center.y, center.z);
		// printf("up: %f %f %f\n", up.x, up.y, up.z);
		// cout << "eye: " << eye.x << " " << eye.y << " " << eye.z << endl;
		// cout << "center: " << center.x << " " << center.y << " " << center.z << endl;
		// cout << "up: " << up.x << " " << up.y << " " << up.z << endl;

		// float z = camera.howFar * cos(playerrot + 3.16);
		// float x = camera.howFar * sin(playerrot + 3.16);
		// float dz = camera.howFar * cos(playerrot);
		// float dx = camera.howFar * sin(playerrot);

		// camera.eye.x = playerx + x;
		// camera.eye.z = playerz + z;
		// camera.eye.y = 0;
		// camera.center.x = playerx;
		// camera.center.z = playerz;
		// camera.center.y = 0;

		float z = howFar * cos(playerrot + 3.16);
		float x = howFar * sin(playerrot + 3.16);
		float dz = howFar * cos(playerrot);
		float dx = howFar * sin(playerrot);

		eye.x = playerx + x;
		eye.z = playerz + z;
		eye.y = yoff;
		cout << "camera " << eye.x << " " << eye.y << " " << eye.z << endl;
		center.x = playerx;
		center.z = playerz;
		center.y = yoff;
		if (topview) {
			center.x = playerx;
			center.z = playerz;
			center.y = -1;
			
		}

		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z);
	}
};

class Marker
{
public:
	Marker() {};
	Vector3f position;
	bool isVisible = true;
	bool isSelected = false;
	void restorePos()
	{
		dbx = position.x;
		dby = position.y;
		dbz = position.z;
	};
	void draw()
	{
		if (!isVisible)
			return;
		glPushMatrix();
		if (isSelected)
		{
			glColor3f(0, 1, 0);
			position = Vector3f(dbx, dby, dbz);
			glTranslatef(dbx, dby, dbz);
		}
		else
		{
			glColor3f(1, 0, 0);
			glTranslatef(position.x, position.y, position.z);
		}
		glutSolidSphere(0.1, 10, 10);
		axis3();
		glPopMatrix();
		glColor3f(1, 1, 1); // reset color
	};
};

// Model Variables
Model_3DS model_house;
Model_3DS model_tree;
DrawableObject model_player;
// DrawableObject model_star;
Model_3DS model_box;
Model_3DS model_vil;
Model_3DS model_stick;
Model_3DS model_demon;
Model_3DS model_pen;
Model_3DS model_axe;
Model_3DS model_hammer;
Model_3DS model_garbagebags;
Model_3DS model_trashcan;
DrawableObject model_van;
//Model_3DS model_greencar;

DrawableObject model_oldcar1;
DrawableObject model_oldcar2;

DrawableObject model_blade;
DrawableObject model_blade2;

Model_3DS model_obst1;
DrawableObject model_creature1;
DrawableObject model_creature21;
DrawableObject model_creature22;
DrawableObject model_creature23;

Model_3DS model_city;
Model_3DS model_b3;

Model_3DS model_finalb2;
Model_3DS model_finalb3;

// Textures
GLTexture tex_ground;
GLTexture tex_ground2;

Camera camera;
Marker m1, m2;

vector<BoundingBox> bbs = { BoundingBox(-116.4, -56, 121, 63), BoundingBox(105.8, -88.2, 119.4, 86.2), BoundingBox(-116, -84, -104, 86.2) };
// m1 Pos:-116.399  63
// m2 Pos:120.999  -56

// m1 Pos:119.399  86.1998
// m2 Pos:105.8  -88.1998

// m1 Pos : -115.999 0.2 86.1998
// m2 Pos : -104 0 - 83.9999

//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	// 	cout << "enabling light" << endl;
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	// GLfloat diffuse[] = {0.5f + dbx, 0.5f + dby, 0.5f + dbz, 1.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	// GLfloat light_position[] = {0.0f + dbx, 10.0f + dby, 0.0f + dbz, 1.0f};
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

// Render Ground Function
//=======================================================================
void RenderGround()
{
	glDisable(GL_LIGHTING); // Disable lighting

	glColor3f(0.6, 0.6, 0.6); // Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D); // Enable 2D texturing

	if (level == 1)
		glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]); // Bind the ground texture
	else
		glBindTexture(GL_TEXTURE_2D, tex_ground2.texture[0]); // Bind the ground textur

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0); // Set quad normal direction.
	glTexCoord2f(0, 0);	 // Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-200, 0, -200);
	glTexCoord2f(5, 0);
	glVertex3f(200, 0, -200);
	glTexCoord2f(5, 5);
	glVertex3f(200, 0, 200);
	glTexCoord2f(0, 5);
	glVertex3f(-200, 0, 200);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING); // Enable lighting again for other entites coming throung the pipeline.
	glColor3f(1, 1, 1);	   // Set material back to white instead of grey used for the ground texture.
}

void drawPlayer()
{
	glPushMatrix();
	glTranslatef(playerx, playeryoff, playerz);
	glRotated(playerrot * 57.2957795131, 0, 1, 0);
	// glRotated(0, 0, 1, 0);

	glScalef(0.8, 0.8, 0.8);
	model_player.Draw();
	glPopMatrix();
}
void setupLights()
{

	GLfloat l0Diffuse[] = { 1.0f, 1.0f, sunColorB, 1.0f };
	GLfloat l0Ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat l0Position[] = { sunPos, 1, 0, 0 }; // 0 infinite distance, 1 at the specified position
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	cout << "sunColor" << sunColorB << endl;
	GLfloat l0Direction[] = { -1.0, 0.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, l0Diffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, l0Ambient);
	glLightfv(GL_LIGHT0, GL_POSITION, l0Position);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	// glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 30.0);
	// glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 90.0);
	// glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, l0Direction);
}
void setupCamera()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, WIDTH / HEIGHT, 0.001, 5000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera.look();
}

void print(int x, int y, int z, char* string)
{
	int len, i;

	//set the position of the text in the window using the x and y coordinates
	glRasterPos3f(x, y, z);

	//get the length of the string to display
	len = (int)strlen(string);

	//loop to display character by character
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}
}
void axis(double length)
{ // draw a z-axis, with cone at end
	glPushMatrix();

	glBegin(GL_LINES);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 0, length); // along the z-axis
	glEnd();

	glTranslated(0, 0, length - 0.2);
	glutWireCone(0.04, 0.2, 12, 9);

	glPopMatrix();
}
void axis3()
{
	glColor3d(0, 0, 0); // draw black lines
	axis(0.5);			// z-axis
	glPushMatrix();
	glRotated(90, 0, 1.0, 0);
	axis(0.5); // y-axis
	glRotated(-90.0, 1, 0, 0);
	axis(0.5); // z-axis
	glPopMatrix();
}

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void myInit(void)
{
	InitLightSource();
	InitMaterial();
	// first person
	camera.resetfp();

	// selected marker
	m1.isSelected = true;

	playerDirection.push_back(0);
	playerDirection.push_back(1);
	playerDirection.push_back(2);
	playerDirection.push_back(3);

	// models Initals
	//model_demon.size = Vector3f(3.6, 0, 7);
	// model_demon.scale = 0.2;

	// model_star.position = Vector3f(0, 2.2, 0);
	// model_star.size = Vector3f(3, 0, 2);
	// model_star.scale = 0.4;

	model_blade.position = Vector3f(-95.199692, 1.400000, 25.600025);
	model_blade.size = Vector3f(3, 0, 2);

	model_blade2.position = Vector3f(107.399506, 0.000000, 53.600132);
	model_blade2.size = Vector3f(3, 0, 2);

	model_oldcar1.position = Vector3f(25, 0.2, 0);
	model_oldcar1.size = Vector3f(17.8, 0, 7.4);

	// the two cars are considered one
	model_oldcar2.position = Vector3f(-43, 0, -30.2);
	model_oldcar2.size = Vector3f(35.6, 0, 7);

	model_van.position = Vector3f(57, 0, 30);
	model_van.size = Vector3f(5.6, 0, 15.4);

	model_creature21.position = Vector3f(64.4, 0, 29);
	model_creature21.size = Vector3f(6.8, 0, 5.0);

	model_creature22.position = Vector3f(-88.399796, -0.600000, -25.200024);
	model_creature22.size = Vector3f(6.8, 0, 5.0);

	model_creature23.position = Vector3f(112.199432, -0.800000, -76.399979);
	model_creature23.size = Vector3f(6.8, 0, 5.0);

	model_creature1.position = Vector3f(95.999680, 1.200000, 0);
	model_creature1.size = Vector3f(20.4, 0, 13.6);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
}

//=======================================================================
// Display Function
//=======================================================================
void myDisplay(void)
{

	setupCamera();
	setupLights();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// sprintf((char *)p0s, "Winner this is a pt=oihr eohirehiowrehiurh rewohgerw");
	// glColor3f(1, 1, 1);
	// print(0, 0, 0, (char *)p0s);

	//Draw position marker, as a sphere
	m1.draw();
	m2.draw();

	// Draw Ground
	RenderGround();

	// Draw player
	drawPlayer();

	// Draw Tree Model

	// Draw axe
	// glPushMatrix();
	// glTranslatef(0, 2.2, 0);
	// glScalef(0.1, 0.1, 0.1);
	// model_axe.Draw();
	// glPopMatrix();

	// glPushMatrix();
	// // glTranslatef(0, 2, 0);
	// glTranslatef(3, 2.2, 0);
	// glScalef(0.5, 0.5, 0.5);
	// glRotatef(90, 0, 1, 0);
	// model_hammer.Draw();
	// glPopMatrix();

	// glPushMatrix();
	// glTranslatef(model_demon.position.x, model_demon.position.y, model_demon.position.z);
	// glScalef(model_demon.scale, model_demon.scale, model_demon.scale);
	// model_demon.Draw();
	// glPopMatrix();

	// Draw Star
	// glPushMatrix();
	// glTranslatef(model_star.position.x, model_star.position.y, model_star.position.z);
	// glRotatef(90, 1, 0, 0);
	// glScalef(model_star.scale, model_star.scale, model_star.scale);
	// model_star.Draw();
	// glPopMatrix();

	// Weapons
	// Draw Stick
	glPushMatrix();
	glTranslatef(playerx, 0, playerz);		// move witht the player
	glRotatef(RAD2DEG(playerrot), 0, 1, 0); // rotate with the player
	glTranslatef(-1, -0.2 + playeryoff, -1);
	glScalef(0.2, 0.2, 0.2);
	glRotatef(-90 - 22, 0, 1, 0);
	if (showStick)
		model_stick.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(playerx, 0, playerz);		// move witht the player
	glRotatef(RAD2DEG(playerrot), 0, 1, 0); // rotate with the player
	glTranslatef(-1.2, 1 + playeryoff, -0.8);
	glRotatef(90 - 25, 0, 1, 0);
	if (showBlade1)
		model_blade.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(playerx, 0, playerz);		// move witht the player
	glRotatef(RAD2DEG(playerrot), 0, 1, 0); // rotate with the player
	glTranslatef(-1, -1.2 + playeryoff, 2.6);
	if (showBlade2)
		model_blade2.Draw();
	glPopMatrix();
	// ------------ End of Weapons

	// glPushMatrix();
	// // glTranslatef(0, 2, 0);
	// // glTranslatef(3, 2.2, 0);
	// glTranslatef(dbx, dby, dbz);
	// glScalef(2, 2, 2);
	// // glRotatef(90, 0, 1, 0);
	// model_garbagebags.Draw();
	// glPopMatrix();

	// glPushMatrix();
	// // glTranslatef(0, 2, 0);
	// glTranslatef(3, 2.2, 0);
	// glScalef(2, 2, 2);
	// glRotatef(90, 0, 1, 0);
	// model_trashcan.Draw();
	// glPopMatrix();

	glPushMatrix();
	glTranslatef(57, 0, 30);
	glScalef(3, 3, 3);
	glRotatef(90, 0, 1, 0);
	model_van.Draw();
	glPopMatrix();



	glPushMatrix();
	glTranslatef(25, 0.2, 0);
	glScalef(3, 3, 3);
	glRotatef(90, 0, 1, 0);
	if (!oldcar1)
	{ // if killed
		glRotatef(-90, 1, 0, 0);
	}
	model_oldcar1.Draw();
	glPopMatrix();


	glPushMatrix();
	glTranslatef(-52.200127, 0.000000, -30.200043);
	glScalef(3, 3, 3);
	glRotatef(90, 0, 1, 0);
	model_oldcar2.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-33.000053, -0.000000, -30.200043);
	glScalef(3, 3, 3);
	glRotatef(90, 0, 1, 0);
	model_oldcar2.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(95.999680, 1.200000, 0);
	glRotatef(-118, 0, 1, 0);
	if (!showCreature1)
	{ // if killed
		glTranslatef(0, 10.6, 0);
		glRotatef(180, 1, 0, 0);
	}
	if (level == 2)
		model_creature1.Draw(); // the 2nd Level creature
	glPopMatrix();

	glPushMatrix();
	glTranslatef(64.4, -0.600000, 29);
	glScalef(0.8, 0.8, 0.8);
	if (!showCreature21) // if is killed
	{
		glTranslatef(0, 1.8, 0);
		glRotatef(-90, 1, 0, 0);
	}
	model_creature21.Draw(); // first Level creatures
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-88.399796, -0.600000, -25.200024);
	glScalef(0.8, 0.8, 0.8);
	if (!showCreature22) // if is killed
	{
		glTranslatef(0, 1.8, 0);
		glRotatef(-90, 1, 0, 0);
	}

	model_creature22.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(112.199432, -0.800000, -76.399979);
	glScalef(0.8, 0.8, 0.8);
	if (!showCreature23) // if is killed
	{
		glTranslatef(0, 1.8, 0);
		glRotatef(-90, 1, 0, 0);
	}
	glRotatef(-0.2, 0, 1, 0);
	model_creature23.Draw();
	glPopMatrix();

	// glPushMatrix();
	// // glTranslatef(0, 2, 0);
	// glTranslatef(30, -50, 0);

	// // model_b3.Draw();
	// glPopMatrix();

	// glPushMatrix();
	// // glTranslatef(0, 2, 0);
	// // glTranslatef(10, 10, 10);
	// glScalef(3, 3, 3);
	// // model_finalb1.Draw();
	// glPopMatrix();

	glPushMatrix();
	glTranslatef(0, -3, 0); // offset to show the ground
	if (level == 1)
		model_finalb2.Draw();
	else
		model_finalb3.Draw();
	glPopMatrix();

	// glPushMatrix();
	// // glTranslatef(0, 2, 0);
	// glTranslatef(3, 0, 0);

	// model_obst1.Draw();
	// glPopMatrix();

	// // Draw pen
	// glPushMatrix();
	// glTranslatef(playerx, 0, playerz);				  // move witht the player
	// glRotatef(90 * playerDirection.front(), 0, 1, 0); // rotate with the player
	// // glTranslatef(dbgx, dbgy, dbgz);
	// glTranslatef(-2.4, 3, 0);
	// glRotatef(-140, 0, 1, 0);
	// glScalef(0.4, 0.4, 0.4);
	// model_pen.Draw();
	// glPopMatrix();

	// Draw Box
	// glPushMatrix();
	// glTranslatef(1, 2, 3);
	// glScalef(2, 2, 2);
	// model_box.Draw();
	// glPopMatrix();

	// Collectables
	glPushMatrix();
	glTranslatef(-95.199692, 1.400000, 25.600025);
	glRotatef(-4.4, 0, 1, 0);
	if (showCBlade1)
		model_blade.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(107.399506, 0.000000, 53.600132);
	if (showCBlade2)
		model_blade2.Draw();
	glPopMatrix();

	//sky box
	glPushMatrix();
	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	// glTranslated(50, 0, 0);
	// glRotated(dbx * 3, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, tex);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 250, 100, 100);
	gluDeleteQuadric(qobj);
	glPopMatrix();

	//Collision Detection
	collisionDetection();

	printf("debug %f, \t%f, \t%f r(%f)\n", dbx, dby, dbz, dbr);
	glFlush();

	cout << "playerx= " << playerx << "; playerz=" << playerz << endl;

	glutSwapBuffers();
}

void collisionDetection()
{
	Vector3f playerPos = Vector3f(playerx, 0, playerz);
	// if (model_star.includesPoint(Vector3f(playerx, 0, playerz)))
	// 	cout << "Collision!!!!!!!!!!!!!!!!!!!!!" << endl;
	// else
	// 	cout << "No Collision" << endl;
	if (model_blade.includesPoint(playerPos))
	{
		cout << "Collision taking blade1" << endl;
		showStick = false;
		showBlade1 = true;
		showBlade2 = false;
		showCBlade1 = false;
	}
	if (model_blade2.includesPoint(playerPos))
	{
		cout << "Collision taking blade2" << endl;
		showStick = false;
		showBlade1 = false;
		showBlade2 = true;
		showCBlade2 = false;
	}
}
void movePlayer(int direction) //
{
	/**					 x -
	 * 		             (3)
	 *                   |
	 * 		z +  (0) <-- p  --> (2)  Z -
	 *                  |
	 *                 (1)
	*				  x +
	*
					 */

	float stepSize = 1;

	float dz = stepSize * cos(playerrot);
	float dx = stepSize * sin(playerrot);

	Vector3f playerPos = Vector3f(playerx + dx, 0, playerz + dz);

	bool collision = false;
	for (int i = 0; i < bbs.size(); i++)
	{
		if (bbs[i].isInside(playerPos))
		{
			collision = true;
		}
	}
	if (!collision)
		return;

	if (model_oldcar1.includesPoint(playerPos))
	{
		oldcar1 = false;
		cout << "Collision with car1" << endl;
		return; // do nothing
	}

	if (model_oldcar2.includesPoint(playerPos))
	{
		cout << "Collision with 2 cars block " << endl;
		return; // do nothing
	}
	if (model_van.includesPoint(playerPos))
	{
		cout << "Collision with red van" << endl;
		return; // do nothing
	}

	playerx = playerx + dx;
	playerz = playerz + dz;
	camera.look();
}
//=======================================================================
// Keyboard Function
//=======================================================================
void myKeyboard(unsigned char key, int x, int y)
{
	float d = 0.5;
	float player_speed = 1;
	switch (key)
	{
	case 't':
		topview = !topview;
		break;
	case 'w': // forwards

		movePlayer(0);
		// camera.setDir();
		break;
	case 's': // backwards
		// nothing
		break;
	case 'a': // left
		// nothing
		break;
	case 'd': // right
		// nothing

		break;
	case ' ': // toggle first/third persion
		if (camera.isFirst)
		{
			camera.resettp();
			camera.isFirst = false;
		}
		else
		{
			camera.isFirst = true;
			camera.resetfp();
		}
		break;

	case '8':
		camera.moveY(d);
		break;
	case '5':
		camera.moveY(-d);
		break;
	case '4':
		camera.moveX(d);
		break;
	case '6':
		camera.moveX(-d);
		break;
	case '7':
		camera.moveZ(d);
		break;
	case '9':
		camera.moveZ(-d);
		break;
		// debug keys
	case 'j':
		dbx += 0.2;
		break;
	case 'k':
		dby += 0.2;
		break;
	case 'l':
		dbz += 0.2;
		break;
	case 'u':
		dbx += -0.2;
		break;
	case 'i':
		dby += -0.2;
		break;
	case 'o':
		dbz += -0.2;
		break;
	case ';':
		dbr += -0.2;
		break;
	case 'p':
		dbr -= -0.2;
		break;
	case 'm':
		if (m1.isSelected)
		{
			m1.isSelected = false;
			m2.isSelected = true;
			m2.restorePos();
		}
		else
		{
			m1.isSelected = true;
			m2.isSelected = false;
			m1.restorePos();
		}
		break;
	case 'M':
	{
		Vector3f d = m1.position - m2.position;
		cout << "disatnce: " << d.x << " " << d.y << " " << d.z << endl;
		cout << "m1 Pos:" << m1.position.x << " " << m1.position.y << " " << m1.position.z << endl;
		cout << "m2 Pos:" << m2.position.x << " " << m2.position.y << " " << m2.position.z << endl;
	}
	break;
	case 27:
		exit(EXIT_SUCCESS);
	}

	glutPostRedisplay();
}
void keySpecial(int key, int x, int y)
{
	float a = 1.0;

	switch (key)
	{
	case GLUT_KEY_UP:
		camera.rotateX(a);
		break;
	case GLUT_KEY_DOWN:
		camera.rotateX(-a);
		break;
	case GLUT_KEY_LEFT:
		camera.rotateY(a);
		break;
	case GLUT_KEY_RIGHT:
		camera.rotateY(-a);
		break;
	}

	glutPostRedisplay();
}

//=======================================================================
// Motion Function
//=======================================================================
void myMotion(int x, int y)
{
	// y = HEIGHT - y;

	// cameraZoom = y;

	glLoadIdentity(); //Clear Model_View Matrix

	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glutPostRedisplay();
}

//=======================================================================
// Mouse Function
//=======================================================================


void victory(int value) {
	PlaySound(TEXT("Sounds/VictorySound.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void uplvl(int value) {
	PlaySound(TEXT("Sounds/UptoLevel2.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void checkGoToLvlTwo()
{
	if (!showCreature21 && !showCreature22 && !showCreature23)
	{			 
		glutTimerFunc(4000, uplvl, 0);
	
		// all creature2 dead
		level = 2; // by default is 1
	}
	if (!showCreature1)
	{

		gameOver = true;
		// TODO: music Success
		glutTimerFunc(5000, victory, 0);


	}
}



void myMouse(int button, int state, int x, int y)
{

	if (state == GLUT_DOWN)
	{
		Vector3f playerPos = Vector3f(playerx, 0, playerz);
		cout << "Mouse CLicK" << endl;
		if (model_creature21.includesPoint(playerPos) && level == 1)
		{
			PlaySound(TEXT("Sounds/VimpireDeath.wav"), NULL, SND_FILENAME | SND_ASYNC);
			cout << "Collision with creature21" << endl;
			showCreature21 = false;
			checkGoToLvlTwo();
				
			return; // do nothing
		}
		if (model_creature22.includesPoint(playerPos) && level == 1)
		{
			PlaySound(TEXT("Sounds/VimpireDeath.wav"), NULL, SND_FILENAME | SND_ASYNC);
			cout << "Collision with creature22" << endl;
			showCreature22 = false;
			checkGoToLvlTwo();
			return; // do nothing
		}
		if (model_creature23.includesPoint(playerPos) && level == 1)
		{
			PlaySound(TEXT("Sounds/VimpireDeath.wav"), NULL, SND_FILENAME | SND_ASYNC);
			cout << "Collision with creature23" << endl;
			showCreature23 = false;
			checkGoToLvlTwo();
			return; // do nothing
		}
		if (model_creature1.includesPoint(playerPos) && level == 2)
		{
			PlaySound(TEXT("Sounds/MonsterDeath.wav"), NULL, SND_FILENAME | SND_ASYNC);
			cout << "Collision with creature1" << endl;
			creature1Health = creature1Health - 1;
			if (creature1Health <= 0) {
				showCreature1 = false;
				checkGoToLvlTwo();
			}
			return; // do nothing
		}
	}
}

//=======================================================================
// Reshape Function
//=======================================================================
void myReshape(int w, int h)
{
	if (h == 0)
	{
		h = 1;
	}

	WIDTH = w;
	HEIGHT = h;

	// set the drawable region of the window
	glViewport(0, 0, w, h);

	// set up the projection matrix
	// setupCamera();

	// go back to modelview matrix so we can move the objects about
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{
	model_stick.Load("Models/stick/L1.3DS");
	model_player.Load("models/player/player.3ds");
	model_garbagebags.Load("models/garbagebags/garbagebags.3ds");
	model_van.Load("models/van/van.3ds");
	model_oldcar1.Load("models/oldcar1/oldcar1.3ds");
	model_oldcar2.Load("models/oldcar2/oldcar2.3ds");

	model_creature1.Load("models/creature1/creature1.3ds");
	model_creature21.Load("models/creature2/creature2.3ds");
	model_creature22.Load("models/creature2/creature2.3ds");
	model_creature23.Load("models/creature2/creature2.3ds");

	// model_axe.Load("models/axe/axe.3ds");
	// model_hammer.Load("models/hammer/hammer.3ds");
	model_blade.Load("models/blade/blade.3ds");
	model_blade2.Load("models/blade2/blade2.3ds");

	// model_trashcan.Load("models/trashcan/trashcan.3ds");
	// model_obst1.Load("models/obst1/obst1.3ds");

	model_finalb2.Load("models/finalb2/finalb2.3ds");
	model_finalb3.Load("models/finalb3/finalb2.3ds"); 

	// Loading texture files
	tex_ground.Load("Textures/ground.bmp");
	tex_ground2.Load("Textures/ground2.bmp");

	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
}

void timeSunMovement(int val)
{
	sunPos += sunDir * 0.1;
	if (sunPos > 5.5)
		sunDir = -1;
	else if (sunPos < -5.5)
		sunDir = 1;

	if (sunColorB <= 0.5)
	{
		sunColorDir = 1;
	}
	else if (sunColorB >= 1.0)
	{
		sunColorDir = -1;
	}
	sunColorB += sunColorDir * 0.1;

	glutPostRedisplay();
	glutTimerFunc(500, timeSunMovement, 0);
}

void ReadMouseMotion(int x, int y)
{
	// This variable is hack to stop glutWarpPointer from triggering an event callback to Mouse(...)
	// This avoids it being called recursively and hanging up the event loop
	static bool just_warped = false;

	if (just_warped)
	{
		just_warped = false;
		return;
	}

	//la cantidad desplazada en pixeles
	int dx = x - WIDTH / 2;
	int dy = y - HEIGHT / 2;

	if (dx)
	{
		double angle = dx * 0.01;
		angle *= -1; // invert
		// cout << angle << endl;
		playerrot += angle;

		camera.look();

		cout << "angle?" << playerrot * 57.2957795131 << endl;

		glutPostRedisplay();
	}

	/*if (dy) {
		Camera.RotatePitch(-CAMERA_SPEED * dy);
	}*/

	glutWarpPointer(WIDTH / 2, HEIGHT / 2);

	just_warped = true;
}
void checkWin(int val)
{
	if (!gameOver)
	{
		PlaySound(TEXT("Sounds/GameOver.wav"), NULL, SND_FILENAME | SND_SYNC);
		// TODO: music fail
	}
}
//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	int res_x, res_y, pos_x, pos_y;
	res_x = glutGet(GLUT_SCREEN_WIDTH);
	res_y = glutGet(GLUT_SCREEN_HEIGHT);
	pos_x = (res_x >> 1) - (SCREEN_WIDTH >> 1);
	pos_y = (res_y >> 1) - (SCREEN_HEIGHT >> 1);

	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);

	//glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);

	//glutInitWindowPosition(0, 0);
	glutInitWindowPosition(pos_x, pos_y);

	glutCreateWindow("Vampire Buster");
	// glutFullScreen();

	glutSetCursor(GLUT_CURSOR_NONE);
	glutMotionFunc(ReadMouseMotion);
	glutPassiveMotionFunc(ReadMouseMotion);

	glutDisplayFunc(myDisplay);

	glutKeyboardFunc(myKeyboard);
	glutSpecialFunc(keySpecial);

	// glutMotionFunc(myMotion);

	glutMouseFunc(myMouse);

	glutReshapeFunc(myReshape);

	glutTimerFunc(3000, timeSunMovement, 0);
	glutTimerFunc(5 * 60 * 1000, checkWin, 0);

	myInit();
	//glutFullScreen();

	LoadAssets();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	GLfloat lmodel_ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

	glutMainLoop();
}