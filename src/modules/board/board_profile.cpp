#include "board_profile.h"

#if defined(BOARD_TDISPLAY_S3)
static const BoardProfile kBoardProfile = {
    BoardId::TDisplayS3,
    "LILYGO T-Display-S3",
    { 14, 0, 15, 4 },
    { true, true, 13, 11, 12 },
    { 320, 170, 1 }
};
#elif defined(BOARD_TQT_PRO)
static const BoardProfile kBoardProfile = {
    BoardId::TQtPro,
    "LILYGO T-QT Pro",
    { 0, 47, -1, 4 },
    { false, false, -1, -1, -1 },
    { 128, 128, 2 }
};
#else
#error "Unsupported board profile. Define BOARD_TDISPLAY_S3 or BOARD_TQT_PRO."
#endif

const BoardProfile& boardGetProfile() {
    return kBoardProfile;
}
