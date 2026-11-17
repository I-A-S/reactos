
set(WINSRV_DIR "${REACTOS_SOURCE_DIR}/win32ss/user/winsrv")
set(CONSRV_DIR "${WINSRV_DIR}/consrv")

set(SRC_FILES
  conhost.c

  ${CONSRV_DIR}/frontends/gui/text.c
  ${CONSRV_DIR}/frontends/gui/conwnd.c
  ${CONSRV_DIR}/frontends/gui/guiterm.c
  ${CONSRV_DIR}/frontends/gui/graphics.c
  ${CONSRV_DIR}/frontends/gui/fullscreen.c
  ${CONSRV_DIR}/frontends/gui/guisettings.c
)

#
# Explicitly enable MS extensions to be able to use unnamed (anonymous) nested structs.
#
if(NOT MSVC)
  set_source_files_properties(${SRC_FILES} PROPERTIES COMPILE_OPTIONS "-fms-extensions")
endif()

remove_definitions(-D_WIN32_WINNT=0x502)
add_definitions(-D_WIN32_WINNT=0x600)

add_executable(conhost
  conhost.rc
  ${SRC_FILES}
)

target_include_directories(conhost PRIVATE
  ${CONSRV_DIR}
  ${WINSRV_DIR}
  ${CONSRV_DIR}/include
  ${REACTOS_SOURCE_DIR}/win32ss/include
  ${REACTOS_SOURCE_DIR}/sdk/include/reactos/subsys
)

set_module_type(conhost win32gui UNICODE)

add_importlibs(conhost msvcrt kernel32 ntdll user32 gdi32)

add_cd_file(TARGET conhost DESTINATION reactos/system32 FOR all)
