#include "Parameters.h"
#include "AutoAssistMode.h"
#include "PluginMode.h"
#include "QualityMode.h"
#include "SafetyMode.h"
#include "SidechainConflictMode.h"
#include "SidechainListenMode.h"
#include "TrackRole.h"
#include "TriggerMode.h"

namespace stagemind::parameters
{
namespace
{
using ParameterLayout = juce::AudioProcessorValueTreeState::ParameterLayout;

juce::ParameterID id(const char* value)
{
    return { value, 1 };
}

std::unique_ptr<juce::AudioParameterFloat> makeFloat(
    const char* parameterId,
    const juce::String& name,
    juce::NormalisableRange<float> range,
    float defaultValue,
    const juce::String& label = {})
{
    return std::make_unique<juce::AudioParameterFloat>(
        id(parameterId),
        name,
        range,
        defaultValue,
        juce::AudioParameterFloatAttributes().withLabel(label));
}

std::unique_ptr<juce::AudioParameterChoice> makeChoice(
    const char* parameterId,
    const juce::String& name,
    const juce::StringArray& choices,
    int defaultIndex)
{
    return std::make_unique<juce::AudioParameterChoice>(
        id(parameterId),
        name,
        choices,
        defaultIndex);
}

std::unique_ptr<juce::AudioParameterBool> makeBool(
    const char* parameterId,
    const juce::String& name,
    bool defaultValue)
{
    return std::make_unique<juce::AudioParameterBool>(
        id(parameterId),
        name,
        defaultValue);
}

std::unique_ptr<juce::AudioParameterInt> makeInt(
    const char* parameterId,
    const juce::String& name,
    int minValue,
    int maxValue,
    int defaultValue)
{
    return std::make_unique<juce::AudioParameterInt>(
        id(parameterId),
        name,
        minValue,
        maxValue,
        defaultValue);
}

juce::NormalisableRange<float> linear(float minValue, float maxValue)
{
    return { minValue, maxValue };
}

juce::NormalisableRange<float> skewed(float minValue, float maxValue, float skew)
{
    juce::NormalisableRange<float> range { minValue, maxValue };
    range.setSkewForCentre(skew);
    return range;
}
} // namespace

ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.reserve(40);

    params.push_back(makeChoice(ids::role, "Role", makeSelectableRoleNames(), selectableIndexForRole(TrackRole::SunoInstrumental)));
    params.push_back(makeFloat(ids::width, "Width", linear(0.0f, 1.0f), 0.50f));
    params.push_back(makeFloat(ids::depth, "Depth", linear(0.0f, 1.0f), 0.30f));
    params.push_back(makeFloat(ids::motion, "Motion", linear(0.0f, 1.0f), 0.00f));
    params.push_back(makeFloat(ids::cleanUp, "Clean Up", linear(0.0f, 1.0f), 0.30f));
    params.push_back(makeFloat(ids::resonance, "Resonance", linear(0.0f, 1.0f), 0.30f));
    params.push_back(makeChoice(ids::safety, "Safety", makeSafetyModeNames(), 1));
    params.push_back(makeFloat(ids::outputGain, "Output", linear(-24.0f, 12.0f), 0.0f, "dB"));

    params.push_back(makeChoice(ids::triggerMode, "Trigger", makeTriggerModeNames(), 0));
    params.push_back(makeBool(ids::sidechainEnabled, "SC Enable", false));
    params.push_back(makeChoice(ids::sidechainMode, "SC Mode", makeSidechainConflictModeNames(), 0));
    params.push_back(makeChoice(ids::sidechainSourceRole, "SC Source", makeSelectableRoleNames(), selectableIndexForRole(TrackRole::LeadVocal)));
    params.push_back(makeFloat(ids::sidechainAmount, "SC Amount", linear(0.0f, 1.0f), 0.30f));
    params.push_back(makeChoice(ids::sidechainListen, "SC Listen", makeSidechainListenModeNames(), 0));
    params.push_back(makeFloat(ids::sidechainAttack, "SC Attack", skewed(1.0f, 200.0f, 50.0f), 50.0f, "ms"));
    params.push_back(makeFloat(ids::sidechainRelease, "SC Release", skewed(20.0f, 1000.0f, 250.0f), 250.0f, "ms"));
    params.push_back(makeFloat(ids::sidechainRangeStart, "SC Start", skewed(20.0f, 20000.0f, 1000.0f), 2000.0f, "Hz"));
    params.push_back(makeFloat(ids::sidechainRangeEnd, "SC End", skewed(20.0f, 20000.0f, 4000.0f), 5000.0f, "Hz"));

    params.push_back(makeFloat(ids::pan, "Pan", linear(-1.0f, 1.0f), 0.0f));
    params.push_back(makeFloat(ids::monoLowCutoff, "Mono Low", skewed(40.0f, 300.0f, 160.0f), 160.0f, "Hz"));
    params.push_back(makeFloat(ids::sideHighPass, "Side HP", skewed(40.0f, 500.0f, 220.0f), 220.0f, "Hz"));
    params.push_back(makeFloat(ids::correlationSafetyThreshold, "Corr Safe", linear(-1.0f, 1.0f), 0.10f));
    params.push_back(makeFloat(ids::resonanceSensitivity, "Res Sens", linear(0.0f, 1.0f), 0.50f));
    params.push_back(makeFloat(ids::maxResonanceReduction, "Max Res Cut", linear(0.0f, 7.0f), 4.0f, "dB"));
    params.push_back(makeFloat(ids::dynamicEqAttack, "Dyn Attack", skewed(1.0f, 200.0f, 30.0f), 30.0f, "ms"));
    params.push_back(makeFloat(ids::dynamicEqRelease, "Dyn Release", skewed(20.0f, 1000.0f, 250.0f), 250.0f, "ms"));
    params.push_back(makeFloat(ids::motionRate, "Motion Rate", skewed(0.01f, 8.0f, 0.20f), 0.20f, "Hz"));
    params.push_back(makeFloat(ids::pseudoDoubleAmount, "Double", linear(0.0f, 1.0f), 0.0f));
    params.push_back(makeFloat(ids::presenceReduction, "Presence", linear(0.0f, 1.0f), 0.0f));
    params.push_back(makeFloat(ids::earlyReflectionAmount, "Early Ref", linear(0.0f, 1.0f), 0.0f));
    params.push_back(makeChoice(ids::analyzerQuality, "Analyzer", makeQualityNames(), 1));
    params.push_back(makeChoice(ids::processingQuality, "Quality", makeProcessingQualityNames(), 1));
    params.push_back(makeBool(ids::linkEnabled, "Link Enable", true));
    params.push_back(makeInt(ids::linkGroup, "Link Group", 0, 16, 1));
    params.push_back(makeChoice(ids::linkRole, "Link Role", makeRoleNamesWithUnknown(), 0));
    params.push_back(makeInt(ids::linkSourceId, "Link Source", 0, 128, 0));
    params.push_back(makeInt(ids::linkTargetId, "Link Target", 0, 128, 0));
    params.push_back(makeInt(ids::linkMode, "Link Mode", 0, 16, 0));
    params.push_back(makeChoice(ids::pluginMode, "Mode", makePluginModeNames(), 0));
    params.push_back(makeChoice(ids::autoAssistMode, "Auto Assist", makeAutoAssistModeNames(), 2));

    return { params.begin(), params.end() };
}
} // namespace stagemind::parameters
