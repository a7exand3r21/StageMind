# StageMind — техническое задание

**Версия:** 1.0, вычищенная спецификация для разработки  
**Тип:** FL Studio-first VST3 audio effect plug-in  
**Стек:** C++20 / JUCE / CMake / VST3  
**Основная DAW:** FL Studio  
**Основной сценарий:** работа со stem-дорожками, включая stems из Suno и других AI music tools  
**Главный артефакт MVP:** `StageMind Node.vst3`

---

## 0. Назначение документа

Этот документ является каноническим ТЗ для разработки StageMind.

Правила чтения:

1. Если старые черновики, README или комментарии в коде расходятся с этим документом, верным считается этот документ.
2. Не создавать скрытые параметры, enum-значения или DSP-пути, которых нет в этом документе.
3. Не менять ID параметров после первого рабочего VST3-билда, если проект уже использовался в FL Studio.
4. Для Codex и других code-agent задач использовать только один этап за раз. Не отдавать весь документ целиком, если задача касается одного модуля.
5. В audio callback запрещены heap allocation, lock, file I/O, logging, GUI calls и любые операции, которые могут ждать.

---

## 1. Что такое StageMind

StageMind — это VST3 insert effect для FL Studio.

Он ставится на stem-дорожку. Пользователь выбирает роль дорожки: вокал, кик, бас, гитара, пэд, Suno stem и так далее. После этого плагин применяет безопасные role-aware правила:

- стереоширина;
- панорама;
- mono low-end;
- side high-pass;
- phase/correlation safety;
- лёгкая частотная чистка;
- sidechain-aware dynamic EQ;
- позже — локальное подавление резонансов, motion, depth, pseudo-double.

Плагин не должен делать микс вместо человека. Он должен быстро давать рабочую стартовую точку и не ломать микс.

Короткая формула продукта:

```text
Role-aware space.
Clean frequency relationships.
FL-friendly automation.
Sidechain without pain.
```

---

## 2. Продуктовые принципы

StageMind должен:

- быстро расставлять stems в пространстве;
- сохранять центр у вокала, кика, баса и важных лидов;
- держать низ в mono или почти mono;
- не разваливать mono compatibility;
- не делать все дорожки широкими одновременно;
- не убивать тембр чрезмерным dynamic EQ;
- показывать, что он делает;
- иметь простые macro-controls;
- быть удобным для FL Automation Clips и Patcher;
- работать с большим количеством инстансов;
- не зависеть от открытого GUI.

StageMind не должен:

- сам управлять FL Mixer routing;
- полагаться на cloud, AI model или online-сервисы;
- автоматически распознавать инструменты в MVP;
- передавать audio stream между инстансами;
- менять latency во время playback;
- создавать параметры динамически;
- выполнять тяжёлую работу в GUI thread, если от неё зависит audio thread.

---

## 3. Архитектура продукта

Долгосрочно продукт состоит из трёх уровней.

```text
1. StageMind Node
2. StageMind Link
3. StageMind Director
```

### 3.1 StageMind Node

Это единственный плагин в MVP.

Node работает как insert effect на одной дорожке.

Он отвечает за:

- role selection;
- role-based spatial processing;
- pan;
- width;
- mono low-end;
- side high-pass;
- correlation safety;
- optional external sidechain input;
- sidechain detector;
- sidechain-aware dynamic EQ;
- local resonance detection позже;
- motion/depth/pseudo-double позже;
- meters;
- FL-friendly APVTS parameters.

### 3.2 StageMind Link

Будущий режим inter-instance control data.

Link не передаёт audio. Он передаёт только control data:

- role;
- group id;
- activity envelope;
- transient events;
- spectral band energy позже.

В MVP Link не реализуется. Параметры резервируются заранее, чтобы не ломать automation compatibility позже.

### 3.3 StageMind Director

Будущий отдельный master/bus плагин.

Director должен видеть Node-инстансы, строить карту сцены, предлагать коррекции и управлять группами. В MVP не реализуется.

---

## 4. Roadmap внедрения

Этапы идут строго по порядку. Каждый этап должен быть рабочим VST3-билдом.

### MVP 1 — Foundation

Цель: загрузить StageMind в FL Studio как стабильный VST3 insert.

Реализовать:

- JUCE/CMake проект;
- VST3 target;
- stereo main input/output;
- optional stereo sidechain input bus named `Sidechain`;
- APVTS parameter registry;
- basic GUI;
- role selector;
- safety selector;
- trigger selector;
- output gain;
- width через Mid/Side;
- mono low-end;
- side high-pass;
- sidechain meter only;
- basic input/output meters;
- state save/restore;
- zero latency.

Не реализовывать:

- FFT;
- resonance detector;
- dynamic EQ;
- pseudo-double;
- motion;
- depth;
- StageMind Link;
- Stage Director.

### MVP 2 — External sidechain dynamic EQ

Реализовать:

- sidechain envelope follower;
- SidechainDynamicEQ;
- mode presets:
  - VocalDucksInstrument;
  - KickDucksBass;
  - SnareDucksInstrument;
  - LeadDucksPad;
  - Custom;
- max 2 active sidechain bands;
- Mid/Side processing options per band;
- gain reduction meter;
- Sidechain Listen: Off / SidechainOnly;
- attack/release smoothing;
- automatable SC Amount.

Не реализовывать FFT.

### MVP 3 — Local resonance detection

Реализовать:

- ResonanceDetector with FFT;
- top 4 resonance peaks;
- dynamic resonance suppression;
- Resonance List in GUI;
- Clean Up macro mapping;
- Resonance macro mapping;
- tests with artificial peaks.

### MVP 4 — Motion / Depth / Pseudo-double

Реализовать:

- MotionProcessor with LFO;
- DepthProcessor lightweight psychoacoustic placeholder;
- PseudoDoubleProcessor;
- role limits;
- correlation safety integration;
- Stage View updates.

### MVP 5 — StageMind Link

Реализовать:

- inter-instance registry;
- control-data publishing;
- control-data reading;
- group id;
- source/target roles;
- offline render safety.

No audio transfer.

### MVP 6 — StageMind Director

Отдельный плагин. Не часть Node MVP.

---

## 5. FL Studio-first требования

StageMind пишется в первую очередь под FL Studio.

Требования:

- основной формат: VST3;
- параметры видны в Fruity Wrapper;
- первые параметры — macro-friendly;
- sidechain bus виден host, если host поддерживает sidechain;
- удобно работает в Patcher;
- работает с FL Automation Clips;
- корректно переживает Smart Disable;
- zero-latency в MVP;
- 10–20 инстансов должны быть реалистичны на современном CPU;
- GUI закрыт — audio processing продолжает работать;
- GUI закрыт — визуальная/analyzer нагрузка снижается.

Не делать native FL plugin. Не пытаться программно управлять FL Mixer routing.

---

## 6. Technology stack

```text
Language: C++20
Framework: JUCE
Build: CMake
Plugin format: VST3
Primary DAW: FL Studio
Primary OS: Windows
Secondary OS: macOS later
```

VST2, CLAP, AU и standalone app не входят в MVP.

---

## 7. Audio bus layout

VST3 должен объявлять:

```text
Main Input: stereo default
Main Output: stereo default
Sidechain Input: optional stereo, bus name "Sidechain"
```

JUCE bus layout:

```cpp
BusesProperties()
    .withInput("Input", juce::AudioChannelSet::stereo(), true)
    .withOutput("Output", juce::AudioChannelSet::stereo(), true)
    .withInput("Sidechain", juce::AudioChannelSet::stereo(), false);
```

Поддержка layout в MVP:

```text
mono main input/output: supported
stereo main input/output: supported
mono sidechain: supported if host provides it
stereo sidechain: supported
surround / 5.1 / 7.1: unsupported in MVP
```

Правила:

- mono main input обрабатывается как dual-mono внутри stereo DSP;
- width не создаёт stereo из mono сам по себе в MVP;
- sidechain может отсутствовать;
- sidechain absence не должен вызывать crash, silence или denormals;
- sidechain listen должен работать только если signal реально приходит;
- unsupported layouts отклонять в `isBusesLayoutSupported`.

---

## 8. Canonical enums

### 8.1 TrackRole

`TrackRole` — единственный источник правды для ролей.

`Unknown` — internal/default state. В GUI не показывать как обычную роль, если только не нужен debug mode.

```cpp
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
```

GUI role list должен содержать все selectable roles:

```text
Lead Vocal
Backing Vocal
Kick
Bass
Snare
HiHat
Percussion
Rhythm Guitar Single
Rhythm Guitar Pair Left
Rhythm Guitar Pair Right
Lead Guitar
Pad
Piano
Synth Lead
Synth Bass
FX
Atmosphere
Suno Vocal
Suno Instrumental
Suno Drums
Suno Bass
Suno Guitar
Suno Synth Pad
Suno Percussion
Suno FX
```

Не сокращать три Rhythm Guitar роли в один GUI пункт, если нет явного mapping layer.

### 8.2 TriggerMode

```cpp
enum class TriggerMode
{
    Off = 0,
    Self,
    ExternalSidechain,
    StageMindLink
};
```

Правила:

```text
Off:
  no trigger-based dynamic processing
  spatial processing remains active

Self:
  analyze own input
  used for local resonance and self-cleanup
  sidechain_enabled is ignored

ExternalSidechain:
  analyze sidechain bus
  meter may run even if processing disabled
  sidechain_enabled gates gain reduction

StageMindLink:
  reserved for future
  no audio behavior in MVP 1–4
```

### 8.3 SafetyMode

```cpp
enum class SafetyMode
{
    MonoSafe = 0,
    Natural,
    ModernWide,
    HeadphonesWide
};
```

### 8.4 SidechainConflictMode

```cpp
enum class SidechainConflictMode
{
    Off = 0,
    MakeSpace,
    VocalDucksInstrument,
    KickDucksBass,
    SnareDucksInstrument,
    LeadDucksPad,
    Custom
};
```

### 8.5 SidechainListenMode

```cpp
enum class SidechainListenMode
{
    Off = 0,
    SidechainOnly,
    MainOnly,
    Delta
};
```

MVP 1–2 implement only:

```text
Off
SidechainOnly
```

`MainOnly` and `Delta` are reserved.

### 8.6 Quality enums

```cpp
enum class AnalyzerQuality
{
    Eco = 0,
    Normal,
    High
};

enum class ProcessingQuality
{
    Draft = 0,
    Normal,
    High
};
```

Default:

```text
AnalyzerQuality: Normal
ProcessingQuality: Normal
```

---

## 9. APVTS parameter registry

Use `juce::AudioProcessorValueTreeState`.

Parameter IDs are stable API. Do not rename, reorder or remove after first used release.

### 9.1 Canonical order

First 8 parameters are fixed for FL and Patcher:

```text
1. role
2. width
3. depth
4. motion
5. clean_up
6. resonance
7. safety
8. output_gain
```

Full MVP+reserved list:

| Order | ID | Type | Range / values | Default | Display |
|---:|---|---|---|---|---|
| 1 | `role` | choice | TrackRole selectable values | `SunoInstrumental` or `Unknown` | Role |
| 2 | `width` | float | 0.0–1.0 | 0.50 | Width |
| 3 | `depth` | float | 0.0–1.0 | 0.30 | Depth |
| 4 | `motion` | float | 0.0–1.0 | 0.00 | Motion |
| 5 | `clean_up` | float | 0.0–1.0 | 0.30 | Clean Up |
| 6 | `resonance` | float | 0.0–1.0 | 0.30 | Resonance |
| 7 | `safety` | choice | SafetyMode | `Natural` | Safety |
| 8 | `output_gain` | float dB | -24.0–12.0 | 0.0 | Output |
| 9 | `trigger_mode` | choice | TriggerMode | `Off` | Trigger |
| 10 | `sidechain_enabled` | bool | false/true | false | SC Enable |
| 11 | `sidechain_mode` | choice | SidechainConflictMode | `Off` | SC Mode |
| 12 | `sidechain_source_role` | choice | TrackRole selectable values | `LeadVocal` | SC Source |
| 13 | `sidechain_amount` | float | 0.0–1.0 | 0.30 | SC Amount |
| 14 | `sidechain_listen` | choice | SidechainListenMode | `Off` | SC Listen |
| 15 | `sidechain_attack` | float ms | 1.0–200.0 | 50.0 | SC Attack |
| 16 | `sidechain_release` | float ms | 20.0–1000.0 | 250.0 | SC Release |
| 17 | `sidechain_range_start` | float Hz | 20.0–20000.0 | 2000.0 | SC Start |
| 18 | `sidechain_range_end` | float Hz | 20.0–20000.0 | 5000.0 | SC End |
| 19 | `pan` | float | -1.0–1.0 | 0.0 | Pan |
| 20 | `mono_low_cutoff` | float Hz | 40.0–300.0 | 160.0 | Mono Low |
| 21 | `side_highpass` | float Hz | 40.0–500.0 | 220.0 | Side HP |
| 22 | `correlation_safety_threshold` | float | -1.0–1.0 | 0.10 | Corr Safe |
| 23 | `resonance_sensitivity` | float | 0.0–1.0 | 0.50 | Res Sens |
| 24 | `max_resonance_reduction` | float dB | 0.0–7.0 | 4.0 | Max Res Cut |
| 25 | `dynamic_eq_attack` | float ms | 1.0–200.0 | 30.0 | Dyn Attack |
| 26 | `dynamic_eq_release` | float ms | 20.0–1000.0 | 250.0 | Dyn Release |
| 27 | `motion_rate` | float Hz | 0.01–8.0 | 0.20 | Motion Rate |
| 28 | `pseudo_double_amount` | float | 0.0–1.0 | 0.0 | Double |
| 29 | `presence_reduction` | float | 0.0–1.0 | 0.0 | Presence |
| 30 | `early_reflection_amount` | float | 0.0–1.0 | 0.0 | Early Ref |
| 31 | `analyzer_quality` | choice | AnalyzerQuality | `Normal` | Analyzer |
| 32 | `processing_quality` | choice | ProcessingQuality | `Normal` | Quality |
| 33 | `link_enabled` | bool | false/true | false | Link Enable |
| 34 | `link_group` | int/choice | 0–16 | 0 | Link Group |
| 35 | `link_role` | choice | TrackRole selectable values | `Unknown` | Link Role |
| 36 | `link_source_id` | int/choice | 0–128 | 0 | Link Source |
| 37 | `link_target_id` | int/choice | 0–128 | 0 | Link Target |
| 38 | `link_mode` | int/choice | reserved | 0 | Link Mode |

Node 0.8.0 appends `plugin_mode` and `auto_assist_mode` after the original reserved Link parameters. Existing first parameters stay stable for FL automation.

Do not create a second automatable depth parameter. Use `depth` as the single depth macro.

### 9.2 Parameter smoothing

All audio-affecting continuous parameters require smoothing:

- `width`;
- `depth`;
- `motion`;
- `clean_up`;
- `resonance`;
- `output_gain`;
- `sidechain_amount`;
- attack/release dependent gain reduction;
- `pan`;
- `mono_low_cutoff` if moved during playback;
- `side_highpass` if moved during playback;
- pseudo-double amount;
- dynamic EQ gain/frequency/Q where applicable.

Recommended baseline:

```text
Gain-like controls: 20–50 ms smoothing
Width/pan: 20–100 ms smoothing
Dynamic EQ gain reduction: attack/release envelope
Role switching: 100–250 ms cross-smoothing where practical
```

No clicks, pops or zipper noise during FL Automation Clips.

---

## 10. Macro mappings

### 10.1 Width

Separate user parameter from internal coefficient.

```text
width: APVTS user macro, 0.0–1.0
widthAmount: internal Mid/Side side-gain coefficient, can be > 1.0
```

Mapping:

```cpp
float mapWidthUserToAmount(float width)
{
    width = juce::jlimit(0.0f, 1.0f, width);

    if (width <= 0.30f)
        return juce::jmap(width, 0.0f, 0.30f, 0.25f, 1.0f);

    if (width <= 0.70f)
        return juce::jmap(width, 0.30f, 0.70f, 1.0f, 1.35f);

    return juce::jmap(width, 0.70f, 1.0f, 1.35f, 1.80f);
}
```

Final coefficient:

```cpp
finalWidthAmount = std::min({ mappedWidthAmount, roleMaxWidth, safetyMaxWidth });
```

Meaning:

```text
0.00–0.30: narrow to natural
0.30–0.70: natural to wide
0.70–1.00: wide to extra-wide, safety-limited
```

### 10.2 Clean Up

```text
0.0: no corrective processing
0.1–0.5: subtle correction
0.5–0.8: normal correction
0.8–1.0: strong correction, show warning if aggressive
```

### 10.3 Resonance

```text
0.0: off
0.3: light control
0.6: natural control
1.0: strong control, role-limited
```

### 10.4 Sidechain amount

```text
0.0: no ducking
0.3: subtle space making
0.6: clear ducking
1.0: strong but role-limited ducking
```

### 10.5 Depth

`depth` is a macro.

```text
0.0: front
0.3: front-mid
0.6: mid-back
1.0: back
```

In MVP 1–3, `depth` exists and saves/restores but does not affect DSP unless DepthProcessor is implemented.

---

## 11. Data model

### 11.1 RoleProfile

```cpp
struct RoleProfile
{
    TrackRole role = TrackRole::Unknown;

    float defaultPan = 0.0f;              // -1 left, 0 center, +1 right
    float defaultWidthAmount = 1.0f;      // internal coefficient: 1 natural, >1 wide
    float maxWidthAmount = 1.35f;         // role clamp
    float defaultDepth = 0.3f;            // 0 front, 1 back
    float defaultMotionAmount = 0.0f;

    float lowMonoCutoffHz = 160.0f;
    float sideHighPassHz = 220.0f;

    float resonanceSensitivity = 0.5f;
    float maxResonanceReductionDb = 4.0f;

    float cleanUpAmount = 0.3f;
    float maxDynamicEqReductionDb = 4.0f;

    bool protectTransient = false;
    bool protectLowEnd = false;
    bool allowPseudoDouble = false;
    bool allowMotion = false;
    bool keepDryCenter = false;

    float priority = 0.5f;                // 0 low, 1 high
};
```

No `optional` values in boolean fields. If a feature is allowed but default-off, set the boolean to `true` and default amount to `0.0f`.

### 11.2 Priority scale

```text
High:        1.00
MediumHigh: 0.80
Medium:      0.60
LowMedium:   0.40
Low:         0.25
```

### 11.3 ResonancePeak

```cpp
struct ResonancePeak
{
    float frequencyHz = 0.0f;
    float severity = 0.0f;              // 0..1
    float suggestedQ = 1.0f;
    float suggestedReductionDb = 0.0f;
};
```

### 11.4 TrackSpectralSummary

No dynamic containers.

```cpp
static constexpr size_t NumSpectralBands = 32;
static constexpr size_t MaxResonancePeaks = 4;

struct TrackSpectralSummary
{
    TrackRole role = TrackRole::Unknown;
    float rms = 0.0f;
    float peak = 0.0f;

    std::array<float, NumSpectralBands> bandEnergy {};
    std::array<float, NumSpectralBands> sideEnergy {};

    std::array<ResonancePeak, MaxResonancePeaks> resonances {};
    uint8_t resonanceCount = 0;
};
```

### 11.5 Sidechain profiles

Max 2 active sidechain bands in MVP.

```cpp
static constexpr size_t MaxSidechainBands = 2;

struct SidechainBand
{
    float frequencyStartHz = 0.0f;
    float frequencyEndHz = 0.0f;
    float maxReductionDb = 0.0f;

    bool processMid = true;
    bool processSide = false;
};

struct SidechainConflictProfile
{
    SidechainConflictMode mode = SidechainConflictMode::Off;

    std::array<SidechainBand, MaxSidechainBands> bands {};
    uint8_t bandCount = 0;

    float attackMs = 50.0f;
    float releaseMs = 250.0f;

    bool preserveLowEnd = true;
    bool preserveTransient = false;
};
```

### 11.6 SidechainAnalysis

```cpp
struct SidechainAnalysis
{
    float rms = 0.0f;
    float peak = 0.0f;
    float envelope = 0.0f;
    bool isActive = false;
    bool transientDetected = false;
};
```

---

## 12. Canonical role profiles

These values are defaults. User parameters and safety mode can clamp or override them.

Use numeric values in code. Do not encode text like `medium` or `optional`.

```cpp
// role, pan, defaultWidth, maxWidth, depth, motion,
// lowMono, sideHP, resSens, maxResCut, cleanup, maxDynCut,
// protectTransient, protectLowEnd, allowPseudoDouble, allowMotion, keepDryCenter, priority

{ TrackRole::LeadVocal,              0.00f, 1.00f, 1.20f, 0.20f, 0.00f, 120.0f, 180.0f, 0.60f, 4.0f, 0.50f, 4.0f, false, false, false, false, true,  1.00f },
{ TrackRole::BackingVocal,           0.35f, 1.25f, 1.55f, 0.60f, 0.15f, 140.0f, 200.0f, 0.60f, 4.0f, 0.50f, 4.0f, false, false, true,  true,  false, 0.60f },

{ TrackRole::Kick,                   0.00f, 0.25f, 0.60f, 0.10f, 0.00f, 180.0f, 200.0f, 0.50f, 3.0f, 0.30f, 3.0f, true,  true,  false, false, true,  1.00f },
{ TrackRole::Bass,                   0.00f, 0.35f, 0.75f, 0.15f, 0.00f, 160.0f, 200.0f, 0.40f, 3.0f, 0.40f, 3.0f, false, true,  false, false, true,  1.00f },
{ TrackRole::Snare,                  0.00f, 0.90f, 1.20f, 0.25f, 0.00f, 140.0f, 180.0f, 0.75f, 5.0f, 0.50f, 4.0f, true,  false, false, false, true,  1.00f },
{ TrackRole::HiHat,                  0.35f, 1.00f, 1.25f, 0.35f, 0.10f, 200.0f, 300.0f, 0.70f, 4.0f, 0.50f, 4.0f, true,  false, false, true,  false, 0.60f },
{ TrackRole::Percussion,             0.45f, 1.20f, 1.50f, 0.45f, 0.20f, 160.0f, 220.0f, 0.55f, 4.0f, 0.50f, 4.0f, true,  false, false, true,  false, 0.60f },

{ TrackRole::RhythmGuitarSingle,     0.35f, 1.10f, 1.45f, 0.45f, 0.05f, 140.0f, 180.0f, 0.55f, 4.0f, 0.50f, 4.0f, false, false, true,  false, false, 0.60f },
{ TrackRole::RhythmGuitarPairLeft,  -0.85f, 1.00f, 1.20f, 0.45f, 0.00f, 140.0f, 180.0f, 0.55f, 4.0f, 0.50f, 4.0f, false, false, false, false, false, 0.60f },
{ TrackRole::RhythmGuitarPairRight,  0.85f, 1.00f, 1.20f, 0.45f, 0.00f, 140.0f, 180.0f, 0.55f, 4.0f, 0.50f, 4.0f, false, false, false, false, false, 0.60f },
{ TrackRole::LeadGuitar,             0.00f, 1.00f, 1.30f, 0.30f, 0.05f, 140.0f, 180.0f, 0.55f, 4.0f, 0.50f, 4.0f, false, false, true,  false, true,  0.80f },

{ TrackRole::Pad,                    0.00f, 1.45f, 1.70f, 0.80f, 0.20f, 180.0f, 220.0f, 0.40f, 3.0f, 0.40f, 3.0f, false, false, true,  true,  false, 0.25f },
{ TrackRole::Piano,                  0.00f, 1.15f, 1.40f, 0.45f, 0.00f, 120.0f, 180.0f, 0.55f, 4.0f, 0.50f, 4.0f, true,  false, false, false, true,  0.60f },
{ TrackRole::SynthLead,              0.00f, 1.10f, 1.35f, 0.30f, 0.10f, 140.0f, 200.0f, 0.55f, 4.0f, 0.50f, 4.0f, false, false, true,  true,  true,  0.80f },
{ TrackRole::SynthBass,              0.00f, 0.35f, 0.75f, 0.15f, 0.00f, 160.0f, 220.0f, 0.40f, 3.0f, 0.40f, 3.0f, false, true,  false, false, true,  1.00f },

{ TrackRole::FX,                     0.00f, 1.50f, 1.80f, 0.70f, 0.35f, 180.0f, 220.0f, 0.35f, 3.0f, 0.30f, 3.0f, false, false, true,  true,  false, 0.25f },
{ TrackRole::Atmosphere,             0.00f, 1.50f, 1.80f, 0.85f, 0.20f, 200.0f, 250.0f, 0.25f, 2.5f, 0.30f, 2.5f, false, false, true,  true,  false, 0.25f },

{ TrackRole::SunoVocal,              0.00f, 1.00f, 1.15f, 0.30f, 0.00f, 120.0f, 180.0f, 0.55f, 3.0f, 0.50f, 3.5f, false, false, false, false, true,  1.00f },
{ TrackRole::SunoInstrumental,       0.00f, 1.10f, 1.35f, 0.45f, 0.05f, 160.0f, 220.0f, 0.55f, 3.0f, 0.50f, 3.5f, false, true,  false, false, false, 0.60f },
{ TrackRole::SunoDrums,              0.00f, 1.00f, 1.25f, 0.30f, 0.00f, 160.0f, 220.0f, 0.55f, 4.0f, 0.50f, 4.0f, true,  true,  false, false, true,  1.00f },
{ TrackRole::SunoBass,               0.00f, 0.35f, 0.75f, 0.15f, 0.00f, 160.0f, 220.0f, 0.40f, 3.0f, 0.30f, 3.0f, false, true,  false, false, true,  1.00f },
{ TrackRole::SunoGuitar,             0.00f, 1.10f, 1.35f, 0.45f, 0.05f, 140.0f, 180.0f, 0.55f, 3.5f, 0.50f, 3.5f, false, false, true,  false, false, 0.60f },
{ TrackRole::SunoSynthPad,           0.00f, 1.40f, 1.65f, 0.70f, 0.15f, 180.0f, 220.0f, 0.40f, 3.0f, 0.40f, 3.0f, false, false, true,  true,  false, 0.40f },
{ TrackRole::SunoPercussion,         0.40f, 1.15f, 1.45f, 0.45f, 0.15f, 160.0f, 220.0f, 0.55f, 3.5f, 0.50f, 3.5f, true,  false, false, true,  false, 0.60f },
{ TrackRole::SunoFX,                 0.00f, 1.45f, 1.75f, 0.70f, 0.25f, 180.0f, 240.0f, 0.35f, 3.0f, 0.30f, 3.0f, false, false, true,  true,  false, 0.25f }
```

Implementation note: use named constants or a factory function. Do not leave raw arrays undocumented in production code.

---

## 13. Safety modes

Safety mode clamps width, pseudo-double, motion and correlation behavior.

| SafetyMode | maxWidthAmount | correlation threshold | low-end mono | pseudo-double | motion |
|---|---:|---:|---|---|---|
| MonoSafe | 1.10 | 0.25 | strong | off/reduced | reduced |
| Natural | 1.35 | 0.10 | normal | role-limited | role-limited |
| ModernWide | 1.60 | 0.00 | protected | allowed | allowed |
| HeadphonesWide | 1.80 | -0.10 | protected | allowed with warning | allowed |

If correlation drops below threshold:

```text
1. reduce width gradually
2. reduce pseudo-double amount if active
3. show warning in GUI
4. never hard-mute audio
```

---

## 14. RolePresetEngine

`RolePresetEngine` converts roles, safety mode and user macros into DSP parameters.

Minimum interface:

```cpp
class RolePresetEngine
{
public:
    const RoleProfile& getProfile(TrackRole role) const noexcept;

    SpatialParams buildSpatialParams(
        TrackRole role,
        SafetyMode safety,
        const UserMacroParams& macros) const noexcept;

    SidechainConflictProfile getSidechainProfile(
        SidechainConflictMode mode,
        TrackRole sourceRole,
        TrackRole targetRole) const noexcept;

    void applyRoleDefaultsToState(
        TrackRole role,
        juce::AudioProcessorValueTreeState& apvts);
};
```

Rules:

- `getProfile` is realtime-safe;
- profile storage is static or preallocated;
- no allocation during audio callback;
- applying defaults to APVTS must not happen inside audio callback;
- role switching must not reset macro values unless user explicitly loads a preset;
- role switching may update internal target values and smooth toward them.

---

## 15. PluginState

`PluginState` owns state serialization policy. It does not own DSP.

It must save and restore:

- all APVTS parameters;
- current preset id/name;
- GUI size;
- plugin version;
- future migration metadata.

Use APVTS state for DAW project recall.

MVP factory presets are hardcoded. User preset files are not required in MVP.

No file I/O from audio thread.

---

## 16. DSP architecture

Audio path order in MVP 1:

```text
Input
  -> meter input snapshot
  -> pan / spatial prep
  -> mono low-end
  -> side high-pass
  -> Mid/Side width
  -> output gain
  -> meter output snapshot
Output
```

Sidechain path in MVP 1:

```text
Optional sidechain input
  -> SidechainDetector
  -> sidechain meter snapshot
  -> no gain reduction yet
```

Audio path order in MVP 2:

```text
Input
  -> spatial processors
  -> SidechainDynamicEQ if enabled and triggered
  -> output gain
Output
```

MVP 3 adds local resonance dynamic EQ. MVP 4 adds motion/depth/pseudo-double.

---

## 17. Mid/Side processing

Formulas:

```text
M = (L + R) * 0.5
S = (L - R) * 0.5

L = M + S
R = M - S
```

Width:

```text
S_processed = S * widthAmount
```

Requirements:

- smooth widthAmount;
- clamp by role and safety;
- protect low-end before wide side gain;
- avoid denormals;
- no dynamic allocation.

---

## 18. Mono low-end

Purpose: low frequencies should not have dangerous side energy.

MVP approach:

- split low/high with simple IIR filters;
- reduce side channel in low band;
- preserve high stereo;
- cutoff from role profile and `mono_low_cutoff` parameter.

Rules:

- no linear-phase EQ in MVP;
- no lookahead;
- zero plugin latency;
- smoothing when cutoff changes.

---

## 19. Side high-pass

Purpose: remove low-frequency side mud.

Apply high-pass to Side channel.

Requirements:

- role default from `sideHighPassHz`;
- user override from `side_highpass`;
- safety mode may raise minimum;
- no unstable filter jumps during automation.

---

## 20. Correlation meter

Range:

```text
+1 = mono-compatible
 0 = decorrelated / wide
-1 = phase problems
```

MVP:

- compute lightweight block correlation;
- publish atomic snapshot for GUI;
- do not allocate;
- do not call GUI from audio thread.

MVP 4:

- use correlation safety to reduce width/pseudo-double gradually.

---

## 21. SidechainDetector

Analyze optional sidechain input.

Requirements:

- mono/stereo sidechain safe;
- absent sidechain safe;
- RMS;
- peak;
- envelope follower;
- `isActive` boolean;
- transient detection later;
- atomic meter snapshot.

No dynamic allocation in process.

---

## 22. Sidechain behavior

`trigger_mode` and `sidechain_enabled` are separate.

| trigger_mode | sidechain_enabled | Meter | Processing |
|---|---:|---|---|
| Off | false/true | optional input meter only | no trigger processing |
| Self | ignored | no external SC required | self-trigger features |
| ExternalSidechain | false | sidechain detector/meter may run | no gain reduction |
| ExternalSidechain | true | sidechain detector/meter runs | sidechain dynamic EQ may run |
| StageMindLink | false/true | no Link behavior in MVP | no processing |

If `ExternalSidechain` is selected but `sidechain_enabled = false`, GUI should display:

```text
External sidechain selected, processing disabled
```

If sidechain is enabled but no signal is detected:

```text
No sidechain signal
```

---

## 23. SidechainDynamicEQ

MVP 2.

Used for:

- Vocal -> Guitar / Pad / Piano / Synth;
- Kick -> Bass;
- Snare -> Instrument;
- Lead -> Pad;
- Custom range.

Requirements:

- no automatic boosts;
- max cut per band: 7 dB;
- default cut: 1.5–4 dB depending mode;
- max active sidechain bands: 2;
- frequency-dependent cuts only;
- no full-band ducking unless explicitly added later;
- smooth gain reduction;
- Sidechain Listen Off / SidechainOnly;
- gain reduction meter.

### 23.1 Sidechain mode presets

#### VocalDucksInstrument

```cpp
mode = VocalDucksInstrument
bandCount = 1
bands[0] = { 2000.0f, 5000.0f, 3.0f, true, false }
attackMs = 80.0f
releaseMs = 350.0f
preserveLowEnd = true
preserveTransient = false
```

Behavior:

```text
When vocal is active, target ducks presence range.
Do not remove body.
Do not collapse width.
```

#### KickDucksBass

```cpp
mode = KickDucksBass
bandCount = 1
bands[0] = { 45.0f, 100.0f, 4.0f, true, false }
attackMs = 5.0f
releaseMs = 150.0f
preserveLowEnd = false
preserveTransient = false
```

Behavior:

```text
Only low range ducks.
Do not pump entire bass.
```

#### SnareDucksInstrument

```cpp
mode = SnareDucksInstrument
bandCount = 1
bands[0] = { 1500.0f, 3000.0f, 2.5f, true, false }
attackMs = 15.0f
releaseMs = 160.0f
preserveLowEnd = true
preserveTransient = true
```

Behavior:

```text
Guitars/pads make room for snare crack.
Do not overcut.
```

#### LeadDucksPad

```cpp
mode = LeadDucksPad
bandCount = 2
bands[0] = { 300.0f, 1200.0f, 2.0f, true, false }
bands[1] = { 1200.0f, 4000.0f, 4.0f, true, true }
attackMs = 120.0f
releaseMs = 500.0f
preserveLowEnd = true
preserveTransient = false
```

Behavior:

```text
Pad leaves mid space.
Side air can remain, but mid must clear first.
```

#### Custom

Use user parameters:

```text
sidechain_range_start
sidechain_range_end
sidechain_amount
sidechain_attack
sidechain_release
Mid/Side mode later or from simplified defaults
```

---

## 24. DynamicEQ

Used for:

- sidechain conflict ducking;
- local resonance suppression;
- self-trigger de-mud/harshness later.

Requirements:

```text
no automatic boosts in MVP
max cut per band: 7 dB
default max cut: 3–4 dB
max active local resonance bands: 4
max active sidechain bands: 2
```

Smoothing required for:

- frequency;
- Q;
- gain reduction;
- attack/release;
- enabled/disabled state.

---

## 25. ResonanceDetector

MVP 3.

Task: find narrow unpleasant peaks inside one track.

FFT MVP:

```text
FFT size: 2048 or 4096
window: Hann
time smoothing: yes
log-frequency grouping: yes
max active peaks: 4
```

A peak may be treated as resonance if:

```text
peakLevel > localAverage + thresholdDb
peakWidth is narrow
peak persists for minimum time
frequency is inside allowed role range
```

Do not treat as resonance:

- transient-only spike;
- bass fundamental;
- vocal body;
- broad tonal area;
- intentionally dominant musical note.

Realtime rule:

- FFT work must not block audio;
- any buffers are preallocated in `prepareToPlay`;
- GUI may display results but DSP must not depend on GUI;
- no dynamic container resizing in callback.

---

## 26. MotionProcessor

MVP 4.

MVP features:

- sine LFO;
- rate in Hz;
- amount;
- role limits;
- no low-frequency motion;
- no motion for center-critical roles.

Allowed by role:

```text
Kick: no
Bass: no
Lead Vocal: no
Snare: no
HiHat: subtle
Percussion: yes
Pad: slow
FX: yes
Atmosphere: slow
Suno roles: conservative
```

Later:

- tempo sync;
- random drift;
- envelope follower;
- side-only motion;
- band-limited motion.

---

## 27. DepthProcessor

MVP 4.

No full reverb in MVP.

Depth is lightweight psychoacoustic processing:

```text
front: dry mostly preserved
mid: small presence reduction, tiny early reflection path
back: more presence reduction, high damping, slightly lower dry level
```

Allowed techniques:

- high-shelf reduction;
- early reflection wet path;
- tiny stereo delay only on wet/decorrelated path;
- dry/wet mix.

Latency clarification:

```text
Zero-latency means the dry/main signal path is not delayed.
The plugin reports zero host latency.
No lookahead, no linear phase, no PDC required.
A tiny wet-path delay is allowed only if the dry path remains undelayed.
```

---

## 28. PseudoDoubleProcessor

MVP 4.

Allowed roles by default:

```text
Backing Vocal
Rhythm Guitar Single
Lead Guitar
Synth Lead
Pad
FX
Atmosphere
Suno Guitar
Suno Synth Pad
Suno FX
```

Forbidden or default-off center-critical roles:

```text
Kick
Bass
Lead Vocal
Snare
HiHat
Synth Bass
Suno Vocal
Suno Bass
Suno Drums
```

MVP pseudo-double:

```text
micro delay: 10–30 ms wet path only
detune later
left/right EQ difference later
modulation later
```

Requirements:

- mono safety;
- correlation monitoring;
- warning if phase risk;
- reduced/off in MonoSafe;
- no PDC latency.

---

## 29. Realtime safety

Forbidden in audio callback:

```text
malloc
new
delete
file I/O
logging
mutex lock
dynamic vector resizing
GUI calls
blocking calls
expensive allocation
host queries that may block
```

Required:

- preallocate buffers in `prepareToPlay`;
- use `std::array` for fixed-size DSP data;
- use lock-free or atomic snapshots for meters;
- GUI reads snapshots;
- audio thread never waits for GUI;
- role changes do not allocate in callback;
- sidechain absent/present changes are safe;
- avoid denormals;
- use RAII outside realtime path;
- no raw owning pointers.

---

## 30. Latency / PDC

MVP must report zero latency.

Forbidden in MVP:

```text
linear-phase EQ
lookahead dynamic EQ
lookahead compressor
lookahead limiter
oversampling with latency
full-path delay
```

If latency is added later:

- report latency to host;
- latency must be stable;
- no dynamic latency changes during playback;
- GUI must show latency;
- FL Studio behavior must be tested.

---

## 31. GUI

GUI must be simple and FL-friendly.

Requirements:

- resizable;
- readable at 100%, 125%, 150%, 200%;
- dark interface preferred;
- no tiny controls;
- no modal windows during playback;
- no text input required for normal use;
- no keyboard focus stealing;
- DSP works with GUI closed;
- GUI never controls audio directly except through APVTS or safe snapshots.

MVP layout:

```text
Top:
  Preset
  Role
  Safety
  Trigger

Center Left:
  Stage View placeholder

Center:
  Width
  Depth
  Motion

Bottom:
  Clean Up
  Resonance
  SC Amount
  Output

Right:
  Input Meter
  Output Meter
  Correlation
  Sidechain Meter
  Gain Reduction Meter
  Resonance List
```

MVP 1 GUI controls:

- Role dropdown;
- Safety dropdown;
- Trigger dropdown;
- Width slider/knob;
- Depth slider/knob;
- Motion slider/knob;
- Clean Up slider/knob;
- Resonance slider/knob;
- Output slider/knob;
- Sidechain Enable toggle;
- Sidechain Mode dropdown;
- Sidechain Amount slider/knob;
- input meter placeholder;
- output meter placeholder;
- sidechain meter placeholder;
- current role label.

---

## 32. Stage View

MVP Stage View can be simple.

```text
2D rectangle or semicircle
x-axis = pan
y-axis = depth
dot size = width
dot color/state = safety/correlation
```

It should show quickly:

- center vs side;
- front vs back;
- narrow vs wide;
- phase warning.

No complex graphics required.

---

## 33. Metering

MVP meters:

```text
Input RMS/Peak
Output RMS/Peak
Sidechain RMS/Peak
Gain Reduction placeholder
Correlation
```

Metering must be lightweight.

Implementation:

- audio thread writes atomic/plain lock-free snapshots;
- GUI polls at a safe timer rate;
- GUI closed reduces visual work;
- audio thread does not allocate for meters.

---

## 34. Presets

MVP factory presets are hardcoded.

Basic presets:

```text
Vocal - Clean Center
Vocal - Airy Lead
Backing Vocal - Wide Pair
Guitar - Single Wider
Guitar - Double Left
Guitar - Double Right
Bass - Mono Safe
Kick - Tight Center
Snare - Focused
HiHat - Controlled Bright
Percussion - Wide Motion
Pad - Wide Back
FX - Wide Motion
Atmosphere - Deep Wide
```

FL workflow presets:

```text
FL - Vocal Center Cleaner
FL - Guitar Clears For Vocal
FL - Bass Ducks From Kick
FL - Pad Clears For Vocal
FL - Snare Focus
FL - HiHat Controlled Side
FL - Percussion Wide Motion
FL - Suno Stem Cleanup
FL - Suno Wide Chorus
FL - Suno Mono Safe Low End
```

Suno presets:

```text
Suno - Lead Vocal Cleanup
Suno - Instrumental Cleanup
Suno - Drums Tighten
Suno - Bass Mono Safe
Suno - Guitar Space
Suno - Synth Pad Wide Safe
Suno - Percussion Clean Wide
Suno - FX Wide Motion
```

User preset files are later scope.

---

## 35. Patcher support

First 8 parameters are fixed:

```text
1. Role
2. Width
3. Depth
4. Motion
5. Clean Up
6. Resonance
7. Safety
8. Output
```

Next common parameters:

```text
9. Trigger
10. SC Enable
11. SC Mode
12. SC Source
13. SC Amount
14. SC Listen
```

Patcher scenarios:

```text
StageMind Guitar Rack:
  StageMind Node
  optional Fruity Parametric EQ 2
  optional delay/reverb
  optional Fruity Balance

StageMind Bass Rack:
  StageMind Node
  optional Maximus / Fruity Limiter after plugin

StageMind Vocal Rack:
  StageMind Node
  optional de-esser
  optional delay/reverb sends

StageMind Suno Cleanup Rack:
  StageMind Node
  optional EQ
  optional saturation
```

---

## 36. FL Studio Smart Disable

Requirements:

- no dependency on continuous processing for state;
- LFO recovers correctly;
- analyzer stale data resets;
- sidechain detector resets gracefully;
- no long tail in MVP;
- no crash after playback stop/start;
- no stuck gain reduction;
- no stuck sidechain envelope;
- render output must match realtime within expected floating-point tolerance.

---

## 37. CPU requirements

Target:

```text
10–20 instances on a modern CPU should be realistic.
```

Requirements:

- GUI closed reduces visual/analyzer overhead;
- analyzer quality adjustable;
- avoid heavy FFT in every instance at high refresh rate;
- no processing on silence if safe;
- Eco / Normal / High analyzer modes;
- Draft / Normal / High processing modes;
- MVP default: Normal / Normal.

---

## 38. File/project structure

Canonical structure:

```text
StageMind/
  CMakeLists.txt
  README.md
  Source/
    Plugin/
      PluginProcessor.h
      PluginProcessor.cpp
      PluginEditor.h
      PluginEditor.cpp

    DSP/
      MidSideProcessor.h
      MidSideProcessor.cpp
      SpatialProcessor.h
      SpatialProcessor.cpp
      LowMonoProcessor.h
      LowMonoProcessor.cpp
      SideHighPassProcessor.h
      SideHighPassProcessor.cpp
      CorrelationMeter.h
      CorrelationMeter.cpp
      MotionProcessor.h
      MotionProcessor.cpp
      DepthProcessor.h
      DepthProcessor.cpp
      ResonanceDetector.h
      ResonanceDetector.cpp
      DynamicEQ.h
      DynamicEQ.cpp
      SidechainDetector.h
      SidechainDetector.cpp
      SidechainDynamicEQ.h
      SidechainDynamicEQ.cpp
      PseudoDoubleProcessor.h
      PseudoDoubleProcessor.cpp

    Model/
      TrackRole.h
      RoleProfile.h
      RolePresetEngine.h
      RolePresetEngine.cpp
      TriggerMode.h
      SafetyMode.h
      SidechainConflictMode.h
      Parameters.h
      PluginState.h
      PluginState.cpp

    UI/
      RoleSelector.h
      RoleSelector.cpp
      MacroControls.h
      MacroControls.cpp
      StageView.h
      StageView.cpp
      MeterView.h
      MeterView.cpp
      ResonanceListView.h
      ResonanceListView.cpp
      SidechainPanel.h
      SidechainPanel.cpp

    Tests/
      MidSideProcessorTests.cpp
      LowMonoProcessorTests.cpp
      SideHighPassProcessorTests.cpp
      CorrelationMeterTests.cpp
      SidechainDetectorTests.cpp
      DynamicEQTests.cpp
```

Do not create `StereoWidthProcessor` unless it has a clearly separate responsibility. Width belongs to `MidSideProcessor` / `SpatialProcessor` in MVP.

---

## 39. Coding style

Requirements:

- clean C++20;
- DSP separated from UI;
- no giant classes;
- no GUI logic in DSP;
- no audio thread blocking;
- no magic numbers in `processBlock`;
- constants named;
- comments for DSP decisions;
- use `juce::SmoothedValue` where appropriate;
- use APVTS;
- use RAII;
- avoid raw owning pointers;
- prefer `std::array` for fixed-size DSP data;
- preallocate buffers;
- role logic separated from processors;
- tests for math-heavy processors.

---

## 40. Testing checklist

### 40.1 Build

```text
Project configures with CMake.
Project builds.
VST3 target is generated.
No compiler warnings that indicate unsafe behavior.
```

### 40.2 Audio pass-through

```text
Plugin passes audio.
Bypass works.
Output gain works.
No clicks.
No silence when sidechain absent.
Mono and stereo inputs work.
```

### 40.3 Parameters

```text
All parameters visible to host.
Parameters save and restore.
Automation works.
Parameter order stable.
No dynamic parameters.
```

### 40.4 Role selection

```text
Role selector changes internal profile.
Role change does not reset macro values.
Role change does not click.
All selectable TrackRole values are reachable from GUI.
```

### 40.5 Width / low-end safety

```text
Width changes stereo image.
Mono Safe limits width.
Correlation responds.
Low-end mono works.
Side high-pass works.
```

### 40.6 Sidechain

```text
Sidechain bus visible.
Sidechain meter works.
Sidechain listen works.
SC Amount affects gain reduction in MVP 2.
No crash without sidechain.
```

### 40.7 Dynamic EQ

```text
Sidechain dynamic EQ reacts.
Gain reduction meter shows action.
No full-band pumping unless explicitly intended.
Max sidechain bands = 2.
```

### 40.8 Resonance

```text
Detector finds obvious narrow peaks.
Dynamic resonance suppression reduces them.
Max active resonance bands = 4.
No vector resizing in callback.
```

---

## 41. FL Studio testing checklist

### 41.1 Basic insert

```text
1. Load plugin on Mixer Insert.
2. Play audio.
3. Bypass plugin.
4. Change role.
5. Move Width.
6. Move Clean Up.
7. Save FL project.
8. Reopen FL project.
9. Confirm state restored.
```

### 41.2 Automation Clips

```text
1. Create automation clip for Width.
2. Create automation clip for Depth.
3. Create automation clip for Clean Up.
4. Create automation clip for SC Amount.
5. Render audio.
6. Confirm no clicks/pops.
```

### 41.3 Sidechain in FL

```text
1. Put StageMind on Bass Insert.
2. Route Kick to Bass using sidechain-only send.
3. Enable Sidechain in StageMind.
4. Confirm sidechain meter moves.
5. Set mode KickDucksBass.
6. Confirm low-frequency gain reduction reacts to kick.
```

### 41.4 Vocal to Guitar

```text
1. Put StageMind on Guitar.
2. Route Vocal to Guitar as sidechain-only.
3. Set Source Role: Lead Vocal.
4. Set mode VocalDucksInstrument.
5. Confirm Guitar ducks 2–5 kHz when Vocal is active.
```

### 41.5 Patcher

```text
1. Load Patcher on Mixer Insert.
2. Load StageMind inside Patcher.
3. Link Width/Depth/Clean Up to Control Surface.
4. Save Patcher preset.
5. Reload preset.
6. Confirm links survive.
```

### 41.6 Smart Disable

```text
1. Enable Smart Disable.
2. Stop playback.
3. Start playback.
4. Confirm no stuck gain reduction.
5. Confirm analyzer/motion recovers.
```

### 41.7 Multiple instances

```text
1. Load 10 instances on 10 stems.
2. Play full project.
3. Close all GUIs.
4. Confirm CPU remains stable.
5. Render project.
6. Confirm no render glitches.
```

---

## 42. Acceptance criteria

### 42.1 MVP 1 accepted when

```text
1. Project builds as VST3.
2. Plugin opens in FL Studio.
3. Main mono/stereo input/output works.
4. Optional sidechain bus exists.
5. Plugin passes audio without sidechain.
6. APVTS parameters exist and are visible in FL.
7. Parameter order matches this document.
8. Role selector exposes all selectable TrackRole values.
9. Width works through Mid/Side.
10. Mono low-end works.
11. Side high-pass works.
12. Output gain works.
13. Basic GUI works.
14. Input/output/sidechain meter placeholders exist.
15. Sidechain meter detects signal.
16. State saves and restores in FL project.
17. No memory allocation in audio callback.
18. No mutex in audio callback.
19. Zero latency is reported.
```

### 42.2 MVP 2 accepted when

```text
1. Sidechain dynamic EQ works.
2. VocalDucksInstrument mode works.
3. KickDucksBass mode works.
4. SnareDucksInstrument mode works.
5. LeadDucksPad mode works.
6. Sidechain Listen Off/SidechainOnly works.
7. Gain Reduction meter works.
8. SC Amount is automatable.
9. Attack/Release work.
10. No clicks with automation.
11. Plugin renders correctly in FL.
12. Max active sidechain bands = 2.
```

### 42.3 MVP 3 accepted when

```text
1. Local resonance detector works.
2. Detector finds narrow test peaks.
3. Dynamic resonance suppression works.
4. Max active resonance bands = 4.
5. Resonance macro controls amount.
6. Clean Up macro affects detection/amount.
7. Resonance list shown in GUI.
8. Sound does not become overprocessed at default settings.
9. No dynamic allocation in analyzer audio path.
```

### 42.4 MVP 4 accepted when

```text
1. Motion works.
2. Motion is role-limited.
3. Depth works as zero-latency wet-path effect.
4. Stage View reflects pan/depth/width.
5. Pseudo-double works for allowed roles.
6. Correlation safety reduces dangerous width.
7. Presets implemented.
```

---

## 43. Codex usage policy

Codex context window should not receive the whole product roadmap when implementing a small module.

Use this pattern:

```text
1. Give Codex the current phase task only.
2. Include canonical enums and parameter IDs if relevant.
3. Include exact changed-file expectations.
4. Ask for build/test result.
5. Ask for assumptions.
6. Do not ask it to implement future phases early.
```

Each Codex response should include:

```text
1. Changed files.
2. Build result.
3. Tests run.
4. Whether VST3 exposes required buses.
5. Whether parameters are visible to host if relevant.
6. Assumptions.
7. Next recommended task.
```

---

## 44. Codex Task 1 — MVP 1 foundation

Use this as the first implementation task.

```text
Create a JUCE CMake VST3 audio effect plugin named StageMind.

Scope:
1. Stereo main input/output.
2. Optional stereo sidechain input bus named "Sidechain".
3. Support mono/stereo main layouts. Reject surround layouts in MVP.
4. Implement APVTS parameters exactly in the canonical order from the spec:
   - role
   - width
   - depth
   - motion
   - clean_up
   - resonance
   - safety
   - output_gain
   - trigger_mode
   - sidechain_enabled
   - sidechain_mode
   - sidechain_source_role
   - sidechain_amount
   - sidechain_listen
   - sidechain_attack
   - sidechain_release
   - sidechain_range_start
   - sidechain_range_end
   - pan
   - mono_low_cutoff
   - side_highpass
   - correlation_safety_threshold
   - resonance_sensitivity
   - max_resonance_reduction
   - dynamic_eq_attack
   - dynamic_eq_release
   - motion_rate
   - pseudo_double_amount
   - presence_reduction
   - early_reflection_amount
   - analyzer_quality
   - processing_quality
   - link_enabled
   - link_group
   - link_role
   - link_source_id
   - link_target_id
   - link_mode

5. Implement model classes:
   - TrackRole
   - RoleProfile
   - RolePresetEngine
   - SafetyMode
   - TriggerMode
   - SidechainConflictMode
   - PluginState
   - Parameters

6. Implement DSP classes:
   - MidSideProcessor
   - LowMonoProcessor
   - SideHighPassProcessor
   - CorrelationMeter stub
   - SidechainDetector basic level detection
   - SpatialProcessor wrapper

7. Basic GUI:
   - Role dropdown exposing all selectable TrackRole values
   - Safety dropdown
   - Trigger dropdown
   - Width slider/knob
   - Depth slider/knob
   - Motion slider/knob
   - Clean Up slider/knob
   - Resonance slider/knob
   - Output slider/knob
   - Sidechain Enable toggle
   - Sidechain Mode dropdown
   - Sidechain Amount slider/knob
   - input meter placeholder
   - output meter placeholder
   - sidechain meter placeholder
   - current role label

8. Process audio:
   - pass-through by default
   - output gain
   - width control through Mid/Side
   - mono low-end
   - side high-pass
   - sidechain meter only
   - no dynamic EQ yet

Do not implement:
- FFT
- resonance detector
- dynamic EQ
- StageMind Link behavior
- Stage Director
- pseudo-double
- depth DSP
- motion DSP

Realtime requirements:
- no memory allocation in audio callback
- no mutex in audio callback
- sidechain absence is safe
- parameters are stable and visible
- plugin reports zero latency

Output required:
1. Changed files.
2. Build result.
3. Whether VST3 exposes sidechain input.
4. Whether parameters are visible to host.
5. Any assumptions.
6. Next recommended task.
```

---

## 45. Codex Task 2 — SidechainDynamicEQ

```text
Implement SidechainDynamicEQ for MVP 2.

Prerequisite:
MVP 1 foundation builds and loads in FL Studio.

Scope:
1. Sidechain envelope follower.
2. Mode presets:
   - VocalDucksInstrument
   - KickDucksBass
   - SnareDucksInstrument
   - LeadDucksPad
   - Custom
3. Max 2 active sidechain bands.
4. Frequency-dependent dynamic cuts.
5. Mid/Side processing per band:
   - processMid
   - processSide
6. Gain reduction meter.
7. Sidechain Listen:
   - Off
   - SidechainOnly
8. Smooth attack/release.
9. SC Amount macro.
10. Respect trigger_mode and sidechain_enabled matrix.

Do not implement:
- FFT
- local resonance detector
- StageMind Link
- full-band compressor
- lookahead

Realtime requirements:
- no allocation in processBlock
- no locks in processBlock
- zero latency
- no dynamic band containers

Output required:
1. Changed files.
2. Build result.
3. Tests run.
4. Sidechain modes verified.
5. Any assumptions.
6. Next recommended task.
```

---

## 46. Codex Task 3 — ResonanceDetector

```text
Implement local resonance detection for MVP 3.

Scope:
1. ResonanceDetector with FFT.
2. Hann window.
3. FFT size 2048 or 4096.
4. Time smoothing.
5. Log-frequency grouping.
6. Peak detection.
7. Top 4 resonance peaks using a fixed-size array.
8. Dynamic resonance suppression using DynamicEQ.
9. Resonance List in GUI.
10. Clean Up macro mapping.
11. Resonance macro mapping.

Do not implement:
- inter-instance conflict engine
- StageMind Link
- Stage Director
- automatic boosts

Realtime requirements:
- preallocate FFT buffers
- no allocation in audio callback
- no GUI dependency
- analyzer must not block audio thread

Tests:
- artificial narrow sine peaks
- noisy material with injected resonance
- max active peaks = 4
- bypass/off behavior

Output required:
1. Changed files.
2. Build result.
3. Tests run.
4. Peak detection behavior.
5. Any assumptions.
6. Next recommended task.
```

---

## 47. Codex Task 4 — Motion / Depth / Pseudo-double

```text
Implement MVP 4 spatial enhancement modules.

Scope:
1. MotionProcessor with sine LFO.
2. Role-limited motion.
3. DepthProcessor as lightweight zero-latency wet-path effect.
4. PseudoDoubleProcessor with 10–30 ms wet-path micro-delay.
5. Correlation safety integration.
6. Stage View updates for pan/depth/width.
7. Presets implemented.

Do not implement:
- full reverb
- lookahead
- linear phase processing
- latency/PDC
- StageMind Link
- Stage Director

Realtime requirements:
- no allocation in processBlock
- no locks
- dry path is not delayed
- plugin still reports zero latency

Output required:
1. Changed files.
2. Build result.
3. Tests run.
4. Correlation safety behavior.
5. Any assumptions.
6. Next recommended task.
```

---

## 48. Future task — StageMind Link

Later only.

Rules:

```text
1. No audio transfer between instances.
2. Only control data.
3. Group ID.
4. Source role.
5. Activity envelope.
6. Spectral bands later.
7. Thread-safe registry.
8. Offline render safe.
9. Project reload safe.
10. No audio-thread locks.
```

---

## 49. Future task — StageMind Director

Separate plugin:

```text
StageMind Director
```

Responsibilities:

```text
1. Show all Node instances.
2. Build 2D/3D scene.
3. Detect conflicts.
4. Suggest corrections.
5. Control Node parameters.
6. Create section presets:
   - Verse
   - Chorus
   - Bridge
   - Drop
```

Not part of Node MVP.

---

## 50. README requirements

Create `README.md`.

Include:

```text
1. What StageMind is.
2. How to build.
3. How to install VST3.
4. How to load in FL Studio.
5. How to use as insert.
6. How to set role.
7. How to route sidechain in FL Studio.
8. How to use Vocal -> Guitar.
9. How to use Kick -> Bass.
10. Known limitations.
11. MVP status.
```

---

## 51. Short user manual draft

### Basic use

```text
1. Put StageMind on a Mixer Insert.
2. Choose Role.
3. Adjust Width, Depth, Motion.
4. Use Clean Up and Resonance gently.
5. Watch Correlation.
6. Use Mono Safe if unsure.
```

### Sidechain use

```text
1. Put StageMind on the target track.
2. Route source track to target as sidechain-only in FL.
3. Enable Sidechain in StageMind.
4. Choose Source Role.
5. Choose Sidechain Mode.
6. Raise SC Amount.
7. Use Sidechain Listen if no signal appears.
```

Recommended default:

```text
Width: 50%
Depth: 30%
Motion: 0–20%
Clean Up: 30–50%
Resonance: 30–50%
Safety: Natural or Mono Safe
SC Amount: 20–50%
```

---

## 52. Final product feel

StageMind should feel like this:

```text
I put it on a stem.
I tell it what the stem is.
It puts the sound in a sensible place.
It keeps the low-end safe.
It warns me if stereo gets dangerous.
It cleans resonances carefully.
It can react to vocal/kick/snare sidechain.
It exposes simple controls to FL automation.
It does not fight me.
```

The plugin is for fast personal workflow in FL Studio, not for replacing a mixing engineer.
