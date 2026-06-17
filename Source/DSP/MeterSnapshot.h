#pragma once

#include "ResonanceTypes.h"
#include <atomic>

namespace stagemind
{
struct MeterSnapshot
{
    std::atomic<float> inputRms { 0.0f };
    std::atomic<float> inputPeak { 0.0f };
    std::atomic<float> outputRms { 0.0f };
    std::atomic<float> outputPeak { 0.0f };
    std::atomic<float> sidechainRms { 0.0f };
    std::atomic<float> sidechainPeak { 0.0f };
    std::atomic<float> sidechainEnvelope { 0.0f };
    std::atomic<float> correlation { 1.0f };
    std::atomic<float> gainReductionDb { 0.0f };
    std::atomic<float> resonanceReductionDb { 0.0f };
    std::atomic<int> autoAssistState { 0 };
    std::atomic<float> autoAssistProgress { 0.0f };
    std::atomic<int> autoAssistActionKind { 0 };
    std::atomic<float> stagePan { 0.0f };
    std::atomic<float> stageDepth { 0.0f };
    std::atomic<float> stageWidth { 0.5f };
    std::atomic<float> stageMotion { 0.0f };
    std::atomic<int> stageMotionAllowed { 1 };
    std::atomic<int> resonanceLearnState { 0 };
    std::atomic<float> resonanceLearnProgress { 0.0f };
    std::atomic<int> linkEnabled { 0 };
    std::atomic<int> linkInstanceId { 0 };
    std::atomic<int> linkGroup { 0 };
    std::atomic<int> linkActivePeers { 0 };
    std::atomic<int> linkNodeCount { 0 };
    std::atomic<int> linkOfflineSuppressed { 0 };
    std::atomic<int> linkPeerId { 0 };
    std::atomic<int> linkPeerRole { 0 };
    std::atomic<float> linkPeerActivity { 0.0f };
    std::atomic<float> linkPeerCorrelation { 1.0f };
    std::atomic<float> linkPeerWidth { 0.0f };
    std::atomic<float> linkPeerDepth { 0.0f };
    std::atomic<float> linkPeerCleanUp { 0.0f };
    std::atomic<float> linkPeerResonance { 0.0f };
    std::atomic<float> linkBandLow { 0.0f };
    std::atomic<float> linkBandLowMid { 0.0f };
    std::atomic<float> linkBandPresence { 0.0f };
    std::atomic<float> linkBandAir { 0.0f };
    std::atomic<float> linkPeerBandLow { 0.0f };
    std::atomic<float> linkPeerBandLowMid { 0.0f };
    std::atomic<float> linkPeerBandPresence { 0.0f };
    std::atomic<float> linkPeerBandAir { 0.0f };
    AtomicResonanceSnapshot resonances;
    AtomicResonanceSnapshot learnedResonances;
};
} // namespace stagemind
