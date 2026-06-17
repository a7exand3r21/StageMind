#pragma once

#include "RoleProfile.h"
#include "SafetyMode.h"
#include "SidechainConflictMode.h"
#include <JuceHeader.h>

namespace stagemind
{
class RolePresetEngine
{
public:
    const RoleProfile& getProfile(TrackRole role) const noexcept;

    SpatialParams buildSpatialParams(
        TrackRole role,
        SafetyMode safety,
        const UserMacroParams& macros) const noexcept;

    void applyRoleDefaultsToState(
        TrackRole role,
        juce::AudioProcessorValueTreeState& apvts) const;

    static float mapWidthUserToAmount(float width) noexcept;
    static SafetyLimits getSafetyLimits(SafetyMode safety) noexcept;
};
} // namespace stagemind
