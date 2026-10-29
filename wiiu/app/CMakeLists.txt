cmake_minimum_required(VERSION 3.13)
project(WiiUAchievementsNative C)

add_executable(WiiUAchievementsNative src/main.c)

target_compile_features(WiiUAchievementsNative PRIVATE c_std_11)
target_compile_options(WiiUAchievementsNative PRIVATE -Wall -Wextra)

# wut is provided by the devkitPro Wii U CMake toolchain.
wut_create_rpx(WiiUAchievementsNative)
wut_create_wuhb(WiiUAchievementsNative
    NAME "Wii U Achievements"
    SHORTNAME "Achievements"
    AUTHOR "Wii U Achievements contributors"
    ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.png"
)
