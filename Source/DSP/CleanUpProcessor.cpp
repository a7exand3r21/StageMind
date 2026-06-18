#include "CleanUpProcessor.h"

#include <algorithm>
#include <cmath>

namespace stagemind
{
void CleanUpProcessor::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    amount.reset(currentSampleRate, 0.04);
    reset();
}

void CleanUpProcessor::reset() noexcept
{
    for (auto& path : paths)
        path.reset();

    amount.setCurrentAndTargetValue(0.0f);
}

void CleanUpProcessor::process(juce::AudioBuffer<float>& buffer, const CleanUpConfig& config) noexcept
{
    amount.setTargetValue(juce::jlimit(0.0f, 1.0f, config.amount));

    const auto numChannels = std::min(buffer.getNumChannels(), maxChannels);
    const auto numSamples = buffer.getNumSamples();

    if (numChannels == 0 || numSamples == 0)
        return;

    const auto lowMidStart = coefficientForCutoff(currentSampleRate, config.lowMidStartHz);
    const auto lowMidEnd = coefficientForCutoff(currentSampleRate, config.lowMidEndHz);
    const auto harshStart = coefficientForCutoff(currentSampleRate, config.harshStartHz);
    const auto harshEnd = coefficientForCutoff(currentSampleRate, config.harshEndHz);
    const auto airStart = coefficientForCutoff(currentSampleRate, config.airStartHz);
    const auto lowMidReduction = juce::jlimit(0.0f, 0.28f, config.lowMidReduction);
    const auto harshReduction = juce::jlimit(0.0f, 0.24f, config.harshReduction);
    const auto airLift = juce::jlimit(0.0f, 0.08f, config.airLift);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto cleanAmount = amount.getNextValue();
        const auto lowMidCut = cleanAmount * lowMidReduction;
        const auto harshCut = cleanAmount * harshReduction;
        const auto lift = cleanAmount * airLift;

        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* samples = buffer.getWritePointer(channel);
            auto& path = paths[static_cast<size_t> (channel)];
            const auto input = samples[sample];
            const auto lowMid = path.lowMid.processBand(input, lowMidStart, lowMidEnd);
            const auto harsh = path.harsh.processBand(input, harshStart, harshEnd);
            const auto air = path.airLowPass.processHighPass(input, airStart);

            samples[sample] = input - lowMid * lowMidCut - harsh * harshCut + air * lift;
        }
    }
}

float CleanUpProcessor::OnePolePath::processLowPass(float input, float coefficient) noexcept
{
    lowState += coefficient * (input - lowState);
    return lowState;
}

float CleanUpProcessor::OnePolePath::processHighPass(float input, float coefficient) noexcept
{
    return input - processLowPass(input, coefficient);
}

void CleanUpProcessor::BandPath::reset() noexcept
{
    startHighPass.reset();
    endLowPass.reset();
}

float CleanUpProcessor::BandPath::processBand(float input, float startCoefficient, float endCoefficient) noexcept
{
    const auto highPassed = startHighPass.processHighPass(input, startCoefficient);
    return endLowPass.processLowPass(highPassed, endCoefficient);
}

void CleanUpProcessor::ChannelPath::reset() noexcept
{
    lowMid.reset();
    harsh.reset();
    airLowPass.reset();
}

float CleanUpProcessor::coefficientForCutoff(double sampleRate, float cutoffHz) noexcept
{
    const auto safeSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    const auto safeCutoff = juce::jlimit(20.0f, static_cast<float> (safeSampleRate * 0.45), cutoffHz);
    const auto omega = juce::MathConstants<float>::twoPi * safeCutoff / static_cast<float> (safeSampleRate);
    return juce::jlimit(0.0f, 1.0f, 1.0f - std::exp(-omega));
}
} // namespace stagemind
