#include "../Source/DSP/DepthProcessor.h"
#include "../Source/DSP/MotionProcessor.h"
#include "../Source/DSP/PseudoDoubleProcessor.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
constexpr double testSampleRate = 48000.0;
constexpr int testBlockSize = 2048;

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
    depthZeroAmountLeavesBufferUntouched();
    depthPositiveAmountChangesBuffer();
    depthAddsEarlyRoomReflection();
    pseudoDoubleKeepsDryPathUndelayed();
    pseudoDoubleZeroAmountLeavesBufferUntouched();
}
