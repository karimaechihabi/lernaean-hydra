AC_INIT([DSTREE], [2.0], [karima.echihabi@ifnilink.com],
             [libdstree], [http://])
AC_PREREQ([2.59])
AM_INIT_AUTOMAKE([1.10 -Wall no-define])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_CC_C_O
AC_CHECK_HEADER(values.h)

if test "$ac_cv_header_values_h" == yes
then
	AC_DEFINE(VALUES, [], [include values.h])
fi

AC_ARG_WITH(no_benchmarking, [  --no-benchmarking            disable stuff])
if test "$no_benchmarking" == "yes"
then
	AC_MSG_NOTICE([You have disabled benchmarking!])
else
	AC_DEFINE(BENCHMARK, [], [Create a benchmark version])
fi

AC_ARG_WITH(with_debug, [  --with-debug            enable stuff])
if test "$with_debug" == "yes"
then
	AC_MSG_NOTICE([You have enabled debugging info!])
	AC_DEFINE(DEBUG, [], [Enable debugging info])
fi

AC_ARG_WITH(with_hardlbl, [  --with-hardlbl            enable stuff])
if test "$with_hardlbl" == "yes"
then
	AC_MSG_NOTICE([You have enabled hard leaf buffer layer!])
	AC_DEFINE(HARDLBL, [], [Enable hard leaf buffer layer])
fi

AC_ARG_WITH(verbose, [  --verbose            enable stuff])
if test "$verbose" == "yes"
then
	AC_MSG_NOTICE([You have enabled verbose output!])
	AC_DEFINE(VERBOSE, [2], [Enable verbose output])
else
	AC_DEFINE(VERBOSE, [0], [Disable verbose output])
fi


AC_CONFIG_FILES([Makefile])
AC_OUTPUT
