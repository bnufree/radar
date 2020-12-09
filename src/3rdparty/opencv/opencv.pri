unix{
    ZCHX_OPENCV_SYS     = unix
} else {
    ZCHX_OPENCV_SYS     = win
}
ZCHX_OPENCV_PATH        = $${PWD}/$${ZCHX_OPENCV_SYS}

exists( $${ZCHX_OPENCV_PATH} ){
    message("find ZCHX_OPENCV DIR   =======================" ++ $$ZCHX_OPENCV_PATH)
    unix{
        ZCHX_OPENCV_HEADERS   = $${ZCHX_OPENCV_PATH}/include/opencv4
        ZCHX_OPENCV_LIBS      = $${ZCHX_OPENCV_PATH}/lib
        INCLUDEPATH += $${ZCHX_OPENCV_HEADERS}
        LIBS +=-L$${ZCHX_OPENCV_LIBS} \
                        -lopencv_core \
                        -lopencv_imgcodecs \
                        -lopencv_imgproc \

        ZCHX_OPENCV_install.files += $${ZCHX_OPENCV_PATH}/lib/libopencv_core.so.4.2
        ZCHX_OPENCV_install.files += $${ZCHX_OPENCV_PATH}/lib/libopencv_imgcodecs.so.4.2
        ZCHX_OPENCV_install.files += $${ZCHX_OPENCV_PATH}/lib/libopencv_imgproc.so.4.2

        ZCHX_OPENCV_install.path = $${IDE_APP_PATH}/
        INSTALLS += ZCHX_OPENCV_install

    }else{
        ZCHX_OPENCV_HEADERS   = $${ZCHX_OPENCV_PATH}/include
        ZCHX_OPENCV_LIBS      = $${ZCHX_OPENCV_PATH}/lib
        INCLUDEPATH += $${ZCHX_OPENCV_HEADERS}
        LIBS +=-L$${ZCHX_OPENCV_LIBS} \
                        -llibopencv_core420 \
                        -llibopencv_imgcodecs420 \
                        -llibopencv_imgproc420

        ZCHX_OPENCV_install.files += $${ZCHX_OPENCV_PATH}/bin/libopencv_core420.dll
        ZCHX_OPENCV_install.files += $${ZCHX_OPENCV_PATH}/bin/libopencv_imgcodecs420.dll
        ZCHX_OPENCV_install.files += $${ZCHX_OPENCV_PATH}/bin/libopencv_imgproc420.dll

        ZCHX_OPENCV_install.path = $${IDE_APP_PATH}/
        INSTALLS += ZCHX_OPENCV_install
    }
}else{
    warning("Cann't find ZCHX_OPENCV DIR   =======================" ++ $$ZCHX_OPENCV_PATH)
}



