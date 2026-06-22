#include "../Source/Link/StageMindLinkRegistry.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
void expect(bool condition, const char* message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

stagemind::LinkPublishState makeState(int group, float activity, int role = 1)
{
    stagemind::LinkPublishState state;
    state.enabled = true;
    state.group = group;
    state.role = role;
    state.activity = activity;
    state.inputRms = activity * 0.5f;
    state.outputRms = activity * 0.4f;
    state.correlation = 0.75f;
    state.pan = 0.15f;
    state.width = 0.5f;
    state.depth = 0.3f;
    state.motion = 0.25f;
    state.cleanUp = 0.4f;
    state.resonance = 0.2f;
    return state;
}

void registryPublishesAndReadsPeerInSameGroup()
{
    auto& registry = stagemind::StageMindLinkRegistry::instance();
    registry.resetForTests();

    const auto first = registry.registerInstance();
    const auto second = registry.registerInstance();

    expect(first.isValid(), "first link handle should be valid");
    expect(second.isValid(), "second link handle should be valid");

    registry.publish(first, makeState(1, 0.2f));
    registry.publish(second, makeState(1, 0.8f));

    stagemind::LinkReadQuery query;
    query.group = 1;
    const auto peer = registry.readPeer(first, query);

    expect(peer.found, "link should find a peer in the same group");
    expect(peer.instanceId == second.instanceId, "link should return the strongest active peer");
    expect(registry.countActivePeers(first, 1) == 1, "link peer count should exclude self");

    registry.unregisterInstance(first);
    registry.unregisterInstance(second);
}

void registryIsolatesGroups()
{
    auto& registry = stagemind::StageMindLinkRegistry::instance();
    registry.resetForTests();

    const auto first = registry.registerInstance();
    const auto second = registry.registerInstance();

    registry.publish(first, makeState(1, 0.8f));
    registry.publish(second, makeState(2, 0.9f));

    stagemind::LinkReadQuery query;
    query.group = 1;
    const auto peer = registry.readPeer(first, query);

    expect(! peer.found, "link should not read peers from another group");

    registry.unregisterInstance(first);
    registry.unregisterInstance(second);
}

void preferredSourceWinsOverActivity()
{
    auto& registry = stagemind::StageMindLinkRegistry::instance();
    registry.resetForTests();

    const auto reader = registry.registerInstance();
    const auto quietSource = registry.registerInstance();
    const auto loudSource = registry.registerInstance();

    registry.publish(reader, makeState(1, 0.1f));
    registry.publish(quietSource, makeState(1, 0.2f));
    registry.publish(loudSource, makeState(1, 0.9f));

    stagemind::LinkReadQuery query;
    query.group = 1;
    query.preferredSourceId = quietSource.instanceId;
    const auto peer = registry.readPeer(reader, query);

    expect(peer.found, "preferred source should be readable");
    expect(peer.instanceId == quietSource.instanceId, "preferred source should win over activity fallback");

    registry.unregisterInstance(reader);
    registry.unregisterInstance(quietSource);
    registry.unregisterInstance(loudSource);
}

void preferredSourceRoleWinsOverActivityFallback()
{
    auto& registry = stagemind::StageMindLinkRegistry::instance();
    registry.resetForTests();

    const auto reader = registry.registerInstance();
    const auto preferredRoleSource = registry.registerInstance();
    const auto louderOtherRoleSource = registry.registerInstance();

    registry.publish(reader, makeState(1, 0.1f, 4));
    registry.publish(preferredRoleSource, makeState(1, 0.3f, 2));
    registry.publish(louderOtherRoleSource, makeState(1, 0.9f, 5));

    stagemind::LinkReadQuery query;
    query.group = 1;
    query.preferredSourceRole = 2;
    const auto peer = registry.readPeer(reader, query);

    expect(peer.found, "preferred source role should be readable");
    expect(peer.instanceId == preferredRoleSource.instanceId, "preferred source role should win over activity fallback");
    expect(peer.role == 2, "preferred source role should be preserved in the peer snapshot");

    registry.unregisterInstance(reader);
    registry.unregisterInstance(preferredRoleSource);
    registry.unregisterInstance(louderOtherRoleSource);
}

void missingPreferredSourceRoleDoesNotFallbackToOtherRole()
{
    auto& registry = stagemind::StageMindLinkRegistry::instance();
    registry.resetForTests();

    const auto reader = registry.registerInstance();
    const auto otherRoleSource = registry.registerInstance();

    registry.publish(reader, makeState(1, 0.1f, 4));
    registry.publish(otherRoleSource, makeState(1, 0.9f, 5));

    stagemind::LinkReadQuery query;
    query.group = 1;
    query.preferredSourceRole = 2;
    const auto peer = registry.readPeer(reader, query);

    expect(! peer.found, "missing preferred source role should not fall back to another role");

    registry.unregisterInstance(reader);
    registry.unregisterInstance(otherRoleSource);
}

void targetRoutingFiltersFallbackPeers()
{
    auto& registry = stagemind::StageMindLinkRegistry::instance();
    registry.resetForTests();

    const auto target = registry.registerInstance();
    const auto otherTarget = registry.registerInstance();
    const auto sourceForTarget = registry.registerInstance();
    const auto sourceForOtherTarget = registry.registerInstance();

    auto firstSourceState = makeState(1, 0.4f);
    firstSourceState.targetId = static_cast<int> (target.instanceId);

    auto otherSourceState = makeState(1, 0.9f);
    otherSourceState.targetId = static_cast<int> (otherTarget.instanceId);

    registry.publish(target, makeState(1, 0.1f));
    registry.publish(otherTarget, makeState(1, 0.1f));
    registry.publish(sourceForTarget, firstSourceState);
    registry.publish(sourceForOtherTarget, otherSourceState);

    stagemind::LinkReadQuery query;
    query.group = 1;
    const auto peer = registry.readPeer(target, query);

    expect(peer.found, "targeted source should be readable");
    expect(peer.instanceId == sourceForTarget.instanceId, "fallback should ignore sources targeting another instance");

    registry.unregisterInstance(target);
    registry.unregisterInstance(otherTarget);
    registry.unregisterInstance(sourceForTarget);
    registry.unregisterInstance(sourceForOtherTarget);
}

void nonRealtimePublishIsIgnored()
{
    auto& registry = stagemind::StageMindLinkRegistry::instance();
    registry.resetForTests();

    const auto reader = registry.registerInstance();
    const auto source = registry.registerInstance();

    registry.publish(reader, makeState(1, 0.1f));
    auto sourceState = makeState(1, 0.9f);
    sourceState.nonRealtime = true;
    registry.publish(source, sourceState);

    stagemind::LinkReadQuery query;
    query.group = 1;
    const auto peer = registry.readPeer(reader, query);

    expect(! peer.found, "non-realtime publish should be disabled for deterministic offline render");

    registry.unregisterInstance(reader);
    registry.unregisterInstance(source);
}

void registryPublishesClampedSpectralBands()
{
    auto& registry = stagemind::StageMindLinkRegistry::instance();
    registry.resetForTests();

    const auto reader = registry.registerInstance();
    const auto source = registry.registerInstance();

    registry.publish(reader, makeState(1, 0.1f));
    auto sourceState = makeState(1, 0.8f);
    sourceState.bands.low = 1.5f;
    sourceState.bands.lowMid = 0.25f;
    sourceState.bands.presence = 0.5f;
    sourceState.bands.air = -0.2f;
    registry.publish(source, sourceState);

    stagemind::LinkReadQuery query;
    query.group = 1;
    const auto peer = registry.readPeer(reader, query);

    expect(peer.found, "spectral band peer should be readable");
    expect(std::abs(peer.bands.low - 1.0f) < 1.0e-6f, "low band should be clamped to one");
    expect(std::abs(peer.bands.lowMid - 0.25f) < 1.0e-6f, "low-mid band should be preserved");
    expect(std::abs(peer.bands.presence - 0.5f) < 1.0e-6f, "presence band should be preserved");
    expect(peer.bands.air == 0.0f, "air band should be clamped to zero");

    registry.unregisterInstance(reader);
    registry.unregisterInstance(source);
}

void registryReadsWholeGroupForDirector()
{
    auto& registry = stagemind::StageMindLinkRegistry::instance();
    registry.resetForTests();

    const auto first = registry.registerInstance();
    const auto second = registry.registerInstance();
    const auto otherGroup = registry.registerInstance();

    registry.publish(first, makeState(1, 0.4f, 2));
    auto secondState = makeState(1, 0.7f, 4);
    secondState.sidechainMode = 2;
    secondState.triggerMode = 3;
    secondState.autoAssistMode = 2;
    secondState.sidechainEnabled = true;
    secondState.sidechainAmount = 0.45f;
    registry.publish(second, secondState);
    registry.publish(otherGroup, makeState(2, 0.9f, 5));

    const auto group = registry.readGroup(1);

    expect(group.count == 2, "director group snapshot should include all active nodes in the requested group");
    expect(group.peers[0].group == 1 && group.peers[1].group == 1, "director group snapshot should preserve group ids");
    expect(group.peers[1].sidechainMode == 2, "director snapshot should preserve target sidechain mode");
    expect(group.peers[1].triggerMode == 3, "director snapshot should preserve target trigger mode");
    expect(group.peers[1].autoAssistMode == 2, "director snapshot should preserve target auto mode");
    expect(group.peers[1].sidechainEnabled, "director snapshot should preserve target SC enable state");
    expect(std::abs(group.peers[1].sidechainAmount - 0.45f) < 1.0e-6f, "director snapshot should preserve target SC amount");
    expect(std::abs(group.peers[1].pan - 0.15f) < 1.0e-6f, "director snapshot should preserve target pan");
    expect(std::abs(group.peers[1].motion - 0.25f) < 1.0e-6f, "director snapshot should preserve target motion");

    registry.unregisterInstance(first);
    registry.unregisterInstance(second);
    registry.unregisterInstance(otherGroup);
}

void registrySubmitsAndConsumesDirectorCommand()
{
    auto& registry = stagemind::StageMindLinkRegistry::instance();
    registry.resetForTests();

    const auto director = registry.registerInstance();
    const auto target = registry.registerInstance();
    const auto peer = registry.registerInstance();

    registry.publish(target, makeState(1, 0.8f, 4));
    registry.publish(peer, makeState(1, 0.7f, 11));

    stagemind::LinkCommand command;
    command.automatic = true;
    command.sourceInstanceId = director.instanceId;
    command.peerInstanceId = peer.instanceId;
    command.actionKind = 3;

    expect(registry.submitCommand(target.instanceId, command), "director command should submit to an active target node");

    const auto consumed = registry.consumeCommand(target);
    expect(consumed.found, "target node should consume the pending director command");
    expect(consumed.automatic, "director command should preserve automatic flag");
    expect(consumed.sourceInstanceId == director.instanceId, "director command should preserve source id");
    expect(consumed.peerInstanceId == peer.instanceId, "director command should preserve peer id");
    expect(consumed.actionKind == command.actionKind, "director command should preserve action kind");
    expect(! registry.consumeCommand(target).found, "director command should be consumed only once");

    registry.unregisterInstance(director);
    registry.unregisterInstance(target);
    registry.unregisterInstance(peer);
}

void registrySubmitsAndConsumesDirectorSpatialCommand()
{
    auto& registry = stagemind::StageMindLinkRegistry::instance();
    registry.resetForTests();

    const auto director = registry.registerInstance();
    const auto target = registry.registerInstance();

    registry.publish(target, makeState(1, 0.8f, 4));

    stagemind::LinkCommand command;
    command.sourceInstanceId = director.instanceId;
    command.setPan = true;
    command.setWidth = true;
    command.setDepth = true;
    command.setMotion = true;
    command.setOutputTrim = true;
    command.setCleanUp = true;
    command.setResonance = true;
    command.setSidechainAmount = true;
    command.setStageGainMode = true;
    command.requestStageGainAnalyze = true;
    command.pan = 2.0f;
    command.width = 1.5f;
    command.depth = -1.0f;
    command.motion = 0.25f;
    command.outputTrimDb = 14.0f;
    command.cleanUp = 0.35f;
    command.resonance = 0.45f;
    command.sidechainAmount = 0.55f;
    command.stageGainMode = 5;

    expect(registry.submitCommand(target.instanceId, command), "director spatial command should submit without a suggestion action");

    const auto consumed = registry.consumeCommand(target);
    expect(consumed.found, "target node should consume the pending director spatial command");
    expect(consumed.sourceInstanceId == director.instanceId, "spatial command should preserve source id");
    expect(consumed.actionKind == 0, "spatial command should not masquerade as a suggestion action");
    expect(consumed.setPan, "spatial command should request pan write");
    expect(consumed.setWidth, "spatial command should request width write");
    expect(consumed.setDepth, "spatial command should request depth write");
    expect(consumed.setMotion, "spatial command should request motion write");
    expect(consumed.setOutputTrim, "spatial command should request output trim write");
    expect(consumed.setCleanUp, "spatial command should request clean up write");
    expect(consumed.setResonance, "spatial command should request resonance write");
    expect(consumed.setSidechainAmount, "spatial command should request sidechain amount write");
    expect(consumed.setStageGainMode, "spatial command should request stage gain mode write");
    expect(consumed.requestStageGainAnalyze, "spatial command should request stage gain analyze");
    expect(std::abs(consumed.pan - 1.0f) < 1.0e-6f, "spatial command pan should be clamped");
    expect(std::abs(consumed.width - 1.0f) < 1.0e-6f, "spatial command width should be clamped");
    expect(std::abs(consumed.depth - 0.0f) < 1.0e-6f, "spatial command depth should be clamped");
    expect(std::abs(consumed.motion - 0.25f) < 1.0e-6f, "spatial command motion should be preserved");
    expect(std::abs(consumed.outputTrimDb - 12.0f) < 1.0e-6f, "spatial command output trim should be clamped");
    expect(std::abs(consumed.cleanUp - 0.35f) < 1.0e-6f, "spatial command clean up should be preserved");
    expect(std::abs(consumed.resonance - 0.45f) < 1.0e-6f, "spatial command resonance should be preserved");
    expect(std::abs(consumed.sidechainAmount - 0.55f) < 1.0e-6f, "spatial command SC amount should be preserved");
    expect(consumed.stageGainMode == 2, "spatial command stage gain mode should be clamped");
    expect(! registry.consumeCommand(target).found, "spatial command should be consumed only once");

    registry.unregisterInstance(director);
    registry.unregisterInstance(target);
}
} // namespace

void runStageMindLinkRegistryTests()
{
    registryPublishesAndReadsPeerInSameGroup();
    registryIsolatesGroups();
    preferredSourceWinsOverActivity();
    preferredSourceRoleWinsOverActivityFallback();
    missingPreferredSourceRoleDoesNotFallbackToOtherRole();
    targetRoutingFiltersFallbackPeers();
    nonRealtimePublishIsIgnored();
    registryPublishesClampedSpectralBands();
    registryReadsWholeGroupForDirector();
    registrySubmitsAndConsumesDirectorCommand();
    registrySubmitsAndConsumesDirectorSpatialCommand();
}
