dnl $Id$
dnl config.m4 for extension spf

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(spf, for spf support,
    [  --with-spf             Include spf support])

if test "$PHP_SPF" != "no"; then

  SEARCH_PATH="/usr/local /usr"
  SEARCH_FOR="/include/spf2/spf.h"
  if test -r $PHP_SPF/$SEARCH_FOR; then
    SPF_DIR=$PHP_SPF
  else
    AC_MSG_CHECKING([for spf files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        SPF_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi

  if test -z "$SPF_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the spf distribution])
  fi

  PHP_ADD_INCLUDE($SPF_DIR/include)

  LIBNAME=spf2
  LIBSYMBOL=SPF_server_new

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $SPF_DIR/lib, SPF_SHARED_LIBADD)
    AC_DEFINE(HAVE_SPFLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong spf lib version or lib not found])
  ],[
    -L$SPF_DIR/lib -lm
  ])
  
  PHP_SUBST(SPF_SHARED_LIBADD)

  PHP_NEW_EXTENSION(spf, spf.c, $ext_shared)
fi
