#pragma once
#include <commands_animation.h>

//Animation canceling

//extern bool g_SkipAnimation;

struct QueueAnim {
	UInt16 groupId;
	Actor* actor;
	float timer;
	int minFrames;
	int iter;
	bool wait;
};

extern std::vector<QueueAnim> queueToSkipGroup;