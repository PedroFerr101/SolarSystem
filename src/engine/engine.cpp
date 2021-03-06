﻿#include <stdlib.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#include </usr/local/Cellar/devil/1.8.0_1/include/IL/il.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#include <IL/il.h>
#endif


#include <stdio.h>
#include <iostream>
#include <fstream>
#include <tinyxml2.h>
#include <string>
#include <vector>
#include <group.h>
#include <operation.h>
#include <translate.h>
#include <rotate.h>
#include <scale.h>
#include <hash.h>
#include <figure.h>
#include <xmlHandler.h>
#include <ViewFrustumCulling.h>

#define _USE_MATH_DEFINES
#include <math.h>


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"


using namespace std;


void prepareLights();
float* computeMPMatrix();

#define _PI_ 3.14159
GLuint vertexCount;

/**
Variável global para utilizar VBOs
*/
GLuint *buffers, *buffersN, *buffersT;
GLuint *idTex;

int loadTexture(std::string s);


//########################################## Settings ##########################################
float cam[3] = {0, 0, 0};
float Lx = 0; float Ly = 0; float Lz = 0;
int startX, startY, tracking = 0;
int alpha = 180, beta = 0, r = 50;
int timebase = 0, frame = 0;
int mode = 0;
bool cullingOFF;
float height = 800;
float width = 800;
//####################################### Variáveis globais ####################################

/** 
Frustum planes for current frame
*/
float** planes;

/**
Variável global com a lista de figuras a desenhar e luzes
*/
vector<Group> groups;
vector<Light*> lights;
/**
Variável global para indicar qual a figura que esta a ser processada (ajuda a contrucao e desenho por VBOS)
*/
int findex;

//################################ Funções para aplicação de VBOs ##############################

/**
Função responsável por ativar e preencher o buffer associado a uma determinada figura
*/
void prepareFigure(Figure f, int f_index) {
	int vector_size = sizeof(float) * f.getNumPoints() * 3; // 3 floats per vertex
	int normal_size = sizeof(float) * f.getNumNormals() * 3;
	int tex_size = sizeof(float) * f.getNumTextures() * 2;
	float* coord_vector = (float*)malloc(sizeof(float) * vector_size);
	float* norm_vector = (float*)malloc(sizeof(float) * vector_size);
	float* tex_vector = (float*)malloc(sizeof(float) * vector_size);
	int i=0;

	for(Point p : f.getPoints()){
		coord_vector[i++] = p.getX();
		coord_vector[i++] = p.getY();
		coord_vector[i++] = p.getZ();
	}

	i = 0;
	for (Point p : f.getNormals()) {
		norm_vector[i++] = p.getX();
		norm_vector[i++] = p.getY();
		norm_vector[i++] = p.getZ();
	}

	i = 0;
	for (Point p : f.getTexture()) {
		tex_vector[i++] = p.getX();
		tex_vector[i++] = p.getY();
	}
	
	//glGenBuffers(1, buffers);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[f_index]);
	glBufferData(GL_ARRAY_BUFFER,  vector_size, coord_vector, GL_STATIC_DRAW);

	//glGenBuffers(1, buffersN);
	glBindBuffer(GL_ARRAY_BUFFER, buffersN[f_index]);
	glBufferData(GL_ARRAY_BUFFER, normal_size, norm_vector, GL_STATIC_DRAW);

	//glGenBuffers(1, buffersT);
	glBindBuffer(GL_ARRAY_BUFFER, buffersT[f_index]);
	glBufferData(GL_ARRAY_BUFFER, tex_size, tex_vector, GL_STATIC_DRAW);

	if (f.getTexPath().empty()) idTex[f_index] = -1;
	else idTex[f_index] = loadTexture(f.getTexPath());

	free(coord_vector);
	free(norm_vector);
	free(tex_vector);
}

/**
 Função que garante que são inicializados e preenchidos os buffers associados a cada uma das figuras de um grupo
*/
void prepareGroup(Group g){
	for(Figure f : g.getFigures()){

		prepareFigure(f,findex);
		++findex;
	}
	for(Group sg : g.getSubGroups()){
		prepareGroup(sg);
	}
}

/**
 Função responsável por inicilizar e preencher todos os buffers, um para cada figura presente na cena
*/
void prepareAllFigures(int n_figures){
	buffers = (GLuint *) malloc(sizeof(GLuint) * n_figures);
	glGenBuffers(n_figures, buffers); 

	buffersN = (GLuint *)malloc(sizeof(GLuint) * n_figures);
	glGenBuffers(n_figures, buffersN);

	buffersT = (GLuint *)malloc(sizeof(GLuint) * n_figures);
	glGenBuffers(n_figures, buffersT);

	idTex = (GLuint*)malloc(sizeof(GLuint) * n_figures);

	findex=0;
	for(Group g : groups){
		prepareGroup(g);
	}

}



/**
 Função que, dada a posicao do buffer associado a uma determinada figura, efetua o seu desenho
*/
void drawFigure(Figure f, int f_index, float* center, float scale) {
	if(f.getFigType() == Figure::FSPHERE){ // se for uma esfera...
		float radius = scale * f.getRadius();

		if(!cullingOFF && !sphereInFrustum(planes, center, scale)){
   			return ; // do not draw sphere
   		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, buffers[f_index]);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, buffersN[f_index]);
	glNormalPointer(GL_FLOAT, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, buffersT[f_index]);
	glTexCoordPointer(2, GL_FLOAT, 0, 0);

	for (Colour g : f.getColours()) {
		float colour[4] = { g.getR(), g.getG(), g.getB(),1 };
		if (g.getType()==0) glMaterialfv(GL_FRONT, GL_DIFFUSE, colour);
		else if (g.getType()==1) glMaterialfv(GL_FRONT, GL_SPECULAR, colour);
		else if (g.getType()==2) glMaterialfv(GL_FRONT, GL_EMISSION, colour);
		else if (g.getType()==3) glMaterialfv(GL_FRONT, GL_AMBIENT, colour);
	}
	
	if (idTex[f_index] != -1) {
		glBindTexture(GL_TEXTURE_2D, idTex[f_index]);
		glDrawArrays(GL_TRIANGLES, 0, f.getNumPoints());
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else glDrawArrays(GL_TRIANGLES, 0, f.getNumPoints());
	
	// RESET

	float amb[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
	float diff[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
	float spec[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float emi[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	glMaterialfv(GL_FRONT, GL_EMISSION, emi);

}


//############################################# GLUT ###########################################

void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window with zero width).
	if (h == 0) h = 1;

	// compute window's aspect ratio 
	float ratio = w * 1.0 / h;

	// Set the projection matrix as current
	glMatrixMode(GL_PROJECTION);
	// Load Identity Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set perspective
	gluPerspective(45.0f, ratio, 1.0f, 1000.0f);

	// return to the model view matrix mode
	glMatrixMode(GL_MODELVIEW);
}


void drawCoordinates() {

	glBegin(GL_LINES);
		// x
		glColor3f(1.0, 0.0, 0.0); // red x	
		glVertex3f(-40.0, 0.0f, 0.0f);
		glVertex3f(40.0, 0.0f, 0.0f);


		// y 
		glColor3f(0.0, 1.0, 0.0); // green y
		glBegin(GL_LINES);
		glVertex3f(0.0, -40.0f, 0.0f);
		glVertex3f(0.0, 40.0f, 0.0f);


		// z 
		glColor3f(0.0, 0.0, 1.0); // blue z
		glVertex3f(0.0, 0.0f, -40.0f);
		glVertex3f(0.0, 0.0f, 40.0f);

	glEnd();
}

//###################################### Render Scene ##########################################


void renderGroup(Group g, float* center,  float scale) {
	float current_scale = scale;
	float* current_center = (float*) malloc(sizeof(float)*3);
	current_center[0] = center[0];
	current_center[1] = center[1];
	current_center[2] = center[2];

	vector<Operation*> ops = g.getOperations();
	vector<Figure> figs = g.getFigures();
	vector<Group> subGroups = g.getSubGroups();


	glPushMatrix();

    for (Operation* o : ops) {
        o->transformacao();
		if (!cullingOFF) {
			if (Scale* s = dynamic_cast<Scale*>(o)) {
				current_scale *= s->getScale();
			}
			if (Translate* t = dynamic_cast<Translate*>(o)) {
				float* t_position = t->getPosition();
				current_center[0] += t_position[0];
				current_center[1] += t_position[1];
				current_center[2] += t_position[2];
			}
			if (DynamicTranslate* dt = dynamic_cast<DynamicTranslate*>(o)) {
				float* t_position = dt->getPosition();
				current_center[0] += t_position[0];
				current_center[1] += t_position[1];
				current_center[2] += t_position[2];
			}
		}       
	}

	for (Figure f : figs) {	
		drawFigure(f, findex, current_center, current_scale);
		++findex; 
	}

	for (Group g : subGroups) {
		renderGroup(g, current_center, current_scale);
	}

	glPopMatrix();
}


/* ###################### TEXTO ######################### */


void renderText(float x, float y, void *font, string s) {
	glWindowPos2d(x, y);
	for (int i=0; s[i] != '\0'; i++) {
		glutBitmapCharacter(font, s[i]);
	}

}

void showInfo() {
	glColor3f(1, 1, 1);

	string mode_str = "Mode: ";
	if (mode == 0) mode_str.append(" Fill");
	else if (mode == 1) mode_str.append("Line");
	else mode_str.append("Point");

	string frustum_str = "Frustum: ";
	if (cullingOFF == true) frustum_str.append("Off");
	else frustum_str.append("On");

	renderText(10, 24, GLUT_BITMAP_HELVETICA_12, mode_str);
	renderText(10, 10, GLUT_BITMAP_HELVETICA_12, frustum_str);
}




/***************/


void renderScene(void) {
	float fps;
	int time;
	char s[64];

	//glClearColor(20.0f, 20.0f, 20.0f, 1);
	// clear buffers
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	

	// set the camera
	glLoadIdentity();
	gluLookAt(cam[0], cam[1], cam[2],
		Lx, Ly, Lz,
		0.0f, 1.0f, 0.0f);

	showInfo();

	prepareLights();


	glColor3b(0, 5, 20);

	if (!cullingOFF) {
		float * mp = computeMPMatrix();

		planes = getFrustumPlanes(mp);
		free(mp);
	}
	
	
	findex=0;
	float center[3] = {0.0f,0.0f,0.0f};
	for (Group g : groups) {		
		renderGroup(g, center, 1);
	}

	if (!cullingOFF) {
		for (int i = 0; i < 6; i++) {
			free(planes[i]);
		}
		free(planes);
	}
	

	// drawCoordinates();

	frame++;
	time = glutGet(GLUT_ELAPSED_TIME);
	if (time - timebase > 1000) {
		fps = frame * 1000.0 / (time - timebase);
		timebase = time;
		frame = 0;
		sprintf(s, "FPS: %f6", fps);
		glutSetWindowTitle(s);
	}

	// End of frame
	glutSwapBuffers();
}

//################# Cálculo matriz a utilizar para definir planos do frustum ###################

float* computeMPMatrix(){
	float * res = (float*) malloc(sizeof(float) * 16);
	float m[16],p[16];

	glGetFloatv(GL_PROJECTION_MATRIX,p);
	glGetFloatv(GL_MODELVIEW_MATRIX,m);

	glPushMatrix();

	glLoadMatrixf(p);
	glMultMatrixf(m);
	glGetFloatv(GL_MODELVIEW_MATRIX, res);

	glPopMatrix();


	return res;
}

//############ Funções responsáveis pelo processamento de açoes do utilizador ##################

void processKeys(unsigned char key, int xx, int yy) {
	key = tolower(key);
	
	if (key == 'p') {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	float k;
	float dx, dy, dz;

	if( key == 'f'){
		cullingOFF = !cullingOFF;
	}
	

	if (key == 'w' || key == 's') {
		if (key == 'w')
			k = 0.2;
		if (key == 's')
			k = -0.2;

		dx = Lx - cam[0];
		dy = Ly - cam[1];
		dz = Lz - cam[2];

		cam[0] = cam[0] + k * dx;
		cam[1] = cam[1] + k * dy;
		cam[2] = cam[2] + k * dz;

		Lx = Lx + k * dx;
		Ly = Ly + k * dy;
		Lz = Lz + k * dz;
	}
	else if (key == 'd' || key == 'a') {
		if (key == 'd') {
			k = 0.2;			
		}
		if (key == 'a') {
			k = -0.2;		
		}

		dx = Lx - cam[0];
		dy = Ly - cam[1];
		dz = Lz - cam[2];
		float upX = 0;
		float upY = 1;
		float upZ = 0;
		float rx = dy*upZ-dz*upY;
		float rz = dx*upY-dy*upX;

		cam[0] = cam[0] + k * rx;
		cam[2] = cam[2] + k * rz;

		Lx = Lx + k * rx;
		Lz = Lz + k * rz;		
	}
	else if (key == 'm') {
		mode = (mode + 1) % 3;
		if (mode == 0) glPolygonMode(GL_FRONT, GL_FILL);
		else if (mode == 1) glPolygonMode(GL_FRONT, GL_LINE);
		else glPolygonMode(GL_FRONT, GL_POINT);
	}
	
}


void processMouseButtons(int button, int state, int xx, int yy) {

	if (state == GLUT_DOWN) {
		startX = xx;
		startY = yy;
		if (button == GLUT_LEFT_BUTTON)
			tracking = 1;
		else if (button == GLUT_RIGHT_BUTTON)
			tracking = 2;
		else
			tracking = 0;
	}
	else if (state == GLUT_UP) {
		if (tracking == 1) {
			alpha += (xx - startX);
			beta += (yy - startY);
		}
		else if (tracking == 2) {

			r -= yy - startY;
			if (r < 3)
				r = 3.0;
		}
		tracking = 0;
	}

}


void processMouseMotion(int xx, int yy) {

	int deltaX, deltaY;
	int alphaAux, betaAux;
	int rAux;

	if (!tracking)
		return;

	deltaX = xx - startX;
	deltaY = yy - startY;

	if (tracking == 1) {


		alphaAux = alpha + deltaX;
		betaAux = beta + deltaY;

		if (betaAux > 85.0)
			betaAux = 85.0;
		else if (betaAux < -85.0)
			betaAux = -85.0;

		rAux = r;
	}
	else if (tracking == 2) {

		alphaAux = alpha;
		betaAux = beta;
		rAux = r - deltaY;
		if (rAux < 3)
			rAux = 3;
	}


	Lx = cam[0] + rAux * sin(alphaAux * 3.14 / 180.0) * cos(betaAux * 3.14 / 180.0);
	Lz = cam[2] + rAux * cos(alphaAux * 3.14 / 180.0) * cos(betaAux * 3.14 / 180.0);
	Ly = cam[1] + rAux * sin(betaAux * 3.14 / 180.0);


}


//############ Funções responsáveis pelo processamento de luzes e texturas ##################


void prepareLights() {	
	int light_nr = 0;

	for(Light* light : lights) {
		glEnable(GL_LIGHT0 + light_nr);

		light->turnOn(GL_LIGHT0 + light_nr);

		light_nr++;	
	}
}

int loadTexture(std::string s) {

	unsigned int t, tw, th;
	unsigned char *texData;
	unsigned int texID;

	ilInit();
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	ilGenImages(1, &t);
	ilBindImage(t);
	ilLoadImage((ILstring)s.c_str());
	tw = ilGetInteger(IL_IMAGE_WIDTH);
	th = ilGetInteger(IL_IMAGE_HEIGHT);
	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	texData = ilGetData();

	glGenTextures(1, &texID);

	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;

}



//########################################### MAIN #############################################



int main(int argc, char **argv) {

	if (argc == 1) {
		printf("Por favor insira todos os parametros necessarios. \n");
		return -1;
	}

	if (readXML(argv[1], &groups, &lights, cam) == XML_SUCCESS){
		// init GLUT and the window
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
		glutInitWindowPosition(100, 100);
		glutInitWindowSize(height, width);
		glutCreateWindow("solar-system");

		// Required callback registry 
		glutDisplayFunc(renderScene);
		glutIdleFunc(renderScene);
		glutReshapeFunc(changeSize);

		// Callback registration for keyboard processing
		glutKeyboardFunc(processKeys);
		glutMouseFunc(processMouseButtons);
		glutMotionFunc(processMouseMotion);
		
		#ifndef __APPLE__	
		// init GLEW
		glewInit();
		#endif	


		int total_n_figures = 0;
		for(Group g : groups){
			total_n_figures += g.getNumFigures();
		}
		

		prepareAllFigures(total_n_figures); 

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		
		//  OpenGL settings
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_NORMALIZE);

		// enter GLUT's main cycle
		glutMainLoop();

		return 1;
	}

	else return 0;
}


#pragma GCC diagnostic pop
