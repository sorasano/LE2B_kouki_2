#pragma once
#include "math.h"
#include "cmath"
#define G 6.674	//���L���͒萔

class Gravity
{
public:
	void Initialize(float x_,float y_, float z_,float weight_);
	void Update(Gravity g);
public:
	float length(Gravity g);	//	������Ԃ�
	float VecGetX(Gravity g);	//���K��
	float VecGetY(Gravity g);	//���K��
	float VecGetZ(Gravity g);	//���K��
public:
	float weight;
	float x;		//�d�͂���������_�̍��W
	float y;
	float z;
	float x2;
	float y2;
	float z2;
	float r;
};

