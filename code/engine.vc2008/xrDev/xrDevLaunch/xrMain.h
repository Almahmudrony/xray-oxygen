/*************************************************
* VERTVER, 2018 (C)
* X-RAY OXYGEN 1.7 PROJECT
*
* Edited: 18 March, 2018
* xrMain.h - Main header for compilation with Qt
* main namespace, classes and vars.
*************************************************/

#pragma once
/////////////////////////////////////////
#include "minimal_CPUID.h"
#include "ui_xrMain.h"
/////////////////////////////////////////
#include <QMainWindow>
#include <QPushButton>
#include <windows.h>
#include <thread>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
/////////////////////////////////////////
#define DLL_API __declspec(dllimport)
#define CXX 199711L
/////////////////////////////////////////
static const QStringList LIST_ITEMS = QStringList() 
<< "-r2" << "-r3" << "-r4" << "-renderdebug";
/////////////////////////////////////////

void CreateRendererList();

namespace Ui {
class xrLaunch;
}

class xrLaunch : public QMainWindow
{
    Q_OBJECT

public:

    explicit xrLaunch(QWidget *parent = 0);
    ~xrLaunch();

private slots:
    void on_pushButton_clicked();			// Run
	void run_xrEngineButton();					// Run the xrEngine
	void on_actionExit_triggered();			// Menu->Launch->Exit
    void on_actionxrEngine_triggered();		// Menu->Launch->Game->xrEngine
    void on_actionOxygen_Team_triggered();
    void on_actionVertver_triggered();
    void on_actionGiperion_2_triggered();
    void on_actionForserX_triggered();

private:
    Ui::xrLaunch *ui;
};