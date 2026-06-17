# StageMind Node 0.8.9 - короткая инструкция для теста

StageMind Node - VST3 insert-effect для FL Studio. Ставится на mixer insert дорожки: вокал, барабаны, бас, гитара, синты, подкладки и другие stems.

## Установка

1. Закрой FL Studio.
2. Скопируй всю папку `StageMind Node.vst3` в VST3-папку или добавь папку `dist\VST3` в FL Plugin Manager.
3. Открой FL Studio.
4. `Options > Manage plugins > Find plugins`.
5. Добавь `StageMind Node` на mixer insert.

Если Windows не даёт заменить файл VST3, закрой FL Studio. Отключение слота в FL не обязано выгружать DLL из процесса host.

## Что уже работает

- Роли дорожек: Suno Vocal, Suno Drums, Suno Bass, Suno Guitar и другие.
- Width, Depth, Motion, Clean Up, Resonance, Output.
- Локальный поиск и подавление резонансов.
- Sidechain Dynamic EQ.
- StageMind Link между несколькими инстансами.
- Director mode внутри того же плагина.
- Auto Assist.
- Director Auto correction.
- Ride Memory: Director запоминает найденные конфликты группы, показывает сколько событий найдено, сколько resolved, и может применить известную коррекцию снова.
- Управление нодами из Director: выбрать ноду, увидеть её значения, двигать Pan/Depth и крутить основные ручки выбранной ноды.

## Быстрый тест Link/Director

1. Поставь StageMind Node на несколько дорожек.
2. Ничего не включай вручную: новые инстансы уже стартуют с `Link On`, `Group 1`, `Auto`.
3. Поставь ещё один StageMind Node и переключи `Mode` в `Director`.
4. Director тоже должен быть на `Group 1`.
5. До playback Director должен видеть установленные Nodes.
6. Во время playback должны появляться activity/conflicts, а после auto-коррекции строки могут стать `resolved`.
7. В Director справа должна появиться строка `Memory auto` или `Memory learning`.
8. Если конфликт найден, счётчик events должен расти. После применения часть событий может стать `resolved`.

## Быстрый тест управления из Director

1. В Director нажми на точку ноды или на линию Width вокруг неё.
2. В средней панели должен появиться выбранный Node.
3. Потяни точку влево/вправо: звук целевой дорожки должен менять панораму.
4. Потяни точку вверх/вниз: у целевой ноды должен меняться Depth.
5. Покрути в средней панели Pan, Width, Depth, Motion, Clean Up, Resonance, SC Amount.
6. Меняться должна выбранная нода, не Director.
7. Сохрани и открой проект заново: значения должны восстановиться.

## Что проверить

- Новый Node сразу имеет `Link` включён, `Group = 1`, `Auto = Auto`.
- Director видит idle Nodes до playback.
- Director показывает Ride Memory status: `Memory auto`, количество events и resolved.
- `Learn Mix` включает ручное обучение памяти.
- `Clear Memory` сбрасывает найденные события.
- Save/reopen сохраняет Ride Memory events в проекте.
- При playback live activity не пропадает.
- Bass/Drums conflict может включить StageMind Link sidechain без ручного FL routing.
- Save/reopen сохраняет значения.
- Render/export не ломается.
- Director click/drag управляет Pan/Depth целевой ноды.
- Средняя панель Director управляет выбранной нодой.
- Конфликты/статусы не должны мигать слишком быстро; resolved-строки должны оставаться читаемыми.

## Ограничения

- Плагин не передаёт audio между инстансами, только control data.
- Полной памяти по таймлайну песни пока нет. Ride Memory запоминает отношения ролей в группе, но пока не знает точные такты/части песни.
- Director не определяется автоматически по master insert. Нужно выбрать `Mode: Director`.
- Если FL реально освобождает ресурсы инстанса, Node исчезает из Director/Link до повторного запуска.
- Плагин не может сам выгрузить VST3 DLL из FL Studio. Для замены файла обычно нужно закрыть FL.
