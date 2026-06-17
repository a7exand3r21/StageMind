#pragma once

#include "LinkSpectralBands.h"
#include <array>
#include <atomic>
#include <cstdint>

namespace stagemind
{
inline constexpr int maxLinkInstances = 128;
inline constexpr std::uint32_t invalidLinkInstanceId = 0;

struct LinkInstanceHandle
{
    std::uint32_t instanceId = invalidLinkInstanceId;
    int slotIndex = -1;

    bool isValid() const noexcept
    {
        return instanceId != invalidLinkInstanceId && slotIndex >= 0 && slotIndex < maxLinkInstances;
    }
};

struct LinkPublishState
{
    bool enabled = false;
    bool nonRealtime = false;
    int group = 0;
    int role = 0;
    int sourceId = 0;
    int targetId = 0;
    int mode = 0;
    int sidechainMode = 0;
    int triggerMode = 0;
    int autoAssistMode = 0;
    bool sidechainEnabled = false;
    float activity = 0.0f;
    float inputRms = 0.0f;
    float outputRms = 0.0f;
    float sidechainEnvelope = 0.0f;
    float sidechainAmount = 0.0f;
    float correlation = 1.0f;
    float pan = 0.0f;
    float width = 0.0f;
    float depth = 0.0f;
    float motion = 0.0f;
    float cleanUp = 0.0f;
    float resonance = 0.0f;
    LinkSpectralBands bands;
};

struct LinkReadQuery
{
    int group = 0;
    std::uint32_t preferredSourceId = invalidLinkInstanceId;
    int preferredSourceRole = 0;
};

struct LinkPeerSnapshot
{
    bool found = false;
    std::uint32_t instanceId = invalidLinkInstanceId;
    int group = 0;
    int role = 0;
    int targetId = 0;
    int mode = 0;
    int sidechainMode = 0;
    int triggerMode = 0;
    int autoAssistMode = 0;
    bool sidechainEnabled = false;
    float activity = 0.0f;
    float inputRms = 0.0f;
    float outputRms = 0.0f;
    float sidechainEnvelope = 0.0f;
    float sidechainAmount = 0.0f;
    float correlation = 1.0f;
    float pan = 0.0f;
    float width = 0.0f;
    float depth = 0.0f;
    float motion = 0.0f;
    float cleanUp = 0.0f;
    float resonance = 0.0f;
    LinkSpectralBands bands;
};

struct LinkGroupSnapshot
{
    int count = 0;
    std::array<LinkPeerSnapshot, maxLinkInstances> peers;
};

struct LinkCommand
{
    bool found = false;
    bool automatic = false;
    std::uint32_t sequence = 0;
    std::uint32_t sourceInstanceId = invalidLinkInstanceId;
    std::uint32_t peerInstanceId = invalidLinkInstanceId;
    int actionKind = 0;
    bool setPan = false;
    bool setWidth = false;
    bool setDepth = false;
    bool setMotion = false;
    bool setCleanUp = false;
    bool setResonance = false;
    bool setSidechainAmount = false;
    float pan = 0.0f;
    float width = 0.0f;
    float depth = 0.0f;
    float motion = 0.0f;
    float cleanUp = 0.0f;
    float resonance = 0.0f;
    float sidechainAmount = 0.0f;
};

class StageMindLinkRegistry final
{
public:
    static StageMindLinkRegistry& instance() noexcept;

    LinkInstanceHandle registerInstance() noexcept;
    void unregisterInstance(LinkInstanceHandle handle) noexcept;
    void publish(LinkInstanceHandle handle, const LinkPublishState& state) noexcept;
    LinkPeerSnapshot readPeer(LinkInstanceHandle handle, LinkReadQuery query) const noexcept;
    LinkGroupSnapshot readGroup(int group) const noexcept;
    bool submitCommand(std::uint32_t targetInstanceId, LinkCommand command) noexcept;
    LinkCommand consumeCommand(LinkInstanceHandle handle) noexcept;
    int countActivePeers(LinkInstanceHandle handle, int group) const noexcept;
    void resetForTests() noexcept;

private:
    struct Slot
    {
        std::atomic<std::uint32_t> instanceId { invalidLinkInstanceId };
        std::atomic<bool> enabled { false };
        std::atomic<int> group { 0 };
        std::atomic<int> role { 0 };
        std::atomic<int> sourceId { 0 };
        std::atomic<int> targetId { 0 };
        std::atomic<int> mode { 0 };
        std::atomic<int> sidechainMode { 0 };
        std::atomic<int> triggerMode { 0 };
        std::atomic<int> autoAssistMode { 0 };
        std::atomic<bool> sidechainEnabled { false };
        std::atomic<float> activity { 0.0f };
        std::atomic<float> inputRms { 0.0f };
        std::atomic<float> outputRms { 0.0f };
        std::atomic<float> sidechainEnvelope { 0.0f };
        std::atomic<float> sidechainAmount { 0.0f };
        std::atomic<float> correlation { 1.0f };
        std::atomic<float> pan { 0.0f };
        std::atomic<float> width { 0.0f };
        std::atomic<float> depth { 0.0f };
        std::atomic<float> motion { 0.0f };
        std::atomic<float> cleanUp { 0.0f };
        std::atomic<float> resonance { 0.0f };
        std::atomic<float> bandLow { 0.0f };
        std::atomic<float> bandLowMid { 0.0f };
        std::atomic<float> bandPresence { 0.0f };
        std::atomic<float> bandAir { 0.0f };
        std::atomic<std::uint32_t> commandSequence { 0 };
        std::atomic<std::uint32_t> commandSourceId { invalidLinkInstanceId };
        std::atomic<std::uint32_t> commandPeerId { invalidLinkInstanceId };
        std::atomic<int> commandActionKind { 0 };
        std::atomic<int> commandAutomatic { 0 };
        std::atomic<int> commandSetPan { 0 };
        std::atomic<int> commandSetWidth { 0 };
        std::atomic<int> commandSetDepth { 0 };
        std::atomic<int> commandSetMotion { 0 };
        std::atomic<int> commandSetCleanUp { 0 };
        std::atomic<int> commandSetResonance { 0 };
        std::atomic<int> commandSetSidechainAmount { 0 };
        std::atomic<float> commandPan { 0.0f };
        std::atomic<float> commandWidth { 0.0f };
        std::atomic<float> commandDepth { 0.0f };
        std::atomic<float> commandMotion { 0.0f };
        std::atomic<float> commandCleanUp { 0.0f };
        std::atomic<float> commandResonance { 0.0f };
        std::atomic<float> commandSidechainAmount { 0.0f };
    };

    static void clearSlotState(Slot& slot) noexcept;
    static bool targetMatches(const Slot& slot, std::uint32_t readerId) noexcept;
    static LinkPeerSnapshot makeSnapshot(const Slot& slot, std::uint32_t instanceId) noexcept;

    std::array<Slot, maxLinkInstances> slots;
    std::atomic<std::uint32_t> nextCommandSequence { 1 };
};
} // namespace stagemind
