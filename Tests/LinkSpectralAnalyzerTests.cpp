#include "../Source/DSP/LinkSpectralAnalyzer.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
constexpr auto testSampleRate = 48000.0;
constexpr auto blockSize = 256;
constexpr auto twoPi = juce::MathConstants<float>::twoPi;

void expect(bool condition, const char* message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

void fillStereoSine(juce::AudioBuffer<float>& buffer, float frequencyHz, int startSample, float gain)
{
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto phase = twoPi * frequencyHz * static_cast<float> (startSample + sample) / static_cast<float> (testSampleRate);
        const auto value = std::sin(phase) * gain;

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample(channel, sample, value);
    }
}

stagemind::LinkSpectralBands feedSine(float frequencyHz)
{
    stagemind::LinkSpectralAnalyzer analyzer;
    analyzer.prepare(testSampleRate);

    juce::AudioBuffer<float> buffer(2, blockSize);
    stagemind::LinkSpectralBands bands;

    for (int block = 0; block < 80; ++block)
    {
        fillStereoSine(buffer, frequencyHz, block * blockSize, 0.5f);
        bands = analyzer.processBlock(buffer);
    }

    return bands;
}

void lowToneProducesLowBandActivity()
{
    const auto bands = feedSine(80.0f);
    expect(bands.low > 0.05f, "low tone should produce low-band activity");
    expect(bands.low > bands.presence, "low tone should read stronger in low than presence");
    expect(bands.low > bands.air, "low tone should read stronger in low than air");
}

void presenceToneProducesPresenceBandActivity()
{
    const auto bands = feedSine(2200.0f);
    expect(bands.presence > 0.05f, "presence tone should produce presence-band activity");
    expect(bands.presence > bands.low, "presence tone should read stronger in presence than low");
}

void resetClearsHeldBandState()
{
    stagemind::LinkSpectralAnalyzer analyzer;
    analyzer.prepare(testSampleRate);

    juce::AudioBuffer<float> buffer(2, blockSize);
    fillStereoSine(buffer, 80.0f, 0, 0.5f);
    const auto beforeReset = analyzer.processBlock(buffer);
    analyzer.reset();
    buffer.clear();
    const auto afterReset = analyzer.processBlock(buffer);

    expect(beforeReset.low > 0.0f, "analyzer should detect energy before reset");
    expect(afterReset.low == 0.0f && afterReset.presence == 0.0f, "reset should clear held spectral state");
}
} // namespace

void runLinkSpectralAnalyzerTests()
{
    lowToneProducesLowBandActivity();
    presenceToneProducesPresenceBandActivity();
    resetClearsHeldBandState();
}
