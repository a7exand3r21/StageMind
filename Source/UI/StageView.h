#pragma once

#include <JuceHeader.h>

namespace stagemind
{
class StageView final : public juce::Component
{
public:
    struct State
    {
        float pan = 0.0f;
        float depth = 0.3f;
        float width = 0.5f;
        float motion = 0.0f;
        float requestedMotion = 0.0f;
        float motionPhase = 0.0f;
        float correlation = 1.0f;
        bool motionAllowed = true;
    };

    void setState(State newState);
    void paint(juce::Graphics& g) override;

private:
    State state;
};
} // namespace stagemind
