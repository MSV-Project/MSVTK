/*==============================================================================

  Library: MSVTK

  Copyright (c) The University of Auckland

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QFileDialog>
#include <QRegExp>
#include <QString>

// MSV includes
#include "msvQGridViewerMainWindow.h"
#include "msvGridViewerPipeline.h"
#include "msvQTimePlayerWidget.h"
#include "msvVTKFileSeriesReader.h"
#include "ui_msvQGridViewerMainWindow.h"
#include "msvQGridViewerAboutDialog.h"

// VTK includes
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"

//------------------------------------------------------------------------------
class msvQGridViewerMainWindowPrivate: public Ui_msvQGridViewerMainWindow
{
  Q_DECLARE_PUBLIC(msvQGridViewerMainWindow);
protected:
  msvQGridViewerMainWindow* const q_ptr;
  msvGridViewerPipeline gridPipeline;

public:
  msvQGridViewerMainWindowPrivate(msvQGridViewerMainWindow& object);
  ~msvQGridViewerMainWindowPrivate();

  virtual void setup(QMainWindow*);
  virtual void setupUi(QMainWindow*);
  virtual void setupView();
  virtual void update();
  virtual void updateView();

  virtual void clear();

  virtual void readGridData(const QString&);
};

//------------------------------------------------------------------------------
// msvQGridViewerMainWindowPrivate methods

//------------------------------------------------------------------------------
msvQGridViewerMainWindowPrivate::msvQGridViewerMainWindowPrivate(msvQGridViewerMainWindow& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
msvQGridViewerMainWindowPrivate::~msvQGridViewerMainWindowPrivate()
{
  this->clear();
}

//------------------------------------------------------------------------------
void msvQGridViewerMainWindowPrivate::clear()
{
  Q_Q(msvQGridViewerMainWindow);

  this->timePlayerWidget->play(false);            // stop the player widget
  this->gridPipeline.clear();
  this->timePlayerWidget->setFilter(0);
  this->timePlayerWidget->updateFromFilter();     // update the player widget
}

//------------------------------------------------------------------------------
void msvQGridViewerMainWindowPrivate::setup(QMainWindow * mainWindow)
{
  this->setupUi(mainWindow);
  this->setupView();
}

//------------------------------------------------------------------------------
void msvQGridViewerMainWindowPrivate::setupUi(QMainWindow * mainWindow)
{
  Q_Q(msvQGridViewerMainWindow);

  this->Ui_msvQGridViewerMainWindow::setupUi(mainWindow);

  q->setStatusBar(0);

  // Connect Menu ToolBars actions
  q->connect(this->actionOpen, SIGNAL(triggered()), q, SLOT(openData()));
  q->connect(this->actionClose, SIGNAL(triggered()), q, SLOT(closeData()));
  q->connect(this->actionExit, SIGNAL(triggered()), q, SLOT(close()));
  q->connect(this->actionAboutGridViewerApplication, SIGNAL(triggered()), q,
             SLOT(aboutApplication()));

  // Playback Controller
  q->connect(this->timePlayerWidget, SIGNAL(currentTimeChanged(double)),
             q, SLOT(onCurrentTimeChanged(double)));

  // Customize QAction icons with standard pixmaps
  QIcon dirIcon = q->style()->standardIcon(QStyle::SP_DirIcon);
  QIcon informationIcon = q->style()->standardIcon(
    QStyle::SP_MessageBoxInformation);

  this->actionOpen->setIcon(dirIcon);
  this->actionAboutGridViewerApplication->setIcon(informationIcon);
}

//------------------------------------------------------------------------------
void msvQGridViewerMainWindowPrivate::setupView()
{
  this->gridPipeline.addToRenderWindow(this->threeDView->GetRenderWindow());
}

//------------------------------------------------------------------------------
void msvQGridViewerMainWindowPrivate::update()
{
  this->updateView();
}

//------------------------------------------------------------------------------
void msvQGridViewerMainWindowPrivate::updateView()
{
  this->threeDView->GetRenderWindow()->Render();
}

//------------------------------------------------------------------------------
void msvQGridViewerMainWindowPrivate::readGridData(const QString& gridFileName)
{
  Q_Q(msvQGridViewerMainWindow);
  QFileInfo fileInfo(gridFileName);
  QDir::setCurrent(fileInfo.absolutePath());
  std::string tmp = gridFileName.toStdString();
  this->gridPipeline.readGridFile(tmp.c_str());

  vtkAlgorithm *endMapper = this->gridPipeline.getEndMapper();
  this->timePlayerWidget->setFilter(endMapper);
  this->timePlayerWidget->updateFromFilter();
}

//------------------------------------------------------------------------------
// msvQGridViewerMainWindow methods

//------------------------------------------------------------------------------
msvQGridViewerMainWindow::msvQGridViewerMainWindow(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new msvQGridViewerMainWindowPrivate(*this))
{
  Q_D(msvQGridViewerMainWindow);
  d->setup(this);
}

//------------------------------------------------------------------------------
msvQGridViewerMainWindow::~msvQGridViewerMainWindow()
{
}

//------------------------------------------------------------------------------
void msvQGridViewerMainWindow::openData()
{
  Q_D(msvQGridViewerMainWindow);

  QString fileName = QFileDialog::getOpenFileName(
    this, tr("Choose MSV Grid Viewer file"), QDir::homePath(),
    tr("*.msv"), 0, QFileDialog::DontResolveSymlinks);
  if (fileName.isEmpty())
    return;

  d->clear();                // Clean Up data and scene
  d->readGridData(fileName); // Load data
  d->update();               // Update the Ui and the View
}

//------------------------------------------------------------------------------
void msvQGridViewerMainWindow::closeData()
{
  Q_D(msvQGridViewerMainWindow);

  d->clear();
  d->update();
}

//------------------------------------------------------------------------------
void msvQGridViewerMainWindow::aboutApplication()
{
  msvQGridViewerAboutDialog about(this);
  about.exec();
}

//------------------------------------------------------------------------------
void msvQGridViewerMainWindow::updateView()
{
  Q_D(msvQGridViewerMainWindow);

  d->updateView();
}

//------------------------------------------------------------------------------
void msvQGridViewerMainWindow::onCurrentTimeChanged(double time)
{
  Q_D(msvQGridViewerMainWindow);
  // update 3D view
  this->updateView();
}
