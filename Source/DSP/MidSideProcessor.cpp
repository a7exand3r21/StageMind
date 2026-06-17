#include "MidSideProcessor.h"

namespace stagemind
{
void MidSideProcessor::prepare(double sampleRate, int)
{
    widthAmount.reset(sampleRate, 0.05);
    widthAmount.setCurrentAndTargetValue(1.0f);
}

void MidSideProcessor::reset() noexcept
{
    widthAmount.setCurrentAndTargetValue(widthAmount.getTargetValue());
}

void MidSideProcessor::setWidthAmount(float targetWidthAmount) noexcept
{
    widthAmount.setTargetValue(juce::jlimit(0.0f, 1.8f, targetWidthAmount));
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
        const auto side = (left[sample] - right[sample]) * 0.5f * width;

        left[sample] = mid + side;
        right[sample] = mid - side;
    }
}
} // namespace stagemind
