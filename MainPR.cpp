/*
    Interactive Solar System

    Controls:
    - Mouse Left Click : Select / enlarge a planet
    - Space            : Pause / Resume animation
    - Arrow Keys       : Rotate camera
    - + / -            : Zoom in / Zoom out
    - R                : Reset camera
    - ESC              : Exit
*/

#define _CRT_SECURE_NO_WARNINGS

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glu.h>
#endif

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define PI 3.14159265358979323846
#define PLANET_COUNT 8

using namespace std;

struct Color {
    float r;
    float g;
    float b;
};

struct Planet {
    const char* name;
    const char* info;

    float radius;
    float orbitRadius;

    float orbitSpeed;
    float rotationSpeed;

    float orbitAngle;
    float selfAngle;

    Color color;

    const char* texturePath;
    GLuint textureId;
    bool hasTexture;
};

Planet planets[PLANET_COUNT];

int selectedPlanet = -1;
bool isPaused = false;

float cameraDistance = 48.0f;
float cameraAngleX = 22.0f;
float cameraAngleY = -28.0f;

int windowWidth = 1200;
int windowHeight = 720;

GLUquadric* sphereQuadric = NULL;

/* Sun Texture */
GLuint sunTextureId = 0;
bool sunHasTexture = false;

/* =========================================================
   Texture Functions
   ========================================================= */

GLuint createFallbackTexture(Color color) {
    const int size = 64;
    unsigned char data[size][size][3];

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            float shade;

            if (((x / 8) + (y / 8)) % 2 == 0)
                shade = 1.0f;
            else
                shade = 0.72f;

            data[y][x][0] = (unsigned char)(color.r * 255.0f * shade);
            data[y][x][1] = (unsigned char)(color.g * 255.0f * shade);
            data[y][x][2] = (unsigned char)(color.b * 255.0f * shade);
        }
    }

    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        size,
        size,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        data
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureId;
}

unsigned int readUInt32(unsigned char* header, int offset) {
    unsigned int value = 0;

    value =
        header[offset] |
        (header[offset + 1] << 8) |
        (header[offset + 2] << 16) |
        (header[offset + 3] << 24);

    return value;
}

unsigned short readUInt16(unsigned char* header, int offset) {
    unsigned short value = 0;

    value =
        header[offset] |
        (header[offset + 1] << 8);

    return value;
}

GLuint LoadTexture(const char* filename, bool& success) {
    success = false;

    FILE* file = fopen(filename, "rb");

    if (file == NULL) {
        cout << "Texture not found: " << filename << endl;
        return 0;
    }

    unsigned char header[54];

    if (fread(header, 1, 54, file) != 54) {
        fclose(file);
        return 0;
    }

    if (header[0] != 'B' || header[1] != 'M') {
        fclose(file);
        return 0;
    }

    unsigned int dataOffset = readUInt32(header, 0x0A);
    unsigned int width = readUInt32(header, 0x12);
    unsigned int height = readUInt32(header, 0x16);
    unsigned short bitsPerPixel = readUInt16(header, 0x1C);

    if (bitsPerPixel != 24) {
        fclose(file);
        return 0;
    }

    unsigned int rowPadded = (width * 3 + 3) & (~3);
    unsigned int imageSize = rowPadded * height;

    unsigned char* rawData = new unsigned char[imageSize];
    unsigned char* imageData = new unsigned char[width * height * 3];

    fseek(file, dataOffset, SEEK_SET);
    fread(rawData, 1, imageSize, file);
    fclose(file);

    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            unsigned int rawIndex = y * rowPadded + x * 3;
            unsigned int imageIndex = ((height - 1 - y) * width + x) * 3;

            imageData[imageIndex + 0] = rawData[rawIndex + 2];
            imageData[imageIndex + 1] = rawData[rawIndex + 1];
            imageData[imageIndex + 2] = rawData[rawIndex + 0];
        }
    }

    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        width,
        height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        imageData
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    delete[] rawData;
    delete[] imageData;

    success = true;
    return textureId;
}

/* =========================================================
   Text Functions
   ========================================================= */

void drawText2D(float x, float y, const char* text, void* font) {
    glRasterPos2f(x, y);

    for (int i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter(font, text[i]);
    }
}

void setup2D() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    gluOrtho2D(0, windowWidth, 0, windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
}

void restore3D() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
}

/* =========================================================
   Initialization
   ========================================================= */

void initializePlanets() {
    planets[0] = {
        "Mercury",
        "Mercury: The smallest planet and closest to the Sun.",
        0.45f,
        4.0f,
        0.45f,
        0.70f,
        0.0f,
        0.0f,
        {0.70f, 0.68f, 0.62f},
        "../assets/mercury.bmp",
        0,
        false
    };

    planets[1] = {
        "Venus",
        "Venus: Very hot planet with a thick atmosphere.",
        0.70f,
        6.0f,
        0.35f,
        0.50f,
        30.0f,
        0.0f,
        {0.95f, 0.72f, 0.35f},
        "../assets/venus.bmp",
        0,
        false
    };

    planets[2] = {
        "Earth",
        "Earth: Our home planet. It contains water and life.",
        0.78f,
        8.3f,
        0.28f,
        0.80f,
        70.0f,
        0.0f,
        {0.20f, 0.45f, 0.95f},
        "../assets/earth.bmp",
        0,
        false
    };

    planets[3] = {
        "Mars",
        "Mars: The red planet, known for its dusty surface.",
        0.62f,
        10.7f,
        0.22f,
        0.70f,
        120.0f,
        0.0f,
        {0.90f, 0.28f, 0.15f},
        "../assets/mars.bmp",
        0,
        false
    };

    planets[4] = {
        "Jupiter",
        "Jupiter: The largest planet in the Solar System.",
        1.55f,
        14.0f,
        0.14f,
        1.00f,
        180.0f,
        0.0f,
        {0.86f, 0.61f, 0.38f},
        "../assets/jupiter.bmp",
        0,
        false
    };

    planets[5] = {
        "Saturn",
        "Saturn: Famous for its beautiful ring system.",
        1.25f,
        17.8f,
        0.10f,
        0.90f,
        240.0f,
        0.0f,
        {0.89f, 0.78f, 0.50f},
        "../assets/saturn.bmp",
        0,
        false
    };

    planets[6] = {
        "Uranus",
        "Uranus: An ice giant with a pale blue color and tilted rotation.",
        1.05f,
        21.2f,
        0.075f,
        0.75f,
        300.0f,
        0.0f,
        {0.45f, 0.85f, 0.95f},
        "../assets/uranus.bmp",
        0,
        false
    };

    planets[7] = {
        "Neptune",
        "Neptune: A distant blue planet with strong winds and storms.",
        1.00f,
        24.5f,
        0.055f,
        0.72f,
        340.0f,
        0.0f,
        {0.20f, 0.35f, 0.95f},
        "../assets/neptune.bmp",
        0,
        false
    };
}

void initializeTextures() {
    sunTextureId = LoadTexture("../assets/sun.bmp", sunHasTexture);

    for (int i = 0; i < PLANET_COUNT; i++) {
        planets[i].textureId = LoadTexture(planets[i].texturePath, planets[i].hasTexture);

        if (!planets[i].hasTexture) {
            planets[i].textureId = createFallbackTexture(planets[i].color);
        }
    }
}

void initializeLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    GLfloat lightPosition[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat lightAmbient[] = { 0.18f, 0.18f, 0.20f, 1.0f };
    GLfloat lightDiffuse[] = { 1.0f, 0.92f, 0.78f, 1.0f };
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
}

void init() {
    glClearColor(0.01f, 0.01f, 0.04f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_NORMALIZE);

    glShadeModel(GL_SMOOTH);

    sphereQuadric = gluNewQuadric();
    gluQuadricTexture(sphereQuadric, GL_TRUE);
    gluQuadricNormals(sphereQuadric, GLU_SMOOTH);

    initializePlanets();
    initializeTextures();
    initializeLighting();
}

/* =========================================================
   Drawing Functions
   ========================================================= */



void drawSun() {
    glDisable(GL_LIGHTING);

    if (sunHasTexture) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, sunTextureId);
        glColor3f(1.0f, 1.0f, 1.0f);

        gluSphere(sphereQuadric, 1.8, 64, 64);
    }
    else {
        glDisable(GL_TEXTURE_2D);

        glColor3f(1.0f, 0.72f, 0.08f);
        glutSolidSphere(1.8, 64, 64);
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

void drawSaturnRing() {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glColor3f(0.80f, 0.72f, 0.50f);

    glBegin(GL_QUAD_STRIP);

    for (int i = 0; i <= 160; i++) {
        float angle = (float)(2.0 * PI * i / 160.0);

        float x1 = cos(angle) * 1.55f;
        float z1 = sin(angle) * 1.55f;

        float x2 = cos(angle) * 2.15f;
        float z2 = sin(angle) * 2.15f;

        glVertex3f(x1, 0.0f, z1);
        glVertex3f(x2, 0.0f, z2);
    }

    glEnd();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
}

void drawUranusRing() {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glColor3f(0.55f, 0.90f, 0.95f);

    glBegin(GL_LINE_LOOP);

    for (int i = 0; i < 160; i++) {
        float angle = (float)(2.0 * PI * i / 160.0);

        float x = cos(angle) * 1.55f;
        float z = sin(angle) * 1.55f;

        glVertex3f(x, 0.0f, z);
    }

    glEnd();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
}

void drawPlanet(Planet planet, int index) {
    glPushMatrix();

    glRotatef(planet.orbitAngle, 0.0f, 1.0f, 0.0f);
    glTranslatef(planet.orbitRadius, 0.0f, 0.0f);
    glRotatef(planet.selfAngle, 0.0f, 1.0f, 0.0f);

    if (index == selectedPlanet) {
        glScalef(1.75f, 1.75f, 1.75f);
    }

    glColor3f(planet.color.r, planet.color.g, planet.color.b);
    glBindTexture(GL_TEXTURE_2D, planet.textureId);

    gluSphere(sphereQuadric, planet.radius, 64, 64);

    if (index == 5) {
        glRotatef(18.0f, 1.0f, 0.0f, 0.0f);
        drawSaturnRing();
    }

    if (index == 6) {
        glRotatef(75.0f, 1.0f, 0.0f, 0.0f);
        drawUranusRing();
    }

    glPopMatrix();
}

void drawHUD() {
    setup2D();

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText2D(
        24.0f,
        (float)(windowHeight - 34),
        "Interactive Solar System",
        GLUT_BITMAP_HELVETICA_18
    );

    glColor3f(0.78f, 0.84f, 1.0f);
    drawText2D(
        24.0f,
        (float)(windowHeight - 64),
        "Mouse: Select Planet | Space: Pause | Arrows: Camera | +/-: Zoom | R: Reset",
        GLUT_BITMAP_HELVETICA_12
    );

    if (selectedPlanet >= 0) {
        glColor3f(1.0f, 0.88f, 0.35f);
        drawText2D(
            24.0f,
            64.0f,
            planets[selectedPlanet].name,
            GLUT_BITMAP_HELVETICA_18
        );

        glColor3f(0.92f, 0.92f, 0.92f);
        drawText2D(
            24.0f,
            38.0f,
            planets[selectedPlanet].info,
            GLUT_BITMAP_HELVETICA_12
        );
    }
    else {
        glColor3f(0.92f, 0.92f, 0.92f);
        drawText2D(
            24.0f,
            38.0f,
            "Click on any planet to enlarge it and show information.",
            GLUT_BITMAP_HELVETICA_12
        );
    }

    restore3D();
}

/* =========================================================
   Animation and Selection
   ========================================================= */

void updateAnimation() {
    if (!isPaused) {
        for (int i = 0; i < PLANET_COUNT; i++) {
            planets[i].orbitAngle += planets[i].orbitSpeed;
            planets[i].selfAngle += planets[i].rotationSpeed;

            if (planets[i].orbitAngle > 360.0f)
                planets[i].orbitAngle -= 360.0f;

            if (planets[i].selfAngle > 360.0f)
                planets[i].selfAngle -= 360.0f;
        }
    }

    glutPostRedisplay();
}

void getPlanetScreenPosition(Planet planet, double& screenX, double& screenY) {
    double modelView[16];
    double projection[16];
    int viewport[4];

    glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    double angleRad = planet.orbitAngle * PI / 180.0;

    double worldX = cos(angleRad) * planet.orbitRadius;
    double worldY = 0.0;
    double worldZ = -sin(angleRad) * planet.orbitRadius;

    double winZ;

    gluProject(
        worldX,
        worldY,
        worldZ,
        modelView,
        projection,
        viewport,
        &screenX,
        &screenY,
        &winZ
    );

    screenY = windowHeight - screenY;
}

void selectPlanetByMouse(int mouseX, int mouseY) {
    double minDistance = 999999.0;
    int nearestPlanet = -1;

    for (int i = 0; i < PLANET_COUNT; i++) {
        double sx;
        double sy;

        getPlanetScreenPosition(planets[i], sx, sy);

        double dx = mouseX - sx;
        double dy = mouseY - sy;

        double distance = sqrt(dx * dx + dy * dy);
        double clickRadius = 24.0 + planets[i].radius * 20.0;

        if (distance < clickRadius && distance < minDistance) {
            minDistance = distance;
            nearestPlanet = i;
        }
    }

    selectedPlanet = nearestPlanet;
}

/* =========================================================
   Callback Functions
   ========================================================= */

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -cameraDistance);
    glRotatef(cameraAngleX, 1.0f, 0.0f, 0.0f);
    glRotatef(cameraAngleY, 0.0f, 1.0f, 0.0f);

    GLfloat lightPosition[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    
    drawSun();

   

    for (int i = 0; i < PLANET_COUNT; i++) {
        drawPlanet(planets[i], i);
    }

    drawHUD();

    glutSwapBuffers();
}

void reshapeFunc(int width, int height) {
    windowWidth = width;
    windowHeight = height;

    if (height == 0)
        height = 1;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(
        45.0,
        (double)width / (double)height,
        1.0,
        150.0
    );

    glMatrixMode(GL_MODELVIEW);
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        selectPlanetByMouse(x, y);
        glutPostRedisplay();
    }
}

void key(unsigned char key, int x, int y) {
    switch (key) {
    case 27:
        exit(0);
        break;

    case ' ':
        isPaused = !isPaused;
        break;

    case '+':
    case '=':
        cameraDistance -= 2.0f;
        if (cameraDistance < 14.0f)
            cameraDistance = 14.0f;
        break;

    case '-':
    case '_':
        cameraDistance += 2.0f;
        if (cameraDistance > 85.0f)
            cameraDistance = 85.0f;
        break;

    case 'r':
    case 'R':
        cameraDistance = 48.0f;
        cameraAngleX = 22.0f;
        cameraAngleY = -28.0f;
        selectedPlanet = -1;
        break;
    }

    glutPostRedisplay();
}

void special(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_LEFT:
        cameraAngleY -= 4.0f;
        break;

    case GLUT_KEY_RIGHT:
        cameraAngleY += 4.0f;
        break;

    case GLUT_KEY_UP:
        cameraAngleX -= 4.0f;
        break;

    case GLUT_KEY_DOWN:
        cameraAngleX += 4.0f;
        break;
    }

    glutPostRedisplay();
}

/* =========================================================
   Main Function
   ========================================================= */

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Interactive Solar System");

#ifndef __APPLE__
    glewInit();
#endif

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshapeFunc);
    glutMouseFunc(mouse);
    glutKeyboardFunc(key);
    glutSpecialFunc(special);
    glutIdleFunc(updateAnimation);

    glutMainLoop();

    return 0;
}