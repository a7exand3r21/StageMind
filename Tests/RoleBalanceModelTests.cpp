#include "../Source/Model/RoleBalanceModel.h"

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

void quietVocalGetsPriorityBoost()
{
    std::array<stagemind::RoleBalanceInput, 3> inputs {{
        { 0, stagemind::TrackRole::LeadVocal, 0.030f, 0.0f, 0.8f, true },
        { 1, stagemind::TrackRole::SunoDrums, 0.120f, 0.0f, 0.8f, true },
        { 2, stagemind::TrackRole::SunoBass, 0.110f, 0.0f, 0.8f, true }
    }};

    const auto decision = stagemind::chooseRoleBalanceDecision(inputs.data(), static_cast<int> (inputs.size()));
    expect(decision.found, "quiet vocal should produce a balance decision");
    expect(decision.index == 0, "quiet vocal should be the correction target");
    expect(decision.correctionDb > 0.0f, "quiet vocal should be boosted");
}

void padAtVocalLevelGetsCut()
{
    std::array<stagemind::RoleBalanceInput, 2> inputs {{
        { 0, stagemind::TrackRole::LeadVocal, 0.100f, 0.0f, 0.8f, true },
        { 1, stagemind::TrackRole::SunoSynthPad, 0.100f, 0.0f, 0.8f, true }
    }};

    const auto decision = stagemind::chooseRoleBalanceDecision(inputs.data(), static_cast<int> (inputs.size()));
    expect(decision.found, "pad at vocal level should produce a balance decision");
    expect(decision.index == 1, "pad should be corrected behind the vocal");
    expect(decision.correctionDb < 0.0f, "pad should be cut");
}

void disabledInputsAreIgnored()
{
    std::array<stagemind::RoleBalanceInput, 2> inputs {{
        { 0, stagemind::TrackRole::LeadVocal, 0.030f, 0.0f, 0.8f, false },
        { 1, stagemind::TrackRole::SunoDrums, 0.120f, 0.0f, 0.8f, true }
    }};

    const auto decision = stagemind::chooseRoleBalanceDecision(inputs.data(), static_cast<int> (inputs.size()));
    expect(! decision.found, "non-auto inputs should be ignored by balance model");
}

void smallGuitarDeviationStaysInsideDeadband()
{
    std::array<stagemind::RoleBalanceInput, 3> inputs {{
        { 0, stagemind::TrackRole::LeadVocal, 0.120f, 0.0f, 0.8f, true },
        { 1, stagemind::TrackRole::SunoGuitar, 0.085f, 0.0f, 0.8f, true },
        { 2, stagemind::TrackRole::SunoBass, 0.100f, 0.0f, 0.8f, true }
    }};

    const auto decision = stagemind::chooseRoleBalanceDecision(inputs.data(), static_cast<int> (inputs.size()));
    expect(decision.index != 1, "small guitar deviation should stay inside its role deadband");
}

void directorBoostIsCapped()
{
    std::array<stagemind::RoleBalanceInput, 3> inputs {{
        { 0, stagemind::TrackRole::LeadVocal, 0.030f, 2.70f, 0.8f, true },
        { 1, stagemind::TrackRole::SunoDrums, 0.120f, 0.0f, 0.8f, true },
        { 2, stagemind::TrackRole::SunoBass, 0.110f, 0.0f, 0.8f, true }
    }};

    const auto decision = stagemind::chooseRoleBalanceDecision(inputs.data(), static_cast<int> (inputs.size()));
    expect(decision.found, "quiet vocal near boost cap should still produce a final small correction");
    expect(decision.nextOutputTrimDb <= stagemind::maxDirectorOutputBoostDb + 0.001f, "director boost should be capped");
}

void strongLocalStageGainBlocksDirectorBoost()
{
    std::array<stagemind::RoleBalanceInput, 3> inputs {{
        { 0, stagemind::TrackRole::LeadVocal, 0.030f, 0.0f, 0.8f, true, 7.5f },
        { 1, stagemind::TrackRole::SunoDrums, 0.120f, 0.0f, 0.8f, true },
        { 2, stagemind::TrackRole::SunoBass, 0.110f, 0.0f, 0.8f, true }
    }};

    const auto decision = stagemind::chooseRoleBalanceDecision(inputs.data(), static_cast<int> (inputs.size()));
    expect(! decision.found || decision.index != 0, "director should not boost a node already boosted hard by local Stage Gain");
}

void staticStageGainIsManualForDirectorBalance()
{
    std::array<stagemind::RoleBalanceInput, 3> inputs {{
        { 0, stagemind::TrackRole::LeadVocal, 0.030f, 0.0f, 0.8f, true, 0.0f, 1 },
        { 1, stagemind::TrackRole::SunoDrums, 0.120f, 0.0f, 0.8f, true },
        { 2, stagemind::TrackRole::SunoBass, 0.110f, 0.0f, 0.8f, true }
    }};

    const auto decision = stagemind::chooseRoleBalanceDecision(inputs.data(), static_cast<int> (inputs.size()));
    expect(! decision.found || decision.index != 0, "director should not trim a node in Static Stage Gain mode");
}

void directorCutIsCapped()
{
    std::array<stagemind::RoleBalanceInput, 2> inputs {{
        { 0, stagemind::TrackRole::LeadVocal, 0.100f, 0.0f, 0.8f, true },
        { 1, stagemind::TrackRole::SunoSynthPad, 0.100f, -2.80f, 0.8f, true }
    }};

    const auto decision = stagemind::chooseRoleBalanceDecision(inputs.data(), static_cast<int> (inputs.size()));
    expect(decision.found, "pad near cut cap should still produce a final small correction");
    expect(decision.nextOutputTrimDb >= stagemind::minDirectorOutputTrimDb - 0.001f, "director cut should be capped");
}
} // namespace

void runRoleBalanceModelTests()
{
    quietVocalGetsPriorityBoost();
    padAtVocalLevelGetsCut();
    disabledInputsAreIgnored();
    smallGuitarDeviationStaysInsideDeadband();
    directorBoostIsCapped();
    strongLocalStageGainBlocksDirectorBoost();
    staticStageGainIsManualForDirectorBalance();
    directorCutIsCapped();
}
