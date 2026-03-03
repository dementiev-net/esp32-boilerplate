#pragma once

/**
 * @brief Инициализирует USB CDC shell для диагностики и сервисных команд.
 * @details
 * - Вызывать один раз из `setup()` после `Serial.begin(...)`.
 * - Неблокирующая.
 * - Побочные эффекты: выводит баннер и prompt в Serial.
 * - При `FEATURE_USB_SHELL=0` работает как no-op.
 * - Не ISR-safe.
 */
void usbShellInit();

/**
 * @brief Периодическая обработка USB CDC shell.
 * @details
 * - Вызывать в каждом проходе `loop()`.
 * - Неблокирующая (построчно парсит входной поток и выполняет команды).
 * - Поддерживает команды диагностики и сервиса (`help`, `status`, `reboot`, `sleep`, работа с файлами).
 * - Побочные эффекты: операции с Serial, файловой системой, перезагрузка/переход в deep sleep.
 * - При `FEATURE_USB_SHELL=0` работает как no-op.
 * - Не ISR-safe.
 */
void usbShellLoop();

