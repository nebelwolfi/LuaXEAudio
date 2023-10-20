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
int lua_Audio_Init(lua_State *L) {
    if (!soloud) {
        soloud = new SoLoud::Soloud();
        soloud->init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::AUTO, SoLoud::Soloud::AUTO, SoLoud::Soloud::AUTO, 8);
    }
    LuaBinding::State S(L);
    AudioBasePath = S.at(1).as<std::string>();
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
        { "Init", lua_Audio_Init },
        { "Shutdown", lua_Audio_Shutdown },
        { "Play", lua_Audio_Play },
        { "File", lua_Audio_File },
        { "FileStream", lua_Audio_FileStream },
        { "Speech", lua_Audio_Speech },
        { "Vizsn", lua_Audio_Vizsn },
        { "Sfxr", lua_Audio_Sfxr },
        { "Mod", lua_Audio_Mod },
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
    auto S = LuaBinding::State(L);
    if (soloud_ref.valid())
        return soloud_ref.push();

    luaL_newlib(L, AudioLib);

    auto Audio = S[-1];
    Audio.fun("PlaySource", [](SoLoud::AudioSource* source) {
        return soloud->play(*source);
    });
    Audio.fun("PlayClocked", [](double aSoundTime, SoLoud::AudioSource* source) {
        return soloud->playClocked(aSoundTime, *source);
    });
    Audio.fun("Play3d", [](SoLoud::AudioSource* source, float x, float y, float z) {
        return soloud->play3d(*source, x, y, z);
    });
    Audio.fun("Play3dClocked", [](double aSoundTime, SoLoud::AudioSource* source, float x, float y, float z) {
        return soloud->play3dClocked(aSoundTime, *source, x, y, z);
    });
    Audio.fun("PlayBackground", [](SoLoud::AudioSource* source) {
        return soloud->playBackground(*source);
    });
    Audio.fun("Seek", [](unsigned int voiceHandle, double aSeconds) {
        return soloud->seek(voiceHandle, aSeconds);
    });
    Audio.fun("Stop", [](unsigned int voiceHandle) {
        return soloud->stop(voiceHandle);
    });
    Audio.fun("StopAll", []() {
        return soloud->stopAll();
    });
    Audio.fun("StopAudioSource", [](SoLoud::AudioSource* source) {
        return soloud->stopAudioSource(*source);
    });
    Audio.fun("CountAudioSource", [](SoLoud::AudioSource* source) {
        return soloud->countAudioSource(*source);
    });
    Audio.fun("SetFilterParameter", [](unsigned int voiceHandle, unsigned int filterId, unsigned int attributeId, float value) {
        return soloud->setFilterParameter(voiceHandle, filterId, attributeId, value);
    });
    Audio.fun("GetFilterParameter", [](unsigned int voiceHandle, unsigned int filterId, unsigned int attributeId) {
        return soloud->getFilterParameter(voiceHandle, filterId, attributeId);
    });
    Audio.fun("FadeFilterParameter", [](unsigned int voiceHandle, unsigned int filterId, unsigned int attributeId, float to, double aTime) {
        return soloud->fadeFilterParameter(voiceHandle, filterId, attributeId, to, aTime);
    });
    Audio.fun("OscillateFilterParameter", [](unsigned int voiceHandle, unsigned int filterId, unsigned int attributeId, float from, float to, double aTime) {
        return soloud->oscillateFilterParameter(voiceHandle, filterId, attributeId, from, to, aTime);
    });
    Audio.fun("GetStreamTime", [](unsigned int voiceHandle) {
        return soloud->getStreamTime(voiceHandle);
    });
    Audio.fun("GetPause", [](unsigned int voiceHandle) {
        return soloud->getPause(voiceHandle);
    });
    Audio.fun("GetVolume", [](unsigned int voiceHandle) {
        return soloud->getVolume(voiceHandle);
    });
    Audio.fun("GetOverallVolume", [](unsigned int voiceHandle) {
        return soloud->getOverallVolume(voiceHandle);
    });
    Audio.fun("GetPan", [](unsigned int voiceHandle) {
        return soloud->getPan(voiceHandle);
    });
    Audio.fun("GetSamplerate", [](unsigned int voiceHandle) {
        return soloud->getSamplerate(voiceHandle);
    });
    Audio.fun("GetProtectVoice", [](unsigned int voiceHandle) {
        return soloud->getProtectVoice(voiceHandle);
    });
    Audio.fun("GetActiveVoiceCount", []() {
        return soloud->getActiveVoiceCount();
    });
    Audio.fun("GetVoiceCount", []() {
        return soloud->getVoiceCount();
    });
    Audio.fun("IsValidVoiceHandle", [](unsigned int voiceHandle) {
        return soloud->isValidVoiceHandle(voiceHandle);
    });
    Audio.fun("GetRelativePlaySpeed", [](unsigned int voiceHandle) {
        return soloud->getRelativePlaySpeed(voiceHandle);
    });
    Audio.fun("GetPostClipScaler", []() {
        return soloud->getPostClipScaler();
    });
    Audio.fun("GetGlobalVolume", []() {
        return soloud->getGlobalVolume();
    });
    Audio.fun("GetMaxActiveVoiceCount", []() {
        return soloud->getMaxActiveVoiceCount();
    });
    Audio.fun("GetLooping", [](unsigned int voiceHandle) {
        return soloud->getLooping(voiceHandle);
    });
    Audio.fun("GetLoopPoint", [](unsigned int voiceHandle) {
        return soloud->getLoopPoint(voiceHandle);
    });
    Audio.fun("SetLoopPoint", [](unsigned int voiceHandle, double aLoopPoint) {
        return soloud->setLoopPoint(voiceHandle, aLoopPoint);
    });
    Audio.fun("SetLooping", [](unsigned int voiceHandle, bool aLooping) {
        return soloud->setLooping(voiceHandle, aLooping);
    });
    Audio.fun("SetMaxActiveVoiceCount", [](unsigned int aVoiceCount) {
        return soloud->setMaxActiveVoiceCount(aVoiceCount);
    });
    Audio.fun("SetInaudibleBehavior", [](unsigned int voiceHandle, bool aMustTick, bool aKill) {
        return soloud->setInaudibleBehavior(voiceHandle, aMustTick, aKill);
    });
    Audio.fun("SetGlobalVolume", [](float aVolume) {
        return soloud->setGlobalVolume(aVolume);
    });
    Audio.fun("SetPostClipScaler", [](float aScaler) {
        return soloud->setPostClipScaler(aScaler);
    });
    Audio.fun("SetPause", [](unsigned int voiceHandle, bool aPause) {
        return soloud->setPause(voiceHandle, aPause);
    });
    Audio.fun("SetPauseAll", [](bool aPause) {
        return soloud->setPauseAll(aPause);
    });
    Audio.fun("SetRelativePlaySpeed", [](unsigned int voiceHandle, float aSpeed) {
        return soloud->setRelativePlaySpeed(voiceHandle, aSpeed);
    });
    Audio.fun("SetProtectVoice", [](unsigned int voiceHandle, bool aProtect) {
        return soloud->setProtectVoice(voiceHandle, aProtect);
    });
    Audio.fun("SetSamplerate", [](unsigned int voiceHandle, float aSamplerate) {
        return soloud->setSamplerate(voiceHandle, aSamplerate);
    });
    Audio.fun("SetPan", [](unsigned int voiceHandle, float aPan) {
        return soloud->setPan(voiceHandle, aPan);
    });
    Audio.fun("SetPanAbsolute", [](unsigned int voiceHandle, float aLVolume, float aRVolume, float aLBVolume, float aRBVolume, float aCVolume, float aSVolume) {
        return soloud->setPanAbsolute(voiceHandle, aLVolume, aRVolume, aLBVolume, aRBVolume, aCVolume, aSVolume);
    });
    Audio.fun("SetVolume", [](unsigned int voiceHandle, float aVolume) {
        return soloud->setVolume(voiceHandle, aVolume);
    });
    Audio.fun("SetDelaySamples", [](unsigned int voiceHandle, unsigned int aSamples) {
        return soloud->setDelaySamples(voiceHandle, aSamples);
    });
    Audio.fun("FadeVolume", [](unsigned int voiceHandle, float aTo, double aTime) {
        return soloud->fadeVolume(voiceHandle, aTo, aTime);
    });
    Audio.fun("FadePan", [](unsigned int voiceHandle, float aTo, double aTime) {
        return soloud->fadePan(voiceHandle, aTo, aTime);
    });
    Audio.fun("FadeRelativePlaySpeed", [](unsigned int voiceHandle, float aTo, double aTime) {
        return soloud->fadeRelativePlaySpeed(voiceHandle, aTo, aTime);
    });
    Audio.fun("FadeGlobalVolume", [](float aTo, double aTime) {
        return soloud->fadeGlobalVolume(aTo, aTime);
    });
    Audio.fun("SchedulePause", [](unsigned int voiceHandle, double aTime) {
        return soloud->schedulePause(voiceHandle, aTime);
    });
    Audio.fun("ScheduleStop", [](unsigned int voiceHandle, double aTime) {
        return soloud->scheduleStop(voiceHandle, aTime);
    });
    Audio.fun("OscillateVolume", [](unsigned int voiceHandle, float aFrom, float aTo, double aTime) {
        return soloud->oscillateVolume(voiceHandle, aFrom, aTo, aTime);
    });
    Audio.fun("OscillatePan", [](unsigned int voiceHandle, float aFrom, float aTo, double aTime) {
        return soloud->oscillatePan(voiceHandle, aFrom, aTo, aTime);
    });
    Audio.fun("OscillateRelativePlaySpeed", [](unsigned int voiceHandle, float aFrom, float aTo, double aTime) {
        return soloud->oscillateRelativePlaySpeed(voiceHandle, aFrom, aTo, aTime);
    });
    Audio.fun("SetGlobalFilter", [](unsigned int filterId, SoLoud::Filter *aFilter) {
        return soloud->setGlobalFilter(filterId, aFilter);
    });
    Audio.fun("SetVisualizationEnable", [](bool aEnable) {
        return soloud->setVisualizationEnable(aEnable);
    });
    Audio.fun("CalcFFT", []() {
        return soloud->calcFFT();
    });
    Audio.fun("GetWave", []() {
        return soloud->getWave();
    });
    Audio.fun("GetLoopCount", [](unsigned int voiceHandle) {
        return soloud->getLoopCount(voiceHandle);
    });
    Audio.fun("CreateVoiceGroup", []() {
        return soloud->createVoiceGroup();
    });
    Audio.fun("DestroyVoiceGroup", [](unsigned int voiceGroupHandle) {
        return soloud->destroyVoiceGroup(voiceGroupHandle);
    });
    Audio.fun("AddVoiceToGroup", [](unsigned int voiceGroupHandle, unsigned int voiceHandle) {
        return soloud->addVoiceToGroup(voiceGroupHandle, voiceHandle);
    });
    Audio.fun("IsVoiceGroup", [](unsigned int voiceGroupHandle) {
        return soloud->isVoiceGroup(voiceGroupHandle);
    });
    Audio.fun("IsVoiceGroupEmpty", [](unsigned int voiceGroupHandle) {
        return soloud->isVoiceGroupEmpty(voiceGroupHandle);
    });
    Audio.fun("Update3dAudio", []() {
        return soloud->update3dAudio();
    });
    Audio.fun("Set3dSoundSpeed", [](float aSpeed) {
        return soloud->set3dSoundSpeed(aSpeed);
    });
    Audio.fun("Get3dSoundSpeed", []() {
        return soloud->get3dSoundSpeed();
    });
    Audio.fun("Set3dListenerParameters", [](float aPosX, float aPosY, float aPosZ, float aAtX, float aAtY, float aAtZ, float aUpX, float aUpY, float aUpZ) {
        return soloud->set3dListenerParameters(aPosX, aPosY, aPosZ, aAtX, aAtY, aAtZ, aUpX, aUpY, aUpZ);
    });
    Audio.fun("Set3dListenerPosition", [](float aPosX, float aPosY, float aPosZ) {
        return soloud->set3dListenerPosition(aPosX, aPosY, aPosZ);
    });
    Audio.fun("Set3dListenerAt", [](float aAtX, float aAtY, float aAtZ) {
        return soloud->set3dListenerAt(aAtX, aAtY, aAtZ);
    });
    Audio.fun("Set3dListenerUp", [](float aUpX, float aUpY, float aUpZ) {
        return soloud->set3dListenerUp(aUpX, aUpY, aUpZ);
    });
    Audio.fun("Set3dListenerVelocity", [](float aVelocityX, float aVelocityY, float aVelocityZ) {
        return soloud->set3dListenerVelocity(aVelocityX, aVelocityY, aVelocityZ);
    });
    Audio.fun("Set3dSourceParameters", [](unsigned int voiceHandle, float aPosX, float aPosY, float aPosZ) {
        return soloud->set3dSourceParameters(voiceHandle, aPosX, aPosY, aPosZ);
    });
    Audio.fun("Set3dSourcePosition", [](unsigned int voiceHandle, float aPosX, float aPosY, float aPosZ) {
        return soloud->set3dSourcePosition(voiceHandle, aPosX, aPosY, aPosZ);
    });
    Audio.fun("Set3dSourceVelocity", [](unsigned int voiceHandle, float aVelocityX, float aVelocityY, float aVelocityZ) {
        return soloud->set3dSourceVelocity(voiceHandle, aVelocityX, aVelocityY, aVelocityZ);
    });
    Audio.fun("Set3dSourceMinMaxDistance", [](unsigned int voiceHandle, float aMinDistance, float aMaxDistance) {
        return soloud->set3dSourceMinMaxDistance(voiceHandle, aMinDistance, aMaxDistance);
    });
    Audio.fun("Set3dSourceAttenuation", [](unsigned int voiceHandle, unsigned int model, float rolloffFactor) {
        return soloud->set3dSourceAttenuation(voiceHandle, model, rolloffFactor);
    });
    Audio.fun("Set3dSourceDopplerFactor", [](unsigned int voiceHandle, float aDopplerFactor) {
        return soloud->set3dSourceDopplerFactor(voiceHandle, aDopplerFactor);
    });
    Audio.fun("Mix", [](float *aBuffer, unsigned int aSamples) {
        return soloud->mix(aBuffer, aSamples);
    });
    Audio.fun("MixSigned16", [](short *aBuffer, unsigned int aSamples) {
        return soloud->mixSigned16(aBuffer, aSamples);
    });
    Audio.cfun("Bus", [](lua_State* L) -> int {
        LuaBinding::State S(L);
        S.alloc<SoLoud::Bus>();
        return 1;
    });

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
            .prop("mSampleCount", &SoLoud::Wav::mSampleCount)
            .prop_fun("length", &SoLoud::Wav::getLength)
            .fun("Load", &SoLoud::Wav::load);

    S.addClass<SoLoud::WavStream>("SoundStream")
            .prop_fun("source", [](SoLoud::WavStream* self) {
                return (SoLoud::AudioSource*)self;
            })
            .prop_fun("length", &SoLoud::WavStream::getLength)
            .prop("mSampleCount", &SoLoud::WavStream::mSampleCount)
            .prop("filename", &SoLoud::WavStream::mFilename)
            .fun("Load", &SoLoud::WavStream::load);

    S.addClass<SoLoud::Speech>("Speech")
            .prop_fun("source", [](SoLoud::Speech* self) {
                return (SoLoud::AudioSource*)self;
            })
            .prop("mBaseFrequency", &SoLoud::Speech::mBaseFrequency)
            .prop("mBaseSpeed", &SoLoud::Speech::mBaseSpeed)
            .prop("mBaseDeclination", &SoLoud::Speech::mBaseDeclination)
            .prop("mBaseWaveform", &SoLoud::Speech::mBaseWaveform)
            .prop("mFrames", &SoLoud::Speech::mFrames)
            .fun("SetText", &SoLoud::Speech::setText)
            .fun("SetParams", &SoLoud::Speech::setParams);

    S.addClass<SoLoud::Vizsn>("Speech")
            .prop_fun("source", [](SoLoud::Vizsn* self) {
                return (SoLoud::AudioSource*)self;
            })
            .fun("SetText", &SoLoud::Vizsn::setText);

    S.addClass<SoLoud::Sfxr>("Sfxr")
            .prop_fun("source", [](SoLoud::Sfxr* self) {
                return (SoLoud::AudioSource*)self;
            })
            .fun("LoadPreset", &SoLoud::Sfxr::loadPreset)
            .fun("LoadParams", &SoLoud::Sfxr::loadParams);

    S.addClass<SoLoud::AudioSource>("AudioSource")
            .prop("flags", &SoLoud::AudioSource::mFlags)
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
            .fun("SetVolume", &SoLoud::AudioSource::setVolume)
            .fun("SetLooping", &SoLoud::AudioSource::setLooping)
            .fun("SetSingleInstance", &SoLoud::AudioSource::setSingleInstance)
            .fun("Set3dMinMaxDistance", &SoLoud::AudioSource::set3dMinMaxDistance)
            .fun("Set3dAttenuation", &SoLoud::AudioSource::set3dAttenuation)
            .fun("Set3dDopplerFactor", &SoLoud::AudioSource::set3dDopplerFactor)
            .fun("Set3dListenerRelative", &SoLoud::AudioSource::set3dListenerRelative)
            .fun("Set3dDistanceDelay", &SoLoud::AudioSource::set3dDistanceDelay)
            .fun("Set3dCollider", &SoLoud::AudioSource::set3dCollider)
            .fun("Set3dAttenuator", &SoLoud::AudioSource::set3dAttenuator)
            .fun("SetInaudibleBehavior", &SoLoud::AudioSource::setInaudibleBehavior)
            .fun("SetLoopPoint", &SoLoud::AudioSource::setLoopPoint)
            .fun("GetLoopPoint", &SoLoud::AudioSource::getLoopPoint)
            .fun("SetFilter", &SoLoud::AudioSource::setFilter);

    S.addClass<SoLoud::Bus>("Bus")
            .ctor<>()
            .fun("Source", [](SoLoud::Bus* self) {
                return (SoLoud::AudioSource*)self;
            })
            .fun("Play", [](SoLoud::Bus* self, SoLoud::AudioSource* source) {
                return self->play(*source);
            })
            .fun("PlayClocked", [](SoLoud::Bus* self, double aSoundTime, SoLoud::AudioSource* source) {
                return self->playClocked(aSoundTime, *source);
            })
            .fun("Play3d", [](SoLoud::Bus* self, SoLoud::AudioSource* source, float x, float y, float z) {
                return self->play3d(*source, x, y, z);
            })
            .fun("Play3dClocked", [](SoLoud::Bus* self, double aSoundTime, SoLoud::AudioSource* source, float x, float y, float z) {
                return self->play3dClocked(aSoundTime, *source, x, y, z);
            })
            .fun("SetChannels" , &SoLoud::Bus::setChannels)
            .fun("SetVisualizationEnable", &SoLoud::Bus::setVisualizationEnable)
            .fun("CalcFFT", &SoLoud::Bus::calcFFT)
            .fun("GetWave", &SoLoud::Bus::getWave)
            .fun("GetApproximateVolume", &SoLoud::Bus::getApproximateVolume)
            .fun("GetActiveVoiceCount", &SoLoud::Bus::getActiveVoiceCount);

    return 1;
}