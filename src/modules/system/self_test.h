#pragma once

#include <Arduino.h>

/**
 * @brief Результат стартового self-test основных подсистем.
 * @details
 * - Формируется функцией `selfTestRun()` после инициализации модулей в `setup()`.
 * - Поля `...Ok` отражают итог проверки конкретной подсистемы.
 */
struct SelfTestResult {
    bool passed = false;
    bool displayOk = false;
    bool buttonsOk = false;
    bool wifiOk = false;
    bool storageNvsOk = false;
    bool storageFileOk = false;
    bool storageOk = false;
    bool batterySupported = false;
    /// @brief true, если на старте получено валидное чтение VBAT.
    bool batteryOk = false;
    bool wifiConnected = false;
    /// @brief true, если в момент self-test активен USB CDC-хост.
    bool usbHostConnected = false;
    int batteryMillivolts = -1;
    int batteryPercent = -1;
    bool topPressed = false;
    bool bottomPressed = false;
};

/**
 * @brief Выполняет стартовый self-test основных модулей.
 * @return Структура с результатами проверки подсистем.
 * @details
 * - Вызывать один раз из `setup()` после `storageInit()`, `batteryInit()`, `buttonsInit()`, `wifiInit()`.
 * - Потенциально блокирующая: содержит чтение ADC батареи.
 * - Побочные эффекты: только чтение состояния модулей и GPIO, без записи в NVS/FS.
 * - Ограничения по платам: батарея может быть `N/A`, если ADC VBAT не поддерживается
 *   профилем платы или валидное чтение VBAT не получено.
 * - При активном USB CDC-хосте батарейный статус в отчете помечается как `USB`.
 * - Не ISR-safe.
 */
SelfTestResult selfTestRun();

/**
 * @brief Печатает результат self-test в Serial.
 * @param result Результат, полученный из `selfTestRun()`.
 * @details
 * - Вызывать из `setup()` после `Serial.begin(...)`.
 * - Потенциально блокирующая из-за `Serial.print*`.
 * - Побочные эффекты: вывод диагностического отчета в Serial.
 * - Не ISR-safe.
 */
void selfTestPrintReport(const SelfTestResult& result);

/**
 * @brief Выводит краткий итог self-test на экран.
 * @param result Результат, полученный из `selfTestRun()`.
 * @param y Координата Y (пиксели) верхней строки отчета.
 * @details
 * - Вызывать из `setup()` после `displayInit()`.
 * - Неблокирующая в рамках короткой отрисовки нескольких строк.
 * - Побочные эффекты: перерисовка области экрана в зоне self-test отчета.
 * - Ограничения по платам: формат строк сокращается для малых дисплеев.
 * - Не ISR-safe.
 */
void selfTestRenderReport(const SelfTestResult& result, int y);
