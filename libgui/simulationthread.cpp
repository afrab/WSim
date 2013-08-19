/**
 *  \brief  Graphical UI using the Qt Toolkit (version 4.6+)
 *  \author David Graeff <david.graeff@web.de>
 *  \date   2013, June
 **/

#include "src/mainworker.h"
#include "config.h"
#include "simulationthread.h"
#include <QDebug>
#include <QMutexLocker>

/**
  * Global variables: The commandline arguments have to be global
  * to be accessable from both threads (gui and simulation).
  */
int _argc;
char** _argv;

SimulationThread* simulationWorker;

SimulationThread::SimulationThread()
{
}

void SimulationThread::run()
{
  shutdown = false;
  framebufferData = 0;
  buttonUp = 0;
  buttonDown = 0;
  qDebug() << "start sim";
  simulationMain(_argc, _argv);
  qDebug() << "stop sim";
  shutdown = false;
  emit finished();
}

void SimulationThread::stopSimulation()
{
	QMutexLocker l(&mMutex);
  shutdown = true;
}

void SimulationThread::setWindowData(int w, int h, const char* title, int memsize)
{
  this->memsize = memsize;
  if (framebufferData)
    free(framebufferData);
  framebufferData = (uint8_t*)malloc(memsize);
  memset(framebufferData, 0, memsize);
  emit setGuiSimData(title, w, h, memsize, framebufferData);
}
 
void SimulationThread::copyBitmap(uint8_t* data)
{
  QMutexLocker l(&mBitmapMutex);
  memcpy(framebufferData, data, memsize);
  emit displayBitmap();
}

void SimulationThread::beginBitmapAccess()
{
  mBitmapMutex.lock();
}

void SimulationThread::endBitmapAccess()
{
  mBitmapMutex.unlock();
}

void SimulationThread::setButtonUp(uint32_t b)
{
	QMutexLocker l(&mMutex);
  buttonUp = b;
}

void SimulationThread::setButtonDown(uint32_t b)
{
	QMutexLocker l(&mMutex);
  buttonDown = b;
}

void SimulationThread::getButtonState(uint32_t& up,uint32_t& down,bool& shutdown)
{
	QMutexLocker l(&mMutex);
	up = buttonUp;
	down = buttonDown;
	shutdown = this->shutdown;
	
	buttonUp = 0;
	buttonDown = 0;
}
