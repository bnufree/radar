ZCHX_3RD_PATH = $${PWD}/3rdparty
OTHER_COMMON_DIR = $${PWD}/other/

CONFIG(release, debug|release) {
    CONFIG_NAME = Release
}
CONFIG(debug, debug|release) {
    CONFIG_NAME = Debug
    DLL_EXT=d
}

unix{
    IDE_APP_PATH = $$PWD/bin/
} else {
    IDE_APP_PATH = $$dirname(PWD)/bin/$${CONFIG_NAME}
}


message("now app_path:" + $${IDE_APP_PATH})
DEFINES += QT_MESSAGELOGCONTEXT
