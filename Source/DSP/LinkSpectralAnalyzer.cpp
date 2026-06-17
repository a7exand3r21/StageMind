#include "LinkSpectralAnalyzer.h"

#include <algorithm>
#include <cmath>

namespace stagemind
{
namespace
{
constexpr auto lowCutoffHz = 160.0f;
constexpr auto lowMidCutoffHz = 800.0f;
constexpr auto presenceCutoffHz = 4000.0f;
constexpr auto bandDisplayScale = 4.0f;

float normalizedBandRms(double sumSquares, int numSamples) noexcept
{
    if (numSamples <= 0)
        return 0.0f;

    const auto rms = std::sqrt(sumSquares / static_cast<double> (numSamples));
    return std::clamp(static_cast<float> (rms) * bandDisplayScale, 0.0f, 1.0f);
}
} // namespace

void LinkSpectralAnalyzer::prepare(double sampleRate) noexcept
{
    sampleRateHz = static_cast<float> (sampleRate > 0.0 ? sampleRate : 44100.0);
    reset();
}

void LinkSpectralAnalyzer::reset() noexcept
{
    lowState = 0.0f;
    lowMidState = 0.0f;
    presenceState = 0.0f;
}

LinkSpectralBands LinkSpectralAnalyzer::processBlock(const juce::AudioBuffer<float>& buffer) noexcept
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    if (numChannels <= 0 || numSamples <= 0)
        return {};

    const auto lowCoefficient = coefficientFor(lowCutoffHz);
    const auto lowMidCoefficient = coefficientFor(lowMidCutoffHz);
    const auto presenceCoefficient = coefficientFor(presenceCutoffHz);
    const auto channelScale = 1.0f / static_cast<float> (numChannels);

    double lowSum = 0.0;
    double lowMidSum = 0.0;
    double presenceSum = 0.0;
    double airSum = 0.0;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        auto mono = 0.0f;
        for (int channel = 0; channel < numChannels; ++channel)
            mono += buffer.getReadPointer(channel)[sample];

        mono *= channelScale;

        lowState += lowCoefficient * (mono - lowState);
        lowMidState += lowMidCoefficient * (mono - lowMidState);
        presenceState += presenceCoefficient * (mono - presenceState);

        const auto low = lowState;
        const auto lowMid = lowMidState - lowState;
        const auto presence = presenceState - lowMidState;
        const auto air = mono - presenceState;

        lowSum += static_cast<double> (low) * low;
        lowMidSum += static_cast<double> (lowMid) * lowMid;
        presenceSum += static_cast<double> (presence) * presence;
        airSum += static_cast<double> (air) * air;
    }

    LinkSpectralBands bands;
    bands.low = normalizedBandRms(lowSum, numSamples);
    bands.lowMid = normalizedBandRms(lowMidSum, numSamples);
    bands.presence = normalizedBandRms(presenceSum, numSamples);
    bands.air = normalizedBandRms(airSum, numSamples);
    return bands;
}

float LinkSpectralAnalyzer::coefficientFor(float cutoffHz) const noexcept
{
    const auto safeSampleRate = std::max(1.0f, sampleRateHz);
    const auto exponent = -2.0f * juce::MathConstants<float>::pi * cutoffHz / safeSampleRate;
    return std::clamp(1.0f - std::exp(exponent), 0.0f, 1.0f);
}
} // namespace stagemind
