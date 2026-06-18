#include "MotionProcessor.h"
#include "../Model/MotionPreset.h"

#include <cmath>

namespace stagemind
{
namespace
{
constexpr float sideMotionDepth = 0.16f;
constexpr float panMotionDepth = 0.24f;

float shapedLfo(MotionPreset preset, float phase) noexcept
{
    const auto sine = std::sin(phase);

    switch (preset)
    {
        case MotionPreset::Pulse:
            return std::tanh(sine * 2.2f);

        case MotionPreset::WideSweep:
            return std::sin(phase * 0.85f);

        case MotionPreset::Orbit:
        case MotionPreset::SlowDrift:
        default:
            return sine;
    }
}

float sideLfo(MotionPreset preset, float phase) noexcept
{
    switch (preset)
    {
        case MotionPreset::Orbit:
            return std::sin(phase + juce::MathConstants<float>::halfPi);

        case MotionPreset::Pulse:
            return std::sin(phase * 2.0f) * 0.65f;

        case MotionPreset::WideSweep:
            return std::sin(phase * 0.85f + juce::MathConstants<float>::halfPi);

        case MotionPreset::SlowDrift:
        default:
            return std::sin(phase);
    }
}

float panDepthForPreset(MotionPreset preset) noexcept
{
    switch (preset)
    {
        case MotionPreset::Orbit:     return panMotionDepth * 0.85f;
        case MotionPreset::Pulse:     return panMotionDepth * 0.58f;
        case MotionPreset::WideSweep: return panMotionDepth * 1.35f;
        case MotionPreset::SlowDrift:
        default:                      return panMotionDepth * 0.55f;
    }
}

float sideDepthForPreset(MotionPreset preset) noexcept
{
    switch (preset)
    {
        case MotionPreset::Orbit:     return sideMotionDepth * 1.10f;
        case MotionPreset::Pulse:     return sideMotionDepth * 0.75f;
        case MotionPreset::WideSweep: return sideMotionDepth * 1.45f;
        case MotionPreset::SlowDrift:
        default:                      return sideMotionDepth * 0.55f;
    }
}
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
    const auto preset = motionPresetFromIndex(config.preset);
    rateHz.setTargetValue(motionPresetEffectiveRateHz(preset, config.rateHz));

    if (buffer.getNumChannels() < 2 || buffer.getNumSamples() == 0)
        return;

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    const auto numSamples = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto motion = amount.getNextValue();
        const auto rate = rateHz.getNextValue();
        const auto lfo = shapedLfo(preset, phase);
        const auto sideScale = 1.0f + sideLfo(preset, phase) * motion * sideDepthForPreset(preset);

        const auto mid = (left[sample] + right[sample]) * 0.5f;
        const auto side = (left[sample] - right[sample]) * 0.5f * sideScale;

        auto processedLeft = mid + side;
        auto processedRight = mid - side;
        const auto pan = lfo * motion * panDepthForPreset(preset);
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
