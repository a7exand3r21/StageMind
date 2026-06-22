#pragma once

#include <JuceHeader.h>

namespace stagemind::parameters
{
namespace ids
{
inline constexpr auto role = "role";
inline constexpr auto width = "width";
inline constexpr auto depth = "depth";
inline constexpr auto motion = "motion";
inline constexpr auto cleanUp = "clean_up";
inline constexpr auto resonance = "resonance";
inline constexpr auto safety = "safety";
inline constexpr auto outputGain = "output_gain";
inline constexpr auto triggerMode = "trigger_mode";
inline constexpr auto sidechainEnabled = "sidechain_enabled";
inline constexpr auto sidechainMode = "sidechain_mode";
inline constexpr auto sidechainSourceRole = "sidechain_source_role";
inline constexpr auto sidechainAmount = "sidechain_amount";
inline constexpr auto sidechainListen = "sidechain_listen";
inline constexpr auto sidechainAttack = "sidechain_attack";
inline constexpr auto sidechainRelease = "sidechain_release";
inline constexpr auto sidechainRangeStart = "sidechain_range_start";
inline constexpr auto sidechainRangeEnd = "sidechain_range_end";
inline constexpr auto pan = "pan";
inline constexpr auto monoLowCutoff = "mono_low_cutoff";
inline constexpr auto sideHighPass = "side_highpass";
inline constexpr auto correlationSafetyThreshold = "correlation_safety_threshold";
inline constexpr auto resonanceSensitivity = "resonance_sensitivity";
inline constexpr auto maxResonanceReduction = "max_resonance_reduction";
inline constexpr auto dynamicEqAttack = "dynamic_eq_attack";
inline constexpr auto dynamicEqRelease = "dynamic_eq_release";
inline constexpr auto motionRate = "motion_rate";
inline constexpr auto pseudoDoubleAmount = "pseudo_double_amount";
inline constexpr auto presenceReduction = "presence_reduction";
inline constexpr auto earlyReflectionAmount = "early_reflection_amount";
inline constexpr auto analyzerQuality = "analyzer_quality";
inline constexpr auto processingQuality = "processing_quality";
inline constexpr auto linkEnabled = "link_enabled";
inline constexpr auto linkGroup = "link_group";
inline constexpr auto linkRole = "link_role";
inline constexpr auto linkSourceId = "link_source_id";
inline constexpr auto linkTargetId = "link_target_id";
inline constexpr auto linkMode = "link_mode";
inline constexpr auto pluginMode = "plugin_mode";
inline constexpr auto autoAssistMode = "auto_assist_mode";
inline constexpr auto motionPreset = "motion_preset";
inline constexpr auto stageGainMode = "stage_gain_mode";
inline constexpr auto stageGainMeterMode = "stage_gain_meter_mode";
inline constexpr auto stageGainTargetDb = "stage_gain_target_db";
inline constexpr auto stageGainThresholdVu = "stage_gain_threshold_vu";
inline constexpr auto stageGainCeilingDb = "stage_gain_ceiling_db";
inline constexpr auto stageGainResponse = "stage_gain_response";
} // namespace ids

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
} // namespace stagemind::parameters
