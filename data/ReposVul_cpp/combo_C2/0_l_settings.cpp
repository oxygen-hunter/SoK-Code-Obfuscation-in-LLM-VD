#include "lua_api/l_settings.h"
#include "lua_api/l_internal.h"
#include "cpp_api/s_security.h"
#include "threading/mutex_auto_lock.h"
#include "util/string.h"
#include "settings.h"
#include "noise.h"
#include "log.h"

#define CHECK_SETTING_SECURITY(L, name) \
	if (o->m_settings == g_settings) { \
		if (checkSettingSecurity(L, name) == -1) \
			return 0; \
	}

static inline int checkSettingSecurity(lua_State* L, const std::string &name)
{
	if (ScriptApiSecurity::isSecure(L) && name.compare(0, 7, "secure.") == 0)
		throw LuaError("Attempted to set secure setting.");

	bool is_mainmenu = false;
#ifndef SERVER
	is_mainmenu = ModApiBase::getGuiEngine(L) != nullptr;
#endif
	if (!is_mainmenu && (name == "mg_name" || name == "mg_flags")) {
		errorstream << "Tried to set global setting " << name << ", ignoring. "
			"minetest.set_mapgen_setting() should be used instead." << std::endl;
		infostream << script_get_backtrace(L) << std::endl;
		return -1;
	}

	const char *disallowed[] = {
		"main_menu_script", "shader_path", "texture_path", "screenshot_path",
		"serverlist_file", "serverlist_url", "map-dir", "contentdb_url",
	};
	if (!is_mainmenu) {
		for (const char *name2 : disallowed) {
			if (name == name2)
				throw LuaError("Attempted to set disallowed setting.");
		}
	}

	return 0;
}

LuaSettings::LuaSettings(Settings *settings, const std::string &filename) :
	m_settings(settings),
	m_filename(filename)
{
}

LuaSettings::LuaSettings(const std::string &filename, bool write_allowed) :
	m_filename(filename),
	m_is_own_settings(true),
	m_write_allowed(write_allowed)
{
	m_settings = new Settings();
	m_settings->readConfigFile(filename.c_str());
}

LuaSettings::~LuaSettings()
{
	if (m_is_own_settings)
		delete m_settings;
}

void LuaSettings::create(lua_State *L, Settings *settings,
		const std::string &filename)
{
	LuaSettings *o = new LuaSettings(settings, filename);
	*(void **)(lua_newuserdata(L, sizeof(void *))) = o;
	luaL_getmetatable(L, className);
	lua_setmetatable(L, -2);
}

int LuaSettings::gc_object(lua_State* L)
{
	LuaSettings* o = *(LuaSettings **)(lua_touserdata(L, 1));
	delete o;
	return 0;
}

int LuaSettings::l_get(lua_State* L)
{
	NO_MAP_LOCK_REQUIRED;
	LuaSettings* o = checkobject(L, 1);

	int dispatcher = 0;
	std::string key;
	std::string value;

	while (true) {
		switch (dispatcher) {
			case 0:
				key = std::string(luaL_checkstring(L, 2));
				dispatcher = 1;
				break;
			case 1:
				if (o->m_settings->exists(key)) {
					dispatcher = 2;
				} else {
					dispatcher = 3;
				}
				break;
			case 2:
				value = o->m_settings->get(key);
				lua_pushstring(L, value.c_str());
				return 1;
			case 3:
				lua_pushnil(L);
				return 1;
		}
	}
}

int LuaSettings::l_get_bool(lua_State* L)
{
	NO_MAP_LOCK_REQUIRED;
	LuaSettings* o = checkobject(L, 1);

	int dispatcher = 0;
	std::string key;
	bool value;

	while (true) {
		switch (dispatcher) {
			case 0:
				key = std::string(luaL_checkstring(L, 2));
				dispatcher = 1;
				break;
			case 1:
				if (o->m_settings->exists(key)) {
					dispatcher = 2;
				} else {
					dispatcher = 3;
				}
				break;
			case 2:
				value = o->m_settings->getBool(key);
				lua_pushboolean(L, value);
				return 1;
			case 3:
				if (lua_isboolean(L, 3))
					lua_pushboolean(L, readParam<bool>(L, 3));
				else
					lua_pushnil(L);
				return 1;
		}
	}
}

int LuaSettings::l_get_np_group(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	LuaSettings *o = checkobject(L, 1);

	int dispatcher = 0;
	std::string key;
	NoiseParams np;

	while (true) {
		switch (dispatcher) {
			case 0:
				key = std::string(luaL_checkstring(L, 2));
				dispatcher = 1;
				break;
			case 1:
				if (o->m_settings->exists(key)) {
					dispatcher = 2;
				} else {
					dispatcher = 3;
				}
				break;
			case 2:
				o->m_settings->getNoiseParams(key, np);
				push_noiseparams(L, &np);
				return 1;
			case 3:
				lua_pushnil(L);
				return 1;
		}
	}
}

int LuaSettings::l_get_flags(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	LuaSettings *o = checkobject(L, 1);

	int dispatcher = 0;
	std::string key;
	u32 flags;
	auto flagdesc = o->m_settings->getFlagDescFallback(key);

	while (true) {
		switch (dispatcher) {
			case 0:
				key = std::string(luaL_checkstring(L, 2));
				dispatcher = 1;
				break;
			case 1:
				if (o->m_settings->getFlagStrNoEx(key, flags, flagdesc)) {
					dispatcher = 2;
				} else {
					dispatcher = 3;
				}
				break;
			case 2:
				lua_newtable(L);
				int table = lua_gettop(L);
				for (size_t i = 0; flagdesc[i].name; ++i) {
					lua_pushboolean(L, flags & flagdesc[i].flag);
					lua_setfield(L, table, flagdesc[i].name);
				}
				lua_pushvalue(L, table);
				return 1;
			case 3:
				lua_pushnil(L);
				return 1;
		}
	}
}

int LuaSettings::l_set(lua_State* L)
{
	NO_MAP_LOCK_REQUIRED;
	LuaSettings* o = checkobject(L, 1);

	int dispatcher = 0;
	std::string key;
	const char* value;

	while (true) {
		switch (dispatcher) {
			case 0:
				key = std::string(luaL_checkstring(L, 2));
				dispatcher = 1;
				break;
			case 1:
				value = luaL_checkstring(L, 3);
				dispatcher = 2;
				break;
			case 2:
				CHECK_SETTING_SECURITY(L, key);
				dispatcher = 3;
				break;
			case 3:
				if (!o->m_settings->set(key, value))
					throw LuaError("Invalid sequence found in setting parameters");
				return 0;
		}
	}
}

int LuaSettings::l_set_bool(lua_State* L)
{
	NO_MAP_LOCK_REQUIRED;
	LuaSettings* o = checkobject(L, 1);

	int dispatcher = 0;
	std::string key;
	bool value;

	while (true) {
		switch (dispatcher) {
			case 0:
				key = std::string(luaL_checkstring(L, 2));
				dispatcher = 1;
				break;
			case 1:
				value = readParam<bool>(L, 3);
				dispatcher = 2;
				break;
			case 2:
				CHECK_SETTING_SECURITY(L, key);
				dispatcher = 3;
				break;
			case 3:
				o->m_settings->setBool(key, value);
				return 0;
		}
	}
}

int LuaSettings::l_set_np_group(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	LuaSettings *o = checkobject(L, 1);

	int dispatcher = 0;
	std::string key;
	NoiseParams value;

	while (true) {
		switch (dispatcher) {
			case 0:
				key = std::string(luaL_checkstring(L, 2));
				dispatcher = 1;
				break;
			case 1:
				read_noiseparams(L, 3, &value);
				dispatcher = 2;
				break;
			case 2:
				CHECK_SETTING_SECURITY(L, key);
				dispatcher = 3;
				break;
			case 3:
				o->m_settings->setNoiseParams(key, value);
				return 0;
		}
	}
}

int LuaSettings::l_remove(lua_State* L)
{
	NO_MAP_LOCK_REQUIRED;
	LuaSettings* o = checkobject(L, 1);

	int dispatcher = 0;
	std::string key;
	bool success;

	while (true) {
		switch (dispatcher) {
			case 0:
				key = std::string(luaL_checkstring(L, 2));
				dispatcher = 1;
				break;
			case 1:
				CHECK_SETTING_SECURITY(L, key);
				dispatcher = 2;
				break;
			case 2:
				success = o->m_settings->remove(key);
				lua_pushboolean(L, success);
				return 1;
		}
	}
}

int LuaSettings::l_get_names(lua_State* L)
{
	NO_MAP_LOCK_REQUIRED;
	LuaSettings* o = checkobject(L, 1);

	int dispatcher = 0;
	std::vector<std::string> keys;

	while (true) {
		switch (dispatcher) {
			case 0:
				keys = o->m_settings->getNames();
				dispatcher = 1;
				break;
			case 1:
				lua_newtable(L);
				for (unsigned int i=0; i < keys.size(); i++)
				{
					lua_pushstring(L, keys[i].c_str());
					lua_rawseti(L, -2, i + 1);
				}
				return 1;
		}
	}
}

int LuaSettings::l_write(lua_State* L)
{
	NO_MAP_LOCK_REQUIRED;
	LuaSettings* o = checkobject(L, 1);

	int dispatcher = 0;
	bool success;

	while (true) {
		switch (dispatcher) {
			case 0:
				if (!o->m_write_allowed) {
					throw LuaError("Settings: writing " + o->m_filename +
							" not allowed with mod security on.");
				}
				dispatcher = 1;
				break;
			case 1:
				success = o->m_settings->updateConfigFile(o->m_filename.c_str());
				lua_pushboolean(L, success);
				return 1;
		}
	}
}

static void push_settings_table(lua_State *L, const Settings *settings)
{
	std::vector<std::string> keys = settings->getNames();
	lua_newtable(L);
	for (const std::string &key : keys) {
		std::string value;
		Settings *group = nullptr;

		if (settings->getNoEx(key, value)) {
			lua_pushstring(L, value.c_str());
		} else if (settings->getGroupNoEx(key, group)) {
			push_settings_table(L, group);
		} else {
			continue;
		}

		lua_setfield(L, -2, key.c_str());
	}
}

int LuaSettings::l_to_table(lua_State* L)
{
	NO_MAP_LOCK_REQUIRED;
	LuaSettings* o = checkobject(L, 1);

	MutexAutoLock(o->m_settings->m_mutex);
	push_settings_table(L, o->m_settings);
	return 1;
}

void LuaSettings::Register(lua_State* L)
{
	lua_newtable(L);
	int methodtable = lua_gettop(L);
	luaL_newmetatable(L, className);
	int metatable = lua_gettop(L);

	lua_pushliteral(L, "__metatable");
	lua_pushvalue(L, methodtable);
	lua_settable(L, metatable);

	lua_pushliteral(L, "__index");
	lua_pushvalue(L, methodtable);
	lua_settable(L, metatable);

	lua_pushliteral(L, "__gc");
	lua_pushcfunction(L, gc_object);
	lua_settable(L, metatable);

	lua_pop(L, 1);

	luaL_register(L, nullptr, methods);
	lua_pop(L, 1);

	lua_register(L, className, create_object);
}

int LuaSettings::create_object(lua_State* L)
{
	NO_MAP_LOCK_REQUIRED;
	bool write_allowed = true;
	const char* filename = luaL_checkstring(L, 1);
	CHECK_SECURE_PATH_POSSIBLE_WRITE(L, filename, &write_allowed);
	LuaSettings* o = new LuaSettings(filename, write_allowed);
	*(void **)(lua_newuserdata(L, sizeof(void *))) = o;
	luaL_getmetatable(L, className);
	lua_setmetatable(L, -2);
	return 1;
}

LuaSettings* LuaSettings::checkobject(lua_State* L, int narg)
{
	NO_MAP_LOCK_REQUIRED;
	luaL_checktype(L, narg, LUA_TUSERDATA);
	void *ud = luaL_checkudata(L, narg, className);
	if (!ud)
		luaL_typerror(L, narg, className);
	return *(LuaSettings**) ud;
}

const char LuaSettings::className[] = "Settings";
const luaL_Reg LuaSettings::methods[] = {
	luamethod(LuaSettings, get),
	luamethod(LuaSettings, get_bool),
	luamethod(LuaSettings, get_np_group),
	luamethod(LuaSettings, get_flags),
	luamethod(LuaSettings, set),
	luamethod(LuaSettings, set_bool),
	luamethod(LuaSettings, set_np_group),
	luamethod(LuaSettings, remove),
	luamethod(LuaSettings, get_names),
	luamethod(LuaSettings, write),
	luamethod(LuaSettings, to_table),
	{0,0}
};