set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include("files.cmake")

set(MACOSX_BUNDLE_ICON_FILE AppIcon)
set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/resources/AppIcon.icns")
set_source_files_properties(${app_icon_macos} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

string(TIMESTAMP TODAY "%d-%m-%Y")

qt6_wrap_ui(WRAPPED_UI_FILES ${UI_FILES})
qt6_add_resources(RCC_SOURCES ${QRC_FILES})

qt_add_executable(BMFGen
  MANUAL_FINALIZATION
  ${SOURCE_FILES}
  ${UI_FILES}
  ${QRC_FILES}
  ${RCC_SOURCES}
  ${WRAPPED_UI_FILES}
  ${app_icon_macos}
)

target_compile_features(BMFGen PRIVATE cxx_std_20)

set(CMAKE_MACOSX_BUNDLE_NAME "BMFGen")
set(CMAKE_MACOSX_BUNDLE_BUNDLE_VERSION "${BMFGEN_VERSION_MAJOR}.${BMFGEN_VERSION_MINOR}.${BMFGEN_VERSION_REVISION}")
set(CMAKE_MACOSX_BUNDLE_COPYRIGHT "Copyright (C) Cemalettin Dervis")

set_property(TARGET BMFGen PROPERTY MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in")

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  enable_language(RC)
  target_sources(BMFGen PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.rc)
endif ()

set_property(TARGET BMFGen PROPERTY VS_GLOBAL_ApplicationIcon "${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.ico")

target_compile_options(BMFGen PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/WX /W4 /wd4702>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

if (NOT MSVC)
  target_compile_options(BMFGen PRIVATE -Wno-unused-function)
endif ()

target_link_libraries(
  BMFGen PRIVATE
  Qt6::Widgets
  Microsoft.GSL::GSL
)

target_sources(BMFGen PRIVATE ${QM_FILES})

target_include_directories(BMFGen PRIVATE src)

target_compile_definitions(BMFGen PRIVATE
  "-DBMFGEN_AUTHOR=\"Cemalettin Dervis\""
  "-DBMFGEN_NAME=\"${CMAKE_PROJECT_NAME}\""
  "-DBMFGEN_DISPLAY_NAME=\"${CMAKE_PROJECT_DESCRIPTION}\""
  "-DBMFGEN_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR}"
  "-DBMFGEN_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR}"
  "-DBMFGEN_WEBSITE=\"https://bmfgen.com\""
  "-DBMFGEN_COMPILER_NAME=\"${CMAKE_CXX_COMPILER_ID}\""
  "-DBMFGEN_COMPILER_VERSION=\"${CMAKE_CXX_COMPILER_VERSION}\""
  "-DBMFGEN_BUILT_FOR=\"${CMAKE_SYSTEM_NAME}\""
  "-DBMFGEN_BUILT_ON=\"${CMAKE_HOST_SYSTEM}\""
  "-DBMFGEN_LINKED_QT_VERSION=\"${Qt6Widgets_VERSION}\""
  "-DBMFGEN_BUILD_DATE=\"${TODAY}\""
)

set_target_properties(BMFGen PROPERTIES
  MACOSX_BUNDLE_GUI_IDENTIFIER com.cemdervis.bmfgen
  MACOSX_BUNDLE_BUNDLE_VERSION ${BMFGEN_VERSION_MAJOR}.${BMFGEN_VERSION_MINOR}.${BMFGEN_VERSION_REVISION}
  MACOSX_BUNDLE_SHORT_VERSION_STRING ${BMFGEN_VERSION_MAJOR}.${BMFGEN_VERSION_MINOR}.${BMFGEN_VERSION_REVISION}
  XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/BMFGen.entitlements"
  MACOSX_BUNDLE TRUE
  WIN32_EXECUTABLE TRUE
)

target_compile_definitions(BMFGen PRIVATE "-DQT_NO_DEBUG_OUTPUT" "-DQT_NO_INFO_OUTPUT" "-DQT_NO_WARNING_OUTPUT")

qt_finalize_executable(BMFGen)

install(TARGETS BMFGen
  BUNDLE DESTINATION .
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set(deploy_tool_options_arg "")
if (APPLE)
  set(deploy_tool_options_arg --hardened-runtime)
elseif (WIN32)
  set(deploy_tool_options_arg --no-compiler-runtime)
endif ()

qt_generate_deploy_app_script(
  TARGET BMFGen
  OUTPUT_SCRIPT deploy_script
  NO_UNSUPPORTED_PLATFORM_ERROR
  DEPLOY_TOOL_OPTIONS ${deploy_tool_options_arg}
)

install(SCRIPT ${deploy_script})

