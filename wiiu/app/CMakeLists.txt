cmake_minimum_required(VERSION 3.13)
project(WiiUAchievementsNative C)

# Source-of-truth: original Windows XML, validated at build time.
find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(PORT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(PORT_GENERATOR "${CMAKE_CURRENT_SOURCE_DIR}/../tools/generate_catalog.py")
set(PORT_CATALOG "${CMAKE_CURRENT_BINARY_DIR}/catalog_generated.c")
set(PORT_XML
    "${PORT_ROOT}/achievements.xml"
    "${PORT_ROOT}/achievements-story.xml"
    "${PORT_ROOT}/achievements-planned.xml"
    "${PORT_ROOT}/achievement-presentation.xml"
)
add_custom_command(
    OUTPUT "${PORT_CATALOG}"
    COMMAND "${Python3_EXECUTABLE}" "${PORT_GENERATOR}"
            --root "${PORT_ROOT}" --out "${PORT_CATALOG}"
    DEPENDS "${PORT_GENERATOR}" ${PORT_XML}
    COMMENT "Validate XML and generate Wii U achievement catalog"
    VERBATIM
)
add_executable(WiiUAchievementsNative src/main.c "${PORT_CATALOG}")
target_include_directories(WiiUAchievementsNative PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_features(WiiUAchievementsNative PRIVATE c_std_11)
target_compile_options(WiiUAchievementsNative PRIVATE -Wall -Wextra)
wut_create_rpx(WiiUAchievementsNative)
wut_create_wuhb(WiiUAchievementsNative
    NAME "Wii U Achievements"
    SHORTNAME "Achievements"
    AUTHOR "Wii U Achievements contributors"
    ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.png"
)
