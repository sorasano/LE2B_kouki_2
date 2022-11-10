#include "PlayerEffect.h"

PlayerEffect* PlayerEffect::GetInstance()
{
	static PlayerEffect instance;
	return &instance;
}

PlayerEffect::PlayerEffect()
{
}

PlayerEffect::~PlayerEffect()
{
}
