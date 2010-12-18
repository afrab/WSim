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
	liblogpkt  \
	libselect  \
	libwsnet   \
	machine    \
	src        \
	platforms  \
	doc


## build wsim
world:
	make && make install && make tests

## indent code
if HAS_INDENT
INDENT_ARGS = \
--use-tabs \
--tab-size 2 \
--indent-level 4 \
--start-left-side-of-comments \
--leave-preprocessor-space

indent:
	@[ ! -d $(top_srcdir)/.svn ] || \
	    [ 0 -eq $$(svn status | grep '\.[hc]$$' | grep -v config.h | tee /dev/fd/2 | wc -l) ] || \
	    ( echo "These files are localy modified.  'commit' or 'revert' them, then retry." >&2; false )
	@for f in $$(find \
	$(top_srcdir)/arch \
	$(top_srcdir)/devices \
	$(top_srcdir)/libconsole \
	$(top_srcdir)/libelf \
	$(top_srcdir)/libetrace \
	$(top_srcdir)/libgdb \
	$(top_srcdir)/libgui \
	$(top_srcdir)/liblogger \
	$(top_srcdir)/liblogpkt \
	$(top_srcdir)/libselect \
	$(top_srcdir)/libtracer \
	$(top_srcdir)/libwsnet \
	$(top_srcdir)/machine \
	$(top_srcdir)/platforms \
	$(top_srcdir)/src \
	$(top_srcdir)/examples \
	-name "*.[hc]"); do \
	    echo "  INDENT $$f"; \
	    $(INDENT) $(INDENT_ARGS) "$$f" || exit 1; \
	done
endif

## builds wsim/wtracer/wconsole/wsnet1le
worldsens:
	cd utils/wtracer 
	cd utils/wsnet1

tests:
	echo "This is a test"
