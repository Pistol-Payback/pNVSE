#pragma once

//#include "commands_animation.h"

namespace Hooks
{
	void initHooks();
}

namespace SaveSystem {
	void SaveWeaponInst(const NVSEInterface* nvse, PluginHandle& pluginHandle);
}