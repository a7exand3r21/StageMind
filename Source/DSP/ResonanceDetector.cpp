#include "ResonanceDetector.h"

namespace stagemind
{
namespace
{
float safeDb(float value) noexcept
{
    return juce::Decibels::gainToDecibels(std::max(value, 1.0e-9f));
}

float thresholdFor(const ResonanceDetectorConfig& config) noexcept
{
    const auto sensitivity = juce::jlimit(0.0f, 1.0f, config.sensitivity);
    const auto cleanUp = juce::jlimit(0.0f, 1.0f, config.cleanUp);
    return juce::jmap(sensitivity * 0.7f + cleanUp * 0.3f, 0.0f, 1.0f, 18.0f, 6.0f);
}

constexpr int localAverageRadius = 12;
constexpr int narrownessRadius = 3;
} // namespace

ResonanceDetector::ResonanceDetector()
    : fft(fftOrder),
      window(static_cast<size_t> (fftSize), juce::dsp::WindowingFunction<float>::hann, true)
{
    reset();
}

void ResonanceDetector::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    reset();
}

void ResonanceDetector::reset() noexcept
{
    fifo.fill(0.0f);
    fftData.fill(0.0f);
    smoothedDb.fill(-120.0f);
    candidatePeaks.fill({});
    latestSnapshot = {};
    fifoIndex = 0;
}

ResonanceSnapshot ResonanceDetector::processBlock(
    const juce::AudioBuffer<float>& buffer,
    const ResonanceDetectorConfig& config) noexcept
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    if (numChannels == 0 || numSamples == 0)
        return latestSnapshot;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float mono = 0.0f;

        for (int channel = 0; channel < numChannels; ++channel)
            mono += buffer.getReadPointer(channel)[sample];

        pushSample(mono / static_cast<float> (numChannels), config);
    }

    return latestSnapshot;
}

void ResonanceDetector::pushSample(float sample, const ResonanceDetectorConfig& config) noexcept
{
    fifo[static_cast<size_t> (fifoIndex)] = sample;
    ++fifoIndex;

    if (fifoIndex >= fftSize)
    {
        analyzeFrame(config);
        fifoIndex = 0;
    }
}

void ResonanceDetector::analyzeFrame(const ResonanceDetectorConfig& config) noexcept
{
    std::copy(fifo.begin(), fifo.end(), fftData.begin());
    std::fill(fftData.begin() + fftSize, fftData.end(), 0.0f);

    window.multiplyWithWindowingTable(fftData.data(), static_cast<size_t> (fftSize));
    fft.performFrequencyOnlyForwardTransform(fftData.data(), true);

    latestSnapshot = {};
    candidatePeaks.fill({});

    for (int bin = 1; bin < numBins; ++bin)
    {
        const auto magnitude = fftData[static_cast<size_t> (bin)] / static_cast<float> (fftSize);
        const auto db = safeDb(magnitude);
        auto& smoothed = smoothedDb[static_cast<size_t> (bin)];
        smoothed = smoothed * 0.82f + db * 0.18f;
    }

    const auto thresholdDb = thresholdFor(config);
    const auto maxReductionDb = juce::jlimit(0.0f, 7.0f, config.maxReductionDb);

    for (int bin = localAverageRadius; bin < numBins - localAverageRadius; ++bin)
    {
        const auto frequencyHz = frequencyForBin(bin);

        if (frequencyHz < 120.0f || frequencyHz > 12000.0f)
            continue;

        const auto center = smoothedDb[static_cast<size_t> (bin)];

        if (center <= smoothedDb[static_cast<size_t> (bin - 1)] || center <= smoothedDb[static_cast<size_t> (bin + 1)])
            continue;

        float localSum = 0.0f;
        int localCount = 0;

        for (int offset = -localAverageRadius; offset <= localAverageRadius; ++offset)
        {
            if (std::abs(offset) <= 1)
                continue;

            localSum += smoothedDb[static_cast<size_t> (bin + offset)];
            ++localCount;
        }

        const auto localAverage = localSum / static_cast<float> (localCount);
        const auto excessDb = center - localAverage;

        const auto narrowEnough =
            center - smoothedDb[static_cast<size_t> (bin - narrownessRadius)] > 2.0f
            && center - smoothedDb[static_cast<size_t> (bin + narrownessRadius)] > 2.0f;

        if (excessDb < thresholdDb || ! narrowEnough)
            continue;

        ResonancePeak peak;
        peak.frequencyHz = frequencyHz;
        peak.severity = juce::jlimit(0.0f, 1.0f, excessDb / 24.0f);
        peak.suggestedQ = juce::jlimit(1.0f, 12.0f, frequencyHz < 500.0f ? 4.0f : 7.0f);
        peak.suggestedReductionDb = juce::jlimit(0.0f, maxReductionDb, maxReductionDb * peak.severity);

        considerPeak(peak);
    }
}

void ResonanceDetector::considerPeak(ResonancePeak peak) noexcept
{
    auto insertIndex = static_cast<int> (maxResonancePeaks);

    for (int i = 0; i < static_cast<int> (maxResonancePeaks); ++i)
    {
        if (peak.severity > candidatePeaks[static_cast<size_t> (i)].severity)
        {
            insertIndex = i;
            break;
        }
    }

    if (insertIndex >= static_cast<int> (maxResonancePeaks))
        return;

    for (int i = static_cast<int> (maxResonancePeaks) - 1; i > insertIndex; --i)
        candidatePeaks[static_cast<size_t> (i)] = candidatePeaks[static_cast<size_t> (i - 1)];

    candidatePeaks[static_cast<size_t> (insertIndex)] = peak;

    latestSnapshot.peakCount = 0;
    for (size_t i = 0; i < maxResonancePeaks; ++i)
    {
        latestSnapshot.peaks[i] = candidatePeaks[i];

        if (candidatePeaks[i].frequencyHz > 0.0f)
            ++latestSnapshot.peakCount;
    }
}

float ResonanceDetector::frequencyForBin(int bin) const noexcept
{
    return static_cast<float> (static_cast<double> (bin) * currentSampleRate / static_cast<double> (fftSize));
}
} // namespace stagemind
