#pragma once

#include <JuceHeader.h>

namespace stagemind::theme
{
inline const auto background = juce::Colour { 0xffd8dadd };
inline const auto panel = juce::Colour { 0xffeef0f1 };
inline const auto panelRaised = juce::Colour { 0xfff8f9f8 };
inline const auto panelInset = juce::Colour { 0xffe2e5e7 };
inline const auto display = juce::Colour { 0xff12161a };
inline const auto displayRaised = juce::Colour { 0xff1c2228 };
inline const auto border = juce::Colour { 0xffc1c6ca };
inline const auto borderDark = juce::Colour { 0xff8f969c };
inline const auto shadow = juce::Colour { 0x33000000 };
inline const auto text = juce::Colour { 0xff273039 };
inline const auto textMuted = juce::Colour { 0xff6f7880 };
inline const auto textOnDisplay = juce::Colour { 0xfff3f6f7 };
inline const auto accent = juce::Colour { 0xff536f76 };
inline const auto accentWarm = juce::Colour { 0xffb08a55 };
inline const auto warning = juce::Colour { 0xffc75d55 };
inline const auto meterCold = juce::Colour { 0xff7c929c };
inline const auto meterWarm = juce::Colour { 0xffc59b59 };
inline const auto meterHot = juce::Colour { 0xffc65d62 };

inline constexpr int margin = 18;
inline constexpr int gap = 10;
inline constexpr int corner = 10;
} // namespace stagemind::theme
