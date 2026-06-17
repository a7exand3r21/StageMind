#pragma once

namespace stagemind
{
class LinkActivityEnvelope final
{
public:
    void prepare(double sampleRateToUse, int maxBlockSizeToUse) noexcept;
    void reset() noexcept;
    float process(float rawActivity, int numSamples) noexcept;
    float getCurrentValue() const noexcept { return currentValue; }

private:
    static float coefficientFor(double sampleRate, int numSamples, double timeSeconds) noexcept;

    double sampleRate = 44100.0;
    int maxBlockSize = 512;
    float currentValue = 0.0f;
};
} // namespace stagemind

