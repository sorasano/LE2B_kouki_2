#pragma once
#include "math.h"
#include "cmath"
#define G 6.674	//–œ—Lˆø—Í’è”

class Gravity
{
public:
	void Initialize(float x_,float y_, float z_,float weight_);
	void Update(Gravity g);
public:
	float length(Gravity g);	//	’·‚³‚ğ•Ô‚·
	float VecGetX(Gravity g);	//³‹K‰»
	float VecGetY(Gravity g);	//³‹K‰»
	float VecGetZ(Gravity g);	//³‹K‰»
public:
	float weight;
	float x;		//d—Í‚ğ‚½‚¹‚é“_‚ÌÀ•W
	float y;
	float z;
	float x2;
	float y2;
	float z2;
	float r;
};

