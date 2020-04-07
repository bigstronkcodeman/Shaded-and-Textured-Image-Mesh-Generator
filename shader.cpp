#ifdef MAC
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <math.h>
#include <fstream>
#include <iostream>
using namespace std;

#define MIN_X_SCREEN -500
#define MAX_X_SCREEN 500
#define MIN_Y_SCREEN -500
#define MAX_Y_SCREEN 500
#define MIN_Z_SCREEN -500
#define MAX_Z_SCREEN 500
#define PIXEL_ROWS 500
#define PIXEL_COLS 500

float scale_amt = 1.75;
float rotate_inc = 2;
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
bool lighting = true;
float Ka = 0.1;
float Kd = 0.5;
float Ks = 0.6;
float Kp = 0.5;

// initialize material properties
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

// initialize light properties
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

// calculate triangle surface normal
void surface_normal(float ax, float ay, float az,
	float bx, float by, float bz,
	float cx, float cy, float cz,
	float& nx, float& ny, float& nz)
{
	// get two tangents u and v
	float ux = bx - ax;
	float uy = by - ay;
	float uz = bz - az;

	float vx = cx - ax; 
	float vy = cy - ay;
	float vz = cz - az;

	// calculate u cross v
	nx = uy * vz - uz * vy;
	ny = uz * vx - ux * vz;
	nz = ux * vy - uy * vx;

	// normalize
	float mag = sqrt(nx * nx + ny * ny + nz * nz);
	nx /= mag;
	ny /= mag;
	nz /= mag;
	nz *= -1;
 }

// initialize mesh properties
void init_mesh() 
{
	// read in depth and rgb values
	ifstream din_depth, din_rgb;
	din_depth.open("penny-depth.txt");
	din_rgb.open("penny-image.txt");
	for (int u = 0; u < PIXEL_ROWS; ++u)
	{
		for (int v = 0; v < PIXEL_COLS; ++v)
		{
			din_depth >> surface[u][v];
			surface[u][v] *= 0.1;
			din_rgb >> r[u][v] >> g[u][v] >> b[u][v];
		}
	}
	din_depth.close();
	din_rgb.close();

	// calculate mesh surface normals
	int i = 0;
	for (int u = 0; u < PIXEL_ROWS - 1; ++u)
	{
		int j = 0;
		for (int v = 0; v < PIXEL_COLS - 1; ++v)
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

	// calculate mesh vertex normals
	i = 0;
	for (int u = 0; u < PIXEL_ROWS; ++u)
	{
		int j = 0;
		for (int v = 0; v < PIXEL_COLS; ++v)
		{
			if (u == 0 || u == PIXEL_ROWS - 1|| v == 0 || v == PIXEL_COLS - 1)
			{
				// outermost polygons are flat and facing the positive 
				// z-direction --> surface normal = (0,0,1)
				nx[u][v] = 0;
				ny[u][v] = 0;
				nz[u][v] = 1;
			}
			else
			{
				// calculate vertex normal as normalized sum of all surface normals
				// from polygons which contain this vertex
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
}

// draw colored polygon
void draw_pic() 
{
	for (int u = 0; u < PIXEL_ROWS - 1; ++u)
	{
		for (int v = 0; v < PIXEL_COLS - 1; ++v)
		{
			glBegin(GL_POLYGON);
			init_material(Ka, Kd, Ks, 100 * Kp, r[u][v] / 255.0, b[u][v] / 255.0, g[u][v] / 255.0);
			glColor3f(r[u][v] / 255.0, b[u][v] / 255.0, g[u][v] / 255.0);
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

// draw mesh wireframe
void draw_mesh() 
{
	glColor3f(0, 0.5, 1);
	int inc = 5;
	for (int u = 0; u < PIXEL_ROWS - inc; u += inc)
	{
		for (int v = 0; v < PIXEL_COLS - inc; v += inc)
		{
			glBegin(GL_LINE_LOOP);
			glVertex3f(((MIN_X_SCREEN / 2.0) + u) * scale_amt, ((MIN_Y_SCREEN / 2.0) + v) * scale_amt, (surface[u][v] * scale_amt) + 3);
			glVertex3f(((MIN_X_SCREEN / 2.0) + u + inc) * scale_amt, ((MIN_Y_SCREEN / 2.0) + v) * scale_amt, (surface[u + inc][v] * scale_amt) + 3);
			glVertex3f(((MIN_X_SCREEN / 2.0) + u + inc) * scale_amt, ((MIN_Y_SCREEN / 2.0) + v + inc) * scale_amt, (surface[u + inc][v + inc] * scale_amt) + 3);
			glVertex3f(((MIN_X_SCREEN / 2.0) + u) * scale_amt, ((MIN_Y_SCREEN / 2.0) + v + inc) * scale_amt, (surface[u][v + inc] * scale_amt) + 3);
			glEnd();
		}
	}
}

// render to screen
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

	glFlush();
}

// keyboard callback
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

// initialize program state
void init()
{
	// initialize openGL
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(MIN_X_SCREEN, MAX_X_SCREEN, MIN_Y_SCREEN, MAX_Y_SCREEN, MIN_Z_SCREEN, MAX_Z_SCREEN);
	glEnable(GL_DEPTH_TEST);

	// initialize lighting and build mesh
	glShadeModel(GL_SMOOTH);
	init_light(GL_LIGHT1, 0, 0, 300, 1, 1, 1);
	init_mesh();
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
