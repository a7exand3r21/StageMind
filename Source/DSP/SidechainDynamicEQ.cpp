#include "SidechainDynamicEQ.h"

namespace stagemind
{
namespace
{
float sortedStart(float startHz, float endHz) noexcept
{
    return juce::jlimit(20.0f, 20000.0f, std::min(startHz, endHz));
}

float sortedEnd(float startHz, float endHz) noexcept
{
    return juce::jlimit(20.0f, 20000.0f, std::max(startHz, endHz));
}
} // namespace

void SidechainDynamicEQ::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    reset();
}

void SidechainDynamicEQ::reset() noexcept
{
    for (auto& band : bands)
        band.reset();

    lastNonEmptyProfile = {};
}

float SidechainDynamicEQ::process(
    juce::AudioBuffer<float>& buffer,
    const SidechainAnalysis& sidechain,
    const SidechainDynamicEQConfig& config) noexcept
{
    auto profile = makeProfile(config);

    if (profile.bandCount > 0)
        lastNonEmptyProfile = profile;
    else if (! config.enabled && lastNonEmptyProfile.bandCount > 0)
        profile = lastNonEmptyProfile;

    const auto bandCount = juce::jmin<size_t>(profile.bandCount, maxSidechainBands);

    if (bandCount == 0 || buffer.getNumChannels() == 0 || buffer.getNumSamples() == 0)
        return 0.0f;

    const auto attackCoefficient = coefficientForMs(currentSampleRate, config.attackMs);
    const auto releaseCoefficient = coefficientForMs(currentSampleRate, config.releaseMs);

    for (size_t bandIndex = 0; bandIndex < bandCount; ++bandIndex)
        bands[bandIndex].configure(profile.bands[bandIndex], currentSampleRate);

    float maxReductionDb = 0.0f;
    const auto numSamples = buffer.getNumSamples();

    if (buffer.getNumChannels() < 2)
    {
        auto* mono = buffer.getWritePointer(0);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            auto value = mono[sample];

            for (size_t bandIndex = 0; bandIndex < bandCount; ++bandIndex)
            {
                auto& band = bands[bandIndex];
                const auto target = targetReductionDb(sidechain, config, band.band);
                band.smoothReduction(target, attackCoefficient, releaseCoefficient);
                maxReductionDb = std::max(maxReductionDb, band.currentReductionDb);

                if (band.band.processMid)
                    value = band.processMid(value);
            }

            mono[sample] = value;
        }

        return maxReductionDb;
    }

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        auto mid = (left[sample] + right[sample]) * 0.5f;
        auto side = (left[sample] - right[sample]) * 0.5f;

        for (size_t bandIndex = 0; bandIndex < bandCount; ++bandIndex)
        {
            auto& band = bands[bandIndex];
            const auto target = targetReductionDb(sidechain, config, band.band);
            band.smoothReduction(target, attackCoefficient, releaseCoefficient);
            maxReductionDb = std::max(maxReductionDb, band.currentReductionDb);

            if (band.band.processMid)
                mid = band.processMid(mid);

            if (band.band.processSide)
                side = band.processSide(side);
        }

        left[sample] = mid + side;
        right[sample] = mid - side;
    }

    return maxReductionDb;
}

SidechainConflictProfile SidechainDynamicEQ::makeProfile(const SidechainDynamicEQConfig& config) noexcept
{
    SidechainConflictProfile profile;
    profile.mode = config.mode;

    switch (config.mode)
    {
        case SidechainConflictMode::VocalDucksInstrument:
            profile.bands[0] = { 1800.0f, 5600.0f, 5.5f, true, false };
            profile.bandCount = 1;
            profile.preserveLowEnd = true;
            break;

        case SidechainConflictMode::KickDucksBass:
            profile.bands[0] = { 38.0f, 130.0f, 8.0f, true, false };
            profile.bandCount = 1;
            profile.preserveLowEnd = false;
            break;

        case SidechainConflictMode::SnareDucksInstrument:
            profile.bands[0] = { 1200.0f, 4200.0f, 5.0f, true, false };
            profile.bandCount = 1;
            profile.preserveLowEnd = true;
            profile.preserveTransient = true;
            break;

        case SidechainConflictMode::LeadDucksPad:
            profile.bands[0] = { 250.0f, 1200.0f, 3.5f, true, false };
            profile.bands[1] = { 1200.0f, 5000.0f, 5.5f, true, true };
            profile.bandCount = 2;
            profile.preserveLowEnd = true;
            break;

        case SidechainConflictMode::Custom:
            profile.bands[0] = { sortedStart(config.customRangeStartHz, config.customRangeEndHz),
                                 sortedEnd(config.customRangeStartHz, config.customRangeEndHz),
                                 6.0f,
                                 true,
                                 false };
            profile.bandCount = 1;
            profile.preserveLowEnd = true;
            break;

        case SidechainConflictMode::MakeSpace:
            profile.bands[0] = { 220.0f, 1000.0f, 3.2f, true, false };
            profile.bands[1] = { 900.0f, 5200.0f, 5.0f, true, true };
            profile.bandCount = 2;
            profile.preserveLowEnd = true;
            break;

        case SidechainConflictMode::Off:
        default:
            profile.bandCount = 0;
            break;
    }

    return profile;
}

float SidechainDynamicEQ::OnePolePath::processLowPass(float input, float coefficient) noexcept
{
    lowState += coefficient * (input - lowState);
    return lowState;
}

float SidechainDynamicEQ::OnePolePath::processHighPass(float input, float coefficient) noexcept
{
    return input - processLowPass(input, coefficient);
}

void SidechainDynamicEQ::BandPath::reset() noexcept
{
    startHighPass.reset();
    endLowPass.reset();
}

float SidechainDynamicEQ::BandPath::processBand(float input, float startCoefficient, float endCoefficient) noexcept
{
    const auto highPassed = startHighPass.processHighPass(input, startCoefficient);
    return endLowPass.processLowPass(highPassed, endCoefficient);
}

void SidechainDynamicEQ::DynamicBand::reset() noexcept
{
    midPath.reset();
    sidePath.reset();
    currentReductionDb = 0.0f;
}

void SidechainDynamicEQ::DynamicBand::configure(const SidechainBand& newBand, double sampleRate) noexcept
{
    band = newBand;

    const auto startHz = sortedStart(newBand.frequencyStartHz, newBand.frequencyEndHz);
    const auto endHz = sortedEnd(newBand.frequencyStartHz, newBand.frequencyEndHz);

    startCoefficient = coefficientForCutoff(sampleRate, startHz);
    endCoefficient = coefficientForCutoff(sampleRate, endHz);
}

void SidechainDynamicEQ::DynamicBand::smoothReduction(float targetReductionDb, float attackCoefficient, float releaseCoefficient) noexcept
{
    const auto coefficient = targetReductionDb > currentReductionDb ? attackCoefficient : releaseCoefficient;
    currentReductionDb = coefficient * currentReductionDb + (1.0f - coefficient) * targetReductionDb;

    if (currentReductionDb < 0.0001f)
        currentReductionDb = 0.0f;
}

float SidechainDynamicEQ::DynamicBand::processMid(float input) noexcept
{
    const auto bandSignal = midPath.processBand(input, startCoefficient, endCoefficient);
    const auto cutLinear = 1.0f - juce::Decibels::decibelsToGain(-currentReductionDb);
    return input - bandSignal * cutLinear;
}

float SidechainDynamicEQ::DynamicBand::processSide(float input) noexcept
{
    const auto bandSignal = sidePath.processBand(input, startCoefficient, endCoefficient);
    const auto cutLinear = 1.0f - juce::Decibels::decibelsToGain(-currentReductionDb);
    return input - bandSignal * cutLinear;
}

float SidechainDynamicEQ::coefficientForCutoff(double sampleRate, float cutoffHz) noexcept
{
    const auto safeSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    const auto safeCutoff = juce::jlimit(20.0f, static_cast<float> (safeSampleRate * 0.45), cutoffHz);
    const auto omega = juce::MathConstants<float>::twoPi * safeCutoff / static_cast<float> (safeSampleRate);
    return juce::jlimit(0.0f, 1.0f, 1.0f - std::exp(-omega));
}

float SidechainDynamicEQ::coefficientForMs(double sampleRate, float milliseconds) noexcept
{
    const auto safeMs = juce::jlimit(1.0f, 1000.0f, milliseconds);
    const auto samples = std::max(1.0, sampleRate * static_cast<double> (safeMs) * 0.001);
    return static_cast<float> (std::exp(-1.0 / samples));
}

float SidechainDynamicEQ::targetReductionDb(
    const SidechainAnalysis& sidechain,
    const SidechainDynamicEQConfig& config,
    const SidechainBand& band) noexcept
{
    if (! config.enabled || ! sidechain.isActive)
        return 0.0f;

    const auto normalizedEnvelope = juce::jlimit(0.0f, 1.0f, sidechain.envelope * 6.5f);
    const auto amount = juce::jlimit(0.0f, 1.0f, config.amount);
    const auto shapedAmount = amount * (0.70f + amount * 0.30f);
    const auto maxReduction = juce::jlimit(0.0f, 10.0f, band.maxReductionDb);

    return maxReduction * shapedAmount * normalizedEnvelope;
}
} // namespace stagemind
