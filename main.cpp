#include "lua.hpp"
#include "LuaBinding.h"
#include "SoLoudLuaBind.h"

int luaopen_audio(lua_State* L) {
    return luaopen_AudioLib(L);
}
