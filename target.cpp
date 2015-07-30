#include <sys/time.h>
#include <iostream>
#include <cmath>
#include <GL/glut.h>
#include <sstream>
#include "vec3f.h"

using namespace std;

#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)

class Target
{
public:
	float tarx, tarz, tarr1, tarr2, tarr3;
	Target()
	{
		tarr1 = 2.5f;
		tarr3 = 3.5f;
		tarr2 = 3.0f;
	} 
};