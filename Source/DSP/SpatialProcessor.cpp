#include "SpatialProcessor.h"

namespace stagemind
{
void SpatialProcessor::prepare(double sampleRate, int maximumBlockSize)
{
    pan.reset(sampleRate, 0.05);
    pan.setCurrentAndTargetValue(0.0f);

    lowMono.prepare(sampleRate, maximumBlockSize);
    sideHighPass.prepare(sampleRate, maximumBlockSize);
    midSide.prepare(sampleRate, maximumBlockSize);
}

void SpatialProcessor::reset() noexcept
{
    pan.setCurrentAndTargetValue(pan.getTargetValue());
    lowMono.reset();
    sideHighPass.reset();
    midSide.reset();
}

void SpatialProcessor::setParams(const SpatialParams& params) noexcept
{
    pan.setTargetValue(juce::jlimit(-1.0f, 1.0f, params.pan));
    lowMono.setCutoffHz(params.monoLowCutoffHz);
    sideHighPass.setCutoffHz(params.sideHighPassHz);
    midSide.setWidthAmount(params.widthAmount);
}

void SpatialProcessor::process(juce::AudioBuffer<float>& buffer) noexcept
{
    applyPan(buffer);
    lowMono.process(buffer);
    sideHighPass.process(buffer);
    midSide.process(buffer);
}

void SpatialProcessor::applyPan(juce::AudioBuffer<float>& buffer) noexcept
{
    if (buffer.getNumChannels() < 2)
        return;

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    const auto numSamples = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto panValue = pan.getNextValue();
        const auto leftGain = panValue <= 0.0f ? 1.0f : 1.0f - panValue;
        const auto rightGain = panValue >= 0.0f ? 1.0f : 1.0f + panValue;

        left[sample] *= leftGain;
        right[sample] *= rightGain;
    }
}
} // namespace stagemind
