#include "MotionProcessor.h"

#include <cmath>

namespace stagemind
{
namespace
{
constexpr float sideMotionDepth = 0.16f;
constexpr float panMotionDepth = 0.24f;
}

void MotionProcessor::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    amount.reset(currentSampleRate, 0.08);
    rateHz.reset(currentSampleRate, 0.20);
    reset();
}

void MotionProcessor::reset() noexcept
{
    phase = 0.0f;
    amount.setCurrentAndTargetValue(0.0f);
    rateHz.setCurrentAndTargetValue(rateHz.getTargetValue());
}

void MotionProcessor::process(juce::AudioBuffer<float>& buffer, const MotionConfig& config) noexcept
{
    amount.setTargetValue(juce::jlimit(0.0f, 1.0f, config.amount));
    rateHz.setTargetValue(juce::jlimit(0.01f, 8.0f, config.rateHz));

    if (buffer.getNumChannels() < 2 || buffer.getNumSamples() == 0)
        return;

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    const auto numSamples = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto motion = amount.getNextValue();
        const auto rate = rateHz.getNextValue();
        const auto lfo = std::sin(phase);
        const auto sideScale = 1.0f + lfo * motion * sideMotionDepth;

        const auto mid = (left[sample] + right[sample]) * 0.5f;
        const auto side = (left[sample] - right[sample]) * 0.5f * sideScale;

        auto processedLeft = mid + side;
        auto processedRight = mid - side;
        const auto pan = lfo * motion * panMotionDepth;
        const auto panAngle = (pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
        const auto panCompensation = juce::MathConstants<float>::sqrt2;
        processedLeft *= std::cos(panAngle) * panCompensation;
        processedRight *= std::sin(panAngle) * panCompensation;

        left[sample] = processedLeft;
        right[sample] = processedRight;

        phase += juce::MathConstants<float>::twoPi * rate / static_cast<float> (currentSampleRate);
        if (phase >= juce::MathConstants<float>::twoPi)
            phase -= juce::MathConstants<float>::twoPi;
    }
}
} // namespace stagemind
