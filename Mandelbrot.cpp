#include "stdafx.h"

#include <cstdlib>                      // standard definitions
#include <iostream>                     // C++ I/O
#include <cstdio>                       // C I/O (for sprintf) 
#include <cmath>                        // standard definitions
#include <string.h>

#include <glew.h>
#include <glut.h>                    // GLUT
#include <GL/glu.h>                     // GLU
#include <GL/gl.h>                      // OpenGL

#include <WinGDI.h>

using namespace std;                    // make std accessible

int numImages = 1000;
double imgZoom = 0.1;

GLuint w = 1920, h = 1080;

GLuint vertexbuffer;
GLuint programID;
GLuint nav_handle, lookup_handle, iter_handle, filter_handle, crosshair_handle;
GLuint texture;

GLdouble position[3] = { 0, 0, 0.5 };
GLfloat pos[2][3];
GLint lookup = 0, filter = 0, iter = 100;
GLint crosshair = 0;
GLint takeImg = 0;
char* vp1 =
"layout(location = 0) in vec3 vertexPosition_modelspace;\
		out vec3 pos;\
		void main(){\
			gl_Position.xyz = vertexPosition_modelspace;\
			gl_Position.w = 1.0;\
			pos = gl_Position.xyz;\
		}";

char* fp1 =
"in vec3 pos;\
		out vec3 color;\
		uniform dvec3 nav;\
		uniform int iter;\
		uniform int lookup;\
		uniform int filter;\
		uniform int crosshair;\
		\
		dvec2 zoom = { nav.z, nav.z*16/9 };\
		dvec2 go = { nav.x, nav.y };\
		\
		int getN(vec2 posi){\
			int i=0;\
			dvec2 z = { 0,0 }; \
			double z0f = 0.0, z0q = 0.0, z1q = 0.0; \
			\
			dvec2 vpos = posi/zoom +go;\
			\
			while(z0q + z1q <= 4 && i<iter){\
				z0f = z[0];\
				z.x = z0q - z1q + vpos.x;\
				z.y = 2 * z0f * z.y + vpos.y; \
				i++;\
				z0q = z.x * z.x;\
				z1q = z.y * z.y;\
			}\
			return i;\
		}\
		void main(){\
			\
			int n = 0;\
			\
			vec2 d = {1.0/1920, 1.0/1080};\
			\
			int n1 = getN(pos.xy); \
			\
			if(filter==0){\
				n = n1;\
			} else if(filter==1){ \
				int n2 = getN(pos.xy + vec2(d.x, d.y)); \
				int n3 = getN(pos.xy + vec2(-d.x, d.y)); \
				int n4 = getN(pos.xy + vec2(d.x, -d.y)); \
				int n5 = getN(pos.xy + vec2(-d.x, -d.y)); \
				n = (n5 + n4 + n3 + n2 + n1) / 5; \
			} else if(filter==2){ \
				int n22 = getN(pos.xy + vec2(d.x / 2, d.y / 2)); \
				int n33 = getN(pos.xy + vec2(-d.x / 2, d.y / 2)); \
				int n44 = getN(pos.xy + vec2(d.x / 2, -d.y / 2)); \
				int n55 = getN(pos.xy + vec2(-d.x / 2, -d.y / 2)); \
				n = (n55 + n44 + n33 + n22 + n1) / 5; \
			} else if(filter==4){ \
				int n22 = getN(pos.xy + vec2(d.x / 2, d.y / 2)); \
				int n33 = getN(pos.xy + vec2(-d.x / 2, d.y / 2)); \
				int n44 = getN(pos.xy + vec2(d.x / 2, -d.y / 2)); \
				int n55 = getN(pos.xy + vec2(-d.x / 2, -d.y / 2)); \
				int n222 = getN(pos.xy+vec2(0,d.y/2));\
				int n333 = getN(pos.xy+vec2(-d.x/2,0));\
				int n444 = getN(pos.xy+vec2(0,-d.y/2));\
				int n555 = getN(pos.xy+vec2(-d.x/2,0));\
				n = (n55 + n44 + n33 + n22 + n1 + n555 + n444 + n333 + n222) / 9; \
			} else if(filter==3){ \
				int n224 = getN(pos.xy + vec2(d.x / 4, d.y / 4)); \
				int n334 = getN(pos.xy + vec2(-d.x / 4, d.y / 4)); \
				int n444 = getN(pos.xy + vec2(d.x / 4, -d.y / 4)); \
				int n554 = getN(pos.xy + vec2(-d.x / 4, -d.y / 4)); \
				n = (n554 + n444 + n334 + n224 + n1) / 5; \
			}else if(filter==5){ \
				int n2 = getN(pos.xy + vec2(d.x, d.y)); \
				int n3 = getN(pos.xy + vec2(-d.x, d.y)); \
				int n4 = getN(pos.xy + vec2(d.x, -d.y)); \
				int n5 = getN(pos.xy + vec2(-d.x, -d.y)); \
				n = min(n1,min(n2,min(n3,min(n4,n5)))); \
			} else if(filter==6){ \
				int n22 = getN(pos.xy + vec2(d.x / 2, d.y / 2)); \
				int n33 = getN(pos.xy + vec2(-d.x / 2, d.y / 2)); \
				int n44 = getN(pos.xy + vec2(d.x / 2, -d.y / 2)); \
				int n55 = getN(pos.xy + vec2(-d.x / 2, -d.y / 2)); \
				n = min(n1,min(n22,min(n33,min(n44,n55)))); \
			}else if(filter==7){ \
				int n22 = getN(pos.xy + vec2(d.x / 2, d.y / 2)); \
				int n33 = getN(pos.xy + vec2(-d.x / 2, d.y / 2)); \
				int n44 = getN(pos.xy + vec2(d.x / 2, -d.y / 2)); \
				int n55 = getN(pos.xy + vec2(-d.x / 2, -d.y / 2)); \
				int n222 = getN(pos.xy + vec2(d.x / 4, d.y / 4)); \
				int n333 = getN(pos.xy + vec2(-d.x / 4, d.y / 4)); \
				int n444 = getN(pos.xy + vec2(d.x / 4, -d.y / 4)); \
				int n555 = getN(pos.xy + vec2(-d.x / 4, -d.y / 4)); \
				n = min(n1,min(n22,min(n33,min(n44,min(n55,min(n222,min(n333,min(n444,n555)))))))); \
			}else if(filter==8){ \
				int n22 = getN(pos.xy + vec2(d.x / 4, d.y / 4)); \
				int n33 = getN(pos.xy + vec2(-d.x / 4, d.y / 4)); \
				int n44 = getN(pos.xy + vec2(d.x / 4, -d.y / 4)); \
				int n55 = getN(pos.xy + vec2(-d.x / 4, -d.y / 4)); \
				int n222 = getN(pos.xy + vec2(d.x / 8, d.y / 8)); \
				int n333 = getN(pos.xy + vec2(-d.x / 8, d.y / 8)); \
				int n444 = getN(pos.xy + vec2(d.x / 8, -d.y / 8)); \
				int n555 = getN(pos.xy + vec2(-d.x / 8, -d.y / 8)); \
				n = min(n1,min(n22,min(n33,min(n44,min(n55,min(n222,min(n333,min(n444,n555)))))))); \
			} else if(filter==9){ \
				\
			}\
			\
			float r,g,b;\
			if(lookup==0){\
				r = (1+cos(radians(float(n>>1))))/2;\
				g = (1+cos(radians(float(n>>1)+90)))/2;\
				b = (1+cos(radians(float(n>>1)+180)))/2;\
			} else if(lookup==1){\
				r = (n-log(log(float(n) / log(4)) / log(2)))/256;\
				g = ((n)-log(log(float(n) / log(4)) / log(2)))/512;\
				b = ((n)-log(log(float(n) / log(4)) / log(2)))/1024;\
			} else if(lookup==2){\
				r = (n-log(log(float(n) / log(4)) / log(2)))/256;\
				g = ((n>>2)-log(log(float(n) / log(4)) / log(2)))/256;\
				b = ((n>>4)-log(log(float(n) / log(4)) / log(2)))/256;\
			} else if(lookup==3){\
				r = (1+cos(radians(float(n))))/2;\
				g = (1+cos(radians(float(n>>1))))/2;\
				b = (1+cos(radians(float(n>>2))))/2;\
			} else if(lookup==4){\
				r = (1+cos(radians(float(n))))/2;\
				g = (1+cos(radians(float(n>>2))))/2;\
				b = (1+cos(radians(float(n>>4))))/2;\
			} else if(lookup==5){\
				r = (1+cos(radians(float(n>>3))))/2;\
				g = (1+cos(radians(float(n>>3)+90)))/2;\
				b = (1+cos(radians(float(n>>3)+180)))/2;\
			} else if(lookup==6){\
				r = (1+cos(radians(float(n>>4))))/2;\
				g = (1+cos(radians(float(n>>4)+90)))/2;\
				b = (1+cos(radians(float(n>>4)+180)))/2;\
			} else if(lookup==7){\
				r = (1+cos(float(n)/100.0))/2;\
				g = (1+cos(float(n)/100.0+1))/2;\
				b = (1+cos(float(n)/100.0+2))/2;\
			}\
			color = n==iter?vec3(0,0,0):vec3(b,g,r);\
			\
			if(crosshair==1)\
			if(pos[0]<0.001 && pos[0]>-0.001 || pos[1]<0.005 && pos[1]>-0.005) \
				color = vec3(1,1,1); \
		}";

GLuint genShaderProg(char* vpString, char* fpString) {
	GLuint progHandle = glCreateProgram();
	GLuint vp = glCreateShader(GL_VERTEX_SHADER);
	GLuint fp = glCreateShader(GL_FRAGMENT_SHADER);

	char* version = "#version 430 core\n#extension GL_ARB_gpu_shader_fp64 : require\n";

	const char *vpSrc[] = { version, vpString };
	const char *fpSrc[] = { version, fpString };

	glShaderSource(vp, 2, vpSrc, NULL);
	glShaderSource(fp, 2, fpSrc, NULL);

	glCompileShader(vp);
	int rvalue;
	glGetShaderiv(vp, GL_COMPILE_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in compiling vp\n");
		exit(30);
	}
	glAttachShader(progHandle, vp);
	glDeleteShader(vp);

	glCompileShader(fp);
	glGetShaderiv(fp, GL_COMPILE_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in compiling fp\n");
		exit(31);
	}

	glAttachShader(progHandle, fp);
	glDeleteShader(fp);

	glBindFragDataLocation(progHandle, 0, "color");
	glLinkProgram(progHandle);

	glGetProgramiv(progHandle, GL_LINK_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in linking sp\n");
		exit(32);
	}
	return progHandle;
}
void SaveAsBMP(const char *fileName)
{
	FILE *file;
	unsigned long imageSize;
	GLbyte *data = NULL;
	GLint viewPort[4];
	GLenum lastBuffer;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	bmfh.bfType = 'MB';
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = 54;
	glGetIntegerv(GL_VIEWPORT, viewPort);
	imageSize = ((viewPort[2] + ((4 - (viewPort[2] % 4)) % 4))*viewPort[3] * 3) + 2;
	bmfh.bfSize = imageSize + sizeof(bmfh) + sizeof(bmih);
	data = (GLbyte*)malloc(imageSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_PACK_SWAP_BYTES, 1);
	glGetIntegerv(GL_READ_BUFFER, (GLint*)&lastBuffer);
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, viewPort[2], viewPort[3], GL_BGR, GL_UNSIGNED_BYTE, data);
	data[imageSize - 1] = 0;
	data[imageSize - 2] = 0;
	glReadBuffer(lastBuffer);
	if (int err = fopen_s(&file, fileName, "wb") != 0) {
		printf("Error while open file for screenshot %d\n", err);
	}
	bmih.biSize = 40;
	bmih.biWidth = viewPort[2];
	bmih.biHeight = viewPort[3];
	bmih.biPlanes = 1;
	bmih.biBitCount = 24;
	bmih.biCompression = 0;
	bmih.biSizeImage = imageSize;
	bmih.biXPelsPerMeter = 45089;
	bmih.biYPelsPerMeter = 45089;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;
	fwrite(&bmfh, sizeof(bmfh), 1, file);
	fwrite(&bmih, sizeof(bmih), 1, file);
	fwrite(data, imageSize, 1, file);
	free(data);
	fclose(file);
}
void renderScene(void) {                          // display callback
	cout << "MyDisplay called" << endl;

	// 1rst attribute buffer : vertices
	glUseProgram(programID);
	glUniform3dv(nav_handle, 1, position);
	glUniform1iv(iter_handle, 1, &iter);
	glUniform1iv(lookup_handle, 1, &lookup);
	glUniform1iv(filter_handle, 1, &filter);
	glUniform1iv(crosshair_handle, 1, &crosshair);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	//texture = genTexture();

	//glUniform1i(glGetUniformLocation(programID, "table"), 0);

	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, 6); // Starting from vertex 0; 3 vertices total -> 1 triangle
	glDisableVertexAttribArray(0);
	glutSwapBuffers();                          // swap buffers

	if (takeImg > 0) {
		char buffer[4];
		_itoa_s(numImages - takeImg, buffer, 10);
		char buf[100];
		strcpy_s(buf, 100, "C:\\Users\\Admin\\Desktop\\img2\\test-GL-");
		strcat_s(buf, 100, buffer);
		strcat_s(buf, 100, ".bmp");
		SaveAsBMP(buf);
		position[2] += position[2] * imgZoom;
		takeImg--;
		glutPostRedisplay();
	}
}
void keyboard(unsigned char key, int x, int y) {
	
	double speed = 0.005;
	double zoomSpeed = 0.8;
	double d = 0.0;
	
	switch (key) {
	case 'w':
		position[1] += speed / position[2];
		break;
	case 's':
		position[1] -= speed / position[2];
		break;
	case 'd':
		position[0] += speed / position[2];
		break;
	case 'a':
		position[0] -= speed / position[2];
		break;
	case '+':
		position[2] += position[2] * zoomSpeed;
		break;
	case '-':
		position[2] -= position[2] * zoomSpeed;
		break;
	case '/':
		lookup--;
		break;
	case '*':
		lookup++;
		break;
	case '9':
		filter++;
		break;
	case '8':
		filter--;
		break;
	case 'u':
		iter /= 1.1;
		break;
	case 'i':
		iter *= 1.1;
		break;
	case 'z':
		position[0] -= ((float)x + position[0]) / 1800;
		position[1] -= ((float)y + position[1]) / 900;;
		break;
	case 'c':
		crosshair = crosshair==0?1:0;
		break;
	case '0':
		SaveAsBMP("C:\\Users\\Admin\\Desktop\\img\\test.bmp");
		break;
	case 'k':
		takeImg = numImages;
		break;
	case 'l':
		takeImg = 0;
		break;
	case 27:
		glutPositionWindow(0, 0);
		glutReshapeWindow(w, h);
		break;
	}
	glutPostRedisplay();
	printf("X: %.15f, Y: %.15f, Z: %.15f\n", position[0], position[1], position[2]);
	printf("vpos: %f, %f\n", 1 / position[2] + position[0], 1 / position[2] + position[1]);
	printf("iter: %d %f\n\n", iter, (iter / position[2]));
	printf("ch: %d\n", crosshair);
}
void init() {
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,

		-1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
	};


	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	programID = genShaderProg(vp1, fp1);
	nav_handle = glGetUniformLocation(programID, "nav");
	iter_handle = glGetUniformLocation(programID, "iter");
	lookup_handle = glGetUniformLocation(programID, "lookup");
	filter_handle = glGetUniformLocation(programID, "filter");
	crosshair_handle = glGetUniformLocation(programID, "crosshair");
}
int main(int argc, char** argv)
{
	cout << "\n";

	glutInit(&argc, argv);                      // OpenGL initializations
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);// double buffering and RGB
	glutInitWindowSize(w, h);               // create a 400x400 window
	glutInitWindowPosition(0, 0);               // ...in the upper left
	glutCreateWindow(argv[0]);                  // create the window
	glutFullScreen();

	glewInit();
	init();

	glutDisplayFunc(renderScene);                 // setup callbacks
	glutKeyboardFunc(keyboard);
	glutMainLoop();                             // start it running
	return 0;                                   // ANSI C expects this
}