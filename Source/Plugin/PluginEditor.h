#pragma once

#include "../Link/LinkSuggestionEngine.h"
#include "../UI/DirectorSceneView.h"
#include "../UI/HardwareLookAndFeel.h"
#include "../UI/MeterView.h"
#include "../UI/ResonanceListView.h"
#include "../UI/StageView.h"
#include "PluginProcessor.h"
#include <array>

namespace stagemind
{
inline constexpr int maxDirectorConflictRecords = 24;

class PluginEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PluginEditor(PluginProcessor& processorToUse);
    ~PluginEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    struct DirectorConflictRecord;

    void timerCallback() override;
    void setupCombo(juce::ComboBox& combo, const juce::StringArray& items);
    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText);
    void setupDirectorRemoteSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText, double minimum, double maximum);
    void setupButton(juce::TextButton& button);
    void setupLinkGroupEditor();
    void layoutLabeledCombo(juce::Label& label, juce::ComboBox& combo, juce::Rectangle<int> bounds);
    void setLinkGroupValue(int group);
    void stepDirectorGroup(int delta);
    void commitLinkGroupText();
    void syncLinkGroupEditorFromParameter();
    void applyCurrentLinkSuggestion();
    void applyParameterValue(const char* parameterId, float value);
    bool isCurrentLinkActionApplied(const LinkSuggestionAction& action) const noexcept;
    float currentParameterValue(const char* parameterId) const noexcept;
    bool isDirectorMode() const noexcept;
    void updateVisibleMode();
    void setNodeControlsVisible(bool shouldBeVisible);
    void updateDirectorView();
    void applyDirectorTip();
    void selectDirectorNode(std::uint32_t instanceId);
    void sendDirectorSpatialCommand(std::uint32_t targetId, float pan, float depth);
    void sendDirectorSelectedControlCommand();
    void syncDirectorSelectedControls(const LinkPeerSnapshot* selectedNode);
    const DirectorConflictRecord* findFirstActiveDirectorConflict() const noexcept;
    void resetDirectorConflictMemory() noexcept;
    void updateRoleLabel();
    ResonanceSnapshot readResonanceSnapshot() const noexcept;

    struct DirectorConflictRecord
    {
        bool used = false;
        bool active = false;
        bool seenThisFrame = false;
        int missingFrames = 0;
        TrackRole currentRole = TrackRole::Unknown;
        TrackRole peerRole = TrackRole::Unknown;
        LinkSuggestionKind kind = LinkSuggestionKind::None;
        std::uint32_t targetId = invalidLinkInstanceId;
        std::uint32_t peerId = invalidLinkInstanceId;
        int targetAutoAssistMode = 0;
        juce::String summary;
        juce::String band;
        juce::String action;
    };

    PluginProcessor& processor;
    juce::AudioProcessorValueTreeState& apvts;
    HardwareLookAndFeel hardwareLookAndFeel;

    juce::Label titleLabel;
    juce::Label currentRoleLabel;

    juce::Label modeLabel;
    juce::Label autoAssistLabel;
    juce::Label roleLabel;
    juce::Label safetyLabel;
    juce::Label triggerLabel;
    juce::Label motionPresetLabel;
    juce::Label sidechainModeLabel;
    juce::Label sidechainListenLabel;
    juce::Label linkSourceRoleLabel;

    juce::ComboBox modeCombo;
    juce::ComboBox autoAssistCombo;
    juce::ComboBox roleCombo;
    juce::ComboBox safetyCombo;
    juce::ComboBox triggerCombo;
    juce::ComboBox motionPresetCombo;
    juce::ComboBox sidechainModeCombo;
    juce::ComboBox sidechainListenCombo;
    juce::ComboBox linkSourceRoleCombo;

    juce::TextButton sidechainEnableButton { "SC Enable" };
    juce::TextButton resonanceLearnButton { "Learn" };
    juce::TextButton linkEnableButton { "Link" };
    juce::TextButton linkApplyTipButton { "Apply Tip" };
    juce::TextButton directorApplyTipButton { "Apply Tip" };
    juce::TextButton directorLearnMixButton { "Learn Mix" };
    juce::TextButton directorClearMemoryButton { "Clear Memory" };
    juce::TextButton directorPreviousGroupButton { "<" };
    juce::TextButton directorNextGroupButton { ">" };

    juce::Label widthLabel;
    juce::Label depthLabel;
    juce::Label motionLabel;
    juce::Label cleanUpLabel;
    juce::Label resonanceLabel;
    juce::Label doubleLabel;
    juce::Label outputLabel;
    juce::Label sidechainAmountLabel;
    juce::Label linkGroupLabel;

    juce::Slider widthSlider;
    juce::Slider depthSlider;
    juce::Slider motionSlider;
    juce::Slider cleanUpSlider;
    juce::Slider resonanceSlider;
    juce::Slider doubleSlider;
    juce::Slider outputSlider;
    juce::Slider sidechainAmountSlider;
    juce::TextEditor linkGroupEditor;

    StageView stageView;
    MeterView inputMeter { "Input" };
    MeterView outputMeter { "Output" };
    MeterView sidechainMeter { "Sidechain" };
    MeterView reductionMeter { "GR" };
    ResonanceListView resonanceList;
    DirectorSceneView directorSceneView;
    juce::Label correlationLabel;
    juce::Label sidechainStatusLabel;
    juce::Label linkStatusLabel;
    juce::Label linkDetailLabel;
    juce::Label linkSuggestionLabel;
    juce::Label linkActionPreviewLabel;
    juce::Label autoAssistStatusLabel;
    juce::Label directorStatusLabel;
    juce::Label directorGroupsLabel;
    juce::Label directorMemoryLabel;
    juce::Label directorSelectedTitleLabel;
    juce::Label directorSelectedDetailLabel;
    juce::Label directorPanLabel;
    juce::Label directorWidthLabel;
    juce::Label directorDepthLabel;
    juce::Label directorMotionLabel;
    juce::Label directorCleanUpLabel;
    juce::Label directorResonanceLabel;
    juce::Label directorSidechainAmountLabel;
    juce::Slider directorPanSlider;
    juce::Slider directorWidthSlider;
    juce::Slider directorDepthSlider;
    juce::Slider directorMotionSlider;
    juce::Slider directorCleanUpSlider;
    juce::Slider directorResonanceSlider;
    juce::Slider directorSidechainAmountSlider;
    juce::Label directorConflictLabel;
    juce::Label directorFooterLabel;

    std::unique_ptr<ComboBoxAttachment> modeAttachment;
    std::unique_ptr<ComboBoxAttachment> autoAssistAttachment;
    std::unique_ptr<ComboBoxAttachment> roleAttachment;
    std::unique_ptr<ComboBoxAttachment> safetyAttachment;
    std::unique_ptr<ComboBoxAttachment> triggerAttachment;
    std::unique_ptr<ComboBoxAttachment> motionPresetAttachment;
    std::unique_ptr<ComboBoxAttachment> sidechainModeAttachment;
    std::unique_ptr<ComboBoxAttachment> sidechainListenAttachment;
    std::unique_ptr<ComboBoxAttachment> linkSourceRoleAttachment;
    std::unique_ptr<ButtonAttachment> sidechainEnableAttachment;
    std::unique_ptr<ButtonAttachment> linkEnableAttachment;
    std::unique_ptr<SliderAttachment> widthAttachment;
    std::unique_ptr<SliderAttachment> depthAttachment;
    std::unique_ptr<SliderAttachment> motionAttachment;
    std::unique_ptr<SliderAttachment> cleanUpAttachment;
    std::unique_ptr<SliderAttachment> resonanceAttachment;
    std::unique_ptr<SliderAttachment> doubleAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;
    std::unique_ptr<SliderAttachment> sidechainAmountAttachment;
    bool updatingLinkGroupEditor = false;
    float stageMotionPhase = 0.0f;
    LinkSuggestionKind currentLinkSuggestionKind = LinkSuggestionKind::None;
    LinkSuggestion heldLinkSuggestion;
    int heldLinkSuggestionFrames = 0;
    int linkResolvedFrames = 0;
    bool directorModeVisible = false;
    std::array<DirectorConflictRecord, maxDirectorConflictRecords> directorConflictRecords;
    int directorConflictGroup = 0;
    int directorConflictWriteIndex = 0;
    int directorCommandStatusFrames = 0;
    juce::String directorCommandStatus;
    std::uint32_t directorSelectedNodeId = invalidLinkInstanceId;
    std::uint32_t directorLastMovedNodeId = invalidLinkInstanceId;
    float directorLastSentPan = 0.0f;
    float directorLastSentDepth = 0.0f;
    bool updatingDirectorSelectedControls = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
} // namespace stagemind
