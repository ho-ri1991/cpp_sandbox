#include <iostream>
#include <sstream>
#include <cstdio>
#include <memory>
#include <lua5.3/lua.hpp>
#include "picojson.h"

const char* jsonString = 
"{"
"      \"Image\": {"
"          \"Width\":  800,"
"          \"Height\": 600,"
"          \"Title\":  \"View from 15th Floor\","
"          \"Thumbnail\": {"
"              \"Url\":    \"http://www.example.com/image/481989943\","
"              \"Height\": 125,"
"              \"Width\":  \"100\""
"          },"
"          \"IDs\": [116, 943, 234, 38793]"
"      },"
"      \"ID\": 100"
"}";

const char* ROTableMetaTableName = "ROTable";

struct JSONNode
{
  picojson::value* pNode = nullptr;
};

void createROTableHelper(lua_State* state, picojson::value& json)
{
  if (json.is<picojson::object>())
  {
    lua_pushlightuserdata(state, &json);
    JSONNode* node = reinterpret_cast<JSONNode*>(lua_newuserdata(state, sizeof(JSONNode)));
    node->pNode = &json;
    luaL_setmetatable(state, ROTableMetaTableName);
    lua_settable(state, LUA_REGISTRYINDEX);
    for (auto& pair: json.get<picojson::object>())
    {
      createROTableHelper(state, pair.second);
    }
  }
  else if (json.is<picojson::array>())
  {
    lua_pushlightuserdata(state, &json);
    JSONNode* node = reinterpret_cast<JSONNode*>(lua_newuserdata(state, sizeof(JSONNode)));
    node->pNode = &json;
    luaL_setmetatable(state, ROTableMetaTableName);
    lua_settable(state, LUA_REGISTRYINDEX);
    for (auto& elem: json.get<picojson::array>())
    {
      createROTableHelper(state, elem);
    }
  }
}

void createROTable(lua_State* state, picojson::value& json, const std::string& name)
{
  JSONNode* node = reinterpret_cast<JSONNode*>(lua_newuserdata(state, sizeof(JSONNode)));
  node->pNode = &json;
  luaL_setmetatable(state, ROTableMetaTableName);
  lua_setglobal(state, name.c_str());
  for (auto& pair: json.get<picojson::object>())
  {
    createROTableHelper(state, pair.second);
  }
}

static JSONNode* checkROTable(lua_State* state)
{
  void* ud = luaL_checkudata(state, 1, ROTableMetaTableName);
  luaL_argcheck(state, ud != NULL, 1, "\"ROTable\" expected");
  return reinterpret_cast<JSONNode*>(ud);
}

static int readOnlyTableGet(lua_State* state)
{
  if (lua_gettop(state) != 2)
  {
    return luaL_error(state, "wrong number of arguments");
  }
  auto node = checkROTable(state)->pNode;
  picojson::value* result = nullptr;
  if (node->is<picojson::object>())
  {
    std::string key = luaL_checklstring(state, 2, nullptr);
    auto& obj = node->get<picojson::object>();
    auto it = obj.find(key);
    if (it == obj.end())
    {
      return luaL_error(state, "invalid key");
    }
    result = &it->second;
  }
  else if (node->is<picojson::array>())
  {
    int index = luaL_checkinteger(state, 2);
    auto& arr = node->get<picojson::array>();
    if (index < 0 || arr.size() < static_cast<std::size_t>(index))
    {
      return luaL_error(state, "array out of range");
    }
    result = &arr[index];
  }

  if (!result)
  {
    return luaL_error(state, "this object does not have index access");
  }
  if (result->is<picojson::object>() || result->is<picojson::array>())
  {
    lua_pushlightuserdata(state, result);
    lua_gettable(state, LUA_REGISTRYINDEX);
  }
  else
  {
    if (result->is<double>())
    {
      lua_pushnumber(state, result->get<double>());
    }
    else if (result->is<std::string>())
    {
      lua_pushstring(state, result->get<std::string>().c_str());
    }
    else if (result->is<bool>())
    {
      lua_pushboolean(state, result->get<bool>());
    }
    else
    {
      return luaL_error(state, "invalid type");
    }
  }
  return 1;
}

static int readOnlyTableSet(lua_State* state)
{
  return luaL_error(state, "this object is readonly");
}

static int readOnlyTableLength(lua_State* state)
{
  auto node = checkROTable(state)->pNode;
  if (node->is<picojson::object>())
  {
    lua_pushinteger(state, node->get<picojson::object>().size());
  }
  else if (node->is<picojson::array>())
  {
    lua_pushinteger(state, node->get<picojson::array>().size());
  }
  else
  {
    return luaL_error(state, "expected object or array");
  }
  return 1;
}

void loadLuaROTable(lua_State* state)
{
  static const luaL_Reg metatable[] =
  {
    {"__index", readOnlyTableGet},
    {"__newindex", readOnlyTableSet},
    {"__len", readOnlyTableLength},
    {NULL, NULL}
  };

  luaL_newmetatable(state, ROTableMetaTableName);
  luaL_setfuncs(state, metatable, 0);
  lua_pushstring(state, "__metatable");
  lua_pushstring(state, "this metatable is protected");
  lua_settable(state, -3);
  lua_pop(state, 1);
}

const char* protectedTableMetatebleName = "ProtectedTable";
const char* protectedTableKey = "readonly_table";
void* protectedTableKeyRegistryKey = nullptr;

static int protectedTableSet(lua_State* state)
{
  if (lua_gettop(state) != 3)
  {
    return luaL_error(state, "wrong number of arguments");
  }
  if (std::strcmp(luaL_checklstring(state, 2, nullptr), protectedTableKey) == 0)
  {
    return luaL_error(state, "this key is readonly");
  }
  lua_rawset(state, 1);
  return 0;
}

static int protectedTableGet(lua_State* state)
{
  if (std::strcmp(luaL_checklstring(state, 2, nullptr), protectedTableKey) == 0)
  {
    lua_pushlightuserdata(state, protectedTableKeyRegistryKey);
    lua_gettable(state, LUA_REGISTRYINDEX);
    return 1;
  }
  else
  {
    return lua_rawget(state, 1);
  }
}

void loadLuaProtectedTable(lua_State* state)
{
  static const luaL_Reg metatable[] =
  {
    {"__index", protectedTableGet},
    {"__newindex", protectedTableSet},
    {NULL, NULL}
  };
  luaL_newmetatable(state, protectedTableMetatebleName);
  luaL_setfuncs(state, metatable, 0);
  lua_pushstring(state, "__metatable");
  lua_pushstring(state, "this metatable is protected");
  lua_settable(state, -3);
  lua_pop(state, 1);
}

int main()
{
  picojson::value json;
  auto err = picojson::parse(json, jsonString);
  if (!err.empty())
  {
    std::cerr << "failed to load json" << std::endl;
    return 1;
  }

  std::unique_ptr<lua_State, std::decay_t<decltype(lua_close)>> state(luaL_newstate(), lua_close);
  lua_State* L = state.get();
  luaL_openlibs(L);
  loadLuaROTable(L);
  loadLuaProtectedTable(L);
  createROTable(L, json, "readonly_table");

  int ret = luaL_loadfile(L, "sample.lua");
  if (ret)
  {
    std::printf("error: %s\n", lua_tostring(L, -1));
    return 1;
  }

  // we can proctect `readonly_table.foo = 1` but
  // we cannot protect `readonly_table = 1` without overriding metatable of global table
  lua_pushlightuserdata(L, &json);
  lua_getglobal(L, protectedTableKey);
  lua_settable(L, LUA_REGISTRYINDEX);
  protectedTableKeyRegistryKey = &json;

  lua_newtable(L);
  luaL_setmetatable(L, protectedTableMetatebleName);
  lua_pushstring(L, "print");
  lua_getglobal(L, "print");
  lua_rawset(L, -3);
//  lua_settable(L, -3);
  lua_pushstring(L, "getmetatable");
  lua_getglobal(L, "getmetatable");
  lua_rawset(L, -3);
  lua_setupvalue(L, -2, 1); 
  ret = lua_pcall(L, 0, 1, 0);
  if (ret)
  {
    std::printf("error: %s\n", lua_tostring(L, -1));
    return 1;
  }

  return 0;
}

