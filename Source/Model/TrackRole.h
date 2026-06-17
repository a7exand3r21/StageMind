#pragma once

#include <array>
#include <juce_core/juce_core.h>

namespace stagemind
{
enum class TrackRole
{
    Unknown = 0,

    LeadVocal,
    BackingVocal,

    Kick,
    Bass,
    Snare,
    HiHat,
    Percussion,

    RhythmGuitarSingle,
    RhythmGuitarPairLeft,
    RhythmGuitarPairRight,
    LeadGuitar,

    Pad,
    Piano,
    SynthLead,
    SynthBass,

    FX,
    Atmosphere,

    SunoVocal,
    SunoInstrumental,
    SunoDrums,
    SunoBass,
    SunoGuitar,
    SunoSynthPad,
    SunoPercussion,
    SunoFX
};

struct RoleChoice
{
    TrackRole role;
    const char* label;
};

inline constexpr std::array<RoleChoice, 25> selectableRoleChoices {{
    { TrackRole::LeadVocal, "Lead Vocal" },
    { TrackRole::BackingVocal, "Backing Vocal" },
    { TrackRole::Kick, "Kick" },
    { TrackRole::Bass, "Bass" },
    { TrackRole::Snare, "Snare" },
    { TrackRole::HiHat, "HiHat" },
    { TrackRole::Percussion, "Percussion" },
    { TrackRole::RhythmGuitarSingle, "Rhythm Guitar Single" },
    { TrackRole::RhythmGuitarPairLeft, "Rhythm Guitar Pair Left" },
    { TrackRole::RhythmGuitarPairRight, "Rhythm Guitar Pair Right" },
    { TrackRole::LeadGuitar, "Lead Guitar" },
    { TrackRole::Pad, "Pad" },
    { TrackRole::Piano, "Piano" },
    { TrackRole::SynthLead, "Synth Lead" },
    { TrackRole::SynthBass, "Synth Bass" },
    { TrackRole::FX, "FX" },
    { TrackRole::Atmosphere, "Atmosphere" },
    { TrackRole::SunoVocal, "Suno Vocal" },
    { TrackRole::SunoInstrumental, "Suno Instrumental" },
    { TrackRole::SunoDrums, "Suno Drums" },
    { TrackRole::SunoBass, "Suno Bass" },
    { TrackRole::SunoGuitar, "Suno Guitar" },
    { TrackRole::SunoSynthPad, "Suno Synth Pad" },
    { TrackRole::SunoPercussion, "Suno Percussion" },
    { TrackRole::SunoFX, "Suno FX" },
}};

inline juce::StringArray makeSelectableRoleNames()
{
    juce::StringArray names;
    for (const auto& choice : selectableRoleChoices)
        names.add(choice.label);
    return names;
}

inline juce::StringArray makeRoleNamesWithUnknown()
{
    auto names = juce::StringArray { "Unknown" };
    names.addArray(makeSelectableRoleNames());
    return names;
}

inline TrackRole roleFromSelectableIndex(int index) noexcept
{
    if (index < 0 || index >= static_cast<int> (selectableRoleChoices.size()))
        return TrackRole::Unknown;

    return selectableRoleChoices[static_cast<size_t> (index)].role;
}

inline int selectableIndexForRole(TrackRole role) noexcept
{
    for (size_t i = 0; i < selectableRoleChoices.size(); ++i)
        if (selectableRoleChoices[i].role == role)
            return static_cast<int> (i);

    return 0;
}

inline TrackRole roleFromIndexWithUnknown(int index) noexcept
{
    if (index == 0)
        return TrackRole::Unknown;

    return roleFromSelectableIndex(index - 1);
}

inline const char* labelForRole(TrackRole role) noexcept
{
    if (role == TrackRole::Unknown)
        return "Unknown";

    for (const auto& choice : selectableRoleChoices)
        if (choice.role == role)
            return choice.label;

    return "Unknown";
}
} // namespace stagemind
