# ESP32 Boilerplate

Стартовый проект для быстрого начала разработки под ESP32 платы LILYGO.

## Поддерживаемые платы

| Плата | Дисплей | Кнопки | SD карта |
|-------|---------|--------|----------|
| LILYGO T-Display-S3 | ST7789 1.9" 320x170 | 2 | ✓ |
| LILYGO T-QT Pro | GC9A01 0.85" 128x128 | 2 | — |

## Board Capabilities Matrix

| Возможность | T-Display-S3 | T-QT Pro | Примечание |
|------------|--------------|----------|------------|
| Дисплей | ✓ ST7789 320x170 | ✓ GC9A01 128x128 | Через `TFT_eSPI` |
| Кнопки | ✓ `TOP=GPIO14`, `BOTTOM=GPIO0` | ✓ `TOP=GPIO0`, `BOTTOM=GPIO47` | Click / LongPress / Hold |
| SD карта | ✓ SDMMC 1-bit | — | В проекте SD только для T-Display-S3 |
| Wi-Fi AP/AUTO (portal) | ✓ | ✓ | WiFiManager |
| Wi-Fi STA через `/wifi.conf` | ✓ (SD) | ✓ (LittleFS, опционально) | Единый формат файла |
| OTA update | ✓ (по умолчанию) | ○ (опционально) | `FEATURE_OTA` |
| BLE advertising + GATT | ✓ (по умолчанию) | ○ (опционально) | `FEATURE_BLE` |
| USB CDC shell | ✓ (по умолчанию) | ✓ (по умолчанию) | `FEATURE_USB_SHELL` |
| Telnet shell (TCP) | ✓ (по умолчанию) | ✓ (по умолчанию) | `FEATURE_TELNET_SHELL` |
| HTTP JSON demo (internet) | ✓ (по умолчанию) | ○ (опционально) | `FEATURE_NET_HTTP` |
| Watchdog + reset diagnostics | ✓ (по умолчанию) | ✓ (по умолчанию) | `FEATURE_WATCHDOG` |
| LittleFS fallback | ○ (опционально) | ✓ (по умолчанию) | `FEATURE_LITTLEFS` |
| NVS (Preferences) | ✓ | ✓ | `NVS_NAMESPACE=boilerplate` |
| NTP время при интернете | ✓ | ✓ | Показывается в статусе `TIME` |
| Boot preloader | ✓ | ✓ | Логотип + прогресс загрузки |
| Яркость подсветки (PWM) | ✓ | ✓ | Команда shell `brightness 0..100` |
| Startup self-test | ✓ | ✓ | `PASS/FAIL` в Serial и на экране |

## Что включено

- **Display** — инициализация дисплея, текст, цвета, примитивы
- **Display Brightness** — PWM-управление подсветкой (`0..100%`) с выводом в статус
- **WiFi** — автоподключение + неблокирующий портал настройки (WifiManager)
- **Buttons** — обработка клика, долгого нажатия и удерживания (hold repeat)
- **Time** — синхронизация точного времени через NTP при наличии интернета
- **Storage** — NVS + файловый backend: SD (T-Display-S3) или LittleFS fallback (T-QT Pro)
- **Power** — измерение батареи (VBAT %) и deep sleep с пробуждением по кнопке
- **OTA** — обновление прошивки по Wi-Fi (push OTA, без собственного сервера, опционально для lightweight-профиля)
- **BLE** — advertising + GATT service (battery %, uptime, version, опционально для lightweight-профиля)
- **USB shell** — минимальный командный интерфейс по CDC для диагностики и сервисных операций
- **Telnet shell** — удаленный командный интерфейс по Wi-Fi (`TCP:23`) с теми же командами, что USB shell
- **Net demo** — периодический HTTP JSON запрос из интернета и вывод значения на экран/в Serial
- **Reliability** — watchdog loop-задачи и диагностика причины reset
- **Self-test** — стартовая проверка display/buttons/wifi/storage/battery с итогом PASS/FAIL

## Быстрый старт

1. Клонировать репозиторий
2. Открыть в VS Code + PlatformIO
3. Выбрать окружение:
   - `lilygo-t-display-s3` или `lilygo-t-qt-pro`
4. Build & Upload

## Профили фич по платам

По умолчанию в `platformio.ini` заданы такие профили:

- `lilygo-t-display-s3`: `FEATURE_OTA=1`, `FEATURE_BLE=1`, `FEATURE_LITTLEFS=0`, `FEATURE_USB_SHELL=1`, `FEATURE_TELNET_SHELL=1`, `FEATURE_NET_HTTP=1`, `FEATURE_WATCHDOG=1` (full-профиль).
- `lilygo-t-qt-pro`: `FEATURE_OTA=0`, `FEATURE_BLE=0`, `FEATURE_LITTLEFS=1`, `FEATURE_USB_SHELL=1`, `FEATURE_TELNET_SHELL=1`, `FEATURE_NET_HTTP=0`, `FEATURE_WATCHDOG=1` (lightweight-профиль с файловым fallback).

При необходимости фичи можно включать обратно через `build_flags`:

```ini
-DFEATURE_OTA=1
-DFEATURE_BLE=1
-DFEATURE_LITTLEFS=1
-DFEATURE_USB_SHELL=1
-DFEATURE_TELNET_SHELL=1
-DFEATURE_NET_HTTP=1
-DFEATURE_WATCHDOG=1
```

Примечание для `lilygo-t-qt-pro`: если включаешь `FEATURE_BLE=1`, добавь `h2zero/NimBLE-Arduino@^1.4.2`
в `lib_deps` этого окружения (`platformio.ini`), так как по умолчанию зависимость подключена только для `t-display-s3`.

## Первый запуск

При первом запуске плата поднимает точку доступа:
- **SSID:** `ESP32-Boilerplate`
- **Password:** `12345678`

Подключись и открой `192.168.4.1` для настройки WiFi.

## Smoke Checklist

### T-Display-S3

1. После прошивки показывается boot-preloader (логотип + прогресс не менее `BOOT_SPLASH_MIN_MS`).
2. Основной экран запускается без мерцания статус-блока.
3. Кнопки:
   - короткое нажатие и long press видны в Serial;
   - при удержании появляется `H` в строке `BTN`.
4. Wi-Fi:
   - в AP режиме видна сеть `ESP32-Boilerplate`, портал открывается на `192.168.4.1`;
   - в STA режиме отображается `IP:...`.
5. NTP:
   - при наличии интернета строка `TIME` меняется с `--:--:--` на текущее время.
6. SD (если карта вставлена):
   - статус `SD:OK`;
   - для нераспознанной карты — проверить `FAT32 (MBR)`.
7. Power:
   - в Serial выводится `Wake: ...` и `Wake button: TOP GPIO...`;
   - долгий клик `BOTTOM` переводит в deep sleep;
   - пробуждение — кнопкой `TOP`.
   - в Serial есть строки `"[System] Reset: ..."` и `"[System] WDT: ..."` после старта.
8. USB shell:
   - в Serial после загрузки есть строка `"[USB] CDC shell ready. Type 'help'."`;
   - команда `status` печатает текущие runtime-параметры.
9. Telnet shell:
   - при подключенном Wi-Fi в Serial есть строка `"[Telnet] Ready: ip=... port=23"`;
   - подключение по TCP (`telnet <ip> 23` или `nc <ip> 23`) открывает prompt `telnet>`.
10. Net demo:
   - по умолчанию включен (`FEATURE_NET_HTTP=1`);
   - при включении и наличии интернета в Serial появляется `"[NET] amount=..."`,
     а в статус-панели — строка `NET:...`.
11. Brightness:
   - команда `brightness` показывает текущую яркость;
   - команда `brightness 30` применяет новый уровень подсветки;
   - в статус-панели в строке `TIME/VB` выводится `BL:xx%`.
12. Self-test:
   - в Serial после старта есть строки `"[SelfTest] ..."` и итог `"[SelfTest] RESULT: PASS/FAIL"`;
   - на экране отображается блок `SELFTEST:PASS/FAIL`.

### T-QT Pro

1. После прошивки показывается boot-preloader (логотип + прогресс не менее `BOOT_SPLASH_MIN_MS`).
2. Основной экран запускается без мерцания статус-блока.
3. Кнопки:
   - короткое нажатие и long press видны в Serial;
   - при удержании появляется `H` в строке `BTN`.
4. Wi-Fi:
   - в AP режиме видна сеть `ESP32-Boilerplate`, портал открывается на `192.168.4.1`;
   - в STA режиме отображается `IP:...`.
5. NTP:
   - при наличии интернета строка `TIME` меняется с `--:--:--` на текущее время.
6. SD:
   - ожидаемое состояние `SD:N/A` (в проекте не используется на T-QT Pro).
7. Power:
   - в Serial выводится `Wake: ...` и `Wake button: TOP GPIO...`;
   - долгий клик `BOTTOM` переводит в deep sleep;
   - пробуждение — кнопкой `TOP`.
   - в Serial есть строки `"[System] Reset: ..."` и `"[System] WDT: ..."` после старта.
8. USB shell:
   - в Serial после загрузки есть строка `"[USB] CDC shell ready. Type 'help'."`;
   - команда `status` печатает текущие runtime-параметры.
9. Telnet shell:
   - при подключенном Wi-Fi в Serial есть строка `"[Telnet] Ready: ip=... port=23"`;
   - подключение по TCP (`telnet <ip> 23` или `nc <ip> 23`) открывает prompt `telnet>`.
10. Net demo:
   - по умолчанию выключен (`FEATURE_NET_HTTP=0`) для экономии флеша;
   - при включении и наличии интернета в Serial появляется `"[NET] amount=..."`,
     а в статус-панели — строка `NET:...`.
11. Brightness:
   - команда `brightness` показывает текущую яркость;
   - команда `brightness 30` применяет новый уровень подсветки;
   - в статус-панели в строке `TIME/VB` выводится `BL:xx%`.
12. Self-test:
   - в Serial после старта есть строки `"[SelfTest] ..."` и итог `"[SelfTest] RESULT: PASS/FAIL"`;
   - на экране отображается блок `SELFTEST:PASS/FAIL`.

## Режимы Wi-Fi через файловый конфиг (`/wifi.conf`)

Можно задать режим Wi-Fi через файл `/wifi.conf`:
- `lilygo-t-display-s3`: файл читается с SD-карты.
- `lilygo-t-qt-pro`: файл читается из LittleFS (если `FEATURE_LITTLEFS=1`).

Если файла нет, используется поведение по умолчанию (`AUTO` через WiFiManager).

Пример `wifi.conf`:

```ini
mode=sta
sta_ssid=MyRouter
sta_password=MyPassword
ap_ssid=ESP32-Boilerplate
ap_password=12345678
```

- `mode=ap` - всегда поднимать точку доступа.
- `mode=sta` - подключаться к роутеру по `sta_ssid/sta_password`.
- `mode=auto` - fallback на WiFiManager.

Ключи `wifi.conf`:

| Ключ | Назначение | Обязательно |
|------|------------|-------------|
| `mode` | Режим Wi-Fi: `ap`, `sta`, `auto` | да |
| `sta_ssid` | SSID роутера для `mode=sta` | для `sta` |
| `sta_password` | Пароль роутера для `mode=sta` | для `sta` |
| `ap_ssid` | SSID точки доступа для `mode=ap` | нет |
| `ap_password` | Пароль AP (минимум 8 символов) | нет |

Где размещать `wifi.conf`:
- `lilygo-t-display-s3`: в корне SD-карты (`/wifi.conf`).
- `lilygo-t-qt-pro`: в `data/wifi.conf`, затем загрузить FS-образ:

```bash
/Users/dementev/.platformio/penv/bin/pio run -e lilygo-t-qt-pro -t uploadfs
```

## LittleFS (T-QT Pro)

- В `lilygo-t-qt-pro` включен `board_build.filesystem = littlefs`.
- При старте `storage` монтирует LittleFS автоматически как fallback-файловую систему.
- API `sdReadFile`/`sdWriteFile`/`sdAppendFile` работает через активный backend:
  - SD на T-Display-S3;
  - LittleFS на T-QT Pro.

## SD карта

- `lilygo-t-display-s3`:
  - поддерживается через SDMMC 1-bit (`CMD=GPIO13`, `CLK=GPIO11`, `D0=GPIO12`);
  - рекомендуемый формат карты: `FAT32 (MBR)`, не `exFAT`.
- `lilygo-t-qt-pro`:
  - SD карта в проекте не используется (`SD:N/A` на экране), вместо нее используется LittleFS fallback.

## Статус на экране

В нижней части экрана отображаются runtime-параметры:

- `W:OK/OFF` - состояние Wi-Fi.
- `M:AUTO/AP/STA` - текущий режим Wi-Fi.
- `SD:OK/NO/N/A` - доступность SD (есть/нет/не поддерживается).
- `BTN T:x[h] B:y[h]` - состояние кнопок; суффикс `H` означает событие удерживания (hold).
- `IP:...` - IP в `STA`, `AP:<ssid>` в `AP`.
- `TIME:HH:MM:SS` - локальное время после NTP-синхронизации (`--:--:--` до синхронизации).
- `VB:xx%` - оценка заряда батареи в процентах (если ADC батареи доступен на плате).
- `BL:xx%` - текущая яркость подсветки дисплея.
- `NET:...` - результат demo HTTP JSON запроса (`NET:OFF` без интернета, `NET:ERR` при ошибке).

## Кнопки (hold repeat)

- `Click` - короткое нажатие.
- `LongPress` - событие один раз после порога `BUTTON_LONG_PRESS_MS`.
- `Hold` - периодическое событие после `LongPress` с интервалом `BUTTON_HOLD_REPEAT_MS`.
- `Bottom LongPress` в runtime переводит устройство в deep sleep.

Порог и интервалы настраиваются в `config.h`:
- `BUTTON_DEBOUNCE_MS`
- `BUTTON_LONG_PRESS_MS`
- `BUTTON_HOLD_REPEAT_MS`

## Точное время (NTP)

- Время синхронизируется автоматически, когда есть подключение к интернету (`Wi-Fi connected`, не AP-only).
- Серверы и таймзона настраиваются в `config.h`:
  - `NTP_TIMEZONE`
  - `NTP_SERVER_1`, `NTP_SERVER_2`, `NTP_SERVER_3`
  - `NTP_RETRY_MS`

## Net demo (HTTP JSON)

- Модуль периодически выполняет HTTP GET к `NET_DEMO_URL` и извлекает строковое поле `NET_DEMO_JSON_KEY`.
- По умолчанию используется endpoint `https://api.coinbase.com/v2/prices/BTC-USD/spot` и ключ `amount` (курс BTC/USD).
- Результат выводится:
  - в Serial: `"[NET] amount=..."`
  - на экран: строка `NET:...` в нижней статус-панели.

Параметры в `config.h`:
- `NET_DEMO_URL`
- `NET_DEMO_JSON_KEY`
- `NET_DEMO_REQUEST_INTERVAL_MS`
- `NET_DEMO_HTTP_TIMEOUT_MS`
- `NET_DEMO_UI_MAX_VALUE_LEN`

## Яркость подсветки

- Подсветка управляется через PWM (LEDC) по пину `TFT_BL` из board-specific setup.
- Команда shell:
  - `brightness` — показать текущее значение;
  - `brightness <0..100>` — установить яркость в процентах.
- Параметры в `config.h`:
  - `DISPLAY_BRIGHTNESS_DEFAULT_PERCENT`
  - `DISPLAY_BRIGHTNESS_PWM_FREQ_HZ`
  - `DISPLAY_BRIGHTNESS_PWM_BITS`
  - `DISPLAY_BRIGHTNESS_PWM_CHANNEL`

## Энергосбережение

- Причина пробуждения выводится в Serial (`Wake: ...`) и на экран при старте.
- Deep sleep активируется по `Bottom LongPress`.
- Пробуждение настроено на `TOP` кнопку (RTC wake pin).
- Калибровка процентов батареи задается в `config.h`:
  - `BATTERY_PERCENT_EMPTY_MV`
  - `BATTERY_PERCENT_FULL_MV`

## Надежность (watchdog/reset)

- При старте в Serial печатается причина reset:
  - `"[System] Reset: <label> (<code>)"`.
- Включен task watchdog loop-задачи:
  - `"[System] WDT: on timeout=<sec>s"`.
- Модуль watchdog регулярно feed'ится из `loop()` и защищает от зависаний.
- Параметры в `config.h`:
  - `FEATURE_WATCHDOG`
  - `WATCHDOG_TIMEOUT_SEC`
  - `WATCHDOG_FEED_INTERVAL_MS`

## Startup self-test

- После инициализации модулей в `setup()` выполняется стартовый self-test:
  - display
  - buttons
  - wifi
  - storage (NVS + file backend)
  - battery (или `N/A`, если не поддерживается платой)
- Результат выводится:
  - в Serial: подробный отчет + `"[SelfTest] RESULT: PASS/FAIL"`
  - на экран: краткий блок `SELFTEST:PASS/FAIL`
- Управление через `config.h`:
  - `FEATURE_SELF_TEST`

## OTA (без сервера)

Проект использует push OTA: после первой прошивки по USB обновления можно отправлять по Wi-Fi.
Для `lilygo-t-qt-pro` в lightweight-профиле OTA по умолчанию выключен (`FEATURE_OTA=0`).

Что происходит в рантайме:
- OTA сервер поднимается автоматически после подключения к сети.
- В Serial виден статус готовности: `"[OTA] Ready: host=... ip=... port=3232"`.
- Во время обновления печатается прогресс и ошибки.

Параметры OTA в `config.h`:
- `OTA_PORT`
- `OTA_PASSWORD`
- `OTA_HOSTNAME_PREFIX`

Как пользоваться OTA:

1. Один раз прошей плату по USB обычным способом.
2. Подключи плату и компьютер к одной Wi-Fi сети.
3. Открой Serial Monitor (`115200`) и дождись строки:
   - `"[OTA] Ready: host=... ip=... port=3232"`.
4. Возьми IP из этой строки и собери свежий `firmware.bin`.
5. Запусти `espota.py` с этим IP, портом и паролем.
6. После окончания OTA плата перезагрузится автоматически.

Если в Serial нет `"[OTA] Ready..."`, значит OTA еще не поднят:
- нет подключения к сети;
- Wi-Fi в процессе reconnection;
- неверно настроен режим Wi-Fi.

Пример OTA-загрузки (T-Display-S3):

```bash
/Users/dementev/.platformio/penv/bin/pio run -e lilygo-t-display-s3
python ~/.platformio/packages/tool-espotapy/espota.py \
  -i 192.168.1.120 \
  -p 3232 \
  -a boilerplate \
  -f .pio/build/lilygo-t-display-s3/firmware.bin
```

Пример OTA-загрузки (T-QT Pro):

```bash
/Users/dementev/.platformio/penv/bin/pio run -e lilygo-t-qt-pro
python ~/.platformio/packages/tool-espotapy/espota.py \
  -i 192.168.1.121 \
  -p 3232 \
  -a boilerplate \
  -f .pio/build/lilygo-t-qt-pro/firmware.bin
```

## BLE

Для `lilygo-t-qt-pro` в lightweight-профиле BLE по умолчанию выключен (`FEATURE_BLE=0`).

При старте поднимается BLE advertising с именем:
- `boilerplate-tdisplay-s3` для T-Display-S3;
- `boilerplate-tqt-pro` для T-QT Pro.

В Serial отображаются события:
- `"[BLE] Advertising started: ..."`
- `"[BLE] Client connected"`
- `"[BLE] Client disconnected"`

GATT service (`BLE_SERVICE_UUID`) содержит характеристики:
- `BLE_CHAR_BATTERY_UUID` — заряд батареи в `%` (или `--` если недоступен);
- `BLE_CHAR_UPTIME_UUID` — uptime в секундах;
- `BLE_CHAR_VERSION_UUID` — текущая `APP_VERSION`.

## USB CDC shell

Минимальный shell работает через `Serial` (USB CDC) и доступен после загрузки:
- `"[USB] CDC shell ready. Type 'help'."`

Команды:
- `help`
- `version`
- `status`
- `wifi`
- `net`
- `brightness [0..100]`
- `storage`
- `cat <path>`
- `write <path> <text>`
- `append <path> <text>`
- `heap`
- `uptime`
- `reboot`
- `sleep`

`status` теперь дополнительно выводит:
- reset reason (`reset=<label>(<code>)`);
- состояние watchdog (`wdt=on/off`).

## Telnet shell (Wi-Fi)

При `FEATURE_TELNET_SHELL=1` shell доступен по TCP после подключения к Wi-Fi:
- в Serial появляется строка `"[Telnet] Ready: ip=... port=23"`;
- подключение: `telnet <ip_платы> 23` или `nc <ip_платы> 23`;

Prompt в сессии: `telnet>`.
Поддерживаются те же команды, что и в USB shell.

## Troubleshooting

### Wi-Fi portal не открывается

1. Проверь, что устройство действительно в AP-режиме:
   - на экране `M:AP` и `IP:AP:<ssid>`;
   - сеть `ESP32-Boilerplate` видна в списке Wi-Fi.
2. Подключись к AP и открой `192.168.4.1` вручную в браузере.
3. На телефоне отключи мобильный интернет на время настройки (иначе captive-портал может не всплыть).
4. Если используется `/wifi.conf` с `mode=sta`, портал может не подниматься:
   - для настройки через портал поставь `mode=auto` или `mode=ap`;
   - проверь корректность `sta_ssid`/`sta_password`.

### Дисплей пустой/черный

1. Проверь выбранное окружение PlatformIO:
   - `lilygo-t-display-s3` для T-Display-S3;
   - `lilygo-t-qt-pro` для T-QT Pro.
2. Пересобери и перепрошей проект после смены окружения.
3. Проверь питание: используй дата-кабель USB (не только зарядный).
4. Если прошивка стартует, но экран пустой, открой `Serial Monitor` (`115200`) и проверь сообщения инициализации.

### SD карта не определяется (`SD:NO`)

1. Для `lilygo-t-qt-pro` это ожидаемо: SD в проекте не используется (`SD:N/A`), используется LittleFS fallback.
2. Для `lilygo-t-display-s3` проверь формат карты:
   - `FAT32 (MBR)`;
   - не `exFAT`.
3. Проверь адаптер и пины SDMMC 1-bit:
   - `CMD=GPIO13`, `CLK=GPIO11`, `D0=GPIO12`.
4. Если карта раньше работала в другой прошивке/устройстве:
   - переформатируй в `FAT32 (MBR)`;
   - проверь карту на другом устройстве.

## Локальная проверка

```bash
/Users/dementev/.platformio/penv/bin/pio test -e native
/Users/dementev/.platformio/penv/bin/pio run -e lilygo-t-display-s3 -e lilygo-t-qt-pro
```

`native`-тесты покрывают:

- парсинг `wifi.conf` (`modules/wifi/wifi_config.cpp`);
- state-machine кнопок (`modules/buttons/buttons_logic.cpp`);
- логику записи/append в storage (`modules/storage/storage_io.cpp`).

## Структура проекта
```
src/
├── main.cpp
├── config.h
└── modules/
    ├── app/
    ├── ble/
    ├── board/
    ├── ota/
    ├── power/
    ├── display/
    ├── time/
    ├── telnet/
    ├── shell/
    ├── usb/
    ├── net/
    ├── system/
    ├── wifi/
    ├── storage/
    └── buttons/
```
