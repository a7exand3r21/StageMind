#include "../Source/DSP/DynamicEQ.h"
#include "../Source/DSP/ResonanceDetector.h"
#include "../Source/DSP/ResonanceLearner.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
constexpr double testSampleRate = 48000.0;
constexpr double flStudioDefaultSampleRate = 44100.0;
constexpr int blockSize = 256;
constexpr float twoPi = juce::MathConstants<float>::twoPi;

void expect(bool condition, const char* message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

float sineSample(float frequencyHz, int sampleIndex, float gain = 1.0f)
{
    const auto phase = twoPi * frequencyHz * static_cast<float> (sampleIndex) / static_cast<float> (testSampleRate);
    return std::sin(phase) * gain;
}

float rms(const juce::AudioBuffer<float>& buffer)
{
    double sum = 0.0;
    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = buffer.getNumChannels();

    for (int channel = 0; channel < numChannels; ++channel)
    {
        const auto* samples = buffer.getReadPointer(channel);

        for (int sample = 0; sample < numSamples; ++sample)
            sum += static_cast<double> (samples[sample]) * samples[sample];
    }

    return static_cast<float> (std::sqrt(sum / static_cast<double> (numSamples * numChannels)));
}

void fillStereoSine(juce::AudioBuffer<float>& buffer, float frequencyHz, int startSample, float gain)
{
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto value = sineSample(frequencyHz, startSample + sample, gain);

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample(channel, sample, value);
    }
}

stagemind::ResonanceSnapshot feedSine(
    stagemind::ResonanceDetector& detector,
    float frequencyHz,
    int totalSamples,
    float gain = 0.8f)
{
    juce::AudioBuffer<float> buffer(2, blockSize);
    stagemind::ResonanceDetectorConfig config;
    config.cleanUp = 1.0f;
    config.sensitivity = 1.0f;
    config.maxReductionDb = 6.0f;

    stagemind::ResonanceSnapshot snapshot;

    for (int start = 0; start < totalSamples; start += blockSize)
    {
        fillStereoSine(buffer, frequencyHz, start, gain);
        snapshot = detector.processBlock(buffer, config);
    }

    return snapshot;
}

void detectsInjectedNarrowPeak()
{
    stagemind::ResonanceDetector detector;
    detector.prepare(testSampleRate, blockSize);

    const auto snapshot = feedSine(detector, 3000.0f, stagemind::ResonanceDetector::fftSize * 8);

    expect(snapshot.peakCount > 0, "detector should report at least one narrow peak");

    bool found = false;
    for (int i = 0; i < static_cast<int> (snapshot.peakCount); ++i)
    {
        const auto frequency = snapshot.peaks[static_cast<size_t> (i)].frequencyHz;
        found = found || std::abs(frequency - 3000.0f) < 80.0f;
    }

    expect(found, "detector should find the injected 3 kHz peak");
}

void lowFrequencyMaterialDoesNotReadPastFftEdges()
{
    stagemind::ResonanceDetector detector;
    detector.prepare(flStudioDefaultSampleRate, blockSize);

    juce::AudioBuffer<float> buffer(2, blockSize);
    stagemind::ResonanceDetectorConfig config;
    config.cleanUp = 1.0f;
    config.sensitivity = 1.0f;
    config.maxReductionDb = 6.0f;

    stagemind::ResonanceSnapshot snapshot;

    for (int start = 0; start < stagemind::ResonanceDetector::fftSize * 8; start += blockSize)
    {
        for (int sample = 0; sample < blockSize; ++sample)
        {
            const auto absoluteSample = start + sample;
            const auto phase = twoPi * 140.0f * static_cast<float> (absoluteSample) / static_cast<float> (flStudioDefaultSampleRate);
            const auto value = std::sin(phase) * 0.8f;

            buffer.setSample(0, sample, value);
            buffer.setSample(1, sample, value);
        }

        snapshot = detector.processBlock(buffer, config);
    }

    expect(snapshot.peakCount <= stagemind::maxResonancePeaks, "low-frequency material should stay within the fixed peak array");
}

void capsResonancesAtFourPeaks()
{
    stagemind::ResonanceDetector detector;
    detector.prepare(testSampleRate, blockSize);

    juce::AudioBuffer<float> buffer(2, blockSize);
    stagemind::ResonanceDetectorConfig config;
    config.cleanUp = 1.0f;
    config.sensitivity = 1.0f;
    config.maxReductionDb = 7.0f;

    stagemind::ResonanceSnapshot snapshot;

    for (int start = 0; start < stagemind::ResonanceDetector::fftSize * 10; start += blockSize)
    {
        for (int sample = 0; sample < blockSize; ++sample)
        {
            const auto absoluteSample = start + sample;
            const auto value =
                sineSample(750.0f, absoluteSample, 0.18f)
                + sineSample(1500.0f, absoluteSample, 0.18f)
                + sineSample(2250.0f, absoluteSample, 0.18f)
                + sineSample(3000.0f, absoluteSample, 0.18f)
                + sineSample(4500.0f, absoluteSample, 0.18f)
                + sineSample(6000.0f, absoluteSample, 0.18f);

            buffer.setSample(0, sample, value);
            buffer.setSample(1, sample, value);
        }

        snapshot = detector.processBlock(buffer, config);
    }

    expect(snapshot.peakCount <= stagemind::maxResonancePeaks, "detector should cap active peaks at four");
}

void suppressesArtificialPeak()
{
    stagemind::DynamicEQ dynamicEQ;
    dynamicEQ.prepare(testSampleRate, blockSize);

    stagemind::ResonanceSnapshot snapshot;
    snapshot.peakCount = 1;
    snapshot.peaks[0].frequencyHz = 3000.0f;
    snapshot.peaks[0].severity = 1.0f;
    snapshot.peaks[0].suggestedQ = 8.0f;
    snapshot.peaks[0].suggestedReductionDb = 6.0f;

    stagemind::ResonanceSuppressionConfig config;
    config.resonanceAmount = 1.0f;
    config.maxReductionDb = 6.0f;
    config.attackMs = 1.0f;
    config.releaseMs = 80.0f;

    juce::AudioBuffer<float> buffer(2, blockSize);
    float before = 0.0f;
    float after = 0.0f;
    float reductionDb = 0.0f;

    for (int block = 0; block < 20; ++block)
    {
        fillStereoSine(buffer, 3000.0f, block * blockSize, 0.6f);
        before = rms(buffer);
        reductionDb = dynamicEQ.processResonances(buffer, snapshot, config);
        after = rms(buffer);
    }

    expect(reductionDb > 0.1f, "dynamic EQ should report resonance reduction");
    expect(after < before * 0.98f, "dynamic EQ should reduce the resonant sine");
}

void zeroAmountLeavesFreshBufferUntouched()
{
    stagemind::DynamicEQ dynamicEQ;
    dynamicEQ.prepare(testSampleRate, blockSize);

    stagemind::ResonanceSnapshot snapshot;
    snapshot.peakCount = 1;
    snapshot.peaks[0].frequencyHz = 3000.0f;
    snapshot.peaks[0].severity = 1.0f;
    snapshot.peaks[0].suggestedQ = 8.0f;
    snapshot.peaks[0].suggestedReductionDb = 6.0f;

    stagemind::ResonanceSuppressionConfig config;
    config.resonanceAmount = 0.0f;

    juce::AudioBuffer<float> buffer(2, blockSize);
    fillStereoSine(buffer, 3000.0f, 0, 0.6f);
    const auto before = rms(buffer);
    const auto reductionDb = dynamicEQ.processResonances(buffer, snapshot, config);
    const auto after = rms(buffer);

    expect(reductionDb == 0.0f, "zero resonance amount should report no reduction from a reset state");
    expect(std::abs(after - before) < 1.0e-6f, "zero resonance amount should leave a fresh buffer untouched");
}

void learnerHoldsFlickeringLivePeaks()
{
    stagemind::ResonanceLearner learner;
    learner.prepare(testSampleRate);

    stagemind::ResonanceSnapshot live;
    live.peakCount = 1;
    live.peaks[0].frequencyHz = 1200.0f;
    live.peaks[0].severity = 0.7f;
    live.peaks[0].suggestedQ = 7.0f;
    live.peaks[0].suggestedReductionDb = 3.0f;

    auto snapshot = learner.process(live, blockSize);
    expect(snapshot.peakCount == 1, "learner should publish a detected live peak");

    snapshot = learner.process({}, blockSize);
    expect(snapshot.peakCount == 1, "learner should hold recent peaks instead of flickering");

    for (int i = 0; i < static_cast<int> (testSampleRate * 2.0 / blockSize); ++i)
        snapshot = learner.process({}, blockSize);

    expect(snapshot.peakCount == 0, "learner should release held peaks after the hold window");
}

void learnerCapturesStableCorrections()
{
    stagemind::ResonanceLearner learner;
    learner.prepare(testSampleRate);
    learner.beginLearn();

    stagemind::ResonanceSnapshot live;
    live.peakCount = 1;
    live.peaks[0].frequencyHz = 2500.0f;
    live.peaks[0].severity = 0.8f;
    live.peaks[0].suggestedQ = 8.0f;
    live.peaks[0].suggestedReductionDb = 4.0f;

    stagemind::ResonanceSnapshot snapshot;

    for (int i = 0; i < static_cast<int> (testSampleRate * 4.2 / blockSize); ++i)
        snapshot = learner.process(live, blockSize);

    expect(learner.getStatus() == stagemind::ResonanceLearner::Status::Learned, "learner should enter learned state");
    expect(snapshot.peakCount == 1, "learner should publish learned correction");
    expect(std::abs(snapshot.peaks[0].frequencyHz - 2500.0f) < 80.0f, "learned correction should preserve frequency");

    for (int i = 0; i < static_cast<int> (testSampleRate * 2.0 / blockSize); ++i)
        snapshot = learner.process({}, blockSize);

    expect(snapshot.peakCount == 1, "learned correction should remain stable after silence");
}

void learnerRestoresProfileAndRidesLiveResonance()
{
    stagemind::ResonanceLearner learner;
    learner.prepare(testSampleRate);

    stagemind::ResonanceSnapshot saved;
    saved.peakCount = 1;
    saved.peaks[0].frequencyHz = 1800.0f;
    saved.peaks[0].severity = 0.8f;
    saved.peaks[0].suggestedQ = 8.0f;
    saved.peaks[0].suggestedReductionDb = 4.0f;
    learner.setLearnedSnapshot(saved);

    expect(learner.getStatus() == stagemind::ResonanceLearner::Status::Learned, "restored profile should mark learner as learned");
    expect(learner.getLearnedSnapshot().peakCount == 1, "restored profile should be readable for state save");

    auto snapshot = learner.process({}, blockSize);
    expect(snapshot.peakCount == 1, "restored profile should stay visible");
    expect(snapshot.peaks[0].suggestedReductionDb == 0.0f, "restored profile should not cut silence");

    stagemind::ResonanceSnapshot live;
    live.peakCount = 1;
    live.peaks[0].frequencyHz = 1820.0f;
    live.peaks[0].severity = 0.7f;
    live.peaks[0].suggestedQ = 7.0f;
    live.peaks[0].suggestedReductionDb = 3.0f;

    snapshot = learner.process(live, blockSize);
    expect(snapshot.peakCount == 1, "live match should keep the learned profile visible");
    expect(snapshot.peaks[0].suggestedReductionDb > 0.1f, "live match should ride reduction up");
}
} // namespace

void runSpatialEnhancementTests();
void runStageMindLinkRegistryTests();
void runLinkActivityEnvelopeTests();
void runLinkSuggestionEngineTests();
void runLinkSpectralAnalyzerTests();
void runRideMemoryTests();
void runRideTimelineMemoryTests();

int main()
{
    detectsInjectedNarrowPeak();
    lowFrequencyMaterialDoesNotReadPastFftEdges();
    capsResonancesAtFourPeaks();
    suppressesArtificialPeak();
    zeroAmountLeavesFreshBufferUntouched();
    learnerHoldsFlickeringLivePeaks();
    learnerCapturesStableCorrections();
    learnerRestoresProfileAndRidesLiveResonance();
    runSpatialEnhancementTests();
    runStageMindLinkRegistryTests();
    runLinkActivityEnvelopeTests();
    runLinkSuggestionEngineTests();
    runLinkSpectralAnalyzerTests();
    runRideMemoryTests();
    runRideTimelineMemoryTests();

    std::cout << "StageMindDSPTests passed\n";
    return 0;
}
