#include "PseudoDoubleProcessor.h"

#include <algorithm>

namespace stagemind
{
void PseudoDoubleProcessor::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    delayBufferSize = std::max(1, static_cast<int> (currentSampleRate * 0.04));
    delayBuffer.setSize(maxChannels, delayBufferSize, false, true, false);
    amount.reset(currentSampleRate, 0.08);
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

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto dryLeft = left[sample];
        const auto dryRight = right[sample];
        const auto doubleAmount = amount.getNextValue();
        const auto delaySamples = std::max(1, static_cast<int> (static_cast<float> (currentSampleRate) * 0.018f));
        const auto wetGain = doubleAmount * 0.18f;
        const auto delayedLeft = readDelay(0, delaySamples);
        const auto delayedRight = readDelay(1, delaySamples);

        left[sample] = dryLeft + delayedRight * wetGain;
        right[sample] = dryRight + delayedLeft * wetGain;

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
