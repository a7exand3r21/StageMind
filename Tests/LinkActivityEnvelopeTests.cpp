#include "../Source/Link/LinkActivityEnvelope.h"

#include <cstdlib>
#include <iostream>

namespace
{
constexpr double sampleRate = 48000.0;
constexpr int blockSize = 256;

void expect(bool condition, const char* message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

void envelopeRisesTowardActivity()
{
    stagemind::LinkActivityEnvelope envelope;
    envelope.prepare(sampleRate, blockSize);

    auto value = 0.0f;
    for (int i = 0; i < 20; ++i)
        value = envelope.process(1.0f, blockSize);

    expect(value > 0.90f, "link activity envelope should rise quickly toward active material");
    expect(value <= 1.0f, "link activity envelope should not exceed one");
}

void envelopeReleasesSlowly()
{
    stagemind::LinkActivityEnvelope envelope;
    envelope.prepare(sampleRate, blockSize);

    for (int i = 0; i < 20; ++i)
        envelope.process(1.0f, blockSize);

    const auto beforeRelease = envelope.getCurrentValue();
    const auto afterOneSilentBlock = envelope.process(0.0f, blockSize);

    expect(beforeRelease > 0.90f, "test precondition: envelope should be high before release");
    expect(afterOneSilentBlock > 0.80f, "link activity envelope should release slowly enough for stable UI status");
    expect(afterOneSilentBlock < beforeRelease, "link activity envelope should move downward during release");
}

void envelopeClampsInput()
{
    stagemind::LinkActivityEnvelope envelope;
    envelope.prepare(sampleRate, blockSize);

    const auto high = envelope.process(4.0f, blockSize);
    expect(high >= 0.0f && high <= 1.0f, "link activity envelope should clamp high input");

    const auto low = envelope.process(-2.0f, blockSize);
    expect(low >= 0.0f && low <= 1.0f, "link activity envelope should clamp low input");
}
} // namespace

void runLinkActivityEnvelopeTests()
{
    envelopeRisesTowardActivity();
    envelopeReleasesSlowly();
    envelopeClampsInput();
}

