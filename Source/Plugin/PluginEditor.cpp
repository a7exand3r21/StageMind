#include "PluginEditor.h"
#include "../Model/AutoAssistMode.h"
#include "../Model/MotionPreset.h"
#include "../Model/PluginMode.h"
#include "../Model/SafetyMode.h"
#include "../Model/SidechainConflictMode.h"
#include "../Model/SidechainListenMode.h"
#include "../Model/TrackRole.h"
#include "../Model/TriggerMode.h"
#include "../UI/Theme.h"

#include <algorithm>
#include <cmath>

namespace stagemind
{
namespace
{
constexpr auto linkActionEpsilon = 0.0005f;
constexpr auto linkSuggestionHoldFrames = 30;
constexpr auto linkResolvedHoldFrames = 90;
constexpr auto directorConflictResolveFrames = 120;
constexpr auto directorGroupCount = 16;

struct NodeHardwareLayout
{
    juce::Rectangle<int> shell;
    juce::Rectangle<int> header;
    juce::Rectangle<int> brand;
    juce::Rectangle<int> glassControls;
    juce::Rectangle<int> modeControl;
    juce::Rectangle<int> autoControl;
    juce::Rectangle<int> sidechainHeaderButton;
    std::array<juce::Rectangle<int>, 5> modules;
};

bool approximatelyEqual(float first, float second) noexcept
{
    return std::abs(first - second) <= linkActionEpsilon;
}

bool isSnapshotLinkActionApplied(const LinkPeerSnapshot& target, const LinkSuggestionAction& action) noexcept
{
    if (! action.available)
        return false;

    if (action.setWidth && ! approximatelyEqual(target.width, action.width))
        return false;

    if (action.setDepth && ! approximatelyEqual(target.depth, action.depth))
        return false;

    if (action.setSidechainMode && target.sidechainMode != action.sidechainModeIndex)
        return false;

    return true;
}

int clampedInt(int value, int minimum, int maximum) noexcept
{
    return juce::jlimit(minimum, maximum, value);
}

int clampedLinkGroup(int value) noexcept
{
    return juce::jlimit(0, directorGroupCount, value);
}

int wrappedDirectorGroup(int group) noexcept
{
    while (group < 1)
        group += directorGroupCount;

    while (group > directorGroupCount)
        group -= directorGroupCount;

    return group;
}

juce::Rectangle<int> takeTop(juce::Rectangle<int>& area, int amount) noexcept
{
    return area.removeFromTop(std::min(amount, area.getHeight()));
}

NodeHardwareLayout makeNodeHardwareLayout(juce::Rectangle<int> localBounds, bool compactHeight)
{
    NodeHardwareLayout layout;
    layout.shell = localBounds.reduced(theme::margin);

    auto inner = layout.shell.reduced(compactHeight ? 12 : 18, compactHeight ? 10 : 16);
    layout.header = takeTop(inner, compactHeight ? 72 : 86);
    takeTop(inner, compactHeight ? 8 : 14);

    layout.brand = layout.header.removeFromLeft(juce::jlimit(190, 310, layout.header.getWidth() / 4));
    layout.sidechainHeaderButton = layout.header.removeFromRight(juce::jlimit(150, 230, layout.header.getWidth() / 4)).reduced(14, compactHeight ? 18 : 22);
    layout.glassControls = layout.header.withTrimmedLeft(juce::jmin(12, layout.header.getWidth())).withTrimmedRight(juce::jmin(12, layout.header.getWidth())).reduced(0, compactHeight ? 8 : 10);

    auto brandUtility = layout.brand.removeFromBottom(compactHeight ? 36 : 40).reduced(6, 3);
    layout.modeControl = brandUtility.removeFromLeft(juce::jmin(112, brandUtility.getWidth() / 2)).reduced(0, 2);
    brandUtility.removeFromLeft(juce::jmin(6, brandUtility.getWidth()));
    layout.autoControl = brandUtility.removeFromLeft(juce::jmin(96, brandUtility.getWidth())).reduced(0, 2);

    auto body = inner.reduced(0, compactHeight ? 0 : 2);
    const auto gap = compactHeight ? 4 : 6;
    const auto usableWidth = std::max(1, body.getWidth() - gap * 4);
    const auto stageWidth = juce::jlimit(190, 330, static_cast<int> (static_cast<float> (usableWidth) * 0.22f));
    const auto spaceWidth = juce::jlimit(150, 240, static_cast<int> (static_cast<float> (usableWidth) * 0.18f));
    const auto characterWidth = juce::jlimit(160, 260, static_cast<int> (static_cast<float> (usableWidth) * 0.20f));
    const auto sidechainWidth = juce::jlimit(155, 245, static_cast<int> (static_cast<float> (usableWidth) * 0.18f));

    auto columns = body;
    layout.modules[0] = columns.removeFromLeft(std::min(stageWidth, columns.getWidth()));
    columns.removeFromLeft(std::min(gap, columns.getWidth()));
    layout.modules[1] = columns.removeFromLeft(std::min(spaceWidth, columns.getWidth()));
    columns.removeFromLeft(std::min(gap, columns.getWidth()));
    layout.modules[2] = columns.removeFromLeft(std::min(characterWidth, columns.getWidth()));
    columns.removeFromLeft(std::min(gap, columns.getWidth()));
    layout.modules[3] = columns.removeFromLeft(std::min(sidechainWidth, columns.getWidth()));
    columns.removeFromLeft(std::min(gap, columns.getWidth()));
    layout.modules[4] = columns;

    return layout;
}

void drawHardwareModule(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& number, const juce::String& title)
{
    const auto panel = bounds.toFloat();
    juce::DropShadow(theme::shadow.withAlpha(0.45f), 8, { 0, 3 }).drawForRectangle(g, bounds);

    juce::ColourGradient gradient(
        theme::panelRaised,
        panel.getCentreX(),
        panel.getY(),
        theme::panelInset,
        panel.getCentreX(),
        panel.getBottom(),
        false);
    gradient.addColour(0.45, juce::Colour { 0xfffdfdfb });
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(panel, 16.0f);

    g.setColour(juce::Colours::white.withAlpha(0.78f));
    g.drawRoundedRectangle(panel.reduced(1.0f), 15.0f, 1.0f);
    g.setColour(theme::border.withAlpha(0.90f));
    g.drawRoundedRectangle(panel.reduced(0.5f), 16.0f, 1.0f);

    auto titleArea = bounds.removeFromTop(42).reduced(12, 0);
    g.setColour(theme::text.withAlpha(0.88f));
    g.setFont(juce::FontOptions { 13.0f, juce::Font::plain });
    g.drawText(number, titleArea.removeFromLeft(28), juce::Justification::centred);
    g.setColour(theme::textMuted.withAlpha(0.85f));
    g.setFont(juce::FontOptions { 13.0f });
    g.drawFittedText(title, titleArea, juce::Justification::centredLeft, 1);
}

void drawHeaderGlass(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    const auto glass = bounds.toFloat();
    juce::DropShadow(theme::shadow.withAlpha(0.50f), 8, { 0, 2 }).drawForRectangle(g, bounds);
    juce::ColourGradient gradient(
        theme::displayRaised,
        glass.getCentreX(),
        glass.getY(),
        theme::display,
        glass.getCentreX(),
        glass.getBottom(),
        false);
    gradient.addColour(0.08, juce::Colour { 0xff2b3339 });
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(glass, 16.0f);
    g.setColour(juce::Colours::white.withAlpha(0.10f));
    g.fillRoundedRectangle(glass.reduced(4.0f).withHeight(glass.getHeight() * 0.36f), 12.0f);
    g.setColour(theme::borderDark.withAlpha(0.70f));
    g.drawRoundedRectangle(glass.reduced(0.5f), 16.0f, 1.0f);

    g.setColour(theme::textOnDisplay.withAlpha(0.18f));
    const auto firstSeparator = glass.getX() + glass.getWidth() / 3.0f;
    const auto secondSeparator = glass.getX() + glass.getWidth() * 2.0f / 3.0f;
    g.drawVerticalLine(static_cast<int> (firstSeparator), glass.getY() + 14.0f, glass.getBottom() - 14.0f);
    g.drawVerticalLine(static_cast<int> (secondSeparator), glass.getY() + 14.0f, glass.getBottom() - 14.0f);
}

void drawBrand(juce::Graphics& g, juce::Rectangle<int> area)
{
    auto brand = area.withTrimmedTop(6).reduced(6, 0);
    g.setFont(juce::FontOptions { 24.0f, juce::Font::plain });
    g.setColour(theme::text);
    g.drawText("STAGE", brand.removeFromTop(28).removeFromLeft(112), juce::Justification::centredLeft);
    auto mind = area.withTrimmedTop(6).withTrimmedLeft(106).withHeight(28);
    g.setColour(juce::Colour { 0xff35d8e3 });
    g.drawText("MIND", mind, juce::Justification::centredLeft);

    g.setColour(theme::textMuted);
    g.setFont(juce::FontOptions { 11.0f });
    g.drawFittedText("S P A T I A L   P R O C E S S O R", area.withTrimmedTop(36).withHeight(18).reduced(6, 0), juce::Justification::centredLeft, 1);
}

const char* strongestBandName(LinkSpectralBands bands) noexcept
{
    auto name = "low";
    auto value = bands.low;

    if (bands.lowMid > value)
    {
        name = "low-mid";
        value = bands.lowMid;
    }

    if (bands.presence > value)
    {
        name = "presence";
        value = bands.presence;
    }

    if (bands.air > value)
        name = "air";

    return name;
}

float strongestBandValue(LinkSpectralBands bands) noexcept
{
    return std::max({ bands.low, bands.lowMid, bands.presence, bands.air });
}

void layoutRotary(juce::Label& label, juce::Slider& slider, juce::Rectangle<int> area)
{
    area = area.reduced(3, 0);
    label.setBounds(takeTop(area, 22));
    slider.setBounds(area);
}

void layoutRotaryRow(
    juce::Rectangle<int> area,
    juce::Label& firstLabel,
    juce::Slider& firstSlider,
    juce::Label& secondLabel,
    juce::Slider& secondSlider,
    juce::Label& thirdLabel,
    juce::Slider& thirdSlider)
{
    const auto columnWidth = std::max(1, area.getWidth() / 3);
    layoutRotary(firstLabel, firstSlider, area.removeFromLeft(columnWidth));
    layoutRotary(secondLabel, secondSlider, area.removeFromLeft(columnWidth));
    layoutRotary(thirdLabel, thirdSlider, area);
}

struct RotaryRef
{
    juce::Label* label = nullptr;
    juce::Slider* slider = nullptr;
};

void layoutRotaryGrid(juce::Rectangle<int> area, std::array<RotaryRef, 4> controls, int count)
{
    if (count <= 0)
        return;

    const auto columns = count == 4 && area.getWidth() < 420 ? 2 : count;
    const auto rows = (count + columns - 1) / columns;
    const auto rowHeight = std::max(1, area.getHeight() / rows);
    auto index = 0;

    for (int row = 0; row < rows && index < count; ++row)
    {
        auto rowArea = area.removeFromTop(row == rows - 1 ? area.getHeight() : rowHeight);
        const auto cellsInRow = std::min(columns, count - index);
        const auto columnWidth = std::max(1, rowArea.getWidth() / cellsInRow);

        for (int column = 0; column < cellsInRow && index < count; ++column, ++index)
        {
            auto cell = column == cellsInRow - 1 ? rowArea : rowArea.removeFromLeft(columnWidth);
            layoutRotary(*controls[static_cast<size_t> (index)].label, *controls[static_cast<size_t> (index)].slider, cell);
        }
    }
}

void layoutMeterRow(
    juce::Rectangle<int> area,
    MeterView& inputMeter,
    MeterView& outputMeter,
    MeterView& sidechainMeter,
    MeterView& reductionMeter)
{
    const auto meterWidth = std::max(1, area.getWidth() / 4);
    inputMeter.setBounds(area.removeFromLeft(meterWidth).reduced(2));
    outputMeter.setBounds(area.removeFromLeft(meterWidth).reduced(2));
    sidechainMeter.setBounds(area.removeFromLeft(meterWidth).reduced(2));
    reductionMeter.setBounds(area.reduced(2));
}

void layoutUtilityCombo(juce::Label& label, juce::ComboBox& combo, juce::Rectangle<int> bounds)
{
    label.setBounds({});
    combo.setBounds(bounds.reduced(0, 1));
}

juce::String shortLabelForRole(TrackRole role)
{
    switch (role)
    {
        case TrackRole::LeadVocal: return "Lead Vox";
        case TrackRole::BackingVocal: return "Back Vox";
        case TrackRole::RhythmGuitarSingle: return "Guitar";
        case TrackRole::RhythmGuitarPairLeft: return "Gtr L";
        case TrackRole::RhythmGuitarPairRight: return "Gtr R";
        case TrackRole::LeadGuitar: return "Lead Gtr";
        case TrackRole::SynthLead: return "Synth Lead";
        case TrackRole::SynthBass: return "Synth Bass";
        case TrackRole::SunoVocal: return "Suno Vox";
        case TrackRole::SunoInstrumental: return "Suno Inst";
        case TrackRole::SunoDrums: return "Suno Drums";
        case TrackRole::SunoBass: return "Suno Bass";
        case TrackRole::SunoGuitar: return "Suno Gtr";
        case TrackRole::SunoSynthPad: return "Suno Pad";
        case TrackRole::SunoPercussion: return "Suno Perc";
        case TrackRole::SunoFX: return "Suno FX";
        case TrackRole::Unknown: return "Any Role";
        default: return labelForRole(role);
    }
}

juce::String suggestionSummary(const LinkSuggestion& suggestion)
{
    auto text = juce::String(suggestion.message);
    return text.replace("Conflict: ", "");
}

juce::String correctionTextForSuggestion(const LinkSuggestion& suggestion)
{
    const auto action = LinkSuggestionEngine::actionFor(suggestion.kind);
    if (! action.available)
        return "Observe";

    auto text = juce::String(suggestion.actionLabel);
    if (action.previewMessage[0] != '\0')
        text += " (" + juce::String(action.previewMessage) + ")";

    return text;
}

juce::String autoAssistShortLabel(int modeIndex)
{
    switch (autoAssistModeFromIndex(modeIndex))
    {
        case AutoAssistMode::Auto:    return "Auto";
        case AutoAssistMode::Suggest: return "Suggest";
        case AutoAssistMode::Off:
        default:                      return "Off";
    }
}

juce::String selectedDirectorNodeText(const LinkPeerSnapshot& node)
{
    juce::String text;
    text += "Selected #" + juce::String(node.instanceId) + " " + shortLabelForRole(static_cast<TrackRole> (node.role)) + "\n";
    text += "Pan " + juce::String(node.pan, 2)
        + "  Depth " + juce::String(static_cast<int> (node.depth * 100.0f)) + "%"
        + "  Width " + juce::String(static_cast<int> (node.width * 100.0f)) + "%\n";
    text += "Motion " + juce::String(static_cast<int> (node.motion * 100.0f)) + "%"
        + "  SC " + juce::String(static_cast<int> (node.sidechainAmount * 100.0f)) + "%\n";
    text += "Auto " + autoAssistShortLabel(node.autoAssistMode)
        + "  Activity " + juce::String(static_cast<int> (node.activity * 100.0f)) + "%"
        + "  Corr " + juce::String(node.correlation, 2);
    return text;
}

juce::String sceneLabelForSuggestion(LinkSuggestionKind kind)
{
    switch (kind)
    {
        case LinkSuggestionKind::StereoSafety:             return "Width";
        case LinkSuggestionKind::VocalSpace:               return "Space";
        case LinkSuggestionKind::KickBass:                 return "Kick/Bass";
        case LinkSuggestionKind::VocalInstrumentDucking:   return "Vocal";
        case LinkSuggestionKind::SnareInstrumentDucking:   return "Snare";
        case LinkSuggestionKind::LeadPadDucking:           return "Lead/Pad";
        case LinkSuggestionKind::DrumsInstrumentDucking:   return "Drums";
        case LinkSuggestionKind::DoubleWide:               return "Wide";
        case LinkSuggestionKind::None:
        default:                                           return "Conflict";
    }
}

juce::String actionLabelForTimelineEvent(int actionKind)
{
    const auto kind = static_cast<LinkSuggestionKind> (actionKind);
    const auto action = LinkSuggestionEngine::actionFor(kind);
    if (action.previewMessage[0] != '\0')
        return action.previewMessage;

    return sceneLabelForSuggestion(kind);
}

const char* rideMemoryBandLabel(int band) noexcept
{
    switch (static_cast<RideMemoryBand> (band))
    {
        case RideMemoryBand::Low:      return "low";
        case RideMemoryBand::LowMid:   return "low-mid";
        case RideMemoryBand::Presence: return "presence";
        case RideMemoryBand::Air:      return "air";
        case RideMemoryBand::Unknown:
        default:                       return "band";
    }
}

juce::String ppqText(double ppqPosition)
{
    return "PPQ " + juce::String(ppqPosition, 1);
}

juce::String timelineEventText(const RideTimelineEvent& event)
{
    auto text = ppqText(event.lastSeenPpq)
        + " " + shortLabelForRole(static_cast<TrackRole> (event.targetRole))
        + " <- " + shortLabelForRole(static_cast<TrackRole> (event.sourceRole))
        + " " + actionLabelForTimelineEvent(event.actionKind);

    text += " / ";
    text += rideMemoryBandLabel(event.band);
    text += event.resolved ? " resolved" : " pending";
    return text;
}

juce::String paddedGroupNumber(int group)
{
    return group < 10 ? "0" + juce::String(group) : juce::String(group);
}

juce::String makeDirectorGroupOverview(
    int selectedGroup,
    const std::array<int, directorGroupCount + 1>& groupCounts,
    const std::array<int, directorGroupCount + 1>& activeCounts)
{
    juce::String text = "Groups  nodes/active\n";

    for (int group = 1; group <= directorGroupCount; ++group)
    {
        if (group > 1)
            text += group % 4 == 1 ? "\n" : "  ";

        const auto count = groupCounts[static_cast<size_t> (group)];
        const auto active = activeCounts[static_cast<size_t> (group)];
        auto item = "G" + paddedGroupNumber(group) + ":";
        item += count > 0 ? juce::String(count) + "/" + juce::String(active) : "-";

        if (group == selectedGroup)
            item = "[" + item + "]";

        text += item;
    }

    return text;
}
} // namespace

PluginEditor::PluginEditor(PluginProcessor& processorToUse)
    : AudioProcessorEditor(&processorToUse),
      processor(processorToUse),
      apvts(processorToUse.getValueTreeState())
{
    setLookAndFeel(&hardwareLookAndFeel);
    setResizable(true, true);
    setResizeLimits(1040, 640, 1600, 980);
    setSize(1280, 760);

    titleLabel.setText("", juce::dontSendNotification);
    titleLabel.setColour(juce::Label::textColourId, theme::text);
    titleLabel.setFont(juce::FontOptions { 22.0f, juce::Font::bold });
    addAndMakeVisible(titleLabel);

    currentRoleLabel.setColour(juce::Label::textColourId, theme::textMuted);
    currentRoleLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(currentRoleLabel);

    setupCombo(modeCombo, makePluginModeNames());
    setupCombo(autoAssistCombo, makeAutoAssistModeNames());
    setupCombo(roleCombo, makeSelectableRoleNames());
    setupCombo(safetyCombo, makeSafetyModeNames());
    setupCombo(triggerCombo, makeTriggerModeNames());
    setupCombo(motionPresetCombo, makeMotionPresetNames());
    setupCombo(sidechainModeCombo, makeSidechainConflictModeNames());
    setupCombo(sidechainListenCombo, { "Off", "Sidechain Only" });
    auto linkSourceRoleNames = makeRoleNamesWithUnknown();
    linkSourceRoleNames.set(0, "Any Role");
    setupCombo(linkSourceRoleCombo, linkSourceRoleNames);
    modeCombo.setComponentID("miniCombo");
    autoAssistCombo.setComponentID("miniCombo");
    roleCombo.setComponentID("headerCombo");
    safetyCombo.setComponentID("headerCombo");
    motionPresetCombo.setComponentID("headerCombo");

    sidechainEnableButton.setClickingTogglesState(true);
    linkEnableButton.setClickingTogglesState(true);
    sidechainEnableButton.setComponentID("pillGlowButton");
    linkEnableButton.setComponentID("glowButton");
    linkApplyTipButton.setComponentID("softButton");
    resonanceLearnButton.setComponentID("pillGlowButton");
    sidechainEnableButton.setColour(juce::TextButton::textColourOffId, theme::accent.darker(0.25f));
    linkEnableButton.setColour(juce::TextButton::textColourOffId, theme::accent.darker(0.15f));
    resonanceLearnButton.setColour(juce::TextButton::textColourOffId, theme::textMuted);
    setupButton(sidechainEnableButton);
    setupButton(linkEnableButton);
    setupButton(linkApplyTipButton);
    setupButton(directorApplyTipButton);
    setupButton(directorLearnMixButton);
    setupButton(directorClearMemoryButton);
    setupButton(directorPreviousGroupButton);
    setupButton(directorNextGroupButton);
    setupButton(resonanceLearnButton);
    sidechainEnableButton.setColour(juce::TextButton::textColourOffId, theme::accent.darker(0.25f));
    linkEnableButton.setColour(juce::TextButton::textColourOffId, theme::accent.darker(0.15f));
    resonanceLearnButton.setColour(juce::TextButton::textColourOffId, theme::textMuted);
    resonanceLearnButton.onClick = [this]
    {
        processor.beginResonanceLearn();
    };
    linkApplyTipButton.onClick = [this]
    {
        applyCurrentLinkSuggestion();
    };
    directorApplyTipButton.onClick = [this]
    {
        applyDirectorTip();
    };
    directorLearnMixButton.onClick = [this]
    {
        processor.beginRideMemoryLearn();
    };
    directorClearMemoryButton.onClick = [this]
    {
        processor.clearRideMemory();
    };
    directorPreviousGroupButton.onClick = [this]
    {
        stepDirectorGroup(-1);
    };
    directorNextGroupButton.onClick = [this]
    {
        stepDirectorGroup(1);
    };

    layoutLabeledCombo(modeLabel, modeCombo, {});
    layoutLabeledCombo(autoAssistLabel, autoAssistCombo, {});
    layoutLabeledCombo(roleLabel, roleCombo, {});
    layoutLabeledCombo(safetyLabel, safetyCombo, {});
    layoutLabeledCombo(triggerLabel, triggerCombo, {});
    layoutLabeledCombo(motionPresetLabel, motionPresetCombo, {});
    layoutLabeledCombo(sidechainModeLabel, sidechainModeCombo, {});
    layoutLabeledCombo(sidechainListenLabel, sidechainListenCombo, {});
    layoutLabeledCombo(linkSourceRoleLabel, linkSourceRoleCombo, {});
    modeLabel.setText("MODE", juce::dontSendNotification);
    autoAssistLabel.setText("AUTO", juce::dontSendNotification);
    roleLabel.setText("ROLE", juce::dontSendNotification);
    safetyLabel.setText("PRESET", juce::dontSendNotification);
    triggerLabel.setText("TRIGGER", juce::dontSendNotification);
    motionPresetLabel.setText("MOTION", juce::dontSendNotification);
    sidechainModeLabel.setText("MAKE SPACE", juce::dontSendNotification);
    sidechainListenLabel.setText("SC LISTEN", juce::dontSendNotification);
    linkSourceRoleLabel.setText("ROLE", juce::dontSendNotification);

    setupSlider(widthSlider, widthLabel, "Width");
    setupSlider(depthSlider, depthLabel, "Depth");
    setupSlider(motionSlider, motionLabel, "Motion");
    setupSlider(cleanUpSlider, cleanUpLabel, "Clean Up");
    setupSlider(resonanceSlider, resonanceLabel, "Resonance");
    setupSlider(doubleSlider, doubleLabel, "Double");
    setupSlider(outputSlider, outputLabel, "Output");
    setupSlider(sidechainAmountSlider, sidechainAmountLabel, "SC Amount");

    linkGroupLabel.setText("Group", juce::dontSendNotification);
    linkGroupLabel.setColour(juce::Label::textColourId, theme::textMuted);
    linkGroupLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(linkGroupLabel);
    setupLinkGroupEditor();

    addAndMakeVisible(stageView);
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);
    addAndMakeVisible(sidechainMeter);
    addAndMakeVisible(reductionMeter);
    addAndMakeVisible(resonanceList);
    addAndMakeVisible(directorSceneView);
    directorSceneView.onNodeSelected = [this](std::uint32_t instanceId)
    {
        selectDirectorNode(instanceId);
    };
    directorSceneView.onNodeMoved = [this](std::uint32_t instanceId, float pan, float depth)
    {
        sendDirectorSpatialCommand(instanceId, pan, depth);
    };

    correlationLabel.setColour(juce::Label::textColourId, theme::text);
    correlationLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(correlationLabel);

    sidechainStatusLabel.setColour(juce::Label::textColourId, theme::textMuted);
    sidechainStatusLabel.setJustificationType(juce::Justification::centred);
    sidechainStatusLabel.setFont(juce::FontOptions { 12.0f });
    addAndMakeVisible(sidechainStatusLabel);

    linkStatusLabel.setColour(juce::Label::textColourId, theme::textMuted);
    linkStatusLabel.setJustificationType(juce::Justification::centred);
    linkStatusLabel.setFont(juce::FontOptions { 12.0f });
    addAndMakeVisible(linkStatusLabel);

    linkDetailLabel.setColour(juce::Label::textColourId, theme::textMuted);
    linkDetailLabel.setJustificationType(juce::Justification::centred);
    linkDetailLabel.setFont(juce::FontOptions { 11.0f });
    addAndMakeVisible(linkDetailLabel);

    linkSuggestionLabel.setColour(juce::Label::textColourId, theme::textMuted);
    linkSuggestionLabel.setJustificationType(juce::Justification::centred);
    linkSuggestionLabel.setFont(juce::FontOptions { 11.0f });
    addAndMakeVisible(linkSuggestionLabel);

    linkActionPreviewLabel.setColour(juce::Label::textColourId, theme::textMuted);
    linkActionPreviewLabel.setJustificationType(juce::Justification::centred);
    linkActionPreviewLabel.setFont(juce::FontOptions { 10.5f });
    addAndMakeVisible(linkActionPreviewLabel);
    linkApplyTipButton.setEnabled(false);

    autoAssistStatusLabel.setColour(juce::Label::textColourId, theme::textMuted);
    autoAssistStatusLabel.setJustificationType(juce::Justification::centred);
    autoAssistStatusLabel.setFont(juce::FontOptions { 10.5f });
    addAndMakeVisible(autoAssistStatusLabel);

    directorStatusLabel.setColour(juce::Label::textColourId, theme::textMuted);
    directorStatusLabel.setJustificationType(juce::Justification::centredRight);
    directorStatusLabel.setFont(juce::FontOptions { 13.0f });
    addAndMakeVisible(directorStatusLabel);

    directorGroupsLabel.setColour(juce::Label::textColourId, theme::textMuted);
    directorGroupsLabel.setJustificationType(juce::Justification::topLeft);
    directorGroupsLabel.setFont(juce::FontOptions { 10.5f });
    addAndMakeVisible(directorGroupsLabel);

    directorMemoryLabel.setColour(juce::Label::textColourId, theme::textMuted);
    directorMemoryLabel.setJustificationType(juce::Justification::topLeft);
    directorMemoryLabel.setFont(juce::FontOptions { 11.0f });
    addAndMakeVisible(directorMemoryLabel);

    directorSelectedTitleLabel.setText("Selected Node", juce::dontSendNotification);
    directorSelectedTitleLabel.setColour(juce::Label::textColourId, theme::text);
    directorSelectedTitleLabel.setJustificationType(juce::Justification::centredLeft);
    directorSelectedTitleLabel.setFont(juce::FontOptions { 14.0f, juce::Font::bold });
    addAndMakeVisible(directorSelectedTitleLabel);

    directorSelectedDetailLabel.setText("Click a node in Stage View", juce::dontSendNotification);
    directorSelectedDetailLabel.setColour(juce::Label::textColourId, theme::textMuted);
    directorSelectedDetailLabel.setJustificationType(juce::Justification::topLeft);
    directorSelectedDetailLabel.setFont(juce::FontOptions { 11.5f });
    addAndMakeVisible(directorSelectedDetailLabel);

    setupDirectorRemoteSlider(directorPanSlider, directorPanLabel, "Pan", -1.0, 1.0);
    setupDirectorRemoteSlider(directorWidthSlider, directorWidthLabel, "Width", 0.0, 1.0);
    setupDirectorRemoteSlider(directorDepthSlider, directorDepthLabel, "Depth", 0.0, 1.0);
    setupDirectorRemoteSlider(directorMotionSlider, directorMotionLabel, "Motion", 0.0, 1.0);
    setupDirectorRemoteSlider(directorCleanUpSlider, directorCleanUpLabel, "Clean Up", 0.0, 1.0);
    setupDirectorRemoteSlider(directorResonanceSlider, directorResonanceLabel, "Resonance", 0.0, 1.0);
    setupDirectorRemoteSlider(directorSidechainAmountSlider, directorSidechainAmountLabel, "SC Amount", 0.0, 1.0);

    const auto remoteSliderChanged = [this]
    {
        sendDirectorSelectedControlCommand();
    };
    directorPanSlider.onValueChange = remoteSliderChanged;
    directorWidthSlider.onValueChange = remoteSliderChanged;
    directorDepthSlider.onValueChange = remoteSliderChanged;
    directorMotionSlider.onValueChange = remoteSliderChanged;
    directorCleanUpSlider.onValueChange = remoteSliderChanged;
    directorResonanceSlider.onValueChange = remoteSliderChanged;
    directorSidechainAmountSlider.onValueChange = remoteSliderChanged;

    directorConflictLabel.setColour(juce::Label::textColourId, theme::textMuted);
    directorConflictLabel.setJustificationType(juce::Justification::topLeft);
    directorConflictLabel.setFont(juce::FontOptions { 13.0f });
    addAndMakeVisible(directorConflictLabel);

    directorFooterLabel.setText("Observe only. Corrections stay in Node.", juce::dontSendNotification);
    directorFooterLabel.setColour(juce::Label::textColourId, theme::textMuted);
    directorFooterLabel.setJustificationType(juce::Justification::centred);
    directorFooterLabel.setFont(juce::FontOptions { 12.0f });
    addAndMakeVisible(directorFooterLabel);

    modeAttachment = std::make_unique<ComboBoxAttachment>(apvts, parameters::ids::pluginMode, modeCombo);
    autoAssistAttachment = std::make_unique<ComboBoxAttachment>(apvts, parameters::ids::autoAssistMode, autoAssistCombo);
    roleAttachment = std::make_unique<ComboBoxAttachment>(apvts, parameters::ids::role, roleCombo);
    safetyAttachment = std::make_unique<ComboBoxAttachment>(apvts, parameters::ids::safety, safetyCombo);
    triggerAttachment = std::make_unique<ComboBoxAttachment>(apvts, parameters::ids::triggerMode, triggerCombo);
    motionPresetAttachment = std::make_unique<ComboBoxAttachment>(apvts, parameters::ids::motionPreset, motionPresetCombo);
    sidechainModeAttachment = std::make_unique<ComboBoxAttachment>(apvts, parameters::ids::sidechainMode, sidechainModeCombo);
    sidechainListenAttachment = std::make_unique<ComboBoxAttachment>(apvts, parameters::ids::sidechainListen, sidechainListenCombo);
    linkSourceRoleAttachment = std::make_unique<ComboBoxAttachment>(apvts, parameters::ids::linkRole, linkSourceRoleCombo);
    sidechainEnableAttachment = std::make_unique<ButtonAttachment>(apvts, parameters::ids::sidechainEnabled, sidechainEnableButton);
    linkEnableAttachment = std::make_unique<ButtonAttachment>(apvts, parameters::ids::linkEnabled, linkEnableButton);
    widthAttachment = std::make_unique<SliderAttachment>(apvts, parameters::ids::width, widthSlider);
    depthAttachment = std::make_unique<SliderAttachment>(apvts, parameters::ids::depth, depthSlider);
    motionAttachment = std::make_unique<SliderAttachment>(apvts, parameters::ids::motion, motionSlider);
    cleanUpAttachment = std::make_unique<SliderAttachment>(apvts, parameters::ids::cleanUp, cleanUpSlider);
    resonanceAttachment = std::make_unique<SliderAttachment>(apvts, parameters::ids::resonance, resonanceSlider);
    doubleAttachment = std::make_unique<SliderAttachment>(apvts, parameters::ids::pseudoDoubleAmount, doubleSlider);
    outputAttachment = std::make_unique<SliderAttachment>(apvts, parameters::ids::outputGain, outputSlider);
    sidechainAmountAttachment = std::make_unique<SliderAttachment>(apvts, parameters::ids::sidechainAmount, sidechainAmountSlider);

    syncLinkGroupEditorFromParameter();
    directorModeVisible = isDirectorMode();
    updateVisibleMode();
    updateRoleLabel();
    startTimerHz(30);
}

PluginEditor::~PluginEditor()
{
    setLookAndFeel(nullptr);
}

void PluginEditor::paint(juce::Graphics& g)
{
    g.fillAll(theme::background);

    auto bounds = getLocalBounds().reduced(theme::margin).toFloat();
    juce::DropShadow(theme::shadow, 18, { 0, 8 }).drawForRectangle(g, bounds.toNearestInt());

    juce::ColourGradient shell(
        theme::panelRaised,
        bounds.getCentreX(),
        bounds.getY(),
        theme::panelInset,
        bounds.getCentreX(),
        bounds.getBottom(),
        false);
    shell.addColour(0.48, theme::panel);
    g.setGradientFill(shell);
    g.fillRoundedRectangle(bounds, static_cast<float> (theme::corner));

    g.setColour(juce::Colours::white.withAlpha(0.78f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), static_cast<float> (theme::corner), 1.0f);
    g.setColour(theme::border);
    g.drawRoundedRectangle(bounds.reduced(0.5f), static_cast<float> (theme::corner), 1.0f);

    if (! isDirectorMode())
    {
        const auto nodeLayout = makeNodeHardwareLayout(getLocalBounds(), getHeight() < 700);
        drawBrand(g, nodeLayout.brand);
        drawHeaderGlass(g, nodeLayout.glassControls);
        drawHardwareModule(g, nodeLayout.modules[0], "1", "S T A G E   V I E W");
        drawHardwareModule(g, nodeLayout.modules[1], "2", "S P A C E");
        drawHardwareModule(g, nodeLayout.modules[2], "3", "C H A R A C T E R");
        drawHardwareModule(g, nodeLayout.modules[3], "4", "S I D E C H A I N");
        drawHardwareModule(g, nodeLayout.modules[4], "5", "M E T E R I N G");
    }

    g.setColour(theme::textMuted.withAlpha(0.72f));
    g.setFont(juce::FontOptions { 8.0f, juce::Font::plain });
    g.drawFittedText(
        "S T A G E M I N D",
        getLocalBounds().removeFromBottom(theme::margin).reduced(0, 2),
        juce::Justification::centred,
        1);
}

void PluginEditor::resized()
{
    updateVisibleMode();

    auto bounds = getLocalBounds().reduced(theme::margin);
    const auto compactHeight = getHeight() < 700;
    const auto innerGap = compactHeight ? 6 : theme::gap;

    if (! isDirectorMode())
    {
        const auto nodeLayout = makeNodeHardwareLayout(getLocalBounds(), compactHeight);

        titleLabel.setBounds({});
        currentRoleLabel.setBounds({});
        roleLabel.setColour(juce::Label::textColourId, juce::Colour { 0xff6cebf1 });
        safetyLabel.setColour(juce::Label::textColourId, juce::Colour { 0xff6cebf1 });
        motionPresetLabel.setColour(juce::Label::textColourId, juce::Colour { 0xff6cebf1 });
        triggerLabel.setColour(juce::Label::textColourId, theme::textMuted);
        sidechainModeLabel.setColour(juce::Label::textColourId, theme::textMuted);
        sidechainListenLabel.setColour(juce::Label::textColourId, theme::textMuted);
        linkSourceRoleLabel.setColour(juce::Label::textColourId, theme::textMuted);
        layoutUtilityCombo(modeLabel, modeCombo, nodeLayout.modeControl);
        layoutUtilityCombo(autoAssistLabel, autoAssistCombo, nodeLayout.autoControl);

        auto glass = nodeLayout.glassControls.reduced(compactHeight ? 18 : 26, compactHeight ? 8 : 12);
        const auto glassColumnWidth = std::max(1, glass.getWidth() / 3);
        layoutLabeledCombo(roleLabel, roleCombo, glass.removeFromLeft(glassColumnWidth).reduced(4, 0));
        layoutLabeledCombo(safetyLabel, safetyCombo, glass.removeFromLeft(glassColumnWidth).reduced(4, 0));
        layoutLabeledCombo(motionPresetLabel, motionPresetCombo, glass.reduced(4, 0));
        sidechainEnableButton.setBounds(nodeLayout.sidechainHeaderButton);

        auto stageModule = nodeLayout.modules[0].reduced(16, 14);
        takeTop(stageModule, compactHeight ? 40 : 48);
        stageView.setBounds(stageModule.reduced(4, 0));

        auto spaceModule = nodeLayout.modules[1].reduced(20, 14);
        takeTop(spaceModule, compactHeight ? 42 : 52);
        const auto spaceKnobHeight = std::max(90, spaceModule.getHeight() / 3);
        layoutRotary(widthLabel, widthSlider, takeTop(spaceModule, spaceKnobHeight));
        layoutRotary(depthLabel, depthSlider, takeTop(spaceModule, spaceKnobHeight));
        layoutRotary(motionLabel, motionSlider, spaceModule);

        auto characterModule = nodeLayout.modules[2].reduced(20, 14);
        takeTop(characterModule, compactHeight ? 42 : 52);
        const auto outputStripHeight = compactHeight ? 64 : 78;
        auto characterKnobs = characterModule;
        characterKnobs.removeFromBottom(outputStripHeight);
        const auto characterKnobHeight = std::max(82, characterKnobs.getHeight() / 3);
        layoutRotary(cleanUpLabel, cleanUpSlider, takeTop(characterKnobs, characterKnobHeight));
        layoutRotary(resonanceLabel, resonanceSlider, takeTop(characterKnobs, characterKnobHeight));
        layoutRotary(doubleLabel, doubleSlider, characterKnobs);

        auto outputStrip = characterModule.removeFromBottom(outputStripHeight);
        outputLabel.setBounds(takeTop(outputStrip, 18));
        outputSlider.setBounds(outputStrip.reduced(6, 4));

        auto sidechainModule = nodeLayout.modules[3].reduced(18, 14);
        takeTop(sidechainModule, compactHeight ? 42 : 52);
        layoutLabeledCombo(triggerLabel, triggerCombo, takeTop(sidechainModule, compactHeight ? 48 : 54).reduced(4, 0));
        takeTop(sidechainModule, compactHeight ? 4 : 8);
        layoutRotary(sidechainAmountLabel, sidechainAmountSlider, takeTop(sidechainModule, compactHeight ? 118 : 148));
        layoutLabeledCombo(sidechainModeLabel, sidechainModeCombo, takeTop(sidechainModule, compactHeight ? 48 : 54).reduced(4, 0));
        layoutLabeledCombo(sidechainListenLabel, sidechainListenCombo, takeTop(sidechainModule, compactHeight ? 48 : 54).reduced(4, 0));
        sidechainStatusLabel.setBounds(takeTop(sidechainModule, compactHeight ? 22 : 26).reduced(4, 0));

        auto linkRow = takeTop(sidechainModule, compactHeight ? 34 : 40).reduced(4, 4);
        linkEnableButton.setBounds(linkRow.removeFromLeft(std::min(64, linkRow.getWidth())));
        linkRow.removeFromLeft(std::min(8, linkRow.getWidth()));
        linkGroupLabel.setBounds(linkRow.removeFromLeft(std::min(54, linkRow.getWidth())));
        linkGroupEditor.setBounds(linkRow.removeFromLeft(std::min(46, linkRow.getWidth())));
        layoutLabeledCombo(linkSourceRoleLabel, linkSourceRoleCombo, takeTop(sidechainModule, compactHeight ? 44 : 50).reduced(4, 0));
        linkStatusLabel.setBounds(takeTop(sidechainModule, compactHeight ? 22 : 24).reduced(4, 0));
        linkDetailLabel.setBounds(takeTop(sidechainModule, compactHeight ? 22 : 24).reduced(4, 0));
        linkSuggestionLabel.setBounds(takeTop(sidechainModule, compactHeight ? 22 : 24).reduced(4, 0));
        linkActionPreviewLabel.setBounds(takeTop(sidechainModule, compactHeight ? 22 : 24).reduced(4, 0));
        linkApplyTipButton.setBounds(takeTop(sidechainModule, compactHeight ? 28 : 32).reduced(4, 2));
        autoAssistStatusLabel.setBounds(sidechainModule.reduced(4, 0));

        auto meteringModule = nodeLayout.modules[4].reduced(16, 14);
        takeTop(meteringModule, compactHeight ? 42 : 52);
        const auto meterHeight = juce::jlimit(150, compactHeight ? 230 : 300, meteringModule.getHeight() / 2);
        layoutMeterRow(takeTop(meteringModule, meterHeight), inputMeter, outputMeter, sidechainMeter, reductionMeter);
        correlationLabel.setBounds(takeTop(meteringModule, compactHeight ? 32 : 38).reduced(4, 0));
        resonanceLearnButton.setBounds(takeTop(meteringModule, compactHeight ? 34 : 40).reduced(8, 4));
        takeTop(meteringModule, compactHeight ? 4 : 8);
        resonanceList.setBounds(meteringModule.reduced(4, 0));
        return;
    }

    auto header = takeTop(bounds, compactHeight ? 38 : 46);
    const auto titleWidth = clampedInt(header.getWidth() / 3, 220, 330);
    titleLabel.setBounds(header.removeFromLeft(titleWidth));
    auto modeArea = header.removeFromLeft(std::min(150, std::max(118, header.getWidth() / 4))).reduced(4, 0);
    layoutLabeledCombo(modeLabel, modeCombo, modeArea);

    auto autoArea = header.removeFromLeft(std::min(150, std::max(118, header.getWidth() / 4))).reduced(4, 0);
    layoutLabeledCombo(autoAssistLabel, autoAssistCombo, autoArea);

    currentRoleLabel.setBounds(header);

    bounds.reduce(theme::gap, compactHeight ? 4 : theme::gap);

    if (isDirectorMode())
    {
        auto directorTop = takeTop(bounds, compactHeight ? 38 : 44).reduced(4);
        linkGroupLabel.setBounds(directorTop.removeFromLeft(58));
        directorPreviousGroupButton.setBounds(directorTop.removeFromLeft(28).reduced(0, compactHeight ? 4 : 6));
        directorTop.removeFromLeft(std::min(4, directorTop.getWidth()));
        linkGroupEditor.setBounds(directorTop.removeFromLeft(52).reduced(0, compactHeight ? 4 : 6));
        directorTop.removeFromLeft(std::min(4, directorTop.getWidth()));
        directorNextGroupButton.setBounds(directorTop.removeFromLeft(28).reduced(0, compactHeight ? 4 : 6));
        directorStatusLabel.setBounds(directorTop);

        takeTop(bounds, innerGap);

        auto body = bounds.reduced(2);
        const auto statusWidth = clampedInt(static_cast<int> (static_cast<float> (body.getWidth()) * 0.26f), 230, 320);
        auto right = body.removeFromRight(statusWidth);
        body.removeFromRight(std::min(innerGap, body.getWidth()));
        const auto inspectorWidth = clampedInt(static_cast<int> (static_cast<float> (body.getWidth()) * 0.34f), 210, 285);
        auto inspector = body.removeFromRight(inspectorWidth);
        body.removeFromRight(std::min(innerGap, body.getWidth()));

        directorSceneView.setBounds(body);
        const auto directorButtonHeight = compactHeight ? 28 : 32;
        const auto directorFooterHeight = compactHeight ? 28 : 34;
        const auto directorGroupsHeight = compactHeight ? 68 : 86;
        const auto directorMemoryHeight = compactHeight ? 78 : 94;

        auto inspectorInner = inspector.reduced(6);
        directorSelectedTitleLabel.setBounds(takeTop(inspectorInner, compactHeight ? 22 : 26));
        directorSelectedDetailLabel.setBounds(takeTop(inspectorInner, compactHeight ? 62 : 76));
        takeTop(inspectorInner, innerGap);
        const auto remoteSliderHeight = compactHeight ? 36 : 42;
        const auto layoutRemoteSlider = [remoteSliderHeight](juce::Rectangle<int>& area, juce::Label& label, juce::Slider& slider)
        {
            auto row = takeTop(area, remoteSliderHeight);
            label.setBounds(takeTop(row, 14));
            slider.setBounds(row);
        };
        layoutRemoteSlider(inspectorInner, directorPanLabel, directorPanSlider);
        layoutRemoteSlider(inspectorInner, directorWidthLabel, directorWidthSlider);
        layoutRemoteSlider(inspectorInner, directorDepthLabel, directorDepthSlider);
        layoutRemoteSlider(inspectorInner, directorMotionLabel, directorMotionSlider);
        layoutRemoteSlider(inspectorInner, directorCleanUpLabel, directorCleanUpSlider);
        layoutRemoteSlider(inspectorInner, directorResonanceLabel, directorResonanceSlider);
        layoutRemoteSlider(inspectorInner, directorSidechainAmountLabel, directorSidechainAmountSlider);

        directorGroupsLabel.setBounds(takeTop(right, directorGroupsHeight).reduced(4));
        takeTop(right, innerGap);
        directorMemoryLabel.setBounds(takeTop(right, directorMemoryHeight).reduced(4));
        auto memoryButtons = takeTop(right, directorButtonHeight);
        directorLearnMixButton.setBounds(memoryButtons.removeFromLeft(std::max(1, memoryButtons.getWidth() / 2)).reduced(4));
        directorClearMemoryButton.setBounds(memoryButtons.reduced(4));
        takeTop(right, innerGap);
        const auto directorConflictHeight = std::max(130, right.getHeight() - directorButtonHeight - directorFooterHeight - innerGap);
        directorConflictLabel.setBounds(takeTop(right, directorConflictHeight).reduced(4));
        directorApplyTipButton.setBounds(takeTop(right, directorButtonHeight).reduced(4));
        directorFooterLabel.setBounds(right.reduced(4));
        return;
    }

    auto top = takeTop(bounds, compactHeight ? 52 : 58);
    const auto comboWidth = std::max(1, top.getWidth() / 4);
    layoutLabeledCombo(roleLabel, roleCombo, top.removeFromLeft(comboWidth).reduced(4));
    layoutLabeledCombo(safetyLabel, safetyCombo, top.removeFromLeft(comboWidth).reduced(4));
    layoutLabeledCombo(triggerLabel, triggerCombo, top.removeFromLeft(comboWidth).reduced(4));
    auto sidechainArea = top.reduced(4);
    takeTop(sidechainArea, 20);
    sidechainEnableButton.setBounds(sidechainArea.reduced(0, 2));

    takeTop(bounds, innerGap);

    auto columns = bounds;
    auto rightWidth = clampedInt(static_cast<int> (static_cast<float> (columns.getWidth()) * 0.22f), 170, 220);
    auto leftWidth = clampedInt(static_cast<int> (static_cast<float> (columns.getWidth()) * 0.32f), 230, 340);

    const auto minimumCenterWidth = 260;
    auto centerWidth = columns.getWidth() - leftWidth - rightWidth - innerGap * 2;
    if (centerWidth < minimumCenterWidth)
    {
        auto shortage = minimumCenterWidth - centerWidth;
        const auto leftReduction = std::min(shortage, std::max(0, leftWidth - 220));
        leftWidth -= leftReduction;
        shortage -= leftReduction;

        const auto rightReduction = std::min(shortage, std::max(0, rightWidth - 165));
        rightWidth -= rightReduction;
    }

    auto left = columns.removeFromLeft(leftWidth);
    columns.removeFromLeft(std::min(innerGap, columns.getWidth()));
    auto right = columns.removeFromRight(rightWidth);
    columns.removeFromRight(std::min(innerGap, columns.getWidth()));
    auto center = bounds;
    center = columns;

    stageView.setBounds(left.reduced(4));

    auto rightArea = right.reduced(2);
    const auto meterHeight = clampedInt(rightArea.getHeight() / 3, 86, compactHeight ? 150 : 240);
    layoutMeterRow(takeTop(rightArea, meterHeight), inputMeter, outputMeter, sidechainMeter, reductionMeter);
    correlationLabel.setBounds(takeTop(rightArea, compactHeight ? 30 : 38).reduced(4));
    layoutLabeledCombo(sidechainModeLabel, sidechainModeCombo, takeTop(rightArea, compactHeight ? 50 : 56).reduced(4));
    layoutLabeledCombo(sidechainListenLabel, sidechainListenCombo, takeTop(rightArea, compactHeight ? 50 : 56).reduced(4));
    sidechainStatusLabel.setBounds(takeTop(rightArea, compactHeight ? 26 : 32).reduced(4));

    auto linkRow = takeTop(rightArea, compactHeight ? 30 : 34).reduced(4);
    linkEnableButton.setBounds(linkRow.removeFromLeft(58));
    linkRow.removeFromLeft(std::min(4, linkRow.getWidth()));
    linkGroupLabel.setBounds(linkRow.removeFromLeft(48));
    linkGroupEditor.setBounds(linkRow.removeFromLeft(std::min(48, linkRow.getWidth())));
    layoutLabeledCombo(linkSourceRoleLabel, linkSourceRoleCombo, takeTop(rightArea, compactHeight ? 46 : 52).reduced(4));
    linkStatusLabel.setBounds(takeTop(rightArea, compactHeight ? 22 : 26).reduced(4));
    linkDetailLabel.setBounds(takeTop(rightArea, compactHeight ? 20 : 22).reduced(4));
    linkSuggestionLabel.setBounds(takeTop(rightArea, compactHeight ? 20 : 22).reduced(4));
    linkActionPreviewLabel.setBounds(takeTop(rightArea, compactHeight ? 20 : 22).reduced(4));
    linkApplyTipButton.setBounds(takeTop(rightArea, compactHeight ? 26 : 30).reduced(4));
    autoAssistStatusLabel.setBounds(takeTop(rightArea, compactHeight ? 20 : 22).reduced(4));

    resonanceLearnButton.setBounds(takeTop(rightArea, compactHeight ? 30 : 34).reduced(4));
    takeTop(rightArea, compactHeight ? 4 : innerGap);
    resonanceList.setBounds(rightArea.reduced(4));

    auto centerArea = center.reduced(2);
    const auto motionPresetHeight = compactHeight ? 42 : 48;
    const auto outputBlockHeight = clampedInt(centerArea.getHeight() / 5, 54, compactHeight ? 68 : 82);
    const auto rowHeight = std::max(96, (centerArea.getHeight() - outputBlockHeight - motionPresetHeight - innerGap * 3) / 2);

    layoutRotaryRow(
        takeTop(centerArea, rowHeight),
        widthLabel,
        widthSlider,
        depthLabel,
        depthSlider,
        motionLabel,
        motionSlider);

    takeTop(centerArea, innerGap);
    layoutLabeledCombo(motionPresetLabel, motionPresetCombo, takeTop(centerArea, motionPresetHeight).reduced(4));
    takeTop(centerArea, innerGap);

    std::array<RotaryRef, 4> bottomControls {{
        { &cleanUpLabel, &cleanUpSlider },
        { &resonanceLabel, &resonanceSlider },
        { &doubleLabel, &doubleSlider },
        { &sidechainAmountLabel, &sidechainAmountSlider }
    }};
    layoutRotaryGrid(takeTop(centerArea, rowHeight), bottomControls, 4);

    takeTop(centerArea, innerGap);
    outputLabel.setBounds(takeTop(centerArea, 22));
    outputSlider.setBounds(centerArea.reduced(20, 0));
}

void PluginEditor::timerCallback()
{
    const auto directorMode = isDirectorMode();
    if (directorMode != directorModeVisible)
    {
        directorModeVisible = directorMode;
        updateVisibleMode();
        resized();
        repaint();
    }

    if (directorMode)
    {
        syncLinkGroupEditorFromParameter();
        updateDirectorView();
        updateRoleLabel();
        return;
    }

    const auto& meterData = processor.getMeters();
    inputMeter.setLevels(
        meterData.inputRms.load(std::memory_order_relaxed),
        meterData.inputPeak.load(std::memory_order_relaxed));
    outputMeter.setLevels(
        meterData.outputRms.load(std::memory_order_relaxed),
        meterData.outputPeak.load(std::memory_order_relaxed));
    sidechainMeter.setLevels(
        meterData.sidechainRms.load(std::memory_order_relaxed),
        meterData.sidechainPeak.load(std::memory_order_relaxed));
    const auto gainReductionLevel = juce::jlimit(
        0.0f,
        1.0f,
        meterData.gainReductionDb.load(std::memory_order_relaxed) / 7.0f);
    reductionMeter.setLevels(gainReductionLevel, gainReductionLevel);

    const auto correlation = meterData.correlation.load(std::memory_order_relaxed);
    correlationLabel.setText("Correlation " + juce::String(correlation, 2), juce::dontSendNotification);
    correlationLabel.setColour(juce::Label::textColourId, correlation < 0.1f ? theme::warning : theme::text);

    const auto requestedMotion = apvts.getRawParameterValue(parameters::ids::motion)->load(std::memory_order_relaxed);
    const auto motionRate = apvts.getRawParameterValue(parameters::ids::motionRate)->load(std::memory_order_relaxed);
    const auto motionPreset = static_cast<int> (
        apvts.getRawParameterValue(parameters::ids::motionPreset)->load(std::memory_order_relaxed));
    stageMotionPhase += juce::MathConstants<float>::twoPi
        * motionPresetEffectiveRateHz(motionPresetFromIndex(motionPreset), motionRate)
        / 30.0f;
    if (stageMotionPhase >= juce::MathConstants<float>::twoPi)
        stageMotionPhase -= juce::MathConstants<float>::twoPi;

    StageView::State state;
    state.pan = meterData.stagePan.load(std::memory_order_relaxed);
    state.depth = meterData.stageDepth.load(std::memory_order_relaxed);
    state.width = meterData.stageWidth.load(std::memory_order_relaxed);
    state.motion = meterData.stageMotion.load(std::memory_order_relaxed);
    state.requestedMotion = requestedMotion;
    state.motionAllowed = meterData.stageMotionAllowed.load(std::memory_order_relaxed) != 0;
    state.motionPhase = stageMotionPhase;
    state.correlation = correlation;
    stageView.setState(state);
    resonanceList.setSnapshot(readResonanceSnapshot());

    const auto learnState = meterData.resonanceLearnState.load(std::memory_order_relaxed);
    const auto learnProgress = juce::jlimit(0.0f, 1.0f, meterData.resonanceLearnProgress.load(std::memory_order_relaxed));
    if (learnState == 1)
    {
        resonanceLearnButton.setButtonText("Learning " + juce::String(static_cast<int> (learnProgress * 100.0f)) + "%");
        resonanceLearnButton.setColour(juce::TextButton::buttonColourId, theme::accent.withAlpha(0.55f));
    }
    else if (learnState == 2)
    {
        resonanceLearnButton.setButtonText("Relearn");
        resonanceLearnButton.setColour(juce::TextButton::buttonColourId, theme::accentWarm.withAlpha(0.35f));
    }
    else
    {
        resonanceLearnButton.setButtonText("Learn");
        resonanceLearnButton.setColour(juce::TextButton::buttonColourId, theme::panelRaised);
    }

    const auto autoMode = autoAssistModeFromIndex(
        static_cast<int> (apvts.getRawParameterValue(parameters::ids::autoAssistMode)->load(std::memory_order_relaxed)));
    const auto autoState = meterData.autoAssistState.load(std::memory_order_relaxed);
    const auto autoProgress = juce::jlimit(0.0f, 1.0f, meterData.autoAssistProgress.load(std::memory_order_relaxed));
    const auto autoAction = LinkSuggestionEngine::actionFor(
        static_cast<LinkSuggestionKind> (meterData.autoAssistActionKind.load(std::memory_order_relaxed)));
    juce::String autoText = "Auto off";
    auto autoColour = theme::textMuted;

    if (autoMode == AutoAssistMode::Suggest)
    {
        autoText = "Auto suggest only";
        autoColour = theme::textMuted;
    }
    else if (autoMode == AutoAssistMode::Auto && autoState == 2)
    {
        autoText = "Auto analyzing " + juce::String(static_cast<int> (autoProgress * 100.0f)) + "%";
        autoColour = theme::accent;
    }
    else if (autoMode == AutoAssistMode::Auto && autoAction.available)
    {
        autoText = "Auto applied: " + juce::String(autoAction.previewMessage);
        autoColour = theme::accent;
    }
    else if (autoMode == AutoAssistMode::Auto && autoState == 3)
    {
        autoText = "Auto resonance rider";
        autoColour = theme::accent;
    }
    else if (autoMode == AutoAssistMode::Auto)
    {
        autoText = autoProgress > 0.0f
            ? "Auto arming " + juce::String(static_cast<int> (autoProgress * 100.0f)) + "%"
            : "Auto waiting group";
        autoColour = autoProgress > 0.0f ? theme::accentWarm : theme::warning;
    }

    autoAssistStatusLabel.setText(autoText, juce::dontSendNotification);
    autoAssistStatusLabel.setColour(juce::Label::textColourId, autoColour);

    const auto trigger = triggerModeFromIndex(static_cast<int> (apvts.getRawParameterValue(parameters::ids::triggerMode)->load(std::memory_order_relaxed)));
    const auto sidechainEnabled = apvts.getRawParameterValue(parameters::ids::sidechainEnabled)->load(std::memory_order_relaxed) >= 0.5f;
    const auto sidechainMode = sidechainConflictModeFromIndex(static_cast<int> (apvts.getRawParameterValue(parameters::ids::sidechainMode)->load(std::memory_order_relaxed)));
    const auto sidechainActive = meterData.sidechainEnvelope.load(std::memory_order_relaxed) > 0.0005f;
    const auto listenMode = sidechainListenModeFromIndex(static_cast<int> (apvts.getRawParameterValue(parameters::ids::sidechainListen)->load(std::memory_order_relaxed)));
    const auto sidechainListenSelected = sidechainListenCombo.getSelectedItemIndex() == 1;

    juce::String status = "Sidechain idle";
    auto statusColour = theme::textMuted;

    if (listenMode == SidechainListenMode::SidechainOnly || sidechainListenSelected)
    {
        status = sidechainActive ? "SC listen active" : "SC listen: no signal";
        statusColour = sidechainActive ? theme::accent : theme::warning;
    }
    else if (trigger == TriggerMode::ExternalSidechain && ! sidechainEnabled)
    {
        status = "External SC: disabled";
        statusColour = theme::warning;
    }
    else if (trigger == TriggerMode::ExternalSidechain && sidechainEnabled && sidechainMode == SidechainConflictMode::Off)
    {
        status = sidechainActive ? "SC mode off" : "SC off: no signal";
        statusColour = sidechainActive ? theme::textMuted : theme::warning;
    }
    else if (trigger == TriggerMode::ExternalSidechain && sidechainEnabled && ! sidechainActive)
    {
        status = "No SC signal";
        statusColour = theme::warning;
    }
    else if (trigger == TriggerMode::ExternalSidechain && sidechainEnabled)
    {
        status = "Sidechain dynamic EQ active";
        statusColour = theme::accent;
    }
    else if (trigger == TriggerMode::StageMindLink && ! sidechainEnabled)
    {
        status = "Link SC: disabled";
        statusColour = theme::warning;
    }
    else if (trigger == TriggerMode::StageMindLink && sidechainEnabled && sidechainMode == SidechainConflictMode::Off)
    {
        status = "Link SC mode off";
        statusColour = theme::warning;
    }
    else if (trigger == TriggerMode::StageMindLink && sidechainEnabled && sidechainActive)
    {
        status = "StageMind Link dynamic EQ active";
        statusColour = theme::accent;
    }
    else if (trigger == TriggerMode::StageMindLink && sidechainEnabled)
    {
        status = "Link SC waiting peer";
        statusColour = theme::warning;
    }

    sidechainStatusLabel.setText(status, juce::dontSendNotification);
    sidechainStatusLabel.setColour(juce::Label::textColourId, statusColour);

    const auto linkRequested = apvts.getRawParameterValue(parameters::ids::linkEnabled)->load(std::memory_order_relaxed) >= 0.5f;
    const auto linkGroup = static_cast<int> (apvts.getRawParameterValue(parameters::ids::linkGroup)->load(std::memory_order_relaxed));
    const auto linkInstanceId = meterData.linkInstanceId.load(std::memory_order_relaxed);
    const auto linkPeerCount = meterData.linkActivePeers.load(std::memory_order_relaxed);
    const auto linkNodeCount = meterData.linkNodeCount.load(std::memory_order_relaxed);
    const auto linkOfflineSuppressed = meterData.linkOfflineSuppressed.load(std::memory_order_relaxed) != 0;
    const auto linkPeerId = meterData.linkPeerId.load(std::memory_order_relaxed);
    const auto linkPeerRole = static_cast<TrackRole> (meterData.linkPeerRole.load(std::memory_order_relaxed));
    const auto linkPeerActivity = meterData.linkPeerActivity.load(std::memory_order_relaxed);
    const auto linkPeerCorrelation = meterData.linkPeerCorrelation.load(std::memory_order_relaxed);
    const auto linkPeerWidth = meterData.linkPeerWidth.load(std::memory_order_relaxed);
    const LinkSpectralBands currentBands {
        meterData.linkBandLow.load(std::memory_order_relaxed),
        meterData.linkBandLowMid.load(std::memory_order_relaxed),
        meterData.linkBandPresence.load(std::memory_order_relaxed),
        meterData.linkBandAir.load(std::memory_order_relaxed)
    };
    const LinkSpectralBands peerBands {
        meterData.linkPeerBandLow.load(std::memory_order_relaxed),
        meterData.linkPeerBandLowMid.load(std::memory_order_relaxed),
        meterData.linkPeerBandPresence.load(std::memory_order_relaxed),
        meterData.linkPeerBandAir.load(std::memory_order_relaxed)
    };
    const auto sourceRole = roleFromIndexWithUnknown(
        static_cast<int> (apvts.getRawParameterValue(parameters::ids::linkRole)->load(std::memory_order_relaxed)));
    const auto currentRole = roleFromSelectableIndex(
        static_cast<int> (apvts.getRawParameterValue(parameters::ids::role)->load(std::memory_order_relaxed)));
    const auto currentWidth = apvts.getRawParameterValue(parameters::ids::width)->load(std::memory_order_relaxed);
    const auto currentDepth = apvts.getRawParameterValue(parameters::ids::depth)->load(std::memory_order_relaxed);

    juce::String linkStatus = "Link off";
    juce::String linkDetail = "Node #" + juce::String(linkInstanceId) + " idle";
    juce::String linkSuggestionText = "Suggestion idle";
    juce::String linkPreviewText;
    auto linkColour = theme::textMuted;
    auto linkDetailColour = theme::textMuted;
    auto linkSuggestionColour = theme::textMuted;
    auto linkPreviewColour = theme::textMuted;

    if (linkRequested && linkGroup <= 0)
    {
        linkStatus = "Link needs group";
        linkDetail = "Use group 1-16";
        linkColour = theme::warning;
        linkDetailColour = theme::warning;
    }
    else if (linkRequested && linkOfflineSuppressed)
    {
        linkStatus = "Offline render";
        linkDetail = "Link publish disabled";
        linkColour = theme::warning;
        linkDetailColour = theme::warning;
    }
    else if (linkRequested && linkPeerCount <= 0)
    {
        linkStatus = "G" + juce::String(linkGroup) + ": " + juce::String(std::max(1, linkNodeCount)) + " node";
        linkDetail = sourceRole == TrackRole::Unknown
            ? "No peers in group"
            : "No " + shortLabelForRole(sourceRole) + " source";
        linkColour = theme::warning;
        linkDetailColour = theme::warning;
    }
    else if (linkRequested && sourceRole != TrackRole::Unknown && linkPeerId <= 0)
    {
        linkStatus = "G" + juce::String(linkGroup) + ": " + juce::String(linkNodeCount) + " nodes";
        linkDetail = "No " + shortLabelForRole(sourceRole) + " source";
        linkColour = theme::warning;
        linkDetailColour = theme::warning;
    }
    else if (linkRequested && linkPeerId > 0)
    {
        const auto peerBandValue = strongestBandValue(peerBands);
        linkStatus = "G" + juce::String(linkGroup)
            + ": " + juce::String(linkNodeCount) + " nodes -> #" + juce::String(linkPeerId);
        linkDetail = shortLabelForRole(linkPeerRole)
            + " " + juce::String(static_cast<int> (linkPeerActivity * 100.0f))
            + "% " + strongestBandName(peerBands) + " " + juce::String(static_cast<int> (peerBandValue * 100.0f))
            + "% corr " + juce::String(linkPeerCorrelation, 2);
        linkColour = theme::accent;
        linkDetailColour = linkPeerCorrelation < 0.1f ? theme::warning : theme::textMuted;
    }
    else if (linkRequested)
    {
        linkStatus = "G" + juce::String(linkGroup) + ": " + juce::String(linkNodeCount) + " nodes";
        linkDetail = "Source " + shortLabelForRole(sourceRole);
        linkColour = theme::accent;
    }

    linkStatusLabel.setText(linkStatus, juce::dontSendNotification);
    linkStatusLabel.setColour(juce::Label::textColourId, linkColour);
    linkDetailLabel.setText(linkDetail, juce::dontSendNotification);
    linkDetailLabel.setColour(juce::Label::textColourId, linkDetailColour);

    LinkSuggestionInput suggestionInput;
    suggestionInput.linkActive = linkRequested && linkGroup > 0 && ! linkOfflineSuppressed;
    suggestionInput.peerFound = linkPeerId > 0;
    suggestionInput.currentRole = currentRole;
    suggestionInput.peerRole = linkPeerRole;
    suggestionInput.currentWidth = currentWidth;
    suggestionInput.currentDepth = currentDepth;
    suggestionInput.currentCorrelation = correlation;
    suggestionInput.peerActivity = linkPeerActivity;
    suggestionInput.peerWidth = linkPeerWidth;
    suggestionInput.currentBands = currentBands;
    suggestionInput.peerBands = peerBands;

    const auto liveSuggestion = LinkSuggestionEngine::evaluate(suggestionInput);
    const auto canHoldSuggestion = suggestionInput.linkActive && suggestionInput.peerFound;

    if (liveSuggestion.hasSuggestion())
    {
        heldLinkSuggestion = liveSuggestion;
        heldLinkSuggestionFrames = linkSuggestionHoldFrames;
    }
    else if (! canHoldSuggestion)
    {
        heldLinkSuggestion = {};
        heldLinkSuggestionFrames = 0;
        linkResolvedFrames = 0;
    }
    else if (heldLinkSuggestionFrames > 0)
    {
        --heldLinkSuggestionFrames;
    }
    else
    {
        heldLinkSuggestion = {};
    }

    const auto suggestion = liveSuggestion.hasSuggestion() ? liveSuggestion : heldLinkSuggestion;
    currentLinkSuggestionKind = suggestion.kind;
    linkSuggestionText = suggestion.message;
    if (suggestion.hasSuggestion())
        linkSuggestionColour = suggestion.severity > 0.75f ? theme::warning : theme::accentWarm;

    const auto currentAction = LinkSuggestionEngine::actionFor(currentLinkSuggestionKind);
    const auto actionAlreadyApplied = isCurrentLinkActionApplied(currentAction);
    if (actionAlreadyApplied)
    {
        linkSuggestionText = suggestion.appliedMessage;
        linkPreviewText = "Resolved";
        linkPreviewColour = theme::accent;
        linkSuggestionColour = theme::accent;
    }
    else if (currentAction.available)
    {
        linkPreviewText = currentAction.previewMessage;

        if (currentAction.requiresManualSidechain
            && (trigger != TriggerMode::ExternalSidechain || ! sidechainEnabled))
        {
            linkPreviewText = "SC Mode only; routing manual";
            linkPreviewColour = theme::accentWarm;
        }
        else if (currentAction.requiresManualSidechain && ! sidechainActive)
        {
            linkPreviewText = "No SC signal yet";
            linkPreviewColour = theme::accentWarm;
        }
        else
        {
            linkPreviewColour = theme::textMuted;
        }
    }
    else if (linkResolvedFrames > 0)
    {
        --linkResolvedFrames;
        linkSuggestionText = "Resolved";
        linkPreviewText = "Resolved";
        linkSuggestionColour = theme::accent;
        linkPreviewColour = theme::accent;
    }

    linkSuggestionLabel.setText(linkSuggestionText, juce::dontSendNotification);
    linkSuggestionLabel.setColour(juce::Label::textColourId, linkSuggestionColour);
    linkSuggestionLabel.setTooltip(suggestion.reason);
    linkActionPreviewLabel.setText(linkPreviewText, juce::dontSendNotification);
    linkActionPreviewLabel.setColour(juce::Label::textColourId, linkPreviewColour);
    linkActionPreviewLabel.setTooltip(currentAction.available ? currentAction.previewMessage : "");
    linkApplyTipButton.setButtonText(actionAlreadyApplied ? "Applied" : suggestion.actionLabel);
    linkApplyTipButton.setTooltip(currentAction.available ? juce::String(suggestion.reason) : juce::String());
    linkApplyTipButton.setEnabled(currentAction.available && ! actionAlreadyApplied);

    syncLinkGroupEditorFromParameter();
    updateRoleLabel();
}

void PluginEditor::setupCombo(juce::ComboBox& combo, const juce::StringArray& items)
{
    combo.setColour(juce::ComboBox::backgroundColourId, theme::display);
    combo.setColour(juce::ComboBox::textColourId, theme::textOnDisplay);
    combo.setColour(juce::ComboBox::outlineColourId, theme::borderDark.withAlpha(0.55f));
    combo.setColour(juce::ComboBox::arrowColourId, theme::textOnDisplay);

    for (int i = 0; i < items.size(); ++i)
        combo.addItem(items[i], i + 1);

    addAndMakeVisible(combo);
}

void PluginEditor::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText)
{
    label.setText(labelText.toUpperCase(), juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, theme::textMuted);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);

    slider.setSliderStyle(labelText == "Output" ? juce::Slider::LinearHorizontal : juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, labelText == "Output" ? 78 : 58, 22);
    if (labelText == "Output")
    {
        slider.textFromValueFunction = [](double value)
        {
            return juce::String(value, 1) + " dB";
        };
    }
    else
    {
        slider.textFromValueFunction = [](double value)
        {
            return juce::String(static_cast<int> (std::round(value * 100.0))) + "%";
        };
    }
    slider.setColour(juce::Slider::rotarySliderFillColourId, theme::accent);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, theme::border);
    slider.setColour(juce::Slider::thumbColourId, theme::text.withAlpha(0.78f));
    slider.setColour(juce::Slider::trackColourId, theme::accent);
    slider.setColour(juce::Slider::backgroundColourId, theme::border.withAlpha(0.65f));
    slider.setColour(juce::Slider::textBoxTextColourId, theme::text);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, theme::panelInset);
    slider.setColour(juce::Slider::textBoxOutlineColourId, theme::border.withAlpha(0.72f));
    addAndMakeVisible(slider);
}

void PluginEditor::setupDirectorRemoteSlider(
    juce::Slider& slider,
    juce::Label& label,
    const juce::String& labelText,
    double minimum,
    double maximum)
{
    label.setText(labelText, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, theme::textMuted);
    label.setJustificationType(juce::Justification::centredLeft);
    label.setFont(juce::FontOptions { 11.0f });
    addAndMakeVisible(label);

    slider.setRange(minimum, maximum, 0.001);
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 62, 20);
    slider.setColour(juce::Slider::trackColourId, theme::border);
    slider.setColour(juce::Slider::backgroundColourId, theme::panelRaised);
    slider.setColour(juce::Slider::thumbColourId, theme::text.withAlpha(0.78f));
    slider.setColour(juce::Slider::textBoxTextColourId, theme::text);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, theme::panelInset);
    slider.setColour(juce::Slider::textBoxOutlineColourId, theme::border);
    addAndMakeVisible(slider);
}

void PluginEditor::setupButton(juce::TextButton& button)
{
    button.setColour(juce::TextButton::buttonColourId, theme::panelRaised);
    button.setColour(juce::TextButton::buttonOnColourId, theme::accent.withAlpha(0.5f));
    button.setColour(juce::TextButton::textColourOffId, theme::text);
    button.setColour(juce::TextButton::textColourOnId, theme::textOnDisplay);
    addAndMakeVisible(button);
}

void PluginEditor::setupLinkGroupEditor()
{
    linkGroupEditor.setInputRestrictions(2, "0123456789");
    linkGroupEditor.setJustification(juce::Justification::centred);
    linkGroupEditor.setSelectAllWhenFocused(true);
    linkGroupEditor.setColour(juce::TextEditor::backgroundColourId, theme::panelInset);
    linkGroupEditor.setColour(juce::TextEditor::textColourId, theme::text);
    linkGroupEditor.setColour(juce::TextEditor::outlineColourId, theme::borderDark.withAlpha(0.55f));
    linkGroupEditor.setColour(juce::TextEditor::focusedOutlineColourId, theme::accent);
    linkGroupEditor.setColour(juce::TextEditor::highlightColourId, theme::accent.withAlpha(0.35f));
    linkGroupEditor.setColour(juce::TextEditor::highlightedTextColourId, theme::text);
    linkGroupEditor.onTextChange = [this]
    {
        commitLinkGroupText();
    };
    linkGroupEditor.onReturnKey = [this]
    {
        commitLinkGroupText();
        linkGroupEditor.unfocusAllComponents();
    };
    linkGroupEditor.onFocusLost = [this]
    {
        commitLinkGroupText();
        syncLinkGroupEditorFromParameter();
    };
    addAndMakeVisible(linkGroupEditor);
}

void PluginEditor::layoutLabeledCombo(juce::Label& label, juce::ComboBox& combo, juce::Rectangle<int> bounds)
{
    if (! label.isVisible())
    {
        label.setColour(juce::Label::textColourId, theme::textMuted);
        addAndMakeVisible(label);
    }

    if (bounds.isEmpty())
        return;

    if (combo.getComponentID() == "headerCombo")
    {
        label.setFont(juce::FontOptions { 10.5f, juce::Font::bold });
        label.setColour(juce::Label::textColourId, juce::Colour { 0xff67eaf2 }.withAlpha(0.85f));
    }
    else
    {
        label.setFont(juce::FontOptions { combo.getComponentID() == "miniCombo" ? 8.5f : 11.0f });
    }

    label.setBounds(bounds.removeFromTop(20));
    combo.setBounds(bounds);
}

void PluginEditor::setLinkGroupValue(int group)
{
    const auto safeGroup = clampedLinkGroup(group);
    if (auto* parameter = apvts.getParameter(parameters::ids::linkGroup))
    {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(static_cast<float> (safeGroup)));
        parameter->endChangeGesture();
    }
}

void PluginEditor::stepDirectorGroup(int delta)
{
    if (delta == 0)
        return;

    const auto currentGroup = clampedLinkGroup(static_cast<int> (currentParameterValue(parameters::ids::linkGroup)));
    auto hasNonEmptyGroup = false;
    std::array<int, directorGroupCount + 1> groupCounts {};

    for (int group = 1; group <= directorGroupCount; ++group)
    {
        const auto snapshot = StageMindLinkRegistry::instance().readGroup(group);
        groupCounts[static_cast<size_t> (group)] = snapshot.count;
        hasNonEmptyGroup = hasNonEmptyGroup || snapshot.count > 0;
    }

    if (hasNonEmptyGroup)
    {
        const auto baseGroup = currentGroup <= 0 ? (delta > 0 ? 0 : directorGroupCount + 1) : currentGroup;
        for (int offset = 1; offset <= directorGroupCount; ++offset)
        {
            const auto candidate = wrappedDirectorGroup(baseGroup + delta * offset);
            if (groupCounts[static_cast<size_t> (candidate)] > 0)
            {
                setLinkGroupValue(candidate);
                return;
            }
        }
    }

    setLinkGroupValue(wrappedDirectorGroup((currentGroup <= 0 ? 1 : currentGroup) + delta));
}

void PluginEditor::commitLinkGroupText()
{
    if (updatingLinkGroupEditor)
        return;

    const auto text = linkGroupEditor.getText().trim();
    if (text.isEmpty())
        return;

    const auto group = clampedLinkGroup(text.getIntValue());
    setLinkGroupValue(group);

    if (juce::String(group) != text)
    {
        const juce::ScopedValueSetter<bool> scope(updatingLinkGroupEditor, true);
        linkGroupEditor.setText(juce::String(group), juce::dontSendNotification);
    }
}

void PluginEditor::syncLinkGroupEditorFromParameter()
{
    if (updatingLinkGroupEditor || linkGroupEditor.hasKeyboardFocus(true))
        return;

    if (const auto* value = apvts.getRawParameterValue(parameters::ids::linkGroup))
    {
        const auto group = clampedLinkGroup(static_cast<int> (value->load(std::memory_order_relaxed)));
        const auto text = juce::String(group);

        if (linkGroupEditor.getText() != text)
        {
            const juce::ScopedValueSetter<bool> scope(updatingLinkGroupEditor, true);
            linkGroupEditor.setText(text, juce::dontSendNotification);
        }
    }
}

void PluginEditor::applyCurrentLinkSuggestion()
{
    const auto action = LinkSuggestionEngine::actionFor(currentLinkSuggestionKind);
    if (! action.available || isCurrentLinkActionApplied(action))
        return;

    if (action.setWidth)
        applyParameterValue(parameters::ids::width, action.width);

    if (action.setDepth)
        applyParameterValue(parameters::ids::depth, action.depth);

    if (action.setSidechainMode)
        applyParameterValue(parameters::ids::sidechainMode, static_cast<float> (action.sidechainModeIndex));

    heldLinkSuggestion = {};
    heldLinkSuggestionFrames = 0;
    linkResolvedFrames = linkResolvedHoldFrames;
}

void PluginEditor::applyParameterValue(const char* parameterId, float value)
{
    if (approximatelyEqual(currentParameterValue(parameterId), value))
        return;

    if (auto* parameter = apvts.getParameter(parameterId))
    {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
        parameter->endChangeGesture();
    }
}

bool PluginEditor::isCurrentLinkActionApplied(const LinkSuggestionAction& action) const noexcept
{
    if (! action.available)
        return false;

    if (action.setWidth && ! approximatelyEqual(currentParameterValue(parameters::ids::width), action.width))
        return false;

    if (action.setDepth && ! approximatelyEqual(currentParameterValue(parameters::ids::depth), action.depth))
        return false;

    if (action.setSidechainMode
        && static_cast<int> (std::round(currentParameterValue(parameters::ids::sidechainMode))) != action.sidechainModeIndex)
    {
        return false;
    }

    return true;
}

float PluginEditor::currentParameterValue(const char* parameterId) const noexcept
{
    if (const auto* value = apvts.getRawParameterValue(parameterId))
        return value->load(std::memory_order_relaxed);

    return 0.0f;
}

bool PluginEditor::isDirectorMode() const noexcept
{
    return pluginModeFromIndex(static_cast<int> (currentParameterValue(parameters::ids::pluginMode))) == PluginMode::Director;
}

void PluginEditor::updateVisibleMode()
{
    const auto directorMode = isDirectorMode();
    setNodeControlsVisible(! directorMode);

    modeLabel.setVisible(true);
    modeCombo.setVisible(true);
    autoAssistLabel.setVisible(true);
    autoAssistCombo.setVisible(true);
    titleLabel.setVisible(true);
    currentRoleLabel.setVisible(true);
    linkGroupLabel.setVisible(true);
    linkGroupEditor.setVisible(true);

    directorSceneView.setVisible(directorMode);
    directorStatusLabel.setVisible(directorMode);
    directorGroupsLabel.setVisible(directorMode);
    directorMemoryLabel.setVisible(directorMode);
    directorSelectedTitleLabel.setVisible(directorMode);
    directorSelectedDetailLabel.setVisible(directorMode);
    directorPanLabel.setVisible(directorMode);
    directorWidthLabel.setVisible(directorMode);
    directorDepthLabel.setVisible(directorMode);
    directorMotionLabel.setVisible(directorMode);
    directorCleanUpLabel.setVisible(directorMode);
    directorResonanceLabel.setVisible(directorMode);
    directorSidechainAmountLabel.setVisible(directorMode);
    directorPanSlider.setVisible(directorMode);
    directorWidthSlider.setVisible(directorMode);
    directorDepthSlider.setVisible(directorMode);
    directorMotionSlider.setVisible(directorMode);
    directorCleanUpSlider.setVisible(directorMode);
    directorResonanceSlider.setVisible(directorMode);
    directorSidechainAmountSlider.setVisible(directorMode);
    directorConflictLabel.setVisible(directorMode);
    directorApplyTipButton.setVisible(directorMode);
    directorLearnMixButton.setVisible(directorMode);
    directorClearMemoryButton.setVisible(directorMode);
    directorPreviousGroupButton.setVisible(directorMode);
    directorNextGroupButton.setVisible(directorMode);
    directorFooterLabel.setVisible(directorMode);
}

void PluginEditor::setNodeControlsVisible(bool shouldBeVisible)
{
    roleLabel.setVisible(shouldBeVisible);
    safetyLabel.setVisible(shouldBeVisible);
    triggerLabel.setVisible(shouldBeVisible);
    sidechainModeLabel.setVisible(shouldBeVisible);
    sidechainListenLabel.setVisible(shouldBeVisible);
    linkSourceRoleLabel.setVisible(shouldBeVisible);

    roleCombo.setVisible(shouldBeVisible);
    safetyCombo.setVisible(shouldBeVisible);
    triggerCombo.setVisible(shouldBeVisible);
    sidechainModeCombo.setVisible(shouldBeVisible);
    sidechainListenCombo.setVisible(shouldBeVisible);
    linkSourceRoleCombo.setVisible(shouldBeVisible);

    sidechainEnableButton.setVisible(shouldBeVisible);
    resonanceLearnButton.setVisible(shouldBeVisible);
    linkEnableButton.setVisible(shouldBeVisible);
    linkApplyTipButton.setVisible(shouldBeVisible);

    widthLabel.setVisible(shouldBeVisible);
    depthLabel.setVisible(shouldBeVisible);
    motionLabel.setVisible(shouldBeVisible);
    motionPresetLabel.setVisible(shouldBeVisible);
    cleanUpLabel.setVisible(shouldBeVisible);
    resonanceLabel.setVisible(shouldBeVisible);
    doubleLabel.setVisible(shouldBeVisible);
    outputLabel.setVisible(shouldBeVisible);
    sidechainAmountLabel.setVisible(shouldBeVisible);

    widthSlider.setVisible(shouldBeVisible);
    depthSlider.setVisible(shouldBeVisible);
    motionSlider.setVisible(shouldBeVisible);
    motionPresetCombo.setVisible(shouldBeVisible);
    cleanUpSlider.setVisible(shouldBeVisible);
    resonanceSlider.setVisible(shouldBeVisible);
    doubleSlider.setVisible(shouldBeVisible);
    outputSlider.setVisible(shouldBeVisible);
    sidechainAmountSlider.setVisible(shouldBeVisible);

    stageView.setVisible(shouldBeVisible);
    inputMeter.setVisible(shouldBeVisible);
    outputMeter.setVisible(shouldBeVisible);
    sidechainMeter.setVisible(shouldBeVisible);
    reductionMeter.setVisible(shouldBeVisible);
    resonanceList.setVisible(shouldBeVisible);
    correlationLabel.setVisible(shouldBeVisible);
    sidechainStatusLabel.setVisible(shouldBeVisible);
    linkStatusLabel.setVisible(shouldBeVisible);
    linkDetailLabel.setVisible(shouldBeVisible);
    linkSuggestionLabel.setVisible(shouldBeVisible);
    linkActionPreviewLabel.setVisible(shouldBeVisible);
    autoAssistStatusLabel.setVisible(shouldBeVisible);
}

void PluginEditor::updateDirectorView()
{
    const auto group = clampedLinkGroup(static_cast<int> (currentParameterValue(parameters::ids::linkGroup)));
    auto hasAnyLinkedGroup = false;
    std::array<int, directorGroupCount + 1> groupCounts {};
    std::array<int, directorGroupCount + 1> activeCounts {};

    for (int groupIndex = 1; groupIndex <= directorGroupCount; ++groupIndex)
    {
        const auto groupSnapshot = StageMindLinkRegistry::instance().readGroup(groupIndex);
        groupCounts[static_cast<size_t> (groupIndex)] = groupSnapshot.count;

        for (int nodeIndex = 0; nodeIndex < groupSnapshot.count; ++nodeIndex)
            if (groupSnapshot.peers[static_cast<size_t> (nodeIndex)].activity > 0.08f)
                ++activeCounts[static_cast<size_t> (groupIndex)];

        hasAnyLinkedGroup = hasAnyLinkedGroup || groupSnapshot.count > 0;
    }

    directorGroupsLabel.setText(makeDirectorGroupOverview(group, groupCounts, activeCounts), juce::dontSendNotification);
    directorGroupsLabel.setColour(juce::Label::textColourId, hasAnyLinkedGroup ? theme::textMuted : theme::warning);

    const auto rideMemory = processor.getRideMemorySnapshot();
    const auto rideTimelineMemory = processor.getRideTimelineSnapshot();
    const auto transport = processor.getTransportSnapshot();
    auto resolvedMemoryCount = 0;
    for (const auto& event : rideMemory.events)
        if (event.used && event.resolved)
            ++resolvedMemoryCount;

    auto resolvedTimelineCount = 0;
    auto groupTimelineCount = 0;
    auto nearbyTimelineCount = 0;
    const auto* nearestTimelineEvent = static_cast<const RideTimelineEvent*> (nullptr);
    auto nearestTimelineDistance = 1.0e9;
    const auto* latestTimelineEvent = static_cast<const RideTimelineEvent*> (nullptr);
    for (const auto& event : rideTimelineMemory.events)
    {
        if (! event.used || event.group != group)
            continue;

        ++groupTimelineCount;

        if (event.resolved)
            ++resolvedTimelineCount;

        if (transport.valid && event.contains(transport.ppqPosition, rideTimelineMergeWindowPpq))
            ++nearbyTimelineCount;

        if (transport.valid)
        {
            const auto distance = std::abs(event.lastSeenPpq - transport.ppqPosition);
            if (distance < nearestTimelineDistance)
            {
                nearestTimelineDistance = distance;
                nearestTimelineEvent = &event;
            }
        }

        if (latestTimelineEvent == nullptr || event.lastSeenPpq > latestTimelineEvent->lastSeenPpq)
            latestTimelineEvent = &event;
    }

    const auto autoMode = autoAssistModeFromIndex(static_cast<int> (currentParameterValue(parameters::ids::autoAssistMode)));
    juce::String memoryText;
    if (rideMemory.learning || rideTimelineMemory.learning)
        memoryText = "Memory learning\n";
    else if (autoMode == AutoAssistMode::Auto)
        memoryText = "Memory auto\n";
    else
        memoryText = "Memory idle\n";

    memoryText += juce::String(rideMemory.count)
        + " events, " + juce::String(resolvedMemoryCount) + " resolved";
    memoryText += "\nTimeline " + juce::String(groupTimelineCount)
        + " / " + juce::String(resolvedTimelineCount) + " resolved";
    memoryText += transport.valid
        ? "\n" + ppqText(transport.ppqPosition)
            + (transport.playing ? " play" : " stop")
            + ", " + juce::String(nearbyTimelineCount) + " nearby"
        : "\nPPQ unavailable";

    if (const auto* eventToShow = nearestTimelineEvent != nullptr ? nearestTimelineEvent : latestTimelineEvent)
        memoryText += "\n" + timelineEventText(*eventToShow);
    directorMemoryLabel.setText(memoryText, juce::dontSendNotification);
    directorMemoryLabel.setColour(
        juce::Label::textColourId,
        rideMemory.count > 0 || rideTimelineMemory.count > 0 || rideMemory.learning || rideTimelineMemory.learning
            ? theme::accent
            : theme::textMuted);
    directorLearnMixButton.setButtonText(rideMemory.learning ? "Learning" : "Learn Mix");
    directorClearMemoryButton.setEnabled(
        rideMemory.count > 0 || rideTimelineMemory.count > 0 || rideMemory.learning || rideTimelineMemory.learning);

    if (group != directorConflictGroup)
    {
        directorConflictGroup = group;
        resetDirectorConflictMemory();
        directorSelectedNodeId = invalidLinkInstanceId;
        directorLastMovedNodeId = invalidLinkInstanceId;
        directorSceneView.setSelectedNode(invalidLinkInstanceId);
    }

    if (group <= 0)
    {
        directorSceneView.setSnapshot({});
        directorSceneView.setConnections({}, 0);
        directorSceneView.setSelectedNode(invalidLinkInstanceId);
        directorSelectedNodeId = invalidLinkInstanceId;
        directorStatusLabel.setText("Director: choose Group 1-16", juce::dontSendNotification);
        directorStatusLabel.setColour(juce::Label::textColourId, theme::warning);
        directorConflictLabel.setText("No group selected", juce::dontSendNotification);
        directorConflictLabel.setColour(juce::Label::textColourId, theme::warning);
        syncDirectorSelectedControls(nullptr);
        directorApplyTipButton.setButtonText("Apply Tip");
        directorApplyTipButton.setEnabled(false);
        directorFooterLabel.setText("Select Group 1-16 to read linked Nodes.", juce::dontSendNotification);
        directorFooterLabel.setColour(juce::Label::textColourId, theme::textMuted);
        return;
    }

    const auto snapshot = StageMindLinkRegistry::instance().readGroup(group);
    directorSceneView.setSnapshot(snapshot);

    LinkPeerSnapshot selectedNode;
    auto selectedNodeFound = false;
    if (directorSelectedNodeId != invalidLinkInstanceId)
    {
        for (int index = 0; index < snapshot.count; ++index)
        {
            const auto& node = snapshot.peers[static_cast<size_t> (index)];
            if (node.instanceId == directorSelectedNodeId)
            {
                selectedNode = node;
                selectedNodeFound = true;
                break;
            }
        }

        if (! selectedNodeFound)
        {
            directorSelectedNodeId = invalidLinkInstanceId;
            directorLastMovedNodeId = invalidLinkInstanceId;
        }
    }
    directorSceneView.setSelectedNode(directorSelectedNodeId);
    syncDirectorSelectedControls(selectedNodeFound ? &selectedNode : nullptr);

    auto activeNodes = 0;
    for (int index = 0; index < snapshot.count; ++index)
    {
        if (snapshot.peers[static_cast<size_t> (index)].activity > 0.08f)
            ++activeNodes;
    }

    directorStatusLabel.setText(
        "G" + juce::String(group) + ": " + juce::String(snapshot.count)
            + " nodes, " + juce::String(activeNodes) + " active",
        juce::dontSendNotification);
    directorStatusLabel.setColour(juce::Label::textColourId, snapshot.count > 0 ? theme::accent : theme::warning);

    for (auto& record : directorConflictRecords)
        record.seenThisFrame = false;

    for (int current = 0; current < snapshot.count; ++current)
    {
        for (int peer = 0; peer < snapshot.count; ++peer)
        {
            if (current == peer)
                continue;

            const auto& currentNode = snapshot.peers[static_cast<size_t> (current)];
            const auto& peerNode = snapshot.peers[static_cast<size_t> (peer)];

            LinkSuggestionInput input;
            input.linkActive = true;
            input.peerFound = true;
            input.currentRole = static_cast<TrackRole> (currentNode.role);
            input.peerRole = static_cast<TrackRole> (peerNode.role);
            input.currentWidth = currentNode.width;
            input.currentDepth = currentNode.depth;
            input.currentCorrelation = currentNode.correlation;
            input.peerActivity = peerNode.activity;
            input.peerWidth = peerNode.width;
            input.currentBands = currentNode.bands;
            input.peerBands = peerNode.bands;

            const auto suggestion = LinkSuggestionEngine::evaluate(input);
            if (! suggestion.hasSuggestion())
                continue;

            const auto action = LinkSuggestionEngine::actionFor(suggestion.kind);
            const auto actionAlreadyApplied = isSnapshotLinkActionApplied(currentNode, action);

            auto* recordToUpdate = static_cast<DirectorConflictRecord*> (nullptr);
            for (auto& record : directorConflictRecords)
            {
                if (record.used
                    && record.currentRole == input.currentRole
                    && record.peerRole == input.peerRole
                    && record.kind == suggestion.kind
                    && record.targetId == currentNode.instanceId
                    && record.peerId == peerNode.instanceId)
                {
                    recordToUpdate = &record;
                    break;
                }
            }

            if (recordToUpdate == nullptr)
            {
                for (auto& record : directorConflictRecords)
                {
                    if (! record.used)
                    {
                        recordToUpdate = &record;
                        break;
                    }
                }
            }

            if (recordToUpdate == nullptr)
            {
                for (auto& record : directorConflictRecords)
                {
                    if (! record.active && record.missingFrames >= directorConflictResolveFrames)
                    {
                        recordToUpdate = &record;
                        break;
                    }
                }
            }

            if (recordToUpdate == nullptr)
            {
                recordToUpdate = &directorConflictRecords[static_cast<size_t> (directorConflictWriteIndex)];
                directorConflictWriteIndex = (directorConflictWriteIndex + 1) % static_cast<int> (directorConflictRecords.size());
            }

            recordToUpdate->used = true;
            recordToUpdate->active = ! actionAlreadyApplied;
            recordToUpdate->seenThisFrame = true;
            recordToUpdate->missingFrames = actionAlreadyApplied ? directorConflictResolveFrames : 0;
            recordToUpdate->currentRole = input.currentRole;
            recordToUpdate->peerRole = input.peerRole;
            recordToUpdate->kind = suggestion.kind;
            recordToUpdate->targetId = currentNode.instanceId;
            recordToUpdate->peerId = peerNode.instanceId;
            recordToUpdate->targetAutoAssistMode = currentNode.autoAssistMode;
            recordToUpdate->summary = suggestionSummary(suggestion);
            recordToUpdate->band = strongestBandName(peerNode.bands);
            recordToUpdate->action = correctionTextForSuggestion(suggestion);
        }
    }

    for (auto& record : directorConflictRecords)
    {
        if (! record.used || record.seenThisFrame)
            continue;

        ++record.missingFrames;
        if (record.missingFrames >= directorConflictResolveFrames)
            record.active = false;
    }

    std::array<DirectorSceneConnection, maxDirectorSceneConnections> sceneConnections {};
    auto sceneConnectionCount = 0;
    for (const auto& record : directorConflictRecords)
    {
        if (! record.used || ! record.active || sceneConnectionCount >= maxDirectorSceneConnections)
            continue;

        auto& connection = sceneConnections[static_cast<size_t> (sceneConnectionCount)];
        connection.active = true;
        connection.targetId = record.targetId;
        connection.peerId = record.peerId;
        connection.label = sceneLabelForSuggestion(record.kind);
        ++sceneConnectionCount;
    }
    directorSceneView.setConnections(sceneConnections, sceneConnectionCount);

    juce::String conflicts;
    auto activeConflictCount = 0;
    auto displayedRows = 0;

    const auto appendRecord = [&conflicts, &displayedRows, &activeConflictCount](const DirectorConflictRecord& record)
    {
        if (displayedRows > 0)
            conflicts += "\n";

        conflicts += shortLabelForRole(record.currentRole)
            + " #" + juce::String(record.targetId)
            + " <- " + shortLabelForRole(record.peerRole)
            + " #" + juce::String(record.peerId)
            + ": " + record.summary
            + " / " + record.band
            + " | ";

        if (record.active)
        {
            conflicts += "Tip: ";
            conflicts += record.action;
            ++activeConflictCount;
        }
        else
        {
            conflicts += "resolved";
        }

        ++displayedRows;
    };

    for (const auto& record : directorConflictRecords)
        if (record.used && record.active && displayedRows < 10)
            appendRecord(record);

    for (const auto& record : directorConflictRecords)
        if (record.used && ! record.active && displayedRows < 10)
            appendRecord(record);

    if (conflicts.isEmpty())
        conflicts = snapshot.count <= 0 ? "No linked Nodes in this group" : "No active conflicts";

    directorConflictLabel.setText(conflicts, juce::dontSendNotification);
    directorConflictLabel.setColour(
        juce::Label::textColourId,
        activeConflictCount > 0 ? theme::accentWarm : theme::textMuted);

    if (const auto* activeConflict = findFirstActiveDirectorConflict())
    {
        directorApplyTipButton.setButtonText("Apply to #" + juce::String(activeConflict->targetId));
        directorApplyTipButton.setTooltip(activeConflict->action);
        directorApplyTipButton.setEnabled(LinkSuggestionEngine::actionFor(activeConflict->kind).available);
    }
    else
    {
        directorApplyTipButton.setButtonText("Apply Tip");
        directorApplyTipButton.setTooltip({});
        directorApplyTipButton.setEnabled(false);
    }

    if (directorCommandStatusFrames > 0)
    {
        --directorCommandStatusFrames;
        directorFooterLabel.setText(directorCommandStatus, juce::dontSendNotification);
        directorFooterLabel.setColour(juce::Label::textColourId, theme::accent);
    }
    else
    {
        const auto footerAutoMode = autoAssistModeFromIndex(static_cast<int> (currentParameterValue(parameters::ids::autoAssistMode)));
        auto footer = footerAutoMode == AutoAssistMode::Auto
            ? juce::String("Director Auto: guarded commands run from the processor.")
            : juce::String("User-approved commands only. Routing stays manual.");

        if (selectedNodeFound)
            footer = "Drag selected Node: Pan/Depth remote control.";
        else if (snapshot.count > 0)
            footer = "Click a Node to inspect. Drag it to move Pan/Depth.";

        directorFooterLabel.setText(footer, juce::dontSendNotification);
        directorFooterLabel.setColour(juce::Label::textColourId, theme::textMuted);
    }
}

void PluginEditor::selectDirectorNode(std::uint32_t instanceId)
{
    if (instanceId == invalidLinkInstanceId)
        return;

    directorSelectedNodeId = instanceId;
    directorLastMovedNodeId = invalidLinkInstanceId;
    directorSceneView.setSelectedNode(instanceId);
    directorCommandStatus = "Selected #" + juce::String(instanceId);
    directorCommandStatusFrames = 45;
}

void PluginEditor::syncDirectorSelectedControls(const LinkPeerSnapshot* selectedNode)
{
    const juce::ScopedValueSetter<bool> scope(updatingDirectorSelectedControls, true);
    const auto hasSelection = selectedNode != nullptr && selectedNode->found;

    directorSelectedTitleLabel.setText(
        hasSelection ? "Node #" + juce::String(selectedNode->instanceId) : "Selected Node",
        juce::dontSendNotification);
    directorSelectedDetailLabel.setText(
        hasSelection
            ? selectedDirectorNodeText(*selectedNode)
            : juce::String("Click a node in Stage View"),
        juce::dontSendNotification);
    directorSelectedDetailLabel.setColour(juce::Label::textColourId, hasSelection ? theme::textMuted : theme::warning);

    directorPanSlider.setEnabled(hasSelection);
    directorWidthSlider.setEnabled(hasSelection);
    directorDepthSlider.setEnabled(hasSelection);
    directorMotionSlider.setEnabled(hasSelection);
    directorCleanUpSlider.setEnabled(hasSelection);
    directorResonanceSlider.setEnabled(hasSelection);
    directorSidechainAmountSlider.setEnabled(hasSelection);

    if (! hasSelection)
        return;

    directorPanSlider.setValue(selectedNode->pan, juce::dontSendNotification);
    directorWidthSlider.setValue(selectedNode->width, juce::dontSendNotification);
    directorDepthSlider.setValue(selectedNode->depth, juce::dontSendNotification);
    directorMotionSlider.setValue(selectedNode->motion, juce::dontSendNotification);
    directorCleanUpSlider.setValue(selectedNode->cleanUp, juce::dontSendNotification);
    directorResonanceSlider.setValue(selectedNode->resonance, juce::dontSendNotification);
    directorSidechainAmountSlider.setValue(selectedNode->sidechainAmount, juce::dontSendNotification);
}

void PluginEditor::sendDirectorSelectedControlCommand()
{
    if (updatingDirectorSelectedControls || directorSelectedNodeId == invalidLinkInstanceId)
        return;

    LinkCommand command;
    command.sourceInstanceId = processor.getLinkInstanceId();
    command.setPan = true;
    command.setWidth = true;
    command.setDepth = true;
    command.setMotion = true;
    command.setCleanUp = true;
    command.setResonance = true;
    command.setSidechainAmount = true;
    command.pan = static_cast<float> (directorPanSlider.getValue());
    command.width = static_cast<float> (directorWidthSlider.getValue());
    command.depth = static_cast<float> (directorDepthSlider.getValue());
    command.motion = static_cast<float> (directorMotionSlider.getValue());
    command.cleanUp = static_cast<float> (directorCleanUpSlider.getValue());
    command.resonance = static_cast<float> (directorResonanceSlider.getValue());
    command.sidechainAmount = static_cast<float> (directorSidechainAmountSlider.getValue());

    const auto sent = StageMindLinkRegistry::instance().submitCommand(directorSelectedNodeId, command);
    directorCommandStatus = sent
        ? "Remote controls sent to #" + juce::String(directorSelectedNodeId)
        : "Target Node is not available";
    directorCommandStatusFrames = sent ? 20 : 60;
}

void PluginEditor::sendDirectorSpatialCommand(std::uint32_t targetId, float pan, float depth)
{
    if (targetId == invalidLinkInstanceId)
        return;

    pan = juce::jlimit(-1.0f, 1.0f, pan);
    depth = juce::jlimit(0.0f, 1.0f, depth);

    if (directorLastMovedNodeId == targetId
        && std::abs(directorLastSentPan - pan) < 0.006f
        && std::abs(directorLastSentDepth - depth) < 0.006f)
    {
        return;
    }

    LinkCommand command;
    command.sourceInstanceId = processor.getLinkInstanceId();
    command.setPan = true;
    command.setDepth = true;
    command.pan = pan;
    command.depth = depth;

    const auto sent = StageMindLinkRegistry::instance().submitCommand(targetId, command);
    if (sent)
    {
        directorSelectedNodeId = targetId;
        directorLastMovedNodeId = targetId;
        directorLastSentPan = pan;
        directorLastSentDepth = depth;
        directorCommandStatus = "Moved #" + juce::String(targetId)
            + " Pan " + juce::String(pan, 2)
            + " Depth " + juce::String(static_cast<int> (depth * 100.0f)) + "%";
        directorCommandStatusFrames = 24;
    }
    else
    {
        directorCommandStatus = "Target Node is not available";
        directorCommandStatusFrames = 60;
    }
}

void PluginEditor::applyDirectorTip()
{
    const auto* conflict = findFirstActiveDirectorConflict();
    if (conflict == nullptr)
        return;

    LinkCommand command;
    command.sourceInstanceId = processor.getLinkInstanceId();
    command.peerInstanceId = conflict->peerId;
    command.actionKind = static_cast<int> (conflict->kind);

    const auto sent = StageMindLinkRegistry::instance().submitCommand(conflict->targetId, command);
    directorCommandStatus = sent
        ? "Sent " + conflict->action + " to #" + juce::String(conflict->targetId)
        : "Target Node is not available";
    directorCommandStatusFrames = 90;
}

const PluginEditor::DirectorConflictRecord* PluginEditor::findFirstActiveDirectorConflict() const noexcept
{
    for (const auto& record : directorConflictRecords)
        if (record.used && record.active && LinkSuggestionEngine::actionFor(record.kind).available)
            return &record;

    return nullptr;
}

void PluginEditor::resetDirectorConflictMemory() noexcept
{
    for (auto& record : directorConflictRecords)
        record = {};

    directorConflictWriteIndex = 0;
    directorCommandStatusFrames = 0;
    directorCommandStatus = {};
}

void PluginEditor::updateRoleLabel()
{
    if (isDirectorMode())
    {
        currentRoleLabel.setText("Mode: Director", juce::dontSendNotification);
        return;
    }

    const auto index = static_cast<int> (apvts.getRawParameterValue(parameters::ids::role)->load(std::memory_order_relaxed));
    currentRoleLabel.setText("Current role: " + juce::String(labelForRole(roleFromSelectableIndex(index))), juce::dontSendNotification);
}

ResonanceSnapshot PluginEditor::readResonanceSnapshot() const noexcept
{
    const auto& meterData = processor.getMeters();
    ResonanceSnapshot snapshot;
    const auto count = juce::jlimit(0, static_cast<int> (maxResonancePeaks), meterData.resonances.peakCount.load(std::memory_order_relaxed));
    snapshot.peakCount = static_cast<uint8_t> (count);

    for (int i = 0; i < count; ++i)
    {
        const auto index = static_cast<size_t> (i);
        auto& peak = snapshot.peaks[index];
        peak.frequencyHz = meterData.resonances.frequencyHz[index].load(std::memory_order_relaxed);
        peak.severity = meterData.resonances.severity[index].load(std::memory_order_relaxed);
        peak.suggestedReductionDb = meterData.resonances.reductionDb[index].load(std::memory_order_relaxed);
        peak.suggestedQ = meterData.resonances.q[index].load(std::memory_order_relaxed);
    }

    return snapshot;
}
} // namespace stagemind
