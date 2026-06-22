#pragma once

#include "TrackRole.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace stagemind
{
inline constexpr int maxRoleBalanceInputs = 128;
inline constexpr int directorBalanceRideStageMode = 2;
inline constexpr float minDirectorOutputTrimDb = -3.0f;
inline constexpr float maxDirectorOutputBoostDb = 3.0f;
inline constexpr float strongLocalStageGainDb = 6.0f;

enum class RoleBalanceFamily
{
    Unknown = 0,
    Vocal,
    Kick,
    Bass,
    Drums,
    Guitar,
    KeysLead,
    Pads,
    FX
};

struct RoleBalanceProfile
{
    RoleBalanceFamily family = RoleBalanceFamily::Unknown;
    float targetOffsetDb = -1.0f;
    float priorityWeight = 1.0f;
    float deadbandDb = 1.4f;
    float correctionGain = 0.32f;
    float maxCutStepDb = 0.75f;
    float maxBoostStepDb = 0.65f;
    float minActivity = 0.10f;
    float minRms = 0.004f;
};

struct RoleBalanceInput
{
    int index = -1;
    TrackRole role = TrackRole::Unknown;
    float outputRms = 0.0f;
    float outputTrimDb = 0.0f;
    float activity = 0.0f;
    bool autoEnabled = false;
    float stageGainDb = 0.0f;
    int stageGainMode = directorBalanceRideStageMode;
};

struct RoleBalanceDecision
{
    bool found = false;
    int index = -1;
    TrackRole role = TrackRole::Unknown;
    RoleBalanceFamily family = RoleBalanceFamily::Unknown;
    float measuredLevelDb = 0.0f;
    float referenceLevelDb = 0.0f;
    float targetOffsetDb = 0.0f;
    float targetLevelDb = 0.0f;
    float deviationDb = 0.0f;
    float correctionDb = 0.0f;
    float nextOutputTrimDb = 0.0f;
    float severity = 0.0f;
};

inline RoleBalanceProfile roleBalanceProfileFor(TrackRole role) noexcept
{
    switch (role)
    {
        case TrackRole::LeadVocal:
        case TrackRole::SunoVocal:
            return { RoleBalanceFamily::Vocal, 1.8f, 1.75f, 1.05f, 0.36f, 0.78f, 0.78f, 0.08f, 0.0035f };

        case TrackRole::BackingVocal:
            return { RoleBalanceFamily::Vocal, -0.8f, 1.05f, 1.25f, 0.30f, 0.70f, 0.58f, 0.08f, 0.0035f };

        case TrackRole::Kick:
            return { RoleBalanceFamily::Kick, 1.0f, 1.45f, 1.05f, 0.33f, 0.76f, 0.66f, 0.10f, 0.0040f };

        case TrackRole::Bass:
        case TrackRole::SynthBass:
        case TrackRole::SunoBass:
            return { RoleBalanceFamily::Bass, 0.45f, 1.45f, 1.10f, 0.34f, 0.76f, 0.70f, 0.10f, 0.0040f };

        case TrackRole::SunoDrums:
            return { RoleBalanceFamily::Drums, 0.85f, 1.35f, 1.10f, 0.32f, 0.74f, 0.64f, 0.10f, 0.0040f };

        case TrackRole::Snare:
            return { RoleBalanceFamily::Drums, 0.25f, 1.15f, 1.20f, 0.30f, 0.66f, 0.56f, 0.10f, 0.0035f };

        case TrackRole::HiHat:
        case TrackRole::Percussion:
        case TrackRole::SunoPercussion:
            return { RoleBalanceFamily::Drums, -1.15f, 0.85f, 1.45f, 0.26f, 0.58f, 0.44f, 0.10f, 0.0030f };

        case TrackRole::RhythmGuitarSingle:
        case TrackRole::RhythmGuitarPairLeft:
        case TrackRole::RhythmGuitarPairRight:
        case TrackRole::SunoGuitar:
            return { RoleBalanceFamily::Guitar, -1.15f, 0.95f, 1.35f, 0.29f, 0.64f, 0.52f, 0.09f, 0.0035f };

        case TrackRole::LeadGuitar:
        case TrackRole::Piano:
        case TrackRole::SynthLead:
            return { RoleBalanceFamily::KeysLead, -0.35f, 1.05f, 1.25f, 0.30f, 0.66f, 0.56f, 0.09f, 0.0035f };

        case TrackRole::Pad:
        case TrackRole::SunoSynthPad:
            return { RoleBalanceFamily::Pads, -2.45f, 0.65f, 1.70f, 0.24f, 0.52f, 0.38f, 0.08f, 0.0030f };

        case TrackRole::SunoInstrumental:
            return { RoleBalanceFamily::Pads, -1.65f, 0.80f, 1.55f, 0.25f, 0.56f, 0.42f, 0.08f, 0.0030f };

        case TrackRole::FX:
        case TrackRole::Atmosphere:
        case TrackRole::SunoFX:
            return { RoleBalanceFamily::FX, -3.20f, 0.45f, 1.90f, 0.22f, 0.45f, 0.32f, 0.07f, 0.0025f };

        case TrackRole::Unknown:
        default:
            return { RoleBalanceFamily::Unknown, -1.25f, 0.75f, 1.60f, 0.25f, 0.55f, 0.42f, 0.10f, 0.0040f };
    }
}

inline bool roleBalanceInputIsActive(const RoleBalanceInput& input) noexcept
{
    const auto profile = roleBalanceProfileFor(input.role);
    return input.index >= 0
        && input.autoEnabled
        && input.stageGainMode == directorBalanceRideStageMode
        && input.activity >= profile.minActivity
        && input.outputRms >= profile.minRms
        && std::isfinite(input.outputRms)
        && std::isfinite(input.outputTrimDb);
}

inline float roleBalanceGainToDecibels(float gain) noexcept
{
    return 20.0f * std::log10(std::max(gain, 1.0e-5f));
}

inline RoleBalanceDecision chooseRoleBalanceDecision(const RoleBalanceInput* inputs, int inputCount) noexcept
{
    struct LevelSample
    {
        int inputIndex = -1;
        float normalizedLevelDb = 0.0f;
        float measuredLevelDb = 0.0f;
        float weight = 1.0f;
    };

    std::array<LevelSample, maxRoleBalanceInputs> samples {};
    auto sampleCount = 0;

    for (int index = 0; index < inputCount && sampleCount < maxRoleBalanceInputs; ++index)
    {
        const auto& input = inputs[index];
        if (! roleBalanceInputIsActive(input))
            continue;

        const auto profile = roleBalanceProfileFor(input.role);
        const auto measured = roleBalanceGainToDecibels(input.outputRms);
        samples[static_cast<size_t> (sampleCount)] = {
            index,
            measured - profile.targetOffsetDb,
            measured,
            profile.priorityWeight
        };
        ++sampleCount;
    }

    if (sampleCount < 2)
        return {};

    std::sort(
        samples.begin(),
        samples.begin() + sampleCount,
        [](const LevelSample& first, const LevelSample& second)
        {
            return first.normalizedLevelDb < second.normalizedLevelDb;
        });

    auto totalWeight = 0.0f;
    auto weightedSum = 0.0f;
    for (int index = 0; index < sampleCount; ++index)
    {
        const auto weight = std::max(0.1f, samples[static_cast<size_t> (index)].weight);
        totalWeight += weight;
        weightedSum += samples[static_cast<size_t> (index)].normalizedLevelDb * weight;
    }

    auto cumulative = 0.0f;
    auto weightedMedian = samples[static_cast<size_t> (sampleCount / 2)].normalizedLevelDb;
    for (int index = 0; index < sampleCount; ++index)
    {
        cumulative += std::max(0.1f, samples[static_cast<size_t> (index)].weight);
        if (cumulative >= totalWeight * 0.5f)
        {
            weightedMedian = samples[static_cast<size_t> (index)].normalizedLevelDb;
            break;
        }
    }

    const auto weightedMean = weightedSum / std::max(0.1f, totalWeight);
    const auto reference = weightedMedian * 0.70f + weightedMean * 0.30f;

    RoleBalanceDecision best;
    auto bestScore = 0.0f;

    for (int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
    {
        const auto inputIndex = samples[static_cast<size_t> (sampleIndex)].inputIndex;
        if (inputIndex < 0 || inputIndex >= inputCount)
            continue;

        const auto& input = inputs[inputIndex];
        const auto profile = roleBalanceProfileFor(input.role);
        const auto deviation = samples[static_cast<size_t> (sampleIndex)].normalizedLevelDb - reference;
        const auto magnitude = std::abs(deviation);
        if (magnitude < profile.deadbandDb)
            continue;

        const auto score = (magnitude - profile.deadbandDb) * (1.0f + profile.priorityWeight * 0.08f);
        if (score <= bestScore)
            continue;

        const auto correction = std::clamp(
            -deviation * profile.correctionGain,
            -profile.maxCutStepDb,
            profile.maxBoostStepDb);
        if (correction > 0.0f && input.stageGainDb >= strongLocalStageGainDb)
            continue;

        const auto nextTrim = std::clamp(
            input.outputTrimDb + correction,
            minDirectorOutputTrimDb,
            maxDirectorOutputBoostDb);
        if (std::abs(nextTrim - input.outputTrimDb) < 0.12f)
            continue;

        best.found = true;
        best.index = input.index;
        best.role = input.role;
        best.family = profile.family;
        best.measuredLevelDb = samples[static_cast<size_t> (sampleIndex)].measuredLevelDb;
        best.referenceLevelDb = reference;
        best.targetOffsetDb = profile.targetOffsetDb;
        best.targetLevelDb = reference + profile.targetOffsetDb;
        best.deviationDb = deviation;
        best.correctionDb = nextTrim - input.outputTrimDb;
        best.nextOutputTrimDb = nextTrim;
        best.severity = std::clamp((magnitude - profile.deadbandDb) / 6.0f, 0.0f, 1.0f);
        bestScore = score;
    }

    return best;
}
} // namespace stagemind
