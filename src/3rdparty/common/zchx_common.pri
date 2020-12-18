unix{
    ZCHX_COMMON_SYS     = unix
} else {
    ZCHX_COMMON_SYS     = win
}
ZCHX_COMMON_PATH        = $${PWD}/$${ZCHX_COMMON_SYS}

exists( $${ZCHX_COMMON_PATH} ) {
    ZCHX_COMMON_HEADERS   = $${ZCHX_COMMON_PATH}/include
    ZCHX_COMMON_LIBS      = $${ZCHX_COMMON_PATH}/lib
    INCLUDEPATH += $${ZCHX_COMMON_HEADERS}
    unix{
        LIBS +=-L$${ZCHX_COMMON_LIBS} -lzchx_common
        ZCHX_COMMON_install.files += $${ZCHX_COMMON_PATH}/lib/libzchx_common.so.1
    } else {
        LIBS +=-L$${ZCHX_COMMON_LIBS} -llibzchx_common
        ZCHX_COMMON_install.files += $${ZCHX_COMMON_PATH}/lib/zchx_common.dll

    }
    ZCHX_COMMON_install.path = $${IDE_APP_PATH}/
    INSTALLS += ZCHX_COMMON_install
    warning("find ZCHX_COMMON_PATH DIR =======================" + $$ZCHX_COMMON_PATH)
} else {
    warning("Cann't find ZCHX_COMMON_PATH DIR =======================" + $$ZCHX_COMMON_PATH)
}
