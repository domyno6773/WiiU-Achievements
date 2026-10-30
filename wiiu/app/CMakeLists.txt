cmake_minimum_required(VERSION 3.13)
project(WiiUAchievementsNative C ASM)

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
    COMMENT "Validate and compile original achievement catalog"
    VERBATIM
)

# Build high-quality theme assets at authoring time. The assembly contains
# binary .incbin references, and the final WUHB embeds all PNGs and TTFs.
set(VIS_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../tools/generate_gpu_assets.py")
set(VIS_DIR "${CMAKE_CURRENT_BINARY_DIR}/gpu-assets")
set(VIS_ASM "${VIS_DIR}/visual_assets.S")
add_custom_command(
    OUTPUT "${VIS_ASM}"
    COMMAND "${Python3_EXECUTABLE}" "${VIS_SCRIPT}"
            --root "${PORT_ROOT}" --out "${VIS_DIR}"
    DEPENDS "${VIS_SCRIPT}"
            "${PORT_ROOT}/assets/fonts/Nunito-Regular.ttf"
            "${PORT_ROOT}/assets/fonts/Nunito-Bold.ttf"
            "${PORT_ROOT}/assets/fonts/Nunito-ExtraBold.ttf"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Bake pixel-accurate UI cards, ocean, medals, shadows and original Nunito"
    VERBATIM
)

add_executable(WiiUAchievementsNative
    src/gpu_ui.c
    "${PORT_CATALOG}"
    "${VIS_ASM}"
)
target_include_directories(WiiUAchievementsNative PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${DEVKITPRO}/portlibs/wiiu/include/SDL2"
)
target_compile_features(WiiUAchievementsNative PRIVATE c_std_11)
target_compile_options(WiiUAchievementsNative PRIVATE -Wall -Wextra -O2)
target_link_libraries(WiiUAchievementsNative PRIVATE
    SDL2_image SDL2_ttf SDL2
    freetype harfbuzz png z bz2 brotlidec brotlicommon m
)
wut_create_rpx(WiiUAchievementsNative)
wut_create_wuhb(WiiUAchievementsNative
    NAME "Wii U Achievements"
    SHORTNAME "Achievements"
    AUTHOR "Wii U Achievements contributors"
    ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.png"
)
