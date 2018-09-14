/* -------------------------------------------------------- */
/* -------------------------------------------------------- */
/*

TP OpenGL - Image Synthesis course

George Drettakis, Adrien Bousseau, Sylvain Lefebvre

mailto: FirstName.LastName@inria.fr

REVES / INRIA

History:
- Adapted to OpenGL 3, 2014-02 (AB)

*/

#define printOpenGLError() printOglError(__FILE__, __LINE__)
extern int printOglError(char*, int);

/* -------------------------------------------------------- */
/*

TP0 - Description

This first TP is dedicated to glut and basic OpenGL operations.

* Documentation:
---------------

You *will* need to access the documentation.
- OpenGL reference  http://www.mevis.de/opengl/opengl.html (more online => use google)
- glut   reference  http://www.opengl.org/documentation/specs/glut/spec3/spec3.html

* Reference:
------------
- OpenGL web site       http://www.opengl.org
- OpenGL specifications http://www.opengl.org/documentation/specs/


Exercises guidelines:
---------------------

=> For each exercise, markers have been placed into the file.
   For instance, markers [1A] and [1B] mark locations of interest
   for exercise 1.

=> If you are under windows, the tp0.cpp will open in Visual.
   You will also need to manually open the shader files
   (eg SimpleFragmentShader.fp SimpleVertexShader.vp etc )
   from the Open->File menu so you can see them and edit them.

=> Use global variables to pass information from the keyboard/mouse to 
   mainRender or other callbacks 

   See for example variable g_W below which is used in mainReshape() to pass
   information to main()

   You will need to create these variables and set them in different routines.

=> When asked to uncomment remove the "//" characters and the "[XX]" marker

=> As a generic rule, *do not delete anything*: use '/*' or '//' comments
   to deactivate parts of the code.
=> Anytime you wonder what the parameters to a function mean or should
   be: *use the documentation*.



Exercices:
----------

 Ex1) (markers [1A] and [1B])

   - Change the color background when pressing the following keys:
     '1' => red; '2' => green; '3' => blue

	 Note: state changes for OpenGL *must happen in mainRender*, not in the
	 keyboard callback; 
     pass such information with global variables as explained previously.

 Ex2)  Open files SimpleFragmentShader.fp SimpleVertexShader.vp (you can 
      do this in VisualStudio by using "Open->File") and try and 
      understand what they do. Summarize their functionality:
      	Answer:1) SimpleVertexShader.vp contains Location 0 contains the vertex position in xyz co-ordinates
		2) SimpleVertexShader contains Location 1 contains the vertex color specification
		3) Color is accesible in the simplefragmentshader.fp 	out_color = vec3(1,0,0); which is red in our case


 Ex3) (marker [3A][3B][3C][3D] in tp0.cpp [3SA] in SimpleVertexShader.fp )
   - study the lines of the markers A-C -- what are these lines ?
     	Answer: 1) 3A Each Vertex Color of the triangle is specified each in separate lines which is red for all of them
		              
		        2) 3B glGenBuffers(1, &colorbuffer) -> It generates a buffer only 1 and specifies the array in which the values are stored( memory location of color buffer).
				glBindBuffer(GL_ARRAY_BUFFER, colorbuffer)-bind a specific vertex buffer object for initialization
				glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW) - load data for vertex buffer object
				3) 3C glEnableVertexAttribArray(1);Associate shader variable with vertex array,color buffer. Here we enable the vertex attributes which is colors of each vertex

   - Set simpleColFragShader to false [3D]
     what have you done with this change; explain what you see when you run the program:
     	Answer:Originally when true it loads values from SimpleFragmentShader.fp, but when set to false it loads the fragment shader values from SimpleFragmentShader_col.fp  where the fragment color is not specified of the triangle vertex , and is default which is red specified from in vec3 fragmentColor;

   - uncomment *all* lines with marker [3SA] in SimpleVertexShader.vp 
     Dont forget to remove the text "[3SA]" from the file !
     what have you done with this change:
	    layout(location = 1) in vec3 vertexColor;
        out vec3 fragmentColor;
        fragmentColor = vertexColor;

	    Answer: Here the simplevertex shader specification of the colors are enabled by uncommenting where by even when simpleColFragShader is set to false it derives the values of the vertex color from the location 1 vertex color.
     Did you need to recompile the C++ program ? If not, why ?
	    Answer: No since SimpleVertexShader.vp is dynamically referenced at run time it doesn't require any compilation process for it to fetch the changed values.

   - Display a triangle with red on topmost vertex, green on bottom left
     and blue on bottom right. To do this, modify the lines in [3A]

 Ex4) (markers [4A][4B][4C])
   - Uncomment [4A] 
     Set simpleColVertShader to false [4C]
     Examine the shader SimpleVertexShader_MVP.vp and compare it to SimpleVertexShader.vp.
     What is the difference ? 
     	Answer:1) SimpleVertexShader_MVP.vp has uniform matrix which does not change from one execution of a shader program to next within a particular rendering call.
		2) gl_Position value is give by the value which is matrix multiplication of MVP.
		3)But in the SimpleVertexShader.vp it is a vertexPosition_modelspace from specified in location 0.
     What do the command on line 4B and the following command do ?
     	Answer:
		glGetUniformLocation(g_glslProgram, "MVP"): Here the value of MVP uniform is retrieved from the g_glslProgram which is the model view projection matrix from the shader.
 Ex5)
   - Show a wireframe triangle using glPolygonMode 
   - Use the 'w' key to switch the wireframe mode on/off

 Ex6)
   - Modify the code so that we can adjust the view distance from the triangle 
     using the '+' and '-' keys.

 Ex7) Please look at the slides for this exercise; also examine Matrix4x4.h to
    understand which constructor to use for Matrix4x4.h

   - Make the triangle automatically rotate around the z axis
     To do this you will need to create a matrix and multiply MVP appropriately.

   - Make the triangle move back and forth on the x axis between [-1,1], 
     while still rotating on itself along the z axis.

    Remember that all changes need to happen in mainRender !

 Ex8)
   - Modify the code so that we can adjust the view distance from the triangle
     using the mouse left button + movement.

 Ex9) Bonus
   - Change the triangle to a cube by creating six quadrilaterals using
     8 vertices. If you use the example from the slides, use v4f to create
     the positions and colors; attention in changing the index (+1 rather than +4).
     You will need to change the number of triangles rendered, and the initialization
     of the buffers for rendering. Dont forget to enable the depth check to get something reasonable !

 Ex10) Bonus+
   - Add a sphere by writing a routine to create the vertices by sampling
     the surface of the sphere.


*/
/* -------------------------------------------------------- */

#ifdef _WIN32

#include <windows.h>        // windows API header (required by gl.h)
#include "glew/include/GL/glew.h" // Support for shaders
#include <GL/gl.h>          // OpenGL header
#include <GL/glu.h>         // OpenGL Utilities header
#include <GL/glut.h>        // OpenGL Utility Toolkit header

#elif __APPLE__

#include <stdlib.h>

#else

#include "glew/include/GL/glew.h"		// Support for shaders
#include <GL/gl.h>          // OpenGL header
#include <GL/glu.h>         // OpenGL Utilities header
#include <GL/glut.h>        // OpenGL Utility Toolkit header
#include <stdlib.h>

#endif

#include <cstdio>
#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "glsl.h" // GLSL Helper functions
#include "Matrix4x4.h" //class to represent matrices
#include "Vertex.h" //class to represent vertices


//path of the shaders
#ifdef _WIN32
#define SRC_PATH	""
#else
#define SRC_PATH	""
#endif

#include "glew/include/GL/glew.h"		// Support for shaders
#ifdef __APPLE__
#include <gl3.h>          // OpenGL header
#include <gl3ext.h>          // OpenGL header
#include <glut.h>        // OpenGL Utility Toolkit header

// if you get a compiler error with above, use below instead
//#include <OpenGL/gl3.h>          // OpenGL header
//#include <OpenGL/gl3ext.h>          // OpenGL header
//#include <GLUT/glut.h>        // OpenGL Utility Toolkit header

#else
#include <GL/gl.h>          // OpenGL header
#include <GL/glut.h>        // OpenGL Utility Toolkit header
#endif

GLhandleARB createGLSLProgram(const char *vp_code,const char *fp_code);
const char *loadStringFromFile(const char *fname);

/* --------------------- Main Window ------------------- */

int          g_MainWindow; // glut Window Id
int          g_W=640;      // window width
int          g_H=480;      // window width
double       g_R = 0.0f;
double       g_G = 0.0f;
double       g_B = 0.0f;
boolean      wire =false;
boolean      plus = false;
boolean      minus = false;
double       plusvaluex = 4.0f;
double       plusvaluey = 3.0f;
double       plusvaluez = 3.0f;


/* --------------------- Geometry ------------------- */
//Vertex Array Object
GLuint VertexArrayID;

// An array of 3 vectors which represents 3 vertices
static const GLfloat g_vertex_buffer_data[] = {
	// [2A] 
  
  1.0f, -1.0f, 0.0f,
  -1.0f, -1.0f, 0.0f,
  0.0f,  1.0f, 0.0f,
};

// One color for each vertex.  [3A]
static const GLfloat g_color_buffer_data[] = {
	// add in three lines here for the 3 colors as requested
	// the syntax is as above for the vertex data
      0, 1, 0, 1,
      1, 0, 0, 1,
      0, 0, 1, 1
};

// This will identify our vertex buffer
GLuint vertexbuffer;

// This will identify our color buffer
GLuint colorbuffer;

/*---------------------- Shaders -------------------*/
GLuint g_glslProgram;

/*---------------------- Camera --------------------*/
m4x4f MVP; //model-view-projection matrix

/* --------------------- Matrix Creation ------------*/
void createMatrices();

/* -------------------------------------------------------- */

void mainKeyboard(unsigned char key, int x, int y)
{
	if (key == 'q') {
		exit(0);
	}
	else if (key == ' ') {
		printf("spacebar pressed\n");
	}
	else if (key == '1') {
		g_R = 1.0f;
		g_G = 0.0f;
		g_B = 0.0f;

	}
	else if (key == '2') {
		g_R = 0.0f;
		g_G = 1.0f;
		g_B = 0.0f;


	}
	else if (key == '3') {
		g_R = 0.0f;
		g_G = 0.0f;
		g_B = 1.0f;
	}
	else if ((key == 'w') && (!wire)) {
		wire = true;
	}
	else if ((key == 'w') && (wire)) {
		wire = false;
	}
	
	// [1A]

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
		if        (btn == GLUT_LEFT_BUTTON) {
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
	// Dark blue background
	glClearColor(g_R, g_G, g_B, 0.0f); // [1B]
	
	// clear screen and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (wire) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	// use our shader
	glUseProgramObjectARB(g_glslProgram);

	// Get a handle for our "MVP" uniform (identify the model-view-projection matrix in the shader)
	// [4B]
	GLuint MatrixID = glGetUniformLocation(g_glslProgram, "MVP");
	
	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &(MVP[0]));
	
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
	  0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
	  3,                  // size, we have 3 components per vertex
	  GL_FLOAT,           // type
	  GL_FALSE,           // normalized?
	  0,                  // stride
	  (void*)0            // array buffer offset
	);

	// 2nd attribute buffer : colors [3C
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glVertexAttribPointer(
	    1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
	    4,                                // size
	    GL_FLOAT,                         // type
	    GL_FALSE,                         // normalized?
	    0,                                // stride
	    (void*)0                          // array buffer offset
	);
  
	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
 
	glDisableVertexAttribArray(0);
	
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

void createMatrices()
{
	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	m4x4f Projection = perspectiveMatrix<float>(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	
	// Camera matrix
	m4x4f View   = lookatMatrix(V3F(1., 0.75, 0.75), // Camera is at (4,3,3), in World Space [4D]
					V3F(0.,0.,0.), // and looks at the origin
					V3F(0.,1.,7.)); // Head is up (set to 0,-1,0 to look upside-down)
				
	// Model matrix : an identity matrix (model will be at the origin)
	m4x4f Model;
	Model.eqIdentity();
	
	// Our ModelViewProjection : multiplication of our 3 matrices
	MVP        = Model*View*Projection; // Remember, matrix multiplication is the other way around
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
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
	// create main window
	g_MainWindow=glutCreateWindow("TP0");
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
	/// Shaders and geometry
	///
	
	//need to init Glew before anything else
#ifdef __APPLE__
	glewExperimental = GL_TRUE;
#endif
	glewInit();
	
	//--- Load the shaders
	
	const char *vp_code;
	const char *fp_code;

	bool simpleColFragShader = false, simpleColVertShader = false;
	if( simpleColFragShader ) // [3D]
		fp_code=loadStringFromFile(SRC_PATH "SimpleFragmentShader.fp"); 
	else
		fp_code=loadStringFromFile(SRC_PATH "SimpleFragmentShader_col.fp");

	if( simpleColVertShader ) // [4C]
		vp_code=loadStringFromFile(SRC_PATH "SimpleVertexShader.vp"); 
	else
		vp_code=loadStringFromFile(SRC_PATH "SimpleVertexShader_MVP.vp"); 

	g_glslProgram = createGLSLProgram(vp_code,fp_code);
	delete [](fp_code);
	delete [](vp_code);
	
	//--- Send the geometry to OpenGL
	// We need a vertex array object
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	// Generate 1 vertex buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// Make the buffer active
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	//same for colors... [3B]
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

 	// [4A]	
 		createMatrices();

	// print a small documentation
	printf("[q]     - quit\n");

	// enter glut main loop - this *never* returns
	glutMainLoop();
	
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
}

/* -------------------------------------------------------- */

