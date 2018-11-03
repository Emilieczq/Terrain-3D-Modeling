#include <stdio.h>
#include <stdlib.h>
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

/**
 * camera location
 */
float camPos[] = {0, 0, 200};
int angleX = 44;
int angleY = -20;
int angleZ = 0;
const int SIZE = 150;
float height[SIZE][SIZE]; //[x][y], float value is the height in the point (x,y)
bool definedHeights = false;
float minHeight = 0;
float maxHeight = 0;
int circles[100][4];
int mode = 1; // 1 = solid polygons; 2 = wireframe; 3 = solid polygons & wireframe
bool isWireframe = false;
bool lightsOn = true;
bool shadingFlat = false;
bool isQuad = true;

/* lighting 1 */
float light_pos0[] = {150, -1000, 150, 1};
float amb0[] = {0.1, 0.1, 0.1, 1};
float diff0[] = {0.8, 0.8, 0.8, 1};
float spec0[] = {0.5, 0.5, 0, 1};
/* lighting 2 */
float light_pos1[] = {-150, -1000, -150, 1};
float amb1[] = {0.1, 0.1, 0.1, 1};
float diff1[] = {0.8, 0.8, 0.8, 1};
float spec1[] = {0, 0, 0.5, 1};

// float m_amb[] = {0.19125, 0.0735, 0.0225, 1};
// float m_diff[] = {0.7038, 0.27048, 0.0828, 1};
// float m_spec[] = {0.256777, 0.137622, 0.086014, 1};
// float shiny = 27;

float m_amb[] = {0.83, 0.52, 0.63, 1.0};
    float m_diff[] = {0.28, 0.87, 0.11, 1.0};
    float m_spec[] = {0.99, 0.91, 0.81, 1.0};
    float shiny = 27;

void randCircles()
{
    srand(time(NULL));
    for (int i = 0; i < 50; i++) // 50 interations
    {
        circles[i][0] = rand() % (SIZE - 1); // circle vertex x
        circles[i][1] = rand() % (SIZE - 1); // circle vertex y
        circles[i][2] = rand() % 50 - 25;    // disp
        circles[i][3] = rand() % 10 + 40;    // terrainCircleSize
    }
}

/**
 * heightmap generation algorithm
 * for each terrain point (tx,tz) do
 *	pd = distance from circle center * 2 / terrainCircleSize
 *	if fabs(pd) <= 1.0
 *		height(tx,tz) +=  disp/2 + cos(pd*3.14)*disp/2;
 */
void circleAlgo(int xc, int zc, int disp, int terrainCircleSize)
{
    for (int x = 0; x <= SIZE - 1; x++)
    {
        for (int z = 0; z <= SIZE - 1; z++)
        {
            float pd = sqrt(pow(xc - x, 2) + pow(zc - z, 2)) / terrainCircleSize;
            if (fabs(pd) <= 1.0)
            {
                height[x][z] += disp / 2 + cos(pd * 3.14) * disp / 2;
            }
        }
    }
}

void resetHeightmap(void)
{
    for (int x = 0; x <= SIZE - 1; x++)
    {
        for (int z = 0; z <= SIZE - 1; z++)
        {
            height[x][z] = 0;
        }
    }
}

void heightmap(void)
{
    resetHeightmap();
    randCircles();
    for (int i = 0; i < 100; i++)
    {
        circleAlgo(circles[i][0], circles[i][1], circles[i][2], circles[i][3]);
    }
    /* define min height and max height*/
    for (int i = 0; i < 50; i++)
    {
        for (int j = 0; j < 50; j++)
        {
            if (height[i][j] < minHeight)
                minHeight = height[i][j];
            else if (height[i][j] > maxHeight)
                maxHeight = height[i][j];
        }
    }
    definedHeights = true;
}

float percentColour(float height)
{
    return (height - minHeight) / (maxHeight - minHeight); // (1,0,0) => (0,1,0) => (0,0,1)
}

void setVertex(int x, int z)
{
    float y = height[x + SIZE / 2][z + SIZE / 2]; // y is height
    float r, g, b;
    if (!isWireframe)
    {
        r = (y - minHeight) / (maxHeight - minHeight);
        g = (1 - r) / 2;
        b = (1 - r) / 2;
        glColor3f(r, g, b);
    }
    else
    {
        glColor3f(1, 1, 1);
    }
    glNormal3f(1, 1, 1); // Normals
    glVertex3f(x, y, z);
}

void drawTerrain(void)
{
    if (isQuad)
        glBegin(GL_QUAD_STRIP);
    else
        glBegin(GL_TRIANGLE_STRIP);
    for (int z = -SIZE / 2; z <= SIZE / 2 - 2; z++)
    {
        if (z % 2 == 0)
        {
            for (int x = -SIZE / 2; x <= SIZE / 2 - 1; x++)
            {
                setVertex(x, z);
                setVertex(x, z + 1);
            }
        }
        else
        {
            for (int x = SIZE / 2 - 1; x >= -SIZE / 2; x--)
            {
                setVertex(x, z + 1);
                setVertex(x, z);
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

    /* LIGHTING */
    glPushMatrix();
    if (lightsOn)
    {
        glLightfv(GL_LIGHT0, GL_POSITION, light_pos0);
        // glLightfv(GL_LIGHT0, GL_AMBIENT, amb0);
        // glLightfv(GL_LIGHT0, GL_DIFFUSE, diff0);
        // glLightfv(GL_LIGHT0, GL_SPECULAR, spec0);

        glLightfv(GL_LIGHT1, GL_POSITION, light_pos1);
        // glLightfv(GL_LIGHT1, GL_AMBIENT, amb1);
        // glLightfv(GL_LIGHT1, GL_DIFFUSE, diff1);
        // glLightfv(GL_LIGHT1, GL_SPECULAR, spec1);
    }
    glPopMatrix();
    if (!definedHeights)
    {
        heightmap();
    }
    /* 3D Model */
    glPushMatrix();
    glRotatef(angleX, 1, 0, 0);
    glRotatef(angleY, 0, 1, 0);
    glRotatef(angleZ, 0, 0, 1);

    /* lighting */
    if (lightsOn)
    {
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_diff);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
    }

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
    
    glPopMatrix();
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

void mouse(int btn, int state, int x, int y){
	if(btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		printf("left button, %i, %i\n", x, y);
	}
}

void special(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_RIGHT:
        if (angleY < 90)
            angleY += 2;
        printRotateInfo();
        break;
    case GLUT_KEY_LEFT:
        if (angleY > -90)
        {
            angleY -= 2;
            printRotateInfo();
        }
        break;
    case GLUT_KEY_DOWN:
        if (angleX < 90)
        {
            angleX += 2;
            printRotateInfo();
        }
        break;
    case GLUT_KEY_UP:
        if (angleX > -90)
        {
            angleX -= 2;
            printRotateInfo();
        }
        break;
    }
    glutPostRedisplay();
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
        {
            angleZ -= 2;
            printRotateInfo();
        }
        break;
    case 'd':
        if (angleZ < 90)
        {
            angleZ += 2;
            printRotateInfo();
        }
        break;
    case 't':
        isQuad = false;
        break;
    case 'y':
        isQuad = true;
        break;
    }
}

void FPS(int val)
{
    glutPostRedisplay();
    glutTimerFunc(17, FPS, 0); // 1sec = 1000, 60fps = 1000/60 = ~17
}

void init(void)
{
    glClearColor(0, 0, 0, 0);
    glColor3f(1, 1, 1);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(45, 1, 1, 1000);
    //glOrtho(-10, 10, -10, 10, -10, 10);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(1000, 1000);
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

    glutMainLoop();

    return (0);
}
