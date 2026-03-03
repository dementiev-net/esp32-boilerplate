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
| Wi-Fi STA через `/wifi.conf` | ✓ | — | На T-QT Pro нет SD-конфига |
| NVS (Preferences) | ✓ | ✓ | `NVS_NAMESPACE=boilerplate` |
| NTP время при интернете | ✓ | ✓ | Показывается в статусе `TIME` |
| Boot preloader | ✓ | ✓ | Логотип + прогресс загрузки |

## Что включено

- **Display** — инициализация дисплея, текст, цвета, примитивы
- **WiFi** — автоподключение + неблокирующий портал настройки (WifiManager)
- **Buttons** — обработка клика, долгого нажатия и удерживания (hold repeat)
- **Time** — синхронизация точного времени через NTP при наличии интернета
- **Storage** — NVS (сохранение между перезагрузками) + SD карта
- **Power** — измерение батареи (VBAT %) и deep sleep с пробуждением по кнопке

## Быстрый старт

1. Клонировать репозиторий
2. Открыть в VS Code + PlatformIO
3. Выбрать окружение:
   - `lilygo-t-display-s3` или `lilygo-t-qt-pro`
4. Build & Upload

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

## Режимы Wi-Fi через SD-конфиг (T-Display-S3)

Для `lilygo-t-display-s3` можно задать режим Wi-Fi через файл `/wifi.conf` на SD-карте.
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

## SD карта

- `lilygo-t-display-s3`:
  - поддерживается через SDMMC 1-bit (`CMD=GPIO13`, `CLK=GPIO11`, `D0=GPIO12`);
  - рекомендуемый формат карты: `FAT32 (MBR)`, не `exFAT`.
- `lilygo-t-qt-pro`:
  - SD карта в проекте не используется (`SD:N/A` на экране).

## Статус на экране

В нижней части экрана отображаются runtime-параметры:

- `W:OK/OFF` - состояние Wi-Fi.
- `M:AUTO/AP/STA` - текущий режим Wi-Fi.
- `SD:OK/NO/N/A` - доступность SD (есть/нет/не поддерживается).
- `BTN T:x[h] B:y[h]` - состояние кнопок; суффикс `H` означает событие удерживания (hold).
- `IP:...` - IP в `STA`, `AP:<ssid>` в `AP`.
- `TIME:HH:MM:SS` - локальное время после NTP-синхронизации (`--:--:--` до синхронизации).
- `VB:xx%` - оценка заряда батареи в процентах (если ADC батареи доступен на плате).

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

## Энергосбережение

- Причина пробуждения выводится в Serial (`Wake: ...`) и на экран при старте.
- Deep sleep активируется по `Bottom LongPress`.
- Пробуждение настроено на `TOP` кнопку (RTC wake pin).
- Калибровка процентов батареи задается в `config.h`:
  - `BATTERY_PERCENT_EMPTY_MV`
  - `BATTERY_PERCENT_FULL_MV`

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

1. Для `lilygo-t-qt-pro` это ожидаемо: SD в проекте не используется (`SD:N/A`).
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
    ├── board/
    ├── power/
    ├── display/
    ├── time/
    ├── wifi/
    ├── storage/
    └── buttons/
```
