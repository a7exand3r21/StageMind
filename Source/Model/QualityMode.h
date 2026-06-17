#pragma once

#include <JuceHeader.h>

namespace stagemind
{
enum class AnalyzerQuality
{
    Eco = 0,
    Normal,
    High
};

enum class ProcessingQuality
{
    Draft = 0,
    Normal,
    High
};

inline juce::StringArray makeQualityNames()
{
    return { "Eco", "Normal", "High" };
}

inline juce::StringArray makeProcessingQualityNames()
{
    return { "Draft", "Normal", "High" };
}
} // namespace stagemind
