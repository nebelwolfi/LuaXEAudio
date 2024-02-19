#pragma once

#include "soloud.h"
#include "soloud_speech.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"
#include "soloud_sfxr.h"
#include "soloud_openmpt.h"
#include "soloud_c.h"
#include "soloud_vizsn.h"

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

void push_source_funcs(auto c) {
    c.prop("flags", &SoLoud::AudioSource::mFlags)
    .prop("baseSamplerate", &SoLoud::AudioSource::mBaseSamplerate)
    .prop("volume", &SoLoud::AudioSource::mVolume)
    .prop("channels", &SoLoud::AudioSource::mChannels)
    .prop("audioSourceID", &SoLoud::AudioSource::mAudioSourceID)
    .prop("minDistance3d", &SoLoud::AudioSource::m3dMinDistance)
    .prop("maxDistance3d", &SoLoud::AudioSource::m3dMaxDistance)
    .prop("attenuationRolloff3d", &SoLoud::AudioSource::m3dAttenuationRolloff)
    .prop("attenuationModel3d", &SoLoud::AudioSource::m3dAttenuationModel)
    .prop("dopplerFactor3d", &SoLoud::AudioSource::m3dDopplerFactor)
    .prop("collider", &SoLoud::AudioSource::mCollider)
    .prop("attenuator", &SoLoud::AudioSource::mAttenuator)
    .prop("colliderData", &SoLoud::AudioSource::mColliderData)
    .prop("loopPoint", &SoLoud::AudioSource::mLoopPoint)
    .fun("set_inaudible_behavior", &SoLoud::AudioSource::setInaudibleBehavior)
    .fun("set_filter", &SoLoud::AudioSource::setFilter);
    c.fun("play", [](decltype(c)::value_type* self) {
        return AudioVoiceHandle(soloud->play(*self));
    });
    c.fun("play_clocked", [](decltype(c)::value_type* self, double aSoundTime) {
        return AudioVoiceHandle(soloud->playClocked(aSoundTime, *self));
    });
    c.fun("play3d", [](decltype(c)::value_type* self, float x, float y, float z) {
        return AudioVoiceHandle(soloud->play3d(*self, x, y, z));
    });
    c.fun("play3d_clocked", [](decltype(c)::value_type* self, double aSoundTime, float x, float y, float z) {
        return AudioVoiceHandle(soloud->play3dClocked(aSoundTime, *self, x, y, z));
    });
    c.fun("play_background", [](decltype(c)::value_type* self) {
        return AudioVoiceHandle(soloud->playBackground(*self));
    });
    c.fun("stop", [](decltype(c)::value_type* self) {
        soloud->stopAudioSource(*self);
    });
    c.fun("count", [](decltype(c)::value_type* self) {
        return soloud->countAudioSource(*self);
    });
}

int luaopen_AudioLib(lua_State *L) {
    if (!soloud) {
        soloud = new SoLoud::Soloud();
        soloud->init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::AUTO, SoLoud::Soloud::AUTO, SoLoud::Soloud::AUTO, MAX_CHANNELS);
    }

    auto S = LuaBinding::State(L);

    lua_newuserdata(L, 0);
    lua_newtable(L);
    lua_pushcclosure(L, lua_Audio_Shutdown, 0);
    lua_setfield(L, -2, "__gc");
    lua_setmetatable(L, -2);
    luaL_ref(L, LUA_REGISTRYINDEX);

    lua_newtable(L);
    lua_pushcfunction(L, +[](lua_State *L) {
        LuaBinding::State S(L);
        lua_newtable(L);
        auto sound = S.alloc<SoLoud::Wav>();
        lua_setfield(L, -2, "source");
        sound->load((AudioBasePath + lua_tostring(L, 1)).c_str());
        auto n = lua_gettop(L);
        if (n > 3)
            sound->setLooping(lua_toboolean(L, 3));
        if (n > 2)
            sound->setVolume(lua_tonumber(L, 2));
        S.push(AudioVoiceHandle(soloud->play(*sound)));
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
        LuaBinding::State S(L);
        S.alloc<SoLoud::Wav>()->load((AudioBasePath + lua_tostring(L, 1)).c_str());
        return 1;
    });
    lua_setfield(L, -2, "make_sound");
    lua_pushcfunction(L, +[](lua_State *L) {
        LuaBinding::State S(L);
        S.alloc<SoLoud::WavStream>()->load((AudioBasePath + lua_tostring(L, 1)).c_str());
        return 1;
    });
    lua_setfield(L, -2, "make_stream");
    lua_pushcfunction(L, +[](lua_State *L) {
        LuaBinding::State S(L);
        S.alloc<SoLoud::Speech>();
        return 1;
    });
    lua_setfield(L, -2, "make_speech");
    lua_pushcfunction(L, +[](lua_State *L) {
        LuaBinding::State S(L);
        S.alloc<SoLoud::Vizsn>();
        return 1;
    });
    lua_setfield(L, -2, "make_vizsn");
    lua_pushcfunction(L, +[](lua_State *L) {
        LuaBinding::State S(L);
        S.alloc<SoLoud::Sfxr>();
        return 1;
    });
    lua_setfield(L, -2, "make_sfxr");
    lua_pushcfunction(L, +[](lua_State *L) {
        LuaBinding::State S(L);
        S.alloc<SoLoud::Bus>();
        return 1;
    });
    lua_setfield(L, -2, "make_bus");
    lua_pushcfunction(L, +[](lua_State *L) {
        LuaBinding::State S(L);
        S.push(AudioVoiceHandle(soloud->createVoiceGroup()));
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

    push_source_funcs(S.addClass<SoLoud::Wav>("Sound")
            .prop("sampleCount", &SoLoud::Wav::mSampleCount)
            .prop_fun("length", &SoLoud::Wav::getLength)
            .fun("load", &SoLoud::Wav::load));

    push_source_funcs(S.addClass<SoLoud::WavStream>("SoundStream")
            .prop_fun("length", &SoLoud::WavStream::getLength)
            .prop("sampleCount", &SoLoud::WavStream::mSampleCount)
            .prop("filename", &SoLoud::WavStream::mFilename));

    push_source_funcs(S.addClass<SoLoud::Speech>("Speech")
            .prop("baseFrequency", &SoLoud::Speech::mBaseFrequency)
            .prop("baseSpeed", &SoLoud::Speech::mBaseSpeed)
            .prop("baseDeclination", &SoLoud::Speech::mBaseDeclination)
            .prop("baseWaveform", &SoLoud::Speech::mBaseWaveform)
            .prop("frames", &SoLoud::Speech::mFrames)
            .fun("set_text", &SoLoud::Speech::setText)
            .fun("set_params", &SoLoud::Speech::setParams));

    push_source_funcs(S.addClass<SoLoud::Vizsn>("Speech")
            .fun("set_text", &SoLoud::Vizsn::setText));

    push_source_funcs(S.addClass<SoLoud::Sfxr>("Sfxr")
            .cfun("load_preset", [](SoLoud::Sfxr* self, lua_State *L) {
                if (lua_gettop(L) < 2)
                    self->loadPreset(lua_tointeger(L, 2), GetTickCount64());
                else
                    self->loadPreset(lua_tointeger(L, 2), lua_tointeger(L, 3));
                return 0;
            })
            .fun("load_params", &SoLoud::Sfxr::loadParams));

    push_source_funcs(S.addClass<SoLoud::Bus>("Bus")
            .fun("play", [](SoLoud::Bus* self, SoLoud::AudioSource* source) {
                return self->play(*source);
            })
            .fun("play_clocked", [](SoLoud::Bus* self, double aSoundTime, SoLoud::AudioSource* source) {
                return self->playClocked(aSoundTime, *source);
            })
            .fun("play3d", [](SoLoud::Bus* self, SoLoud::AudioSource* source, float x, float y, float z) {
                return self->play3d(*source, x, y, z);
            })
            .fun("play3d_clocked", [](SoLoud::Bus* self, double aSoundTime, SoLoud::AudioSource* source, float x, float y, float z) {
                return self->play3dClocked(aSoundTime, *source, x, y, z);
            })
            .fun("set_channels" , &SoLoud::Bus::setChannels)
            .fun("set_visualization_enable", &SoLoud::Bus::setVisualizationEnable)
            .fun("calc_fft", &SoLoud::Bus::calcFFT)
            .fun("get_wave", &SoLoud::Bus::getWave)
            .fun("get_approximate_volume", &SoLoud::Bus::getApproximateVolume)
            .fun("get_active_voice_count", &SoLoud::Bus::getActiveVoiceCount));

    S.addClass<AudioVoiceHandle>("VoiceHandle")
            .fun("stop", [](AudioVoiceHandle* self) {
                soloud->stop(self->handle);
            })
            .fun("seek", [](AudioVoiceHandle* self, double aSeconds) {
                soloud->seek(self->handle, aSeconds);
            })
            .fun("set_filter_parameter", [](AudioVoiceHandle* vh, unsigned int filterId, unsigned int attributeId, float value) {
                return soloud->setFilterParameter(vh->handle, filterId, attributeId, value);
            })
            .fun("get_filter_parameter", [](AudioVoiceHandle* vh, unsigned int filterId, unsigned int attributeId) {
                return soloud->getFilterParameter(vh->handle, filterId, attributeId);
            })
            .fun("fade_filter_parameter", [](AudioVoiceHandle* vh, unsigned int filterId, unsigned int attributeId, float to, double aTime) {
                return soloud->fadeFilterParameter(vh->handle, filterId, attributeId, to, aTime);
            })
            .fun("oscillate_filter_parameter", [](AudioVoiceHandle* vh, unsigned int filterId, unsigned int attributeId, float from, float to, double aTime) {
                return soloud->oscillateFilterParameter(vh->handle, filterId, attributeId, from, to, aTime);
            })
            .prop_fun("stream_time", [](AudioVoiceHandle* vh) {
                return soloud->getStreamTime(vh->handle);
            })
            .prop_fun("pause", [](AudioVoiceHandle* vh) {
                return soloud->getPause(vh->handle);
            }, [](AudioVoiceHandle* vh, bool aPause) {
                soloud->setPause(vh->handle, aPause);
            })
            .prop_fun("volume", [](AudioVoiceHandle* vh) {
                return soloud->getVolume(vh->handle);
            }, [](AudioVoiceHandle* vh, float aVolume) {
                soloud->setVolume(vh->handle, aVolume);
            })
            .prop_fun("overall_volume", [](AudioVoiceHandle* vh) {
                return soloud->getOverallVolume(vh->handle);
            })
            .prop_fun("pan", [](AudioVoiceHandle* vh) {
                return soloud->getPan(vh->handle);
            }, [](AudioVoiceHandle* vh, float aPan) {
                soloud->setPan(vh->handle, aPan);
            })
            .prop_fun("samplerate", [](AudioVoiceHandle* vh) {
                return soloud->getSamplerate(vh->handle);
            }, [](AudioVoiceHandle* vh, float aSamplerate) {
                soloud->setSamplerate(vh->handle, aSamplerate);
            })
            .prop_fun("protect_voice", [](AudioVoiceHandle* vh) {
                return soloud->getProtectVoice(vh->handle);
            }, [](AudioVoiceHandle* vh, bool aProtect) {
                soloud->setProtectVoice(vh->handle, aProtect);
            })
            .prop_fun("relative_play_speed", [](AudioVoiceHandle* vh) {
                return soloud->getRelativePlaySpeed(vh->handle);
            }, [](AudioVoiceHandle* vh, float aSpeed) {
                soloud->setRelativePlaySpeed(vh->handle, aSpeed);
            })
            .prop_fun("valid", [](AudioVoiceHandle* vh) {
                return soloud->isValidVoiceHandle(vh->handle);
            })
            .prop_fun("looping", [](AudioVoiceHandle* vh) {
                return soloud->getLooping(vh->handle);
            }, [](AudioVoiceHandle* vh, bool aLooping) {
                soloud->setLooping(vh->handle, aLooping);
            })
            .prop_fun("loop_point", [](AudioVoiceHandle* vh) {
                return soloud->getLoopPoint(vh->handle);
            }, [](AudioVoiceHandle* vh, double aLoopPoint) {
                return soloud->setLoopPoint(vh->handle, aLoopPoint);
            })
            .fun("set_inaudible_behavior", [](AudioVoiceHandle* vh, bool aMustTick, bool aKill) {
                return soloud->setInaudibleBehavior(vh->handle, aMustTick, aKill);
            })
            .fun("set3d_source_parameters", [](AudioVoiceHandle* vh, float aPosX, float aPosY, float aPosZ) {
                return soloud->set3dSourceParameters(vh->handle, aPosX, aPosY, aPosZ);
            })
            .fun("set3d_source_position", [](AudioVoiceHandle* vh, float aPosX, float aPosY, float aPosZ) {
                return soloud->set3dSourcePosition(vh->handle, aPosX, aPosY, aPosZ);
            })
            .fun("set3d_source_velocity", [](AudioVoiceHandle* vh, float aVelocityX, float aVelocityY, float aVelocityZ) {
                return soloud->set3dSourceVelocity(vh->handle, aVelocityX, aVelocityY, aVelocityZ);
            })
            .fun("set3d_source_min_max_distance", [](AudioVoiceHandle* vh, float aMinDistance, float aMaxDistance) {
                return soloud->set3dSourceMinMaxDistance(vh->handle, aMinDistance, aMaxDistance);
            })
            .fun("set3d_source_attenuation", [](AudioVoiceHandle* vh, unsigned int model, float rolloffFactor) {
                return soloud->set3dSourceAttenuation(vh->handle, model, rolloffFactor);
            })
            .fun("set3d_source_doppler_factor", [](AudioVoiceHandle* vh, float aDopplerFactor) {
                return soloud->set3dSourceDopplerFactor(vh->handle, aDopplerFactor);
            })
            .fun("set_relative_play_speed", [](AudioVoiceHandle* vh, float aSpeed) {
                return soloud->setRelativePlaySpeed(vh->handle, aSpeed);
            })
            .fun("set_protect_voice", [](AudioVoiceHandle* vh, bool aProtect) {
                return soloud->setProtectVoice(vh->handle, aProtect);
            })
            .fun("set_pan_absolute", [](AudioVoiceHandle* vh, float aLVolume, float aRVolume, float aLBVolume, float aRBVolume, float aCVolume, float aSVolume) {
                return soloud->setPanAbsolute(vh->handle, aLVolume, aRVolume, aLBVolume, aRBVolume, aCVolume, aSVolume);
            })
            .fun("set_delay_samples", [](AudioVoiceHandle* vh, unsigned int aSamples) {
                return soloud->setDelaySamples(vh->handle, aSamples);
            })
            .fun("fade_volume", [](AudioVoiceHandle* vh, float aTo, double aTime) {
                return soloud->fadeVolume(vh->handle, aTo, aTime);
            })
            .fun("fade_pan", [](AudioVoiceHandle* vh, float aTo, double aTime) {
                return soloud->fadePan(vh->handle, aTo, aTime);
            })
            .fun("fade_relative_play_speed", [](AudioVoiceHandle* vh, float aTo, double aTime) {
                return soloud->fadeRelativePlaySpeed(vh->handle, aTo, aTime);
            })
            .fun("fade_global_volume", [](float aTo, double aTime) {
                return soloud->fadeGlobalVolume(aTo, aTime);
            })
            .fun("schedule_pause", [](AudioVoiceHandle* vh, double aTime) {
                return soloud->schedulePause(vh->handle, aTime);
            })
            .fun("schedule_stop", [](AudioVoiceHandle* vh, double aTime) {
                return soloud->scheduleStop(vh->handle, aTime);
            })
            .fun("oscillate_volume", [](AudioVoiceHandle* vh, float aFrom, float aTo, double aTime) {
                return soloud->oscillateVolume(vh->handle, aFrom, aTo, aTime);
            })
            .fun("oscillate_pan", [](AudioVoiceHandle* vh, float aFrom, float aTo, double aTime) {
                return soloud->oscillatePan(vh->handle, aFrom, aTo, aTime);
            })
            .fun("oscillate_relative_play_speed", [](AudioVoiceHandle* vh, float aFrom, float aTo, double aTime) {
                return soloud->oscillateRelativePlaySpeed(vh->handle, aFrom, aTo, aTime);
            })
            .prop_fun("loop_count", [](AudioVoiceHandle* vh) {
                return soloud->getLoopCount(vh->handle);
            });

    return 1;
}