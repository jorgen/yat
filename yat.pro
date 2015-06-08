TEMPLATE=subdirs
CONFIG += ordered

!no_tests {
    SUBDIRS += tests
} else {
    message(Tests are disabled)
}

SUBDIRS += qml yat_app

CONFIG_VARS = $${OUT_PWD}$${QMAKE_DIR_SEP}.config.vars
QMAKE_CACHE = $${OUT_PWD}$${QMAKE_DIR_SEP}.qmake.cache

exists($$CONFIG_VARS) {
    system(echo include\\\($$CONFIG_VARS\\\) >> $$QMAKE_CACHE)
}
