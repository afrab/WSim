
/**
 *  \brief  Graphical UI using the Qt Toolkit (version 4.6+)
 *  \author David Graeff <david.graeff@web.de>
 *  \date   2013, June
 **/

#include "mainwindow.h"
#include "simulationthread.h"
#include <QApplication>

/**
  * This main method is used if the gui is enabled
  */
int main(int argc, char* argv[])
{
  _argc = argc;
  _argv = argv;
  QApplication app(argc, argv);
  app.setApplicationName("wsim");
  app.setOrganizationDomain ("org.wsim");
  MainWindow w;
  w.show();
  app.exec();
  return 0;
}

