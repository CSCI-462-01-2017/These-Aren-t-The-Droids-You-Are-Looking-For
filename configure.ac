dnl Process this file with autoconf to produce a configure script.
dnl
dnl Tools versions as available on Debian 7 (wheezy) [Debian 8 is still too recent]:
dnl autoconf 2.69  automake 1.11.6  gettext 0.18.1
dnl
dnl Note about compilation flags:
dnl
dnl CFLAGS/CPPFLAGS/LDFLAGS are used when compiling some configure test
dnl programs. This can lead to some problems.
dnl For instance, if "-fsanitize=adress" is set before looking for the Lua lib,
dnl configure exits with a failure due to a memleak detection.
dnl
dnl In order to avoid any issue with configure, we use FD_ prefixed vars to
dnl store our own flags. Those are added to AM_ vars in the Makefile.am

AC_PREREQ([2.69])
AC_INIT([freedroidRPG],[0.16+git],[http://bugs.freedroid.org],[freedroidrpg],[http://www.freedroid.org])
AC_CONFIG_SRCDIR([INSTALL.freedroidRPG])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

dnl See src/savegame/convert.c to know how to define those values
AC_DEFINE([SAVEGAME_VERSION], ["001601"], [Savegame version])
AC_DEFINE([SAVEGAME_REVISION], ["05"], [Savegame revision])

dnl Redefine some macros, to follow silent mode

AC_DEFUN([SILENT_ECHO],
	[AS_VAR_IF([silent], [yes],
		[],
		[AS_ECHO([$1])]
	)]
)

AC_DEFUN([SILENT_BOX],
	[AS_VAR_IF([silent], [yes],
		[],
		[AS_BOX([$1])]
	)]
)

dnl Write #define config statements into 'config.h'

AC_CONFIG_HEADERS([config.h])

dnl Detect the canonical host and target build environment

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

dnl Set automake options (require AC_CANONICAL_TARGET)

AM_INIT_AUTOMAKE([tar-ustar -Wno-portability])

dnl Commandline options

AC_ARG_ENABLE([opengl],
				[AS_HELP_STRING([--disable-opengl],[do not build the OpenGL support (not recommended!) [default=auto]])],
				[want_gl="$enableval"], [want_gl=yes])
AC_ARG_ENABLE([sound],
				[AS_HELP_STRING([--disable-sound],[do not include support for audio playback (no music, no SFX) [default=auto]])],
				[want_sound="$enableval"], [want_sound=yes])
AC_ARG_ENABLE([fastmath],
				[AS_HELP_STRING([--enable-fastmath],[compile with -ffast-math (this can produce strange results due to 'imprecise' math computations) [default=no]])],
				[fast_math="$enableval"], [fast_math=no])
AC_ARG_ENABLE([debug],
				[AS_HELP_STRING([--enable-debug],[compile with debugging support [default=no]])],
				[use_debug="$enableval"], [use_debug=no])
AC_ARG_ENABLE([native-arch],
				[AS_HELP_STRING([--enable-native-arch],[compile with -march=native and -mtune=native [default=no]])],
				[arch_native="$enableval"], [arch_native=no])
AC_ARG_ENABLE([sanitize-address],
				[AS_HELP_STRING([--enable-sanitize-address],[compile with -fsanitize=address [default=no]])],
				[sanitize_address="$enableval"], [sanitize_address=no])
AC_ARG_WITH(  [extra-warnings],
				[AS_HELP_STRING([--with-extra-warnings],[generate more warning messages when compiling [default=no]])],
				[more_warn="$withval"], [more_warn=no])
AC_ARG_ENABLE([backtrace],
				[AS_HELP_STRING([--disable-backtrace],[disable the use of backtrace() to generate nice bug reports [default=enabled]])],
				[want_backtrace="$enableval"], [want_backtrace=yes])
AC_ARG_WITH(  [embedded-lua],
				[AS_HELP_STRING([--with-embedded-lua],[uses the Lua libraries bundled with FreedroidRPG instead of system libraries [default=system lua]])],
				[embedded_lua="$withval"], [embedded_lua=no])
AC_ARG_ENABLE([rtprof],
				[AS_HELP_STRING([--enable-rtprof],[enable the integration of the ingame real-time profiler [default=no]])],
				[want_rtprof="$enableval"], [want_rtprof=no])
AC_ARG_ENABLE([dev-tools],
				[AS_HELP_STRING([--enable-dev-tools],[compile the additional dev tools [default=no]])],
				[dev_tools="$enableval"], [dev_tools=no])
AC_ARG_WITH(  [open-cmd],
				[AS_HELP_STRING([--with-open-cmd=CMD],[command used to open and display a text file on screen [default='notepad' on Windows, 'open' on MacOSX, 'xdg-open' otherwise]])],
				[open_cmd="$withval"], [open_cmd=""])

dnl Checks for programs.

SILENT_BOX([Checking compulsory programs])

AC_PROG_CC
AC_PROG_CC_STDC
XIPH_C_COMPILER_IS_CLANG
AC_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB
AC_PROG_MAKE_SET
AC_PATH_PROG([AWK], [gawk], [])    dnl GNU version of awk awk is needed in some po/ scripts
AC_PATH_PROG([LUADOX], [lua2dox], [])
AM_PATH_PYTHON([2.6])
AC_PATH_TARGET_TOOL([WINDRES], [windres], [windres])
AM_GNU_GETTEXT_VERSION([0.18.2])
AM_GNU_GETTEXT([external])
AM_ICONV

PKG_PROG_PKG_CONFIG([])
if test -z "$PKG_CONFIG"; then
    AC_MSG_WARN([pkg-config not found])
    AC_MSG_WARN([required libraries cannot be detected])
    AC_MSG_WARN([please install pkg-config])
fi

dnl Not sure if the following tests are really needed
dnl AC_PATH_X
dnl AC_PATH_XTRA
dnl AS_VAR_IF([$have_x], [yes],
dnl 	[CFLAGS="$CFLAGS $X_CFLAGS"]
dnl )

AS_CASE([$target],
	[*-*-mingw32*],
		[
			AS_VAR_APPEND([FD_CPPFLAGS], [" -DFD_DATADIR='\".\"' -D_POSIX_C_SOURCE=1"])
			default_open_cmd="notepad"
			win32="true"
			libintl_checked="yes"
		],
	[*darwin*],
		[
			AS_VAR_APPEND([FD_CPPFLAGS], [" -DFD_DATADIR='\"\$(pkgdatadir)\"'"])
			dnl Darwin has single-precision math-functions in here
			AS_VAR_APPEND([FD_LDFLAGS], [" -lmx"])
			default_open_cmd="open"
			macosx="true"
		],
	[*bsd*],
		[
			AS_VAR_APPEND([FD_CPPFLAGS], [" -DFD_DATADIR='\"\$(pkgdatadir)\"'"])
			AS_VAR_APPEND([FD_LDFLAGS], [" -rdynamic"])
			default_open_cmd="xdg-open"
			bsd="true"
		],
	[
		AS_VAR_APPEND([FD_CPPFLAGS], [" -DFD_DATADIR='\"\$(pkgdatadir)\"'"])
		AS_VAR_APPEND([FD_LDFLAGS], [" -rdynamic -Wl,--as-needed"])
		default_open_cmd="xdg-open"
	]
)
AM_CONDITIONAL([WIN32],    [test "x$win32" = "xtrue"])
AM_CONDITIONAL([NONLINUX], [test "x$macosx" = "xtrue" -o "x$win32" = "xtrue"])

AS_VAR_IF([open_cmd], [], [open_cmd=$default_open_cmd])
AC_DEFINE_UNQUOTED([OPENTXT_CMD], ["$open_cmd"], [Set to the command to use to display a text file])

AS_VAR_IF([LIBINTL], [],
	[],
	[AC_DEFINE([USE_LIBINTL], [1], [GNU gettext is in libintl (instead of libc)])]
)

dnl Checks for libraries.

SILENT_BOX([Checking compulsory libraries])

dnl Force use of embedded lua in ASAN mode
summary_lua="embedded"
AS_VAR_IF([sanitize_address], [yes],
	[
		embedded_lua="yes"
		summary_lua="embedded (due to use of ASAN mode)"
	]
)

AC_CHECK_LIB([m], [sin],
	[],
	[AC_MSG_ERROR([libm not found!! No maths library?? What kinda crazy system is that??])]
)

AC_CHECK_LIB([z], [compress],
	[],
	[AC_MSG_ERROR([zlib is needed to run FreedroidRPG! see http://www.gzip.org/zlib/])]
)

AC_CHECK_LIB([jpeg], [jpeg_start_compress],
	[],
	[AC_MSG_ERROR([libjpeg needed to run FreedroidRPG! see http://www.ijg.org/])]
)

AC_CHECK_LIB([png], [png_read_png],
	[],
	[AC_MSG_ERROR([libpng needed to run FreedroidRPG! see http://www.libpng.org/pub/png/libpng.html])]
)

SDL_VERSION=1.2.3
AM_PATH_SDL($SDL_VERSION,
	[
		AC_DEFINE([HAVE_LIBSDL], [1], [Define to 1 if SDL lib was found])
		AS_VAR_APPEND([FD_CFLAGS], [" $SDL_CFLAGS"])
		AS_VAR_APPEND([FD_LIBS], [" $SDL_LIBS"])
	],
	[AC_MSG_ERROR([SDL version $SDL_VERSION needed to run FreedroidRPG! see http://libsdl.org/])]
)

AC_CHECK_LIB([SDL_image], [IMG_LoadJPG_RW],
	[],
	[AC_MSG_ERROR([SDL_image library needed for FreedroidRPG! see http://www.libsdl.org/])]
)

AC_CHECK_LIB([SDL_gfx], [zoomSurface],
	[],
	[AC_MSG_ERROR([SDL_gfx library needed for FreedroidRPG! Please refer to the INSTALL.freedroidRPG file])]
)

dnl Checking for optional libraries

SILENT_BOX([Checking optional libraries])

dnl the 'new way' to check for openGL (sets 'no_gl=yes' if not found)
ACX_PTHREAD
AX_CHECK_GL

AC_MSG_CHECKING(for openGL libraries)
AS_VAR_IF([no_gl], [yes],
	[
		summary_have_gl="no"
		AC_MSG_RESULT([no])
		AC_MSG_WARN([openGL libs not found! ==> Therefore we are building freedroid only with SDL-graphics support.])
		AC_MSG_WARN([(if compiling on Darwin, try "./configure --with-apple-opengl-framework")])
	],
	[
		AS_VAR_IF([want_gl], [yes],
			[
				summary_have_gl="yes"
				AC_MSG_RESULT([yes])
				AC_DEFINE([HAVE_LIBGL], [1], [Define to 1 if openGL libs were found])
				AS_VAR_APPEND([FD_CFLAGS], [" $GL_FLAGS"])
				AS_VAR_APPEND([FD_LIBS], [" $GL_LIBS"])
				PKG_CHECK_MODULES([GLEW], [glew >= 1.6])
				AS_VAR_APPEND([FD_CFLAGS], [" $GLEW_CFLAGS"])
				AS_VAR_APPEND([FD_LIBS], [" $GLEW_LIBS"])
			],
			[
				summary_have_gl="disabled"
				AC_MSG_RESULT([disabled by request])
				AC_MSG_WARN([openGL disabled, but the libraries were found! The gaming experience would be much better with openGL rendering.])
			]
		)
	]
)

AS_VAR_IF([want_sound], [yes],
	[
		summary_sound="yes"
		with_sound="yes"
		AC_CHECK_LIB([SDL_mixer], [Mix_ChannelFinished],
			[],
			[
				AC_MSG_WARN([libSDL_mixer not found! see http://www.libsdl.org/ -> compiling without sound support])
				summary_sound="SDL_mixer not found"
				with_sound="no"
			]
		)
	],
	[
		summary_sound="no"
		with_sound="no"
	]
)
AS_VAR_IF([with_sound], [yes],
	[
		AC_CHECK_LIB([ogg], [oggpack_read],
			[],
			[
				AC_MSG_WARN([libogg not found! -> compiling without sound support])
				summary_sound="no libogg"
				with_sound="no"
			]
		)
	]
)
AS_VAR_IF([with_sound], [yes],
	[
		AC_CHECK_LIB([vorbis], [vorbis_block_init],
			[],
			[
				AC_MSG_WARN([libvorbis not found! -> compiling without sound support])
				summary_sound="no libvorbis"
				with_sound="no"
			]
		)
	]
)
AS_VAR_IF([with_sound], [yes],
	[AC_DEFINE([WITH_SOUND], [1], [Define to 1 to build with sound system])]
)

AS_VAR_IF([dev_tools], [yes],
	[
		summary_dev_tools="yes"
		AC_CHECK_LIB([SDL_ttf], [TTF_RenderUNICODE_Blended],
			[],
			[
				AC_MSG_WARN([libSDL_ttf not found! --> compiling with some font tools disabled])
				summary_dev_tools="make_bmchars not built due to SDL_ttf lib missing -optional-"
			]
		)
	],
	[
		summary_dev_tools="no"
	]
)
AM_CONDITIONAL([WITH_MAKE_BMCHARS], [test "x$ac_cv_lib_SDL_ttf_TTF_RenderUNICODE_Blended" = "xyes"])

AS_VAR_IF([want_rtprof], [yes],
	[
		summary_rtprof="yes"
		with_rtprof="yes"
		AC_CHECK_LIB([rt], [clock_gettime],
			[],
			[
				AC_MSG_WARN([librt (or clock_gettime) not found! -> the ingame real-time profiler will not be compiled])
				summary_rtprof="no"
				with_rtprof="no"
			]
		)
	],
	[
		summary_rtprof="no"
		with_rtprof="no"
	]
)
AS_VAR_IF([with_rtprof], [yes],
	[AC_DEFINE([WITH_RTPROF], [1], [Define to 1 if rt lib were found and rtprof was enabled])]
)

AS_VAR_IF([bsd], [true],
	[
		AC_CHECK_LIB([execinfo], [backtrace],
			[],
			[AC_MSG_WARN([libexecinfo not found! Backtraces can not be generated on BSD systems if libexecinfo is not present])]
		)
	]
)

AS_VAR_IF([embedded_lua], [no],
	[
		KYUA_LUA()
		AS_VAR_IF([lua_found], [yes],
			[
				summary_lua="system-wide"
				AS_VAR_APPEND([FD_CFLAGS], [" $LUA_CFLAGS"])
				AS_VAR_APPEND([FD_LIBS], [" $LUA_LIBS"])
			],
			[
				embedded_lua="yes"
				summary_lua="embedded (due to Lua 5.3 not found system-wide)"
				AC_MSG_NOTICE([No system-wide Lua 5.3 library found. Using embedded Lua instead])
			]
		)
	]
)

dnl Checks for header files.

SILENT_BOX([Checking header files])

AC_CHECK_HEADERS([execinfo.h fcntl.h fenv.h float.h inttypes.h langinfo.h libgen.h])
AC_CHECK_HEADERS([libintl.h limits.h locale.h signal.h soundcard.h stddef.h stdint.h stdlib.h])
AC_CHECK_HEADERS([string.h strings.h sys/ioctl.h sys/soundcard.h unistd.h])

dnl Checks for typedefs, structures, and compiler characteristics.

SILENT_BOX([Checking types])

AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_TIME
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

dnl Checks for library functions.

SILENT_BOX([Checking library functions])

AS_VAR_IF([cross_compiling], [no],
	[
		AC_FUNC_MALLOC
		AC_FUNC_REALLOC
	]
)
AC_FUNC_MKTIME
AC_FUNC_STRCOLL
AC_CHECK_FUNCS([alphasort atexit clock_gettime dirname floor getcwd memchr memmove memset mkdir])
AC_CHECK_FUNCS([nl_langinfo pow putenv rint scandir setenv setlocale sqrt strchr strcspn])
AC_CHECK_FUNCS([strdup strerror strpbrk strrchr strspn strstr strtol])
AS_VAR_IF([want_backtrace], [yes], [AC_CHECK_FUNCS([backtrace])])

dnl Set build flags accordingly to what was requested and found

AS_VAR_APPEND([FD_CFLAGS], [" -pipe -Wall -Wno-format-zero-length"])

AS_VAR_IF([arch_native], [yes],
	[AS_VAR_APPEND([FD_CFLAGS], [" -O3 -march=native -mtune=native"])]
)

AS_VAR_IF([sanitize_address], [yes],
	[
		AC_MSG_NOTICE([Building with -fsanitize=address -fno-omit-frame-pointer. Also using embedded Lua!])
		AS_VAR_APPEND([FD_CFLAGS], [" -fsanitize=address -fno-omit-frame-pointer"])
	]
)

AS_VAR_IF([embedded_lua], [yes],
	[
		AS_VAR_APPEND([FD_CPPFLAGS], [" -I\$(top_srcdir)/lua"])
		AS_VAR_SET([FD_LIBS], ["\$(top_builddir)/lua/liblua.a $FD_LIBS"])
	],
)

AS_VAR_IF([USE_NLS], [yes],
	[
		AS_VAR_APPEND([FD_LIBS], [" $LIBINTL $LIBICONV"])
		AS_VAR_APPEND([FD_CPPFLAGS], [" -DLOCALEDIR='\"\$(localedir)\"'"])
	]
)

AS_VAR_IF([use_debug], [yes],
	[AS_VAR_APPEND([FD_CFLAGS], [" -g3 -ggdb -O0"])]
)

AS_VAR_IF([more_warn], [yes],
	[AS_VAR_APPEND([FD_CFLAGS], [" -Wextra -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-align -Wnested-externs -Wunreachable-code"])]
)

AS_VAR_IF([fast_math], [yes],
	[AS_VAR_APPEND([FD_CFLAGS], [" -ffast-math"])]
)

AC_SUBST([FD_CFLAGS])
AC_SUBST([FD_CPPFLAGS])
AC_SUBST([FD_LDFLAGS])
AC_SUBST([FD_LIBS])

dnl Define check conditions, to be used in Makefile.am

AM_CONDITIONAL([BUILD_LUA],       [test "x$embedded_lua" = "xyes"])
AM_CONDITIONAL([BUILD_DEV_TOOLS], [test "x$dev_tools" = "xyes"])

dnl Files to generate

SILENT_BOX([Creating output files])

AC_CONFIG_FILES([
	Makefile
	lua/Makefile
	src/Makefile
	tools/Makefile
		tools/atlas/Makefile
	win32/Makefile
	doc/Doxyfile doc/Makefile
		doc/manual/Doxyfile doc/manual/Makefile
		doc/tools/Doxyfile doc/tools/Makefile
	po/Makefile
		po/po-src/Makevars po/po-src/Makefile.in
		po/po-dialogs/Makevars po/po-dialogs/Makefile.in
		po/po-data/Makevars po/po-data/Makefile.in
		po/l10n-others/Makefile
	pkgs/Makefile
		pkgs/freedesktop/Makefile
])

AS_VAR_IF([dev_tools], [yes],
	[
		AC_CONFIG_FILES([
			tools/font/Makefile
			tools/image/Makefile
		])
	]
)

dnl Generate and run config.status

AC_OUTPUT

dnl Display a summary of the build configuration

AS_VAR_IF([xiph_cv_c_compiler_clang], [yes],
	[summary_compiler="clang"],
	[summary_compiler=$CC]
)

SILENT_ECHO([])
SILENT_BOX([Summary])
SILENT_ECHO(["

	CPPFLAGS:    $FD_CPPFLAGS $CPPFLAGS
	CFLAGS:      $FD_CFLAGS $CFLAGS
	LDFLAGS:     $FD_LDFLAGS $LDFLAGS
	LIBS:        $LIBS $FD_LIBS

	compiler:                        $summary_compiler
	used Lua library:                $summary_lua
	compile with OpenGL support:     $summary_have_gl
	compile with sound support:      $summary_sound
	compile dev tools:               $summary_dev_tools
	embed the real-time profiler:    $summary_rtprof
	localization enabled:            $USE_NLS"
])

AS_VAR_IF([cross_compiling], [yes],
	[SILENT_ECHO(["	cross-compiling detected, so will not build some subdirs"])]
)

AS_IF([test "x$LIBINTL" != x -a "x$libintl_checked" != "xyes"],
	[
		AS_ECHO([])
		AS_ECHO([])
		AS_BOX([!!! WARNING!!!])
		AS_ECHO(["
	On your system, gettext is defined in an external libintl library.
	We can not ensure that it behaves as we expect.
	You could encounter some issues when saving and loading savegames.
	Please contact us (see README), and tell us if it works or not on your system.
	If it works, we will remove this warning for your system.
	It if does not work, we will try to provide a workaround."
		])
	]
)

SILENT_ECHO([])
