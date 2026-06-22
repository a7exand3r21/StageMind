#include "LinkSuggestionEngine.h"

#include <algorithm>

namespace stagemind
{
namespace
{
bool isVocal(TrackRole role) noexcept
{
    return role == TrackRole::LeadVocal
        || role == TrackRole::BackingVocal
        || role == TrackRole::SunoVocal;
}

bool isBass(TrackRole role) noexcept
{
    return role == TrackRole::Bass
        || role == TrackRole::SynthBass
        || role == TrackRole::SunoBass;
}

bool isKickOrDrums(TrackRole role) noexcept
{
    return role == TrackRole::Kick
        || role == TrackRole::SunoDrums;
}

bool isSnare(TrackRole role) noexcept
{
    return role == TrackRole::Snare;
}

bool isDrumStem(TrackRole role) noexcept
{
    return role == TrackRole::SunoDrums;
}

bool isMelodicLead(TrackRole role) noexcept
{
    return role == TrackRole::LeadGuitar
        || role == TrackRole::SynthLead;
}

bool isPadLike(TrackRole role) noexcept
{
    return role == TrackRole::Pad
        || role == TrackRole::Atmosphere
        || role == TrackRole::SunoSynthPad
        || role == TrackRole::SunoInstrumental;
}

bool isWideBed(TrackRole role) noexcept
{
    return role == TrackRole::Pad
        || role == TrackRole::Atmosphere
        || role == TrackRole::FX
        || role == TrackRole::SunoInstrumental
        || role == TrackRole::SunoSynthPad
        || role == TrackRole::SunoFX
        || role == TrackRole::RhythmGuitarSingle
        || role == TrackRole::SunoGuitar;
}

bool isHarmonicInstrument(TrackRole role) noexcept
{
    return isWideBed(role)
        || role == TrackRole::Piano
        || role == TrackRole::RhythmGuitarPairLeft
        || role == TrackRole::RhythmGuitarPairRight
        || role == TrackRole::LeadGuitar
        || role == TrackRole::SynthLead;
}

float overlap(float currentBand, float peerBand) noexcept
{
    return std::min(std::clamp(currentBand, 0.0f, 1.0f), std::clamp(peerBand, 0.0f, 1.0f));
}

bool hasBandOverlap(float currentBand, float peerBand, float threshold = 0.08f) noexcept
{
    return overlap(currentBand, peerBand) >= threshold;
}

LinkSuggestion makeSuggestion(
    LinkSuggestionKind kind,
    float severity,
    const char* message,
    const char* reason,
    const char* actionLabel,
    const char* appliedMessage) noexcept
{
    LinkSuggestion suggestion;
    suggestion.kind = kind;
    suggestion.severity = severity;
    suggestion.message = message;
    suggestion.reason = reason;
    suggestion.actionLabel = actionLabel;
    suggestion.appliedMessage = appliedMessage;
    return suggestion;
}
} // namespace

LinkSuggestion LinkSuggestionEngine::evaluate(const LinkSuggestionInput& input) noexcept
{
    if (! input.linkActive || ! input.peerFound || input.peerActivity < 0.08f)
        return {};

    const auto currentWidth = std::clamp(input.currentWidth, 0.0f, 1.0f);
    const auto peerWidth = std::clamp(input.peerWidth, 0.0f, 1.0f);
    const auto peerActivity = std::clamp(input.peerActivity, 0.0f, 1.0f);
    const auto currentDepth = std::clamp(input.currentDepth, 0.0f, 1.0f);
    const auto currentCorrelation = std::clamp(input.currentCorrelation, -1.0f, 1.0f);
    const auto lowOverlap = overlap(input.currentBands.low, input.peerBands.low);
    const auto lowMidOverlap = overlap(input.currentBands.lowMid, input.peerBands.lowMid);
    const auto presenceOverlap = overlap(input.currentBands.presence, input.peerBands.presence);

    if (currentCorrelation < 0.10f && currentWidth > 0.62f)
    {
        return makeSuggestion(
            LinkSuggestionKind::StereoSafety,
            0.90f,
            "Conflict: stereo safety",
            "Current width and low correlation are risky. Reduce Width first.",
            "Reduce Width",
            "Applied: Width reduced");
    }

    if (isBass(input.currentRole) && isKickOrDrums(input.peerRole) && lowOverlap >= 0.08f)
    {
        return makeSuggestion(
            LinkSuggestionKind::KickBass,
            0.68f + lowOverlap * 0.25f,
            "Conflict: low overlap",
            "Both linked nodes are active in the low band. Let the kick/drum peer duck bass lows.",
            "Set Kick Duck",
            "Applied: Kick Duck");
    }

    if (isWideBed(input.currentRole)
        && isVocal(input.peerRole)
        && input.peerBands.presence >= 0.06f
        && (currentWidth > 0.62f || currentDepth < 0.35f))
    {
        return makeSuggestion(
            LinkSuggestionKind::VocalSpace,
            0.70f + input.peerBands.presence * 0.15f,
            "Conflict: vocal space",
            "The vocal peer has presence energy while this bed is wide or too close to the front.",
            "Make Room",
            "Applied: vocal room");
    }

    if (isHarmonicInstrument(input.currentRole) && isVocal(input.peerRole) && hasBandOverlap(input.currentBands.presence, input.peerBands.presence))
    {
        return makeSuggestion(
            LinkSuggestionKind::VocalInstrumentDucking,
            0.58f + presenceOverlap * 0.35f,
            "Conflict: presence overlap",
            "The vocal peer and this instrument both have presence-band energy.",
            "Set Vocal Duck",
            "Applied: Vocal Duck");
    }

    if (isHarmonicInstrument(input.currentRole) && isSnare(input.peerRole) && hasBandOverlap(input.currentBands.presence, input.peerBands.presence))
    {
        return makeSuggestion(
            LinkSuggestionKind::SnareInstrumentDucking,
            0.55f + presenceOverlap * 0.35f,
            "Conflict: snare bite",
            "The snare peer and this instrument overlap in the attack/presence band.",
            "Set Snare Duck",
            "Applied: Snare Duck");
    }

    const auto currentInstrumentBand = std::max(input.currentBands.lowMid, input.currentBands.presence);
    const auto drumStemOverlap = std::max(lowMidOverlap, presenceOverlap);
    const auto drumStemBroadMask = peerActivity >= 0.18f && currentInstrumentBand >= 0.06f;

    if (isHarmonicInstrument(input.currentRole)
        && isDrumStem(input.peerRole)
        && (drumStemOverlap >= 0.06f || drumStemBroadMask))
    {
        const auto drumOverlap = std::max(drumStemOverlap, std::min(currentInstrumentBand, peerActivity * 0.5f));
        return makeSuggestion(
            LinkSuggestionKind::DrumsInstrumentDucking,
            0.52f + drumOverlap * 0.32f,
            "Conflict: drums vs instrument",
            "The drum stem and this instrument overlap in low-mid or presence energy. Use a broad duck so drums push this track back.",
            "Set Broad Duck",
            "Applied: Broad Duck");
    }

    if (isPadLike(input.currentRole)
        && isMelodicLead(input.peerRole)
        && (hasBandOverlap(input.currentBands.lowMid, input.peerBands.lowMid)
            || hasBandOverlap(input.currentBands.presence, input.peerBands.presence)))
    {
        const auto leadOverlap = std::max(lowMidOverlap, presenceOverlap);
        return makeSuggestion(
            LinkSuggestionKind::LeadPadDucking,
            0.55f + leadOverlap * 0.35f,
            "Conflict: lead vs pad",
            "The lead peer and this pad overlap in low-mid or presence energy.",
            "Set Lead Duck",
            "Applied: Lead Duck");
    }

    if (isWideBed(input.currentRole) && isWideBed(input.peerRole) && currentWidth > 0.74f && peerWidth > 0.74f)
    {
        return makeSuggestion(
            LinkSuggestionKind::DoubleWide,
            0.55f + peerActivity * 0.25f,
            "Conflict: double-wide",
            "Both linked beds are wide. Reduce this instance before the mix loses center focus.",
            "Reduce Width",
            "Applied: width reduced");
    }

    return {};
}

LinkSuggestionAction LinkSuggestionEngine::actionFor(LinkSuggestionKind kind) noexcept
{
    LinkSuggestionAction action;

    switch (kind)
    {
        case LinkSuggestionKind::StereoSafety:
            action.available = true;
            action.previewMessage = "Width -> 55%";
            action.setWidth = true;
            action.width = 0.55f;
            break;

        case LinkSuggestionKind::KickBass:
            action.available = true;
            action.previewMessage = "Ducking -> Kick";
            action.setSidechainMode = true;
            action.sidechainModeIndex = 3;
            action.requiresManualSidechain = true;
            break;

        case LinkSuggestionKind::VocalInstrumentDucking:
            action.available = true;
            action.previewMessage = "Ducking -> Vocal";
            action.setSidechainMode = true;
            action.sidechainModeIndex = 2;
            action.requiresManualSidechain = true;
            break;

        case LinkSuggestionKind::SnareInstrumentDucking:
            action.available = true;
            action.previewMessage = "Ducking -> Snare";
            action.setSidechainMode = true;
            action.sidechainModeIndex = 4;
            action.requiresManualSidechain = true;
            break;

        case LinkSuggestionKind::LeadPadDucking:
            action.available = true;
            action.previewMessage = "Ducking -> Lead";
            action.setSidechainMode = true;
            action.sidechainModeIndex = 5;
            action.requiresManualSidechain = true;
            break;

        case LinkSuggestionKind::DrumsInstrumentDucking:
            action.available = true;
            action.previewMessage = "Ducking -> Broad";
            action.setSidechainMode = true;
            action.sidechainModeIndex = 1;
            action.requiresManualSidechain = true;
            break;

        case LinkSuggestionKind::VocalSpace:
            action.available = true;
            action.previewMessage = "Width 58%, Depth 48%";
            action.setWidth = true;
            action.width = 0.58f;
            action.setDepth = true;
            action.depth = 0.48f;
            break;

        case LinkSuggestionKind::DoubleWide:
            action.available = true;
            action.previewMessage = "Width -> 62%";
            action.setWidth = true;
            action.width = 0.62f;
            break;

        case LinkSuggestionKind::None:
        default:
            break;
    }

    return action;
}
} // namespace stagemind
