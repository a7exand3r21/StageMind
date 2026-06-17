#pragma once

#include "../Link/LinkSpectralBands.h"
#include <juce_audio_basics/juce_audio_basics.h>

namespace stagemind
{
class LinkSpectralAnalyzer final
{
public:
    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    LinkSpectralBands processBlock(const juce::AudioBuffer<float>& buffer) noexcept;

private:
    float coefficientFor(float cutoffHz) const noexcept;

    float sampleRateHz = 44100.0f;
    float lowState = 0.0f;
    float lowMidState = 0.0f;
    float presenceState = 0.0f;
};
} // namespace stagemind
