#include "../Source/DSP/CleanUpProcessor.h"
#include "../Source/DSP/DepthProcessor.h"
#include "../Source/DSP/LevelRider.h"
#include "../Source/DSP/MidSideProcessor.h"
#include "../Source/DSP/MotionProcessor.h"
#include "../Source/DSP/PseudoDoubleProcessor.h"
#include "../Source/DSP/StageGainLoudnessMeter.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
constexpr double testSampleRate = 48000.0;
constexpr int testBlockSize = 2048;
constexpr float twoPi = juce::MathConstants<float>::twoPi;

void expect(bool condition, const char* message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

void fillOppositeSideSignal(juce::AudioBuffer<float>& buffer)
{
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        buffer.setSample(0, sample, 0.5f);
        buffer.setSample(1, sample, -0.5f);
    }
}

void fillCenteredSignal(juce::AudioBuffer<float>& buffer)
{
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        buffer.setSample(0, sample, 0.5f);
        buffer.setSample(1, sample, 0.5f);
    }
}

void fillCenteredSine(juce::AudioBuffer<float>& buffer, float frequencyHz, int startSample, float gain = 0.5f)
{
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto phase = twoPi * frequencyHz * static_cast<float> (startSample + sample) / static_cast<float> (testSampleRate);
        const auto value = std::sin(phase) * gain;
        buffer.setSample(0, sample, value);
        buffer.setSample(1, sample, value);
    }
}

std::pair<float, float> rmsAndPeak(const juce::AudioBuffer<float>& buffer)
{
    double sumSquares = 0.0;
    auto count = 0;
    float peak = 0.0f;

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        const auto* samples = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            const auto value = samples[sample];
            sumSquares += static_cast<double> (value) * static_cast<double> (value);
            peak = std::max(peak, std::abs(value));
            ++count;
        }
    }

    if (count == 0)
        return {};

    return { static_cast<float> (std::sqrt(sumSquares / static_cast<double> (count))), peak };
}

float maxDifference(const juce::AudioBuffer<float>& first, const juce::AudioBuffer<float>& second)
{
    float maximum = 0.0f;

    for (int channel = 0; channel < first.getNumChannels(); ++channel)
    {
        const auto* a = first.getReadPointer(channel);
        const auto* b = second.getReadPointer(channel);

        for (int sample = 0; sample < first.getNumSamples(); ++sample)
            maximum = std::max(maximum, std::abs(a[sample] - b[sample]));
    }

    return maximum;
}

float maxStereoDifference(const juce::AudioBuffer<float>& buffer)
{
    float maximum = 0.0f;
    const auto* left = buffer.getReadPointer(0);
    const auto* right = buffer.getReadPointer(1);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        maximum = std::max(maximum, std::abs(left[sample] - right[sample]));

    return maximum;
}

void motionZeroAmountLeavesBufferUntouched()
{
    stagemind::MotionProcessor processor;
    processor.prepare(testSampleRate, testBlockSize);

    juce::AudioBuffer<float> buffer(2, testBlockSize);
    fillOppositeSideSignal(buffer);

    juce::AudioBuffer<float> reference;
    reference.makeCopyOf(buffer);

    stagemind::MotionConfig config;
    config.amount = 0.0f;
    config.rateHz = 1.0f;
    processor.process(buffer, config);

    expect(maxDifference(buffer, reference) < 1.0e-6f, "zero motion amount should leave buffer untouched");
}

void motionChangesAllowedSideSignal()
{
    stagemind::MotionProcessor processor;
    processor.prepare(testSampleRate, testBlockSize);

    juce::AudioBuffer<float> buffer(2, testBlockSize);
    fillOppositeSideSignal(buffer);

    juce::AudioBuffer<float> reference;
    reference.makeCopyOf(buffer);

    stagemind::MotionConfig config;
    config.amount = 1.0f;
    config.rateHz = 2.0f;

    for (int i = 0; i < 4; ++i)
        processor.process(buffer, config);

    expect(maxDifference(buffer, reference) > 0.001f, "motion should modulate the side signal when enabled");
}

void motionMovesCenteredSignalWhenEnabled()
{
    stagemind::MotionProcessor processor;
    processor.prepare(testSampleRate, testBlockSize);

    juce::AudioBuffer<float> buffer(2, testBlockSize);
    fillCenteredSignal(buffer);

    stagemind::MotionConfig config;
    config.amount = 1.0f;
    config.rateHz = 2.0f;

    for (int i = 0; i < 4; ++i)
        processor.process(buffer, config);

    expect(maxStereoDifference(buffer) > 0.001f, "motion should create audible left-right movement on centered material");
}

void motionPresetChangesMovementShape()
{
    stagemind::MotionProcessor slowProcessor;
    stagemind::MotionProcessor sweepProcessor;
    slowProcessor.prepare(testSampleRate, testBlockSize);
    sweepProcessor.prepare(testSampleRate, testBlockSize);

    juce::AudioBuffer<float> slowBuffer(2, testBlockSize);
    juce::AudioBuffer<float> sweepBuffer(2, testBlockSize);
    fillCenteredSignal(slowBuffer);
    fillCenteredSignal(sweepBuffer);

    stagemind::MotionConfig slowConfig;
    slowConfig.amount = 1.0f;
    slowConfig.rateHz = 1.0f;
    slowConfig.preset = 0;

    stagemind::MotionConfig sweepConfig = slowConfig;
    sweepConfig.preset = 3;

    for (int i = 0; i < 8; ++i)
    {
        slowProcessor.process(slowBuffer, slowConfig);
        sweepProcessor.process(sweepBuffer, sweepConfig);
    }

    expect(maxDifference(slowBuffer, sweepBuffer) > 0.001f, "motion presets should produce distinct movement");
}

void widthCreatesAudibleSideFromCenteredSignal()
{
    stagemind::MidSideProcessor processor;
    processor.prepare(testSampleRate, testBlockSize);
    processor.setWidthAmount(2.2f);

    juce::AudioBuffer<float> buffer(2, testBlockSize);

    for (int block = 0; block < 4; ++block)
    {
        fillCenteredSine(buffer, 1200.0f, block * testBlockSize);
        processor.process(buffer);
    }

    expect(maxStereoDifference(buffer) > 0.02f, "high width should create audible side from centered material");
}

void depthZeroAmountLeavesBufferUntouched()
{
    stagemind::DepthProcessor processor;
    processor.prepare(testSampleRate, testBlockSize);

    juce::AudioBuffer<float> buffer(2, testBlockSize);
    fillOppositeSideSignal(buffer);

    juce::AudioBuffer<float> reference;
    reference.makeCopyOf(buffer);

    stagemind::DepthConfig config;
    config.amount = 0.0f;
    config.presenceReduction = 0.0f;
    config.earlyReflectionAmount = 1.0f;
    processor.process(buffer, config);

    expect(maxDifference(buffer, reference) < 1.0e-6f, "zero depth amount should leave buffer untouched");
}

void depthPositiveAmountChangesBuffer()
{
    stagemind::DepthProcessor processor;
    processor.prepare(testSampleRate, testBlockSize);

    juce::AudioBuffer<float> buffer(2, testBlockSize);
    fillOppositeSideSignal(buffer);

    juce::AudioBuffer<float> reference;
    reference.makeCopyOf(buffer);

    stagemind::DepthConfig config;
    config.amount = 1.0f;
    config.presenceReduction = 0.35f;
    config.earlyReflectionAmount = 1.0f;

    for (int i = 0; i < 2; ++i)
        processor.process(buffer, config);

    expect(maxDifference(buffer, reference) > 0.001f, "positive depth amount should alter tone or reflections");
}

void depthAddsEarlyRoomReflection()
{
    stagemind::DepthProcessor processor;
    processor.prepare(testSampleRate, testBlockSize);

    juce::AudioBuffer<float> buffer(2, testBlockSize);
    buffer.clear();
    buffer.setSample(0, 0, 1.0f);
    buffer.setSample(1, 0, 1.0f);

    stagemind::DepthConfig config;
    config.amount = 1.0f;
    config.presenceReduction = 0.35f;
    config.earlyReflectionAmount = 1.0f;
    processor.process(buffer, config);

    expect(buffer.getSample(0, 0) > 0.99f, "depth should keep the dry impulse zero-latency");
    expect(buffer.getSample(1, 0) > 0.99f, "depth should keep the dry impulse zero-latency");

    bool foundRoomTail = false;
    for (int sample = 350; sample < buffer.getNumSamples(); ++sample)
    {
        foundRoomTail = foundRoomTail
            || std::abs(buffer.getSample(0, sample)) > 0.0005f
            || std::abs(buffer.getSample(1, sample)) > 0.0005f;
    }

    expect(foundRoomTail, "depth should add early room-like reflections after the dry impulse");
}

void cleanUpZeroAmountLeavesBufferUntouched()
{
    stagemind::CleanUpProcessor processor;
    processor.prepare(testSampleRate, testBlockSize);

    juce::AudioBuffer<float> buffer(2, testBlockSize);
    fillOppositeSideSignal(buffer);

    juce::AudioBuffer<float> reference;
    reference.makeCopyOf(buffer);

    stagemind::CleanUpConfig config;
    config.amount = 0.0f;
    processor.process(buffer, config);

    expect(maxDifference(buffer, reference) < 1.0e-6f, "zero clean-up amount should leave buffer untouched");
}

void cleanUpPositiveAmountChangesTone()
{
    stagemind::CleanUpProcessor processor;
    processor.prepare(testSampleRate, testBlockSize);

    juce::AudioBuffer<float> buffer(2, testBlockSize);
    juce::AudioBuffer<float> reference(2, testBlockSize);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto phaseLowMid = twoPi * 420.0f * static_cast<float> (sample) / static_cast<float> (testSampleRate);
        const auto phaseHarsh = twoPi * 3600.0f * static_cast<float> (sample) / static_cast<float> (testSampleRate);
        const auto value = std::sin(phaseLowMid) * 0.35f + std::sin(phaseHarsh) * 0.25f;
        buffer.setSample(0, sample, value);
        buffer.setSample(1, sample, value);
    }

    reference.makeCopyOf(buffer);

    stagemind::CleanUpConfig config;
    config.amount = 1.0f;
    config.lowMidReduction = 0.20f;
    config.harshReduction = 0.18f;
    config.airLift = 0.0f;

    for (int i = 0; i < 4; ++i)
        processor.process(buffer, config);

    expect(maxDifference(buffer, reference) > 0.001f, "positive clean-up amount should alter tone");
}

void levelRiderCompensatesQuieterSections()
{
    stagemind::LevelRider rider;
    rider.prepare(testSampleRate, testBlockSize);

    stagemind::LevelRiderConfig config;
    config.enabled = true;
    config.activeThresholdRms = 0.001f;
    config.learnSeconds = 4.0f;
    config.maxBoostDb = 4.0f;

    juce::AudioBuffer<float> buffer(2, testBlockSize);
    stagemind::LevelRiderStatus status;

    for (int block = 0; block < 24; ++block)
    {
        fillCenteredSine(buffer, 440.0f, block * testBlockSize, 0.45f);
        const auto levels = rmsAndPeak(buffer);
        status = rider.process(buffer, levels.first, levels.second, config);
    }

    float beforeQuiet = 0.0f;
    float afterQuiet = 0.0f;
    for (int block = 0; block < 48; ++block)
    {
        fillCenteredSine(buffer, 440.0f, block * testBlockSize, 0.10f);
        const auto beforeLevels = rmsAndPeak(buffer);
        beforeQuiet = beforeLevels.first;
        status = rider.process(buffer, beforeLevels.first, beforeLevels.second, config);
        afterQuiet = rmsAndPeak(buffer).first;
    }

    expect(status.active, "level rider should stay active on audible material");
    expect(status.gainDb > 1.0f, "level rider should request boost for a quieter section");
    expect(afterQuiet > beforeQuiet * 1.25f, "level rider should audibly lift a quieter section");
}

void stageGainRideMovesTowardFixedTarget()
{
    stagemind::LevelRider rider;
    rider.prepare(testSampleRate, testBlockSize);

    stagemind::LevelRiderConfig config;
    config.mode = stagemind::LevelRiderMode::Ride;
    config.activeThresholdRms = 0.001f;
    config.fixedTargetRms = 0.16f;
    config.ceilingDb = -1.0f;
    config.response = 1.0f;
    config.maxBoostDb = 18.0f;
    config.maxCutDb = 24.0f;

    juce::AudioBuffer<float> buffer(2, testBlockSize);
    float before = 0.0f;
    float after = 0.0f;
    stagemind::LevelRiderStatus status;

    for (int block = 0; block < 18; ++block)
    {
        fillCenteredSine(buffer, 440.0f, block * testBlockSize, 0.04f);
        const auto levels = rmsAndPeak(buffer);
        before = levels.first;
        status = rider.process(buffer, levels.first, levels.second, config);
        after = rmsAndPeak(buffer).first;
    }

    expect(status.active, "stage gain ride should become active on audible material");
    expect(status.gainDb > 6.0f, "stage gain ride should apply a visible boost toward the fixed target");
    expect(after > before * 2.0f, "stage gain ride should audibly lift quiet material");
}

void stageGainStaticAnalyzeHoldsGain()
{
    stagemind::LevelRider rider;
    rider.prepare(testSampleRate, testBlockSize);

    stagemind::LevelRiderConfig config;
    config.mode = stagemind::LevelRiderMode::Static;
    config.activeThresholdRms = 0.001f;
    config.fixedTargetRms = 0.14f;
    config.ceilingDb = -1.0f;
    config.response = 1.0f;
    config.maxBoostDb = 18.0f;
    config.maxCutDb = 24.0f;
    config.analyzeRequested = true;

    juce::AudioBuffer<float> buffer(2, testBlockSize);
    fillCenteredSine(buffer, 440.0f, 0, 0.04f);
    auto levels = rmsAndPeak(buffer);
    auto status = rider.process(buffer, levels.first, levels.second, config);

    expect(status.held, "static stage gain should hold a correction after analyze");
    expect(rider.hasHeldGain(), "static stage gain should store the held correction");
    expect(rider.getHeldGainDb() > 6.0f, "static stage gain should store a meaningful boost for quiet material");

    config.analyzeRequested = false;
    float before = 0.0f;
    float after = 0.0f;
    for (int block = 0; block < 18; ++block)
    {
        fillCenteredSine(buffer, 440.0f, block * testBlockSize, 0.04f);
        levels = rmsAndPeak(buffer);
        before = levels.first;
        status = rider.process(buffer, levels.first, levels.second, config);
        after = rmsAndPeak(buffer).first;
    }

    expect(status.held, "static stage gain should keep using the held correction");
    expect(after > before * 2.0f, "static stage gain should apply the held correction after analysis");
}

void stageGainLufsMeterProducesUsableLevel()
{
    stagemind::StageGainLoudnessMeter meter;
    meter.prepare(testSampleRate);

    juce::AudioBuffer<float> buffer(2, testBlockSize);
    float measured = 0.0f;
    for (int block = 0; block < 40; ++block)
    {
        fillCenteredSine(buffer, 440.0f, block * testBlockSize, 0.25f);
        measured = meter.process(buffer, stagemind::StageGainMeterMode::LufsMomentary);
    }

    const auto lufsDb = meter.currentDb(stagemind::StageGainMeterMode::LufsMomentary);
    const auto target = stagemind::StageGainLoudnessMeter::targetLevelFromDb(
        stagemind::StageGainMeterMode::LufsMomentary,
        -18.0f);

    expect(measured > 0.01f, "LUFS momentary meter should produce a usable signal level");
    expect(lufsDb > -60.0f && lufsDb < 0.0f, "LUFS momentary meter should report a plausible loudness value");
    expect(target > juce::Decibels::decibelsToGain(-18.0f), "LUFS target conversion should include loudness offset");
}

void stageGainLookAheadLimiterConstrainsPeaks()
{
    stagemind::LevelRider rider;
    rider.prepare(testSampleRate, testBlockSize);

    stagemind::LevelRiderConfig config;
    config.mode = stagemind::LevelRiderMode::Ride;
    config.activeThresholdRms = 0.001f;
    config.fixedTargetRms = 0.8f;
    config.targetDisplayDb = -2.0f;
    config.ceilingDb = -6.0f;
    config.response = 1.0f;
    config.maxBoostDb = 18.0f;
    config.maxCutDb = 24.0f;

    juce::AudioBuffer<float> buffer(2, testBlockSize);
    float outputPeak = 0.0f;

    for (int block = 0; block < 12; ++block)
    {
        fillCenteredSine(buffer, 120.0f, block * testBlockSize, 0.9f);
        const auto levels = rmsAndPeak(buffer);
        rider.process(buffer, levels.first, levels.second, config);
        outputPeak = rmsAndPeak(buffer).second;
    }

    const auto ceilingGain = juce::Decibels::decibelsToGain(config.ceilingDb);
    expect(outputPeak <= ceilingGain * 1.04f, "look-ahead limiter should constrain output peaks near the ceiling");
}

void pseudoDoubleKeepsDryPathUndelayed()
{
    stagemind::PseudoDoubleProcessor processor;
    processor.prepare(testSampleRate, testBlockSize);

    juce::AudioBuffer<float> buffer(2, testBlockSize);
    buffer.clear();
    buffer.setSample(0, 0, 1.0f);
    buffer.setSample(1, 0, 1.0f);

    stagemind::PseudoDoubleConfig config;
    config.amount = 1.0f;
    processor.process(buffer, config);

    expect(buffer.getSample(0, 0) > 0.99f, "pseudo-double must keep dry left path undelayed");
    expect(buffer.getSample(1, 0) > 0.99f, "pseudo-double must keep dry right path undelayed");

    bool foundWetDelay = false;
    for (int sample = 400; sample < buffer.getNumSamples(); ++sample)
        foundWetDelay = foundWetDelay || buffer.getSample(0, sample) > 0.015f || buffer.getSample(1, sample) > 0.015f;

    expect(foundWetDelay, "pseudo-double should add delayed wet signal after the dry impulse");
}

void pseudoDoubleCreatesStereoDifferenceOnMonoSine()
{
    stagemind::PseudoDoubleProcessor processor;
    processor.prepare(testSampleRate, testBlockSize);

    juce::AudioBuffer<float> buffer(2, testBlockSize);
    stagemind::PseudoDoubleConfig config;
    config.amount = 1.0f;

    for (int block = 0; block < 12; ++block)
    {
        fillCenteredSine(buffer, 440.0f, block * testBlockSize);
        processor.process(buffer, config);
    }

    expect(maxStereoDifference(buffer) > 0.004f, "pseudo-double should create stereo difference on mono pitched material");
}

void pseudoDoubleZeroAmountLeavesBufferUntouched()
{
    stagemind::PseudoDoubleProcessor processor;
    processor.prepare(testSampleRate, testBlockSize);

    juce::AudioBuffer<float> buffer(2, testBlockSize);
    fillOppositeSideSignal(buffer);

    juce::AudioBuffer<float> reference;
    reference.makeCopyOf(buffer);

    stagemind::PseudoDoubleConfig config;
    config.amount = 0.0f;
    processor.process(buffer, config);

    expect(maxDifference(buffer, reference) < 1.0e-6f, "zero pseudo-double amount should leave buffer untouched");
}
} // namespace

void runSpatialEnhancementTests()
{
    motionZeroAmountLeavesBufferUntouched();
    motionChangesAllowedSideSignal();
    motionMovesCenteredSignalWhenEnabled();
    motionPresetChangesMovementShape();
    widthCreatesAudibleSideFromCenteredSignal();
    depthZeroAmountLeavesBufferUntouched();
    depthPositiveAmountChangesBuffer();
    depthAddsEarlyRoomReflection();
    cleanUpZeroAmountLeavesBufferUntouched();
    cleanUpPositiveAmountChangesTone();
    levelRiderCompensatesQuieterSections();
    stageGainRideMovesTowardFixedTarget();
    stageGainStaticAnalyzeHoldsGain();
    stageGainLufsMeterProducesUsableLevel();
    stageGainLookAheadLimiterConstrainsPeaks();
    pseudoDoubleKeepsDryPathUndelayed();
    pseudoDoubleCreatesStereoDifferenceOnMonoSine();
    pseudoDoubleZeroAmountLeavesBufferUntouched();
}
