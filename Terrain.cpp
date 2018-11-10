/**
 * Created by Zhenqi
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <time.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#endif

using namespace std;

float camPos[] = {0, 0, 200};
int angleX = 44;
int angleY = -20;
int angleZ = 0;
const int terrainSize = 100;         // terrain's size is 100 * 100
float heights[terrainSize][terrainSize];    // [x][z], float value is the height (y) of the point (x, z)
float normals[terrainSize][terrainSize][3]; // normal vertices
bool definedHeights = false;
float minHeight = 0;
float maxHeight = 0;
const int iterations = 50;
int circles[iterations][4]; // used for circle algorithm
int mode = 1;               // 1 = solid polygons; 2 = wireframe; 3 = solid polygons & wireframe
bool isWireframe = false;   // used for mode 3 to draw wireframe
bool lightsOn = true;       // true: turn on two lights; false: turn off lights
bool shadingFlat = false;   // true: flat shading; false: Gouraud shading
bool isQuad = true;         // true: wireframe is quad; false: triangle
bool isCircleAlgo = true;   // true: use circle algorithm; false: use fault algorithm

/* lighting 1 */
float light_pos0[] = {50, -500, 50, 1};
float amb0[] = {0.1, 0.1, 0.1, 1};
float diff0[] = {0.8, 0.8, 0.8, 1};
float spec0[] = {0.5, 0.5, 0.5, 1};
/* lighting 2 */
float light_pos1[] = {-50, -500, -50, 1};
float amb1[] = {0.1, 0.1, 0.1, 1};
float diff1[] = {0.8, 0.8, 0.8, 1};
float spec1[] = {0.5, 0.5, 0.5, 1};
/* materials for lighting */
float m_amb[] = {0.19125, 0.0735, 0.0225, 1};
float m_diff[] = {0.7038, 0.27048, 0.0828, 1};
float m_spec[] = {0.256777, 0.137622, 0.086014, 1};
float shiny = 0.3;

/**
 * heightmap generation algorithm: Circle algorithm
 * for each terrain point (tx,tz) do
 *	pd = distance from circle center * 2 / terrainCircleSize
 *	if fabs(pd) <= 1.0
 *		height(tx,tz) +=  disp/2 + cos(pd*3.14)*disp/2;
 */
void randCircles()
{
    srand(time(NULL));
    for (int i = 0; i < iterations; i++) // 50 iterations
    {
        circles[i][0] = rand() % (terrainSize - 1); // circle vertex x
        circles[i][1] = rand() % (terrainSize - 1); // circle vertex y
        circles[i][2] = rand() % 30 - 15;    // disp: random -15 ~ 15
        circles[i][3] = rand() % 10 + 10;    // terrainCircleSize random 10 ~ 20
    }
}
void circleAlgo(int xc, int zc, int disp, int terrainCircleSize)
{
    for (int x = 1; x <= terrainSize - 1; x++)
    {
        for (int z = 1; z <= terrainSize - 1; z++)
        {
            float pd = sqrt(pow(xc - x, 2) + pow(zc - z, 2)) / terrainCircleSize;
            if (fabs(pd) <= 1.0)
                heights[x][z] += disp / 2 + cos(pd * 3.14) * disp / 2;
        }
    }
}

/**
 * heightmap generation algorithm: Fault algorithm
 */
void faultAlgo()
{
    for (int i = 0; i < 200; i++)
    {
        float r = static_cast<float>(rand());
        float d = sqrt(terrainSize * terrainSize + terrainSize * terrainSize);
        float c = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * d - d / 2;

        for (int x = 0; x < terrainSize; x++)
        {
            for (int z = 0; z < terrainSize; z++)
            {
                if (sin(r) * x + cos(r) * z - c > 0)
                    heights[x][z]++;
                else
                    heights[x][z]--;
            }
        }
    }
}

void resetHeightmap(void)
{
    for (int x = 0; x <= terrainSize - 1; x++)
        for (int z = 0; z <= terrainSize - 1; z++)
            heights[x][z] = 0;
}

void defineNormals()
{
    float x1, y1, z1, x2, y2, z2, xc, yc, zc;
    for (int x = 0; x < terrainSize - 1; x++)
    {
        for (int z = 0; z < terrainSize - 1; z++)
        {
            x1 = x + 1;
            y1 = heights[x + 1][z] - heights[x][z];
            z1 = z;

            x2 = x + 1;
            y2 = heights[x + 1][z + 1] - heights[x][z];
            z2 = z + 1;

            xc = y1 * z2 - z1 * y2;
            yc = z1 * x2 - x1 * z2;
            zc = x1 * y2 - y1 * x2;
            float n = sqrtf(xc * xc + yc * yc + zc * zc);

            normals[x][z][0] = xc / n;
            normals[x][z][1] = yc / n;
            normals[x][z][2] = zc / n;
        }
    }
}

void heightmap(void)
{
    resetHeightmap();
    if (isCircleAlgo)
    {
        randCircles();
        for (int i = 0; i < 100; i++)
        {
            circleAlgo(circles[i][0], circles[i][1], circles[i][2], circles[i][3]);
        }
    }
    else
    {
        faultAlgo();
    }
    /* define min height and max height*/
    minHeight = 0;
    maxHeight = 0;
    for (int i = 0; i < 50; i++)
    {
        for (int j = 0; j < 50; j++)
        {
            if (heights[i][j] < minHeight)
                minHeight = heights[i][j];
            else if (heights[i][j] > maxHeight)
                maxHeight = heights[i][j];
        }
    }
    definedHeights = true; // to avoid the duplication of heightmap algorithm

    defineNormals();
}

void setVertex(int x, int z)
{
    float y = heights[x + terrainSize / 2][z + terrainSize / 2]; // y is height
    float r, g, b;
    if (!isWireframe)
    {
        float percent = (y - minHeight) / (maxHeight - minHeight);
        if (percent > 0.5) // from yellow (low) to red (hight)
        {
            r = 1;
            g = 1 - (percent - 0.5) * 2;
            b = 0;
        }
        else // from green (low) to yellow (high)
        {
            r = percent * 2;
            g = 1;
            b = 0;
        }
        glColor3f(r, g, b);
    }
    else
    {
        glColor3f(1, 1, 1); // white colour for wireframe in mode 3
    }
    glNormal3fv(normals[x + terrainSize / 2][z + terrainSize / 2]); // Normals
    glVertex3f(x, y, z);
}

void set2DVertex(int x, int z)
{
    float y = heights[x + terrainSize / 2][z + terrainSize / 2]; // y is height
    float r, g, b;

    float percent = (y - minHeight) / (maxHeight - minHeight);
    if (percent > 0.5) // from yellow (low) to red (hight)
    {
        r = 1;
        g = 1 - (percent - 0.5) * 2;
        b = 0;
    }
    else // from green (low) to yellow (high)
    {
        r = percent * 2;
        g = 1;
        b = 0;
    }
    glColor3f(r, g, b);

    glVertex2d(x, -z);
}

void drawTerrain(void)
{
    if (isQuad)
        glBegin(GL_QUAD_STRIP);
    else
        glBegin(GL_TRIANGLE_STRIP);
    for (int z = -terrainSize / 2; z <= terrainSize / 2 - 2; z++)
    {
        if (z % 2 == 0)
        {
            for (int x = -terrainSize / 2; x <= terrainSize / 2 - 1; x++)
            {
                setVertex(x, z);
                setVertex(x, z + 1);
            }
        }
        else
        {
            for (int x = terrainSize / 2 - 1; x >= -terrainSize / 2; x--)
            {
                setVertex(x, z + 1);
                setVertex(x, z);
            }
        }
    }
    glEnd();
}

void draw2DOverview(void)
{
    if (isQuad)
        glBegin(GL_QUAD_STRIP);
    else
        glBegin(GL_TRIANGLE_STRIP);
    for (int z = -terrainSize / 2; z <= terrainSize / 2 - 2; z++)
    {
        if (z % 2 == 0)
        {
            for (int x = -terrainSize / 2; x <= terrainSize / 2 - 1; x++)
            {
                set2DVertex(x, z);
                set2DVertex(x, z + 1);
            }
        }
        else
        {
            for (int x = terrainSize / 2 - 1; x >= -terrainSize / 2; x--)
            {
                set2DVertex(x, z + 1);
                set2DVertex(x, z);
            }
        }
    }
    glEnd();
}

void printRotateInfo(void)
{
    printf("[INFO]Rotate: X: %i, Y: %i, Z: %i\n", angleX, angleY, angleZ);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camPos[0], camPos[1], camPos[2], 0, 0, 0, 0, 1, 0);

    if (!definedHeights)
    {
        heightmap();
    }
    /* LIGHTING */
    if (lightsOn)
    {
        glLightfv(GL_LIGHT0, GL_POSITION, light_pos0);
        glLightfv(GL_LIGHT0, GL_AMBIENT, amb0);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diff0);
        glLightfv(GL_LIGHT0, GL_SPECULAR, spec0);

        glLightfv(GL_LIGHT1, GL_POSITION, light_pos1);
        glLightfv(GL_LIGHT1, GL_AMBIENT, amb1);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, diff1);
        glLightfv(GL_LIGHT1, GL_SPECULAR, spec1);
    }
    if (lightsOn)
    {
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_diff);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
    }

    glRotatef(angleX, 1, 0, 0);
    glRotatef(angleY, 0, 1, 0);
    glRotatef(angleZ, 0, 0, 1);

    /* 3 modes */
    if (mode == 1)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawTerrain();
    }
    else if (mode == 2)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawTerrain();
    }
    else
    {
        isWireframe = true;
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawTerrain();
        isWireframe = false;
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawTerrain();
    }
    glutSwapBuffers();
}

void displayOverview(void)
{
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    draw2DOverview();
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //gluOrtho2D(0, w, 0, h);
    gluPerspective(45, (float)((w + 0.0f) / h), 1, 1000);

    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);
}

void reshapeOverview(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-50, 50, -50, 50);
    //gluPerspective(45, (float)((w + 0.0f) / h), 1, 1000);

    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);
}

void special(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_RIGHT:
        if (angleY < 90)
            angleY += 2;
        break;
    case GLUT_KEY_LEFT:
        if (angleY > -90)
            angleY -= 2;
        break;
    case GLUT_KEY_DOWN:
        if (angleX < 90)
            angleX += 2;
        break;
    case GLUT_KEY_UP:
        if (angleX > -90)
            angleX -= 2;
        break;
    }
    glutPostRedisplay();
}
void keyboardOverview(unsigned char key, int xIn, int yIn)
{
    switch (key)
    {
    case 'q':
    case 27: // esc
        exit(0);
        break;
    }
}
void keyboard(unsigned char key, int xIn, int yIn)
{
    int mod = glutGetModifiers();

    switch (key)
    {
    case 'q':
    case 27: // esc
        exit(0);
        break;
    case 'r':
        definedHeights = false;
        break;
    case 'w':
        mode++;
        mode = mode % 3;
        break;
    case 'l':
        if (lightsOn) // turn off lights
        {
            glDisable(GL_LIGHTING);
            glDisable(GL_LIGHT0);
            glDisable(GL_LIGHT1);
            lightsOn = false;
        }
        else // turn on lights
        {
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
            glEnable(GL_LIGHT1);
            lightsOn = true;
        }
        break;
    case 's':
        if (shadingFlat)
        {
            shadingFlat = false;
            glShadeModel(GL_SMOOTH);
        }
        else
        {
            shadingFlat = true;
            glShadeModel(GL_FLAT);
        }
        break;
    case 'a':
        if (angleZ > -90)
            angleZ -= 2;
        break;
    case 'd':
        if (angleZ < 90)
            angleZ += 2;
        break;
    case 't':
        isQuad = false;
        break;
    case 'y':
        isQuad = true;
        break;
    case 'f':
        isCircleAlgo = !isCircleAlgo;
        heightmap();
        break;
    case 'i':
        printRotateInfo();
        break;
    }
}

void FPS(int val)
{
    glutPostRedisplay();
    glutTimerFunc(0, FPS, 0); // 1sec = 1000, 60fps = 1000/60 = ~17
}

void init(void)
{
    glClearColor(0, 0, 0, 0);
    glColor3f(1, 1, 1);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(45, 1, 1, 1000);
}

void initOverview(void)
{
    glClearColor(0, 0, 0, 0);
    glColor3f(1, 1, 1);

    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(-50, 50, -50, 50);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void printGuide(void)
{
    std::cout << "#########################################################################\n"
              << "#                   Welcome to use this Terrain tool!                   #\n"
              << "#########################################################################\n"
              << "There are two windows: one is 3D Model, the other is 2D overview.\n"
              << "**Attention** To show synchronized 2D overview, you should select the small window (Windows) or press any key except q or esc (MacOS).\n"
              << "Unfortunately, the 2D overview is not synchronized immediately with 3D model. :(\n"
              << "-------------------------------------------------------------------------\n"
              << "A list of commands (you should select the big window at first):\n"
              << "left/right"
              << "-> rotate the 3D model about y axis\n"
              << "up/down   "
              << "-> rotate the 3D model about x axis\n"
              << "   a/d\t"
              << "-> rotate the 3D model about z axis\n"
              << "     i\t"
              << "-> print current rotation info\n"
              << "     r\t"
              << "-> generate a new random terrain using current heightmap algorithm\n"
              << "     f\t"
              << "-> toggle current algorithm between circle algorithm and fault algorithm, the default algorithm is circle algorithm\n"
              << "     y\t"
              << "-> draw the terrain using quads (strips)\n"
              << "     t\t"
              << "-> draw the terrain using triangles (strips)\n"
              << "     w\t"
              << "-> toggle wireframe mode between three options: 1. solid polygons; 2. wireframe; 3. both solid polygons and wireframe\n"
              << "     l\t"
              << "-> turn on/off the lights\n"
              << "     s\t"
              << "-> toggle current shading between flat shading and Gouraud shading, the default shading is Gouraud shading\n"
              << " q/esc\t"
              << "-> close the windows\n"
              << "################################# Terrain #################################" << std::endl;
}

int main(int argc, char **argv)
{
    printGuide();
    glutInit(&argc, argv);

    /* 3D Model */
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Terrain");

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, FPS, 0);

    /* lighting */
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    glShadeModel(GL_SMOOTH);

    /* enable backface culling */
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    init();

    /* 2D overview */
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(300, 300);
    glutInitWindowPosition(1000, 500);
    glutCreateWindow("2D Overview");

    glutDisplayFunc(displayOverview);
    glutKeyboardFunc(keyboardOverview);
    glutSpecialFunc(special);
    glutReshapeFunc(reshapeOverview);
    glutTimerFunc(0, FPS, 0);
    initOverview();

    glutMainLoop();
    return (0);
}
