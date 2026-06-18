#include "DynamicEQ.h"

namespace stagemind
{
void DynamicEQ::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;

    for (auto& band : bands)
        band.prepare(currentSampleRate);
}

void DynamicEQ::reset() noexcept
{
    for (auto& band : bands)
        band.reset();
}

float DynamicEQ::processResonances(
    juce::AudioBuffer<float>& buffer,
    const ResonanceSnapshot& snapshot,
    const ResonanceSuppressionConfig& config) noexcept
{
    const auto peakCount = juce::jmin<int>(snapshot.peakCount, static_cast<int> (maxResonancePeaks));

    for (int i = 0; i < static_cast<int> (maxResonancePeaks); ++i)
    {
        if (i < peakCount)
            bands[static_cast<size_t> (i)].setTarget(snapshot.peaks[static_cast<size_t> (i)], config.resonanceAmount, config.maxReductionDb);
        else
            bands[static_cast<size_t> (i)].release();
    }

    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = juce::jmin(buffer.getNumChannels(), 2);

    if (numChannels == 0 || numSamples == 0)
        return 0.0f;

    const auto attackCoefficient = coefficientForMs(currentSampleRate, config.attackMs);
    const auto releaseCoefficient = coefficientForMs(currentSampleRate, config.releaseMs);
    float maxReductionDb = 0.0f;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        for (auto& band : bands)
        {
            const auto coefficient = band.targetReductionDb > band.currentReductionDb ? attackCoefficient : releaseCoefficient;
            band.currentReductionDb = coefficient * band.currentReductionDb + (1.0f - coefficient) * band.targetReductionDb;

            if (band.currentReductionDb < 0.0001f)
                band.currentReductionDb = 0.0f;

            maxReductionDb = std::max(maxReductionDb, band.currentReductionDb);

            if (band.currentReductionDb <= 0.0f)
                continue;

            const auto frequency = band.frequencyHz.getNextValue();
            const auto qValue = band.q.getNextValue();
            const auto width = juce::jlimit(20.0f, 4000.0f, frequency / juce::jmax(1.0f, qValue));
            const auto startHz = juce::jlimit(20.0f, 20000.0f, frequency - width * 0.5f);
            const auto endHz = juce::jlimit(startHz + 20.0f, 20000.0f, frequency + width * 0.5f);
            const auto startCoefficient = coefficientForCutoff(currentSampleRate, startHz);
            const auto endCoefficient = coefficientForCutoff(currentSampleRate, endHz);
            const auto cutLinear = 1.0f - juce::Decibels::decibelsToGain(-band.currentReductionDb);

            for (int channel = 0; channel < numChannels; ++channel)
            {
                auto* samples = buffer.getWritePointer(channel);
                const auto bandSignal = band.paths[static_cast<size_t> (channel)].processBand(samples[sample], startCoefficient, endCoefficient);
                samples[sample] -= bandSignal * cutLinear;
            }
        }
    }

    return maxReductionDb;
}

float DynamicEQ::OnePolePath::processLowPass(float input, float coefficient) noexcept
{
    lowState += coefficient * (input - lowState);
    return lowState;
}

float DynamicEQ::OnePolePath::processHighPass(float input, float coefficient) noexcept
{
    return input - processLowPass(input, coefficient);
}

void DynamicEQ::BandPath::reset() noexcept
{
    startHighPass.reset();
    endLowPass.reset();
}

float DynamicEQ::BandPath::processBand(float input, float startCoefficient, float endCoefficient) noexcept
{
    const auto highPassed = startHighPass.processHighPass(input, startCoefficient);
    return endLowPass.processLowPass(highPassed, endCoefficient);
}

void DynamicEQ::Band::prepare(double sampleRate) noexcept
{
    frequencyHz.reset(sampleRate, 0.08);
    q.reset(sampleRate, 0.08);
    reset();
}

void DynamicEQ::Band::reset() noexcept
{
    for (auto& path : paths)
        path.reset();

    frequencyHz.setCurrentAndTargetValue(1000.0f);
    q.setCurrentAndTargetValue(4.0f);
    currentReductionDb = 0.0f;
    targetReductionDb = 0.0f;
}

void DynamicEQ::Band::setTarget(const ResonancePeak& peak, float resonanceAmount, float maxReductionDb) noexcept
{
    if (peak.frequencyHz <= 0.0f || resonanceAmount <= 0.0f)
    {
        release();
        return;
    }

    const auto intensity = juce::jlimit(0.0f, 1.35f, resonanceAmount);
    const auto reductionCeilingDb = juce::jlimit(0.0f, 8.0f, maxReductionDb);
    const auto reductionShape = 0.75f + intensity * 0.45f;

    frequencyHz.setTargetValue(juce::jlimit(80.0f, 16000.0f, peak.frequencyHz));
    q.setTargetValue(juce::jlimit(1.0f, 14.0f, peak.suggestedQ * (0.92f + intensity * 0.14f)));
    targetReductionDb = juce::jlimit(
        0.0f,
        8.0f,
        std::min(peak.suggestedReductionDb * reductionShape, reductionCeilingDb) * intensity);
}

void DynamicEQ::Band::release() noexcept
{
    targetReductionDb = 0.0f;
}

float DynamicEQ::coefficientForCutoff(double sampleRate, float cutoffHz) noexcept
{
    const auto safeSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    const auto safeCutoff = juce::jlimit(20.0f, static_cast<float> (safeSampleRate * 0.45), cutoffHz);
    const auto omega = juce::MathConstants<float>::twoPi * safeCutoff / static_cast<float> (safeSampleRate);
    return juce::jlimit(0.0f, 1.0f, 1.0f - std::exp(-omega));
}

float DynamicEQ::coefficientForMs(double sampleRate, float milliseconds) noexcept
{
    const auto safeMs = juce::jlimit(1.0f, 1000.0f, milliseconds);
    const auto samples = std::max(1.0, sampleRate * static_cast<double> (safeMs) * 0.001);
    return static_cast<float> (std::exp(-1.0 / samples));
}
} // namespace stagemind
