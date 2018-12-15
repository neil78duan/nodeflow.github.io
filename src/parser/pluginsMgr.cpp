/* file pluginsMgr.cpp
 *
 * plugins manager, include dll-base-plugin and script-base-plugin
 *
 * create by duan
 *
 * 2018.12.7
 */

#include "logic_parser/pluginsMgr.h"
#include "logic_parser/logicEngineRoot.h"


PluginsMgr::PluginsMgr()
{
}

PluginsMgr::~PluginsMgr()
{
}


bool PluginsMgr::loadPlugin(const char *pluginName, const char *pluginPath, bool isSystem)
{
	if (!pluginName || !pluginPath) {
		nd_logerror("load dll:input name error\n");
		return false;
	}

	plugin_map &plugins = isSystem ? m_sysPlugins : m_plugins;

	plugin_map::iterator it = plugins.find(pluginName);
	if (it != plugins.end()) {
		nd_logerror("load dll:%s already loaded\n", pluginName);
		return false;
	}
	HINSTANCE hinst = nd_dll_load(pluginPath);
	if (!hinst) {
		nd_logerror("load dll:%s\n", nd_last_error());
		return false;
	}
	std::string init_call = "nodeflow_init_";
	init_call += pluginName;

	logic_plugin_init_func initfunc = (logic_plugin_init_func)nd_dll_entry(hinst, init_call.c_str());
	if (initfunc) {
		if (-1 == initfunc()) {
			nd_dll_unload(hinst);

			nd_logerror("called function:%s\n", init_call.c_str());
			return false;
		}
	}
	plugins[pluginName] = hinst;
	return true;
}
bool PluginsMgr::unLoadPlugin(const char *pluginName, bool isSystem)
{
	if (!pluginName) {
		nd_logerror("load dll:input name error\n");
		return false;
	}
	plugin_map &plugins = isSystem ? m_sysPlugins : m_plugins;

	plugin_map::iterator it = plugins.find(pluginName);
	if (it == plugins.end()) {
		nd_logerror("load dll:%s not had been loaded\n", pluginName);
		return false;
	}

	std::string init_call = "nodeflow_destroy_";
	init_call += pluginName;

	logic_plugin_init_func initfunc = (logic_plugin_init_func)nd_dll_entry(it->second, init_call.c_str());
	if (initfunc) {
		initfunc();
		nd_dll_unload(it->second);
	}
	plugins.erase(it);
	return true;
}

void PluginsMgr::DestroyPlugins(bool isSystem)
{
	plugin_map &plugins = isSystem ? m_sysPlugins : m_plugins;

	for (plugin_map::iterator it = plugins.begin(); it != plugins.end(); ++it) {
		std::string init_call = "nodeflow_destroy_";
		init_call += it->first.c_str();

		logic_plugin_init_func initfunc = (logic_plugin_init_func)nd_dll_entry(it->second, init_call.c_str());
		if (initfunc) {
			initfunc();
			nd_dll_unload(it->second);
		}
	}
	plugins.clear();
}