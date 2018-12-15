/* file pluginsMgr.h
 *
 * plugins manager, include dll-base-plugin and script-base-plugin
 *
 * create by duan
 *
 * 2018.12.7
 */

#ifndef _PLUGINS_MGR_H_
#define _PLUGINS_MGR_H_

#include "nd_common/nd_common.h"
#include "logic_parser/logicApi4c.h"
#include <map>
#include <string>
#include <vector>

typedef int(*logic_plugin_init_func)();
typedef void(*logic_plugin_menu_func)();

struct menu_func_info
{
	char name[64];
	logic_plugin_menu_func func;
};
struct menu_func_info_script
{
	char name[64];
	char funcname[64];
};

typedef menu_func_info * (*get_plugin_menu) (int *);

class LOGIC_PARSER_CLASS PluginsMgr
{
public:
	PluginsMgr();
	~PluginsMgr();

	bool loadPlugin(const char *pluginName, const char *pluginPath, bool isSystem = false);
	bool unLoadPlugin(const char *pluginName, bool isSystem = false);
	void DestroyPlugins(bool isSystem = false);

	bool loadScriptPlugin(const char *pluginName, const char *pluginPath, bool isSystem = false);
	bool unLoadScriptPlugin(const char *pluginName, bool isSystem = false);

	typedef std::vector<menu_func_info>menu_func_vct;
	typedef std::vector<menu_func_info_script>menu_script_vct;
	typedef std::map<std::string, HINSTANCE> plugin_map;
	typedef std::map<std::string, menu_func_vct> plugin_menus_map;
	typedef std::map<std::string, menu_script_vct> plugin_menus_script_map;

	typedef std::vector<std::string> plugin_script_vct;
private:
	bool _loadMenus(HINSTANCE hPlugin);
	bool _loadScriptMenus(const char *pluginName);

	plugin_map m_plugins;
	plugin_map m_sysPlugins;
	plugin_menus_map m_menus;

	plugin_script_vct m_scriptPlugins;
	plugin_menus_script_map m_scriptsMenu;

};

#endif
