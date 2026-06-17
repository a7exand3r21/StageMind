#pragma once

#include <JuceHeader.h>

namespace stagemind
{
struct SidechainAnalysis
{
    float rms = 0.0f;
    float peak = 0.0f;
    float envelope = 0.0f;
    bool isActive = false;
    bool transientDetected = false;
};

class SidechainDetector
{
public:
    void prepare(double sampleRate, int maximumBlockSize);
    void reset() noexcept;
    SidechainAnalysis processBlock(const juce::AudioBuffer<float>& sidechainBuffer) noexcept;

private:
    double currentSampleRate = 44100.0;
    float envelope = 0.0f;
    float attackCoefficient = 0.0f;
    float releaseCoefficient = 0.0f;
};
} // namespace stagemind
