/**
 *  \brief  Graphical UI using the Qt Toolkit (version 4.6+)
 *  \author David Graeff <david.graeff@web.de>
 *  \date   2013, June
 **/

#ifndef SIMULATIONTHREAD_H
#define SIMULATIONTHREAD_H

#include <QThread>
#include <QMutex>
#include <inttypes.h>


/**
  * Global variables: The commandline arguments have to be global
  * to be accessable from both threads (gui and simulation).
  */
extern int _argc;
extern char** _argv;

class SimulationThread : public QThread
{
    Q_OBJECT
public:
	SimulationThread();
	// core to gui thread communication
	void setWindowData(int w, int h, const char* title, int memsize);
	
	// core to gui thread communication
	// slow down to 50ms update time, we do no
	// draw the pixmap immediatelly here, but notify
	// the Qt event system of new pixmap data
	void copyBitmap(uint8_t* data);
  // Use begin/end bitmap access methods to get the bitmap lock
  // and release it
  void beginBitmapAccess();
  void endBitmapAccess();
	void stopSimulation();
    void setButtonUp(uint32_t);
    void setButtonDown(uint32_t);
	void getButtonState(uint32_t&,uint32_t&,bool&);
public:
    int memsize;
private:
    uint8_t* framebufferData;
	QMutex mMutex;
  QMutex mBitmapMutex;
    uint32_t buttonUp;
    uint32_t buttonDown;
	bool shutdown;
    void run();
Q_SIGNALS:
    void finished();
    void setGuiSimData(const QString& title, int w, int h, int memsize, uint8_t* data);
    void displayBitmap();
};
// We have to know the worker object, from where we where started.
// We will use that object to communicate with the gui main thread.
extern SimulationThread* simulationWorker;

#endif // SIMULATIONTHREAD_H
