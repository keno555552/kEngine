#pragma once
#include "Scene.h"

class Menu :public Scene
{
	public:
	Menu(kEngine* system);

private:
	kEngine* system_ = nullptr;				// 借り

};

