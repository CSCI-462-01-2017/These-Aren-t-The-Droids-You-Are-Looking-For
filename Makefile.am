## Process this file with automake to produce Makefile.in
# $Id: Makefile.am,v 1.26 2007-09-15 18:02:36 ahuillet Exp $

## make sure rebuilding uses the right aclocal-switches
include include.mk

# Initialize automake variables
ACLOCAL_AMFLAGS = -I m4
AM_MAKEFLAGS    = CC="$(CC)" RANLIB=$(RANLIB) STRIP=$(STRIP)
AM_CFLAGS       = @FD_CFLAGS@
AM_CPPFLAGS     = @FD_CPPFLAGS@
AM_LDFLAGS      = @FD_LDFLAGS@
LIBS            = @FD_LIBS@ @LIBS@
AM_DISTCHECK_CONFIGURE_FLAGS = --enable-dev-tools

if WIN32
WIN32_PREBUILD = win32
endif

if BUILD_LUA
MAYBE_EMBEDDED_LUA = lua
endif

SUBDIRS = $(MAYBE_EMBEDDED_LUA) $(WIN32_PREBUILD) src tools doc po pkgs

man_MANS = freedroidRPG.6
README_l10n = $(basename $(notdir $(wildcard $(top_srcdir)/README-*)))
dist_doc_DATA = README $(README_l10n) HELP_WANTED NEWS

freedroid_data = data lua_modules

EXTRA_DIST = m4 lua $(man_MANS) $(freedroid_data) ChangeLog INSTALL.freedroidRPG
CHECKFLAGS=env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy

check:
	$(CHECKFLAGS) ./src/freedroidRPG -nb text      || exit 1
	$(CHECKFLAGS) ./src/freedroidRPG -nb dialog    || exit 2
	$(CHECKFLAGS) ./src/freedroidRPG -nb loadship  || exit 3
	$(CHECKFLAGS) ./src/freedroidRPG -nb dynarray  || exit 4
	$(CHECKFLAGS) ./src/freedroidRPG -nb mapgen    || exit 5
	$(CHECKFLAGS) ./src/freedroidRPG -nb leveltest || exit 6
	$(CHECKFLAGS) ./src/freedroidRPG -nb event     || exit 7


dist-hook:
	find $(distdir) -name ".git"         | xargs rm -rf
	find $(distdir) -name "*~"           | xargs rm -f
	find $(distdir) -name ".#*"          | xargs rm -f
	find $(distdir)/lua -name "*.o"      | xargs rm -f
	find $(distdir)/lua -name "*.a"      | xargs rm -f
	find $(distdir)/lua -name "Makefile" | xargs rm -f

install-data-local:
	@echo "Installing the data-files ..."
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	cd $(srcdir); \
	for dir in $(freedroid_data) ; do \
		find $${dir} -type d ! -path "*.git*" -exec echo $(DESTDIR)$(pkgdatadir)/{} \; | xargs $(mkinstalldirs) ; \
		find $${dir} -type f ! -path "*.git*" -and ! -name "Makefile*" -and ! -name "*~" -and ! -name ".#*" -exec $(INSTALL_DATA) {} $(DESTDIR)$(pkgdatadir)/{} \; ; \
	done
	@echo "..done."

uninstall-local:
	@echo "Removing data-files ..."
	rm -rf $(DESTDIR)$(pkgdatadir)
	rm -rf $(DESTDIR)$(docdir)
	@echo "...done."

changelog:
	@git log --pretty=tformat:"------------------------------------------------------------------------%n%h | %an | %ai%n%n%w(72)%s"

sign:
	@echo "Generating signatures ..."
	gpg --digest-algo SHA512  -o freedroidrpg-$(VERSION).tar.bz2.sig --detach-sign freedroidrpg-$(VERSION).tar.bz2
	md5sum freedroidrpg-$(VERSION).tar.bz2 > freedroidrpg-$(VERSION).tar.bz2.md5

## Win32 specific stuff follows here

win_txts = COPYING AUTHORS README ChangeLog
win_dir = freedroidrpg-win32
dist-win32: src/freedroidRPG.exe
	@if [ "x$(DLL_PATH)" = "x" ] ; then \
	  echo "DLL_PATH not defined. Aborting."; \
	  exit 1; \
	 fi
	rm -rf $(win_dir)
	mkdir -p $(win_dir)
	rsync -ar --exclude='.git' --exclude='Makefile*' --exclude='*~' --exclude='.#*' data lua_modules locale $(win_dir)
	cp src/freedroidRPG.exe $(win_dir)
	for i in $(win_txts); do \
	   sed 's/$$/\r/' $$i > $(win_dir)/$$i.txt; \
	done
	perl win32/pkgApp.pl $(win_dir) freedroidRPG.exe $(DLL_PATH)

$(win_dir): dist-win32

package-win32: $(filter-out $(wildcard $(win_dir)), $(win_dir))
	rm -f freedroidRPG-*-win32.exe
	rm -f freedroidRPG.nsi
	win32/install_file_lists.py win32/freedroidRPG.nsi.in $(win_dir) > freedroidRPG.nsi
	makensis -V1 -DPRODUCT_VERSION="$(VERSION)" -DSRC_DIR="${win_dir}" freedroidRPG.nsi

macosx_dir = freedroidRPG.app
dist-macosx: src/freedroidRPG
	rm -rf $(macosx_dir)
	mkdir -p $(macosx_dir)/Contents
	mkdir -p $(macosx_dir)/Contents/MacOS
	mkdir -p $(macosx_dir)/Contents/Resources
	mkdir -p $(macosx_dir)/Contents/Frameworks
	cp macosx/Icon.icns $(macosx_dir)/Contents/Resources/Icon.icns
	cp src/freedroidRPG $(macosx_dir)/Contents/MacOS
	sed -e "s/__#PROGNAME#__/$(PACKAGE_NAME)/" -e "s/__#VERSION#__/$(PACKAGE_VERSION)/" macosx/Info.plist.in > $(macosx_dir)/Contents/Info.plist
	echo "APPL????" > $(macosx_dir)/Contents/PkgInfo
	rsync -ar --exclude='.git' --exclude='Makefile*' --exclude='*~' --exclude='.#*' data lua_modules locale $(macosx_dir)/Contents
	perl macosx/pkgApp.pl $(macosx_dir)/Contents MacOS/freedroidRPG Frameworks

doc:
	$(MAKE) -C doc $(AM_MAKEFLAGS) doc

update-po: update-po-@USE_NLS@
update-po-no:
	@echo "NLS is disabled. Nothing to do."
update-po-yes:
	cd po && $(MAKE) $(AM_MAKEFLAGS) update-po

update-gmo: update-gmo-@USE_NLS@
update-gmo-no:
	@echo "NLS is disabled. Nothing to do."
update-gmo-yes:
	cd po && $(MAKE) $(AM_MAKEFLAGS) update-gmo

l10n-stats: l10n-stats-@USE_NLS@
l10n-stats-no:
	@echo "NLS is disabled. Nothing to do."
l10n-stats-yes:
	@cd po && $(MAKE) -s $(AM_MAKEFLAGS) l10n-stats

help:
	@echo "Make targets:"
	@echo " -----"
	@echo " * all                   - Default compilation (same as 'make')"
	@echo " * update-po             - Update the translation files in po/"
	@echo " * update-gmo            - Compile the translation files"
	@echo " * l10n-stats            - Output localization statistics"
	@echo " * local-install         - Install the compiled translation files into the local locale/"
	@echo " * clean                 - Remove files modified/generated by build"
	@echo " * check                 - Runs certain benchmarks"
	@echo " * doc                   - Creates doxygen documentation"
	@echo " * cppcheck              - Checks the code using cppcheck (static code analysation)"
	@echo " * cppcheck-report       - Checks the code using cppcheck and generates a html report"
	@echo " * gource                - Generate gource video from git log (requires git repository)"
	@echo " * gource_ffmpeg         - Generate gource video from git log and record using ffmpeg (requires git repository, ffmpeg and x264)"
	@echo " * help                  - This output"

.PHONY: sign doc changelog help
.PHONY: update-po-no update-po-yes update-gmo-no update-gmo-yes local-install-no local-install-yes
