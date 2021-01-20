unix{
    ZCHX_ZMQ_SYS     = unix
} else {
    ZCHX_ZMQ_SYS     = win
}
ZCHX_ZMQ_PATH        = $${PWD}/$${ZCHX_ZMQ_SYS}

exists( $${ZCHX_ZMQ_PATH} ) {
    ZCHX_ZMQ_HEADERS   = $${ZCHX_ZMQ_PATH}/include
    ZCHX_ZMQ_LIBS      = $${ZCHX_ZMQ_PATH}/lib
    INCLUDEPATH += $${ZCHX_ZMQ_HEADERS}
    unix{
        LIBS +=-L$${ZCHX_ZMQ_LIBS} -lzmq
        Zmq_install.files += $${ZCHX_ZMQ_PATH}/lib/libzmq.so*
        Zmq_install.path = $$IDE_APP_PATH
        INSTALLS += Zmq_install
    } else {
        LIBS +=-L$${ZCHX_ZMQ_LIBS} -llibzmq-v120-mt-4_0_4
        Zmq_install.files += $${ZCHX_ZMQ_PATH}/bin/libzmq-v120-mt-4_0_4.dll
        Zmq_install.path = $$IDE_APP_PATH
        INSTALLS += Zmq_install
    }
    warning("find ZCHX_ZMQ_PATH DIR =======================" + $$ZCHX_ZMQ_PATH)
} else {
    warning("Cann't find ZCHX_ZMQ_PATH DIR =======================" + $$ZCHX_ZMQ_PATH)
}
