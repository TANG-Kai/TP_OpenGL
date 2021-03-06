// ----------------------------------------------
// Informatique Graphique 3D & R�alit� Virtuelle.
// Travaux Pratiques
// Introduction � OpenGL
// Copyright (C) 2015 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <GL/glut.h>

using namespace std;

// App parameters
static const unsigned int DEFAULT_SCREENWIDTH = 1024;
static const unsigned int DEFAULT_SCREENHEIGHT = 768;
static string appTitle ("Informatique Graphique & Realite Virtuelle - Travaux Pratiques - Introduction a OpenGL");
static GLint window;
static unsigned int screenWidth;
static unsigned int screenHeight;

// Camera parameters
static float fovAngle;
static float aspectRatio;
static float nearPlane;
static float farPlane;
static float camPhi; // Expressing the camera position in polar coordinate, in the frame of the target
static float camTheta;
static float camDist2Target;
static float camTargetX;
static float camTargetY;
static float camTargetZ;

static int x_ancient = -1;
static int y_ancient = -1;

// Texture parameter
static int height = 200;
static int width = 400;

static GLuint texture;

//time parameter
static float currentTime;
static float acceleration = 0.0;

#define glAddTriV(i,j,d){\
    double theta = i/height * M_PI;\
    double phi = j/width* 2*M_PI;\
    float x;\
    float y;\
    float z;\
    polar2Cartesian(phi,theta,d,x,y,z);\
    glColor3f(x,y,z);\
    glTexCoord2f(i,j);\
    glVertex3f(x,y,z);}

void polar2Cartesian (float phi, float theta, float d, float & x, float & y, float & z) {
    x = d*sin (theta) * cos (phi);
    y = d*cos (theta);
    z = d*sin (theta) * sin (phi);
}

void printUsage () {
    std::cerr << std::endl // send a line break to the standard error output
              << appTitle << std::endl
              << "Author : Tamy Boubekeur" << std::endl << std::endl
              << "Usage : ./main [<file.off>]" << std::endl
              << "Cammandes clavier :" << std::endl
              << "------------------" << std::endl
              << " ?: Print help" << std::endl
              << " w: Toggle wireframe mode" << std::endl
              << " <drag>+<left button>: rotate model" << std::endl
              << " <drag>+<right button>: move model" << std::endl
              << " <drag>+<middle button>: zoom" << std::endl
              << " q, <esc>: Quit" << std::endl << std::endl;
}

void genCheckerboard(unsigned int width, unsigned int height, unsigned char * image){//color_with_number_c__of_i_j(i,j,c)=f(c+3xj + widx3*i)
    for(unsigned int i=0;i<height;i++)
        for(unsigned int j=0;j<width;j++)
        {
            if((i+j)%2 ==0) //rouge
            {
                image[((width*i + j)*3 )%(height*width*3)] =255;
                image[((width*i + j)*3+ 1)%(height*width*3)]=0;
                image[((width*i + j)*3 + 2)%(height*width*3)]= 0;
            }
            else //bleu
            {
                image[((width*i + j)*3 )%(height*width*3)] =255;
                image[((width*i+j)*3+1)%(height*width*3)]=255;
                image[((width*i + j)*3 + 2)%(height*width*3)]=255;
            }
        }
}

void init () {
    // OpenGL initialization
    glCullFace (GL_BACK);     // Specifies the faces to cull (here the ones pointing away from the camera)
    glEnable (GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
    glDepthFunc (GL_LESS); // Specify the depth test for the z-buffer
    glEnable (GL_DEPTH_TEST); // Enable the z-buffer in the rasterization
    glLineWidth (2.0); // Set the width of edges in GL_LINE polygon mode
    glClearColor (0.0f, 0.0f, 0.0f, 1.0f); // Background color
    //Texture initialization
    int width = 100;
    int height = 100;
    unsigned char * image = new unsigned char[width*height*3];
    genCheckerboard(width,height,image);
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1,&texture);
    glBindTexture(GL_TEXTURE_2D,texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,image);
    //Ligntening initialization
    glEnable(GL_LIGHTING);
    GLfloat light_position[4] = {10.0f, 10.0f, 10.0f, 1.0f};
    GLfloat color[4] = {1.0f, 0.9f, 0.8f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
    glLightfv(GL_LIGHT0, GL_SPECULAR, color);
    glEnable(GL_LIGHT0);
    GLfloat light_position1[4] = {-10.0f, 0.0f, -1.0f, 1.0f};
    GLfloat color1[4] = {0.0f, 0.1f, 0.3f, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, color1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, color1);
    glEnable(GL_LIGHT1);
    // Camera initialization
    fovAngle = 45.f;
    nearPlane = 0.01;
    farPlane = 10.0;
    camPhi = M_PI/2.0;
    camTheta = M_PI/2.0;
    camDist2Target = 5.0;
    camTargetX = 0.0;
    camTargetY = 0.0;
    camTargetZ = 0.0;
}

void setupCamera () {
    glMatrixMode (GL_PROJECTION); // Set the projection matrix as current. All upcoming matrix manipulations will affect it.
    glLoadIdentity ();
    gluPerspective (fovAngle, aspectRatio, nearPlane, farPlane); // Set the current projection matrix with the camera intrinsics
    glMatrixMode (GL_MODELVIEW); // Set the modelview matrix as current. All upcoming matrix manipulations will affect it.
    glLoadIdentity ();
    float camPosX, camPosY, camPosZ;
    polar2Cartesian (camPhi, camTheta, camDist2Target, camPosX, camPosY, camPosZ);
    camPosX += camTargetX;
    camPosY += camTargetY;
    camPosZ += camTargetZ;
    gluLookAt (camPosX, camPosY, camPosZ, camTargetX, camTargetY, camTargetZ, 0.0, 1.0, 0.0); // Set up the current modelview matrix with camera transform
}

void reshape (int w, int h) {
    screenWidth = w;
    screenHeight = h;
    aspectRatio = static_cast<float>(w)/static_cast<float>(h);
    glViewport (0, 0, (GLint)w, (GLint)h); // Dimension of the drawing region in the window
    setupCamera ();
}

void glSphere(float x,float y, float z, float radius){
    double const r = radius;
    //double const delta = M_PI / _num_of_points;
    // set an offset
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(x,y,z);
    glBegin(GL_TRIANGLES);
    double const d = r;
    for(int i=0;i<height-1;i++)
        for(int j=0;j<width-1;j++){
            glAddTriV(i,j,d);
            glAddTriV(i,j+1,d);
            glAddTriV(i+1,j,d);

            glAddTriV(i+1,j,d);
            glAddTriV(i,j+1,d);
            glAddTriV(i+1,j+1,d);
        }
    glEnd();
    glPopMatrix();
}
void glSphereWithMat(float x,float y, float z, float radius,
                     float difR, float difG, float difB, float specR, float specG, float specB, float shininess){
    // set meterial
    GLfloat material_color[4] = {difR,difG,difB,1.0f};
    GLfloat material_specular[4] = {specR,specG,specB,1.0};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_color);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    glBindTexture(GL_TEXTURE_2D,texture);
    glSphere(x,y,z,radius);
}
void display () {
    setupCamera ();
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.
// Put your drawing code (glBegin, glVertex, glCallList, glDrawArray, etc) here
//    for(int i=0;i<3;i++){
//        for(int j=0;j<3;j++){
//            glSphereWithMat(i-1,j-1,0,0.5,0.1*i,0.1*j,0.5,0.1*i,0.1*j,0.5,0.6);
//        }
//    }
//    for(int i=0;i<2;i++)
//        for(int j=0;j<2;j++)
//            glSphereWithMat(i-0.5,j-0.5,1.2,0.5,0.1*i,0.2*j,0.5,0.2*i,0.1*j,0.5,0.7);
//    glSphereWithMat(0,0,1.9,0.5,0.1,0.2,0.5,0.2,0.1,0.3,0.5);
    //a sphere that goes
    double bouge = ((int)(currentTime*(1+acceleration))/50)%100-50;//-50~50
    double ct =sin(bouge/50*M_PI);
    glSphereWithMat(ct,0,0,0.5,0.1,0.2,0.5,0.2,0.1,0.3,0.5);
    glFlush (); // Ensures any previous OpenGL call has been executed
    glutSwapBuffers ();  // swap the render buffer and the displayed (screen) one
}
void keyboard (unsigned char keyPressed, int x, int y) {
    switch (keyPressed) {
    case 'w':
        GLint mode[2];
        glGetIntegerv (GL_POLYGON_MODE, mode);
        glPolygonMode (GL_FRONT_AND_BACK, mode[1] ==  GL_FILL ? GL_LINE : GL_FILL);
        break;
    case '+':
        acceleration +=0.1;
        break;
    case '-':
        acceleration -=0.1;
        if(acceleration<-1) acceleration = -1;
        break;
    case 'q':
    case 27:
        exit (0);
        break;
    default:
        printUsage ();
        break;
    }
    glutPostRedisplay ();
}

void mouse (int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
            if (state == GLUT_UP) {
                    x_ancient = -1;
                    y_ancient = -1;
            }
            else  {// state = GLUT_DOWN
                    x_ancient = x;
                    y_ancient = y;
            }
    }

}

void motion (int x, int y) {

    // left button is down
    if (x_ancient >= 0) {

            // update camera's direction
            camPhi+=(x - x_ancient) * 0.0001f;
            camTheta+=(y - y_ancient) * 0.0001f;
    }

}

// This function is executed in an infinite loop. It updated the window title
// (frame-per-second, model size) and ask for rendering
void idle () {
    currentTime = glutGet((GLenum)GLUT_ELAPSED_TIME);
    glutPostRedisplay ();
}

int main (int argc, char ** argv) {
    glutInit (&argc, argv); // Initialize a glut app
    glutInitDisplayMode (GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE); // Setup a RGBA framebuffer to display, with a depth buffer (z-buffer), in double buffer mode (fill a buffer then update the screen)
    glutInitWindowSize (DEFAULT_SCREENWIDTH, DEFAULT_SCREENHEIGHT); // Set the window app size on screen
    window = glutCreateWindow (appTitle.c_str ()); // create the window
    init (); // Your initialization code (OpenGL states, geometry, material, lights, etc)
    glutReshapeFunc (reshape); // Callback function executed whenever glut need to setup the projection matrix
    glutDisplayFunc (display); // Callback function executed when the window app need to be redrawn
    glutKeyboardFunc (keyboard); // Callback function executed when the keyboard is used
    glutMouseFunc (mouse); // Callback function executed when a mouse button is clicked
    glutMotionFunc (motion); // Callback function executed when the mouse move
    glutIdleFunc (idle); // Callback function executed continuously when no other event happens (good for background procesing or animation for instance).
    printUsage (); // By default, display the usage help of the program
    glutMainLoop ();
    return 0;
}
