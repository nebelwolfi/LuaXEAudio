#include "lua.hpp"
#include <string>
namespace LuaBinding {
    using string_type = std::string;
}
#include "LuaBinding.h"
#include "SoLoudLuaBind.h"

int luaopen_audio(lua_State* L) {
    return luaopen_AudioLib(L);
}
