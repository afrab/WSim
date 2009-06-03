
WSIM_BASE= \
	../../machine/libmachine.a 	   \
	../../devices/libdevices.a         \
	../../src/libwsim.a                \
	../../libconsole/libconsole.a      \
	../../libelf/libelf.a              \
	../../libetrace/libetrace.a        \
	../../libgdb/libgdb.a              \
	../../libgui/libgui.a              \
	../../libtracer/libtracer.a	   \
        ../../libselect/libselect.a        \
	../../liblogger/liblogger.a        \
	../../arch/common/libmcucommon.a   \
	@EXTRALIBS@

WORLDSENSLIB=

if BUILD_WORLDSENS

if BUILD_WSNET1 
WORLDSENSLIB+=../../libwsnet/libwsnet1.a
endif

if BUILD_WSNET2
WORLDSENSLIB+=../../libwsnet/libwsnet2.a
endif

else ## don't build worldsens
WORLDSENSLIB+=../../libwsnet/libwsnet0.a
endif

WSIMADD= ${WSIM_BASE} ../../libwsnet/libwsnet0.a
WSIMADD_WORLDSENS= ${WSIM_BASE} ${WORLDSENSLIB}

export WSIMADD WSIMADD_WORLDSENS

SUBDIRS=wsn430 ot2006 otsetre ez430 tests telosb \
	ez430rf mosar mica2 micaz iclbsn wisenode senslab \
	mcugen



