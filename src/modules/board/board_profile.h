#pragma once

#include <stdint.h>

/**
 * @brief Идентификатор целевой платы.
 */
enum class BoardId : uint8_t {
    TDisplayS3,
    TQtPro
};

/**
 * @brief GPIO-пины, используемые платой в проекте.
 */
struct BoardPins {
    /// @brief GPIO верхней кнопки.
    uint8_t buttonTop;
    /// @brief GPIO нижней кнопки.
    uint8_t buttonBottom;
    /// @brief GPIO включения питания/подсветки дисплея (`-1`, если не используется).
    int8_t displayPowerPin;
    /// @brief GPIO ADC для измерения напряжения батареи (`-1`, если не выведен).
    int8_t batteryAdcPin;
};

/**
 * @brief Настройки SD-подсистемы платы.
 */
struct BoardSdConfig {
    bool supported;
    bool useSdMmc1Bit;
    int8_t sdmmcCmdPin;
    int8_t sdmmcClkPin;
    int8_t sdmmcD0Pin;
};

/**
 * @brief Параметры экрана текущей платы.
 */
struct BoardDisplayConfig {
    uint16_t width;
    uint16_t height;
    uint8_t rotation;
};

/**
 * @brief Полный профиль платы для board-specific поведения.
 */
struct BoardProfile {
    BoardId id;
    const char* label;
    BoardPins pins;
    BoardSdConfig sd;
    BoardDisplayConfig display;
};

/**
 * @brief Возвращает статический профиль текущей платы сборки.
 * @details
 * - Выбор профиля делается на этапе компиляции по `BOARD_TDISPLAY_S3`/`BOARD_TQT_PRO`.
 * - Структура используется как единая точка board-specific параметров.
 */
const BoardProfile& boardGetProfile();
