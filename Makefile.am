# order is important (platforms must be at the end)
SUBDIRS = arch     \
	devices    \
	libconsole \
	libelf     \
	libetrace  \
	libgdb     \
	libgui     \
	libtracer  \
	liblogger  \
	libselect  \
	libwsnet   \
	machine    \
	src        \
	platforms  \
	doc

## build wsim
world:
	make && make install && make tests

## builds wsim/wtracer/wconsole/wsnet1le
worldsens:
	cd utils/wtracer 
	cd utils/wsnet1

tests:
	echo "This is a test"
