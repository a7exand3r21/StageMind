#include "LowMonoProcessor.h"

namespace stagemind
{
void LowMonoProcessor::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    cutoff.reset(currentSampleRate, 0.05);
    cutoff.setCurrentAndTargetValue(160.0f);
    reset();
}

void LowMonoProcessor::reset() noexcept
{
    sideLowState = 0.0f;
    updateCoefficient(cutoff.getTargetValue());
}

void LowMonoProcessor::setCutoffHz(float cutoffHz) noexcept
{
    cutoff.setTargetValue(juce::jlimit(40.0f, 300.0f, cutoffHz));
}

void LowMonoProcessor::updateCoefficient(float cutoffHz) noexcept
{
    lastCutoffHz = juce::jlimit(40.0f, 300.0f, cutoffHz);
    const auto omega = juce::MathConstants<float>::twoPi * lastCutoffHz / static_cast<float> (currentSampleRate);
    coefficient = juce::jlimit(0.0f, 1.0f, 1.0f - std::exp(-omega));
}

void LowMonoProcessor::process(juce::AudioBuffer<float>& buffer) noexcept
{
    if (buffer.getNumChannels() < 2)
        return;

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    const auto numSamples = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto cutoffHz = cutoff.getNextValue();
        if (std::abs(cutoffHz - lastCutoffHz) > 0.5f)
            updateCoefficient(cutoffHz);

        const auto mid = (left[sample] + right[sample]) * 0.5f;
        const auto side = (left[sample] - right[sample]) * 0.5f;

        sideLowState += coefficient * (side - sideLowState);
        const auto sideHighOnly = side - sideLowState;

        left[sample] = mid + sideHighOnly;
        right[sample] = mid - sideHighOnly;
    }
}
} // namespace stagemind
