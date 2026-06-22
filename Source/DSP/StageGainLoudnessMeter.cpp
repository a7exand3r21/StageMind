#include "StageGainLoudnessMeter.h"

#include <cmath>

namespace stagemind
{
namespace
{
constexpr auto lufsPowerOffset = -0.691f;

float safeMeanSquareToDb(float meanSquare) noexcept
{
    return lufsPowerOffset + 10.0f * std::log10(juce::jmax(1.0e-12f, meanSquare));
}
} // namespace

void StageGainLoudnessMeter::prepare(double sampleRate) noexcept
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    reset();
}

void StageGainLoudnessMeter::reset() noexcept
{
    vuMeanSquare = 0.0f;
    momentaryMeanSquare = 0.0f;
    shortTermMeanSquare = 0.0f;
    integratedMeanSquare = 0.0f;
    integratedSamples = 0.0;
    lastRawMeanSquare = 0.0f;
    lastWeightedMeanSquare = 0.0f;
    hpPreviousInput.fill(0.0f);
    hpPreviousOutput.fill(0.0f);
}

float StageGainLoudnessMeter::process(const juce::AudioBuffer<float>& buffer, StageGainMeterMode mode) noexcept
{
    const auto numSamples = buffer.getNumSamples();
    if (buffer.getNumChannels() <= 0 || numSamples <= 0)
        return 0.0f;

    lastRawMeanSquare = rawMeanSquare(buffer);
    lastWeightedMeanSquare = isLufsMode(mode) ? kWeightedMeanSquare(buffer) : lastRawMeanSquare;

    vuMeanSquare = smoothMeanSquare(vuMeanSquare, lastRawMeanSquare, 0.30f, numSamples);
    momentaryMeanSquare = smoothMeanSquare(momentaryMeanSquare, lastWeightedMeanSquare, 0.40f, numSamples);
    shortTermMeanSquare = smoothMeanSquare(shortTermMeanSquare, lastWeightedMeanSquare, 3.0f, numSamples);
    updateIntegrated(lastWeightedMeanSquare, numSamples);

    switch (mode)
    {
        case StageGainMeterMode::Vu:             return std::sqrt(juce::jmax(0.0f, vuMeanSquare));
        case StageGainMeterMode::Rms:            return std::sqrt(juce::jmax(0.0f, lastRawMeanSquare));
        case StageGainMeterMode::LufsMomentary:  return std::sqrt(juce::jmax(0.0f, momentaryMeanSquare));
        case StageGainMeterMode::LufsShortTerm:  return std::sqrt(juce::jmax(0.0f, shortTermMeanSquare));
        case StageGainMeterMode::LufsIntegrated: return std::sqrt(juce::jmax(0.0f, integratedMeanSquare));
        case StageGainMeterMode::Dbfs:
        default:                                 return std::sqrt(juce::jmax(0.0f, lastRawMeanSquare));
    }
}

float StageGainLoudnessMeter::currentDb(StageGainMeterMode mode) const noexcept
{
    switch (mode)
    {
        case StageGainMeterMode::Vu:
            return juce::Decibels::gainToDecibels(std::sqrt(juce::jmax(1.0e-12f, vuMeanSquare)));
        case StageGainMeterMode::Rms:
        case StageGainMeterMode::Dbfs:
            return juce::Decibels::gainToDecibels(std::sqrt(juce::jmax(1.0e-12f, lastRawMeanSquare)));
        case StageGainMeterMode::LufsMomentary:
            return safeMeanSquareToDb(momentaryMeanSquare);
        case StageGainMeterMode::LufsShortTerm:
            return safeMeanSquareToDb(shortTermMeanSquare);
        case StageGainMeterMode::LufsIntegrated:
            return safeMeanSquareToDb(integratedMeanSquare);
        default:
            return -120.0f;
    }
}

float StageGainLoudnessMeter::targetLevelFromDb(StageGainMeterMode mode, float targetDb) noexcept
{
    if (isLufsMode(mode))
        return std::pow(10.0f, (targetDb - lufsPowerOffset) / 20.0f);

    return juce::Decibels::decibelsToGain(targetDb);
}

float StageGainLoudnessMeter::levelToDb(StageGainMeterMode mode, float level) noexcept
{
    const auto safeLevel = juce::jmax(1.0e-6f, level);
    if (isLufsMode(mode))
        return lufsPowerOffset + 20.0f * std::log10(safeLevel);

    return juce::Decibels::gainToDecibels(safeLevel);
}

float StageGainLoudnessMeter::rawMeanSquare(const juce::AudioBuffer<float>& buffer) const noexcept
{
    double sumSquares = 0.0;
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    for (int channel = 0; channel < numChannels; ++channel)
    {
        const auto* samples = buffer.getReadPointer(channel);
        for (int sample = 0; sample < numSamples; ++sample)
            sumSquares += static_cast<double> (samples[sample]) * samples[sample];
    }

    return static_cast<float> (sumSquares / static_cast<double> (juce::jmax(1, numChannels * numSamples)));
}

float StageGainLoudnessMeter::kWeightedMeanSquare(const juce::AudioBuffer<float>& buffer) noexcept
{
    double sumSquares = 0.0;
    const auto numChannels = juce::jmin(buffer.getNumChannels(), static_cast<int> (hpPreviousInput.size()));
    const auto numSamples = buffer.getNumSamples();
    const auto hpCoefficient = std::exp(-2.0f * juce::MathConstants<float>::pi * 60.0f / static_cast<float> (currentSampleRate));

    for (int channel = 0; channel < numChannels; ++channel)
    {
        const auto* samples = buffer.getReadPointer(channel);
        auto previousInput = hpPreviousInput[static_cast<size_t> (channel)];
        auto previousOutput = hpPreviousOutput[static_cast<size_t> (channel)];

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const auto input = samples[sample];
            const auto highPassed = hpCoefficient * (previousOutput + input - previousInput);
            previousInput = input;
            previousOutput = highPassed;

            const auto shelfApprox = highPassed * 1.18f;
            sumSquares += static_cast<double> (shelfApprox) * shelfApprox;
        }

        hpPreviousInput[static_cast<size_t> (channel)] = previousInput;
        hpPreviousOutput[static_cast<size_t> (channel)] = previousOutput;
    }

    return static_cast<float> (sumSquares / static_cast<double> (juce::jmax(1, numChannels * numSamples)));
}

float StageGainLoudnessMeter::smoothMeanSquare(float previous, float incoming, float seconds, int numSamples) const noexcept
{
    const auto safeSeconds = juce::jmax(0.01f, seconds);
    const auto coefficient = 1.0f - std::exp(
        -static_cast<float> (numSamples) / (static_cast<float> (currentSampleRate) * safeSeconds));
    return previous + coefficient * (incoming - previous);
}

void StageGainLoudnessMeter::updateIntegrated(float blockMeanSquare, int numSamples) noexcept
{
    const auto blockLufs = safeMeanSquareToDb(blockMeanSquare);
    if (blockLufs < -70.0f)
        return;

    if (integratedSamples > currentSampleRate)
    {
        const auto integratedLufs = safeMeanSquareToDb(integratedMeanSquare);
        if (blockLufs < integratedLufs - 10.0f)
            return;
    }

    const auto incomingWeight = static_cast<double> (juce::jmax(1, numSamples));
    const auto totalWeight = integratedSamples + incomingWeight;
    integratedMeanSquare = static_cast<float> (
        (static_cast<double> (integratedMeanSquare) * integratedSamples
            + static_cast<double> (blockMeanSquare) * incomingWeight)
        / totalWeight);
    integratedSamples = totalWeight;
}
} // namespace stagemind
