/* -------------------------------------------------------- */
/* -------------------------------------------------------- */
/*

TP OpenGL - Image Synthesis course

George Drettakis, Adrien Bousseau, Sylvain Lefebvre

mailto: FirstName.LastName@inria.fr

REVES / INRIA

History:
- Adapted to OpenGL 3, 2014-02 (AB)

Documentation:
- OpenGL tutorial, provides detailed instructions but relies on a different representation of matrices and 3D models
http://www.opengl-tutorial.org/beginners-tutorials/
- GLSL reference
https://www.opengl.org/documentation/glsl/


Questions:

Q0
  - Launch the application: what do you see? 
  - Make the window as big as possible to see the little views.

  **Answer 1:** Window contains 9 * 9 Matrix in which every position of the image is displayed.

Q1 
  - The lightfield contains 9x9 views. Update the pixel shader to display only the central view.

** HINT ** you will need to operate on the texture coordinates. Draw a diagram of the 9x9 array of images, and remember that UV coordinates are between 0 and 1. You need to remap the coordinates that scan the 9x9 set of views so they scan only the central view.
 
** Please save the original version of the fragment shader for later use (BONUS question)

  ** Answer 2:** Central View is obtained by dividing the input fragment co-ordinate( along both x and y axis) by 9.0.


  - Implement an interactive change of view using the x and y mouse coordinates (show the top-left view when the mouse is in the top left corner, all the way to the bottom-right view when the mouse is in the bottom-right corner).

    You need to send the mouse coordinates to the shader, and then display the view accordingly.

** HINT ** You will need to use the window width and height, defined in the global variables g_W and g_H. You will also need to define new variable (in C and uniforms for the shader) for the mouse position. Use the mainMotion function to control the view, this way you will have a continuous change of viewpoint as you move the mouse.

    **Answer3:** 1) First float Mouse Co-ordinates are first declared and then they are calculated with respect to x and y axis of the width and height of the window.
	xpoint = (float(x) / g_W*9);
	ypoint = (float(y) / g_H*9);
	2) Then these values are passed as a uniform variable to mousept which contains both x and y co-ordinates
	3) In Fragment shader these values are then added with the fragment co-ordinates and then passed to the texture function along with the myTextureSampler.
 
Q2
  - Modify the shader to average all the views of the lightfield. You should now obtain a shallow depth of field where only part of the scene is sharp while other parts are blurry.
  **Answer:**
  1) Here the values of all the values are averaged across 81 points for each view by iterating across x and y axis with each having 9 points
  2) color is obtained with each value and then added.
  3) finally color value is averaged by 81 points to get the light field.

Q3
  A lightfield can be synthetically refocused by translating the views with respect to the central one (see refocusing_shift.png, where (i,j) denotes the coordinates of the view in the 9x9 grid). To implement refocusing, you need to:
 
  - Translate each view (i,j) by alpha*(i-i0, j-j0), where (i0=4, j0=4) is the central view. Alpha controls the position of the plane in focus in the scene. Varying alpha between -0.0003 and 0.0003 should be sufficient.

  - Implement an interactive change of focus using the y mouse coordinates to control alpha.

  **Answer:**
  1) Here alpha is obtained as a value of Mouse co-ordinates with respect to y axis.
  2) To Translate each view is added by alpha*vec2(i-4, j-4) after obtaining the calculation of all light views with 81 points
  3) Then average value of each point is obtained by the average of 81 points.

** HINT ** use the mainMotion function to control alpha, this way you will have a continuous change
         of focus as you move the mouse.
  
BONUS:

   You used the texture coordinates in the fragment shader to display the central view in Q1; this is required to do the remaining questions.

   However, you can also display the central view with two other methods (one involving the C++ program and another involving the vertex shader), and just use the original fragment shader you have saved. Please provide these solutions and explain what you did.

   **Answer**:
   To obtain central view as similar to Question1 the values of fragmentUV has to be pre-processed.This can done in either:
   1)Vertex Shader
   2)CPP file

   1)In the vertex shader: 
   i)We already we are passing the value of fragmentUV from vertexUV , we know we have to divide by 9 to obtain the central view
   directly performing the calculation in vertex shader instead of fragment shader.

   ii) Then we just pass the fragmentUV variable directly which will be used for the FragmentShader.

   In the CPP file:
   i)We are creating the geometry of the  g_uv_buffer_data by 12 points of the triangle passed to vertexUV in fragment shader
   by uvbuffer in location 1

   ii)These values have to be each divided by 9.0 while creating the buffer data for each of the 12 points of the two triangles
   g_uv_buffer_data[i] = (g_uv_buffer_data[i] * (1. / 9.));
   This will give us the central view with respect to our light field.

    iii) Then as usual pass the fragmentUV variable directly which will be used for the FragmentShader.
*/

#pragma warning(push, 0)

#ifdef _WIN32
#include <windows.h>        // windows API header (required by gl.h)
#include <stdlib.h>
#endif

#include "glew/include/GL/glew.h"	// Support for shaders

#ifdef __APPLE__

#include <gl3.h>          // OpenGL header
#include <gl3ext.h>       // OpenGL header
#include <glut.h>         // OpenGL Utility Toolkit header

#else

#include <GL/gl.h>          // OpenGL header
#include <GL/glu.h>         // OpenGL Utilities header
#include <GL/glut.h>        // OpenGL Utility Toolkit header

#endif

#include <cstdio>
#include <cmath>

#include "glsl.h" // GLSL Helper functions
#include "TextureLoader.h" // Structure to represent a texture

#pragma warning(pop)

//path of the shaders
#ifdef _WIN32
#define SRC_PATH	""
#else
#define SRC_PATH	""
#endif

#include <iostream>
using namespace std;

/* --------------------- Main Window ------------------- */

int          g_MainWindow; // glut Window Id
int          g_W=640;      // window width
int          g_H=480;      // window Height

float xpoint;
float ypoint;
/* --------------------- Geometry ------------------- */

//Vertex Array Object
GLuint VertexArrayID;
// This will identify our vertex buffer
GLuint vertexbuffer;
// This will identify our texture coordinate buffer
GLuint uvbuffer;

// This will identify the texture
GLuint textureID;


/*---------------------- Shaders -------------------*/
GLuint g_glslProgram;


/* -------------------------------------------------------- */

void mainKeyboard(unsigned char key, int x, int y) 
{
	if (key == 'q') {
		exit (0);
	} else if (key == ' ') {
		printf("spacebar pressed\n");
	} 

	printf("key '%c' pressed\n",key);
}

/* -------------------------------------------------------- */

void mainMouse(int btn, int state, int x, int y) 
{
	if (state == GLUT_DOWN) {
		if (btn == GLUT_LEFT_BUTTON) {
			printf("Left mouse button pressed at coordinates %d,%d\n",x,y);
		} else if (btn == GLUT_RIGHT_BUTTON) {
			printf("Right mouse button pressed at coordinates %d,%d\n",x,y);
		}
	} else if (state == GLUT_UP) {
		if (btn == GLUT_LEFT_BUTTON) {
			printf("Left mouse button release at coordinates %d,%d\n",x,y);
		} else if (btn == GLUT_RIGHT_BUTTON) {
			printf("Right mouse button release at coordinates %d,%d\n",x,y);
		}
	}
}

/* -------------------------------------------------------- */

void mainMotion(int x,int y)
{
	printf("Mouse is at %d,%d\n",x,y);
	//Question 1
	xpoint = (float(x) / g_W*9);
	ypoint = (float(y) / g_H*9);
}

/* -------------------------------------------------------- */

void mainReshape(int w,int h)
{
	printf("Resizing window to %d,%d\n",w,h);
	g_W=w;
	g_H=h;
	// set viewport to the entire window
	glViewport(0,0,g_W,g_H);
}

/* -------------------------------------------------------- */

void mainRender()
{
	// Dark background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	// clear screen and depth buffer
	glClear(GL_COLOR_BUFFER_BIT);
	
	// use our shader
	glUseProgramObjectARB(g_glslProgram);
	
	
	//--- Camera ---//
	// No camera, just draw a quad over the entire screen
	
	
	//--- Texturing: cover the quad with the image
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(g_glslProgram, "myTextureSampler");
	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	// Set our "myTextureSampler" sampler to user Texture Unit 0
	glUniform1i(TextureID, 0); 

    //Question 1
	GLuint mousepoint = glGetUniformLocation(g_glslProgram, "mousept");
	glUniform2f(mousepoint, xpoint, ypoint);
	
	
	//--- Geometry ---//
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
	  0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
	  2,                  // size, 2 coordinates per vertex (x,y)
	  GL_FLOAT,           // type
	  GL_FALSE,           // normalized?
	  0,                  // stride
	  (void*)0            // array buffer offset
	);
	
	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(
	    1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
	    2,                                // size, 2 texture coordinates (u,v)
	    GL_FLOAT,                         // type
	    GL_FALSE,                         // normalized? No
	    0,                                // stride
	    (void*)0                          // array buffer offset
	);
	
	// Draw the quad (2 triangles)
	glDrawArrays(GL_TRIANGLES, 0, 6); // Starting from vertex 0; 3 vertices total -> 2 triangle.
 
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	
	
	// swap - this call exchanges the back and front buffer
	// swap is synchronized on the screen vertical sync
	glutSwapBuffers();
}

/* -------------------------------------------------------- */

void idle( void )
{
	// whenever the application has free time, ask for a screen refresh
	glutPostRedisplay();
}

/* -------------------------------------------------------- */

void loadShaders()
{
	const char *fp_code=loadStringFromFile(SRC_PATH "SimpleFragmentShader.fp");
	const char *vp_code=loadStringFromFile(SRC_PATH "SimpleVertexShader.vp");
	g_glslProgram = createGLSLProgram(vp_code,fp_code);
	delete [](fp_code);
	delete [](vp_code);
}

void loadTexture()
{
    
    unsigned int width, height;
    unsigned char * data = loadBMPRaw("images/lego_9x9.bmp", width, height);
    // Create one OpenGL texture
    glGenTextures(1, &textureID);
    
    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
    // Poor filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    delete[] data;
}

void createGeometry()
{
	// Create a quad made of two triangles. A triangle contains 3 vertices, each vertex has 2 coordinates (screen space)
	// Position
	GLfloat* g_vertex_buffer_data;
	g_vertex_buffer_data = new GLfloat[6*2];
	//bottom right triangle. Note: the OpenGL screen covers [-1;1] x [-1;1]
	g_vertex_buffer_data[0] = -1.0f; g_vertex_buffer_data[1] = -1.0f; 
	g_vertex_buffer_data[2] = 1.0f; g_vertex_buffer_data[3] = -1.0f; 
	g_vertex_buffer_data[4] = 1.0f; g_vertex_buffer_data[5] = 1.0f; 
	//top left triangle
	g_vertex_buffer_data[6] = -1.0f; g_vertex_buffer_data[7] = -1.0f; 
	g_vertex_buffer_data[8] = 1.0f; g_vertex_buffer_data[9] = 1.0f; 
	g_vertex_buffer_data[10] = -1.0f; g_vertex_buffer_data[11] = 1.0f; 
	
	// UVs
	GLfloat* g_uv_buffer_data;
	g_uv_buffer_data = new GLfloat[6*2];
	//bottom right triangle. Set the coordinates so that the quad covers [0;1] x [0;1]
	g_uv_buffer_data[0] = 0.0f; g_uv_buffer_data[1] = 1.0f;
	g_uv_buffer_data[2] = 1.0f; g_uv_buffer_data[3] = 1.0f;
	g_uv_buffer_data[4] = 1.0f; g_uv_buffer_data[5] = 0.0f;
	//top left triangle
	g_uv_buffer_data[6] = 0.0f; g_uv_buffer_data[7] = 1.0f;
	g_uv_buffer_data[8] = 1.0f; g_uv_buffer_data[9] = 0.0f;
	g_uv_buffer_data[10] = 0.0f; g_uv_buffer_data[11] = 0.0f;

	// Bonus Solution 1
	// Display the central view, modifying the coordinates of the texture
    for (int i = 0; i <= 11; i++) {
		g_uv_buffer_data[i] = (g_uv_buffer_data[i] * (1. / 9.));
	}
	

	//--- Send the geometry to OpenGL
	// We need a vertex array object
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	// Generate 1 vertex buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// Make the buffer active
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), g_vertex_buffer_data, GL_STATIC_DRAW); 
	//same for UVs...
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), g_uv_buffer_data, GL_STATIC_DRAW);
	
	
	//clean up
	delete [] g_vertex_buffer_data;
	delete [] g_uv_buffer_data;
}

/* -------------------------------------------------------- */

void cleanUp()
{
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
}

/* -------------------------------------------------------- */

int
main(int argc, char **argv) 
{
	///
	///  glut Window
	///
	// main glut init
	glutInit(&argc, argv);
	// initial window size
	glutInitWindowSize(g_W,g_H); 
	// init display mode
#ifdef __APPLE__
    glutInitDisplayMode( GLUT_3_2_CORE_PROFILE |  GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#else
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
	// create main window
	g_MainWindow=glutCreateWindow("TP5");
	// set main window as current window
	glutSetWindow(g_MainWindow);
	/// setup glut callbacks
	// mouse (whenever a button is pressed)
	glutMouseFunc(mainMouse);
	// motion (whenever the mouse is moved *while* a button is down)
	glutMotionFunc(mainMotion);
	// keyboard (whenever a character key is pressed)
	glutKeyboardFunc(mainKeyboard);
	// display  (whenerver the screen needs to be painted)
	glutDisplayFunc(mainRender);
	// reshape (whenever the window size changes)
	glutReshapeFunc(mainReshape);
	// idle (whenever the application as some free time)
	glutIdleFunc(idle);

	///
	/// Shaders, geometry, texture and camera
	///
	
	//need to init Glew before anything else
#ifdef __APPLE__
    glewExperimental = GL_TRUE;
#endif
	glewInit();
	
	loadShaders();
	
	//Send the geometry to OpenGL
	createGeometry();
	
	//load the lightfield image
	loadTexture();

	// print a small documentation
	printf("[q]     - quit\n");

	// enter glut main loop - this *never* returns
	glutMainLoop();
	
	cleanUp();
}

/* -------------------------------------------------------- */

