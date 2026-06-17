#include "CorrelationMeter.h"

namespace stagemind
{
void CorrelationMeter::reset() noexcept
{
    lastValue = 1.0f;
}

float CorrelationMeter::processBlock(const juce::AudioBuffer<float>& buffer) noexcept
{
    if (buffer.getNumChannels() < 2 || buffer.getNumSamples() == 0)
    {
        lastValue = 1.0f;
        return lastValue;
    }

    const auto* left = buffer.getReadPointer(0);
    const auto* right = buffer.getReadPointer(1);
    const auto numSamples = buffer.getNumSamples();

    double cross = 0.0;
    double leftPower = 0.0;
    double rightPower = 0.0;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto l = static_cast<double> (left[sample]);
        const auto r = static_cast<double> (right[sample]);
        cross += l * r;
        leftPower += l * l;
        rightPower += r * r;
    }

    const auto denominator = std::sqrt(leftPower * rightPower);
    lastValue = denominator > 1.0e-12 ? static_cast<float> (cross / denominator) : 1.0f;
    lastValue = juce::jlimit(-1.0f, 1.0f, lastValue);
    return lastValue;
}
} // namespace stagemind
