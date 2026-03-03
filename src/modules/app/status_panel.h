#pragma once

#include "runtime_state.h"

/**
 * @brief Сбрасывает внутренний кэш строк status-панели.
 * @details Вызывать при старте UI перед первым `statusPanelRender`.
 * - Неблокирующая (работает только с RAM-кэшем).
 * - Не ISR-safe.
 */
void statusPanelReset();

/**
 * @brief Отрисовывает нижний status-блок построчно без полного redraw области.
 * @param state Актуальный runtime-снимок.
 * @param force true для принудительной перерисовки всех строк.
 * @details
 * - Вызывать из `loop()` после обработки событий.
 * - Потенциально блокирующая на время SPI-отрисовки (дисплейные операции).
 * - Побочные эффекты: изменяет содержимое экрана в нижней зоне status-панели
 *   и дополнительной строке `NET` над ней.
 * - Ограничения по платам: геометрия панели зависит от board profile (размер дисплея).
 * - Не ISR-safe.
 */
void statusPanelRender(const RuntimeSnapshot& state, bool force = false);
