#pragma once

namespace Hooks
{
	void initHooks();
	void CMDPatchHooks();

}

namespace SaveSystem {
	void SaveWeaponInst(const NVSEInterface* nvse, PluginHandle& pluginHandle);
}

namespace PluginFunctions {

	void initkNVSE();
	void initJIP();

}