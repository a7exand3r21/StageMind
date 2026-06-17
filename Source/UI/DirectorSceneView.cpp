#include "DirectorSceneView.h"
#include "../Model/TrackRole.h"
#include "Theme.h"

#include <algorithm>
#include <cmath>

namespace stagemind
{
namespace
{
juce::String shortRoleLabel(TrackRole role)
{
    switch (role)
    {
        case TrackRole::LeadVocal:        return "Lead Vox";
        case TrackRole::BackingVocal:     return "Back Vox";
        case TrackRole::RhythmGuitarSingle:
        case TrackRole::SunoGuitar:       return "Guitar";
        case TrackRole::LeadGuitar:       return "Lead Gtr";
        case TrackRole::SynthLead:        return "Synth Lead";
        case TrackRole::SunoVocal:        return "Suno Vox";
        case TrackRole::SunoInstrumental: return "Suno Inst";
        case TrackRole::SunoDrums:        return "Suno Drums";
        case TrackRole::SunoBass:         return "Suno Bass";
        case TrackRole::SunoSynthPad:     return "Suno Pad";
        case TrackRole::SunoFX:           return "Suno FX";
        default:                          return labelForRole(role);
    }
}

juce::Point<float> positionForNode(const LinkPeerSnapshot& node, juce::Rectangle<float> plot) noexcept
{
    const auto pan01 = juce::jlimit(0.0f, 1.0f, (node.pan + 1.0f) * 0.5f);
    return {
        plot.getX() + plot.getWidth() * pan01,
        plot.getBottom() - plot.getHeight() * juce::jlimit(0.0f, 1.0f, node.depth)
    };
}
} // namespace

void DirectorSceneView::setSnapshot(const LinkGroupSnapshot& newSnapshot)
{
    snapshot = newSnapshot;
    repaint();
}

void DirectorSceneView::setConnections(
    const std::array<DirectorSceneConnection, maxDirectorSceneConnections>& newConnections,
    int newConnectionCount)
{
    connections = newConnections;
    connectionCount = juce::jlimit(0, maxDirectorSceneConnections, newConnectionCount);
    repaint();
}

void DirectorSceneView::setSelectedNode(std::uint32_t instanceId)
{
    if (selectedNodeId == instanceId)
        return;

    selectedNodeId = instanceId;
    repaint();
}

juce::Rectangle<float> DirectorSceneView::plotBounds() const noexcept
{
    return getLocalBounds().reduced(34, 38).toFloat();
}

std::uint32_t DirectorSceneView::findNearestNode(juce::Point<float> position) const noexcept
{
    if (snapshot.count <= 0)
        return invalidLinkInstanceId;

    const auto plot = plotBounds();
    auto bestId = invalidLinkInstanceId;
    auto bestDistance = 58.0f;

    for (int index = 0; index < snapshot.count; ++index)
    {
        const auto& node = snapshot.peers[static_cast<size_t> (index)];
        const auto nodePosition = positionForNode(node, plot);
        const auto widthHalf = 18.0f + juce::jlimit(0.0f, 1.0f, node.width) * 38.0f;
        const auto dx = std::max(0.0f, std::abs(position.x - nodePosition.x) - widthHalf);
        const auto dy = std::abs(position.y - nodePosition.y);
        const auto distance = std::hypot(dx, dy);
        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestId = node.instanceId;
        }
    }

    return bestId;
}

std::pair<float, float> DirectorSceneView::panDepthForPoint(juce::Point<float> position) const noexcept
{
    const auto plot = plotBounds();
    if (plot.isEmpty())
        return { 0.0f, 0.0f };

    const auto x01 = juce::jlimit(0.0f, 1.0f, (position.x - plot.getX()) / std::max(1.0f, plot.getWidth()));
    const auto y01 = juce::jlimit(0.0f, 1.0f, (position.y - plot.getY()) / std::max(1.0f, plot.getHeight()));
    return { x01 * 2.0f - 1.0f, 1.0f - y01 };
}

void DirectorSceneView::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(theme::panel);
    g.fillRoundedRectangle(bounds, static_cast<float> (theme::corner));
    g.setColour(theme::border);
    g.drawRoundedRectangle(bounds.reduced(0.5f), static_cast<float> (theme::corner), 1.0f);

    auto stage = getLocalBounds().reduced(22);
    g.setColour(theme::border.withAlpha(0.8f));
    g.drawHorizontalLine(stage.getCentreY(), static_cast<float> (stage.getX()), static_cast<float> (stage.getRight()));
    g.drawVerticalLine(stage.getCentreX(), static_cast<float> (stage.getY()), static_cast<float> (stage.getBottom()));

    g.setColour(theme::textMuted);
    g.setFont(juce::FontOptions { 12.0f });
    g.drawText("Back", stage.removeFromTop(18), juce::Justification::centredRight);
    g.drawText("Front", getLocalBounds().reduced(22).removeFromBottom(18), juce::Justification::centredRight);

    if (snapshot.count <= 0)
    {
        g.setColour(theme::textMuted);
        g.setFont(juce::FontOptions { 15.0f });
        g.drawText("No linked Nodes in this group", getLocalBounds(), juce::Justification::centred);
        return;
    }

    const auto plot = plotBounds();
    const auto findNodePosition = [this, plot](std::uint32_t instanceId, juce::Point<float>& position) noexcept
    {
        for (int index = 0; index < snapshot.count; ++index)
        {
            const auto& node = snapshot.peers[static_cast<size_t> (index)];
            if (node.instanceId == instanceId)
            {
                position = positionForNode(node, plot);
                return true;
            }
        }

        return false;
    };

    for (int index = 0; index < connectionCount; ++index)
    {
        const auto& connection = connections[static_cast<size_t> (index)];
        juce::Point<float> peer;
        juce::Point<float> target;

        if (! findNodePosition(connection.peerId, peer) || ! findNodePosition(connection.targetId, target))
            continue;

        const auto colour = connection.active ? theme::accentWarm : theme::textMuted;
        const auto line = juce::Line<float> { peer, target };
        const auto midpoint = line.getPointAlongLine(0.52f);

        g.setColour(colour.withAlpha(connection.active ? 0.24f : 0.12f));
        g.drawLine(line, connection.active ? 7.0f : 4.0f);

        g.setColour(colour.withAlpha(connection.active ? 0.78f : 0.42f));
        g.drawArrow(line, connection.active ? 2.2f : 1.4f, 9.0f, 12.0f);

        g.setColour(theme::panelRaised.withAlpha(0.92f));
        const auto badge = juce::Rectangle<float> { midpoint.x - 42.0f, midpoint.y - 10.0f, 84.0f, 20.0f };
        g.fillRoundedRectangle(badge, 5.0f);
        g.setColour(colour);
        g.drawRoundedRectangle(badge.reduced(0.5f), 5.0f, 1.0f);
        g.setFont(juce::FontOptions { 10.5f, juce::Font::bold });
        g.drawFittedText(
            connection.label.isNotEmpty() ? connection.label : ("#" + juce::String(connection.peerId) + " -> #" + juce::String(connection.targetId)),
            badge.toNearestInt().reduced(4, 1),
            juce::Justification::centred,
            1);
    }

    for (int index = 0; index < snapshot.count; ++index)
    {
        const auto& node = snapshot.peers[static_cast<size_t> (index)];
        const auto role = static_cast<TrackRole> (node.role);
        const auto position = positionForNode(node, plot);
        const auto x = position.x;
        const auto y = position.y;
        const auto radius = 7.0f + juce::jlimit(0.0f, 1.0f, node.activity) * 9.0f;
        const auto widthHalf = 10.0f + juce::jlimit(0.0f, 1.0f, node.width) * 34.0f;
        const auto colour = node.correlation < 0.1f
            ? theme::warning
            : (node.activity > 0.08f ? theme::accent : theme::textMuted);
        const auto selected = node.instanceId == selectedNodeId;

        g.setColour(colour.withAlpha(0.20f));
        g.fillEllipse(x - radius * 1.8f, y - radius * 1.8f, radius * 3.6f, radius * 3.6f);
        g.setColour(colour.withAlpha(0.55f));
        g.drawLine(x - widthHalf, y, x + widthHalf, y, 3.0f);
        if (selected)
        {
            const auto selection = juce::Rectangle<float> { x - widthHalf - 8.0f, y - radius - 7.0f, widthHalf * 2.0f + 16.0f, radius * 2.0f + 14.0f };
            g.setColour(theme::accentWarm.withAlpha(0.90f));
            g.drawRoundedRectangle(selection, 6.0f, 1.6f);
        }
        g.setColour(colour);
        g.fillEllipse(x - radius, y - radius, radius * 2.0f, radius * 2.0f);

        g.setColour(theme::text);
        g.setFont(juce::FontOptions { 11.0f, juce::Font::bold });
        g.drawText(
            "#" + juce::String(node.instanceId) + " " + shortRoleLabel(role),
            juce::Rectangle<float> { x - 56.0f, y + radius + 3.0f, 112.0f, 18.0f },
            juce::Justification::centred);
    }
}

void DirectorSceneView::mouseDown(const juce::MouseEvent& event)
{
    const auto nodeId = findNearestNode(event.position);
    draggingNodeId = nodeId;

    if (nodeId == invalidLinkInstanceId)
        return;

    selectedNodeId = nodeId;
    repaint();

    if (onNodeSelected)
        onNodeSelected(nodeId);
}

void DirectorSceneView::mouseDrag(const juce::MouseEvent& event)
{
    if (draggingNodeId == invalidLinkInstanceId)
        return;

    const auto [pan, depth] = panDepthForPoint(event.position);
    if (onNodeMoved)
        onNodeMoved(draggingNodeId, pan, depth);
}

void DirectorSceneView::mouseUp(const juce::MouseEvent&)
{
    draggingNodeId = invalidLinkInstanceId;
}
} // namespace stagemind
