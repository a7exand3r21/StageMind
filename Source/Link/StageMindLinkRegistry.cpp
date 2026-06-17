#include "StageMindLinkRegistry.h"

#include <algorithm>

namespace stagemind
{
StageMindLinkRegistry& StageMindLinkRegistry::instance() noexcept
{
    static StageMindLinkRegistry registry;
    return registry;
}

LinkInstanceHandle StageMindLinkRegistry::registerInstance() noexcept
{
    for (int index = 0; index < maxLinkInstances; ++index)
    {
        auto& slot = slots[static_cast<size_t> (index)];
        auto expected = invalidLinkInstanceId;
        const auto instanceId = static_cast<std::uint32_t> (index + 1);

        if (slot.instanceId.compare_exchange_strong(expected, instanceId, std::memory_order_acq_rel))
        {
            clearSlotState(slot);
            return { instanceId, index };
        }
    }

    return {};
}

void StageMindLinkRegistry::unregisterInstance(LinkInstanceHandle handle) noexcept
{
    if (! handle.isValid())
        return;

    auto& slot = slots[static_cast<size_t> (handle.slotIndex)];
    if (slot.instanceId.load(std::memory_order_acquire) != handle.instanceId)
        return;

    slot.enabled.store(false, std::memory_order_release);
    clearSlotState(slot);
    slot.instanceId.store(invalidLinkInstanceId, std::memory_order_release);
}

void StageMindLinkRegistry::publish(LinkInstanceHandle handle, const LinkPublishState& state) noexcept
{
    if (! handle.isValid())
        return;

    auto& slot = slots[static_cast<size_t> (handle.slotIndex)];
    if (slot.instanceId.load(std::memory_order_acquire) != handle.instanceId)
        return;

    const auto shouldPublish = state.enabled && ! state.nonRealtime && state.group > 0;
    if (! shouldPublish)
    {
        slot.enabled.store(false, std::memory_order_release);
        return;
    }

    slot.group.store(state.group, std::memory_order_relaxed);
    slot.role.store(state.role, std::memory_order_relaxed);
    slot.sourceId.store(state.sourceId, std::memory_order_relaxed);
    slot.targetId.store(state.targetId, std::memory_order_relaxed);
    slot.mode.store(state.mode, std::memory_order_relaxed);
    slot.sidechainMode.store(state.sidechainMode, std::memory_order_relaxed);
    slot.triggerMode.store(state.triggerMode, std::memory_order_relaxed);
    slot.autoAssistMode.store(state.autoAssistMode, std::memory_order_relaxed);
    slot.sidechainEnabled.store(state.sidechainEnabled, std::memory_order_relaxed);
    slot.activity.store(std::clamp(state.activity, 0.0f, 1.0f), std::memory_order_relaxed);
    slot.inputRms.store(std::max(0.0f, state.inputRms), std::memory_order_relaxed);
    slot.outputRms.store(std::max(0.0f, state.outputRms), std::memory_order_relaxed);
    slot.sidechainEnvelope.store(std::max(0.0f, state.sidechainEnvelope), std::memory_order_relaxed);
    slot.sidechainAmount.store(std::clamp(state.sidechainAmount, 0.0f, 1.0f), std::memory_order_relaxed);
    slot.correlation.store(std::clamp(state.correlation, -1.0f, 1.0f), std::memory_order_relaxed);
    slot.pan.store(std::clamp(state.pan, -1.0f, 1.0f), std::memory_order_relaxed);
    slot.width.store(std::clamp(state.width, 0.0f, 1.0f), std::memory_order_relaxed);
    slot.depth.store(std::clamp(state.depth, 0.0f, 1.0f), std::memory_order_relaxed);
    slot.motion.store(std::clamp(state.motion, 0.0f, 1.0f), std::memory_order_relaxed);
    slot.cleanUp.store(std::clamp(state.cleanUp, 0.0f, 1.0f), std::memory_order_relaxed);
    slot.resonance.store(std::clamp(state.resonance, 0.0f, 1.0f), std::memory_order_relaxed);
    const auto bands = clampedBands(state.bands);
    slot.bandLow.store(bands.low, std::memory_order_relaxed);
    slot.bandLowMid.store(bands.lowMid, std::memory_order_relaxed);
    slot.bandPresence.store(bands.presence, std::memory_order_relaxed);
    slot.bandAir.store(bands.air, std::memory_order_relaxed);
    slot.enabled.store(true, std::memory_order_release);
}

LinkPeerSnapshot StageMindLinkRegistry::readPeer(LinkInstanceHandle handle, LinkReadQuery query) const noexcept
{
    if (! handle.isValid() || query.group <= 0)
        return {};

    if (query.preferredSourceId != invalidLinkInstanceId)
    {
        for (const auto& slot : slots)
        {
            const auto instanceId = slot.instanceId.load(std::memory_order_acquire);
            if (instanceId != query.preferredSourceId || instanceId == handle.instanceId)
                continue;

            if (! slot.enabled.load(std::memory_order_acquire))
                continue;

            if (slot.group.load(std::memory_order_relaxed) == query.group && targetMatches(slot, handle.instanceId))
                return makeSnapshot(slot, instanceId);
        }
    }

    if (query.preferredSourceRole != 0)
    {
        LinkPeerSnapshot bestRoleMatch;
        auto bestRoleActivity = -1.0f;

        for (const auto& slot : slots)
        {
            const auto instanceId = slot.instanceId.load(std::memory_order_acquire);
            if (instanceId == invalidLinkInstanceId || instanceId == handle.instanceId)
                continue;

            if (! slot.enabled.load(std::memory_order_acquire))
                continue;

            if (slot.group.load(std::memory_order_relaxed) != query.group)
                continue;

            if (! targetMatches(slot, handle.instanceId))
                continue;

            if (slot.role.load(std::memory_order_relaxed) != query.preferredSourceRole)
                continue;

            const auto activity = slot.activity.load(std::memory_order_relaxed);
            if (activity > bestRoleActivity)
            {
                bestRoleActivity = activity;
                bestRoleMatch = makeSnapshot(slot, instanceId);
            }
        }

        return bestRoleMatch;
    }

    LinkPeerSnapshot best;
    auto bestActivity = -1.0f;

    for (const auto& slot : slots)
    {
        const auto instanceId = slot.instanceId.load(std::memory_order_acquire);
        if (instanceId == invalidLinkInstanceId || instanceId == handle.instanceId)
            continue;

        if (! slot.enabled.load(std::memory_order_acquire))
            continue;

        if (slot.group.load(std::memory_order_relaxed) != query.group)
            continue;

        if (! targetMatches(slot, handle.instanceId))
            continue;

        const auto activity = slot.activity.load(std::memory_order_relaxed);
        if (activity > bestActivity)
        {
            bestActivity = activity;
            best = makeSnapshot(slot, instanceId);
        }
    }

    return best;
}

LinkGroupSnapshot StageMindLinkRegistry::readGroup(int group) const noexcept
{
    LinkGroupSnapshot snapshot;
    if (group <= 0)
        return snapshot;

    for (const auto& slot : slots)
    {
        if (snapshot.count >= maxLinkInstances)
            break;

        const auto instanceId = slot.instanceId.load(std::memory_order_acquire);
        if (instanceId == invalidLinkInstanceId)
            continue;

        if (! slot.enabled.load(std::memory_order_acquire))
            continue;

        if (slot.group.load(std::memory_order_relaxed) != group)
            continue;

        snapshot.peers[static_cast<size_t> (snapshot.count)] = makeSnapshot(slot, instanceId);
        ++snapshot.count;
    }

    return snapshot;
}

bool StageMindLinkRegistry::submitCommand(std::uint32_t targetInstanceId, LinkCommand command) noexcept
{
    const auto hasDirectParameterCommand = command.setPan
        || command.setWidth
        || command.setDepth
        || command.setMotion
        || command.setCleanUp
        || command.setResonance
        || command.setSidechainAmount;
    if (targetInstanceId == invalidLinkInstanceId || (command.actionKind == 0 && ! hasDirectParameterCommand))
        return false;

    for (auto& slot : slots)
    {
        const auto instanceId = slot.instanceId.load(std::memory_order_acquire);
        if (instanceId != targetInstanceId)
            continue;

        if (! slot.enabled.load(std::memory_order_acquire))
            return false;

        auto sequence = nextCommandSequence.fetch_add(1, std::memory_order_relaxed);
        if (sequence == 0)
            sequence = nextCommandSequence.fetch_add(1, std::memory_order_relaxed);

        slot.commandSourceId.store(command.sourceInstanceId, std::memory_order_relaxed);
        slot.commandPeerId.store(command.peerInstanceId, std::memory_order_relaxed);
        slot.commandActionKind.store(command.actionKind, std::memory_order_relaxed);
        slot.commandAutomatic.store(command.automatic ? 1 : 0, std::memory_order_relaxed);
        slot.commandSetPan.store(command.setPan ? 1 : 0, std::memory_order_relaxed);
        slot.commandSetWidth.store(command.setWidth ? 1 : 0, std::memory_order_relaxed);
        slot.commandSetDepth.store(command.setDepth ? 1 : 0, std::memory_order_relaxed);
        slot.commandSetMotion.store(command.setMotion ? 1 : 0, std::memory_order_relaxed);
        slot.commandSetCleanUp.store(command.setCleanUp ? 1 : 0, std::memory_order_relaxed);
        slot.commandSetResonance.store(command.setResonance ? 1 : 0, std::memory_order_relaxed);
        slot.commandSetSidechainAmount.store(command.setSidechainAmount ? 1 : 0, std::memory_order_relaxed);
        slot.commandPan.store(std::clamp(command.pan, -1.0f, 1.0f), std::memory_order_relaxed);
        slot.commandWidth.store(std::clamp(command.width, 0.0f, 1.0f), std::memory_order_relaxed);
        slot.commandDepth.store(std::clamp(command.depth, 0.0f, 1.0f), std::memory_order_relaxed);
        slot.commandMotion.store(std::clamp(command.motion, 0.0f, 1.0f), std::memory_order_relaxed);
        slot.commandCleanUp.store(std::clamp(command.cleanUp, 0.0f, 1.0f), std::memory_order_relaxed);
        slot.commandResonance.store(std::clamp(command.resonance, 0.0f, 1.0f), std::memory_order_relaxed);
        slot.commandSidechainAmount.store(std::clamp(command.sidechainAmount, 0.0f, 1.0f), std::memory_order_relaxed);
        slot.commandSequence.store(sequence, std::memory_order_release);
        return true;
    }

    return false;
}

LinkCommand StageMindLinkRegistry::consumeCommand(LinkInstanceHandle handle) noexcept
{
    if (! handle.isValid())
        return {};

    auto& slot = slots[static_cast<size_t> (handle.slotIndex)];
    if (slot.instanceId.load(std::memory_order_acquire) != handle.instanceId)
        return {};

    const auto sequence = slot.commandSequence.exchange(0, std::memory_order_acq_rel);
    if (sequence == 0)
        return {};

    LinkCommand command;
    command.found = true;
    command.automatic = slot.commandAutomatic.load(std::memory_order_relaxed) != 0;
    command.sequence = sequence;
    command.sourceInstanceId = slot.commandSourceId.load(std::memory_order_relaxed);
    command.peerInstanceId = slot.commandPeerId.load(std::memory_order_relaxed);
    command.actionKind = slot.commandActionKind.load(std::memory_order_relaxed);
    command.setPan = slot.commandSetPan.load(std::memory_order_relaxed) != 0;
    command.setWidth = slot.commandSetWidth.load(std::memory_order_relaxed) != 0;
    command.setDepth = slot.commandSetDepth.load(std::memory_order_relaxed) != 0;
    command.setMotion = slot.commandSetMotion.load(std::memory_order_relaxed) != 0;
    command.setCleanUp = slot.commandSetCleanUp.load(std::memory_order_relaxed) != 0;
    command.setResonance = slot.commandSetResonance.load(std::memory_order_relaxed) != 0;
    command.setSidechainAmount = slot.commandSetSidechainAmount.load(std::memory_order_relaxed) != 0;
    command.pan = slot.commandPan.load(std::memory_order_relaxed);
    command.width = slot.commandWidth.load(std::memory_order_relaxed);
    command.depth = slot.commandDepth.load(std::memory_order_relaxed);
    command.motion = slot.commandMotion.load(std::memory_order_relaxed);
    command.cleanUp = slot.commandCleanUp.load(std::memory_order_relaxed);
    command.resonance = slot.commandResonance.load(std::memory_order_relaxed);
    command.sidechainAmount = slot.commandSidechainAmount.load(std::memory_order_relaxed);
    return command;
}

int StageMindLinkRegistry::countActivePeers(LinkInstanceHandle handle, int group) const noexcept
{
    if (! handle.isValid() || group <= 0)
        return 0;

    auto count = 0;
    for (const auto& slot : slots)
    {
        const auto instanceId = slot.instanceId.load(std::memory_order_acquire);
        if (instanceId == invalidLinkInstanceId || instanceId == handle.instanceId)
            continue;

        if (! slot.enabled.load(std::memory_order_acquire))
            continue;

        if (slot.group.load(std::memory_order_relaxed) == group)
            ++count;
    }

    return count;
}

void StageMindLinkRegistry::resetForTests() noexcept
{
    nextCommandSequence.store(1, std::memory_order_release);

    for (auto& slot : slots)
    {
        slot.enabled.store(false, std::memory_order_release);
        clearSlotState(slot);
        slot.instanceId.store(invalidLinkInstanceId, std::memory_order_release);
    }
}

void StageMindLinkRegistry::clearSlotState(Slot& slot) noexcept
{
    slot.enabled.store(false, std::memory_order_release);
    slot.group.store(0, std::memory_order_relaxed);
    slot.role.store(0, std::memory_order_relaxed);
    slot.sourceId.store(0, std::memory_order_relaxed);
    slot.targetId.store(0, std::memory_order_relaxed);
    slot.mode.store(0, std::memory_order_relaxed);
    slot.sidechainMode.store(0, std::memory_order_relaxed);
    slot.triggerMode.store(0, std::memory_order_relaxed);
    slot.autoAssistMode.store(0, std::memory_order_relaxed);
    slot.sidechainEnabled.store(false, std::memory_order_relaxed);
    slot.activity.store(0.0f, std::memory_order_relaxed);
    slot.inputRms.store(0.0f, std::memory_order_relaxed);
    slot.outputRms.store(0.0f, std::memory_order_relaxed);
    slot.sidechainEnvelope.store(0.0f, std::memory_order_relaxed);
    slot.sidechainAmount.store(0.0f, std::memory_order_relaxed);
    slot.correlation.store(1.0f, std::memory_order_relaxed);
    slot.pan.store(0.0f, std::memory_order_relaxed);
    slot.width.store(0.0f, std::memory_order_relaxed);
    slot.depth.store(0.0f, std::memory_order_relaxed);
    slot.motion.store(0.0f, std::memory_order_relaxed);
    slot.cleanUp.store(0.0f, std::memory_order_relaxed);
    slot.resonance.store(0.0f, std::memory_order_relaxed);
    slot.bandLow.store(0.0f, std::memory_order_relaxed);
    slot.bandLowMid.store(0.0f, std::memory_order_relaxed);
    slot.bandPresence.store(0.0f, std::memory_order_relaxed);
    slot.bandAir.store(0.0f, std::memory_order_relaxed);
    slot.commandSequence.store(0, std::memory_order_release);
    slot.commandSourceId.store(invalidLinkInstanceId, std::memory_order_relaxed);
    slot.commandPeerId.store(invalidLinkInstanceId, std::memory_order_relaxed);
    slot.commandActionKind.store(0, std::memory_order_relaxed);
    slot.commandAutomatic.store(0, std::memory_order_relaxed);
    slot.commandSetPan.store(0, std::memory_order_relaxed);
    slot.commandSetWidth.store(0, std::memory_order_relaxed);
    slot.commandSetDepth.store(0, std::memory_order_relaxed);
    slot.commandSetMotion.store(0, std::memory_order_relaxed);
    slot.commandSetCleanUp.store(0, std::memory_order_relaxed);
    slot.commandSetResonance.store(0, std::memory_order_relaxed);
    slot.commandSetSidechainAmount.store(0, std::memory_order_relaxed);
    slot.commandPan.store(0.0f, std::memory_order_relaxed);
    slot.commandWidth.store(0.0f, std::memory_order_relaxed);
    slot.commandDepth.store(0.0f, std::memory_order_relaxed);
    slot.commandMotion.store(0.0f, std::memory_order_relaxed);
    slot.commandCleanUp.store(0.0f, std::memory_order_relaxed);
    slot.commandResonance.store(0.0f, std::memory_order_relaxed);
    slot.commandSidechainAmount.store(0.0f, std::memory_order_relaxed);
}

LinkPeerSnapshot StageMindLinkRegistry::makeSnapshot(const Slot& slot, std::uint32_t instanceId) noexcept
{
    LinkPeerSnapshot snapshot;
    snapshot.found = true;
    snapshot.instanceId = instanceId;
    snapshot.group = slot.group.load(std::memory_order_relaxed);
    snapshot.role = slot.role.load(std::memory_order_relaxed);
    snapshot.targetId = slot.targetId.load(std::memory_order_relaxed);
    snapshot.mode = slot.mode.load(std::memory_order_relaxed);
    snapshot.sidechainMode = slot.sidechainMode.load(std::memory_order_relaxed);
    snapshot.triggerMode = slot.triggerMode.load(std::memory_order_relaxed);
    snapshot.autoAssistMode = slot.autoAssistMode.load(std::memory_order_relaxed);
    snapshot.sidechainEnabled = slot.sidechainEnabled.load(std::memory_order_relaxed);
    snapshot.activity = slot.activity.load(std::memory_order_relaxed);
    snapshot.inputRms = slot.inputRms.load(std::memory_order_relaxed);
    snapshot.outputRms = slot.outputRms.load(std::memory_order_relaxed);
    snapshot.sidechainEnvelope = slot.sidechainEnvelope.load(std::memory_order_relaxed);
    snapshot.sidechainAmount = slot.sidechainAmount.load(std::memory_order_relaxed);
    snapshot.correlation = slot.correlation.load(std::memory_order_relaxed);
    snapshot.pan = slot.pan.load(std::memory_order_relaxed);
    snapshot.width = slot.width.load(std::memory_order_relaxed);
    snapshot.depth = slot.depth.load(std::memory_order_relaxed);
    snapshot.motion = slot.motion.load(std::memory_order_relaxed);
    snapshot.cleanUp = slot.cleanUp.load(std::memory_order_relaxed);
    snapshot.resonance = slot.resonance.load(std::memory_order_relaxed);
    snapshot.bands.low = slot.bandLow.load(std::memory_order_relaxed);
    snapshot.bands.lowMid = slot.bandLowMid.load(std::memory_order_relaxed);
    snapshot.bands.presence = slot.bandPresence.load(std::memory_order_relaxed);
    snapshot.bands.air = slot.bandAir.load(std::memory_order_relaxed);
    return snapshot;
}

bool StageMindLinkRegistry::targetMatches(const Slot& slot, std::uint32_t readerId) noexcept
{
    const auto targetId = slot.targetId.load(std::memory_order_relaxed);
    return targetId == 0 || static_cast<std::uint32_t> (targetId) == readerId;
}
} // namespace stagemind
