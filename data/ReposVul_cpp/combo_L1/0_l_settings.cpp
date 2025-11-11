#include "lua_api/l_settings.h"
#include "lua_api/l_internal.h"
#include "cpp_api/s_security.h"
#include "threading/mutex_auto_lock.h"
#include "util/string.h" // FlagDesc
#include "settings.h"
#include "noise.h"
#include "log.h"

#define OX0A7D9CCF(L, OXED8B1397) \
	if (OXDABAFB39->OX2A936C05 == g_settings) { \
		if (OX7B4DF339(L, OXED8B1397) == -1) \
			return 0; \
	}

static inline int OX7B4DF339(lua_State* L, const std::string &OXED8B1397)
{
	if (ScriptApiSecurity::isSecure(L) && OXED8B1397.compare(0, 7, "secure.") == 0)
		throw LuaError("Attempted to set secure setting.");

	bool OXED1F5BC6 = false;
#ifndef SERVER
	OXED1F5BC6 = ModApiBase::getGuiEngine(L) != nullptr;
#endif
	if (!OXED1F5BC6 && (OXED8B1397 == "mg_name" || OXED8B1397 == "mg_flags")) {
		errorstream << "Tried to set global setting " << OXED8B1397 << ", ignoring. "
			"minetest.set_mapgen_setting() should be used instead." << std::endl;
		infostream << script_get_backtrace(L) << std::endl;
		return -1;
	}

	const char *OXE6A7B7B7[] = {
		"main_menu_script", "shader_path", "texture_path", "screenshot_path",
		"serverlist_file", "serverlist_url", "map-dir", "contentdb_url",
	};
	if (!OXED1F5BC6) {
		for (const char *OX7A0E8D80 : OXE6A7B7B7) {
			if (OXED8B1397 == OX7A0E8D80)
				throw LuaError("Attempted to set disallowed setting.");
		}
	}

	return 0;
}

class OX4E4B8A00 {
public:
	OX4E4B8A00(Settings *OX2A936C05, const std::string &OX2B5F8E0D) :
		OX2A936C05(OX2A936C05),
		OX2B5F8E0D(OX2B5F8E0D)
	{
	}

	OX4E4B8A00(const std::string &OX2B5F8E0D, bool OXF36B9BCA) :
		OX2B5F8E0D(OX2B5F8E0D),
		OX5B39A81B(true),
		OXF36B9BCA(OXF36B9BCA)
	{
		OX2A936C05 = new Settings();
		OX2A936C05->readConfigFile(OX2B5F8E0D.c_str());
	}

	~OX4E4B8A00()
	{
		if (OX5B39A81B)
			delete OX2A936C05;
	}

	void OXD5E5E55E(lua_State *L, Settings *OX2A936C05,
			const std::string &OX2B5F8E0D)
	{
		OX4E4B8A00 *OXDABAFB39 = new OX4E4B8A00(OX2A936C05, OX2B5F8E0D);
		*(void **)(lua_newuserdata(L, sizeof(void *))) = OXDABAFB39;
		luaL_getmetatable(L, OX1C6E0606);
		lua_setmetatable(L, -2);
	}

	int OX5066F8F6(lua_State* L)
	{
		OX4E4B8A00* OXDABAFB39 = *(OX4E4B8A00 **)(lua_touserdata(L, 1));
		delete OXDABAFB39;
		return 0;
	}

	int OX9A1F5AD7(lua_State* L)
	{
		NO_MAP_LOCK_REQUIRED;
		OX4E4B8A00* OXDABAFB39 = OX1F98B7BC(L, 1);

		std::string OX18E6B8F6 = std::string(luaL_checkstring(L, 2));
		if (OXDABAFB39->OX2A936C05->exists(OX18E6B8F6)) {
			std::string OX9DDFB58D = OXDABAFB39->OX2A936C05->get(OX18E6B8F6);
			lua_pushstring(L, OX9DDFB58D.c_str());
		} else {
			lua_pushnil(L);
		}

		return 1;
	}

	int OX9D1E2B92(lua_State* L)
	{
		NO_MAP_LOCK_REQUIRED;
		OX4E4B8A00* OXDABAFB39 = OX1F98B7BC(L, 1);

		std::string OX18E6B8F6 = std::string(luaL_checkstring(L, 2));
		if (OXDABAFB39->OX2A936C05->exists(OX18E6B8F6)) {
			bool OX9DDFB58D = OXDABAFB39->OX2A936C05->getBool(OX18E6B8F6);
			lua_pushboolean(L, OX9DDFB58D);
		} else {
			if (lua_isboolean(L, 3))
				lua_pushboolean(L, readParam<bool>(L, 3));
			else
				lua_pushnil(L);
		}

		return 1;
	}

	int OX5D4F9DDC(lua_State *L)
	{
		NO_MAP_LOCK_REQUIRED;
		OX4E4B8A00 *OXDABAFB39 = OX1F98B7BC(L, 1);

		std::string OX18E6B8F6 = std::string(luaL_checkstring(L, 2));
		if (OXDABAFB39->OX2A936C05->exists(OX18E6B8F6)) {
			NoiseParams OX9DDFB58D;
			OXDABAFB39->OX2A936C05->getNoiseParams(OX18E6B8F6, OX9DDFB58D);
			push_noiseparams(L, &OX9DDFB58D);
		} else {
			lua_pushnil(L);
		}

		return 1;
	}

	int OX5EC6C95D(lua_State *L)
	{
		NO_MAP_LOCK_REQUIRED;
		OX4E4B8A00 *OXDABAFB39 = OX1F98B7BC(L, 1);
		std::string OX18E6B8F6 = std::string(luaL_checkstring(L, 2));

		u32 OX1BFF3D30 = 0;
		auto OX0F66432B = OXDABAFB39->OX2A936C05->getFlagDescFallback(OX18E6B8F6);
		if (OXDABAFB39->OX2A936C05->getFlagStrNoEx(OX18E6B8F6, OX1BFF3D30, OX0F66432B)) {
			lua_newtable(L);
			int OX2C279F3B = lua_gettop(L);
			for (size_t i = 0; OX0F66432B[i].name; ++i) {
				lua_pushboolean(L, OX1BFF3D30 & OX0F66432B[i].flag);
				lua_setfield(L, OX2C279F3B, OX0F66432B[i].name);
			}
			lua_pushvalue(L, OX2C279F3B);
		} else {
			lua_pushnil(L);
		}

		return 1;
	}

	int OXB8F28E64(lua_State* L)
	{
		NO_MAP_LOCK_REQUIRED;
		OX4E4B8A00* OXDABAFB39 = OX1F98B7BC(L, 1);

		std::string OX18E6B8F6 = std::string(luaL_checkstring(L, 2));
		const char* OX9DDFB58D = luaL_checkstring(L, 3);

		OX0A7D9CCF(L, OX18E6B8F6);

		if (!OXDABAFB39->OX2A936C05->set(OX18E6B8F6, OX9DDFB58D))
			throw LuaError("Invalid sequence found in setting parameters");

		return 0;
	}

	int OX8C842E4E(lua_State* L)
	{
		NO_MAP_LOCK_REQUIRED;
		OX4E4B8A00* OXDABAFB39 = OX1F98B7BC(L, 1);

		std::string OX18E6B8F6 = std::string(luaL_checkstring(L, 2));
		bool OX9DDFB58D = readParam<bool>(L, 3);

		OX0A7D9CCF(L, OX18E6B8F6);

		OXDABAFB39->OX2A936C05->setBool(OX18E6B8F6, OX9DDFB58D);

		return 0;
	}

	int OXCCCFAB86(lua_State *L)
	{
		NO_MAP_LOCK_REQUIRED;
		OX4E4B8A00 *OXDABAFB39 = OX1F98B7BC(L, 1);

		std::string OX18E6B8F6 = std::string(luaL_checkstring(L, 2));
		NoiseParams OX9DDFB58D;
		read_noiseparams(L, 3, &OX9DDFB58D);

		OX0A7D9CCF(L, OX18E6B8F6);

		OXDABAFB39->OX2A936C05->setNoiseParams(OX18E6B8F6, OX9DDFB58D);

		return 0;
	}

	int OX6FDC00F9(lua_State* L)
	{
		NO_MAP_LOCK_REQUIRED;
		OX4E4B8A00* OXDABAFB39 = OX1F98B7BC(L, 1);

		std::string OX18E6B8F6 = std::string(luaL_checkstring(L, 2));

		OX0A7D9CCF(L, OX18E6B8F6);

		bool OX9DDFB58D = OXDABAFB39->OX2A936C05->remove(OX18E6B8F6);
		lua_pushboolean(L, OX9DDFB58D);

		return 1;
	}

	int OX65E71E5C(lua_State* L)
	{
		NO_MAP_LOCK_REQUIRED;
		OX4E4B8A00* OXDABAFB39 = OX1F98B7BC(L, 1);

		std::vector<std::string> OXC7FD3A3D = OXDABAFB39->OX2A936C05->getNames();

		lua_newtable(L);
		for (unsigned int i=0; i < OXC7FD3A3D.size(); i++)
		{
			lua_pushstring(L, OXC7FD3A3D[i].c_str());
			lua_rawseti(L, -2, i + 1);
		}

		return 1;
	}

	int OXA6F1EEAC(lua_State* L)
	{
		NO_MAP_LOCK_REQUIRED;
		OX4E4B8A00* OXDABAFB39 = OX1F98B7BC(L, 1);

		if (!OXDABAFB39->OXF36B9BCA) {
			throw LuaError("Settings: writing " + OXDABAFB39->OX2B5F8E0D +
					" not allowed with mod security on.");
		}

		bool OX9DDFB58D = OXDABAFB39->OX2A936C05->updateConfigFile(OXDABAFB39->OX2B5F8E0D.c_str());
		lua_pushboolean(L, OX9DDFB58D);

		return 1;
	}

	static void OX3F06337A(lua_State *L, const Settings *OX2A936C05)
	{
		std::vector<std::string> OXC7FD3A3D = OX2A936C05->getNames();
		lua_newtable(L);
		for (const std::string &OX18E6B8F6 : OXC7FD3A3D) {
			std::string OX9DDFB58D;
			Settings *OX2F9F2A78 = nullptr;

			if (OX2A936C05->getNoEx(OX18E6B8F6, OX9DDFB58D)) {
				lua_pushstring(L, OX9DDFB58D.c_str());
			} else if (OX2A936C05->getGroupNoEx(OX18E6B8F6, OX2F9F2A78)) {
				OX3F06337A(L, OX2F9F2A78);
			} else {
				continue;
			}

			lua_setfield(L, -2, OX18E6B8F6.c_str());
		}
	}

	int OX4B08C6E8(lua_State* L)
	{
		NO_MAP_LOCK_REQUIRED;
		OX4E4B8A00* OXDABAFB39 = OX1F98B7BC(L, 1);

		MutexAutoLock(OXDABAFB39->OX2A936C05->m_mutex);
		OX3F06337A(L, OXDABAFB39->OX2A936C05);
		return 1;
	}

	static void OX0F4D5C2B(lua_State* L)
	{
		lua_newtable(L);
		int OX0F9A9D94 = lua_gettop(L);
		luaL_newmetatable(L, OX1C6E0606);
		int OXCF45E4A7 = lua_gettop(L);

		lua_pushliteral(L, "__metatable");
		lua_pushvalue(L, OX0F9A9D94);
		lua_settable(L, OXCF45E4A7);

		lua_pushliteral(L, "__index");
		lua_pushvalue(L, OX0F9A9D94);
		lua_settable(L, OXCF45E4A7);

		lua_pushliteral(L, "__gc");
		lua_pushcfunction(L, OX5066F8F6);
		lua_settable(L, OXCF45E4A7);

		lua_pop(L, 1);

		luaL_register(L, nullptr, OX5F0D7B9E);
		lua_pop(L, 1);

		lua_register(L, OX1C6E0606, OXF7E0B9D7);
	}

	int OXF7E0B9D7(lua_State* L)
	{
		NO_MAP_LOCK_REQUIRED;
		bool OXF36B9BCA = true;
		const char* OX2B5F8E0D = luaL_checkstring(L, 1);
		CHECK_SECURE_PATH_POSSIBLE_WRITE(L, OX2B5F8E0D, &OXF36B9BCA);
		OX4E4B8A00* OXDABAFB39 = new OX4E4B8A00(OX2B5F8E0D, OXF36B9BCA);
		*(void **)(lua_newuserdata(L, sizeof(void *))) = OXDABAFB39;
		luaL_getmetatable(L, OX1C6E0606);
		lua_setmetatable(L, -2);
		return 1;
	}

	OX4E4B8A00* OX1F98B7BC(lua_State* L, int OX3B8D1C3A)
	{
		NO_MAP_LOCK_REQUIRED;
		luaL_checktype(L, OX3B8D1C3A, LUA_TUSERDATA);
		void *OX5F3B8C3E = luaL_checkudata(L, OX3B8D1C3A, OX1C6E0606);
		if (!OX5F3B8C3E)
			luaL_typerror(L, OX3B8D1C3A, OX1C6E0606);
		return *(OX4E4B8A00**) OX5F3B8C3E;
	}

	const char OX1C6E0606[] = "Settings";
	const luaL_Reg OX5F0D7B9E[] = {
		{"get", OX9A1F5AD7},
		{"get_bool", OX9D1E2B92},
		{"get_np_group", OX5D4F9DDC},
		{"get_flags", OX5EC6C95D},
		{"set", OXB8F28E64},
		{"set_bool", OX8C842E4E},
		{"set_np_group", OXCCCFAB86},
		{"remove", OX6FDC00F9},
		{"get_names", OX65E71E5C},
		{"write", OXA6F1EEAC},
		{"to_table", OX4B08C6E8},
		{0, 0}
	};
};