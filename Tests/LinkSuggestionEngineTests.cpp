#include "../Source/Link/LinkSuggestionEngine.h"

#include <cstdlib>
#include <iostream>

namespace
{
void expect(bool condition, const char* message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

stagemind::LinkSuggestionInput activeInput()
{
    stagemind::LinkSuggestionInput input;
    input.linkActive = true;
    input.peerFound = true;
    input.currentRole = stagemind::TrackRole::SunoSynthPad;
    input.peerRole = stagemind::TrackRole::LeadVocal;
    input.currentWidth = 0.75f;
    input.currentDepth = 0.25f;
    input.currentCorrelation = 0.8f;
    input.peerActivity = 0.6f;
    input.peerWidth = 0.5f;
    input.currentBands.low = 0.18f;
    input.currentBands.lowMid = 0.22f;
    input.currentBands.presence = 0.24f;
    input.currentBands.air = 0.04f;
    input.peerBands.low = 0.18f;
    input.peerBands.lowMid = 0.20f;
    input.peerBands.presence = 0.22f;
    input.peerBands.air = 0.04f;
    return input;
}

void inactiveLinkHasNoSuggestion()
{
    auto input = activeInput();
    input.linkActive = false;

    const auto suggestion = stagemind::LinkSuggestionEngine::evaluate(input);
    expect(! suggestion.hasSuggestion(), "inactive link should not produce suggestions");
}

void missingPeerHasNoSuggestion()
{
    auto input = activeInput();
    input.peerFound = false;

    const auto suggestion = stagemind::LinkSuggestionEngine::evaluate(input);
    expect(! suggestion.hasSuggestion(), "missing peer should not produce suggestions");
}

void lowCorrelationSuggestsStereoSafety()
{
    auto input = activeInput();
    input.currentCorrelation = 0.02f;

    const auto suggestion = stagemind::LinkSuggestionEngine::evaluate(input);
    expect(suggestion.kind == stagemind::LinkSuggestionKind::StereoSafety, "low correlation should suggest stereo safety");
    expect(suggestion.message[0] != '\0' && suggestion.reason[0] != '\0', "stereo safety should explain the conflict");
    expect(suggestion.actionLabel[0] != '\0' && suggestion.appliedMessage[0] != '\0', "stereo safety should expose UI labels");

    const auto action = stagemind::LinkSuggestionEngine::actionFor(suggestion.kind);
    expect(action.available && action.setWidth && action.width < input.currentWidth, "stereo safety action should reduce width");
    expect(action.previewMessage[0] != '\0', "stereo safety action should expose a preview message");
}

void bassAgainstKickSuggestsSidechainMode()
{
    auto input = activeInput();
    input.currentRole = stagemind::TrackRole::SunoBass;
    input.peerRole = stagemind::TrackRole::Kick;
    input.currentWidth = 0.25f;
    input.currentCorrelation = 0.8f;

    const auto suggestion = stagemind::LinkSuggestionEngine::evaluate(input);
    expect(suggestion.kind == stagemind::LinkSuggestionKind::KickBass, "bass against kick should suggest Kick Ducks Bass");

    const auto action = stagemind::LinkSuggestionEngine::actionFor(suggestion.kind);
    expect(action.available && action.setSidechainMode && action.sidechainModeIndex == 3, "kick bass action should select Kick Ducks Bass");
    expect(action.requiresManualSidechain, "kick bass action should keep sidechain routing explicit");
    expect(action.previewMessage[0] != '\0', "kick bass action should expose a preview message");
}

void bassAgainstKickNeedsLowBandOverlap()
{
    auto input = activeInput();
    input.currentRole = stagemind::TrackRole::SunoBass;
    input.peerRole = stagemind::TrackRole::Kick;
    input.currentWidth = 0.25f;
    input.currentCorrelation = 0.8f;
    input.currentBands.low = 0.02f;
    input.peerBands.low = 0.03f;

    const auto suggestion = stagemind::LinkSuggestionEngine::evaluate(input);
    expect(! suggestion.hasSuggestion(), "bass/kick link should wait for low-band overlap before suggesting Kick Duck");
}

void wideBedAgainstVocalSuggestsSpace()
{
    const auto suggestion = stagemind::LinkSuggestionEngine::evaluate(activeInput());
    expect(suggestion.kind == stagemind::LinkSuggestionKind::VocalSpace, "wide bed against vocal should suggest making room");

    const auto action = stagemind::LinkSuggestionEngine::actionFor(suggestion.kind);
    expect(action.available && action.setWidth && action.setDepth, "vocal space action should set width and depth");
}

void settledInstrumentAgainstVocalSuggestsDuckingMode()
{
    auto input = activeInput();
    input.currentRole = stagemind::TrackRole::RhythmGuitarSingle;
    input.peerRole = stagemind::TrackRole::LeadVocal;
    input.currentWidth = 0.45f;
    input.currentDepth = 0.55f;

    const auto suggestion = stagemind::LinkSuggestionEngine::evaluate(input);
    expect(
        suggestion.kind == stagemind::LinkSuggestionKind::VocalInstrumentDucking,
        "settled instrument against vocal should suggest Vocal Ducks Instrument");

    const auto action = stagemind::LinkSuggestionEngine::actionFor(suggestion.kind);
    expect(
        action.available && action.setSidechainMode && action.sidechainModeIndex == 2,
        "vocal instrument action should select Vocal Ducks Instrument");
}

void instrumentAgainstSnareSuggestsSnareDuckingMode()
{
    auto input = activeInput();
    input.currentRole = stagemind::TrackRole::SunoGuitar;
    input.peerRole = stagemind::TrackRole::Snare;
    input.currentWidth = 0.45f;
    input.currentDepth = 0.55f;

    const auto suggestion = stagemind::LinkSuggestionEngine::evaluate(input);
    expect(
        suggestion.kind == stagemind::LinkSuggestionKind::SnareInstrumentDucking,
        "instrument against snare should suggest Snare Ducks Instrument");

    const auto action = stagemind::LinkSuggestionEngine::actionFor(suggestion.kind);
    expect(
        action.available && action.setSidechainMode && action.sidechainModeIndex == 4,
        "snare instrument action should select Snare Ducks Instrument");
}

void instrumentAgainstSunoDrumsSuggestsMakeSpaceMode()
{
    auto input = activeInput();
    input.currentRole = stagemind::TrackRole::SunoGuitar;
    input.peerRole = stagemind::TrackRole::SunoDrums;
    input.currentWidth = 0.45f;
    input.currentDepth = 0.55f;
    input.currentBands.lowMid = 0.11f;
    input.currentBands.presence = 0.08f;
    input.peerBands.lowMid = 0.02f;
    input.peerBands.presence = 0.03f;
    input.peerActivity = 0.48f;

    const auto suggestion = stagemind::LinkSuggestionEngine::evaluate(input);
    expect(
        suggestion.kind == stagemind::LinkSuggestionKind::DrumsInstrumentDucking,
        "instrument against Suno Drums should suggest broad Make Space ducking");

    const auto action = stagemind::LinkSuggestionEngine::actionFor(suggestion.kind);
    expect(
        action.available && action.setSidechainMode && action.sidechainModeIndex == 1,
        "drums instrument action should select Make Space");
}

void padAgainstLeadSuggestsLeadPadDuckingMode()
{
    auto input = activeInput();
    input.currentRole = stagemind::TrackRole::SunoSynthPad;
    input.peerRole = stagemind::TrackRole::LeadGuitar;
    input.currentWidth = 0.45f;
    input.currentDepth = 0.55f;

    const auto suggestion = stagemind::LinkSuggestionEngine::evaluate(input);
    expect(suggestion.kind == stagemind::LinkSuggestionKind::LeadPadDucking, "pad against lead should suggest Lead Ducks Pad");

    const auto action = stagemind::LinkSuggestionEngine::actionFor(suggestion.kind);
    expect(
        action.available && action.setSidechainMode && action.sidechainModeIndex == 5,
        "lead pad action should select Lead Ducks Pad");
}

void doubleWidePeersSuggestWidthCheck()
{
    auto input = activeInput();
    input.currentRole = stagemind::TrackRole::Pad;
    input.peerRole = stagemind::TrackRole::SunoSynthPad;
    input.currentWidth = 0.9f;
    input.peerWidth = 0.9f;
    input.currentDepth = 0.7f;

    const auto suggestion = stagemind::LinkSuggestionEngine::evaluate(input);
    expect(suggestion.kind == stagemind::LinkSuggestionKind::DoubleWide, "two wide beds should suggest avoiding a double-wide stack");

    const auto action = stagemind::LinkSuggestionEngine::actionFor(suggestion.kind);
    expect(action.available && action.setWidth && action.width < input.currentWidth, "double-wide action should reduce width");
}
} // namespace

void runLinkSuggestionEngineTests()
{
    inactiveLinkHasNoSuggestion();
    missingPeerHasNoSuggestion();
    lowCorrelationSuggestsStereoSafety();
    bassAgainstKickSuggestsSidechainMode();
    bassAgainstKickNeedsLowBandOverlap();
    wideBedAgainstVocalSuggestsSpace();
    settledInstrumentAgainstVocalSuggestsDuckingMode();
    instrumentAgainstSnareSuggestsSnareDuckingMode();
    instrumentAgainstSunoDrumsSuggestsMakeSpaceMode();
    padAgainstLeadSuggestsLeadPadDuckingMode();
    doubleWidePeersSuggestWidthCheck();
}
