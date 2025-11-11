#include <Python.h>
#include <iostream>
#include <vector>
#include <string>

extern "C" {
    static inline int checkSettingSecurity(lua_State* L, const std::string &name);
    int luaopen_luasettings(lua_State* L);
}

#define NO_MAP_LOCK_REQUIRED
#define CHECK_SETTING_SECURITY(L, name) \
    if (o->m_settings == g_settings) { \
        if (checkSettingSecurity(L, name) == -1) \
            return 0; \
    }

class Settings {
public:
    bool exists(const std::string &key) const { return true; }
    std::string get(const std::string &key) const { return "value"; }
    bool getBool(const std::string &key) const { return true; }
    void readConfigFile(const char* filename) {}
    void setBool(const std::string &key, bool value) {}
    bool updateConfigFile(const char* filename) { return true; }
    bool remove(const std::string &key) { return true; }
    std::vector<std::string> getNames() const { return {"key1", "key2"}; }
};

class LuaSettings {
public:
    Settings *m_settings;
    std::string m_filename;
    bool m_is_own_settings;
    bool m_write_allowed;

    LuaSettings(Settings *settings, const std::string &filename);
    LuaSettings(const std::string &filename, bool write_allowed);
    ~LuaSettings();
    static void create(lua_State *L, Settings *settings, const std::string &filename);
    static int gc_object(lua_State* L);
    static int l_get(lua_State* L);
    static int l_get_bool(lua_State* L);
    static int l_set(lua_State* L);
    static int l_set_bool(lua_State* L);
    static int l_remove(lua_State* L);
    static int l_get_names(lua_State* L);
    static int l_write(lua_State* L);
    static void Register(lua_State* L);
    static int create_object(lua_State* L);
    static LuaSettings* checkobject(lua_State* L, int narg);
    static const char className[];
    static const luaL_Reg methods[];
};

LuaSettings::LuaSettings(Settings *settings, const std::string &filename)
    : m_settings(settings), m_filename(filename), m_is_own_settings(false), m_write_allowed(true) {}

LuaSettings::LuaSettings(const std::string &filename, bool write_allowed)
    : m_filename(filename), m_is_own_settings(true), m_write_allowed(write_allowed) {
    m_settings = new Settings();
    m_settings->readConfigFile(filename.c_str());
}

LuaSettings::~LuaSettings() {
    if (m_is_own_settings)
        delete m_settings;
}

void LuaSettings::create(lua_State *L, Settings *settings, const std::string &filename) {
    LuaSettings *o = new LuaSettings(settings, filename);
    *(void **)(lua_newuserdata(L, sizeof(void *))) = o;
    luaL_getmetatable(L, className);
    lua_setmetatable(L, -2);
}

int LuaSettings::gc_object(lua_State* L) {
    LuaSettings* o = *(LuaSettings **)(lua_touserdata(L, 1));
    delete o;
    return 0;
}

int LuaSettings::l_get(lua_State* L) {
    NO_MAP_LOCK_REQUIRED;
    LuaSettings* o = checkobject(L, 1);

    std::string key = std::string(luaL_checkstring(L, 2));
    if (o->m_settings->exists(key)) {
        std::string value = o->m_settings->get(key);
        lua_pushstring(L, value.c_str());
    } else {
        lua_pushnil(L);
    }

    return 1;
}

int LuaSettings::l_get_bool(lua_State* L) {
    NO_MAP_LOCK_REQUIRED;
    LuaSettings* o = checkobject(L, 1);

    std::string key = std::string(luaL_checkstring(L, 2));
    if (o->m_settings->exists(key)) {
        bool value = o->m_settings->getBool(key);
        lua_pushboolean(L, value);
    } else {
        if (lua_isboolean(L, 3))
            lua_pushboolean(L, lua_toboolean(L, 3));
        else
            lua_pushnil(L);
    }

    return 1;
}

int LuaSettings::l_set(lua_State* L) {
    NO_MAP_LOCK_REQUIRED;
    LuaSettings* o = checkobject(L, 1);

    std::string key = std::string(luaL_checkstring(L, 2));
    const char* value = luaL_checkstring(L, 3);

    CHECK_SETTING_SECURITY(L, key);

    if (!o->m_settings->set(key, value))
        throw LuaError("Invalid sequence found in setting parameters");

    return 0;
}

int LuaSettings::l_set_bool(lua_State* L) {
    NO_MAP_LOCK_REQUIRED;
    LuaSettings* o = checkobject(L, 1);

    std::string key = std::string(luaL_checkstring(L, 2));
    bool value = lua_toboolean(L, 3);

    CHECK_SETTING_SECURITY(L, key);

    o->m_settings->setBool(key, value);

    return 0;
}

int LuaSettings::l_remove(lua_State* L) {
    NO_MAP_LOCK_REQUIRED;
    LuaSettings* o = checkobject(L, 1);

    std::string key = std::string(luaL_checkstring(L, 2));

    CHECK_SETTING_SECURITY(L, key);

    bool success = o->m_settings->remove(key);
    lua_pushboolean(L, success);

    return 1;
}

int LuaSettings::l_get_names(lua_State* L) {
    NO_MAP_LOCK_REQUIRED;
    LuaSettings* o = checkobject(L, 1);

    std::vector<std::string> keys = o->m_settings->getNames();

    lua_newtable(L);
    for (unsigned int i=0; i < keys.size(); i++)
    {
        lua_pushstring(L, keys[i].c_str());
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

int LuaSettings::l_write(lua_State* L) {
    NO_MAP_LOCK_REQUIRED;
    LuaSettings* o = checkobject(L, 1);

    if (!o->m_write_allowed) {
        throw LuaError("Settings: writing " + o->m_filename +
                " not allowed with mod security on.");
    }

    bool success = o->m_settings->updateConfigFile(o->m_filename.c_str());
    lua_pushboolean(L, success);

    return 1;
}

void LuaSettings::Register(lua_State* L) {
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

int LuaSettings::create_object(lua_State* L) {
    NO_MAP_LOCK_REQUIRED;
    bool write_allowed = true;
    const char* filename = luaL_checkstring(L, 1);
    LuaSettings* o = new LuaSettings(filename, write_allowed);
    *(void **)(lua_newuserdata(L, sizeof(void *))) = o;
    luaL_getmetatable(L, className);
    lua_setmetatable(L, -2);
    return 1;
}

LuaSettings* LuaSettings::checkobject(lua_State* L, int narg) {
    NO_MAP_LOCK_REQUIRED;
    luaL_checktype(L, narg, LUA_TUSERDATA);
    void *ud = luaL_checkudata(L, narg, className);
    if (!ud)
        luaL_typerror(L, narg, className);
    return *(LuaSettings**) ud;
}

const char LuaSettings::className[] = "Settings";
const luaL_Reg LuaSettings::methods[] = {
    {"get", l_get},
    {"get_bool", l_get_bool},
    {"set", l_set},
    {"set_bool", l_set_bool},
    {"remove", l_remove},
    {"get_names", l_get_names},
    {"write", l_write},
    {0,0}
};

static PyObject* PyInit_mylib() {
    return nullptr;
}

int luaopen_luasettings(lua_State* L) {
    Py_Initialize();
    PyImport_AppendInittab("mylib", PyInit_mylib);
    PyRun_SimpleString("import mylib");
    LuaSettings::Register(L);
    return 1;
}