# StageMind Node 0.11.3 - короткая инструкция для теста

StageMind Node - VST3 insert-effect для FL Studio. Ставится на mixer insert дорожки: вокал, барабаны, бас, гитара, синты, подкладки и другие stems.

## Установка

1. Закрой FL Studio.
2. Скопируй всю папку `StageMind Node.vst3` в VST3-папку или добавь папку `dist\VST3` в FL Plugin Manager.
3. Открой FL Studio.
4. `Options > Manage plugins > Find plugins`.
5. Добавь `StageMind Node` на mixer insert.

Если Windows не даёт заменить файл VST3, закрой FL Studio. Отключение слота внутри FL не всегда выгружает DLL из процесса host.

## Что работает

- Роли дорожек: vocal, drums, bass, guitar, synth, pad, FX и другие.
- Width, Depth, Motion, Clean Up, Resonance, Doubler, Output Trim.
- Stage Gain: `Off`, `Static`, `Ride`.
- Stage Gain meter modes: `dBFS`, `VU`, `RMS`, `LUFS M`, `LUFS S`, `LUFS I`.
- Stage Gain `Ride`: плавно подтягивает уровень к target.
- Stage Gain `Static`: кнопка `Analyze` запоминает фиксированную коррекцию.
- Director `Analyze All`: отправляет Static Analyze всем нодам выбранной группы.
- Look-ahead ceiling limiter после Stage Gain.
- Диагностический CSV-лог с решениями автоматики и снапшотами нод.
- Защита от частого переключения ducking-режимов одной ноды.
- Director Balance трогает Output Trim только у нод в `Stage Gain: Ride`.
- Director Balance ограничен диапазоном `-3 dB` / `+3 dB`.
- Если выключить `SC Enable` на уже настроенном ducking, Auto не включает его обратно.
- Локальный поиск и подавление резонансов.
- Sidechain Dynamic EQ.
- StageMind Link между несколькими инстансами.
- Director mode внутри того же плагина.
- Auto Assist, Director Auto correction, Ride Memory, Balance Timeline Memory.
- Save/reopen сохраняет параметры, память и Static Stage Gain hold.

## Быстрый тест Stage Gain

1. Поставь StageMind Node на дорожку с неровной громкостью.
2. Выбери `Stage Gain: Ride`.
3. Поставь `Target` около `-18 dB`, `Threshold` около `-12 VU`, `Ceiling` около `-1 dB`.
4. Запусти playback и смотри строку статуса: должно появляться `Ride +... dB` или `Ride -... dB`.
5. Переключи meter mode: `dBFS`, `VU`, `RMS`, `LUFS M/S/I`. Реакция будет немного отличаться.
6. Переключи `Stage Gain` в `Static`.
7. Во время звучащего участка нажми `Analyze`.
8. Статус должен перейти в `Hold ... dB`.
9. Сохрани проект, открой заново и проверь, что hold остался.

## Быстрый тест Analyze All

1. Поставь StageMind Node на несколько дорожек.
2. У всех должна быть `Group = 1` и `Link` включён.
3. Поставь ещё один StageMind Node и выбери `Mode: Director`.
4. В Director нажми `Analyze All` во время playback.
5. Ноды группы должны перейти в Static Stage Gain и поймать свой `Hold`.

## Быстрый тест limiter

1. На одной ноде выбери `Stage Gain: Ride`.
2. Поставь `Ceiling`, например, `-6 dB`.
3. Подай громкий материал.
4. Output peak не должен уходить сильно выше ceiling.
5. Плагин теперь имеет небольшую фиксированную latency для look-ahead. Это нормально.

## Что проверить дополнительно

- Новый Node стартует с `Link On`, `Group = 1`, `Auto = Auto`.
- Director видит idle Nodes до playback.
- В playback появляются activity/conflicts.
- Render/export не ломается.
- Resize окна сохраняется после закрытия/открытия UI.
- Director click/drag управляет Pan/Depth целевой ноды.

## Диагностический лог

Во время работы плагин пишет CSV сюда:

`C:\Users\<user>\Documents\StageMind\Logs\StageMind-session-YYYYMMDD-HHMMSS.csv`

После теста пришли последний файл из этой папки и коротко напиши, что звучало неправильно: например "гитара не получила авто-коррекцию", "бас стал слишком тихим", "Director видел конфликт, но не применил". В логе будут видны роли, группы, режимы Stage Gain, auto decisions, Link-команды, уровни, correlation, transport PPQ/BPM и события Analyze/Analyze All. У Director snapshot теперь должны быть заполнены `link_nodes` и `active_peers`.

## Ограничения

- LUFS modes - практичные Stage Gain meters, не сертифицированный broadcast QC.
- `Analyze All` анализирует текущий момент playback, а не весь трек offline.
- Ceiling limiter - safety ceiling, не полноценный mastering limiter с oversampling/ISP.
- Плагин не передаёт audio между инстансами, только control data.
- Director не выбирается автоматически по master insert. Нужно выбрать `Mode: Director`.
