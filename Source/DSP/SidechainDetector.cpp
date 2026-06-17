#include "SidechainDetector.h"

namespace stagemind
{
namespace
{
float coefficientForMs(double sampleRate, float milliseconds) noexcept
{
    const auto samples = std::max(1.0, sampleRate * static_cast<double> (milliseconds) * 0.001);
    return static_cast<float> (std::exp(-1.0 / samples));
}
} // namespace

void SidechainDetector::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    attackCoefficient = coefficientForMs(currentSampleRate, 10.0f);
    releaseCoefficient = coefficientForMs(currentSampleRate, 120.0f);
    reset();
}

void SidechainDetector::reset() noexcept
{
    envelope = 0.0f;
}

SidechainAnalysis SidechainDetector::processBlock(const juce::AudioBuffer<float>& sidechainBuffer) noexcept
{
    SidechainAnalysis analysis;

    const auto numSamples = sidechainBuffer.getNumSamples();
    const auto numChannels = sidechainBuffer.getNumChannels();

    if (numSamples == 0 || numChannels == 0)
        return analysis;

    double sumSquares = 0.0;
    float peak = 0.0f;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float mixed = 0.0f;

        for (int channel = 0; channel < numChannels; ++channel)
            mixed += sidechainBuffer.getReadPointer(channel)[sample];

        mixed /= static_cast<float> (numChannels);

        const auto magnitude = std::abs(mixed);
        peak = std::max(peak, magnitude);
        sumSquares += static_cast<double> (mixed) * mixed;

        const auto coefficient = magnitude > envelope ? attackCoefficient : releaseCoefficient;
        envelope = coefficient * envelope + (1.0f - coefficient) * magnitude;
    }

    analysis.rms = static_cast<float> (std::sqrt(sumSquares / static_cast<double> (numSamples)));
    analysis.peak = peak;
    analysis.envelope = envelope;
    analysis.isActive = envelope > 0.0005f;
    return analysis;
}
} // namespace stagemind
