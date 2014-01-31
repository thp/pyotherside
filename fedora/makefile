# Convenience makefile to build an RPM for Fedora from Git
#
# Requirements:
#  - All build-dependencies (see .spec.in)
#  - rpmbuild
#  - git
#
# A source tarball and .spec file will be created in the rpmbuild home
# (usually ~/rpmbuild). From there, a binary RPM will be built.

NAME := pyotherside-qml-plugin-python3-qt5
VERSION := 1.0.0
GIT_REVISION := $(shell git show-ref HEAD | head -c 8)
RELEASE := 8+git$(GIT_REVISION)

RPMBUILD_HOME ?= $(HOME)/rpmbuild

RPMBUILD ?= rpmbuild
GIT ?= git
GZIP ?= gzip
SED ?= sed

RPMBUILD_SOURCES := $(RPMBUILD_HOME)/SOURCES
RPMBUILD_SPECS := $(RPMBUILD_HOME)/SPECS

TARBALL := $(RPMBUILD_SOURCES)/$(NAME)-$(VERSION).tar.gz
SPECFILE := $(RPMBUILD_SPECS)/$(NAME)-$(VERSION).spec
SPECFILE_IN := pyotherside.spec.in

# Inject RPM metadata into .spec file
SEDARGS += -e 's|%NAME%|$(NAME)|g'
SEDARGS += -e 's|%VERSION%|$(VERSION)|g'
SEDARGS += -e 's|%RELEASE%|$(RELEASE)|g'

# Create a source tarball from Git
GITARGS += archive --format=tar
GITARGS += --prefix=$(NAME)-$(VERSION)/
GITARGS += $(GIT_REVISION) 

rpm: $(TARBALL) $(SPECFILE)
	$(RPMBUILD) -bb $(SPECFILE)

$(RPMBUILD_SOURCES) $(RPMBUILD_SPECS):
	mkdir -p $@

$(TARBALL): $(RPMBUILD_SOURCES)
	cd .. && $(GIT) $(GITARGS) | $(GZIP) >$@

$(SPECFILE): $(SPECFILE_IN) $(RPMBUILD_SPECS)
	$(SED) $(SEDARGS) $< >$@

clean:
	$(RM) $(TARBALL) $(SPECFILE)

.PHONY: rpm $(TARBALL) clean
