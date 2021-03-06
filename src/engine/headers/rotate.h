#ifndef ROTATE_OP
#define ROTATE_OP

#include "operation.h"
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include "glut.h"
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

class Rotate : public Operation {
	float angle, x, y, z;
    
    public:
        Rotate(double a, double xx, double yy, double zz) {
            angle = a;
            x = xx;
            y = yy;
            z = zz;
        }
    
        void transformacao(){

            glRotatef(angle, x, y, z);
        }
};

class DynamicRotate : public Operation {
	float time, x, y, z;

public:
	DynamicRotate(double t, double xx, double yy, double zz) {
		time = t;
		x = xx;
		y = yy;
		z = zz;
	}

	void transformacao() {

		glRotatef((glutGet(GLUT_ELAPSED_TIME)/(time*1000))*360, x, y, z);
	}
};

#pragma GCC diagnostic pop
#endif

