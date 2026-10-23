cmake_minimum_required(VERSION 3.25)
project(win16_style VERSION 0.7.0 LANGUAGES C RC)
if(NOT MINGW OR NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
  message(FATAL_ERROR "The product profile requires native MinGW-w64 GCC C17")
endif()
if(NOT WIN16STYLE_ARCH MATCHES "^(x86|x64)$")
  message(FATAL_ERROR "Select WIN16STYLE_ARCH=x86 or x64")
endif()
if((WIN16STYLE_ARCH STREQUAL "x86" AND NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
   OR (WIN16STYLE_ARCH STREQUAL "x64" AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8))
  message(FATAL_ERROR "Use the matching x86/x64 MinGW-w64 GCC toolchain")
endif()
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
add_compile_options(-Wall -Wextra -Werror -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-lto)
add_link_options(-static -static-libgcc -fno-lto)
set(WIN16STYLE_X86_BUILD "${CMAKE_SOURCE_DIR}/build/.ninja/x86" CACHE PATH "Matching x86 fixture build")
set(core_sources
  attachment.c control_scope.c controls.c dllmain.c font.c layout.c
  menu_bar.c menu_bar_paint.c menu_items.c menu_layout.c menu_paint.c
  menu_scope.c native_api.c scroll_layout.c scroll_track.c theme.c win16_style.c)
list(TRANSFORM core_sources PREPEND "src/ntw16s-dll/")
if(WIN16STYLE_ARCH STREQUAL "x86")
  set(dll_name ntw16s32)
else()
  set(dll_name ntw16s64)
endif()
add_library(ntw16s SHARED ${core_sources}
  src/ntw16s-dll/${dll_name}.def src/ntw16s-dll/version.rc)
set_target_properties(ntw16s PROPERTIES OUTPUT_NAME ${dll_name} PREFIX "")
target_include_directories(ntw16s PRIVATE src ${CMAKE_SOURCE_DIR})
target_compile_definitions(ntw16s PRIVATE NTW16S_BUILD UNICODE _UNICODE _WIN32_WINNT=0x0A00)
get_filename_component(gcc_bin "${CMAKE_C_COMPILER}" DIRECTORY)
execute_process(COMMAND "${CMAKE_C_COMPILER}" -print-prog-name=as
  OUTPUT_VARIABLE gcc_assembler OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
file(TO_CMAKE_PATH "${gcc_assembler}" gcc_assembler)
set(subclass_import "${CMAKE_CURRENT_BINARY_DIR}/libntw16s-subclass.a")
if(WIN16STYLE_ARCH STREQUAL "x86")
  set(subclass_def "${CMAKE_SOURCE_DIR}/src/ntw16s-dll/injection/subclass32.def")
else()
  set(subclass_def "${CMAKE_SOURCE_DIR}/src/ntw16s-dll/injection/subclass64.def")
endif()
add_custom_command(OUTPUT "${subclass_import}"
  COMMAND "${gcc_bin}/dlltool.exe" -d "${subclass_def}" -l "${subclass_import}" --kill-at --as "${gcc_assembler}"
  DEPENDS "${subclass_def}" VERBATIM)
add_custom_target(ntw16s-subclass DEPENDS "${subclass_import}")
add_dependencies(ntw16s ntw16s-subclass)
target_link_libraries(ntw16s PRIVATE "${subclass_import}" user32 gdi32 comctl32 dwmapi uxtheme)
set(injection_root src/ntw16s-dll/injection)
add_library(ntw16s-installer STATIC ${injection_root}/context.c ${injection_root}/installer.c
  ${injection_root}/staging.c ${injection_root}/helper.c)
target_include_directories(ntw16s-installer PUBLIC src)
target_compile_definitions(ntw16s-installer PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)
set(minhook_root ${injection_root}/minhook/src)
if(WIN16STYLE_ARCH STREQUAL "x86")
  set(hde_source ${minhook_root}/hde/hde32.c)
else()
  set(hde_source ${minhook_root}/hde/hde64.c)
endif()
target_sources(ntw16s PRIVATE ${injection_root}/entry.c ${injection_root}/window_titles.c
  ${injection_root}/create_process.c ${injection_root}/unwind.c ${injection_root}/startup_hooks.c
  ${minhook_root}/buffer.c ${minhook_root}/trampoline.c ${hde_source})
target_link_libraries(ntw16s PRIVATE ntw16s-installer)
set(explicit_manifest_options)
# Some native MinGW distributions add a manifest from the GCC endfile spec.
# Preserve that toolchain's other CRT endfiles; this target owns its manifest.
execute_process(COMMAND "${CMAKE_C_COMPILER}" -dumpspecs OUTPUT_VARIABLE gcc_specs
  COMMAND_ERROR_IS_FATAL ANY)
string(REGEX MATCH "\\*endfile:\n([^\n]+)" endfile_match "${gcc_specs}")
set(endfile_spec "${CMAKE_MATCH_1}")
if(endfile_spec MATCHES "default-manifest\\.o")
  string(REPLACE "%{!shared:%:if-exists(default-manifest.o%s)}" "" explicit_endfile "${endfile_spec}")
  if(explicit_endfile MATCHES "default-manifest\\.o")
    message(FATAL_ERROR "Unrecognized GCC default manifest spec; explicit manifest must not be merged")
  endif()
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/explicit-manifest.specs" "*endfile:\n${explicit_endfile}\n\n")
  set(explicit_manifest_options "-specs=${CMAKE_CURRENT_BINARY_DIR}/explicit-manifest.specs")
endif()
if(WIN16STYLE_ARCH STREQUAL "x64")
  add_executable(demo WIN32 src/demo-exe/main.c src/demo-exe/gallery.c src/demo-exe/version.rc)
  target_compile_definitions(demo PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)

  target_link_libraries(demo PRIVATE user32)
  target_include_directories(demo PRIVATE src ${CMAKE_SOURCE_DIR})
  target_link_options(demo PRIVATE ${explicit_manifest_options})
  add_executable(run16s src/run16s-exe/main.c src/run16s-exe/version.rc)
  target_compile_definitions(run16s PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)

  target_link_libraries(run16s PRIVATE ntw16s-installer)
  target_include_directories(run16s PRIVATE ${CMAKE_SOURCE_DIR})
endif()
include(CTest)
if(BUILD_TESTING)
  find_program(test_python python REQUIRED)
  add_executable(wait-target tests/run16s-exe/wait_target.c)
  target_link_options(wait-target PRIVATE -municode)
  if(WIN16STYLE_ARCH STREQUAL "x64")
    add_executable(wait-gui-target WIN32 tests/run16s-exe/wait_target.c)
    target_compile_definitions(wait-gui-target PRIVATE WAIT_GUI)
    target_link_options(wait-gui-target PRIVATE -municode)
    add_test(NAME launch/wait COMMAND ${test_python} ${CMAKE_SOURCE_DIR}/tests/run16s-exe/wait.py
      $<TARGET_FILE:run16s> ${WIN16STYLE_X86_BUILD}/ntw16s32.dll $<TARGET_FILE:ntw16s>
      ${WIN16STYLE_X86_BUILD}/wait-target.exe $<TARGET_FILE:wait-target> $<TARGET_FILE:wait-gui-target>)
    set_tests_properties(launch/wait PROPERTIES TIMEOUT 190 RUN_SERIAL TRUE)
  endif()
  add_executable(launch-unwind tests/run16s-exe/unwind.c ${injection_root}/unwind.c)
  target_include_directories(launch-unwind PRIVATE src)
  add_executable(launch-startup-hooks tests/run16s-exe/startup_hooks.c
    ${injection_root}/startup_hooks.c ${minhook_root}/buffer.c ${minhook_root}/trampoline.c ${hde_source})
  target_include_directories(launch-startup-hooks PRIVATE src)
  add_test(NAME launch/unwind COMMAND launch-unwind)
  add_test(NAME launch/startup-hooks COMMAND launch-startup-hooks)
  set_tests_properties(launch/unwind launch/startup-hooks PROPERTIES TIMEOUT 20 RUN_SERIAL TRUE)
  add_executable(creation-contract tests/integration/creation_contract.c)

  target_compile_definitions(creation-contract PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)
  target_link_libraries(creation-contract PRIVATE user32 gdi32 comctl32)
  add_executable(rollback-contract tests/integration/rollback_contract.c)

  target_compile_definitions(rollback-contract PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)
  target_link_libraries(rollback-contract PRIVATE user32 gdi32 comctl32)
  add_executable(propagate-target tests/integration/propagate_target.c)

  target_compile_definitions(propagate-target PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)
  target_link_libraries(propagate-target PRIVATE user32 gdi32 comctl32)
  add_executable(launch-target tests/integration/automatic_target.c tests/integration/automatic_target.rc)
  target_compile_definitions(launch-target PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)

  target_link_libraries(launch-target PRIVATE user32 gdi32 comctl32)
  add_executable(launch-payload tests/run16s-exe/payload.c)
  target_link_libraries(launch-payload PRIVATE ntw16s-installer)

  add_test(NAME launch/payload COMMAND launch-payload)
  add_executable(launch-staging tests/run16s-exe/staging.c)
  target_link_libraries(launch-staging PRIVATE ntw16s-installer)

  add_test(NAME launch/staging COMMAND launch-staging --parent)
  set_tests_properties(launch/payload launch/staging PROPERTIES TIMEOUT 20 RUN_SERIAL TRUE)
  if(WIN16STYLE_ARCH STREQUAL "x64")
    add_test(NAME demo/boundary COMMAND ${test_python} ${CMAKE_SOURCE_DIR}/tests/demo-exe/boundary.py $<TARGET_FILE:demo>)
    set_tests_properties(demo/boundary PROPERTIES TIMEOUT 20)
    add_executable(demo-observe tests/demo-exe/observe.c)
    target_compile_definitions(demo-observe PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)

    target_link_libraries(demo-observe PRIVATE user32 gdi32)
    add_test(NAME demo/pair COMMAND demo-observe $<TARGET_FILE:run16s> $<TARGET_FILE:ntw16s>
      ${WIN16STYLE_X86_BUILD}/ntw16s32.dll $<TARGET_FILE:demo>)
    set_tests_properties(demo/pair PROPERTIES TIMEOUT 100 RUN_SERIAL TRUE)
    add_test(NAME launch/cli COMMAND ${test_python} ${CMAKE_SOURCE_DIR}/tests/run16s-exe/cli.py $<TARGET_FILE:run16s>)
    set_tests_properties(launch/cli PROPERTIES TIMEOUT 60)
    add_executable(propagate-basic tests/integration/propagate_basic.c)

    target_compile_definitions(propagate-basic PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)
    add_test(NAME propagate/basic COMMAND propagate-basic $<TARGET_FILE:run16s>
      ${WIN16STYLE_X86_BUILD}/ntw16s32.dll $<TARGET_FILE:ntw16s>
      ${WIN16STYLE_X86_BUILD}/propagate-target.exe $<TARGET_FILE:propagate-target>)
    set_tests_properties(propagate/basic PROPERTIES TIMEOUT 320 RUN_SERIAL TRUE)
    add_executable(creation-contract-driver tests/integration/creation_contract_driver.c)

    target_compile_definitions(creation-contract-driver PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)
    add_test(NAME propagate/contract COMMAND creation-contract-driver $<TARGET_FILE:run16s>
      ${WIN16STYLE_X86_BUILD}/ntw16s32.dll $<TARGET_FILE:ntw16s>
      ${WIN16STYLE_X86_BUILD}/creation-contract.exe $<TARGET_FILE:creation-contract>)
    set_tests_properties(propagate/contract PROPERTIES TIMEOUT 520 RUN_SERIAL TRUE)
    add_executable(rollback-contract-driver tests/integration/rollback_contract_driver.c)

    target_compile_definitions(rollback-contract-driver PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)
    add_test(NAME propagate/rollback COMMAND rollback-contract-driver $<TARGET_FILE:run16s>
      ${WIN16STYLE_X86_BUILD}/ntw16s32.dll $<TARGET_FILE:ntw16s>
      ${WIN16STYLE_X86_BUILD}/rollback-contract.exe $<TARGET_FILE:rollback-contract>)
    set_tests_properties(propagate/rollback PROPERTIES TIMEOUT 320 RUN_SERIAL TRUE)
    add_executable(launch-automatic tests/integration/automatic_launch.c)

    target_compile_definitions(launch-automatic PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)
    add_test(NAME launch/automatic COMMAND launch-automatic $<TARGET_FILE:run16s>
      ${WIN16STYLE_X86_BUILD}/ntw16s32.dll $<TARGET_FILE:ntw16s>
      ${WIN16STYLE_X86_BUILD}/launch-target.exe $<TARGET_FILE:launch-target>)
    set_tests_properties(launch/automatic PROPERTIES TIMEOUT 100 RUN_SERIAL TRUE)
    add_test(NAME launch/dialogs COMMAND launch-automatic $<TARGET_FILE:run16s>
      ${WIN16STYLE_X86_BUILD}/ntw16s32.dll $<TARGET_FILE:ntw16s>
      ${WIN16STYLE_X86_BUILD}/launch-target.exe $<TARGET_FILE:launch-target> dialogs)
    set_tests_properties(launch/dialogs PROPERTIES TIMEOUT 520 RUN_SERIAL TRUE)
    add_test(NAME launch/messages COMMAND launch-automatic $<TARGET_FILE:run16s>
      ${WIN16STYLE_X86_BUILD}/ntw16s32.dll $<TARGET_FILE:ntw16s>
      ${WIN16STYLE_X86_BUILD}/launch-target.exe $<TARGET_FILE:launch-target> messages)
    set_tests_properties(launch/messages PROPERTIES TIMEOUT 520 RUN_SERIAL TRUE)
  endif()
  add_executable(core-abi tests/ntw16s-dll/abi.c)
  target_include_directories(core-abi PRIVATE src)
  target_compile_definitions(core-abi PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)

  add_test(NAME core/abi COMMAND core-abi $<TARGET_FILE:ntw16s>)
  set_tests_properties(core/abi PROPERTIES TIMEOUT 20)
  function(core_fixture target source)
    add_executable(${target} tests/ntw16s-dll/${source})
    target_include_directories(${target} PRIVATE src)
    target_compile_definitions(${target} PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)

    target_link_libraries(${target} PRIVATE ntw16s user32 gdi32 comctl32 uxtheme)
  endfunction()
  add_executable(core-menu-cancel tests/ntw16s-dll/win16_style_menu_cancel.c)
  target_include_directories(core-menu-cancel PRIVATE src)
  target_compile_definitions(core-menu-cancel PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)
  target_link_options(core-menu-cancel PRIVATE -municode)
  target_link_libraries(core-menu-cancel PRIVATE user32 gdi32)
  add_test(NAME core/menu-cancel COMMAND core-menu-cancel $<TARGET_FILE:ntw16s>)
  set_tests_properties(core/menu-cancel PROPERTIES TIMEOUT 40 RUN_SERIAL TRUE)
  core_fixture(core-menu-dispatch win16_style_menu_dispatch.c)
  core_fixture(core-edit-clipboard win16_style_edit_clipboard.c)
  add_test(NAME core/menu-dispatch COMMAND core-menu-dispatch)
  add_test(NAME core/edit-clipboard COMMAND core-edit-clipboard)
  set_tests_properties(core/edit-clipboard PROPERTIES SKIP_RETURN_CODE 77)
  set(test_common_sources ${core_sources})
  list(REMOVE_ITEM test_common_sources src/ntw16s-dll/dllmain.c src/ntw16s-dll/controls.c src/ntw16s-dll/win16_style.c)
  add_library(test-painter-common STATIC ${test_common_sources})
  target_include_directories(test-painter-common PRIVATE src)
  target_compile_definitions(test-painter-common PRIVATE NTW16S_BUILD UNICODE _UNICODE _WIN32_WINNT=0x0A00)

  foreach(kind IN ITEMS control-present dc-failure)
    if(kind STREQUAL "control-present")
      set(fault_file win16_style_control_present.c)
      set(other_painter src/ntw16s-dll/win16_style.c)
    else()
      set(fault_file win16_style_dc_failure.c)
      set(other_painter src/ntw16s-dll/controls.c)
    endif()
    add_executable(core-${kind} tests/ntw16s-dll/${fault_file} ${other_painter})
    target_include_directories(core-${kind} PRIVATE src)
    target_compile_definitions(core-${kind} PRIVATE NTW16S_BUILD UNICODE _UNICODE _WIN32_WINNT=0x0A00)

    target_link_libraries(core-${kind} PRIVATE test-painter-common "${subclass_import}" user32 gdi32 comctl32 dwmapi uxtheme)
    # These fixtures compile the real attachment/painter objects directly, so
    # their executable also needs the same v5-compatible ordinal import.
    add_dependencies(core-${kind} ntw16s-subclass)
    add_test(NAME core/${kind} COMMAND core-${kind})
  endforeach()
  add_test(NAME core/control-present-negative COMMAND ${test_python} ${CMAKE_SOURCE_DIR}/tests/ntw16s-dll/painter_negative.py $<TARGET_FILE:core-control-present>)
  set_tests_properties(core/control-present-negative PROPERTIES TIMEOUT 40 RUN_SERIAL TRUE)
  add_executable(core-scroll-native tests/ntw16s-dll/win16_style_scroll_api.c)
  target_compile_definitions(core-scroll-native PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)

  add_test(NAME core/scroll-native COMMAND core-scroll-native)
  set_tests_properties(core/menu-dispatch core/edit-clipboard core/control-present core/dc-failure core/scroll-native PROPERTIES TIMEOUT 100 RUN_SERIAL TRUE)
  core_fixture(core-runtime runtime.c)
  core_fixture(core-frames frames.c)
  core_fixture(core-font win16_style_font.c)
  core_fixture(core-tabs win16_style_list_tabs.c)
  core_fixture(core-lifetime win16_style_control_lifetime.c)
  core_fixture(core-default win16_style_default_button.c)
  core_fixture(core-terminal ntw16s_terminal.c)
  target_sources(core-terminal PRIVATE tests/ntw16s-dll/terminal.rc)
  target_link_options(core-terminal PRIVATE ${explicit_manifest_options})
  core_fixture(core-sizebox win16_style_sizebox.c)
  add_executable(core-geometry tests/ntw16s-dll/win16_style_scroll_geometry.c
    src/ntw16s-dll/scroll_layout.c src/ntw16s-dll/scroll_track.c)
  target_include_directories(core-geometry PRIVATE src)
  target_compile_definitions(core-geometry PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)

  add_test(NAME core/runtime COMMAND core-runtime)
  add_test(NAME core/runtime-native COMMAND core-runtime --native)
  add_test(NAME core/frames COMMAND core-frames)
  add_test(NAME core/font COMMAND core-font)
  add_test(NAME core/font-dpi96 COMMAND core-font --dpi96)
  add_test(NAME core/tabs COMMAND core-tabs)
  add_test(NAME core/lifetime COMMAND core-lifetime)
  add_test(NAME core/default COMMAND core-default $<TARGET_FILE:ntw16s>)
  add_test(NAME core/terminal COMMAND core-terminal $<TARGET_FILE:ntw16s>)
  add_test(NAME core/sizebox-native COMMAND core-sizebox)
  add_test(NAME core/sizebox-styled COMMAND core-sizebox --styled)
  add_test(NAME core/geometry COMMAND core-geometry)
  set_tests_properties(core/runtime core/runtime-native core/frames core/font core/font-dpi96 core/tabs core/lifetime
    core/default core/terminal core/sizebox-native core/sizebox-styled core/geometry
    PROPERTIES TIMEOUT 100 RUN_SERIAL TRUE)
  add_executable(launch-failures tests/run16s-exe/failures.c)
  target_link_libraries(launch-failures PRIVATE ntw16s-installer)

  add_test(NAME launch/failures COMMAND launch-failures ${WIN16STYLE_X86_BUILD}/launch-failures.exe)
  set_tests_properties(launch/failures PROPERTIES TIMEOUT 50 RUN_SERIAL TRUE)
  if(WIN16STYLE_ARCH STREQUAL "x64")
    add_test(NAME launch/long-path COMMAND ${test_python} ${CMAKE_SOURCE_DIR}/tests/run16s-exe/long_path.py
      $<TARGET_FILE:run16s> ${WIN16STYLE_X86_BUILD}/ntw16s32.dll $<TARGET_FILE:ntw16s>
      ${WIN16STYLE_X86_BUILD}/launch-failures.exe $<TARGET_FILE:launch-failures>)
    set_tests_properties(launch/long-path PROPERTIES TIMEOUT 70 RUN_SERIAL TRUE)
    add_test(NAME launch/cli-failures COMMAND ${test_python} ${CMAKE_SOURCE_DIR}/tests/run16s-exe/cli_failures.py
      $<TARGET_FILE:run16s> ${WIN16STYLE_X86_BUILD}/ntw16s32.dll $<TARGET_FILE:ntw16s>
      ${WIN16STYLE_X86_BUILD}/launch-target.exe $<TARGET_FILE:launch-target>)
    set_tests_properties(launch/cli-failures PROPERTIES TIMEOUT 130 RUN_SERIAL TRUE)
  endif()
endif()

# Entry-point encoding is explicit; native `main` fixtures keep their C entry.
foreach(target IN ITEMS demo run16s creation-contract rollback-contract propagate-target
    launch-target launch-staging demo-observe propagate-basic creation-contract-driver
    rollback-contract-driver launch-automatic core-abi core-runtime core-frames core-default
    core-terminal core-dc-failure launch-failures)
  if(TARGET ${target})
    target_link_options(${target} PRIVATE -municode)
  endif()
endforeach()
