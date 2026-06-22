#include "MidSideProcessor.h"

#include <cmath>

namespace stagemind
{
void MidSideProcessor::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    monoSpreadCoefficient = coefficientForCutoff(currentSampleRate, 1600.0f);
    widthAmount.reset(sampleRate, 0.05);
    widthAmount.setCurrentAndTargetValue(1.0f);
    monoSpreadLowState = 0.0f;
}

void MidSideProcessor::reset() noexcept
{
    widthAmount.setCurrentAndTargetValue(widthAmount.getTargetValue());
    monoSpreadLowState = 0.0f;
}

void MidSideProcessor::setWidthAmount(float targetWidthAmount) noexcept
{
    widthAmount.setTargetValue(juce::jlimit(0.0f, 2.35f, targetWidthAmount));
}

void MidSideProcessor::process(juce::AudioBuffer<float>& buffer) noexcept
{
    if (buffer.getNumChannels() < 2)
        return;

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    const auto numSamples = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto width = widthAmount.getNextValue();
        const auto mid = (left[sample] + right[sample]) * 0.5f;
        const auto naturalSide = (left[sample] - right[sample]) * 0.5f;
        monoSpreadLowState += monoSpreadCoefficient * (mid - monoSpreadLowState);
        const auto midAir = mid - monoSpreadLowState;
        const auto monoSpread = juce::jlimit(0.0f, 0.34f, (width - 1.0f) * 0.26f);
        const auto side = naturalSide * width + midAir * monoSpread;

        left[sample] = mid + side;
        right[sample] = mid - side;
    }
}

float MidSideProcessor::coefficientForCutoff(double sampleRate, float cutoffHz) noexcept
{
    const auto safeSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    const auto safeCutoff = juce::jlimit(20.0f, static_cast<float> (safeSampleRate * 0.45), cutoffHz);
    const auto omega = juce::MathConstants<float>::twoPi * safeCutoff / static_cast<float> (safeSampleRate);
    return juce::jlimit(0.0f, 1.0f, 1.0f - std::exp(-omega));
}
} // namespace stagemind
