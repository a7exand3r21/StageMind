#pragma once

#include "LinkSpectralBands.h"
#include "../Model/TrackRole.h"

namespace stagemind
{
enum class LinkSuggestionKind
{
    None = 0,
    StereoSafety,
    VocalSpace,
    KickBass,
    VocalInstrumentDucking,
    SnareInstrumentDucking,
    LeadPadDucking,
    DoubleWide,
    DrumsInstrumentDucking
};

struct LinkSuggestionInput
{
    bool linkActive = false;
    bool peerFound = false;
    TrackRole currentRole = TrackRole::Unknown;
    TrackRole peerRole = TrackRole::Unknown;
    float currentWidth = 0.0f;
    float currentDepth = 0.0f;
    float currentCorrelation = 1.0f;
    float peerActivity = 0.0f;
    float peerWidth = 0.0f;
    LinkSpectralBands currentBands;
    LinkSpectralBands peerBands;
};

struct LinkSuggestion
{
    LinkSuggestionKind kind = LinkSuggestionKind::None;
    float severity = 0.0f;
    const char* message = "Suggestion idle";
    const char* reason = "";
    const char* actionLabel = "Apply Tip";
    const char* appliedMessage = "Applied";

    bool hasSuggestion() const noexcept
    {
        return kind != LinkSuggestionKind::None;
    }
};

struct LinkSuggestionAction
{
    bool available = false;
    const char* previewMessage = "";
    bool setWidth = false;
    float width = 0.0f;
    bool setDepth = false;
    float depth = 0.0f;
    bool setSidechainMode = false;
    int sidechainModeIndex = 0;
    bool requiresManualSidechain = false;
};

class LinkSuggestionEngine final
{
public:
    static LinkSuggestion evaluate(const LinkSuggestionInput& input) noexcept;
    static LinkSuggestionAction actionFor(LinkSuggestionKind kind) noexcept;
};
} // namespace stagemind
