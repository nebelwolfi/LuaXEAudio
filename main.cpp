#include "lua.hpp"
#include <string>
#include <shared/bind.h>
#include "SoLoudLuaBind.h"

int luaopen_audio(lua_State* L) {
    return luaopen_AudioLib(L);
}
