
add_executable(conhost conhost.cpp conhost.rc)
set_module_type(conhost win32gui UNICODE)
add_importlibs(conhost msvcrt kernel32 ntdll)
add_cd_file(TARGET conhost DESTINATION reactos/system32 FOR all)
