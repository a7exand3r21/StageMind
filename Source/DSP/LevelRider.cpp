#include "LevelRider.h"

#include <cmath>

namespace stagemind
{
namespace
{
float safeGainToDb(float gain) noexcept
{
    return juce::Decibels::gainToDecibels(juce::jmax(1.0e-5f, gain));
}

float bufferPeak(const juce::AudioBuffer<float>& buffer) noexcept
{
    auto peak = 0.0f;
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        const auto* samples = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            peak = juce::jmax(peak, std::abs(samples[sample]));
    }

    return peak;
}
} // namespace

int LevelRider::getLookAheadSamplesForSampleRate(double sampleRate) noexcept
{
    const auto safeSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    return juce::jlimit(32, 512, static_cast<int> (std::ceil(safeSampleRate * 0.003)));
}

void LevelRider::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    lookAheadSamples = getLookAheadSamplesForSampleRate(currentSampleRate);
    lookAheadBuffer.setSize(2, lookAheadSamples + 1, false, true, true);
    lookAheadBuffer.clear();
    lookAheadWriteIndex = 0;
    currentResponseSeconds = 0.35f;
    gain.reset(currentSampleRate, currentResponseSeconds);
    reset();
}

void LevelRider::reset() noexcept
{
    staticAnalyzeArmed = false;
    limiterGain = 1.0f;
    limiterHoldSamples = 0;
    lookAheadWriteIndex = 0;
    lookAheadBuffer.clear();
    gain.setCurrentAndTargetValue(1.0f);
}

void LevelRider::setTargetRms(float newTargetRms) noexcept
{
    targetRms = juce::jlimit(0.0f, 0.6f, newTargetRms);
}

void LevelRider::setHeldGainDb(float newHeldGainDb) noexcept
{
    heldGainDb = juce::jlimit(-36.0f, 24.0f, newHeldGainDb);
    staticGainReady = true;
    staticAnalyzeArmed = false;
}

void LevelRider::clearHeldGain() noexcept
{
    heldGainDb = 0.0f;
    staticGainReady = false;
    staticAnalyzeArmed = false;
}

LevelRiderStatus LevelRider::process(
    juce::AudioBuffer<float>& buffer,
    float currentRms,
    float currentPeak,
    const LevelRiderConfig& config) noexcept
{
    LevelRiderStatus status;
    status.targetRms = targetRms;

    const auto safeRms = juce::jmax(0.0f, currentRms);
    const auto safePeak = juce::jmax(0.0f, currentPeak);
    auto mode = config.mode;
    if (mode == LevelRiderMode::Off && config.enabled)
        mode = LevelRiderMode::LegacyAuto;

    status.modeIndex = static_cast<int> (mode);

    if (mode == LevelRiderMode::Off)
    {
        gain.setTargetValue(1.0f);
        applyGain(buffer);
        applyLookAheadLimiter(buffer, config.ceilingDb, false);
        status.gainDb = safeGainToDb(gain.getCurrentValue());
        status.outputPeakDb = safeGainToDb(bufferPeak(buffer));
        return status;
    }

    if (mode == LevelRiderMode::LegacyAuto)
        setResponseSeconds(0.35f);
    else
        setResponseSeconds(responseToSeconds(config.response));

    if (config.analyzeRequested)
        staticAnalyzeArmed = mode == LevelRiderMode::Static;

    const auto active = safeRms >= config.activeThresholdRms;

    if (mode == LevelRiderMode::Static && staticGainReady && ! staticAnalyzeArmed)
    {
        auto desiredDb = heldGainDb;
        if (safePeak > 0.0f)
        {
            const auto ceilingGain = juce::Decibels::decibelsToGain(config.ceilingDb);
            const auto peakLimitedBoostDb = safeGainToDb(ceilingGain / safePeak);
            desiredDb = juce::jmin(desiredDb, peakLimitedBoostDb);
        }

        gain.setTargetValue(juce::Decibels::decibelsToGain(desiredDb));
        applyGain(buffer);
        applyLookAheadLimiter(buffer, config.ceilingDb, true);

        status.active = std::abs(desiredDb) >= 0.05f;
        status.held = true;
        status.analyzing = false;
        status.targetRms = config.fixedTargetRms;
        status.targetDb = config.targetDisplayDb;
        status.gainDb = safeGainToDb(gain.getCurrentValue());
        status.outputPeakDb = safeGainToDb(bufferPeak(buffer));
        return status;
    }

    if (! active)
    {
        gain.setTargetValue(1.0f);
        applyGain(buffer);
        applyLookAheadLimiter(buffer, config.ceilingDb, mode != LevelRiderMode::LegacyAuto);
        status.analyzing = staticAnalyzeArmed;
        status.targetRms = mode == LevelRiderMode::LegacyAuto ? targetRms : config.fixedTargetRms;
        status.targetDb = mode == LevelRiderMode::LegacyAuto ? safeGainToDb(status.targetRms) : config.targetDisplayDb;
        status.gainDb = safeGainToDb(gain.getCurrentValue());
        status.outputPeakDb = safeGainToDb(bufferPeak(buffer));
        return status;
    }

    if (mode == LevelRiderMode::Static || mode == LevelRiderMode::Ride)
    {
        auto desiredDb = calculateTargetGainDb(safeRms, safePeak, config.fixedTargetRms, config);
        if (mode == LevelRiderMode::Static)
        {
            if (staticAnalyzeArmed)
            {
                heldGainDb = desiredDb;
                staticGainReady = true;
                staticAnalyzeArmed = false;
            }
            else
            {
                gain.setTargetValue(1.0f);
                applyGain(buffer);
                applyLookAheadLimiter(buffer, config.ceilingDb, true);
                status.analyzing = false;
                status.targetRms = config.fixedTargetRms;
                status.targetDb = config.targetDisplayDb;
                status.gainDb = safeGainToDb(gain.getCurrentValue());
                status.outputPeakDb = safeGainToDb(bufferPeak(buffer));
                return status;
            }
        }

        if (mode == LevelRiderMode::Static)
            desiredDb = heldGainDb;

        gain.setTargetValue(juce::Decibels::decibelsToGain(desiredDb));
        applyGain(buffer);
        applyLookAheadLimiter(buffer, config.ceilingDb, true);

        status.active = true;
        status.held = mode == LevelRiderMode::Static && staticGainReady;
        status.analyzing = staticAnalyzeArmed;
        status.targetRms = config.fixedTargetRms;
        status.targetDb = config.targetDisplayDb;
        status.gainDb = safeGainToDb(gain.getCurrentValue());
        status.outputPeakDb = safeGainToDb(bufferPeak(buffer));
        return status;
    }

    const auto clampedRms = juce::jlimit(config.targetFloorRms, config.targetCeilingRms, safeRms);
    if (targetRms <= 0.0f)
        targetRms = clampedRms;
    else
    {
        const auto samples = juce::jmax(1, buffer.getNumSamples());
        const auto learnSeconds = juce::jmax(1.0f, config.learnSeconds);
        const auto learnCoefficient = 1.0f - std::exp(-static_cast<float> (samples) / (static_cast<float> (currentSampleRate) * learnSeconds));
        targetRms += learnCoefficient * (clampedRms - targetRms);
    }

    auto desiredDb = calculateTargetGainDb(safeRms, safePeak, targetRms, config);

    gain.setTargetValue(juce::Decibels::decibelsToGain(desiredDb));
    applyGain(buffer);
    applyLookAheadLimiter(buffer, config.ceilingDb, true);

    status.active = true;
    status.targetRms = targetRms;
    status.targetDb = safeGainToDb(targetRms);
    status.gainDb = safeGainToDb(gain.getCurrentValue());
    status.outputPeakDb = safeGainToDb(bufferPeak(buffer));
    return status;
}

float LevelRider::responseToSeconds(float response) noexcept
{
    const auto normalized = juce::jlimit(0.0f, 1.0f, response);
    return juce::jmap(normalized, 2.5f, 0.05f);
}

void LevelRider::setResponseSeconds(float seconds) noexcept
{
    const auto clampedSeconds = juce::jlimit(0.02f, 3.0f, seconds);
    if (std::abs(clampedSeconds - currentResponseSeconds) < 0.01f)
        return;

    currentResponseSeconds = clampedSeconds;
    limiterReleaseCoefficient = 1.0f - std::exp(-1.0f / (static_cast<float> (currentSampleRate) * juce::jmax(0.04f, clampedSeconds * 0.20f)));
    gain.reset(currentSampleRate, currentResponseSeconds);
}

float LevelRider::calculateTargetGainDb(
    float sourceRms,
    float sourcePeak,
    float desiredTargetRms,
    const LevelRiderConfig& config) const noexcept
{
    auto desiredDb = safeGainToDb(juce::jmax(1.0e-5f, desiredTargetRms) / juce::jmax(1.0e-5f, sourceRms));
    desiredDb = juce::jlimit(-juce::jmax(0.0f, config.maxCutDb), juce::jmax(0.0f, config.maxBoostDb), desiredDb);

    if (sourcePeak > 0.0f)
    {
        const auto ceilingGain = juce::Decibels::decibelsToGain(config.ceilingDb);
        const auto peakLimitedBoostDb = safeGainToDb(ceilingGain / sourcePeak);
        desiredDb = juce::jmin(desiredDb, peakLimitedBoostDb);
    }

    return desiredDb;
}

void LevelRider::applyLookAheadLimiter(juce::AudioBuffer<float>& buffer, float ceilingDb, bool limiterEnabled) noexcept
{
    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = buffer.getNumChannels();
    if (numSamples <= 0 || numChannels <= 0 || lookAheadSamples <= 0 || lookAheadBuffer.getNumSamples() <= 0)
        return;

    const auto channelsToDelay = juce::jmin(numChannels, lookAheadBuffer.getNumChannels());
    const auto delayBufferSize = lookAheadBuffer.getNumSamples();
    const auto ceilingGain = juce::Decibels::decibelsToGain(ceilingDb);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        auto incomingPeak = 0.0f;
        for (int channel = 0; channel < numChannels; ++channel)
            incomingPeak = juce::jmax(incomingPeak, std::abs(buffer.getSample(channel, sample)));

        if (limiterEnabled && incomingPeak > ceilingGain && incomingPeak > 1.0e-6f)
        {
            limiterGain = juce::jmin(limiterGain, ceilingGain / incomingPeak);
            limiterHoldSamples = lookAheadSamples;
        }
        else if (limiterHoldSamples > 0)
        {
            --limiterHoldSamples;
        }
        else
        {
            limiterGain += (1.0f - limiterGain) * limiterReleaseCoefficient;
            if (limiterGain > 0.9999f)
                limiterGain = 1.0f;
        }

        for (int channel = 0; channel < channelsToDelay; ++channel)
        {
            const auto incoming = buffer.getSample(channel, sample);
            const auto delayed = lookAheadBuffer.getSample(channel, lookAheadWriteIndex);
            lookAheadBuffer.setSample(channel, lookAheadWriteIndex, incoming);
            buffer.setSample(channel, sample, delayed * limiterGain);
        }

        for (int channel = channelsToDelay; channel < numChannels; ++channel)
            buffer.setSample(channel, sample, 0.0f);

        ++lookAheadWriteIndex;
        if (lookAheadWriteIndex >= delayBufferSize)
            lookAheadWriteIndex = 0;
    }
}

void LevelRider::applyGain(juce::AudioBuffer<float>& buffer) noexcept
{
    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = buffer.getNumChannels();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto currentGain = gain.getNextValue();

        for (int channel = 0; channel < numChannels; ++channel)
            buffer.getWritePointer(channel)[sample] *= currentGain;
    }
}
} // namespace stagemind
