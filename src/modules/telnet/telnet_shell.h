#pragma once

/**
 * @brief Инициализирует Telnet shell (TCP) для удаленных команд по Wi-Fi.
 * @details
 * - Вызывать один раз из `setup()` после `wifiInit()`.
 * - Неблокирующая.
 * - Побочные эффекты: подготавливает внутреннее состояние TCP shell.
 * - При `FEATURE_TELNET_SHELL=0` работает как no-op.
 * - Не ISR-safe.
 */
void telnetShellInit();

/**
 * @brief Периодическая обработка Telnet shell.
 * @details
 * - Вызывать в каждом проходе `loop()`.
 * - Неблокирующая: запускает сервер при подключенном Wi-Fi, обслуживает одного TCP-клиента.
 * - Поддерживает те же команды, что и USB shell.
 * - Побочные эффекты: сетевой ввод/вывод, выполнение команд (FS/NVS/reboot/sleep и т.д.).
 * - При `FEATURE_TELNET_SHELL=0` работает как no-op.
 * - Не ISR-safe.
 */
void telnetShellLoop();

