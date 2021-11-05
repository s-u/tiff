AC_INIT([tiff],[1.0],[Simon.Urbanek@r-project.org])
AC_CONFIG_SRCDIR(src/common.c)

# find R home and set correct compiler + flags
: ${R_HOME="`R RHOME`"}
if test -z "${R_HOME}"; then
  AC_MSG_ERROR([cannot determine R_HOME. Make sure you use R CMD INSTALL!])
fi
RBIN="${R_HOME}/bin/R"

: ${PKG_MODULE=libtiff-4}

# pick all flags for testing from R
: ${CC=`"${RBIN}" CMD config CC`}
: ${CFLAGS=`"${RBIN}" CMD config CFLAGS`}
: ${CPPFLAGS=`"${RBIN}" CMD config CPPFLAGS`}
: ${LDFLAGS=`"${RBIN}" CMD config LDFLAGS`}
: ${CPP="$CC -E"}

# honor PKG_xx overrides
LIBS="${LIBS} ${PKG_LIBS}"
# for CPPFLAGS we will superfluously double R's flags
# since we'll set PKG_CPPFLAGS with this, but that shouldn't hurt
CPPFLAGS="${CPPFLAGS} ${PKG_CPPFLAGS}"

AC_PATH_PROG([PKG_CONFIG], [pkg-config])
AC_LANG(C)
AC_CHECK_HEADERS([unistd.h])

AC_CHECK_LIB([tiff],[TIFFClientOpen],,[
  AC_MSG_NOTICE([Using libtiff directly doesn't work, falling back to pkg-config.])
  AC_MSG_NOTICE([Note that you can provide correct PKG_LIBS to allow installation without pkg-config.])
  AC_MSG_CHECKING([for pkg-config])
  AS_IF(["$PKG_CONFIG" --version], , [AC_MSG_RESULT([no])
     AC_MSG_ERROR([Provided flags don't work and pkg-config is not present.
You can either set PKG_LIBS to the correct flags or install pkg-config.])])
  AC_MSG_CHECKING([whether pkg-config knows about $PKG_MODULE])
  AS_IF([$PKG_CONFIG $PKG_MODULE], [AC_MSG_RESULT(yes)], [AC_MSG_RESULT([no])
     AC_MSG_ERROR([Install libtiff-dev or equivalent first (or set PKG_MODULE if non-standard)])])
  TIFF_CPPFLAGS="`$PKG_CONFIG --cflags $PKG_MODULE`"
  TIFF_LIBS="`$PKG_CONFIG --libs $PKG_MODULE`"
  LIBS0="$LIBS"
  LIBS="$LIBS0 ${TIFF_LIBS}"
  CPPFLAGS="$CPPFLAGS ${TIFF_CPPFLAGS}"
  AC_MSG_CHECKING([whether ${TIFF_CPPFLAGS} ${TIFF_LIBS} works])
  AC_LINK_IFELSE([AC_LANG_CALL([], [TIFFClientOpen])], AC_MSG_RESULT([yes]), [
    AC_MSG_RESULT([no])
    AC_MSG_CHECKING([whether --static helps])
    TIFF_LIBS="`$PKG_CONFIG --static --libs $PKG_MODULE`"
    LIBS="$LIBS0 ${TIFF_LIBS}"
    AC_LINK_IFELSE([AC_LANG_CALL([], [TIFFClientOpen])], AC_MSG_RESULT([yes]),,[
      AC_MSG_RESULT([no])
      AC_MSG_ERROR([Cannot get libtiff to work, check config.log for details.])
    ])
  ])
])

AC_MSG_NOTICE([ PKG_CPPFLAGS: $CPPFLAGS])
AC_MSG_NOTICE([ PKG_LIBS    : $LIBS])

## check headers only after we're done with pkg-config
AC_CHECK_HEADERS([tiff.h tiffio.h],, [AC_MSG_ERROR([TIFF headers are not usable.
Please make sure you have installed development files for libtiff.])])

AC_ARG_VAR([PKG_CPPFLAGS],[custom C preprocessor flags for package compilation])
AC_ARG_VAR([PKG_LIBS],[custom libraries for package compilation])
AC_ARG_VAR([PKG_CONFIG],[path to the pkg-config executable (pkg-config)])
AC_ARG_VAR([PKG_MODULE],[name of the tiff pkg-config module (libtiff-4)])

AC_CONFIG_FILES(src/Makevars)
AC_OUTPUT
