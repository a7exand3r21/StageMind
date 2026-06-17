#pragma once

#include "../Link/StageMindLinkRegistry.h"
#include <JuceHeader.h>
#include <functional>

namespace stagemind
{
inline constexpr int maxDirectorSceneConnections = 8;

struct DirectorSceneConnection
{
    bool active = false;
    std::uint32_t targetId = invalidLinkInstanceId;
    std::uint32_t peerId = invalidLinkInstanceId;
    juce::String label;
};

class DirectorSceneView final : public juce::Component
{
public:
    void setSnapshot(const LinkGroupSnapshot& newSnapshot);
    void setConnections(
        const std::array<DirectorSceneConnection, maxDirectorSceneConnections>& newConnections,
        int newConnectionCount);
    void setSelectedNode(std::uint32_t instanceId);
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    std::function<void(std::uint32_t)> onNodeSelected;
    std::function<void(std::uint32_t, float, float)> onNodeMoved;

private:
    juce::Rectangle<float> plotBounds() const noexcept;
    std::uint32_t findNearestNode(juce::Point<float> position) const noexcept;
    std::pair<float, float> panDepthForPoint(juce::Point<float> position) const noexcept;

    LinkGroupSnapshot snapshot;
    std::array<DirectorSceneConnection, maxDirectorSceneConnections> connections;
    int connectionCount = 0;
    std::uint32_t selectedNodeId = invalidLinkInstanceId;
    std::uint32_t draggingNodeId = invalidLinkInstanceId;
};
} // namespace stagemind
