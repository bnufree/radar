unix{
    ZCHX_PROTOBUF_SYS     = unix
} else {
    ZCHX_PROTOBUF_SYS     = win
}
ZCHX_PROTOBUF_PATH        = $${PWD}/$${ZCHX_PROTOBUF_SYS}

exists( $${ZCHX_PROTOBUF_PATH} ) {
    ZCHX_PROTOBUF_HEADERS   = $${ZCHX_PROTOBUF_PATH}/include
    ZCHX_PROTOBUF_LIBS      = $${ZCHX_PROTOBUF_PATH}/lib
    INCLUDEPATH += $${ZCHX_PROTOBUF_HEADERS}
    unix{
        LIBS += -L$${ZCHX_PROTOBUF_LIBS} -lprotobuf
        ProtoBuf_install.files += $${ZCHX_PROTOBUF_PATH}/lib/libprotobuf.so*
        ProtoBuf_install.path = $${IDE_APP_PATH}/
        INSTALLS += ProtoBuf_install
    } else {
        LIBS += -L$${ZCHX_PROTOBUF_LIBS} -llibprotobuf
        ProtoBuf_install.files += $${ZCHX_PROTOBUF_PATH}/bin/libprotobuf-9.dll
        ProtoBuf_install.path = $${IDE_APP_PATH}/
        INSTALLS += ProtoBuf_install
    }
    warning("find ZCHX_PROTOBUF_PATH DIR =======================" + $$ZCHX_PROTOBUF_PATH)
} else {
    warning("Cann't find ZCHX_PROTOBUF_PATH DIR =======================" + $$ZCHX_PROTOBUF_PATH)
}
