#pragma once

#include "../DSP/CorrelationMeter.h"
#include "../DSP/DepthProcessor.h"
#include "../DSP/DynamicEQ.h"
#include "../DSP/LinkSpectralAnalyzer.h"
#include "../DSP/MeterSnapshot.h"
#include "../DSP/MotionProcessor.h"
#include "../DSP/PseudoDoubleProcessor.h"
#include "../DSP/ResonanceDetector.h"
#include "../DSP/ResonanceLearner.h"
#include "../DSP/SidechainDetector.h"
#include "../DSP/SidechainDynamicEQ.h"
#include "../DSP/SpatialProcessor.h"
#include "../Link/LinkActivityEnvelope.h"
#include "../Link/StageMindLinkRegistry.h"
#include "../Model/Parameters.h"
#include "../Model/PluginState.h"
#include "../Model/RolePresetEngine.h"
#include <JuceHeader.h>

namespace stagemind
{
enum class LinkSuggestionKind;
struct LinkSuggestionAction;

class PluginProcessor final : public juce::AudioProcessor, private juce::Timer
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState() noexcept { return apvts; }
    const MeterSnapshot& getMeters() const noexcept { return meters; }
    std::uint32_t getLinkInstanceId() const noexcept { return linkHandle.instanceId; }
    void beginResonanceLearn() noexcept;
    void beginRideMemoryLearn() noexcept;
    void clearRideMemory() noexcept;
    RideMemorySnapshot getRideMemorySnapshot() const noexcept;

private:
    static BusesProperties createBuses();
    static std::pair<float, float> calculateRmsAndPeak(const juce::AudioBuffer<float>& buffer) noexcept;

    void timerCallback() override;
    float rawValue(const char* parameterId) const noexcept;
    void updateDirectorAutoCommands();
    void applyLinkCommand(LinkCommand command);
    void applyPendingAutoAssist();
    void applyAutoLinkAction(LinkSuggestionKind kind, std::uint32_t sourceInstanceId);
    void applyParameterValueFromCommand(const char* parameterId, float value);
    bool isLinkActionAppliedForAuto(const LinkSuggestionAction& action) const noexcept;
    UserMacroParams makeMacroParams() const noexcept;
    void updateOutputGainTarget() noexcept;
    void applyOutputGain(juce::AudioBuffer<float>& buffer) noexcept;
    void renderSidechainListen(const juce::AudioBuffer<float>& sidechainBuffer, juce::AudioBuffer<float>& mainBuffer) noexcept;
    SidechainAnalysis makeEffectiveSidechainAnalysis(const SidechainAnalysis& externalSidechain) const noexcept;
    SidechainDynamicEQConfig makeSidechainEQConfig() const noexcept;
    ResonanceDetectorConfig makeResonanceDetectorConfig() const noexcept;
    ResonanceSuppressionConfig makeResonanceSuppressionConfig() const noexcept;
    MotionConfig makeMotionConfig(const RoleProfile& profile, SafetyMode safety) const noexcept;
    DepthConfig makeDepthConfig(const RoleProfile& profile, SafetyMode safety) const noexcept;
    PseudoDoubleConfig makePseudoDoubleConfig(const RoleProfile& profile, SafetyMode safety) const noexcept;
    void applyCorrelationSafety(SpatialParams& params) const noexcept;
    void publishStageState(
        const SpatialParams& spatialParams,
        const DepthConfig& depthConfig,
        const MotionConfig& motionConfig,
        bool motionAllowed) noexcept;
    void updateCorrelationSafety(float correlation, float threshold) noexcept;
    void publishLinkState(
        TrackRole role,
        std::pair<float, float> inputLevels,
        std::pair<float, float> outputLevels,
        float correlation,
        float sidechainEnvelope,
        LinkSpectralBands bands,
        float effectivePan,
        int numSamples) noexcept;
    void publishIdleLinkState() noexcept;
    void publishDisabledLinkState() noexcept;
    bool isDirectorMode() const noexcept;
    void updateAutoLinkAssist(
        TrackRole role,
        float correlation,
        LinkSpectralBands bands,
        const LinkGroupSnapshot& groupSnapshot,
        TrackRole preferredSourceRole,
        bool canReadGroup,
        int numSamples) noexcept;
    void updateAutoAssistBeforeResonance(TrackRole role, std::pair<float, float> inputLevels, int numSamples) noexcept;
    void publishAutoAssistState(int state, float progress) noexcept;
    void resetAutoAssistTracking() noexcept;
    void resetAutoLinkTracking() noexcept;
    void applyFactoryPreset(int index);
    void publishResonanceLearnState() noexcept;
    void publishResonances(const ResonanceSnapshot& snapshot) noexcept;
    void publishLearnedResonances(const ResonanceSnapshot& snapshot) noexcept;
    ResonanceSnapshot readLearnedResonanceSnapshot() const noexcept;
    juce::AudioBuffer<float> getOptionalSidechainBuffer(juce::AudioBuffer<float>& buffer) noexcept;

    juce::AudioProcessorValueTreeState apvts;
    LinkInstanceHandle linkHandle;

    RolePresetEngine rolePresetEngine;
    SpatialProcessor spatialProcessor;
    MotionProcessor motionProcessor;
    DepthProcessor depthProcessor;
    PseudoDoubleProcessor pseudoDoubleProcessor;
    CorrelationMeter correlationMeter;
    ResonanceDetector resonanceDetector;
    ResonanceLearner resonanceLearner;
    DynamicEQ dynamicEQ;
    SidechainDetector sidechainDetector;
    SidechainDynamicEQ sidechainDynamicEQ;
    MeterSnapshot meters;
    LinkActivityEnvelope linkActivityEnvelope;
    LinkSpectralAnalyzer linkSpectralAnalyzer;
    mutable juce::CriticalSection rideMemoryLock;
    RideMemory rideMemory;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outputGain { 1.0f };
    std::atomic<bool> resonanceLearnRequested { false };
    std::atomic<bool> autoResonanceTuningPending { false };
    std::atomic<int> autoPendingLinkActionKind { 0 };
    std::atomic<int> autoPendingLinkSourceId { 0 };
    std::atomic<int> linkAudioPublishFreshTicks { 0 };
    std::atomic<int> autoLinkSidechainSourceId { 0 };
    std::atomic<float> autoLinkSidechainActivity { 0.0f };
    float correlationSafetyScale = 1.0f;
    double currentSampleRate = 44100.0;
    int autoStableSamples = 0;
    int autoLinkCandidateKind = 0;
    int autoLinkCandidateSourceId = 0;
    int autoLinkCandidateSamples = 0;
    int directorAutoCommandCooldownTicks = 0;
    int autoTrackedGroup = 0;
    int autoTrackedRole = 0;
    bool autoResonanceStarted = false;
    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};
} // namespace stagemind
