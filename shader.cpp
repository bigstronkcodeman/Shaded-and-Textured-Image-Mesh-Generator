#ifdef MAC
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
using namespace std;

#define MIN_X_VIEW -50
#define MAX_X_VIEW 50
#define MIN_Y_VIEW -50
#define MAX_Y_VIEW 50
#define MIN_Z_VIEW -50
#define MAX_Z_VIEW 50
#define MIN_X_SCREEN -500
#define MAX_X_SCREEN 500
#define MIN_Y_SCREEN -500
#define MAX_Y_SCREEN 500
#define MIN_Z_SCREEN -500
#define MAX_Z_SCREEN 500

#define PIXEL_ROWS 500
#define PIXEL_COLS 500

float scale_amt = 1.75;
float rotate_inc = 3;

int xangle = 0;
int yangle = 0;
int zangle = 270;

float surface[PIXEL_ROWS][PIXEL_COLS];

float surface_nx[PIXEL_ROWS][PIXEL_COLS * 2];
float surface_ny[PIXEL_ROWS][PIXEL_COLS * 2];
float surface_nz[PIXEL_ROWS][PIXEL_COLS * 2];

float nx[PIXEL_ROWS][PIXEL_COLS];
float ny[PIXEL_ROWS][PIXEL_COLS];
float nz[PIXEL_ROWS][PIXEL_COLS];

float r[PIXEL_ROWS][PIXEL_COLS];
float g[PIXEL_ROWS][PIXEL_COLS];
float b[PIXEL_ROWS][PIXEL_COLS];

bool drawMesh = false;
bool drawPic = true;
bool drawNormals = false;
bool lighting = true;

float Ka = 0.1;
float Kd = 0.4;
float Ks = 0.5;
float Kp = 0.8;

void init_material(float Ka, float Kd, float Ks, float Kp,
	float Mr, float Mg, float Mb)
{
	float ambient[] = { Ka * Mr, Ka * Mg, Ka * Mb, 1.0 };
	float diffuse[] = { Kd * Mr, Kd * Mg, Kd * Mb, 1.0 };
	float specular[] = { Ks * Mr, Ks * Mg, Ks * Mb, 1.0 };

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, Kp);
}

void init_light(int light_source, float Lx, float Ly, float Lz,
	float Lr, float Lg, float Lb)
{
	float light_position[] = { Lx, Ly, Lz, 0.0 };
	float light_color[] = { Lr, Lg, Lb, 1.0 };

	glEnable(GL_LIGHTING);
	glEnable(light_source);
	glLightfv(light_source, GL_POSITION, light_position);
	glLightfv(light_source, GL_AMBIENT, light_color);
	glLightfv(light_source, GL_DIFFUSE, light_color);
	glLightfv(light_source, GL_SPECULAR, light_color);
	glLightf(light_source, GL_CONSTANT_ATTENUATION, 1.0);
	glLightf(light_source, GL_LINEAR_ATTENUATION, 0.0);
	glLightf(light_source, GL_QUADRATIC_ATTENUATION, 0.0);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
}

void printSurfaceNormals()
{
	for (int i = 0; i < PIXEL_ROWS; ++i)
	{
		for (int j = 0; j < PIXEL_COLS; ++j)
		{
			cout << "surface_normal[" << i << "][" << j << "]: ("
				<< surface_nx[i][j] << "," << surface_ny[i][j] << ","
				<< surface_nz[i][j] << ")\n";
		}
	}
}

void surface_normal(float ax, float ay, float az,
	float bx, float by, float bz,
	float cx, float cy, float cz,
	float& nx, float& ny, float& nz)
{
	float ux = bx - ax;
	float uy = by - ay;
	float uz = bz - az;

	float vx = cx - ax; 
	float vy = cy - ay;
	float vz = cz - az;

	float centroid_x = (ax + bx + cx) / 3.0;
	float centroid_y = (ay + by + cy) / 3.0;
	float centroid_z = (az + bz + cz) / 3.0;

	nx = uy * vz - uz * vy;
	ny = uz * vx - ux * vz;
	nz = ux * vy - uy * vx;

	float mag = sqrt(nx * nx + ny * ny + nz * nz);
	nx /= mag;
	ny /= mag;
	nz /= mag;

	nz < 0 ? nz *= -1 : nz;
 }

void initMesh() 
{
	ifstream din_depth, din_rgb;
	din_depth.open("penny-depth.txt");

	for (int u = 0; u < PIXEL_ROWS; u++)
	{
		for (int v = 0; v < PIXEL_COLS; v++)
		{
			din_depth >> surface[u][v];
			surface[u][v] *= 0.1;
		}
	}

	din_depth.close();

	int i = 0;
	for (int u = 0; u < PIXEL_ROWS - 1; u++)
	{
		int j = 0;
		for (int v = 0; v < PIXEL_COLS - 1; v++)
		{
			surface_normal(((MIN_X_SCREEN / 2.0) + u + 1) * scale_amt, ((MIN_X_SCREEN / 2.0) + v + 1) * scale_amt, surface[u + 1][v + 1] * scale_amt,
				((MIN_X_SCREEN / 2.0) + u) * scale_amt, ((MIN_X_SCREEN / 2.0) + v) * scale_amt, surface[u][v] * scale_amt,
				((MIN_X_SCREEN / 2.0) + u) * scale_amt, ((MIN_X_SCREEN / 2.0) + v + 1) * scale_amt, surface[u][v + 1] * scale_amt,
				surface_nx[i][j], surface_ny[i][j], surface_nz[i][j]);

			++j;

			surface_normal(((MIN_X_SCREEN / 2.0) + u + 1) * scale_amt, ((MIN_X_SCREEN / 2.0) + v + 1) * scale_amt, surface[u + 1][v + 1] * scale_amt,
				((MIN_X_SCREEN / 2.0) + u + 1) * scale_amt, ((MIN_X_SCREEN / 2.0) + v) * scale_amt, surface[u + 1][v] * scale_amt,
				((MIN_X_SCREEN / 2.0) + u) * scale_amt, ((MIN_X_SCREEN / 2.0) + v) * scale_amt, surface[u][v] * scale_amt,
				surface_nx[i][j], surface_ny[i][j], surface_nz[i][j]);
			++j;
		}
		++i;
	}

	i = 0;
	for (int u = 0; u < PIXEL_ROWS; ++u)
	{
		int j = 0;
		for (int v = 0; v < PIXEL_COLS; ++v)
		{
			if (u == 0 || u == PIXEL_ROWS - 1|| v == 0 || v == PIXEL_COLS - 1)
			{
				nx[u][v] = 0;
				ny[u][v] = 0;
				nz[u][v] = 1;
			}
			else
			{
				nx[u][v] = surface_nx[i][j] + surface_nx[i][j + 1] + surface_nx[i][j + 2]
					+ surface_nx[i + 1][j + 1] + surface_nx[i + 1][j + 2] + surface_nx[i + 1][j + 3];
				ny[u][v] = surface_ny[i][j] + surface_ny[i][j + 1] + surface_ny[i][j + 2]
					+ surface_ny[i + 1][j + 1] + surface_ny[i + 1][j + 2] + surface_ny[i + 1][j + 3];
				nz[u][v] = surface_nz[i][j] + surface_nz[i][j + 1] + surface_nz[i][j + 2]
					+ surface_nz[i + 1][j + 1] + surface_nz[i + 1][j + 2] + surface_nz[i + 1][j + 3];

				j += 2;
				
				float magnitude = sqrt(nx[u][v] * nx[u][v] 
					+ ny[u][v] * ny[u][v] 
					+ nz[u][v] * nz[u][v]);
				nx[u][v] /= magnitude;
				ny[u][v] /= magnitude;
				nz[u][v] /= magnitude;

			}
		}

		i += 1;
	}

	din_rgb.open("penny-image.txt");
	for (int u = 0; u < PIXEL_ROWS; u++)
	{
		for (int v = 0; v < PIXEL_COLS; v++)
		{
			din_rgb >> r[u][v]
				>> g[u][v]
				>> b[u][v];
		}
	}

	din_rgb.close();

}

void draw_normals()
{
	glColor3f(1, 0, 0);
	float extra_scale = 20;
	float inc = 5;
	for (int u = 0; u < PIXEL_ROWS; u += inc)
	{
		for (int v = 0; v < PIXEL_COLS; v += inc)
		{
			glBegin(GL_LINES);
			glVertex3f(((MIN_X_SCREEN / 2.0) + u) * scale_amt, ((MIN_Y_SCREEN / 2.0) + v) * scale_amt, surface[u][v] * scale_amt);
			glVertex3f((((MIN_X_SCREEN / 2.0) + u + nx[u][v]) * scale_amt) + nx[u][v] * extra_scale, (((MIN_Y_SCREEN / 2.0) + v + ny[u][v]) * scale_amt) + ny[u][v] * extra_scale, (surface[u][v] * scale_amt) + nz[u][v] * extra_scale);
			glEnd();
		}
	}
}

void draw_pic() 
{
	for (int u = 0; u < PIXEL_ROWS - 1; u++)
	{
		for (int v = 0; v < PIXEL_COLS - 1; v++)
		{
			glBegin(GL_POLYGON);
			glColor3f(r[u][v] / 255.0, b[u][v] / 255.0, g[u][v] / 255.0);
			init_material(Ka, Kd, Ks, 100 * Kp, r[u][v] / 255.0, b[u][v] / 255.0, g[u][v] / 255.0);
			glNormal3f(nx[u][v], ny[u][v], nz[u][v]);
			glVertex3f(((MIN_X_SCREEN / 2.0) + u) * scale_amt, ((MIN_Y_SCREEN / 2.0) + v) * scale_amt, surface[u][v] * scale_amt);
			glNormal3f(nx[u + 1][v], ny[u + 1][v], nz[u + 1][v]);
			glVertex3f(((MIN_X_SCREEN / 2.0) + u + 1) * scale_amt, ((MIN_Y_SCREEN / 2.0) + v) * scale_amt, surface[u + 1][v] * scale_amt);
			glNormal3f(nx[u + 1][v + 1], ny[u + 1][v + 1], nz[u + 1][v + 1]);
			glVertex3f(((MIN_X_SCREEN / 2.0) + u + 1) * scale_amt, ((MIN_Y_SCREEN / 2.0) + v + 1) * scale_amt, surface[u + 1][v + 1] * scale_amt);
			glNormal3f(nx[u][v + 1], ny[u][v + 1], nz[u][v + 1]);
			glVertex3f(((MIN_X_SCREEN / 2.0) + u) * scale_amt, ((MIN_Y_SCREEN / 2.0) + v + 1) * scale_amt, surface[u][v + 1] * scale_amt);
			glEnd();
		}
	}
}

void draw_mesh() 
{
	glColor3f(0, 0.5, 1);
	int inc = 5;
	for (int u = 0; u <= PIXEL_ROWS - inc; u += inc)
	{
		u > 0 ? u-- : u;

		for (int v = 0; v <= PIXEL_COLS - inc; v += inc)
		{
			v > 0 ? v-- : v;

			glBegin(GL_LINES);
			glColor3f(0, 0.5, 1);
			glVertex3f(((MIN_X_SCREEN / 2.0) + u) * scale_amt, ((MIN_Y_SCREEN / 2.0) + v) * scale_amt, (surface[u][v] + 3) * scale_amt);
			glVertex3f(((MIN_X_SCREEN / 2.0) + u) * scale_amt, ((MIN_Y_SCREEN / 2.0) + v + inc) * scale_amt, (surface[u][v + inc] + 3) * scale_amt);
			glEnd();

			glBegin(GL_LINES);
			glColor3f(0, 0.5, 1);
			glVertex3f(((MIN_X_SCREEN / 2.0) + v) * scale_amt, ((MIN_Y_SCREEN / 2.0) + u) * scale_amt, (surface[v][u] + 3) * scale_amt);
			glVertex3f(((MIN_X_SCREEN / 2.0) + v + inc) * scale_amt, ((MIN_Y_SCREEN / 2.0) + u) * scale_amt, (surface[v][u + inc] + 3) * scale_amt);
			glEnd();

			glBegin(GL_LINES);
			glColor3f(0, 0.5, 1);
			glVertex3f(((MIN_X_SCREEN / 2.0) + u) * scale_amt, ((MIN_Y_SCREEN / 2.0) + v) * scale_amt, (surface[u][v] + 3) * scale_amt);
			glVertex3f(((MIN_X_SCREEN / 2.0) + u + inc) * scale_amt, ((MIN_Y_SCREEN / 2.0) + v + inc) * scale_amt, (surface[u + inc][v + inc] + 3) * scale_amt);
			glEnd();

			v > 0 ? v++ : v;
		}

		u > 0 ? u++ : u;
	}

	int u = PIXEL_ROWS - 1;
	for (int v = 0; v <= PIXEL_COLS - inc; v += inc)
	{
		v > 0 ? v-- : v;

		glBegin(GL_LINES);
		glVertex3f(((MIN_X_SCREEN / 2.0) + u) * scale_amt, ((MIN_Y_SCREEN / 2.0) + v) * scale_amt, (surface[u][v] + 3) * scale_amt);
		glVertex3f(((MIN_X_SCREEN / 2.0) + u) * scale_amt, ((MIN_Y_SCREEN / 2.0) + v + inc) * scale_amt, (surface[u][v + inc] + 3) * scale_amt);
		glEnd();

		glBegin(GL_LINES);
		glVertex3f(((MIN_X_SCREEN / 2.0) + v) * scale_amt, ((MIN_Y_SCREEN / 2.0) + u) * scale_amt, (surface[v][u] + 3) * scale_amt);
		glVertex3f(((MIN_X_SCREEN / 2.0) + v + inc) * scale_amt, ((MIN_Y_SCREEN / 2.0) + u) * scale_amt, (surface[v + inc][u] + 3) * scale_amt);
		glEnd();

		v > 0 ? v++ : v;
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(xangle, 1.0, 0.0, 0.0);
	glRotatef(yangle, 0.0, 1.0, 0.0);
	glRotatef(zangle, 0.0, 0.0, 1.0);

	if (drawPic)
	{
		if (lighting)
		{
			glEnable(GL_LIGHTING);
		}
		else
		{
			glDisable(GL_LIGHTING);
		}
		draw_pic();
	}

	if (drawMesh)
	{
		glDisable(GL_LIGHTING);
		draw_mesh();
		glEnable(GL_LIGHTING);
	}

	if (drawNormals)
	{
		glDisable(GL_LIGHTING);
		draw_normals();
		glEnable(GL_LIGHTING);
	}

	glFlush();
}

void keyboard(unsigned char key, int x, int y)
{
	if (key == 'x')
		xangle -= rotate_inc;
	else if (key == 'X')
		xangle += rotate_inc;
	else if (key == 'y')
		yangle -= rotate_inc;
	else if (key == 'Y')
		yangle += rotate_inc;
	else if (key == 'z')
		zangle -= rotate_inc;
	else if (key == 'Z')
		zangle += rotate_inc;
	else if (key == 'm' || key == 'M')
		drawMesh = !drawMesh;
	else if (key == 'p' || key == 'P')
		drawPic = !drawPic;
	else if (key == 'n' || key == 'N')
		drawNormals = !drawNormals;
	else if (key == 'l' || key == 'L') {
		lighting = !lighting;
		lighting ? rotate_inc++ : rotate_inc--;
	}
	else if (key == '+')
		scale_amt += 0.01;
	else if (key == '-') 
		scale_amt -= 0.01;

	glutPostRedisplay();
}

void init()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(MIN_X_SCREEN, MAX_X_SCREEN, MIN_Y_SCREEN, MAX_Y_SCREEN, MIN_Z_SCREEN, MAX_Z_SCREEN);
	glEnable(GL_DEPTH_TEST);

	glShadeModel(GL_SMOOTH);
	init_light(GL_LIGHT0, 500, 500, 250, 1, 1, 1);
	init_light(GL_LIGHT1, 0, 0, 250, 1, 1, 1);
	//init_light(GL_LIGHT1, -250, 250, 125, 1, 1, 1);
	//init_light(GL_LIGHT2, 250, -250, 125, 1, 1, 1);

	initMesh();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(250, 250);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE | GLUT_DEPTH);
	glutCreateWindow("Penny");
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	init();
	glutMainLoop();
	return 0;
}
