TEMPLATE = subdirs
SUBDIRS += src tests

tests.depends = src

include(pyotherside.pri)

tar.target = $${PROJECTNAME}-$${VERSION}.tar
tar.commands = git archive --format=tar --prefix=$${PROJECTNAME}-$${VERSION}/ --output=$@ $${VERSION}

targz.target = $${PROJECTNAME}-$${VERSION}.tar.gz
targz.depends = tar
targz.commands = gzip $^

sdist.target = sdist
sdist.depends = targz

QMAKE_EXTRA_TARGETS += tar targz sdist
