TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += common
!unix{
    SUBDIRS += app
}
SUBDIRS += server



