#pragma once

#include "soloud.h"
#include "soloud_speech.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"
#include "soloud_sfxr.h"
#include "soloud_openmpt.h"
#include "soloud_c.h"
#include "soloud_vizsn.h"

SoLoud::Soloud* soloud;
LuaBinding::ObjectRef soloud_ref = LuaBinding::NilRef;
std::string AudioBasePath;
int lua_Audio_SetBasePath(lua_State *L) {
    if (lua_gettop(L) < 1)
    {
        lua_pushstring(L, AudioBasePath.c_str());
        return 1;
    }
    AudioBasePath = luaL_checkstring(L, 1);
    return 0;
}
int lua_Audio_Shutdown(lua_State *L) {
    if (soloud) {
        soloud->deinit();
        delete soloud;
        soloud = nullptr;
    }
    soloud_ref.pop();
    return 0;
}
int lua_Audio_Play(lua_State *L) {
    LuaBinding::State S(L);
    auto sample = S.alloc<SoLoud::Wav>();
    auto res = sample->load((AudioBasePath + S.at(1).as<std::string>()).c_str());
    if (res != 0) {
        luaL_error(L, "Failed to load audio file: %s", S.at(1).as<std::string>().c_str());
        return 0;
    }
    if (S.n() > 3)
        sample->setLooping(lua_toboolean(L, 3));
    if (S.n() > 2)
        S.push(soloud->play(*sample, lua_tonumber(L, 2)));
    else
        S.push(soloud->play(*sample));
    return 2;
}
int lua_Audio_File(lua_State *L) {
    LuaBinding::State S(L);
    S.alloc<SoLoud::Wav>()->load((AudioBasePath + S.at(1).as<std::string>()).c_str());
    return 1;
}
int lua_Audio_FileStream(lua_State *L) {
    LuaBinding::State S(L);
    S.alloc<SoLoud::WavStream>()->load((AudioBasePath + S.at(1).as<std::string>()).c_str());
    return 1;
}
int lua_Audio_Speech(lua_State *L) {
    LuaBinding::State S(L);
    S.alloc<SoLoud::Speech>();
    return 1;
}
int lua_Audio_Vizsn(lua_State *L) {
    LuaBinding::State S(L);
    S.alloc<SoLoud::Vizsn>();
    return 1;
}
int lua_Audio_Sfxr(lua_State *L) {
    LuaBinding::State S(L);
    S.alloc<SoLoud::Sfxr>();
    return 1;
}
int lua_Audio_Mod(lua_State *L) {
    LuaBinding::State S(L);
    S.alloc<SoLoud::Openmpt>()->load((AudioBasePath + S.at(1).as<std::string>()).c_str());
    return 1;
}

luaL_Reg AudioLib [] = {
        { "base_path", lua_Audio_SetBasePath },
        { "shutdown", lua_Audio_Shutdown },
        { "play", lua_Audio_Play },
        { "file", lua_Audio_File },
        { "file_stream", lua_Audio_FileStream },
        { "speech", lua_Audio_Speech },
        { "vizsn", lua_Audio_Vizsn },
        { "sfxr", lua_Audio_Sfxr },
        { "mod", lua_Audio_Mod },
        { NULL, NULL }
};

int luaclose_AudioLib(lua_State *L) {
    if (soloud_ref.valid(L))
        soloud_ref.pop();
    else
        soloud_ref.invalidate();
    return 0;
}

int luaopen_AudioLib(lua_State *L) {
    if (!soloud) {
        soloud = new SoLoud::Soloud();
        soloud->init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::AUTO, SoLoud::Soloud::AUTO, SoLoud::Soloud::AUTO, 8);
    }

    auto S = LuaBinding::State(L);
    if (soloud_ref.valid())
        return soloud_ref.push();

    luaL_newlib(L, AudioLib);

    auto Audio = S[-1];
    Audio.fun("play_source", [](SoLoud::AudioSource* source) {
        return soloud->play(*source);
    });
    Audio.fun("play_clocked", [](double aSoundTime, SoLoud::AudioSource* source) {
        return soloud->playClocked(aSoundTime, *source);
    });
    Audio.fun("play3d", [](SoLoud::AudioSource* source, float x, float y, float z) {
        return soloud->play3d(*source, x, y, z);
    });
    Audio.fun("play3d_clocked", [](double aSoundTime, SoLoud::AudioSource* source, float x, float y, float z) {
        return soloud->play3dClocked(aSoundTime, *source, x, y, z);
    });
    Audio.fun("play_background", [](SoLoud::AudioSource* source) {
        return soloud->playBackground(*source);
    });
    Audio.fun("seek", [](unsigned int voiceHandle, double aSeconds) {
        return soloud->seek(voiceHandle, aSeconds);
    });
    Audio.fun("stop", [](unsigned int voiceHandle) {
        return soloud->stop(voiceHandle);
    });
    Audio.fun("stop_all", []() {
        return soloud->stopAll();
    });
    Audio.fun("stop_audio_source", [](SoLoud::AudioSource* source) {
        return soloud->stopAudioSource(*source);
    });
    Audio.fun("count_audio_source", [](SoLoud::AudioSource* source) {
        return soloud->countAudioSource(*source);
    });
    Audio.fun("set_filter_parameter", [](unsigned int voiceHandle, unsigned int filterId, unsigned int attributeId, float value) {
        return soloud->setFilterParameter(voiceHandle, filterId, attributeId, value);
    });
    Audio.fun("get_filter_parameter", [](unsigned int voiceHandle, unsigned int filterId, unsigned int attributeId) {
        return soloud->getFilterParameter(voiceHandle, filterId, attributeId);
    });
    Audio.fun("fade_filter_parameter", [](unsigned int voiceHandle, unsigned int filterId, unsigned int attributeId, float to, double aTime) {
        return soloud->fadeFilterParameter(voiceHandle, filterId, attributeId, to, aTime);
    });
    Audio.fun("oscillate_filter_parameter", [](unsigned int voiceHandle, unsigned int filterId, unsigned int attributeId, float from, float to, double aTime) {
        return soloud->oscillateFilterParameter(voiceHandle, filterId, attributeId, from, to, aTime);
    });
    Audio.fun("get_stream_time", [](unsigned int voiceHandle) {
        return soloud->getStreamTime(voiceHandle);
    });
    Audio.fun("get_pause", [](unsigned int voiceHandle) {
        return soloud->getPause(voiceHandle);
    });
    Audio.fun("get_volume", [](unsigned int voiceHandle) {
        return soloud->getVolume(voiceHandle);
    });
    Audio.fun("get_overall_volume", [](unsigned int voiceHandle) {
        return soloud->getOverallVolume(voiceHandle);
    });
    Audio.fun("get_pan", [](unsigned int voiceHandle) {
        return soloud->getPan(voiceHandle);
    });
    Audio.fun("get_samplerate", [](unsigned int voiceHandle) {
        return soloud->getSamplerate(voiceHandle);
    });
    Audio.fun("get_protect_voice", [](unsigned int voiceHandle) {
        return soloud->getProtectVoice(voiceHandle);
    });
    Audio.fun("get_active_voice_count", []() {
        return soloud->getActiveVoiceCount();
    });
    Audio.fun("get_voice_count", []() {
        return soloud->getVoiceCount();
    });
    Audio.fun("is_valid_voice_handle", [](unsigned int voiceHandle) {
        return soloud->isValidVoiceHandle(voiceHandle);
    });
    Audio.fun("get_relative_play_speed", [](unsigned int voiceHandle) {
        return soloud->getRelativePlaySpeed(voiceHandle);
    });
    Audio.fun("get_post_clip_scaler", []() {
        return soloud->getPostClipScaler();
    });
    Audio.fun("get_global_volume", []() {
        return soloud->getGlobalVolume();
    });
    Audio.fun("get_max_active_voice_count", []() {
        return soloud->getMaxActiveVoiceCount();
    });
    Audio.fun("get_looping", [](unsigned int voiceHandle) {
        return soloud->getLooping(voiceHandle);
    });
    Audio.fun("get_loop_point", [](unsigned int voiceHandle) {
        return soloud->getLoopPoint(voiceHandle);
    });
    Audio.fun("set_loop_point", [](unsigned int voiceHandle, double aLoopPoint) {
        return soloud->setLoopPoint(voiceHandle, aLoopPoint);
    });
    Audio.fun("set_looping", [](unsigned int voiceHandle, bool aLooping) {
        return soloud->setLooping(voiceHandle, aLooping);
    });
    Audio.fun("set_max_active_voice_count", [](unsigned int aVoiceCount) {
        return soloud->setMaxActiveVoiceCount(aVoiceCount);
    });
    Audio.fun("set_inaudible_behavior", [](unsigned int voiceHandle, bool aMustTick, bool aKill) {
        return soloud->setInaudibleBehavior(voiceHandle, aMustTick, aKill);
    });
    Audio.fun("set_global_volume", [](float aVolume) {
        return soloud->setGlobalVolume(aVolume);
    });
    Audio.fun("set_post_clip_scaler", [](float aScaler) {
        return soloud->setPostClipScaler(aScaler);
    });
    Audio.fun("set_pause", [](unsigned int voiceHandle, bool aPause) {
        return soloud->setPause(voiceHandle, aPause);
    });
    Audio.fun("set_pause_all", [](bool aPause) {
        return soloud->setPauseAll(aPause);
    });
    Audio.fun("set_relative_play_speed", [](unsigned int voiceHandle, float aSpeed) {
        return soloud->setRelativePlaySpeed(voiceHandle, aSpeed);
    });
    Audio.fun("set_protect_voice", [](unsigned int voiceHandle, bool aProtect) {
        return soloud->setProtectVoice(voiceHandle, aProtect);
    });
    Audio.fun("set_samplerate", [](unsigned int voiceHandle, float aSamplerate) {
        return soloud->setSamplerate(voiceHandle, aSamplerate);
    });
    Audio.fun("set_pan", [](unsigned int voiceHandle, float aPan) {
        return soloud->setPan(voiceHandle, aPan);
    });
    Audio.fun("set_pan_absolute", [](unsigned int voiceHandle, float aLVolume, float aRVolume, float aLBVolume, float aRBVolume, float aCVolume, float aSVolume) {
        return soloud->setPanAbsolute(voiceHandle, aLVolume, aRVolume, aLBVolume, aRBVolume, aCVolume, aSVolume);
    });
    Audio.fun("set_volume", [](unsigned int voiceHandle, float aVolume) {
        return soloud->setVolume(voiceHandle, aVolume);
    });
    Audio.fun("set_delay_samples", [](unsigned int voiceHandle, unsigned int aSamples) {
        return soloud->setDelaySamples(voiceHandle, aSamples);
    });
    Audio.fun("fade_volume", [](unsigned int voiceHandle, float aTo, double aTime) {
        return soloud->fadeVolume(voiceHandle, aTo, aTime);
    });
    Audio.fun("fade_pan", [](unsigned int voiceHandle, float aTo, double aTime) {
        return soloud->fadePan(voiceHandle, aTo, aTime);
    });
    Audio.fun("fade_relative_play_speed", [](unsigned int voiceHandle, float aTo, double aTime) {
        return soloud->fadeRelativePlaySpeed(voiceHandle, aTo, aTime);
    });
    Audio.fun("fade_global_volume", [](float aTo, double aTime) {
        return soloud->fadeGlobalVolume(aTo, aTime);
    });
    Audio.fun("schedule_pause", [](unsigned int voiceHandle, double aTime) {
        return soloud->schedulePause(voiceHandle, aTime);
    });
    Audio.fun("schedule_stop", [](unsigned int voiceHandle, double aTime) {
        return soloud->scheduleStop(voiceHandle, aTime);
    });
    Audio.fun("oscillate_volume", [](unsigned int voiceHandle, float aFrom, float aTo, double aTime) {
        return soloud->oscillateVolume(voiceHandle, aFrom, aTo, aTime);
    });
    Audio.fun("oscillate_pan", [](unsigned int voiceHandle, float aFrom, float aTo, double aTime) {
        return soloud->oscillatePan(voiceHandle, aFrom, aTo, aTime);
    });
    Audio.fun("oscillate_relative_play_speed", [](unsigned int voiceHandle, float aFrom, float aTo, double aTime) {
        return soloud->oscillateRelativePlaySpeed(voiceHandle, aFrom, aTo, aTime);
    });
    Audio.fun("set_global_filter", [](unsigned int filterId, SoLoud::Filter *aFilter) {
        return soloud->setGlobalFilter(filterId, aFilter);
    });
    Audio.fun("set_visualization_enable", [](bool aEnable) {
        return soloud->setVisualizationEnable(aEnable);
    });
    Audio.fun("calc_fft", []() {
        return soloud->calcFFT();
    });
    Audio.fun("get_wave", []() {
        return soloud->getWave();
    });
    Audio.fun("get_loop_count", [](unsigned int voiceHandle) {
        return soloud->getLoopCount(voiceHandle);
    });
    Audio.fun("create_voice_group", []() {
        return soloud->createVoiceGroup();
    });
    Audio.fun("destroy_voice_group", [](unsigned int voiceGroupHandle) {
        return soloud->destroyVoiceGroup(voiceGroupHandle);
    });
    Audio.fun("add_voice_to_group", [](unsigned int voiceGroupHandle, unsigned int voiceHandle) {
        return soloud->addVoiceToGroup(voiceGroupHandle, voiceHandle);
    });
    Audio.fun("is_voice_group", [](unsigned int voiceGroupHandle) {
        return soloud->isVoiceGroup(voiceGroupHandle);
    });
    Audio.fun("is_voice_group_empty", [](unsigned int voiceGroupHandle) {
        return soloud->isVoiceGroupEmpty(voiceGroupHandle);
    });
    Audio.fun("update3d_audio", []() {
        return soloud->update3dAudio();
    });
    Audio.fun("set3d_sound_speed", [](float aSpeed) {
        return soloud->set3dSoundSpeed(aSpeed);
    });
    Audio.fun("get3d_sound_speed", []() {
        return soloud->get3dSoundSpeed();
    });
    Audio.fun("set3d_listener_parameters", [](float aPosX, float aPosY, float aPosZ, float aAtX, float aAtY, float aAtZ, float aUpX, float aUpY, float aUpZ) {
        return soloud->set3dListenerParameters(aPosX, aPosY, aPosZ, aAtX, aAtY, aAtZ, aUpX, aUpY, aUpZ);
    });
    Audio.fun("set3d_listener_position", [](float aPosX, float aPosY, float aPosZ) {
        return soloud->set3dListenerPosition(aPosX, aPosY, aPosZ);
    });
    Audio.fun("set3d_listener_at", [](float aAtX, float aAtY, float aAtZ) {
        return soloud->set3dListenerAt(aAtX, aAtY, aAtZ);
    });
    Audio.fun("set3d_listener_up", [](float aUpX, float aUpY, float aUpZ) {
        return soloud->set3dListenerUp(aUpX, aUpY, aUpZ);
    });
    Audio.fun("set3d_listener_velocity", [](float aVelocityX, float aVelocityY, float aVelocityZ) {
        return soloud->set3dListenerVelocity(aVelocityX, aVelocityY, aVelocityZ);
    });
    Audio.fun("set3d_source_parameters", [](unsigned int voiceHandle, float aPosX, float aPosY, float aPosZ) {
        return soloud->set3dSourceParameters(voiceHandle, aPosX, aPosY, aPosZ);
    });
    Audio.fun("set3d_source_position", [](unsigned int voiceHandle, float aPosX, float aPosY, float aPosZ) {
        return soloud->set3dSourcePosition(voiceHandle, aPosX, aPosY, aPosZ);
    });
    Audio.fun("set3d_source_velocity", [](unsigned int voiceHandle, float aVelocityX, float aVelocityY, float aVelocityZ) {
        return soloud->set3dSourceVelocity(voiceHandle, aVelocityX, aVelocityY, aVelocityZ);
    });
    Audio.fun("set3d_source_min_max_distance", [](unsigned int voiceHandle, float aMinDistance, float aMaxDistance) {
        return soloud->set3dSourceMinMaxDistance(voiceHandle, aMinDistance, aMaxDistance);
    });
    Audio.fun("set3d_source_attenuation", [](unsigned int voiceHandle, unsigned int model, float rolloffFactor) {
        return soloud->set3dSourceAttenuation(voiceHandle, model, rolloffFactor);
    });
    Audio.fun("set3d_source_doppler_factor", [](unsigned int voiceHandle, float aDopplerFactor) {
        return soloud->set3dSourceDopplerFactor(voiceHandle, aDopplerFactor);
    });
    Audio.fun("mix", [](float *aBuffer, unsigned int aSamples) {
        return soloud->mix(aBuffer, aSamples);
    });
    Audio.fun("mix_signed16", [](short *aBuffer, unsigned int aSamples) {
        return soloud->mixSigned16(aBuffer, aSamples);
    });
    Audio.cfun("bus", [](lua_State* L) -> int {
        LuaBinding::State S(L);
        S.alloc<SoLoud::Bus>();
        return 1;
    });

    lua_newuserdata(L, 0);
    lua_newtable(L);
    lua_pushcclosure(L, luaclose_AudioLib, 0);
    lua_setfield(L, -2, "__gc");
    lua_setmetatable(L, -2);
    luaL_ref(L, LUA_REGISTRYINDEX);

    lua_newtable(L);
    lua_setfield(L, -2, "SFXR");

    Audio["SFXR"].set("COIN", (int)SoLoud::Sfxr::SFXR_PRESETS::COIN);
    Audio["SFXR"].set("LASER", (int)SoLoud::Sfxr::SFXR_PRESETS::LASER);
    Audio["SFXR"].set("EXPLOSION", (int)SoLoud::Sfxr::SFXR_PRESETS::EXPLOSION);
    Audio["SFXR"].set("POWERUP", (int)SoLoud::Sfxr::SFXR_PRESETS::POWERUP);
    Audio["SFXR"].set("HURT", (int)SoLoud::Sfxr::SFXR_PRESETS::HURT);
    Audio["SFXR"].set("JUMP", (int)SoLoud::Sfxr::SFXR_PRESETS::JUMP);
    Audio["SFXR"].set("BLIP", (int)SoLoud::Sfxr::SFXR_PRESETS::BLIP);

    lua_pushvalue(L, -1);
    soloud_ref = S.at(-1);

    S.addClass<SoLoud::Wav>("Sound")
            .prop_fun("source", [](SoLoud::Wav* self) {
                return (SoLoud::AudioSource*)self;
            })
            .prop("sample_count", &SoLoud::Wav::mSampleCount)
            .prop_fun("length", &SoLoud::Wav::getLength)
            .fun("load", &SoLoud::Wav::load);

    S.addClass<SoLoud::WavStream>("SoundStream")
            .prop_fun("source", [](SoLoud::WavStream* self) {
                return (SoLoud::AudioSource*)self;
            })
            .prop_fun("length", &SoLoud::WavStream::getLength)
            .prop("sample_count", &SoLoud::WavStream::mSampleCount)
            .prop("filename", &SoLoud::WavStream::mFilename)
            .fun("load", &SoLoud::WavStream::load);

    S.addClass<SoLoud::Speech>("Speech")
            .prop_fun("source", [](SoLoud::Speech* self) {
                return (SoLoud::AudioSource*)self;
            })
            .prop("base_frequency", &SoLoud::Speech::mBaseFrequency)
            .prop("base_speed", &SoLoud::Speech::mBaseSpeed)
            .prop("base_declination", &SoLoud::Speech::mBaseDeclination)
            .prop("base_waveform", &SoLoud::Speech::mBaseWaveform)
            .prop("frames", &SoLoud::Speech::mFrames)
            .fun("set_text", &SoLoud::Speech::setText)
            .fun("set_params", &SoLoud::Speech::setParams);

    S.addClass<SoLoud::Vizsn>("Speech")
            .prop_fun("source", [](SoLoud::Vizsn* self) {
                return (SoLoud::AudioSource*)self;
            })
            .fun("set_text", &SoLoud::Vizsn::setText);

    S.addClass<SoLoud::Sfxr>("Sfxr")
            .prop_fun("source", [](SoLoud::Sfxr* self) {
                return (SoLoud::AudioSource*)self;
            })
            .fun("load_preset", &SoLoud::Sfxr::loadPreset)
            .fun("load_params", &SoLoud::Sfxr::loadParams);

    S.addClass<SoLoud::AudioSource>("AudioSource")
            .prop("flags", &SoLoud::AudioSource::mFlags)
            .prop("base_samplerate", &SoLoud::AudioSource::mBaseSamplerate)
            .prop("volume", &SoLoud::AudioSource::mVolume)
            .prop("channels", &SoLoud::AudioSource::mChannels)
            .prop("audio_source_iD", &SoLoud::AudioSource::mAudioSourceID)
            .prop("min_distance3d", &SoLoud::AudioSource::m3dMinDistance)
            .prop("max_distance3d", &SoLoud::AudioSource::m3dMaxDistance)
            .prop("attenuation_rolloff3d", &SoLoud::AudioSource::m3dAttenuationRolloff)
            .prop("attenuation_model3d", &SoLoud::AudioSource::m3dAttenuationModel)
            .prop("doppler_factor3d", &SoLoud::AudioSource::m3dDopplerFactor)
            .prop("collider", &SoLoud::AudioSource::mCollider)
            .prop("attenuator", &SoLoud::AudioSource::mAttenuator)
            .prop("collider_data", &SoLoud::AudioSource::mColliderData)
            .prop("loop_point", &SoLoud::AudioSource::mLoopPoint)
            .fun("set_volume", &SoLoud::AudioSource::setVolume)
            .fun("set_looping", &SoLoud::AudioSource::setLooping)
            .fun("set_single_instance", &SoLoud::AudioSource::setSingleInstance)
            .fun("set3d_min_max_distance", &SoLoud::AudioSource::set3dMinMaxDistance)
            .fun("set3d_attenuation", &SoLoud::AudioSource::set3dAttenuation)
            .fun("set3d_doppler_factor", &SoLoud::AudioSource::set3dDopplerFactor)
            .fun("set3d_listener_relative", &SoLoud::AudioSource::set3dListenerRelative)
            .fun("set3d_distance_delay", &SoLoud::AudioSource::set3dDistanceDelay)
            .fun("set3d_collider", &SoLoud::AudioSource::set3dCollider)
            .fun("set3d_attenuator", &SoLoud::AudioSource::set3dAttenuator)
            .fun("set_inaudible_behavior", &SoLoud::AudioSource::setInaudibleBehavior)
            .fun("set_loop_point", &SoLoud::AudioSource::setLoopPoint)
            .fun("get_loop_point", &SoLoud::AudioSource::getLoopPoint)
            .fun("set_filter", &SoLoud::AudioSource::setFilter);

    S.addClass<SoLoud::Bus>("Bus")
            .ctor<>()
            .fun("source", [](SoLoud::Bus* self) {
                return (SoLoud::AudioSource*)self;
            })
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
            .fun("setChannels" , &SoLoud::Bus::setChannels)
            .fun("set_visualization_enable", &SoLoud::Bus::setVisualizationEnable)
            .fun("calc_fft", &SoLoud::Bus::calcFFT)
            .fun("get_wave", &SoLoud::Bus::getWave)
            .fun("get_approximate_volume", &SoLoud::Bus::getApproximateVolume)
            .fun("get_active_voice_count", &SoLoud::Bus::getActiveVoiceCount);

    return 1;
}