#pragma once

#include "TrackRole.h"

namespace stagemind
{
struct RoleProfile
{
    TrackRole role = TrackRole::Unknown;

    float defaultPan = 0.0f;
    float defaultWidthAmount = 1.0f;
    float maxWidthAmount = 1.35f;
    float defaultDepth = 0.3f;
    float defaultMotionAmount = 0.0f;

    float lowMonoCutoffHz = 160.0f;
    float sideHighPassHz = 220.0f;

    float resonanceSensitivity = 0.5f;
    float maxResonanceReductionDb = 4.0f;

    float cleanUpAmount = 0.3f;
    float maxDynamicEqReductionDb = 4.0f;

    bool protectTransient = false;
    bool protectLowEnd = false;
    bool allowPseudoDouble = false;
    bool allowMotion = false;
    bool keepDryCenter = false;

    float priority = 0.5f;
};

struct UserMacroParams
{
    float width = 0.5f;
    float depth = 0.3f;
    float motion = 0.0f;
    float cleanUp = 0.3f;
    float resonance = 0.3f;
    float pan = 0.0f;
    float monoLowCutoffHz = 160.0f;
    float sideHighPassHz = 220.0f;
};

struct SpatialParams
{
    float pan = 0.0f;
    float widthAmount = 1.0f;
    float monoLowCutoffHz = 160.0f;
    float sideHighPassHz = 220.0f;
};

struct SafetyLimits
{
    float maxWidthAmount = 1.35f;
    float correlationThreshold = 0.10f;
};
} // namespace stagemind
