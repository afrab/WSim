
noinst_LIBRARIES=libgui.a

INCLUDES=-I$(top_srcdir)

libgui_a_CFLAGS=
libgui_a_SOURCES=ui.h ui.cpp qt_main.cpp mainwindow.cpp mainwindow.h moc_mainwindow.cpp \
		simulationthread.h simulationthread.cpp moc_simulationthread.cpp

# MOC: Moc class
moc_mainwindow.cpp: mainwindow.h
	@echo "  Qt MOC   $@"
	$(AM_V_GEN)$(QTMOC) $< -o moc_mainwindow.cpp
moc_simulationthread.cpp: simulationthread.h
	@echo "  Qt MOC   $@"
	$(AM_V_GEN)$(QTMOC) $< -o moc_simulationthread.cpp

# UIC: Create header file from Qt ui description file
mainwindow.cpp: ui_mainwindow.h
ui_mainwindow.h: mainwindow.ui
	@echo "  Qt UI   $@"
	$(AM_V_GEN)$(QTUIC) -n $< -o ui_mainwindow.h

clean:
	-rm -f ui_mainwindow.h moc_mainwindow.cpp moc_simulationthread.cpp *.o