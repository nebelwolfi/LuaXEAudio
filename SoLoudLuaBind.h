#pragma once

#include "soloud.h"
#include "soloud_speech.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"
#include "soloud_sfxr.h"
#include "soloud_openmpt.h"
#include "soloud_c.h"
#include "soloud_vizsn.h"
#include <shared/env.h>

struct AudioVoiceHandle {
    SoLoud::handle handle;
};

SoLoud::Soloud* soloud;
std::string AudioBasePath;
int lua_Audio_Shutdown(lua_State *L) {
    if (soloud) {
        soloud->deinit();
        delete soloud;
        soloud = nullptr;
    }
    return 0;
}

#define src_prop_fun(name) \
    [](lua_State *L) -> int { \
        auto self = *(SoLoud::AudioSource**) lua_touserdata(L, 1); \
        lua_pushnumber(L, self->name); \
        return 1; \
    }, [](lua_State *L) -> int { \
        auto self = *(SoLoud::AudioSource**) lua_touserdata(L, 1); \
        self->name = lua_tonumber(L, 3); \
        return 0; \
    }

void push_source_funcs(auto c) {
    c.prop("flags", src_prop_fun(mFlags))
    .prop("baseSamplerate", src_prop_fun(mBaseSamplerate))
    .prop("volume", src_prop_fun(mVolume))
    .prop("channels", src_prop_fun(mChannels))
    .prop("audioSourceID", src_prop_fun(mAudioSourceID))
    .prop("minDistance3d", src_prop_fun(m3dMinDistance))
    .prop("maxDistance3d", src_prop_fun(m3dMaxDistance))
    .prop("attenuationRolloff3d", src_prop_fun(m3dAttenuationRolloff))
    .prop("attenuationModel3d", src_prop_fun(m3dAttenuationModel))
    .prop("dopplerFactor3d", src_prop_fun(m3dDopplerFactor))
    .prop("colliderData", src_prop_fun(mColliderData))
    .prop("loopPoint", src_prop_fun(mLoopPoint))
    //.fun("set_inaudible_behavior", src_prop_fun(setInaudibleBehavior))
    //.fun("set_filter", src_prop_fun(setFilter))
    ;
    c.fun("play", [](lua_State *L) -> int {
        auto self = *(SoLoud::AudioSource**) lua_touserdata(L, 1);
        new (lua::alloc<AudioVoiceHandle>(L)) AudioVoiceHandle(soloud->play(*self));
        return 1;
    });
    c.fun("play_clocked", [](lua_State *L) -> int {
        auto self = *(SoLoud::AudioSource**) lua_touserdata(L, 1);
        new (lua::alloc<AudioVoiceHandle>(L)) AudioVoiceHandle(soloud->playClocked(lua_tonumber(L, 2), *self));
        return 1;
    });
    c.fun("play3d", [](lua_State *L) -> int {
        auto self = *(SoLoud::AudioSource**) lua_touserdata(L, 1);
        new (lua::alloc<AudioVoiceHandle>(L)) AudioVoiceHandle(soloud->play3d(*self, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4)));
        return 1;
    });
    c.fun("play3d_clocked", [](lua_State *L) -> int {
        auto self = *(SoLoud::AudioSource**) lua_touserdata(L, 1);
        new (lua::alloc<AudioVoiceHandle>(L)) AudioVoiceHandle(soloud->play3dClocked(lua_tonumber(L, 2), *self, lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5)));
        return 1;
    });
    c.fun("play_background", [](lua_State *L) -> int {
        auto self = *(SoLoud::AudioSource**) lua_touserdata(L, 1);
        new (lua::alloc<AudioVoiceHandle>(L)) AudioVoiceHandle(soloud->playBackground(*self));
        return 1;
    });
    c.fun("stop", [](lua_State *L) -> int {
        auto self = *(SoLoud::AudioSource**) lua_touserdata(L, 1);
        soloud->stopAudioSource(*self);
        return 0;
    });
    c.fun("count", [](lua_State *L) -> int {
        auto self = *(SoLoud::AudioSource**) lua_touserdata(L, 1);
        lua_pushinteger(L, soloud->countAudioSource(*self));
        return 1;
    });
}

#define prop_fun(type, name, push, to) \
    [](lua_State *L) -> int { \
        auto self = *(type**) lua_touserdata(L, 1); \
        push(L, self->name); \
        return 1; \
    }, [](lua_State *L) -> int { \
        auto self = *(type**) lua_touserdata(L, 1); \
        self->name = to(L, 3); \
        return 0; \
    }

#define prop_fun2(type, name, push) \
    [](lua_State *L) -> int { \
        auto self = *(type**) lua_touserdata(L, 1); \
        push(L, self->name); \
        return 1; \
    }

int luaopen_AudioLib(lua_State *L) {
    if (!soloud) {
        soloud = new SoLoud::Soloud();
        soloud->init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::AUTO, SoLoud::Soloud::AUTO, SoLoud::Soloud::AUTO, MAX_CHANNELS);
    }
    lua::env::init(L);
    lua::env::on_close(lua_Audio_Shutdown);

    lua_newtable(L);
    lua_pushcfunction(L, +[](lua_State *L) {
        lua_newtable(L);
        auto sound = new (lua::alloc<SoLoud::Wav>(L)) SoLoud::Wav();
        lua_setfield(L, -2, "source");
        sound->load((AudioBasePath + luaL_checkstring(L, 1)).c_str());
        auto n = lua_gettop(L);
        if (n > 3)
            sound->setLooping(lua_toboolean(L, 3));
        if (n > 2)
            sound->setVolume(lua_tonumber(L, 2));
        new (lua::alloc<AudioVoiceHandle>(L)) AudioVoiceHandle(soloud->play(*sound));
        lua_setfield(L, -2, "handle");
        return 1;
    });
    lua_setfield(L, -2, "play");
    lua_pushcclosure(L, +[](lua_State *L) {
        soloud->stopAll();
        return 0;
    }, 0);
    lua_setfield(L, -2, "stop");
    lua_pushcclosure(L, +[](lua_State *L) {
        soloud->setPostClipScaler(lua_tonumber(L, 1));
        return 0;
    }, 0);
    lua_setfield(L, -2, "set_post_clip_scaler");
    lua_pushcclosure(L, +[](lua_State *L) {
        soloud->setPauseAll(lua_toboolean(L, 1));
        return 0;
    }, 0);
    lua_setfield(L, -2, "set_pause_all");
    lua_pushcclosure(L, +[](lua_State *L) {
        soloud->setVisualizationEnable(lua_toboolean(L, 1));
        return 0;
    }, 0);
    lua_setfield(L, -2, "set_visualization_enable");
    lua_pushcclosure(L, +[](lua_State *L) {
        soloud->fadeGlobalVolume(lua_tonumber(L, 1), lua_tonumber(L, 2));
        return 0;
    }, 0);
    lua_setfield(L, -2, "fade_global_volume");
    lua_pushcclosure(L, +[](lua_State *L) {
        soloud->setGlobalFilter(lua_tointeger(L, 1), (SoLoud::Filter*)lua_touserdata(L, 2));
        return 0;
    }, 0);
    lua_setfield(L, -2, "set_global_filter");
    lua_pushcclosure(L, +[](lua_State *L) {
        soloud->update3dAudio();
        return 0;
    }, 0);
    lua_setfield(L, -2, "update3d_audio");
    lua_pushcclosure(L, +[](lua_State *L) {
        soloud->set3dListenerParameters(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8), lua_tonumber(L, 9));
        return 0;
    }, 0);
    lua_setfield(L, -2, "set3d_listener_parameters");
    lua_pushcclosure(L, +[](lua_State *L) {
        soloud->set3dListenerPosition(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3));
        return 0;
    }, 0);
    lua_setfield(L, -2, "set3d_listener_position");
    lua_pushcclosure(L, +[](lua_State *L) {
        soloud->set3dListenerAt(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3));
        return 0;
    }, 0);
    lua_setfield(L, -2, "set3d_listener_target");
    lua_pushcclosure(L, +[](lua_State *L) {
        soloud->set3dListenerUp(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3));
        return 0;
    }, 0);
    lua_setfield(L, -2, "set3d_listener_up");
    lua_pushcclosure(L, +[](lua_State *L) {
        soloud->set3dListenerVelocity(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3));
        return 0;
    }, 0);
    lua_setfield(L, -2, "set3d_listener_velocity");
    lua_pushcfunction(L, +[](lua_State *L) {
        (new (lua::alloc<SoLoud::Wav>(L)) SoLoud::Wav())->load((AudioBasePath + lua_tostring(L, 1)).c_str());
        return 1;
    });
    lua_setfield(L, -2, "make_sound");
    lua_pushcfunction(L, +[](lua_State *L) {
        (new (lua::alloc<SoLoud::Wav>(L)) SoLoud::Wav())->load((AudioBasePath + lua_tostring(L, 1)).c_str());
        return 1;
    });
    lua_setfield(L, -2, "make_stream");
    lua_pushcfunction(L, +[](lua_State *L) {
        //S.alloc<SoLoud::Speech>();
        new (lua::alloc<SoLoud::Speech>(L)) SoLoud::Speech();
        return 1;
    });
    lua_setfield(L, -2, "make_speech");
    lua_pushcfunction(L, +[](lua_State *L) {
        //S.alloc<SoLoud::Vizsn>();
        new (lua::alloc<SoLoud::Vizsn>(L)) SoLoud::Vizsn();
        return 1;
    });
    lua_setfield(L, -2, "make_vizsn");
    lua_pushcfunction(L, +[](lua_State *L) {
        //S.alloc<SoLoud::Sfxr>();
        new (lua::alloc<SoLoud::Sfxr>(L)) SoLoud::Sfxr();
        return 1;
    });
    lua_setfield(L, -2, "make_sfxr");
    lua_pushcfunction(L, +[](lua_State *L) {
        //S.alloc<SoLoud::Bus>();
        new (lua::alloc<SoLoud::Bus>(L)) SoLoud::Bus();
        return 1;
    });
    lua_setfield(L, -2, "make_bus");
    lua_pushcfunction(L, +[](lua_State *L) {
        //S.push(AudioVoiceHandle(soloud->createVoiceGroup()));
        new (lua::alloc<AudioVoiceHandle>(L)) AudioVoiceHandle(soloud->createVoiceGroup());
        return 1;
    });
    lua_setfield(L, -2, "make_voice_group");
    lua_newtable(L);
    lua_pushinteger(L, SoLoud::AudioSource::FLAGS::SHOULD_LOOP); lua_setfield(L, -2, "SHOULD_LOOP");
    lua_pushinteger(L, SoLoud::AudioSource::FLAGS::SINGLE_INSTANCE); lua_setfield(L, -2, "SINGLE_INSTANCE");
    lua_pushinteger(L, SoLoud::AudioSource::FLAGS::VISUALIZATION_DATA); lua_setfield(L, -2, "VISUALIZATION_DATA");
    lua_pushinteger(L, SoLoud::AudioSource::FLAGS::PROCESS_3D); lua_setfield(L, -2, "PROCESS_3D");
    lua_pushinteger(L, SoLoud::AudioSource::FLAGS::LISTENER_RELATIVE); lua_setfield(L, -2, "LISTENER_RELATIVE");
    lua_pushinteger(L, SoLoud::AudioSource::FLAGS::DISTANCE_DELAY); lua_setfield(L, -2, "DISTANCE_DELAY");
    lua_pushinteger(L, SoLoud::AudioSource::FLAGS::INAUDIBLE_KILL); lua_setfield(L, -2, "INAUDIBLE_KILL");
    lua_pushinteger(L, SoLoud::AudioSource::FLAGS::INAUDIBLE_TICK); lua_setfield(L, -2, "INAUDIBLE_TICK");
    lua_setfield(L, -2, "flags");
    lua_newtable(L);
    lua_pushinteger(L, 0); lua_setfield(L, -2, "COIN");
    lua_pushinteger(L, 1); lua_setfield(L, -2, "LASER");
    lua_pushinteger(L, 2); lua_setfield(L, -2, "EXPLOSION");
    lua_pushinteger(L, 3); lua_setfield(L, -2, "POWERUP");
    lua_pushinteger(L, 4); lua_setfield(L, -2, "HURT");
    lua_pushinteger(L, 5); lua_setfield(L, -2, "JUMP");
    lua_pushinteger(L, 6); lua_setfield(L, -2, "BLIP");
    lua_setfield(L, -2, "sfxr");
    lua_newtable(L);
    lua_pushcfunction(L, +[](lua_State *L) {
        auto idx = lua_tostring(L, 2);
        if (strcmp(idx, "path") == 0) {
            lua_pushstring(L, AudioBasePath.c_str());
            return 1;
        }
        if (strcmp(idx, "voiceCount") == 0) {
            lua_pushinteger(L, soloud->getVoiceCount());
            return 1;
        }
        if (strcmp(idx, "activeVoiceCount") == 0) {
            lua_pushinteger(L, soloud->getActiveVoiceCount());
            return 1;
        }
        if (strcmp(idx, "globalVolume") == 0) {
            lua_pushnumber(L, soloud->getGlobalVolume());
            return 1;
        }
        if (strcmp(idx, "soundSpeed") == 0) {
            lua_pushnumber(L, soloud->get3dSoundSpeed());
            return 1;
        }
        if (strcmp(idx, "maxActiveVoiceCount") == 0) {
            lua_pushinteger(L, soloud->getMaxActiveVoiceCount());
            return 1;
        }
        return 0;
    });
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, +[](lua_State *L) {
        auto idx = lua_tostring(L, 2);
        if (strcmp(idx, "path") == 0) {
            AudioBasePath = luaL_checkstring(L, 3);
            return 0;
        }
        if (strcmp(idx, "maxActiveVoiceCount") == 0) {
            soloud->setMaxActiveVoiceCount(luaL_checkinteger(L, 3));
            return 0;
        }
        if (strcmp(idx, "globalVolume") == 0) {
            soloud->setGlobalVolume(luaL_checknumber(L, 3));
            return 0;
        }
        if (strcmp(idx, "soundSpeed") == 0) {
            soloud->set3dSoundSpeed(luaL_checknumber(L, 3));
            return 0;
        }
        return 0;
    });
    lua_setfield(L, -2, "__newindex");
    lua_setmetatable(L, -2);

    push_source_funcs(lua::bind::add<SoLoud::Wav>(L, "Sound")
            .prop("sampleCount", prop_fun(SoLoud::Wav, mSampleCount, lua_pushinteger, lua_tointeger))
            .prop("length", [](lua_State *L) -> int {
                auto self = *(SoLoud::Wav**) lua_touserdata(L, 1);
                lua_pushnumber(L, self->getLength());
                return 1;
            })
            .fun("load", [](lua_State *L) -> int {
                auto self = *(SoLoud::Wav**) lua_touserdata(L, 1);
                self->load(luaL_checkstring(L, 2));
                return 0;
            })
            );

    push_source_funcs(lua::bind::add<SoLoud::WavStream>(L, "SoundStream")
            .prop("length", [](lua_State *L) -> int {
                auto self = *(SoLoud::WavStream**) lua_touserdata(L, 1);
                lua_pushnumber(L, self->getLength());
                return 1;
            })
            .prop("sampleCount", prop_fun(SoLoud::WavStream, mSampleCount, lua_pushinteger, lua_tointeger))
            .prop("filename", prop_fun2(SoLoud::WavStream, mFilename, lua_pushstring))
        );

    push_source_funcs(lua::bind::add<SoLoud::Speech>(L, "Speech")
            .prop("baseFrequency", prop_fun(SoLoud::Speech, mBaseFrequency, lua_pushnumber, lua_tonumber))
            .prop("baseSpeed", prop_fun(SoLoud::Speech, mBaseSpeed, lua_pushnumber, lua_tonumber))
            .prop("baseDeclination", prop_fun(SoLoud::Speech, mBaseDeclination, lua_pushnumber, lua_tonumber))
            .prop("baseWaveform", prop_fun(SoLoud::Speech, mBaseWaveform, lua_pushnumber, lua_tonumber))
            .prop("frames", prop_fun(SoLoud::Speech, mFrames, lua_pushnumber, lua_tonumber))
            .fun("set_text", [](lua_State *L) -> int {
                auto self = *(SoLoud::Speech**) lua_touserdata(L, 1);
                self->setText(luaL_checkstring(L, 2));
                return 0;
            })
            .fun("set_params", [](lua_State *L) -> int {
                auto self = *(SoLoud::Speech**) lua_touserdata(L, 1);
                self->setParams(luaL_checkinteger(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checkinteger(L, 5));
                return 0;
            })
        );

    push_source_funcs(lua::bind::add<SoLoud::Vizsn>(L, "Speech")
            .fun("set_text", [](lua_State *L) -> int {
                auto self = *(SoLoud::Vizsn**) lua_touserdata(L, 1);
                self->setText((char*)luaL_checkstring(L, 2));
                return 0;
            })
            );

    push_source_funcs(lua::bind::add<SoLoud::Sfxr>(L, "Sfxr")
            .fun("load_preset", [](lua_State *L) {
                auto self = *(SoLoud::Sfxr**) lua_touserdata(L, 1);
                if (lua_gettop(L) < 2)
                    self->loadPreset(lua_tointeger(L, 2), GetTickCount64());
                else
                    self->loadPreset(lua_tointeger(L, 2), lua_tointeger(L, 3));
                return 0;
            })
            .fun("load_params", [](lua_State *L) {
                auto self = *(SoLoud::Sfxr**) lua_touserdata(L, 1);
                self->loadParams(luaL_checkstring(L, 2));
                return 0;
            })
        );

    push_source_funcs(lua::bind::add<SoLoud::Bus>(L, "Bus")
            .fun("play", [](lua_State *L) -> int {
                auto self = *(SoLoud::Bus**) lua_touserdata(L, 1);
                new (lua::alloc<AudioVoiceHandle>(L)) AudioVoiceHandle(soloud->play(*self));
                return 1;
            })
            .fun("play_clocked", [](lua_State *L) -> int {
                auto self = *(SoLoud::Bus**) lua_touserdata(L, 1);
                new (lua::alloc<AudioVoiceHandle>(L)) AudioVoiceHandle(soloud->playClocked(lua_tonumber(L, 2), *self));
                return 1;
            })
            .fun("play3d", [](lua_State *L) -> int {
                auto self = *(SoLoud::Bus**) lua_touserdata(L, 1);
                new (lua::alloc<AudioVoiceHandle>(L)) AudioVoiceHandle(soloud->play3d(*self, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4)));
                return 1;
            })
            .fun("play3d_clocked", [](lua_State *L) -> int {
                auto self = *(SoLoud::Bus**) lua_touserdata(L, 1);
                new (lua::alloc<AudioVoiceHandle>(L)) AudioVoiceHandle(soloud->play3dClocked(lua_tonumber(L, 2), *self, lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5)));
                return 1;
            })
            .fun("set_channels", [](lua_State *L) -> int {
                auto self = *(SoLoud::Bus**) lua_touserdata(L, 1);
                self->setChannels(lua_tointeger(L, 2));
                return 0;
            })
            .fun("set_visualization_enable", [](lua_State *L) -> int {
                auto self = *(SoLoud::Bus**) lua_touserdata(L, 1);
                self->setVisualizationEnable(lua_toboolean(L, 2));
                return 0;
            })
            .fun("calc_fft", [](lua_State *L) -> int {
                auto self = *(SoLoud::Bus**) lua_touserdata(L, 1);
                auto fft = self->calcFFT();
                lua_newtable(L);
                for (int i = 0; i < 256; i++) {
                    lua_pushnumber(L, fft[i]);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            })
            .fun("get_wave", [](lua_State *L) -> int {
                auto self = *(SoLoud::Bus**) lua_touserdata(L, 1);
                auto wave = self->getWave();
                lua_newtable(L);
                for (int i = 0; i < 256; i++) {
                    lua_pushnumber(L, wave[i]);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            })
            .fun("get_approximate_volume", [](lua_State *L) -> int {
                auto self = *(SoLoud::Bus**) lua_touserdata(L, 1);
                lua_pushnumber(L, self->getApproximateVolume(lua_tointeger(L, 2)));
                return 1;
            })
            .fun("get_active_voice_count", [](lua_State *L) -> int {
                auto self = *(SoLoud::Bus**) lua_touserdata(L, 1);
                lua_pushinteger(L, self->getActiveVoiceCount());
                return 1;
            })
        );

    lua::bind::add<AudioVoiceHandle>(L, "VoiceHandle")
            .fun("stop", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->stop(self->handle);
                return 0;
            })
            .fun("seek", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->seek(self->handle, lua_tonumber(L, 2));
                return 0;
            })
            .fun("set_filter_parameter", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->setFilterParameter(self->handle, lua_tointeger(L, 2), lua_tointeger(L, 3), lua_tonumber(L, 4));
                return 0;
            })
            .fun("get_filter_parameter", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                lua_pushnumber(L, soloud->getFilterParameter(self->handle, lua_tointeger(L, 2), lua_tointeger(L, 3)));
                return 1;
            })
            .fun("fade_filter_parameter", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->fadeFilterParameter(self->handle, lua_tointeger(L, 2), lua_tointeger(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));
                return 0;
            })
            .fun("oscillate_filter_parameter", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->oscillateFilterParameter(self->handle, lua_tointeger(L, 2), lua_tointeger(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6));
                return 0;
            })
            .prop("stream_time", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                lua_pushnumber(L, soloud->getStreamTime(self->handle));
                return 1;
            })
            .prop("pause", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                lua_pushboolean(L, soloud->getPause(self->handle));
                return 1;
            }, [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->setPause(self->handle, lua_toboolean(L, 3));
                return 0;
            })
            .prop("volume", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                lua_pushnumber(L, soloud->getVolume(self->handle));
                return 1;
            }, [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->setVolume(self->handle, lua_tonumber(L, 3));
                return 0;
            })
            .prop("overall_volume", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                lua_pushnumber(L, soloud->getOverallVolume(self->handle));
                return 1;
            })
            .prop("pan", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                lua_pushnumber(L, soloud->getPan(self->handle));
                return 1;
            }, [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->setPan(self->handle, lua_tonumber(L, 3));
                return 0;
            })
            .prop("samplerate", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                lua_pushnumber(L, soloud->getSamplerate(self->handle));
                return 1;
            }, [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->setSamplerate(self->handle, lua_tonumber(L, 3));
                return 0;
            })
            .prop("protect_voice", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                lua_pushboolean(L, soloud->getProtectVoice(self->handle));
                return 1;
            }, [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->setProtectVoice(self->handle, lua_toboolean(L, 3));
                return 0;
            })
            .prop("relative_play_speed", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                lua_pushnumber(L, soloud->getRelativePlaySpeed(self->handle));
                return 1;
            }, [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->setRelativePlaySpeed(self->handle, lua_tonumber(L, 3));
                return 0;
            })
            .prop("valid", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                lua_pushboolean(L, soloud->isValidVoiceHandle(self->handle));
                return 1;
            })
            .prop("looping", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                lua_pushboolean(L, soloud->getLooping(self->handle));
                return 1;
            }, [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->setLooping(self->handle, lua_toboolean(L, 3));
                return 0;
            })
            .prop("loop_point", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                lua_pushnumber(L, soloud->getLoopPoint(self->handle));
                return 1;
            }, [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->setLoopPoint(self->handle, lua_tonumber(L, 3));
                return 0;
            })
            .fun("set_inaudible_behavior", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->setInaudibleBehavior(self->handle, lua_toboolean(L, 2), lua_toboolean(L, 3));
                return 0;
            })
            .fun("set3d_source_parameters", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->set3dSourceParameters(self->handle, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
                return 0;
            })
            .fun("set3d_source_position", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->set3dSourcePosition(self->handle, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
                return 0;
            })
            .fun("set3d_source_velocity", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->set3dSourceVelocity(self->handle, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
                return 0;
            })
            .fun("set3d_source_min_max_distance", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->set3dSourceMinMaxDistance(self->handle, lua_tonumber(L, 2), lua_tonumber(L, 3));
                return 0;
            })
            .fun("set3d_source_attenuation", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->set3dSourceAttenuation(self->handle, lua_tointeger(L, 2), lua_tonumber(L, 3));
                return 0;
            })
            .fun("set3d_source_doppler_factor", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->set3dSourceDopplerFactor(self->handle, lua_tonumber(L, 2));
                return 0;
            })
            .fun("set_relative_play_speed", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->setRelativePlaySpeed(self->handle, lua_tonumber(L, 2));
                return 0;
            })
            .fun("set_protect_voice", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->setProtectVoice(self->handle, lua_toboolean(L, 2));
                return 0;
            })
            .fun("set_pan_absolute", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->setPanAbsolute(self->handle, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7));
                return 0;
            })
            .fun("set_delay_samples", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->setDelaySamples(self->handle, lua_tointeger(L, 2));
                return 0;
            })
            .fun("fade_volume", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->fadeVolume(self->handle, lua_tonumber(L, 2), lua_tonumber(L, 3));
                return 0;
            })
            .fun("fade_pan", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->fadePan(self->handle, lua_tonumber(L, 2), lua_tonumber(L, 3));
                return 0;
            })
            .fun("fade_relative_play_speed", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->fadeRelativePlaySpeed(self->handle, lua_tonumber(L, 2), lua_tonumber(L, 3));
                return 0;
            })
            .fun("fade_global_volume", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->fadeGlobalVolume(lua_tonumber(L, 2), lua_tonumber(L, 3));
                return 0;
            })
            .fun("schedule_pause", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->schedulePause(self->handle, lua_tonumber(L, 2));
                return 0;
            })
            .fun("schedule_stop", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->scheduleStop(self->handle, lua_tonumber(L, 2));
                return 0;
            })
            .fun("oscillate_volume", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->oscillateVolume(self->handle, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
                return 0;
            })
            .fun("oscillate_pan", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->oscillatePan(self->handle, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
                return 0;
            })
            .fun("oscillate_relative_play_speed", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                soloud->oscillateRelativePlaySpeed(self->handle, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
                return 0;
            })
            .prop("loop_count", [](lua_State *L) -> int {
                auto self = lua::check<AudioVoiceHandle>(L, 1);
                lua_pushinteger(L, soloud->getLoopCount(self->handle));
                return 1;
            })
        ;

    return 1;
}