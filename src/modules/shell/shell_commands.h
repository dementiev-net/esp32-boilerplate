#pragma once

#include <Arduino.h>

/**
 * @brief Выполняет одну строку команды shell и печатает ответ в заданный поток.
 * @param rawLine Входная строка команды (может содержать пробелы и аргументы).
 * @param out Поток вывода (`Serial`, `WiFiClient` и т.д.).
 * @param sourceTag Короткий тег источника (`"usb"`, `"telnet"`), используется в префиксах логов.
 * @details
 * - Вызывать из обработчиков shell-интерфейсов после получения завершенной строки.
 * - Потенциально блокирующая для команд с I/O (`cat`, `write`, `append`) и сетевых/системных операций.
 * - Побочные эффекты: доступ к FS/NVS, изменение яркости, перезагрузка, переход в deep sleep.
 * - Не ISR-safe.
 */
void shellExecuteCommand(const String& rawLine, Print& out, const char* sourceTag);

