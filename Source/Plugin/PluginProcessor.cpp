#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../Link/LinkSuggestionEngine.h"
#include "../Model/AutoAssistMode.h"
#include "../Model/MotionPreset.h"
#include "../Model/PluginMode.h"
#include "../Model/RoleBalanceModel.h"
#include "../Model/SidechainListenMode.h"
#include "../Model/StageGainMeterMode.h"
#include "../Model/StageGainMode.h"
#include "../Model/TriggerMode.h"

#include <algorithm>
#include <cmath>

namespace stagemind
{
namespace
{
constexpr float autoAnalyzeSignalThreshold = 0.003f;
constexpr double autoAnalyzeStableSeconds = 2.0;
constexpr double autoLinkStableSeconds = 0.45;
constexpr float autoParameterEpsilon = 0.0005f;
constexpr float autoSidechainDefaultAmount = 0.62f;
constexpr int directorAutoCommandCooldownDefaultTicks = 6;
constexpr auto editorWidthProperty = "editor_width";
constexpr auto editorHeightProperty = "editor_height";
constexpr int editorDefaultWidth = 1280;
constexpr int editorDefaultHeight = 760;
constexpr int editorMinWidth = 1040;
constexpr int editorMinHeight = 640;
constexpr int editorMaxWidth = 1600;
constexpr int editorMaxHeight = 980;
constexpr int diagnosticSnapshotIntervalTicks = 20;

struct FactoryPreset
{
    const char* name = "";
    TrackRole role = TrackRole::SunoInstrumental;
    SafetyMode safety = SafetyMode::Natural;
    float width = 0.5f;
    float depth = 0.3f;
    float motion = 0.0f;
    float cleanUp = 0.3f;
    float resonance = 0.3f;
    float pseudoDouble = 0.0f;
    float presence = 0.0f;
    float earlyRef = 0.0f;
};

constexpr std::array<FactoryPreset, 8> factoryPresets {{
    { "Vocal - Forward Clean", TrackRole::LeadVocal, SafetyMode::Natural, 0.24f, 0.08f, 0.00f, 0.76f, 0.58f, 0.22f, 0.04f, 0.02f },
    { "Backing Vocal - Wide Doubler", TrackRole::BackingVocal, SafetyMode::ModernWide, 0.90f, 0.50f, 0.08f, 0.48f, 0.38f, 0.82f, 0.08f, 0.16f },
    { "Guitar - Big Stereo", TrackRole::RhythmGuitarSingle, SafetyMode::ModernWide, 0.88f, 0.34f, 0.05f, 0.50f, 0.42f, 0.86f, 0.06f, 0.10f },
    { "Bass - Tight Duck Ready", TrackRole::Bass, SafetyMode::MonoSafe, 0.06f, 0.06f, 0.00f, 0.52f, 0.38f, 0.00f, 0.00f, 0.00f },
    { "Pad - Far Halo", TrackRole::Pad, SafetyMode::HeadphonesWide, 0.96f, 0.94f, 0.30f, 0.30f, 0.24f, 0.58f, 0.28f, 0.52f },
    { "FX - Wide Sweep", TrackRole::FX, SafetyMode::HeadphonesWide, 1.00f, 0.86f, 0.70f, 0.24f, 0.22f, 0.70f, 0.20f, 0.42f },
    { "Suno - Stem Repair", TrackRole::SunoInstrumental, SafetyMode::Natural, 0.46f, 0.30f, 0.00f, 0.88f, 0.78f, 0.00f, 0.14f, 0.08f },
    { "Suno - Synth Pad Huge", TrackRole::SunoSynthPad, SafetyMode::HeadphonesWide, 0.98f, 0.88f, 0.26f, 0.34f, 0.28f, 0.62f, 0.20f, 0.42f }
}};

float safetyMotionScale(SafetyMode safety) noexcept
{
    switch (safety)
    {
        case SafetyMode::MonoSafe:       return 0.35f;
        case SafetyMode::ModernWide:     return 1.10f;
        case SafetyMode::HeadphonesWide: return 1.15f;
        case SafetyMode::Natural:
        default:                         return 1.0f;
    }
}

float safetyDoubleScale(SafetyMode safety) noexcept
{
    switch (safety)
    {
        case SafetyMode::MonoSafe:       return 0.0f;
        case SafetyMode::ModernWide:     return 1.10f;
        case SafetyMode::HeadphonesWide: return 1.15f;
        case SafetyMode::Natural:
        default:                         return 1.0f;
    }
}

float safetyDepthScale(SafetyMode safety) noexcept
{
    switch (safety)
    {
        case SafetyMode::MonoSafe:       return 0.55f;
        case SafetyMode::ModernWide:     return 1.05f;
        case SafetyMode::HeadphonesWide: return 1.10f;
        case SafetyMode::Natural:
        default:                         return 1.0f;
    }
}

float normalizedWidthAmount(float widthAmount) noexcept
{
    return juce::jlimit(0.0f, 1.0f, juce::jmap(widthAmount, 0.25f, 1.80f, 0.0f, 1.0f));
}

void setParameterValue(juce::AudioProcessorValueTreeState& apvts, const char* id, float value)
{
    if (auto* parameter = apvts.getParameter(id))
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
}

bool targetAllowsReader(const LinkPeerSnapshot& peer, std::uint32_t readerId) noexcept
{
    return peer.targetId == 0 || static_cast<std::uint32_t> (peer.targetId) == readerId;
}

bool isDuckingAction(const LinkSuggestionAction& action) noexcept
{
    return action.setSidechainMode && action.sidechainModeIndex > 0;
}

bool isActiveDuckingState(int sidechainMode, int triggerMode, bool sidechainEnabled, float sidechainAmount) noexcept
{
    const auto trigger = triggerModeFromIndex(triggerMode);
    return sidechainMode > 0
        && sidechainEnabled
        && sidechainAmount >= 0.35f
        && (trigger == TriggerMode::ExternalSidechain || trigger == TriggerMode::StageMindLink);
}

bool isManualDuckingBypassState(int sidechainMode, int triggerMode, bool sidechainEnabled) noexcept
{
    const auto trigger = triggerModeFromIndex(triggerMode);
    return ! sidechainEnabled
        && (sidechainMode > 0 || trigger == TriggerMode::StageMindLink || trigger == TriggerMode::ExternalSidechain);
}

bool isSnapshotActionApplied(const LinkPeerSnapshot& target, const LinkSuggestionAction& action) noexcept
{
    if (! action.available)
        return false;

    if (action.setWidth && std::abs(target.width - action.width) > autoParameterEpsilon)
        return false;

    if (action.setDepth && std::abs(target.depth - action.depth) > autoParameterEpsilon)
        return false;

    if (action.setSidechainMode)
    {
        if (! isDuckingAction(action))
        {
            if (target.sidechainMode != action.sidechainModeIndex)
                return false;
        }
        else if (isManualDuckingBypassState(target.sidechainMode, target.triggerMode, target.sidechainEnabled))
        {
            return true;
        }
        else if (! isActiveDuckingState(
            target.sidechainMode,
            target.triggerMode,
            target.sidechainEnabled,
            target.sidechainAmount))
        {
            return false;
        }
    }

    return true;
}

float autoSuggestionScore(LinkSuggestionKind kind, float severity) noexcept
{
    auto priority = 0.0f;
    switch (kind)
    {
        case LinkSuggestionKind::StereoSafety:             priority = 0.30f; break;
        case LinkSuggestionKind::KickBass:                 priority = 0.24f; break;
        case LinkSuggestionKind::VocalInstrumentDucking:   priority = 0.20f; break;
        case LinkSuggestionKind::DrumsInstrumentDucking:   priority = 0.18f; break;
        case LinkSuggestionKind::SnareInstrumentDucking:   priority = 0.16f; break;
        case LinkSuggestionKind::LeadPadDucking:           priority = 0.14f; break;
        case LinkSuggestionKind::VocalSpace:               priority = 0.10f; break;
        case LinkSuggestionKind::DoubleWide:               priority = 0.08f; break;
        case LinkSuggestionKind::None:
        default:                                           priority = 0.0f; break;
    }

    return severity + priority;
}

RoleBalanceInput makeRoleBalanceInput(const LinkPeerSnapshot& node, int index) noexcept
{
    RoleBalanceInput input;
    input.index = index;
    input.role = static_cast<TrackRole> (node.role);
    input.outputRms = node.outputRms;
    input.outputTrimDb = node.outputTrimDb;
    input.activity = node.activity;
    input.autoEnabled = autoAssistModeFromIndex(node.autoAssistMode) == AutoAssistMode::Auto;
    input.stageGainDb = node.stageGainDb;
    input.stageGainMode = node.stageGainMode;
    return input;
}

bool isActiveBalanceNode(const LinkPeerSnapshot& node) noexcept
{
    return node.found && roleBalanceInputIsActive(makeRoleBalanceInput(node, 0));
}

int countActiveBalanceNodes(const LinkGroupSnapshot& snapshot) noexcept
{
    auto result = 0;
    for (int index = 0; index < snapshot.count; ++index)
        if (isActiveBalanceNode(snapshot.peers[static_cast<size_t> (index)]))
            ++result;

    return result;
}

struct DirectorBalanceCorrection
{
    bool found = false;
    LinkPeerSnapshot target;
    float nextOutputTrimDb = 0.0f;
    float correctionDb = 0.0f;
    float deviationDb = 0.0f;
    float severity = 0.0f;
};

DirectorBalanceCorrection findDirectorBalanceCorrection(const LinkGroupSnapshot& snapshot) noexcept
{
    std::array<RoleBalanceInput, maxLinkInstances> inputs {};
    auto inputCount = 0;

    for (int index = 0; index < snapshot.count; ++index)
    {
        const auto& node = snapshot.peers[static_cast<size_t> (index)];
        if (! node.found)
            continue;

        inputs[static_cast<size_t> (inputCount)] = makeRoleBalanceInput(node, index);
        ++inputCount;
    }

    const auto decision = chooseRoleBalanceDecision(inputs.data(), inputCount);
    if (! decision.found || decision.index < 0 || decision.index >= snapshot.count)
        return {};

    DirectorBalanceCorrection best;
    best.found = true;
    best.target = snapshot.peers[static_cast<size_t> (decision.index)];
    best.nextOutputTrimDb = decision.nextOutputTrimDb;
    best.correctionDb = decision.correctionDb;
    best.deviationDb = decision.deviationDb;
    best.severity = decision.severity;
    return best;
}

bool balanceTimelineHasPendingEvent(
    const BalanceTimelineSnapshot& memory,
    int group,
    int targetRole,
    double ppqPosition) noexcept
{
    if (group <= 0 || targetRole <= 0 || ! std::isfinite(ppqPosition))
        return false;

    for (const auto& event : memory.events)
        if (event.used
            && ! event.resolved
            && event.group == group
            && event.targetRole == targetRole
            && event.contains(ppqPosition, balanceTimelineMergeWindowPpq))
        {
            return true;
        }

    return false;
}

int strongestRideMemoryBand(LinkSpectralBands currentBands, LinkSpectralBands peerBands) noexcept
{
    const auto low = std::min(currentBands.low, peerBands.low);
    const auto lowMid = std::min(currentBands.lowMid, peerBands.lowMid);
    const auto presence = std::min(currentBands.presence, peerBands.presence);
    const auto air = std::min(currentBands.air, peerBands.air);

    auto band = RideMemoryBand::Unknown;
    auto value = 0.0f;

    if (low > value)
    {
        band = RideMemoryBand::Low;
        value = low;
    }

    if (lowMid > value)
    {
        band = RideMemoryBand::LowMid;
        value = lowMid;
    }

    if (presence > value)
    {
        band = RideMemoryBand::Presence;
        value = presence;
    }

    if (air > value)
        band = RideMemoryBand::Air;

    return static_cast<int> (band);
}

bool rolesMatchForRideMemory(const LinkPeerSnapshot& node, int role) noexcept
{
    return node.found && role > 0 && node.role == role;
}

juce::CriticalSection& diagnosticLogLock() noexcept
{
    static juce::CriticalSection lock;
    return lock;
}

juce::File diagnosticLogFile()
{
    static const auto file = []()
    {
        const auto timestamp = juce::Time::getCurrentTime().formatted("%Y%m%d-%H%M%S");
        return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
            .getChildFile("StageMind")
            .getChildFile("Logs")
            .getChildFile("StageMind-session-" + timestamp + ".csv");
    }();

    return file;
}

juce::String csvEscape(juce::String text)
{
    text = text.replace("\"", "\"\"");
    return "\"" + text + "\"";
}

juce::String dbFromGain(float gain)
{
    return juce::String(juce::Decibels::gainToDecibels(juce::jmax(1.0e-6f, gain)), 2);
}

void appendDiagnosticLogLine(const juce::String& line)
{
    const juce::ScopedLock lock(diagnosticLogLock());
    const auto file = diagnosticLogFile();
    const auto parent = file.getParentDirectory();
    parent.createDirectory();

    const auto needsHeader = ! file.existsAsFile() || file.getSize() == 0;
    juce::FileOutputStream stream(file);
    if (! stream.openedOk())
        return;

    stream.setPosition(file.getSize());
    if (needsHeader)
    {
        stream << "time,event,instance,mode,role,group,auto_mode,stage_mode,stage_meter,target_db,threshold_vu,ceiling_db,response,input_rms_db,output_rms_db,output_peak_db,correlation,gr_db,res_db,stage_gain_db,stage_target_db,stage_out_peak_db,link_nodes,active_peers,peer_id,peer_role,auto_state,auto_action,director_balance_active,director_balance_role,director_deviation_db,director_correction_db,playing,ppq,bpm,note\n";
    }

    stream << line << "\n";
    stream.flush();
}
} // namespace

PluginProcessor::PluginProcessor()
    : AudioProcessor(createBuses()),
      apvts(*this, nullptr, "StageMindState", parameters::createParameterLayout()),
      linkHandle(StageMindLinkRegistry::instance().registerInstance())
{
    setLatencySamples(0);
    publishIdleLinkState();
    startTimerHz(20);
    writeDiagnosticEvent(
        "instance_start",
        "version=" + juce::String(JucePlugin_VersionString) + " log=" + diagnosticLogFile().getFullPathName());
}

PluginProcessor::~PluginProcessor()
{
    writeDiagnosticEvent("instance_end");
    stopTimer();
    StageMindLinkRegistry::instance().unregisterInstance(linkHandle);
}

juce::AudioProcessor::BusesProperties PluginProcessor::createBuses()
{
    return BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
        .withInput("Sidechain", juce::AudioChannelSet::stereo(), false);
}

void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    spatialProcessor.prepare(sampleRate, samplesPerBlock);
    motionProcessor.prepare(sampleRate, samplesPerBlock);
    depthProcessor.prepare(sampleRate, samplesPerBlock);
    cleanUpProcessor.prepare(sampleRate, samplesPerBlock);
    pseudoDoubleProcessor.prepare(sampleRate, samplesPerBlock);
    resonanceDetector.prepare(sampleRate, samplesPerBlock);
    resonanceLearner.prepare(sampleRate);
    dynamicEQ.prepare(sampleRate, samplesPerBlock);
    levelRider.prepare(sampleRate, samplesPerBlock);
    stageGainLoudnessMeter.prepare(sampleRate);
    sidechainDetector.prepare(sampleRate, samplesPerBlock);
    sidechainDynamicEQ.prepare(sampleRate, samplesPerBlock);
    linkActivityEnvelope.prepare(sampleRate, samplesPerBlock);
    linkSpectralAnalyzer.prepare(sampleRate);
    correlationMeter.reset();
    correlationSafetyScale = 1.0f;

    outputGain.reset(sampleRate, 0.03);
    outputGain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(rawValue(parameters::ids::outputGain)));
    setLatencySamples(LevelRider::getLookAheadSamplesForSampleRate(currentSampleRate));
    resetAutoAssistTracking();
    writeDiagnosticEvent(
        "prepare_to_play",
        "sr=" + juce::String(currentSampleRate, 0) + " block=" + juce::String(samplesPerBlock)
            + " latency=" + juce::String(getLatencySamples()));
}

void PluginProcessor::releaseResources()
{
    publishDisabledLinkState();
    motionProcessor.reset();
    depthProcessor.reset();
    cleanUpProcessor.reset();
    pseudoDoubleProcessor.reset();
    sidechainDetector.reset();
    sidechainDynamicEQ.reset();
    resonanceDetector.reset();
    resonanceLearner.resetRuntime();
    dynamicEQ.reset();
    levelRider.reset();
    stageGainLoudnessMeter.reset();
    correlationMeter.reset();
    linkActivityEnvelope.reset();
    linkSpectralAnalyzer.reset();
    writeDiagnosticEvent("release_resources");
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mainInput = layouts.getMainInputChannelSet();
    const auto mainOutput = layouts.getMainOutputChannelSet();

    if (mainInput != mainOutput)
        return false;

    if (mainInput != juce::AudioChannelSet::mono() && mainInput != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.inputBuses.size() > 1)
    {
        const auto sidechain = layouts.inputBuses[1];
        if (! sidechain.isDisabled()
            && sidechain != juce::AudioChannelSet::mono()
            && sidechain != juce::AudioChannelSet::stereo())
        {
            return false;
        }
    }

    return true;
}

void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    publishTransportPosition();

    auto mainBuffer = getBusBuffer(buffer, false, 0);

    for (auto channel = mainBuffer.getNumChannels(); channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    const auto inputLevels = calculateRmsAndPeak(mainBuffer);
    meters.inputRms.store(inputLevels.first, std::memory_order_relaxed);
    meters.inputPeak.store(inputLevels.second, std::memory_order_relaxed);

    if (isDirectorMode())
    {
        LevelRiderConfig latencyConfig;
        latencyConfig.mode = LevelRiderMode::Off;
        latencyConfig.ceilingDb = -0.2f;
        levelRider.process(mainBuffer, inputLevels.first, inputLevels.second, latencyConfig);
        const auto directorOutputLevels = calculateRmsAndPeak(mainBuffer);

        meters.outputRms.store(directorOutputLevels.first, std::memory_order_relaxed);
        meters.outputPeak.store(directorOutputLevels.second, std::memory_order_relaxed);
        meters.sidechainRms.store(0.0f, std::memory_order_relaxed);
        meters.sidechainPeak.store(0.0f, std::memory_order_relaxed);
        meters.sidechainEnvelope.store(0.0f, std::memory_order_relaxed);
        meters.correlation.store(correlationMeter.processBlock(mainBuffer), std::memory_order_relaxed);
        meters.gainReductionDb.store(0.0f, std::memory_order_relaxed);
        meters.resonanceReductionDb.store(0.0f, std::memory_order_relaxed);
        meters.levelRideActive.store(0, std::memory_order_relaxed);
        meters.levelRideGainDb.store(0.0f, std::memory_order_relaxed);
        meters.levelRideTargetRms.store(0.0f, std::memory_order_relaxed);
        meters.levelRideTargetDb.store(-120.0f, std::memory_order_relaxed);
        meters.levelRideOutputPeakDb.store(-120.0f, std::memory_order_relaxed);
        meters.levelRideHeld.store(0, std::memory_order_relaxed);
        meters.levelRideAnalyzing.store(0, std::memory_order_relaxed);
        meters.levelRideMode.store(0, std::memory_order_relaxed);
        meters.resonances.peakCount.store(0, std::memory_order_relaxed);
        publishDisabledLinkState();
        return;
    }

    auto sidechainBuffer = getOptionalSidechainBuffer(buffer);
    const auto sidechain = sidechainDetector.processBlock(sidechainBuffer);
    meters.sidechainRms.store(sidechain.rms, std::memory_order_relaxed);
    meters.sidechainPeak.store(sidechain.peak, std::memory_order_relaxed);
    meters.sidechainEnvelope.store(sidechain.envelope, std::memory_order_relaxed);

    const auto role = roleFromSelectableIndex(static_cast<int> (rawValue(parameters::ids::role)));
    const auto& profile = rolePresetEngine.getProfile(role);
    const auto safety = safetyModeFromIndex(static_cast<int> (rawValue(parameters::ids::safety)));
    auto spatialParams = rolePresetEngine.buildSpatialParams(role, safety, makeMacroParams());
    applyCorrelationSafety(spatialParams);
    const auto motionConfig = makeMotionConfig(profile, safety);
    const auto depthConfig = makeDepthConfig(profile, safety);
    const auto cleanUpConfig = makeCleanUpConfig(profile, role);
    const auto pseudoDoubleConfig = makePseudoDoubleConfig(profile, safety);
    publishStageState(spatialParams, depthConfig, motionConfig, profile.allowMotion);

    if (sidechainListenModeFromIndex(static_cast<int> (rawValue(parameters::ids::sidechainListen))) == SidechainListenMode::SidechainOnly)
    {
        renderSidechainListen(sidechainBuffer, mainBuffer);
        const auto listenPreLatencyLevels = calculateRmsAndPeak(mainBuffer);
        LevelRiderConfig latencyConfig;
        latencyConfig.mode = LevelRiderMode::Off;
        latencyConfig.ceilingDb = -0.2f;
        levelRider.process(mainBuffer, listenPreLatencyLevels.first, listenPreLatencyLevels.second, latencyConfig);
        updateOutputGainTarget();
        applyOutputGain(mainBuffer);

        const auto listenOutputLevels = calculateRmsAndPeak(mainBuffer);
        const auto listenCorrelation = correlationMeter.processBlock(mainBuffer);
        const auto listenBands = rawValue(parameters::ids::linkEnabled) >= 0.5f
            ? linkSpectralAnalyzer.processBlock(mainBuffer)
            : LinkSpectralBands {};
        meters.outputRms.store(listenOutputLevels.first, std::memory_order_relaxed);
        meters.outputPeak.store(listenOutputLevels.second, std::memory_order_relaxed);
        meters.correlation.store(listenCorrelation, std::memory_order_relaxed);
        meters.gainReductionDb.store(0.0f, std::memory_order_relaxed);
        meters.levelRideActive.store(0, std::memory_order_relaxed);
        meters.levelRideGainDb.store(0.0f, std::memory_order_relaxed);
        meters.levelRideTargetRms.store(0.0f, std::memory_order_relaxed);
        meters.levelRideTargetDb.store(-120.0f, std::memory_order_relaxed);
        meters.levelRideOutputPeakDb.store(-120.0f, std::memory_order_relaxed);
        meters.levelRideHeld.store(0, std::memory_order_relaxed);
        meters.levelRideAnalyzing.store(0, std::memory_order_relaxed);
        meters.levelRideMode.store(0, std::memory_order_relaxed);
        publishLinkState(
            role,
            inputLevels,
            listenOutputLevels,
            listenCorrelation,
            sidechain.envelope,
            listenBands,
            spatialParams.pan,
            mainBuffer.getNumSamples());
        return;
    }

    spatialProcessor.setParams(spatialParams);
    spatialProcessor.process(mainBuffer);
    motionProcessor.process(mainBuffer, motionConfig);
    depthProcessor.process(mainBuffer, depthConfig);
    cleanUpProcessor.process(mainBuffer, cleanUpConfig);
    pseudoDoubleProcessor.process(mainBuffer, pseudoDoubleConfig);

    const auto effectiveSidechain = makeEffectiveSidechainAnalysis(sidechain);
    meters.sidechainRms.store(effectiveSidechain.rms, std::memory_order_relaxed);
    meters.sidechainPeak.store(effectiveSidechain.peak, std::memory_order_relaxed);
    meters.sidechainEnvelope.store(effectiveSidechain.envelope, std::memory_order_relaxed);

    const auto gainReductionDb = sidechainDynamicEQ.process(mainBuffer, effectiveSidechain, makeSidechainEQConfig());

    if (resonanceLearnRequested.exchange(false, std::memory_order_acq_rel))
        resonanceLearner.beginLearn();

    updateAutoAssistBeforeResonance(role, inputLevels, mainBuffer.getNumSamples());

    const auto liveResonanceSnapshot = resonanceDetector.processBlock(mainBuffer, makeResonanceDetectorConfig());
    const auto resonanceSnapshot = resonanceLearner.process(liveResonanceSnapshot, mainBuffer.getNumSamples());
    publishResonanceLearnState();
    publishLearnedResonances(resonanceLearner.getLearnedSnapshot());
    publishResonances(resonanceSnapshot);

    const auto resonanceReductionDb = dynamicEQ.processResonances(mainBuffer, resonanceSnapshot, makeResonanceSuppressionConfig());
    meters.resonanceReductionDb.store(resonanceReductionDb, std::memory_order_relaxed);
    meters.gainReductionDb.store(std::max(gainReductionDb, resonanceReductionDb), std::memory_order_relaxed);

    const auto preLevelRideLevels = calculateRmsAndPeak(mainBuffer);
    const auto stageGainMode = stageGainModeFromIndex(static_cast<int> (rawValue(parameters::ids::stageGainMode)));
    const auto stageGainMeterMode = stageGainMeterModeFromIndex(static_cast<int> (rawValue(parameters::ids::stageGainMeterMode)));
    const auto stageGainLevel = stageGainLoudnessMeter.process(mainBuffer, stageGainMeterMode);
    const auto levelRideSourceLevel = stageGainMode == StageGainMode::Off
        ? preLevelRideLevels.first
        : stageGainLevel;
    updateOutputGainTarget();
    applyOutputGain(mainBuffer);
    const auto preLimiterLevels = calculateRmsAndPeak(mainBuffer);
    const auto levelRideStatus = levelRider.process(
        mainBuffer,
        levelRideSourceLevel,
        preLimiterLevels.second,
        makeLevelRiderConfig());
    meters.levelRideActive.store(levelRideStatus.active ? 1 : 0, std::memory_order_relaxed);
    meters.levelRideGainDb.store(levelRideStatus.gainDb, std::memory_order_relaxed);
    meters.levelRideTargetRms.store(levelRideStatus.targetRms, std::memory_order_relaxed);
    meters.levelRideTargetDb.store(levelRideStatus.targetDb, std::memory_order_relaxed);
    meters.levelRideOutputPeakDb.store(levelRideStatus.outputPeakDb, std::memory_order_relaxed);
    meters.levelRideHeld.store(levelRideStatus.held ? 1 : 0, std::memory_order_relaxed);
    meters.levelRideAnalyzing.store(levelRideStatus.analyzing ? 1 : 0, std::memory_order_relaxed);
    meters.levelRideMode.store(levelRideStatus.modeIndex, std::memory_order_relaxed);

    const auto correlation = correlationMeter.processBlock(mainBuffer);
    meters.correlation.store(correlation, std::memory_order_relaxed);
    const auto safetyLimits = RolePresetEngine::getSafetyLimits(safety);
    const auto threshold = std::max(rawValue(parameters::ids::correlationSafetyThreshold), safetyLimits.correlationThreshold);
    updateCorrelationSafety(correlation, threshold);

    const auto outputLevels = calculateRmsAndPeak(mainBuffer);
    const auto spectralBands = rawValue(parameters::ids::linkEnabled) >= 0.5f
        ? linkSpectralAnalyzer.processBlock(mainBuffer)
        : LinkSpectralBands {};
    meters.outputRms.store(outputLevels.first, std::memory_order_relaxed);
    meters.outputPeak.store(outputLevels.second, std::memory_order_relaxed);
    publishLinkState(
        role,
        inputLevels,
        outputLevels,
        correlation,
        sidechain.envelope,
        spectralBands,
        spatialParams.pan,
        mainBuffer.getNumSamples());
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor(*this);
}

std::pair<int, int> PluginProcessor::getSavedEditorSize() const
{
    const auto width = static_cast<int> (apvts.state.getProperty(editorWidthProperty, editorDefaultWidth));
    const auto height = static_cast<int> (apvts.state.getProperty(editorHeightProperty, editorDefaultHeight));

    return {
        juce::jlimit(editorMinWidth, editorMaxWidth, width),
        juce::jlimit(editorMinHeight, editorMaxHeight, height)
    };
}

void PluginProcessor::setSavedEditorSize(int width, int height)
{
    const auto safeWidth = juce::jlimit(editorMinWidth, editorMaxWidth, width);
    const auto safeHeight = juce::jlimit(editorMinHeight, editorMaxHeight, height);

    if (static_cast<int> (apvts.state.getProperty(editorWidthProperty, 0)) != safeWidth)
        apvts.state.setProperty(editorWidthProperty, safeWidth, nullptr);

    if (static_cast<int> (apvts.state.getProperty(editorHeightProperty, 0)) != safeHeight)
        apvts.state.setProperty(editorHeightProperty, safeHeight, nullptr);
}

void PluginProcessor::beginResonanceLearn() noexcept
{
    resonanceLearnRequested.store(true, std::memory_order_release);
}

void PluginProcessor::beginStageGainAnalyze() noexcept
{
    stageGainAnalyzeRequested.store(true, std::memory_order_release);
    writeDiagnosticEvent("stage_gain_analyze", "local");
}

int PluginProcessor::requestStageGainAnalyzeForCurrentGroup()
{
    const auto group = static_cast<int> (rawValue(parameters::ids::linkGroup));
    if (group <= 0)
        return 0;

    const auto snapshot = StageMindLinkRegistry::instance().readGroup(group);
    auto sent = 0;

    for (int index = 0; index < snapshot.count; ++index)
    {
        const auto& peer = snapshot.peers[static_cast<size_t> (index)];
        if (! peer.found || peer.instanceId == linkHandle.instanceId)
            continue;

        LinkCommand command;
        command.sourceInstanceId = linkHandle.instanceId;
        command.setStageGainMode = true;
        command.stageGainMode = static_cast<int> (StageGainMode::Static);
        command.requestStageGainAnalyze = true;

        if (StageMindLinkRegistry::instance().submitCommand(peer.instanceId, command))
            ++sent;
    }

    writeDiagnosticEvent("director_analyze_all", "sent=" + juce::String(sent));
    return sent;
}

void PluginProcessor::beginRideMemoryLearn() noexcept
{
    {
        const juce::ScopedLock lock(rideMemoryLock);
        rideMemory.setLearning(true);
    }

    {
        const juce::ScopedLock lock(rideTimelineMemoryLock);
        rideTimelineMemory.setLearning(true);
    }

    {
        const juce::ScopedLock lock(balanceTimelineMemoryLock);
        balanceTimelineMemory.setLearning(true);
    }
}

void PluginProcessor::clearRideMemory() noexcept
{
    {
        const juce::ScopedLock lock(rideMemoryLock);
        rideMemory.clear();
    }

    {
        const juce::ScopedLock lock(rideTimelineMemoryLock);
        rideTimelineMemory.clear();
    }

    {
        const juce::ScopedLock lock(balanceTimelineMemoryLock);
        balanceTimelineMemory.clear();
    }
}

RideMemorySnapshot PluginProcessor::getRideMemorySnapshot() const noexcept
{
    const juce::ScopedLock lock(rideMemoryLock);
    return rideMemory.snapshot();
}

RideTimelineSnapshot PluginProcessor::getRideTimelineSnapshot() const noexcept
{
    const juce::ScopedLock lock(rideTimelineMemoryLock);
    return rideTimelineMemory.snapshot();
}

BalanceTimelineSnapshot PluginProcessor::getBalanceTimelineSnapshot() const noexcept
{
    const juce::ScopedLock lock(balanceTimelineMemoryLock);
    return balanceTimelineMemory.snapshot();
}

int PluginProcessor::getNumPrograms()
{
    return static_cast<int> (factoryPresets.size());
}

int PluginProcessor::getCurrentProgram()
{
    return currentProgram;
}

void PluginProcessor::setCurrentProgram(int index)
{
    applyFactoryPreset(juce::jlimit(0, getNumPrograms() - 1, index));
}

const juce::String PluginProcessor::getProgramName(int index)
{
    if (index < 0 || index >= getNumPrograms())
        return {};

    return factoryPresets[static_cast<size_t> (index)].name;
}

void PluginProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    RideMemorySnapshot rideMemoryState;
    {
        const juce::ScopedLock lock(rideMemoryLock);
        rideMemoryState = rideMemory.snapshot();
    }

    RideTimelineSnapshot rideTimelineMemoryState;
    {
        const juce::ScopedLock lock(rideTimelineMemoryLock);
        rideTimelineMemoryState = rideTimelineMemory.snapshot();
    }

    BalanceTimelineSnapshot balanceTimelineMemoryState;
    {
        const juce::ScopedLock lock(balanceTimelineMemoryLock);
        balanceTimelineMemoryState = balanceTimelineMemory.snapshot();
    }

    PluginState::writeToBlock(
        apvts,
        readLearnedResonanceSnapshot(),
        rideMemoryState,
        rideTimelineMemoryState,
        balanceTimelineMemoryState,
        levelRider.getTargetRms(),
        levelRider.getHeldGainDb(),
        levelRider.hasHeldGain(),
        destData);
}

void PluginProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    const auto restored = PluginState::restoreFromData(apvts, data, sizeInBytes);
    resonanceLearner.setLearnedSnapshot(restored.learnedResonances);
    levelRider.setTargetRms(restored.levelRiderTargetRms);
    if (restored.levelRiderHasHeldGain)
        levelRider.setHeldGainDb(restored.levelRiderHeldGainDb);
    else
        levelRider.clearHeldGain();
    publishLearnedResonances(restored.learnedResonances);
    {
        const juce::ScopedLock lock(rideMemoryLock);
        rideMemory.restore(restored.rideMemory);
    }
    {
        const juce::ScopedLock lock(rideTimelineMemoryLock);
        rideTimelineMemory.restore(restored.rideTimelineMemory);
    }
    {
        const juce::ScopedLock lock(balanceTimelineMemoryLock);
        balanceTimelineMemory.restore(restored.balanceTimelineMemory);
    }
    resetAutoAssistTracking();
}

std::pair<float, float> PluginProcessor::calculateRmsAndPeak(const juce::AudioBuffer<float>& buffer) noexcept
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    if (numChannels == 0 || numSamples == 0)
        return { 0.0f, 0.0f };

    double sumSquares = 0.0;
    float peak = 0.0f;

    for (int channel = 0; channel < numChannels; ++channel)
    {
        const auto* samples = buffer.getReadPointer(channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const auto value = samples[sample];
            sumSquares += static_cast<double> (value) * value;
            peak = std::max(peak, std::abs(value));
        }
    }

    const auto divisor = static_cast<double> (numChannels * numSamples);
    return { static_cast<float> (std::sqrt(sumSquares / divisor)), peak };
}

float PluginProcessor::rawValue(const char* parameterId) const noexcept
{
    if (const auto* value = apvts.getRawParameterValue(parameterId))
        return value->load(std::memory_order_relaxed);

    return 0.0f;
}

void PluginProcessor::publishTransportPosition() noexcept
{
    auto valid = false;
    auto playing = false;
    auto ppqPosition = 0.0;
    auto bpm = 120.0;

    if (auto* currentPlayHead = getPlayHead())
    {
        if (const auto position = currentPlayHead->getPosition())
        {
            if (const auto ppq = position->getPpqPosition())
            {
                ppqPosition = *ppq;
                valid = std::isfinite(ppqPosition) && ppqPosition >= 0.0;
            }

            if (const auto tempo = position->getBpm())
                bpm = std::clamp(*tempo, 20.0, 400.0);

            playing = position->getIsPlaying();
        }
    }

    transportPositionValid.store(valid ? 1 : 0, std::memory_order_relaxed);
    transportIsPlaying.store(playing ? 1 : 0, std::memory_order_relaxed);
    transportPpqPosition.store(valid ? ppqPosition : 0.0, std::memory_order_relaxed);
    transportBpm.store(bpm, std::memory_order_relaxed);
}

TransportPositionSnapshot PluginProcessor::getTransportSnapshot() const noexcept
{
    TransportPositionSnapshot snapshot;
    snapshot.valid = transportPositionValid.load(std::memory_order_relaxed) != 0;
    snapshot.playing = transportIsPlaying.load(std::memory_order_relaxed) != 0;
    snapshot.ppqPosition = transportPpqPosition.load(std::memory_order_relaxed);
    snapshot.bpm = transportBpm.load(std::memory_order_relaxed);
    return snapshot;
}

void PluginProcessor::writeDiagnosticEvent(const juce::String& eventName, const juce::String& note)
{
    const auto transport = getTransportSnapshot();
    const auto pluginMode = static_cast<int> (rawValue(parameters::ids::pluginMode));
    const auto role = static_cast<int> (rawValue(parameters::ids::role));
    const auto group = static_cast<int> (rawValue(parameters::ids::linkGroup));
    const auto autoMode = static_cast<int> (rawValue(parameters::ids::autoAssistMode));
    const auto stageMode = static_cast<int> (rawValue(parameters::ids::stageGainMode));
    const auto stageMeterMode = static_cast<int> (rawValue(parameters::ids::stageGainMeterMode));

    juce::StringArray fields;
    fields.add(csvEscape(juce::Time::getCurrentTime().formatted("%Y-%m-%d %H:%M:%S")));
    fields.add(csvEscape(eventName));
    fields.add(juce::String(linkHandle.instanceId));
    fields.add(juce::String(pluginMode));
    fields.add(juce::String(role));
    fields.add(juce::String(group));
    fields.add(juce::String(autoMode));
    fields.add(juce::String(stageMode));
    fields.add(juce::String(stageMeterMode));
    fields.add(juce::String(rawValue(parameters::ids::stageGainTargetDb), 2));
    fields.add(juce::String(rawValue(parameters::ids::stageGainThresholdVu), 2));
    fields.add(juce::String(rawValue(parameters::ids::stageGainCeilingDb), 2));
    fields.add(juce::String(rawValue(parameters::ids::stageGainResponse), 3));
    fields.add(dbFromGain(meters.inputRms.load(std::memory_order_relaxed)));
    fields.add(dbFromGain(meters.outputRms.load(std::memory_order_relaxed)));
    fields.add(dbFromGain(meters.outputPeak.load(std::memory_order_relaxed)));
    fields.add(juce::String(meters.correlation.load(std::memory_order_relaxed), 3));
    fields.add(juce::String(meters.gainReductionDb.load(std::memory_order_relaxed), 2));
    fields.add(juce::String(meters.resonanceReductionDb.load(std::memory_order_relaxed), 2));
    fields.add(juce::String(meters.levelRideGainDb.load(std::memory_order_relaxed), 2));
    fields.add(juce::String(meters.levelRideTargetDb.load(std::memory_order_relaxed), 2));
    fields.add(juce::String(meters.levelRideOutputPeakDb.load(std::memory_order_relaxed), 2));
    fields.add(juce::String(meters.linkNodeCount.load(std::memory_order_relaxed)));
    fields.add(juce::String(meters.linkActivePeers.load(std::memory_order_relaxed)));
    fields.add(juce::String(meters.linkPeerId.load(std::memory_order_relaxed)));
    fields.add(juce::String(meters.linkPeerRole.load(std::memory_order_relaxed)));
    fields.add(juce::String(meters.autoAssistState.load(std::memory_order_relaxed)));
    fields.add(juce::String(meters.autoAssistActionKind.load(std::memory_order_relaxed)));
    fields.add(juce::String(meters.directorBalanceActive.load(std::memory_order_relaxed)));
    fields.add(juce::String(meters.directorBalanceTargetRole.load(std::memory_order_relaxed)));
    fields.add(juce::String(meters.directorBalanceDeviationDb.load(std::memory_order_relaxed), 2));
    fields.add(juce::String(meters.directorBalanceCorrectionDb.load(std::memory_order_relaxed), 2));
    fields.add(transport.playing ? "1" : "0");
    fields.add(juce::String(transport.valid ? transport.ppqPosition : 0.0, 3));
    fields.add(juce::String(transport.bpm, 2));
    fields.add(csvEscape(note));

    appendDiagnosticLogLine(fields.joinIntoString(","));
}

void PluginProcessor::writeDiagnosticSnapshot()
{
    const auto eventName = isDirectorMode() ? "director_snapshot" : "node_snapshot";
    writeDiagnosticEvent(eventName);
}

void PluginProcessor::timerCallback()
{
    ++diagnosticSnapshotTicks;
    if (diagnosticSnapshotTicks >= diagnosticSnapshotIntervalTicks)
    {
        diagnosticSnapshotTicks = 0;
        const auto transport = getTransportSnapshot();
        const auto hasAudibleSignal =
            meters.inputRms.load(std::memory_order_relaxed) > 0.001f
            || meters.outputRms.load(std::memory_order_relaxed) > 0.001f
            || meters.sidechainEnvelope.load(std::memory_order_relaxed) > 0.001f;

        if (transport.playing || (! transport.valid && hasAudibleSignal))
            writeDiagnosticSnapshot();
    }

    if (isDirectorMode())
    {
        updateDirectorAutoCommands();
        return;
    }

    const auto freshTicks = linkAudioPublishFreshTicks.load(std::memory_order_relaxed);
    if (freshTicks > 0)
        linkAudioPublishFreshTicks.store(freshTicks - 1, std::memory_order_relaxed);
    else
        publishIdleLinkState();

    applyLinkCommand(StageMindLinkRegistry::instance().consumeCommand(linkHandle));
    applyPendingAutoAssist();
}

void PluginProcessor::updateDirectorAutoCommands()
{
    if (directorAutoCommandCooldownTicks > 0)
        --directorAutoCommandCooldownTicks;

    const auto directorAutoEnabled =
        autoAssistModeFromIndex(static_cast<int> (rawValue(parameters::ids::autoAssistMode))) == AutoAssistMode::Auto;

    const auto group = juce::jlimit(0, 16, static_cast<int> (rawValue(parameters::ids::linkGroup)));
    meters.linkInstanceId.store(static_cast<int> (linkHandle.instanceId), std::memory_order_relaxed);
    meters.linkGroup.store(group, std::memory_order_relaxed);

    if (group <= 0)
    {
        meters.linkEnabled.store(0, std::memory_order_relaxed);
        meters.linkNodeCount.store(0, std::memory_order_relaxed);
        meters.linkActivePeers.store(0, std::memory_order_relaxed);
        meters.directorBalanceActive.store(0, std::memory_order_relaxed);
        meters.directorBalanceTargetRole.store(0, std::memory_order_relaxed);
        meters.directorBalanceDeviationDb.store(0.0f, std::memory_order_relaxed);
        meters.directorBalanceCorrectionDb.store(0.0f, std::memory_order_relaxed);
        return;
    }

    const auto snapshot = StageMindLinkRegistry::instance().readGroup(group);
    meters.linkEnabled.store(1, std::memory_order_relaxed);
    meters.linkNodeCount.store(snapshot.count, std::memory_order_relaxed);
    meters.linkActivePeers.store(countActiveBalanceNodes(snapshot), std::memory_order_relaxed);

    if (snapshot.count < 2)
    {
        meters.directorBalanceActive.store(0, std::memory_order_relaxed);
        meters.directorBalanceTargetRole.store(0, std::memory_order_relaxed);
        meters.directorBalanceDeviationDb.store(0.0f, std::memory_order_relaxed);
        meters.directorBalanceCorrectionDb.store(0.0f, std::memory_order_relaxed);
        return;
    }

    RideMemorySnapshot memoryState;
    {
        const juce::ScopedLock lock(rideMemoryLock);
        memoryState = rideMemory.snapshot();
    }

    RideTimelineSnapshot timelineMemoryState;
    {
        const juce::ScopedLock lock(rideTimelineMemoryLock);
        timelineMemoryState = rideTimelineMemory.snapshot();
    }

    BalanceTimelineSnapshot balanceTimelineMemoryState;
    {
        const juce::ScopedLock lock(balanceTimelineMemoryLock);
        balanceTimelineMemoryState = balanceTimelineMemory.snapshot();
    }

    const auto transport = getTransportSnapshot();
    if (! transport.playing)
    {
        meters.directorBalanceActive.store(0, std::memory_order_relaxed);
        meters.directorBalanceTargetRole.store(0, std::memory_order_relaxed);
        meters.directorBalanceDeviationDb.store(0.0f, std::memory_order_relaxed);
        meters.directorBalanceCorrectionDb.store(0.0f, std::memory_order_relaxed);
        return;
    }

    const auto canSendCommand = directorAutoEnabled && directorAutoCommandCooldownTicks <= 0;
    const auto shouldLearnMemory = memoryState.learning || directorAutoEnabled;
    const auto shouldLearnTimelineMemory = (timelineMemoryState.learning || directorAutoEnabled) && transport.valid;
    const auto shouldLearnBalanceTimeline = (balanceTimelineMemoryState.learning || directorAutoEnabled) && transport.valid;
    const auto balanceCorrection = findDirectorBalanceCorrection(snapshot);

    meters.directorBalanceMemoryEvents.store(balanceTimelineMemoryState.count, std::memory_order_relaxed);
    meters.directorBalanceActive.store(balanceCorrection.found ? 1 : 0, std::memory_order_relaxed);
    meters.directorBalanceTargetRole.store(balanceCorrection.found ? balanceCorrection.target.role : 0, std::memory_order_relaxed);
    meters.directorBalanceDeviationDb.store(balanceCorrection.found ? balanceCorrection.deviationDb : 0.0f, std::memory_order_relaxed);
    meters.directorBalanceCorrectionDb.store(balanceCorrection.found ? balanceCorrection.correctionDb : 0.0f, std::memory_order_relaxed);

    if (shouldLearnBalanceTimeline)
    {
        const juce::ScopedLock lock(balanceTimelineMemoryLock);
        if (balanceCorrection.found)
        {
            balanceTimelineMemory.observe(
                group,
                balanceCorrection.target.role,
                transport.ppqPosition,
                balanceCorrection.correctionDb,
                balanceCorrection.severity,
                false);
        }
        else if (countActiveBalanceNodes(snapshot) >= 2)
        {
            balanceTimelineMemory.markNearbyResolved(group, transport.ppqPosition);
        }
    }

    if (canSendCommand)
    {
        if (transport.valid && timelineMemoryState.count > 0)
        {
            for (const auto& event : timelineMemoryState.events)
            {
                if (! event.used || event.group != group || event.actionKind <= 0)
                    continue;

                if (! event.contains(transport.ppqPosition, rideTimelineMergeWindowPpq))
                    continue;

                const auto action = LinkSuggestionEngine::actionFor(static_cast<LinkSuggestionKind> (event.actionKind));
                if (! action.available)
                    continue;

                for (int current = 0; current < snapshot.count; ++current)
                {
                    const auto& currentNode = snapshot.peers[static_cast<size_t> (current)];
                    if (! rolesMatchForRideMemory(currentNode, event.targetRole)
                        || autoAssistModeFromIndex(currentNode.autoAssistMode) != AutoAssistMode::Auto)
                    {
                        continue;
                    }

                    if (isSnapshotActionApplied(currentNode, action))
                    {
                        {
                            const juce::ScopedLock lock(rideMemoryLock);
                            rideMemory.markResolved(event.group, event.targetRole, event.sourceRole, event.actionKind, event.band);
                        }
                        {
                            const juce::ScopedLock lock(rideTimelineMemoryLock);
                            rideTimelineMemory.markResolved(
                                event.group,
                                event.targetRole,
                                event.sourceRole,
                                event.actionKind,
                                event.band,
                                transport.ppqPosition);
                        }
                        continue;
                    }

                    for (int peer = 0; peer < snapshot.count; ++peer)
                    {
                        if (current == peer)
                            continue;

                        const auto& peerNode = snapshot.peers[static_cast<size_t> (peer)];
                        if (! rolesMatchForRideMemory(peerNode, event.sourceRole))
                            continue;

                        LinkCommand command;
                        command.automatic = true;
                        command.sourceInstanceId = linkHandle.instanceId;
                        command.peerInstanceId = peerNode.instanceId;
                        command.actionKind = event.actionKind;

                        const auto sent = StageMindLinkRegistry::instance().submitCommand(currentNode.instanceId, command);
                        writeDiagnosticEvent(
                            "director_timeline_memory_command",
                            "sent=" + juce::String(sent ? 1 : 0)
                                + " target=" + juce::String(currentNode.instanceId)
                                + " peer=" + juce::String(peerNode.instanceId)
                                + " action=" + juce::String(command.actionKind));
                        directorAutoCommandCooldownTicks = directorAutoCommandCooldownDefaultTicks;
                        return;
                    }
                }
            }
        }

        for (const auto& event : memoryState.events)
        {
            if (! event.used || event.group != group || event.actionKind <= 0)
                continue;

            const auto action = LinkSuggestionEngine::actionFor(static_cast<LinkSuggestionKind> (event.actionKind));
            if (! action.available)
                continue;

            for (int current = 0; current < snapshot.count; ++current)
            {
                const auto& currentNode = snapshot.peers[static_cast<size_t> (current)];
                if (! rolesMatchForRideMemory(currentNode, event.targetRole)
                    || autoAssistModeFromIndex(currentNode.autoAssistMode) != AutoAssistMode::Auto)
                {
                    continue;
                }

                if (isSnapshotActionApplied(currentNode, action))
                {
                    {
                        const juce::ScopedLock lock(rideMemoryLock);
                        rideMemory.markResolved(event.group, event.targetRole, event.sourceRole, event.actionKind, event.band);
                    }

                    if (transport.valid)
                    {
                        const juce::ScopedLock lock(rideTimelineMemoryLock);
                        rideTimelineMemory.markResolved(
                            event.group,
                            event.targetRole,
                            event.sourceRole,
                            event.actionKind,
                            event.band,
                            transport.ppqPosition);
                    }
                    continue;
                }

                for (int peer = 0; peer < snapshot.count; ++peer)
                {
                    if (current == peer)
                        continue;

                    const auto& peerNode = snapshot.peers[static_cast<size_t> (peer)];
                    if (! rolesMatchForRideMemory(peerNode, event.sourceRole))
                        continue;

                    LinkCommand command;
                    command.automatic = true;
                    command.sourceInstanceId = linkHandle.instanceId;
                    command.peerInstanceId = peerNode.instanceId;
                    command.actionKind = event.actionKind;

                    const auto sent = StageMindLinkRegistry::instance().submitCommand(currentNode.instanceId, command);
                    writeDiagnosticEvent(
                        "director_ride_memory_command",
                        "sent=" + juce::String(sent ? 1 : 0)
                            + " target=" + juce::String(currentNode.instanceId)
                            + " peer=" + juce::String(peerNode.instanceId)
                            + " action=" + juce::String(command.actionKind));
                    directorAutoCommandCooldownTicks = directorAutoCommandCooldownDefaultTicks;
                    return;
                }
            }
        }
    }

    if (! canSendCommand && ! shouldLearnMemory)
        return;

    LinkSuggestion bestSuggestion;
    LinkPeerSnapshot bestTarget;
    LinkPeerSnapshot bestPeer;
    auto bestScore = -1.0f;

    for (int current = 0; current < snapshot.count; ++current)
    {
        const auto& currentNode = snapshot.peers[static_cast<size_t> (current)];
        if (autoAssistModeFromIndex(currentNode.autoAssistMode) != AutoAssistMode::Auto)
            continue;

        for (int peer = 0; peer < snapshot.count; ++peer)
        {
            if (current == peer)
                continue;

            const auto& peerNode = snapshot.peers[static_cast<size_t> (peer)];

            LinkSuggestionInput input;
            input.linkActive = true;
            input.peerFound = true;
            input.currentRole = static_cast<TrackRole> (currentNode.role);
            input.peerRole = static_cast<TrackRole> (peerNode.role);
            input.currentWidth = currentNode.width;
            input.currentDepth = currentNode.depth;
            input.currentCorrelation = currentNode.correlation;
            input.peerActivity = peerNode.activity;
            input.peerWidth = peerNode.width;
            input.currentBands = currentNode.bands;
            input.peerBands = peerNode.bands;

            const auto suggestion = LinkSuggestionEngine::evaluate(input);
            if (! suggestion.hasSuggestion())
                continue;

            const auto action = LinkSuggestionEngine::actionFor(suggestion.kind);
            if (! action.available)
                continue;

            const auto band = strongestRideMemoryBand(currentNode.bands, peerNode.bands);
            const auto actionAlreadyApplied = isSnapshotActionApplied(currentNode, action);

            if (shouldLearnMemory)
            {
                const juce::ScopedLock lock(rideMemoryLock);
                rideMemory.observe(
                    group,
                    currentNode.role,
                    peerNode.role,
                    static_cast<int> (suggestion.kind),
                    band,
                    suggestion.severity,
                    actionAlreadyApplied);
            }
            if (shouldLearnTimelineMemory)
            {
                const juce::ScopedLock lock(rideTimelineMemoryLock);
                rideTimelineMemory.observe(
                    group,
                    currentNode.role,
                    peerNode.role,
                    static_cast<int> (suggestion.kind),
                    band,
                    transport.ppqPosition,
                    suggestion.severity,
                    actionAlreadyApplied);
            }
            else if (actionAlreadyApplied)
            {
                const juce::ScopedLock lock(rideMemoryLock);
                rideMemory.markResolved(
                    group,
                    currentNode.role,
                    peerNode.role,
                    static_cast<int> (suggestion.kind),
                    band);
            }

            if (actionAlreadyApplied && transport.valid)
            {
                const juce::ScopedLock lock(rideTimelineMemoryLock);
                rideTimelineMemory.markResolved(
                    group,
                    currentNode.role,
                    peerNode.role,
                    static_cast<int> (suggestion.kind),
                    band,
                    transport.ppqPosition);
            }

            if (actionAlreadyApplied || ! canSendCommand)
                continue;

            const auto score = autoSuggestionScore(suggestion.kind, suggestion.severity);
            if (score > bestScore)
            {
                bestScore = score;
                bestSuggestion = suggestion;
                bestTarget = currentNode;
                bestPeer = peerNode;
            }
        }
    }

    const auto balanceMemoryPending = balanceCorrection.found
        && transport.valid
        && balanceTimelineHasPendingEvent(
            balanceTimelineMemoryState,
            group,
            balanceCorrection.target.role,
            transport.ppqPosition);

    if (bestSuggestion.hasSuggestion() && bestScore >= (balanceMemoryPending ? 0.82f : 0.74f))
    {
        LinkCommand command;
        command.automatic = true;
        command.sourceInstanceId = linkHandle.instanceId;
        command.peerInstanceId = bestPeer.instanceId;
        command.actionKind = static_cast<int> (bestSuggestion.kind);

        const auto sent = StageMindLinkRegistry::instance().submitCommand(bestTarget.instanceId, command);
        writeDiagnosticEvent(
            "director_auto_suggestion",
            "sent=" + juce::String(sent ? 1 : 0)
                + " target=" + juce::String(bestTarget.instanceId)
                + " peer=" + juce::String(bestPeer.instanceId)
                + " action=" + juce::String(command.actionKind));
        directorAutoCommandCooldownTicks = directorAutoCommandCooldownDefaultTicks;
        return;
    }

    if (canSendCommand && balanceCorrection.found)
    {
        LinkCommand command;
        command.automatic = true;
        command.sourceInstanceId = linkHandle.instanceId;
        command.setOutputTrim = true;
        command.outputTrimDb = balanceCorrection.nextOutputTrimDb;

        const auto sent = StageMindLinkRegistry::instance().submitCommand(balanceCorrection.target.instanceId, command);
        writeDiagnosticEvent(
            "director_balance_trim",
            "sent=" + juce::String(sent ? 1 : 0)
                + " target=" + juce::String(balanceCorrection.target.instanceId)
                + " deviation_db=" + juce::String(balanceCorrection.deviationDb, 2)
                + " correction_db=" + juce::String(balanceCorrection.correctionDb, 2)
                + " next_output_db=" + juce::String(balanceCorrection.nextOutputTrimDb, 2));
        directorAutoCommandCooldownTicks = directorAutoCommandCooldownDefaultTicks;
        return;
    }

    if (! bestSuggestion.hasSuggestion())
        return;

    LinkCommand command;
    command.automatic = true;
    command.sourceInstanceId = linkHandle.instanceId;
    command.peerInstanceId = bestPeer.instanceId;
    command.actionKind = static_cast<int> (bestSuggestion.kind);

    const auto sent = StageMindLinkRegistry::instance().submitCommand(bestTarget.instanceId, command);
    writeDiagnosticEvent(
        "director_memory_suggestion",
        "sent=" + juce::String(sent ? 1 : 0)
            + " target=" + juce::String(bestTarget.instanceId)
            + " peer=" + juce::String(bestPeer.instanceId)
            + " action=" + juce::String(command.actionKind));
    directorAutoCommandCooldownTicks = directorAutoCommandCooldownDefaultTicks;
}

void PluginProcessor::applyLinkCommand(LinkCommand command)
{
    if (! command.found)
        return;

    writeDiagnosticEvent(
        "link_command",
        "seq=" + juce::String(command.sequence)
            + " source=" + juce::String(command.sourceInstanceId)
            + " peer=" + juce::String(command.peerInstanceId)
            + " action=" + juce::String(command.actionKind)
            + " auto=" + juce::String(command.automatic ? 1 : 0)
            + " set_pan=" + juce::String(command.setPan ? 1 : 0)
            + " set_width=" + juce::String(command.setWidth ? 1 : 0)
            + " set_depth=" + juce::String(command.setDepth ? 1 : 0)
            + " set_output=" + juce::String(command.setOutputTrim ? 1 : 0)
            + " analyze=" + juce::String(command.requestStageGainAnalyze ? 1 : 0));

    auto handledDirectParameter = false;
    if (command.setPan)
    {
        const auto role = roleFromSelectableIndex(static_cast<int> (rawValue(parameters::ids::role)));
        const auto& profile = rolePresetEngine.getProfile(role);
        applyParameterValueFromCommand(parameters::ids::pan, juce::jlimit(-1.0f, 1.0f, command.pan - profile.defaultPan));
        handledDirectParameter = true;
    }

    if (command.setDepth)
    {
        applyParameterValueFromCommand(parameters::ids::depth, command.depth);
        handledDirectParameter = true;
    }

    if (command.setWidth)
    {
        applyParameterValueFromCommand(parameters::ids::width, command.width);
        handledDirectParameter = true;
    }

    if (command.setMotion)
    {
        applyParameterValueFromCommand(parameters::ids::motion, command.motion);
        handledDirectParameter = true;
    }

    if (command.setOutputTrim)
    {
        applyParameterValueFromCommand(parameters::ids::outputGain, command.outputTrimDb);
        handledDirectParameter = true;
    }

    if (command.setCleanUp)
    {
        applyParameterValueFromCommand(parameters::ids::cleanUp, command.cleanUp);
        handledDirectParameter = true;
    }

    if (command.setResonance)
    {
        applyParameterValueFromCommand(parameters::ids::resonance, command.resonance);
        handledDirectParameter = true;
    }

    if (command.setSidechainAmount)
    {
        applyParameterValueFromCommand(parameters::ids::sidechainAmount, command.sidechainAmount);
        handledDirectParameter = true;
    }

    if (command.setStageGainMode)
    {
        applyParameterValueFromCommand(parameters::ids::stageGainMode, static_cast<float> (command.stageGainMode));
        handledDirectParameter = true;
    }

    if (command.requestStageGainAnalyze)
    {
        stageGainAnalyzeRequested.store(true, std::memory_order_release);
        handledDirectParameter = true;
    }

    if (handledDirectParameter && command.actionKind == 0)
        return;

    const auto kind = static_cast<LinkSuggestionKind> (command.actionKind);
    const auto action = LinkSuggestionEngine::actionFor(kind);
    if (! action.available)
        return;

    if (command.automatic)
    {
        if (autoAssistModeFromIndex(static_cast<int> (rawValue(parameters::ids::autoAssistMode))) == AutoAssistMode::Auto)
            applyAutoLinkAction(kind, command.peerInstanceId);

        return;
    }

    if (action.setWidth)
        applyParameterValueFromCommand(parameters::ids::width, action.width);

    if (action.setDepth)
        applyParameterValueFromCommand(parameters::ids::depth, action.depth);

    if (action.setSidechainMode)
        applyParameterValueFromCommand(parameters::ids::sidechainMode, static_cast<float> (action.sidechainModeIndex));
}

void PluginProcessor::applyPendingAutoAssist()
{
    const auto pendingActionKind = autoPendingLinkActionKind.exchange(0, std::memory_order_acq_rel);
    const auto pendingSourceId = static_cast<std::uint32_t> (
        autoPendingLinkSourceId.exchange(0, std::memory_order_acq_rel));
    const auto tuneResonance = autoResonanceTuningPending.exchange(false, std::memory_order_acq_rel);

    if (pendingActionKind == 0 && ! tuneResonance)
        return;

    if (autoAssistModeFromIndex(static_cast<int> (rawValue(parameters::ids::autoAssistMode))) != AutoAssistMode::Auto)
        return;

    writeDiagnosticEvent(
        "pending_auto_assist",
        "action=" + juce::String(pendingActionKind)
            + " source=" + juce::String(pendingSourceId)
            + " tune_resonance=" + juce::String(tuneResonance ? 1 : 0));

    if (pendingActionKind > 0)
        applyAutoLinkAction(static_cast<LinkSuggestionKind> (pendingActionKind), pendingSourceId);

    if (tuneResonance)
    {
        if (rawValue(parameters::ids::cleanUp) < 0.45f)
            applyParameterValueFromCommand(parameters::ids::cleanUp, 0.55f);

        if (rawValue(parameters::ids::resonance) < 0.40f)
            applyParameterValueFromCommand(parameters::ids::resonance, 0.45f);
    }
}

void PluginProcessor::applyAutoLinkAction(LinkSuggestionKind kind, std::uint32_t sourceInstanceId)
{
    const auto action = LinkSuggestionEngine::actionFor(kind);
    if (! action.available)
        return;

    if (isDuckingAction(action) && isLinkActionAppliedForAuto(action))
    {
        meters.autoAssistActionKind.store(static_cast<int> (kind), std::memory_order_relaxed);
        writeDiagnosticEvent(
            "auto_link_action_skipped",
            "kind=" + juce::String(static_cast<int> (kind))
                + " source=" + juce::String(sourceInstanceId)
                + " reason=ducking_already_active");
        return;
    }

    if (action.setWidth && std::abs(rawValue(parameters::ids::width) - action.width) > autoParameterEpsilon)
        applyParameterValueFromCommand(parameters::ids::width, action.width);

    if (action.setDepth && std::abs(rawValue(parameters::ids::depth) - action.depth) > autoParameterEpsilon)
        applyParameterValueFromCommand(parameters::ids::depth, action.depth);

    if (action.setSidechainMode)
    {
        if (sourceInstanceId != invalidLinkInstanceId)
            autoLinkSidechainSourceId.store(static_cast<int> (sourceInstanceId), std::memory_order_relaxed);

        if (static_cast<int> (rawValue(parameters::ids::sidechainMode)) != action.sidechainModeIndex)
            applyParameterValueFromCommand(parameters::ids::sidechainMode, static_cast<float> (action.sidechainModeIndex));

        const auto trigger = triggerModeFromIndex(static_cast<int> (rawValue(parameters::ids::triggerMode)));
        const auto externalSidechainIsAlreadyWorking =
            trigger == TriggerMode::ExternalSidechain
            && rawValue(parameters::ids::sidechainEnabled) >= 0.5f
            && meters.sidechainEnvelope.load(std::memory_order_relaxed) > 0.0005f;

        if (! externalSidechainIsAlreadyWorking && trigger != TriggerMode::StageMindLink)
            applyParameterValueFromCommand(parameters::ids::triggerMode, 3.0f);

        if (rawValue(parameters::ids::sidechainEnabled) < 0.5f)
            applyParameterValueFromCommand(parameters::ids::sidechainEnabled, 1.0f);

        if (rawValue(parameters::ids::sidechainAmount) < 0.35f)
            applyParameterValueFromCommand(parameters::ids::sidechainAmount, autoSidechainDefaultAmount);
    }

    writeDiagnosticEvent(
        "auto_link_action",
        "kind=" + juce::String(static_cast<int> (kind))
            + " source=" + juce::String(sourceInstanceId)
            + " width=" + juce::String(rawValue(parameters::ids::width), 3)
            + " depth=" + juce::String(rawValue(parameters::ids::depth), 3)
            + " sidechain_mode=" + juce::String(static_cast<int> (rawValue(parameters::ids::sidechainMode)))
            + " sidechain_amount=" + juce::String(rawValue(parameters::ids::sidechainAmount), 3));
    meters.autoAssistActionKind.store(static_cast<int> (kind), std::memory_order_relaxed);
}

bool PluginProcessor::isLinkActionAppliedForAuto(const LinkSuggestionAction& action) const noexcept
{
    if (! action.available)
        return false;

    if (action.setWidth && std::abs(rawValue(parameters::ids::width) - action.width) > autoParameterEpsilon)
        return false;

    if (action.setDepth && std::abs(rawValue(parameters::ids::depth) - action.depth) > autoParameterEpsilon)
        return false;

    if (action.setSidechainMode)
    {
        const auto currentSidechainMode = static_cast<int> (rawValue(parameters::ids::sidechainMode));
        const auto trigger = triggerModeFromIndex(static_cast<int> (rawValue(parameters::ids::triggerMode)));
        const auto sidechainEnabled = rawValue(parameters::ids::sidechainEnabled) >= 0.5f;
        if (isDuckingAction(action)
            && isManualDuckingBypassState(
                currentSidechainMode,
                static_cast<int> (trigger),
                sidechainEnabled))
        {
            return true;
        }

        const auto duckingAlreadyActive = isActiveDuckingState(
            currentSidechainMode,
            static_cast<int> (trigger),
            sidechainEnabled,
            rawValue(parameters::ids::sidechainAmount));

        if (isDuckingAction(action) && duckingAlreadyActive)
            return true;

        if (currentSidechainMode != action.sidechainModeIndex)
            return false;

        if (trigger != TriggerMode::StageMindLink && trigger != TriggerMode::ExternalSidechain)
            return false;

        if (rawValue(parameters::ids::sidechainEnabled) < 0.5f)
            return false;

        if (rawValue(parameters::ids::sidechainAmount) < 0.35f)
            return false;
    }

    return true;
}

void PluginProcessor::applyParameterValueFromCommand(const char* parameterId, float value)
{
    if (auto* parameter = apvts.getParameter(parameterId))
    {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
        parameter->endChangeGesture();
    }
}

UserMacroParams PluginProcessor::makeMacroParams() const noexcept
{
    UserMacroParams params;
    params.width = rawValue(parameters::ids::width);
    params.depth = rawValue(parameters::ids::depth);
    params.motion = rawValue(parameters::ids::motion);
    params.cleanUp = rawValue(parameters::ids::cleanUp);
    params.resonance = rawValue(parameters::ids::resonance);
    params.pan = rawValue(parameters::ids::pan);
    params.monoLowCutoffHz = rawValue(parameters::ids::monoLowCutoff);
    params.sideHighPassHz = rawValue(parameters::ids::sideHighPass);
    return params;
}

void PluginProcessor::updateOutputGainTarget() noexcept
{
    outputGain.setTargetValue(juce::Decibels::decibelsToGain(rawValue(parameters::ids::outputGain)));
}

void PluginProcessor::applyOutputGain(juce::AudioBuffer<float>& buffer) noexcept
{
    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = buffer.getNumChannels();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto gain = outputGain.getNextValue();

        for (int channel = 0; channel < numChannels; ++channel)
            buffer.getWritePointer(channel)[sample] *= gain;
    }
}

LevelRiderConfig PluginProcessor::makeLevelRiderConfig() noexcept
{
    LevelRiderConfig config;
    const auto selectedStageGainMode = stageGainModeFromIndex(static_cast<int> (rawValue(parameters::ids::stageGainMode)));
    const auto selectedMeterMode = stageGainMeterModeFromIndex(static_cast<int> (rawValue(parameters::ids::stageGainMeterMode)));
    const auto selectedAutoMode = autoAssistModeFromIndex(static_cast<int> (rawValue(parameters::ids::autoAssistMode)));
    config.enabled = selectedAutoMode == AutoAssistMode::Auto;
    config.activeThresholdRms = 0.004f;
    config.targetFloorRms = 0.018f;
    config.targetCeilingRms = 0.42f;
    config.maxBoostDb = 4.0f;
    config.maxCutDb = 5.0f;
    config.learnSeconds = 18.0f;

    if (selectedStageGainMode == StageGainMode::Off)
    {
        config.mode = config.enabled ? LevelRiderMode::LegacyAuto : LevelRiderMode::Off;
        config.ceilingDb = -0.2f;
        stageGainAnalyzeRequested.store(false, std::memory_order_release);
        return config;
    }

    config.mode = selectedStageGainMode == StageGainMode::Static ? LevelRiderMode::Static : LevelRiderMode::Ride;
    const auto targetDb = rawValue(parameters::ids::stageGainTargetDb);
    config.fixedTargetRms = StageGainLoudnessMeter::targetLevelFromDb(selectedMeterMode, targetDb);
    config.targetDisplayDb = targetDb;
    config.activeThresholdRms = StageGainLoudnessMeter::targetLevelFromDb(
        selectedMeterMode,
        targetDb + rawValue(parameters::ids::stageGainThresholdVu));
    config.ceilingDb = rawValue(parameters::ids::stageGainCeilingDb);
    config.response = rawValue(parameters::ids::stageGainResponse);
    config.maxBoostDb = 18.0f;
    config.maxCutDb = 24.0f;
    config.analyzeRequested = stageGainAnalyzeRequested.exchange(false, std::memory_order_acq_rel);
    return config;
}

void PluginProcessor::renderSidechainListen(
    const juce::AudioBuffer<float>& sidechainBuffer,
    juce::AudioBuffer<float>& mainBuffer) noexcept
{
    const auto numSamples = mainBuffer.getNumSamples();
    const auto mainChannels = mainBuffer.getNumChannels();
    const auto sidechainChannels = sidechainBuffer.getNumChannels();

    if (sidechainChannels == 0 || sidechainBuffer.getNumSamples() == 0)
    {
        mainBuffer.clear();
        return;
    }

    for (int channel = 0; channel < mainChannels; ++channel)
    {
        auto* destination = mainBuffer.getWritePointer(channel);

        if (mainChannels == 1 && sidechainChannels > 1)
        {
            const auto* left = sidechainBuffer.getReadPointer(0);
            const auto* right = sidechainBuffer.getReadPointer(1);

            for (int sample = 0; sample < numSamples; ++sample)
                destination[sample] = (left[sample] + right[sample]) * 0.5f;
        }
        else
        {
            const auto sourceChannel = juce::jmin(channel, sidechainChannels - 1);
            const auto* source = sidechainBuffer.getReadPointer(sourceChannel);
            std::copy(source, source + numSamples, destination);
        }
    }
}

SidechainAnalysis PluginProcessor::makeEffectiveSidechainAnalysis(const SidechainAnalysis& externalSidechain) const noexcept
{
    if (triggerModeFromIndex(static_cast<int> (rawValue(parameters::ids::triggerMode))) != TriggerMode::StageMindLink)
        return externalSidechain;

    SidechainAnalysis analysis;
    const auto autoSourceId = autoLinkSidechainSourceId.load(std::memory_order_relaxed);
    const auto linkReady =
        rawValue(parameters::ids::linkEnabled) >= 0.5f
        && rawValue(parameters::ids::linkGroup) > 0.0f
        && (autoSourceId > 0 || meters.linkPeerId.load(std::memory_order_relaxed) > 0)
        && meters.linkOfflineSuppressed.load(std::memory_order_relaxed) == 0;
    const auto activity = juce::jlimit(
        0.0f,
        1.0f,
        autoSourceId > 0
            ? autoLinkSidechainActivity.load(std::memory_order_relaxed)
            : meters.linkPeerActivity.load(std::memory_order_relaxed));

    analysis.rms = activity * 0.25f;
    analysis.peak = activity;
    analysis.envelope = activity;
    analysis.isActive = linkReady && activity > 0.02f;
    analysis.transientDetected = activity > 0.65f;
    return analysis;
}

SidechainDynamicEQConfig PluginProcessor::makeSidechainEQConfig() const noexcept
{
    SidechainDynamicEQConfig config;
    config.mode = sidechainConflictModeFromIndex(static_cast<int> (rawValue(parameters::ids::sidechainMode)));
    const auto trigger = triggerModeFromIndex(static_cast<int> (rawValue(parameters::ids::triggerMode)));
    config.enabled = (trigger == TriggerMode::ExternalSidechain || trigger == TriggerMode::StageMindLink)
        && rawValue(parameters::ids::sidechainEnabled) >= 0.5f
        && config.mode != SidechainConflictMode::Off;
    config.amount = rawValue(parameters::ids::sidechainAmount);
    config.attackMs = rawValue(parameters::ids::sidechainAttack);
    config.releaseMs = rawValue(parameters::ids::sidechainRelease);
    config.customRangeStartHz = rawValue(parameters::ids::sidechainRangeStart);
    config.customRangeEndHz = rawValue(parameters::ids::sidechainRangeEnd);
    return config;
}

CleanUpConfig PluginProcessor::makeCleanUpConfig(const RoleProfile& profile, TrackRole role) const noexcept
{
    CleanUpConfig config;
    config.amount = rawValue(parameters::ids::cleanUp);
    config.lowMidReduction = 0.12f + profile.cleanUpAmount * 0.18f;
    config.harshReduction = 0.10f + profile.resonanceSensitivity * 0.18f;
    config.airLift = profile.protectLowEnd ? 0.0f : 0.025f + profile.cleanUpAmount * 0.055f;

    switch (role)
    {
        case TrackRole::LeadVocal:
        case TrackRole::BackingVocal:
        case TrackRole::SunoVocal:
            config.lowMidReduction = 0.28f;
            config.harshReduction = 0.31f;
            config.airLift = 0.075f;
            config.lowMidStartHz = 130.0f;
            config.lowMidEndHz = 760.0f;
            config.harshStartHz = 2200.0f;
            config.harshEndHz = 8200.0f;
            config.airStartHz = 7600.0f;
            break;

        case TrackRole::Bass:
        case TrackRole::SynthBass:
        case TrackRole::SunoBass:
        case TrackRole::Kick:
            config.lowMidReduction = 0.15f;
            config.harshReduction = 0.075f;
            config.airLift = 0.0f;
            config.lowMidStartHz = 170.0f;
            config.lowMidEndHz = 520.0f;
            config.harshStartHz = 1200.0f;
            config.harshEndHz = 4200.0f;
            config.airStartHz = 9000.0f;
            break;

        case TrackRole::RhythmGuitarSingle:
        case TrackRole::RhythmGuitarPairLeft:
        case TrackRole::RhythmGuitarPairRight:
        case TrackRole::LeadGuitar:
        case TrackRole::SunoGuitar:
        case TrackRole::Piano:
            config.lowMidReduction = 0.30f;
            config.harshReduction = 0.25f;
            config.airLift = 0.050f;
            config.lowMidStartHz = 170.0f;
            config.lowMidEndHz = 920.0f;
            config.harshStartHz = 2500.0f;
            config.harshEndHz = 7800.0f;
            break;

        case TrackRole::Snare:
        case TrackRole::HiHat:
        case TrackRole::Percussion:
        case TrackRole::SunoDrums:
        case TrackRole::SunoPercussion:
            config.lowMidReduction = 0.17f;
            config.harshReduction = 0.22f;
            config.airLift = 0.040f;
            config.lowMidStartHz = 180.0f;
            config.lowMidEndHz = 820.0f;
            config.harshStartHz = 2800.0f;
            config.harshEndHz = 9000.0f;
            break;

        case TrackRole::Pad:
        case TrackRole::Atmosphere:
        case TrackRole::FX:
        case TrackRole::SunoSynthPad:
        case TrackRole::SunoFX:
            config.lowMidReduction = 0.20f;
            config.harshReduction = 0.17f;
            config.airLift = 0.075f;
            config.lowMidStartHz = 160.0f;
            config.lowMidEndHz = 780.0f;
            config.harshStartHz = 3000.0f;
            config.harshEndHz = 8600.0f;
            break;

        default:
            break;
    }

    if (profile.protectTransient)
        config.harshReduction *= 0.82f;

    if (profile.protectLowEnd)
    {
        config.lowMidReduction *= 0.78f;
        config.airLift = 0.0f;
    }

    return config;
}

ResonanceDetectorConfig PluginProcessor::makeResonanceDetectorConfig() const noexcept
{
    ResonanceDetectorConfig config;
    config.cleanUp = rawValue(parameters::ids::cleanUp);
    config.sensitivity = rawValue(parameters::ids::resonanceSensitivity);
    config.maxReductionDb = rawValue(parameters::ids::maxResonanceReduction);
    return config;
}

ResonanceSuppressionConfig PluginProcessor::makeResonanceSuppressionConfig() const noexcept
{
    const auto cleanUp = rawValue(parameters::ids::cleanUp);
    const auto resonance = rawValue(parameters::ids::resonance);
    const auto characterDrive = resonance * (1.0f + resonance * 0.45f + cleanUp * 0.35f);
    ResonanceSuppressionConfig config;
    config.resonanceAmount = juce::jlimit(0.0f, 1.60f, characterDrive);
    config.maxReductionDb = juce::jlimit(
        0.0f,
        10.0f,
        rawValue(parameters::ids::maxResonanceReduction) + cleanUp * 1.8f + resonance * 1.1f);
    config.attackMs = juce::jmax(1.0f, rawValue(parameters::ids::dynamicEqAttack) * juce::jmap(resonance, 0.0f, 1.0f, 1.0f, 0.50f));
    config.releaseMs = juce::jmax(24.0f, rawValue(parameters::ids::dynamicEqRelease) * juce::jmap(cleanUp, 0.0f, 1.0f, 1.0f, 0.72f));
    return config;
}

MotionConfig PluginProcessor::makeMotionConfig(const RoleProfile& profile, SafetyMode safety) const noexcept
{
    MotionConfig config;
    if (! profile.allowMotion)
        return config;

    config.amount = juce::jlimit(
        0.0f,
        1.0f,
        rawValue(parameters::ids::motion) * safetyMotionScale(safety) * correlationSafetyScale);
    config.rateHz = rawValue(parameters::ids::motionRate);
    config.preset = static_cast<int> (rawValue(parameters::ids::motionPreset));
    return config;
}

DepthConfig PluginProcessor::makeDepthConfig(const RoleProfile& profile, SafetyMode safety) const noexcept
{
    const auto centerScale = profile.keepDryCenter ? 0.70f : 1.0f;
    const auto depthAmount = juce::jlimit(
        0.0f,
        1.0f,
        rawValue(parameters::ids::depth) * safetyDepthScale(safety) * centerScale);

    DepthConfig config;
    config.amount = depthAmount;
    config.presenceReduction = juce::jlimit(
        0.0f,
        0.90f,
        depthAmount * (0.30f + rawValue(parameters::ids::presenceReduction) * 0.48f));
    config.earlyReflectionAmount = juce::jlimit(
        0.0f,
        1.0f,
        0.48f + rawValue(parameters::ids::earlyReflectionAmount) * 0.52f);
    return config;
}

PseudoDoubleConfig PluginProcessor::makePseudoDoubleConfig(const RoleProfile& profile, SafetyMode safety) const noexcept
{
    PseudoDoubleConfig config;
    if (! profile.allowPseudoDouble)
        return config;

    config.amount = juce::jlimit(
        0.0f,
        1.0f,
        rawValue(parameters::ids::pseudoDoubleAmount) * safetyDoubleScale(safety) * correlationSafetyScale);
    return config;
}

void PluginProcessor::applyCorrelationSafety(SpatialParams& params) const noexcept
{
    if (params.widthAmount > 1.0f)
        params.widthAmount = 1.0f + (params.widthAmount - 1.0f) * correlationSafetyScale;
}

void PluginProcessor::publishStageState(
    const SpatialParams& spatialParams,
    const DepthConfig& depthConfig,
    const MotionConfig& motionConfig,
    bool motionAllowed) noexcept
{
    meters.stagePan.store(spatialParams.pan, std::memory_order_relaxed);
    meters.stageDepth.store(depthConfig.amount, std::memory_order_relaxed);
    meters.stageWidth.store(normalizedWidthAmount(spatialParams.widthAmount), std::memory_order_relaxed);
    meters.stageMotion.store(motionConfig.amount, std::memory_order_relaxed);
    meters.stageMotionAllowed.store(motionAllowed ? 1 : 0, std::memory_order_relaxed);
}

void PluginProcessor::updateCorrelationSafety(float correlation, float threshold) noexcept
{
    auto target = 1.0f;
    if (correlation < threshold)
        target = juce::jlimit(0.25f, 1.0f, 1.0f - (threshold - correlation) * 1.6f);

    const auto coefficient = target < correlationSafetyScale ? 0.16f : 0.04f;
    correlationSafetyScale += (target - correlationSafetyScale) * coefficient;
}

void PluginProcessor::publishLinkState(
    TrackRole role,
    std::pair<float, float> inputLevels,
    std::pair<float, float> outputLevels,
    float correlation,
    float sidechainEnvelope,
    LinkSpectralBands bands,
    float effectivePan,
    int numSamples) noexcept
{
    const auto linkEnabled = rawValue(parameters::ids::linkEnabled) >= 0.5f;
    const auto group = static_cast<int> (rawValue(parameters::ids::linkGroup));
    const auto sourceId = static_cast<int> (rawValue(parameters::ids::linkSourceId));
    const auto targetId = static_cast<int> (rawValue(parameters::ids::linkTargetId));
    const auto linkMode = static_cast<int> (rawValue(parameters::ids::linkMode));
    const auto preferredSourceRole = roleFromIndexWithUnknown(static_cast<int> (rawValue(parameters::ids::linkRole)));
    const auto activity = juce::jlimit(
        0.0f,
        1.0f,
        std::max({ inputLevels.first * 2.0f, outputLevels.first * 2.0f, sidechainEnvelope }));
    const auto smoothedActivity = linkEnabled
        ? linkActivityEnvelope.process(activity, numSamples)
        : 0.0f;

    if (! linkEnabled)
    {
        linkActivityEnvelope.reset();
        linkSpectralAnalyzer.reset();
    }

    LinkPublishState state;
    state.enabled = linkEnabled;
    state.nonRealtime = isNonRealtime();
    state.group = group;
    state.role = static_cast<int> (role);
    state.sourceId = sourceId;
    state.targetId = targetId;
    state.mode = linkMode;
    state.sidechainMode = static_cast<int> (rawValue(parameters::ids::sidechainMode));
    state.triggerMode = static_cast<int> (rawValue(parameters::ids::triggerMode));
    state.stageGainMode = static_cast<int> (rawValue(parameters::ids::stageGainMode));
    state.autoAssistMode = static_cast<int> (rawValue(parameters::ids::autoAssistMode));
    state.sidechainEnabled = rawValue(parameters::ids::sidechainEnabled) >= 0.5f;
    state.activity = smoothedActivity;
    state.inputRms = inputLevels.first;
    state.outputRms = outputLevels.first;
    state.sidechainEnvelope = sidechainEnvelope;
    state.sidechainAmount = rawValue(parameters::ids::sidechainAmount);
    state.correlation = correlation;
    state.pan = effectivePan;
    state.width = rawValue(parameters::ids::width);
    state.depth = rawValue(parameters::ids::depth);
    state.motion = rawValue(parameters::ids::motion);
    state.outputTrimDb = rawValue(parameters::ids::outputGain);
    state.stageGainDb = meters.levelRideGainDb.load(std::memory_order_relaxed);
    state.cleanUp = rawValue(parameters::ids::cleanUp);
    state.resonance = rawValue(parameters::ids::resonance);
    state.bands = bands;

    auto& registry = StageMindLinkRegistry::instance();
    registry.publish(linkHandle, state);

    LinkReadQuery query;
    query.group = group;
    query.preferredSourceId = static_cast<std::uint32_t> (juce::jlimit(0, maxLinkInstances, sourceId));
    query.preferredSourceRole = static_cast<int> (preferredSourceRole);

    const auto canRead = state.enabled && ! state.nonRealtime && group > 0;
    const auto peer = canRead ? registry.readPeer(linkHandle, query) : LinkPeerSnapshot {};
    const auto groupSnapshot = canRead ? registry.readGroup(group) : LinkGroupSnapshot {};
    const auto peerCount = canRead ? registry.countActivePeers(linkHandle, group) : 0;
    const auto nodeCount = canRead ? peerCount + 1 : (linkEnabled && group > 0 ? 1 : 0);
    const auto offlineSuppressed = linkEnabled && group > 0 && state.nonRealtime;

    meters.linkEnabled.store(canRead ? 1 : 0, std::memory_order_relaxed);
    meters.linkInstanceId.store(static_cast<int> (linkHandle.instanceId), std::memory_order_relaxed);
    meters.linkGroup.store(group, std::memory_order_relaxed);
    meters.linkActivePeers.store(peerCount, std::memory_order_relaxed);
    meters.linkNodeCount.store(nodeCount, std::memory_order_relaxed);
    meters.linkOfflineSuppressed.store(offlineSuppressed ? 1 : 0, std::memory_order_relaxed);
    meters.linkPeerId.store(peer.found ? static_cast<int> (peer.instanceId) : 0, std::memory_order_relaxed);
    meters.linkPeerRole.store(peer.found ? peer.role : 0, std::memory_order_relaxed);
    meters.linkPeerActivity.store(peer.found ? peer.activity : 0.0f, std::memory_order_relaxed);
    meters.linkPeerCorrelation.store(peer.found ? peer.correlation : 1.0f, std::memory_order_relaxed);
    meters.linkPeerWidth.store(peer.found ? peer.width : 0.0f, std::memory_order_relaxed);
    meters.linkPeerDepth.store(peer.found ? peer.depth : 0.0f, std::memory_order_relaxed);
    meters.linkPeerCleanUp.store(peer.found ? peer.cleanUp : 0.0f, std::memory_order_relaxed);
    meters.linkPeerResonance.store(peer.found ? peer.resonance : 0.0f, std::memory_order_relaxed);
    meters.linkBandLow.store(bands.low, std::memory_order_relaxed);
    meters.linkBandLowMid.store(bands.lowMid, std::memory_order_relaxed);
    meters.linkBandPresence.store(bands.presence, std::memory_order_relaxed);
    meters.linkBandAir.store(bands.air, std::memory_order_relaxed);
    meters.linkPeerBandLow.store(peer.found ? peer.bands.low : 0.0f, std::memory_order_relaxed);
    meters.linkPeerBandLowMid.store(peer.found ? peer.bands.lowMid : 0.0f, std::memory_order_relaxed);
    meters.linkPeerBandPresence.store(peer.found ? peer.bands.presence : 0.0f, std::memory_order_relaxed);
    meters.linkPeerBandAir.store(peer.found ? peer.bands.air : 0.0f, std::memory_order_relaxed);

    linkAudioPublishFreshTicks.store(4, std::memory_order_relaxed);
    updateAutoLinkAssist(role, correlation, bands, groupSnapshot, preferredSourceRole, canRead && groupSnapshot.count > 1, numSamples);
}

void PluginProcessor::publishIdleLinkState() noexcept
{
    if (isDirectorMode())
    {
        publishDisabledLinkState();
        return;
    }

    const auto linkEnabled = rawValue(parameters::ids::linkEnabled) >= 0.5f;
    const auto group = static_cast<int> (rawValue(parameters::ids::linkGroup));
    const auto sourceId = static_cast<int> (rawValue(parameters::ids::linkSourceId));
    const auto targetId = static_cast<int> (rawValue(parameters::ids::linkTargetId));
    const auto linkMode = static_cast<int> (rawValue(parameters::ids::linkMode));
    const auto preferredSourceRole = roleFromIndexWithUnknown(static_cast<int> (rawValue(parameters::ids::linkRole)));

    LinkPublishState state;
    state.enabled = linkEnabled;
    state.nonRealtime = isNonRealtime();
    state.group = group;
    const auto idleRole = roleFromSelectableIndex(static_cast<int> (rawValue(parameters::ids::role)));
    const auto idleSafety = safetyModeFromIndex(static_cast<int> (rawValue(parameters::ids::safety)));
    const auto idleSpatialParams = rolePresetEngine.buildSpatialParams(idleRole, idleSafety, makeMacroParams());

    state.role = static_cast<int> (idleRole);
    state.sourceId = sourceId;
    state.targetId = targetId;
    state.mode = linkMode;
    state.sidechainMode = static_cast<int> (rawValue(parameters::ids::sidechainMode));
    state.triggerMode = static_cast<int> (rawValue(parameters::ids::triggerMode));
    state.stageGainMode = static_cast<int> (rawValue(parameters::ids::stageGainMode));
    state.autoAssistMode = static_cast<int> (rawValue(parameters::ids::autoAssistMode));
    state.sidechainEnabled = rawValue(parameters::ids::sidechainEnabled) >= 0.5f;
    state.activity = 0.0f;
    state.inputRms = 0.0f;
    state.outputRms = 0.0f;
    state.sidechainEnvelope = 0.0f;
    state.sidechainAmount = rawValue(parameters::ids::sidechainAmount);
    state.correlation = 1.0f;
    state.pan = idleSpatialParams.pan;
    state.width = rawValue(parameters::ids::width);
    state.depth = rawValue(parameters::ids::depth);
    state.motion = rawValue(parameters::ids::motion);
    state.outputTrimDb = rawValue(parameters::ids::outputGain);
    state.stageGainDb = 0.0f;
    state.cleanUp = rawValue(parameters::ids::cleanUp);
    state.resonance = rawValue(parameters::ids::resonance);

    auto& registry = StageMindLinkRegistry::instance();
    registry.publish(linkHandle, state);

    LinkReadQuery query;
    query.group = group;
    query.preferredSourceId = static_cast<std::uint32_t> (juce::jlimit(0, maxLinkInstances, sourceId));
    query.preferredSourceRole = static_cast<int> (preferredSourceRole);

    const auto canRead = state.enabled && ! state.nonRealtime && group > 0;
    const auto peer = canRead ? registry.readPeer(linkHandle, query) : LinkPeerSnapshot {};
    const auto peerCount = canRead ? registry.countActivePeers(linkHandle, group) : 0;
    const auto nodeCount = canRead ? peerCount + 1 : (linkEnabled && group > 0 ? 1 : 0);
    const auto offlineSuppressed = linkEnabled && group > 0 && state.nonRealtime;

    meters.linkEnabled.store(canRead ? 1 : 0, std::memory_order_relaxed);
    meters.linkInstanceId.store(static_cast<int> (linkHandle.instanceId), std::memory_order_relaxed);
    meters.linkGroup.store(group, std::memory_order_relaxed);
    meters.linkActivePeers.store(peerCount, std::memory_order_relaxed);
    meters.linkNodeCount.store(nodeCount, std::memory_order_relaxed);
    meters.linkOfflineSuppressed.store(offlineSuppressed ? 1 : 0, std::memory_order_relaxed);
    meters.linkPeerId.store(peer.found ? static_cast<int> (peer.instanceId) : 0, std::memory_order_relaxed);
    meters.linkPeerRole.store(peer.found ? peer.role : 0, std::memory_order_relaxed);
    meters.linkPeerActivity.store(peer.found ? peer.activity : 0.0f, std::memory_order_relaxed);
    meters.linkPeerCorrelation.store(peer.found ? peer.correlation : 1.0f, std::memory_order_relaxed);
    meters.linkPeerWidth.store(peer.found ? peer.width : 0.0f, std::memory_order_relaxed);
    meters.linkPeerDepth.store(peer.found ? peer.depth : 0.0f, std::memory_order_relaxed);
    meters.linkPeerCleanUp.store(peer.found ? peer.cleanUp : 0.0f, std::memory_order_relaxed);
    meters.linkPeerResonance.store(peer.found ? peer.resonance : 0.0f, std::memory_order_relaxed);
    meters.linkBandLow.store(0.0f, std::memory_order_relaxed);
    meters.linkBandLowMid.store(0.0f, std::memory_order_relaxed);
    meters.linkBandPresence.store(0.0f, std::memory_order_relaxed);
    meters.linkBandAir.store(0.0f, std::memory_order_relaxed);
    meters.linkPeerBandLow.store(peer.found ? peer.bands.low : 0.0f, std::memory_order_relaxed);
    meters.linkPeerBandLowMid.store(peer.found ? peer.bands.lowMid : 0.0f, std::memory_order_relaxed);
    meters.linkPeerBandPresence.store(peer.found ? peer.bands.presence : 0.0f, std::memory_order_relaxed);
    meters.linkPeerBandAir.store(peer.found ? peer.bands.air : 0.0f, std::memory_order_relaxed);
}

void PluginProcessor::publishDisabledLinkState() noexcept
{
    LinkPublishState state;
    state.enabled = false;
    StageMindLinkRegistry::instance().publish(linkHandle, state);

    linkActivityEnvelope.reset();
    linkSpectralAnalyzer.reset();
    meters.linkEnabled.store(0, std::memory_order_relaxed);
    meters.linkGroup.store(0, std::memory_order_relaxed);
    meters.linkActivePeers.store(0, std::memory_order_relaxed);
    meters.linkNodeCount.store(0, std::memory_order_relaxed);
    meters.linkOfflineSuppressed.store(0, std::memory_order_relaxed);
    meters.linkPeerId.store(0, std::memory_order_relaxed);
    meters.linkPeerRole.store(0, std::memory_order_relaxed);
    meters.linkPeerActivity.store(0.0f, std::memory_order_relaxed);
    meters.linkPeerCorrelation.store(1.0f, std::memory_order_relaxed);
    meters.linkPeerWidth.store(0.0f, std::memory_order_relaxed);
    meters.linkPeerDepth.store(0.0f, std::memory_order_relaxed);
    meters.linkPeerCleanUp.store(0.0f, std::memory_order_relaxed);
    meters.linkPeerResonance.store(0.0f, std::memory_order_relaxed);
    meters.linkBandLow.store(0.0f, std::memory_order_relaxed);
    meters.linkBandLowMid.store(0.0f, std::memory_order_relaxed);
    meters.linkBandPresence.store(0.0f, std::memory_order_relaxed);
    meters.linkBandAir.store(0.0f, std::memory_order_relaxed);
    meters.linkPeerBandLow.store(0.0f, std::memory_order_relaxed);
    meters.linkPeerBandLowMid.store(0.0f, std::memory_order_relaxed);
    meters.linkPeerBandPresence.store(0.0f, std::memory_order_relaxed);
    meters.linkPeerBandAir.store(0.0f, std::memory_order_relaxed);
    resetAutoLinkTracking();
}

bool PluginProcessor::isDirectorMode() const noexcept
{
    return pluginModeFromIndex(static_cast<int> (rawValue(parameters::ids::pluginMode))) == PluginMode::Director;
}

void PluginProcessor::updateAutoLinkAssist(
    TrackRole role,
    float correlation,
    LinkSpectralBands bands,
    const LinkGroupSnapshot& groupSnapshot,
    TrackRole preferredSourceRole,
    bool canReadGroup,
    int numSamples) noexcept
{
    const auto mode = autoAssistModeFromIndex(static_cast<int> (rawValue(parameters::ids::autoAssistMode)));
    if (mode != AutoAssistMode::Auto || ! canReadGroup)
    {
        resetAutoLinkTracking();
        return;
    }

    const auto currentAutoSourceId = static_cast<std::uint32_t> (autoLinkSidechainSourceId.load(std::memory_order_relaxed));
    auto currentAutoSourceFound = false;
    LinkSuggestion bestSuggestion;
    LinkSuggestionAction bestAction;
    LinkPeerSnapshot bestPeer;
    auto bestScore = -1.0f;

    for (int index = 0; index < groupSnapshot.count; ++index)
    {
        const auto& peer = groupSnapshot.peers[static_cast<size_t> (index)];
        if (! peer.found || peer.instanceId == linkHandle.instanceId)
            continue;

        if (! targetAllowsReader(peer, linkHandle.instanceId))
            continue;

        if (preferredSourceRole != TrackRole::Unknown
            && peer.role != static_cast<int> (preferredSourceRole))
        {
            continue;
        }

        if (peer.instanceId == currentAutoSourceId)
        {
            autoLinkSidechainActivity.store(peer.activity, std::memory_order_relaxed);
            currentAutoSourceFound = true;
        }

        LinkSuggestionInput input;
        input.linkActive = true;
        input.peerFound = true;
        input.currentRole = role;
        input.peerRole = static_cast<TrackRole> (peer.role);
        input.currentWidth = rawValue(parameters::ids::width);
        input.currentDepth = rawValue(parameters::ids::depth);
        input.currentCorrelation = correlation;
        input.peerActivity = peer.activity;
        input.peerWidth = peer.width;
        input.currentBands = bands;
        input.peerBands = peer.bands;

        const auto suggestion = LinkSuggestionEngine::evaluate(input);
        if (! suggestion.hasSuggestion())
            continue;

        const auto action = LinkSuggestionEngine::actionFor(suggestion.kind);
        if (! action.available)
            continue;

        const auto score = autoSuggestionScore(suggestion.kind, suggestion.severity);
        if (score > bestScore)
        {
            bestScore = score;
            bestSuggestion = suggestion;
            bestAction = action;
            bestPeer = peer;
        }
    }

    if (currentAutoSourceId != invalidLinkInstanceId && ! currentAutoSourceFound)
        autoLinkSidechainActivity.store(0.0f, std::memory_order_relaxed);

    if (! bestSuggestion.hasSuggestion())
    {
        autoLinkCandidateKind = 0;
        autoLinkCandidateSourceId = 0;
        autoLinkCandidateSamples = 0;
        return;
    }

    if (isLinkActionAppliedForAuto(bestAction))
    {
        meters.autoAssistActionKind.store(static_cast<int> (bestSuggestion.kind), std::memory_order_relaxed);
        autoLinkCandidateKind = 0;
        autoLinkCandidateSourceId = 0;
        autoLinkCandidateSamples = 0;
        return;
    }

    if (bestAction.setSidechainMode)
    {
        autoLinkSidechainSourceId.store(static_cast<int> (bestPeer.instanceId), std::memory_order_relaxed);
        autoLinkSidechainActivity.store(bestPeer.activity, std::memory_order_relaxed);
    }

    const auto kind = static_cast<int> (bestSuggestion.kind);
    const auto sourceId = static_cast<int> (bestPeer.instanceId);
    if (autoLinkCandidateKind != kind || autoLinkCandidateSourceId != sourceId)
    {
        autoLinkCandidateKind = kind;
        autoLinkCandidateSourceId = sourceId;
        autoLinkCandidateSamples = 0;
    }

    const auto stableSampleTarget = std::max(1, static_cast<int> (currentSampleRate * autoLinkStableSeconds));
    autoLinkCandidateSamples = std::min(autoLinkCandidateSamples + std::max(0, numSamples), stableSampleTarget);

    if (autoLinkCandidateSamples >= stableSampleTarget)
    {
        autoPendingLinkActionKind.store(kind, std::memory_order_release);
        autoPendingLinkSourceId.store(sourceId, std::memory_order_release);
        autoLinkCandidateSamples = 0;
    }
}

void PluginProcessor::updateAutoAssistBeforeResonance(TrackRole role, std::pair<float, float> inputLevels, int numSamples) noexcept
{
    const auto mode = autoAssistModeFromIndex(static_cast<int> (rawValue(parameters::ids::autoAssistMode)));
    if (mode == AutoAssistMode::Off)
    {
        resetAutoAssistTracking();
        publishAutoAssistState(0, 0.0f);
        return;
    }

    if (mode == AutoAssistMode::Suggest)
    {
        publishAutoAssistState(1, 0.0f);
        return;
    }

    const auto linkEnabled = rawValue(parameters::ids::linkEnabled) >= 0.5f;
    const auto group = juce::jlimit(0, 16, static_cast<int> (rawValue(parameters::ids::linkGroup)));
    const auto roleIndex = static_cast<int> (role);

    if (group != autoTrackedGroup || roleIndex != autoTrackedRole)
    {
        autoTrackedGroup = group;
        autoTrackedRole = roleIndex;
        autoStableSamples = 0;
        autoResonanceStarted = resonanceLearner.hasLearnedSnapshot();
    }

    if (resonanceLearner.getStatus() == ResonanceLearner::Status::Learning)
    {
        publishAutoAssistState(2, resonanceLearner.getProgress());
        return;
    }

    if (resonanceLearner.hasLearnedSnapshot())
    {
        autoResonanceStarted = true;
        publishAutoAssistState(3, 1.0f);
        return;
    }

    const auto offlineSuppressed = meters.linkOfflineSuppressed.load(std::memory_order_relaxed) != 0;
    const auto nodeCount = meters.linkNodeCount.load(std::memory_order_relaxed);
    const auto groupReady = linkEnabled && group > 0 && nodeCount >= 2 && ! offlineSuppressed;
    const auto signalReady = inputLevels.first >= autoAnalyzeSignalThreshold;

    if (! groupReady || ! signalReady || autoResonanceStarted)
    {
        autoStableSamples = 0;
        publishAutoAssistState(1, 0.0f);
        return;
    }

    const auto stableSampleTarget = std::max(1, static_cast<int> (currentSampleRate * autoAnalyzeStableSeconds));
    autoStableSamples = std::min(autoStableSamples + std::max(0, numSamples), stableSampleTarget);
    const auto progress = juce::jlimit(0.0f, 1.0f, static_cast<float> (autoStableSamples) / static_cast<float> (stableSampleTarget));
    publishAutoAssistState(1, progress);

    if (progress >= 1.0f)
    {
        resonanceLearner.beginLearn();
        autoResonanceStarted = true;
        autoResonanceTuningPending.store(true, std::memory_order_release);
        publishAutoAssistState(2, 0.0f);
    }
}

void PluginProcessor::publishAutoAssistState(int state, float progress) noexcept
{
    meters.autoAssistState.store(state, std::memory_order_relaxed);
    meters.autoAssistProgress.store(juce::jlimit(0.0f, 1.0f, progress), std::memory_order_relaxed);
}

void PluginProcessor::resetAutoAssistTracking() noexcept
{
    autoStableSamples = 0;
    resetAutoLinkTracking();
    autoTrackedGroup = juce::jlimit(0, 16, static_cast<int> (rawValue(parameters::ids::linkGroup)));
    autoTrackedRole = static_cast<int> (roleFromSelectableIndex(static_cast<int> (rawValue(parameters::ids::role))));
    autoResonanceStarted = resonanceLearner.hasLearnedSnapshot();
}

void PluginProcessor::resetAutoLinkTracking() noexcept
{
    autoLinkCandidateKind = 0;
    autoLinkCandidateSourceId = 0;
    autoLinkCandidateSamples = 0;
    autoPendingLinkActionKind.store(0, std::memory_order_relaxed);
    autoPendingLinkSourceId.store(0, std::memory_order_relaxed);
    autoLinkSidechainSourceId.store(0, std::memory_order_relaxed);
    autoLinkSidechainActivity.store(0.0f, std::memory_order_relaxed);
    meters.autoAssistActionKind.store(0, std::memory_order_relaxed);
}

void PluginProcessor::applyFactoryPreset(int index)
{
    const auto safeIndex = juce::jlimit(0, getNumPrograms() - 1, index);
    const auto& preset = factoryPresets[static_cast<size_t> (safeIndex)];
    currentProgram = safeIndex;

    setParameterValue(apvts, parameters::ids::role, static_cast<float> (selectableIndexForRole(preset.role)));
    setParameterValue(apvts, parameters::ids::safety, static_cast<float> (static_cast<int> (preset.safety)));
    setParameterValue(apvts, parameters::ids::width, preset.width);
    setParameterValue(apvts, parameters::ids::depth, preset.depth);
    setParameterValue(apvts, parameters::ids::motion, preset.motion);
    setParameterValue(apvts, parameters::ids::cleanUp, preset.cleanUp);
    setParameterValue(apvts, parameters::ids::resonance, preset.resonance);
    setParameterValue(apvts, parameters::ids::pseudoDoubleAmount, preset.pseudoDouble);
    setParameterValue(apvts, parameters::ids::presenceReduction, preset.presence);
    setParameterValue(apvts, parameters::ids::earlyReflectionAmount, preset.earlyRef);
}

void PluginProcessor::publishResonanceLearnState() noexcept
{
    meters.resonanceLearnState.store(static_cast<int> (resonanceLearner.getStatus()), std::memory_order_relaxed);
    meters.resonanceLearnProgress.store(resonanceLearner.getProgress(), std::memory_order_relaxed);
}

void PluginProcessor::publishResonances(const ResonanceSnapshot& snapshot) noexcept
{
    const auto count = juce::jmin<int>(snapshot.peakCount, static_cast<int> (maxResonancePeaks));
    meters.resonances.peakCount.store(count, std::memory_order_relaxed);

    for (int i = 0; i < static_cast<int> (maxResonancePeaks); ++i)
    {
        const auto index = static_cast<size_t> (i);
        const auto peak = i < count ? snapshot.peaks[index] : ResonancePeak {};

        meters.resonances.frequencyHz[index].store(peak.frequencyHz, std::memory_order_relaxed);
        meters.resonances.severity[index].store(peak.severity, std::memory_order_relaxed);
        meters.resonances.reductionDb[index].store(peak.suggestedReductionDb, std::memory_order_relaxed);
        meters.resonances.q[index].store(peak.suggestedQ, std::memory_order_relaxed);
    }
}

void PluginProcessor::publishLearnedResonances(const ResonanceSnapshot& snapshot) noexcept
{
    const auto count = juce::jmin<int>(snapshot.peakCount, static_cast<int> (maxResonancePeaks));
    meters.learnedResonances.peakCount.store(count, std::memory_order_relaxed);

    for (int i = 0; i < static_cast<int> (maxResonancePeaks); ++i)
    {
        const auto index = static_cast<size_t> (i);
        const auto peak = i < count ? snapshot.peaks[index] : ResonancePeak {};

        meters.learnedResonances.frequencyHz[index].store(peak.frequencyHz, std::memory_order_relaxed);
        meters.learnedResonances.severity[index].store(peak.severity, std::memory_order_relaxed);
        meters.learnedResonances.reductionDb[index].store(peak.suggestedReductionDb, std::memory_order_relaxed);
        meters.learnedResonances.q[index].store(peak.suggestedQ, std::memory_order_relaxed);
    }
}

ResonanceSnapshot PluginProcessor::readLearnedResonanceSnapshot() const noexcept
{
    ResonanceSnapshot snapshot;
    const auto count = juce::jmin<int>(
        meters.learnedResonances.peakCount.load(std::memory_order_relaxed),
        static_cast<int> (maxResonancePeaks));
    snapshot.peakCount = static_cast<uint8_t> (std::max(0, count));

    for (int i = 0; i < static_cast<int> (snapshot.peakCount); ++i)
    {
        const auto index = static_cast<size_t> (i);
        auto& peak = snapshot.peaks[index];
        peak.frequencyHz = meters.learnedResonances.frequencyHz[index].load(std::memory_order_relaxed);
        peak.severity = meters.learnedResonances.severity[index].load(std::memory_order_relaxed);
        peak.suggestedReductionDb = meters.learnedResonances.reductionDb[index].load(std::memory_order_relaxed);
        peak.suggestedQ = meters.learnedResonances.q[index].load(std::memory_order_relaxed);
    }

    return snapshot;
}

juce::AudioBuffer<float> PluginProcessor::getOptionalSidechainBuffer(juce::AudioBuffer<float>& buffer) noexcept
{
    if (getBusCount(true) <= 1)
        return {};

    if (const auto* sidechainBus = getBus(true, 1); sidechainBus == nullptr || ! sidechainBus->isEnabled())
        return {};

    return getBusBuffer(buffer, true, 1);
}
} // namespace stagemind

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new stagemind::PluginProcessor();
}
