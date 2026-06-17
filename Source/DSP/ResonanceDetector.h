#pragma once

#include "ResonanceTypes.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

namespace stagemind
{
struct ResonanceDetectorConfig
{
    float cleanUp = 0.3f;
    float sensitivity = 0.5f;
    float maxReductionDb = 4.0f;
};

class ResonanceDetector
{
public:
    static constexpr int fftOrder = 11;
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int numBins = fftSize / 2;

    ResonanceDetector();

    void prepare(double sampleRate, int maximumBlockSize);
    void reset() noexcept;

    ResonanceSnapshot processBlock(
        const juce::AudioBuffer<float>& buffer,
        const ResonanceDetectorConfig& config) noexcept;

private:
    void pushSample(float sample, const ResonanceDetectorConfig& config) noexcept;
    void analyzeFrame(const ResonanceDetectorConfig& config) noexcept;
    void considerPeak(ResonancePeak peak) noexcept;
    float frequencyForBin(int bin) const noexcept;

    double currentSampleRate = 44100.0;
    int fifoIndex = 0;
    ResonanceSnapshot latestSnapshot;

    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;

    std::array<float, fftSize> fifo {};
    std::array<float, fftSize * 2> fftData {};
    std::array<float, numBins> smoothedDb {};
    std::array<ResonancePeak, maxResonancePeaks> candidatePeaks {};
};
} // namespace stagemind
