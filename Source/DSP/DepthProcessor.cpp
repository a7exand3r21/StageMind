#include "DepthProcessor.h"

#include <algorithm>
#include <cmath>

namespace stagemind
{
void DepthProcessor::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    delayBufferSize = std::max(1, static_cast<int> (currentSampleRate * 0.04));
    reflectionDelaySamples = std::max(1, static_cast<int> (currentSampleRate * 0.012));
    delayBuffer.setSize(maxChannels, delayBufferSize, false, true, false);
    amount.reset(currentSampleRate, 0.08);
    presenceReduction.reset(currentSampleRate, 0.08);
    earlyReflectionAmount.reset(currentSampleRate, 0.08);
    dampingCoefficient = coefficientForCutoff(currentSampleRate, 6500.0f);
    reset();
}

void DepthProcessor::reset() noexcept
{
    delayBuffer.clear();
    presenceLowState.fill(0.0f);
    wetLowState.fill(0.0f);
    writePosition = 0;
    amount.setCurrentAndTargetValue(0.0f);
    presenceReduction.setCurrentAndTargetValue(0.0f);
    earlyReflectionAmount.setCurrentAndTargetValue(0.0f);
}

void DepthProcessor::process(juce::AudioBuffer<float>& buffer, const DepthConfig& config) noexcept
{
    amount.setTargetValue(juce::jlimit(0.0f, 1.0f, config.amount));
    presenceReduction.setTargetValue(juce::jlimit(0.0f, 1.0f, config.presenceReduction));
    earlyReflectionAmount.setTargetValue(juce::jlimit(0.0f, 1.0f, config.earlyReflectionAmount));

    const auto numChannels = std::min(buffer.getNumChannels(), maxChannels);
    const auto numSamples = buffer.getNumSamples();

    if (numChannels == 0 || numSamples == 0)
        return;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto depth = amount.getNextValue();
        const auto presenceCut = presenceReduction.getNextValue();
        const auto wetAmount = earlyReflectionAmount.getNextValue();
        const auto dryScale = 1.0f - depth * 0.05f;
        const auto wetGain = depth * wetAmount * 0.12f;

        std::array<float, maxChannels> dry {};

        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* samples = buffer.getWritePointer(channel);
            const auto input = samples[sample];
            dry[static_cast<size_t> (channel)] = input;

            presenceLowState[static_cast<size_t> (channel)] += dampingCoefficient * (input - presenceLowState[static_cast<size_t> (channel)]);
            const auto high = input - presenceLowState[static_cast<size_t> (channel)];
            samples[sample] = (input - high * presenceCut) * dryScale;
        }

        for (int channel = 0; channel < numChannels; ++channel)
        {
            const auto sourceChannel = numChannels > 1 ? 1 - channel : channel;
            auto wet = readDelay(sourceChannel, reflectionDelaySamples);
            wetLowState[static_cast<size_t> (channel)] += dampingCoefficient * (wet - wetLowState[static_cast<size_t> (channel)]);
            wet = wetLowState[static_cast<size_t> (channel)];
            buffer.getWritePointer(channel)[sample] += wet * wetGain;
        }

        for (int channel = 0; channel < numChannels; ++channel)
            writeDelay(channel, dry[static_cast<size_t> (channel)]);

        ++writePosition;
        if (writePosition >= delayBufferSize)
            writePosition = 0;
    }
}

float DepthProcessor::readDelay(int channel, int delaySamples) const noexcept
{
    auto readPosition = writePosition - delaySamples;
    while (readPosition < 0)
        readPosition += delayBufferSize;

    return delayBuffer.getReadPointer(channel)[readPosition];
}

void DepthProcessor::writeDelay(int channel, float value) noexcept
{
    delayBuffer.getWritePointer(channel)[writePosition] = value;
}

float DepthProcessor::coefficientForCutoff(double sampleRate, float cutoffHz) noexcept
{
    const auto safeSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    const auto safeCutoff = juce::jlimit(20.0f, static_cast<float> (safeSampleRate * 0.45), cutoffHz);
    const auto omega = juce::MathConstants<float>::twoPi * safeCutoff / static_cast<float> (safeSampleRate);
    return juce::jlimit(0.0f, 1.0f, 1.0f - std::exp(-omega));
}
} // namespace stagemind
