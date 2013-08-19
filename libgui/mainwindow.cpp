/**
 *  \brief  Graphical UI using the Qt Toolkit (version 4.6+)
 *  \author David Graeff <david.graeff@web.de>
 *  \date   2013, June
 **/

#include "src/mainworker.h"
#include "ui.h"
#include "config.h"
#include "gui_defines.h"
#include "simulationthread.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCloseEvent>
#include <QDebug>
#include <QPainter>
#include <QFontMetrics>
#include <QMessageBox>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), currentTimerID(0)
{
    ui->setupUi(this);
    ui->centralWidget->setScaledContents(false);
    ui->centralWidget->setAlignment(Qt::AlignCenter);
    ui->actionQuit->setIcon(QIcon::fromTheme("application-exit"));
    ui->actionQuit->setIconVisibleInMenu(true);
    ui->actionAbout->setIcon(QIcon::fromTheme("help-about"));
    ui->actionAbout->setIconVisibleInMenu(true);
    ui->action_Re_Start->setIcon(QIcon::fromTheme("media-playback-start"));
    ui->action_Re_Start->setIconVisibleInMenu(true);
    ui->actionStop->setIcon(QIcon::fromTheme("media-playback-stop"));
    ui->actionStop->setIconVisibleInMenu(true);
    restoreState(QSettings().value("mainwindow/state").toByteArray());
    restoreGeometry(QSettings().value("mainwindow/geometry").toByteArray());
	ui->actionStop->setEnabled(false);
	mSimulationThread = 0;
	mCloseOnFinish = false;
	on_action_Re_Start_triggered();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* e)
{
  QSettings().setValue("mainwindow/state", saveState());
  QSettings().setValue("mainwindow/geometry", saveGeometry());
	if (mSimulationThread) {
		mCloseOnFinish = true;
		mSimulationThread->stopSimulation();
		e->accept();
	}
	QMainWindow::closeEvent(e);
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::on_actionQuit_triggered()
{
    close();
}

void MainWindow::on_action_Re_Start_triggered()
{
	statusBar()->showMessage(tr("Start simulator..."),1000);
  buttonDown = 0;
  buttonUp = 0;
	ui->actionStop->setEnabled(true);
	ui->action_Re_Start->setEnabled(false);
	mSimulationThread = new SimulationThread;
	connect(mSimulationThread, SIGNAL(finished()), mSimulationThread, SLOT(deleteLater()) );
	connect(mSimulationThread, SIGNAL(finished()), this, SLOT(finishedSimulation()) );
    connect(mSimulationThread, SIGNAL(setGuiSimData(const QString&, int, int, int, uint8_t*)), this, SLOT(setGuiSimData(const QString&, int, int, int, uint8_t*)));
    connect(mSimulationThread, SIGNAL(displayBitmap()), this, SLOT(updateBitmap()));
    // set global object simulationWorker. This is used by the simulation c code
    // to communicate with the gui main thread
    simulationWorker = mSimulationThread;
	
	mSimulationThread->start();
}

void MainWindow::on_actionStop_triggered()
{
	if (mSimulationThread)
		mSimulationThread->stopSimulation();
}

void MainWindow::on_actionAbout_triggered()
{
	QMessageBox::about(this, tr("About"), tr("Graphical UI for WSim using the Qt Toolkit.\n"
	"David Graeff, copyright 2013\n"
	"Based on Antoine Fraboulets code, copyright 2005"
	));
}

void MainWindow::finishedSimulation()
{
	statusBar()->showMessage(tr("Simulator finished!"),3000);
	ui->action_Re_Start->setEnabled(true);
	ui->actionStop->setEnabled(false);
	// Disable buttons if simulator finished
	for (int i=0;i<mButtons.size();++i)
		mButtons[i]->setEnabled(false);
  // stop draw timer
  if (currentTimerID)
    killTimer(currentTimerID);
  currentTimerID = 0;
	// draw white background and text
	QPainter painter;
	painter.begin(&image);
  painter.fillRect(image.rect(), QColor(Qt::white));
	painter.setPen(QColor(Qt::black)); // The font color comes from user select on a QColorDialog
	QFont f = font();
	f.setPointSize(20);
	painter.setFont(f);
	QString t = tr("No simulation running!");
	painter.drawText((w-QFontMetrics(f).width(t))/2, h/2, t);  // Draw a number on the image
	painter.end();
	ui->centralWidget->setPixmap(QPixmap::fromImage(image));
  
	// delete worker
	mSimulationThread = 0;
	simulationWorker = 0;
	if (mCloseOnFinish)
		close();
}

/**
  * Override timerEvent. Called
  * by the Qt event system after setup with startTimer(ms).
  * By default, this method is called every 50ms if new
  * data is available to draw.
  */
void MainWindow::timerEvent ( QTimerEvent * event ) {
  // Only do something if the timerid is correct
  if (event->timerId() != currentTimerID)
    return;
  // We reestablish the timer event every time,
  // if no new data is available, we will not
  // start the timer.
  killTimer(currentTimerID);
  currentTimerID = 0;
  displayBitmap();
}

void MainWindow::updateBitmap()
{
  // if the gui pixmap update timer is already running, do nothing
  if (currentTimerID)
    return;

  // New data is available: Start the timer and also show the
  // new pixmap immediatelly.
  currentTimerID = startTimer(50);
  displayBitmap();
}

void MainWindow::displayBitmap()
{
    mSimulationThread->beginBitmapAccess();
    ui->centralWidget->setPixmap(QPixmap::fromImage(image));
    mSimulationThread->endBitmapAccess();
}

void MainWindow::setGuiSimData(const QString& title, int w, int h, int memsize, uint8_t* data)
{
  this->memsize = memsize;
  this->framebufferData = data;
  this->title = title;
  this->w = w;
  this->h = h;
  // We construct a QImage that internally works on the frameData memory. WSim uses
  // a RGB888 in-memory format.
  image = QImage(data, w,h, w*3, QImage::Format_RGB888);
  setWindowTitle(title);
  // Add buttons
  QToolBar *bar = ui->mainToolBar;
  bar->clear();
  qDeleteAll(mButtons);
  mButtons.clear();
  
  struct ui_t* machine_ui = get_machine_ui();
  for (int i=0;i<8;++i) {
    QString title = QString::fromUtf8(machine_ui->buttons[i].name);
	if (title.isEmpty())
		continue;
    wsimButton* btn = new wsimButton(title, machine_ui->buttons[i].pin, this);
	mButtons.append(btn);
    connect(btn, SIGNAL(pressed()), SLOT(btnpressed()));
    connect(btn, SIGNAL(released()), SLOT(btnreleased()));
    bar->addWidget(btn);
  }
  
  int buttonsWidth = qMax(w, ui->mainToolBar->sizeHint().width());
  ui->centralWidget->resize(buttonsWidth ,h);  
  if (this->width() << buttonsWidth)
    this->resize(buttonsWidth, this->height());
}

void MainWindow::btnpressed()
{
	if (!mSimulationThread)
		return;
	wsimButton* btn = (wsimButton*)sender();
	buttonDown |= btn->buttoncode;
  buttonUp &= ~btn->buttoncode;
	mSimulationThread->setButtonDown(buttonDown);
}

void MainWindow::btnreleased()
{
	if (!mSimulationThread)
		return;
	wsimButton* btn = (wsimButton*)sender();
	buttonUp |= btn->buttoncode;
  buttonDown &= ~btn->buttoncode;
	mSimulationThread->setButtonUp(buttonUp);
}
