#pragma once

#include <juce_core/juce_core.h>

namespace stagemind
{
enum class PluginMode
{
    Node = 0,
    Director
};

inline juce::StringArray makePluginModeNames()
{
    return { "Node", "Director" };
}

inline PluginMode pluginModeFromIndex(int index) noexcept
{
    return index == 1 ? PluginMode::Director : PluginMode::Node;
}
} // namespace stagemind
