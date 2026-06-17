#pragma once

#include <JuceHeader.h>

namespace stagemind::theme
{
inline const auto background = juce::Colour { 0xff101216 };
inline const auto panel = juce::Colour { 0xff171b22 };
inline const auto panelRaised = juce::Colour { 0xff202632 };
inline const auto border = juce::Colour { 0xff333b49 };
inline const auto text = juce::Colour { 0xffeef2f8 };
inline const auto textMuted = juce::Colour { 0xff9ba6b5 };
inline const auto accent = juce::Colour { 0xff49c7b8 };
inline const auto accentWarm = juce::Colour { 0xffffb347 };
inline const auto warning = juce::Colour { 0xffff6b5f };
inline const auto meterCold = juce::Colour { 0xff4aa3ff };
inline const auto meterWarm = juce::Colour { 0xffffc857 };
inline const auto meterHot = juce::Colour { 0xffff4d6d };

inline constexpr int margin = 16;
inline constexpr int gap = 10;
inline constexpr int corner = 8;
} // namespace stagemind::theme
