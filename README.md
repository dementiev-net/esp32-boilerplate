# ESP32 Boilerplate

Стартовый проект для быстрого начала разработки под ESP32 платы LILYGO.

## Поддерживаемые платы

| Плата | Дисплей | Кнопки | SD карта |
|-------|---------|--------|----------|
| LILYGO T-Display-S3 | ST7789 1.9" 320x170 | 2 | ✓ |
| LILYGO T-QT Pro | GC9A01 0.85" 128x128 | 2 | — |

## Что включено

- **Display** — инициализация дисплея, текст, цвета, примитивы
- **WiFi** — автоподключение + неблокирующий портал настройки (WifiManager)
- **Buttons** — обработка клика и долгого нажатия
- **Storage** — NVS (сохранение между перезагрузками) + SD карта

## Быстрый старт

1. Клонировать репозиторий
2. Открыть в VS Code + PlatformIO
3. Выбрать окружение (`lilygo-t-display-s3` или `lilygo-t-qt-pro`)
4. Build & Upload

## Первый запуск

При первом запуске плата поднимает точку доступа:
- **SSID:** `ESP32-Boilerplate`
- **Password:** `12345678`

Подключись и открой `192.168.4.1` для настройки WiFi.

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
- `BTN T:x B:y` - текущее состояние кнопок.
- `IP:...` - IP в `STA`, `AP:<ssid>` в `AP`.

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
    ├── display/
    ├── wifi/
    ├── storage/
    └── buttons/
```
