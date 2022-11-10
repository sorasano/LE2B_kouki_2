#include "Gravity.h"

void Gravity::Initialize(float x_, float y_, float z_, float weight_)
{
	x = x_;
	y = y_;
	z = z_;
	weight = weight_;
}

void Gravity::Update(Gravity g)
{
	if (abs(x - g.x) >= 0.01f)
	{
		if (x - g.x < 0)
		{
			x = x + ((weight * g.weight) / (length(g) * length(g))) * G * VecGetX(g) * 100;
		}
		else
		{
			x = x - ((weight * g.weight) / (length(g) * length(g))) * G * VecGetX(g) * 100;
		}
	}
	if (abs(y - g.y) >= 0.01f)
	{
		if (y - g.y < 0)
		{
			y = y + ((weight * g.weight) / (length(g) * length(g))) * G * VecGetY(g) * 100;
		}
		else
		{
			y = y - ((weight * g.weight) / (length(g) * length(g))) * G * VecGetY(g) * 100;
		}
	}
	if (abs(z - g.z) >= 0.01f)
	{
		if (z - g.z < 0)
		{
			z  = z + ((weight * g.weight) / (length(g) * length(g))) * G * VecGetZ(g) * 100;
		}
		else
		{
			z = z - ((weight * g.weight) / (length(g) * length(g))) * G * VecGetZ(g) * 100;
		}
	}
}

float Gravity::length(Gravity g)
{
	float x2 = abs(x - g.x) * abs(x - g.x);
	float y2 = abs(y - g.y) * abs(y - g.y);
	float z2 = abs(z - g.z) * abs(z - g.z);
	return sqrt(x2 + y2 + z2);
}

float Gravity::VecGetX(Gravity g)
{
	return abs(g.x - x) / length(g);
}

float Gravity::VecGetY(Gravity g)
{
	return abs(g.y - y) / length(g);
}

float Gravity::VecGetZ(Gravity g)
{
	return abs(g.z - z) / length(g);
}
