#include "RolePresetEngine.h"
#include "Parameters.h"

namespace stagemind
{
namespace
{
constexpr std::array<RoleProfile, 26> profiles {{
    { TrackRole::Unknown, 0.00f, 1.00f, 1.35f, 0.30f, 0.00f, 160.0f, 220.0f, 0.50f, 4.0f, 0.30f, 4.0f, false, false, false, false, false, 0.50f },

    { TrackRole::LeadVocal,             0.00f, 1.00f, 1.20f, 0.20f, 0.00f, 120.0f, 180.0f, 0.60f, 4.0f, 0.50f, 4.0f, false, false, false, false, true,  1.00f },
    { TrackRole::BackingVocal,          0.35f, 1.25f, 1.55f, 0.60f, 0.15f, 140.0f, 200.0f, 0.60f, 4.0f, 0.50f, 4.0f, false, false, true,  true,  false, 0.60f },
    { TrackRole::Kick,                  0.00f, 0.25f, 0.60f, 0.10f, 0.00f, 180.0f, 200.0f, 0.50f, 3.0f, 0.30f, 3.0f, true,  true,  false, false, true,  1.00f },
    { TrackRole::Bass,                  0.00f, 0.35f, 0.75f, 0.15f, 0.00f, 160.0f, 200.0f, 0.40f, 3.0f, 0.40f, 3.0f, false, true,  false, false, true,  1.00f },
    { TrackRole::Snare,                 0.00f, 0.90f, 1.20f, 0.25f, 0.00f, 140.0f, 180.0f, 0.75f, 5.0f, 0.50f, 4.0f, true,  false, false, false, true,  1.00f },
    { TrackRole::HiHat,                 0.35f, 1.00f, 1.25f, 0.35f, 0.10f, 200.0f, 300.0f, 0.70f, 4.0f, 0.50f, 4.0f, true,  false, false, true,  false, 0.60f },
    { TrackRole::Percussion,            0.45f, 1.20f, 1.50f, 0.45f, 0.20f, 160.0f, 220.0f, 0.55f, 4.0f, 0.50f, 4.0f, true,  false, false, true,  false, 0.60f },
    { TrackRole::RhythmGuitarSingle,    0.35f, 1.10f, 1.45f, 0.45f, 0.05f, 140.0f, 180.0f, 0.55f, 4.0f, 0.50f, 4.0f, false, false, true,  false, false, 0.60f },
    { TrackRole::RhythmGuitarPairLeft, -0.85f, 1.00f, 1.20f, 0.45f, 0.00f, 140.0f, 180.0f, 0.55f, 4.0f, 0.50f, 4.0f, false, false, false, false, false, 0.60f },
    { TrackRole::RhythmGuitarPairRight, 0.85f, 1.00f, 1.20f, 0.45f, 0.00f, 140.0f, 180.0f, 0.55f, 4.0f, 0.50f, 4.0f, false, false, false, false, false, 0.60f },
    { TrackRole::LeadGuitar,            0.00f, 1.00f, 1.30f, 0.30f, 0.05f, 140.0f, 180.0f, 0.55f, 4.0f, 0.50f, 4.0f, false, false, true,  false, true,  0.80f },
    { TrackRole::Pad,                   0.00f, 1.45f, 1.70f, 0.80f, 0.20f, 180.0f, 220.0f, 0.40f, 3.0f, 0.40f, 3.0f, false, false, true,  true,  false, 0.25f },
    { TrackRole::Piano,                 0.00f, 1.15f, 1.40f, 0.45f, 0.00f, 120.0f, 180.0f, 0.55f, 4.0f, 0.50f, 4.0f, true,  false, false, false, true,  0.60f },
    { TrackRole::SynthLead,             0.00f, 1.10f, 1.35f, 0.30f, 0.10f, 140.0f, 200.0f, 0.55f, 4.0f, 0.50f, 4.0f, false, false, true,  true,  true,  0.80f },
    { TrackRole::SynthBass,             0.00f, 0.35f, 0.75f, 0.15f, 0.00f, 160.0f, 220.0f, 0.40f, 3.0f, 0.40f, 3.0f, false, true,  false, false, true,  1.00f },
    { TrackRole::FX,                    0.00f, 1.50f, 1.80f, 0.70f, 0.35f, 180.0f, 220.0f, 0.35f, 3.0f, 0.30f, 3.0f, false, false, true,  true,  false, 0.25f },
    { TrackRole::Atmosphere,            0.00f, 1.50f, 1.80f, 0.85f, 0.20f, 200.0f, 250.0f, 0.25f, 2.5f, 0.30f, 2.5f, false, false, true,  true,  false, 0.25f },
    { TrackRole::SunoVocal,             0.00f, 1.00f, 1.15f, 0.30f, 0.00f, 120.0f, 180.0f, 0.55f, 3.0f, 0.50f, 3.5f, false, false, false, false, true,  1.00f },
    { TrackRole::SunoInstrumental,      0.00f, 1.10f, 1.35f, 0.45f, 0.05f, 160.0f, 220.0f, 0.55f, 3.0f, 0.50f, 3.5f, false, true,  false, false, false, 0.60f },
    { TrackRole::SunoDrums,             0.00f, 1.00f, 1.25f, 0.30f, 0.00f, 160.0f, 220.0f, 0.55f, 4.0f, 0.50f, 4.0f, true,  true,  false, false, true,  1.00f },
    { TrackRole::SunoBass,              0.00f, 0.35f, 0.75f, 0.15f, 0.00f, 160.0f, 220.0f, 0.40f, 3.0f, 0.30f, 3.0f, false, true,  false, false, true,  1.00f },
    { TrackRole::SunoGuitar,            0.00f, 1.10f, 1.35f, 0.45f, 0.05f, 140.0f, 180.0f, 0.55f, 3.5f, 0.50f, 3.5f, false, false, true,  false, false, 0.60f },
    { TrackRole::SunoSynthPad,          0.00f, 1.40f, 1.65f, 0.70f, 0.15f, 180.0f, 220.0f, 0.40f, 3.0f, 0.40f, 3.0f, false, false, true,  true,  false, 0.40f },
    { TrackRole::SunoPercussion,        0.40f, 1.15f, 1.45f, 0.45f, 0.15f, 160.0f, 220.0f, 0.55f, 3.5f, 0.50f, 3.5f, true,  false, false, true,  false, 0.60f },
    { TrackRole::SunoFX,                0.00f, 1.45f, 1.75f, 0.70f, 0.25f, 180.0f, 240.0f, 0.35f, 3.0f, 0.30f, 3.0f, false, false, true,  true,  false, 0.25f }
}};

constexpr float clamp01(float value) noexcept
{
    return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}
} // namespace

const RoleProfile& RolePresetEngine::getProfile(TrackRole role) const noexcept
{
    for (const auto& profile : profiles)
        if (profile.role == role)
            return profile;

    return profiles.front();
}

float RolePresetEngine::mapWidthUserToAmount(float width) noexcept
{
    width = clamp01(width);

    if (width <= 0.30f)
        return juce::jmap(width, 0.0f, 0.30f, 0.25f, 1.0f);

    if (width <= 0.70f)
        return juce::jmap(width, 0.30f, 0.70f, 1.0f, 1.35f);

    return juce::jmap(width, 0.70f, 1.0f, 1.35f, 1.80f);
}

SafetyLimits RolePresetEngine::getSafetyLimits(SafetyMode safety) noexcept
{
    switch (safety)
    {
        case SafetyMode::MonoSafe:       return { 1.10f,  0.25f };
        case SafetyMode::ModernWide:     return { 1.60f,  0.00f };
        case SafetyMode::HeadphonesWide: return { 1.80f, -0.10f };
        case SafetyMode::Natural:
        default:                         return { 1.35f,  0.10f };
    }
}

SpatialParams RolePresetEngine::buildSpatialParams(
    TrackRole role,
    SafetyMode safety,
    const UserMacroParams& macros) const noexcept
{
    const auto& profile = getProfile(role);
    const auto limits = getSafetyLimits(safety);

    SpatialParams params;
    params.pan = juce::jlimit(-1.0f, 1.0f, profile.defaultPan + macros.pan);
    params.widthAmount = std::min({ mapWidthUserToAmount(macros.width), profile.maxWidthAmount, limits.maxWidthAmount });
    params.monoLowCutoffHz = juce::jlimit(40.0f, 300.0f, macros.monoLowCutoffHz);
    params.sideHighPassHz = juce::jlimit(40.0f, 500.0f, macros.sideHighPassHz);

    return params;
}

void RolePresetEngine::applyRoleDefaultsToState(
    TrackRole role,
    juce::AudioProcessorValueTreeState& apvts) const
{
    const auto& profile = getProfile(role);

    // Role pan is applied in buildSpatialParams; the APVTS pan value is a user offset.
    if (auto* parameter = apvts.getParameter(parameters::ids::pan))
        parameter->setValueNotifyingHost(parameter->convertTo0to1(0.0f));

    if (auto* parameter = apvts.getParameter(parameters::ids::monoLowCutoff))
        parameter->setValueNotifyingHost(parameter->convertTo0to1(profile.lowMonoCutoffHz));

    if (auto* parameter = apvts.getParameter(parameters::ids::sideHighPass))
        parameter->setValueNotifyingHost(parameter->convertTo0to1(profile.sideHighPassHz));
}
} // namespace stagemind
