TEMPLATE = subdirs
SUBDIRS += src tests qtquicktests

tests.depends = src

include(pyotherside.pri)

!win32 {
    # The make used in the Qt MSVC toolchain does not support $^, but
    # as we are not going to do source builds on Windows, just make
    # the source release (sdist) target depend on anything but win32.
    tar.target = $${PROJECTNAME}-$${VERSION}.tar
    tar.commands = git archive --format=tar --prefix=$${PROJECTNAME}-$${VERSION}/ --output=$@ $${VERSION}

    targz.target = $${PROJECTNAME}-$${VERSION}.tar.gz
    targz.depends = tar
    targz.commands = gzip $^

    sdist.target = sdist
    sdist.depends = targz

    QMAKE_EXTRA_TARGETS += tar targz sdist
}
