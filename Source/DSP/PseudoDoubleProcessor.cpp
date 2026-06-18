#include "PseudoDoubleProcessor.h"

#include <algorithm>

namespace stagemind
{
void PseudoDoubleProcessor::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    delayBufferSize = std::max(1, static_cast<int> (currentSampleRate * 0.06));
    delayBuffer.setSize(maxChannels, delayBufferSize, false, true, false);
    amount.reset(currentSampleRate, 0.04);
    reset();
}

void PseudoDoubleProcessor::reset() noexcept
{
    delayBuffer.clear();
    writePosition = 0;
    amount.setCurrentAndTargetValue(0.0f);
}

void PseudoDoubleProcessor::process(juce::AudioBuffer<float>& buffer, const PseudoDoubleConfig& config) noexcept
{
    amount.setTargetValue(juce::jlimit(0.0f, 1.0f, config.amount));

    if (buffer.getNumChannels() < 2 || buffer.getNumSamples() == 0)
        return;

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    const auto numSamples = buffer.getNumSamples();
    const auto leftShortDelaySamples = std::max(1, static_cast<int> (static_cast<float> (currentSampleRate) * 0.0115f));
    const auto rightShortDelaySamples = std::max(1, static_cast<int> (static_cast<float> (currentSampleRate) * 0.0168f));
    const auto leftLongDelaySamples = std::max(1, static_cast<int> (static_cast<float> (currentSampleRate) * 0.0230f));
    const auto rightLongDelaySamples = std::max(1, static_cast<int> (static_cast<float> (currentSampleRate) * 0.0310f));

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto dryLeft = left[sample];
        const auto dryRight = right[sample];
        const auto doubleAmount = amount.getNextValue();
        const auto wetGain = doubleAmount * (0.24f + doubleAmount * 0.12f);
        const auto delayedLeftShort = readDelay(0, leftShortDelaySamples);
        const auto delayedRightShort = readDelay(1, rightShortDelaySamples);
        const auto delayedLeftLong = readDelay(0, leftLongDelaySamples);
        const auto delayedRightLong = readDelay(1, rightLongDelaySamples);
        const auto drySide = (dryLeft - dryRight) * 0.5f;
        const auto sideLift = drySide * doubleAmount * 0.10f;
        const auto wetLeft = delayedRightShort * 0.72f - delayedLeftLong * 0.32f + delayedRightLong * 0.16f;
        const auto wetRight = delayedLeftShort * 0.72f - delayedRightLong * 0.32f + delayedLeftLong * 0.16f;

        left[sample] = dryLeft + wetLeft * wetGain + sideLift;
        right[sample] = dryRight + wetRight * wetGain - sideLift;

        writeDelay(0, dryLeft);
        writeDelay(1, dryRight);

        ++writePosition;
        if (writePosition >= delayBufferSize)
            writePosition = 0;
    }
}

float PseudoDoubleProcessor::readDelay(int channel, int delaySamples) const noexcept
{
    auto readPosition = writePosition - delaySamples;
    while (readPosition < 0)
        readPosition += delayBufferSize;

    return delayBuffer.getReadPointer(channel)[readPosition];
}

void PseudoDoubleProcessor::writeDelay(int channel, float value) noexcept
{
    delayBuffer.getWritePointer(channel)[writePosition] = value;
}
} // namespace stagemind
