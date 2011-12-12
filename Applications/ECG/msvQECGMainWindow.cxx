/*==============================================================================

  Library: MSVTK

  Copyright (c) Kitware Inc.

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
#include "QFileDialog"
#include "QRegExp"
#include "QString"

// MSV includes
#include "msvQECGMainWindow.h"
#include "msvQTimePlayerWidget.h"
#include "msvVTKECGButtonsManager.h"
#include "msvVTKPolyDataFileSeriesReader.h"
#include "ui_msvQECGMainWindow.h"

// VTK includes
#include "vtkActor.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkSmartPointer.h"

//-----------------------------------------------------------------------------
class msvQECGMainWindowPrivate: public Ui_msvQECGMainWindow
{
  Q_DECLARE_PUBLIC(msvQECGMainWindow);
protected:
  msvQECGMainWindow* const q_ptr;

  void readCartoPoints(QDir&);

  vtkSmartPointer<vtkRenderer>        threeDRenderer;

  // CartoPoints Pipeline
  vtkSmartPointer<msvVTKPolyDataFileSeriesReader> cartoPointsReader;
  vtkSmartPointer<vtkPolyDataReader>              polyDataReader;
  vtkSmartPointer<vtkPolyDataMapper>              cartoPointsMapper;
  vtkSmartPointer<vtkActor>                       cartoPointsActor;

  // buttonsManager
  vtkSmartPointer<msvVTKECGButtonsManager> buttonsManager;

public:
  msvQECGMainWindowPrivate(msvQECGMainWindow& object);
  ~msvQECGMainWindowPrivate();

  virtual void setup(QMainWindow*);
  virtual void setupUi(QMainWindow*);
  virtual void setupView();
  virtual void update();
  virtual void updateUi();
  virtual void updateView();

  virtual void clear();

  virtual void readCartoData(const QString&);
  static bool fileLessThan(const QString &, const QString &);
};

//-----------------------------------------------------------------------------
// msvQECGMainWindowPrivate methods

msvQECGMainWindowPrivate::msvQECGMainWindowPrivate(msvQECGMainWindow& object)
  : q_ptr(&object)
{
  // Renderer
  this->threeDRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->threeDRenderer->SetBackground(0.1, 0.2, 0.4);

  // CartoPoints Readers
  this->polyDataReader    = vtkSmartPointer<vtkPolyDataReader>::New();
  this->cartoPointsReader =
    vtkSmartPointer<msvVTKPolyDataFileSeriesReader>::New();
  this->cartoPointsReader->SetReader(this->polyDataReader);

  // Create Pipeline for the CartoPoints
  this->cartoPointsMapper = vtkPolyDataMapper::New();
  this->cartoPointsMapper->ScalarVisibilityOff();
  this->cartoPointsMapper->SetInputConnection(
    this->cartoPointsReader->GetOutputPort());
  this->cartoPointsActor = vtkActor::New();
  this->cartoPointsActor->SetMapper(this->cartoPointsMapper);
}

//-----------------------------------------------------------------------------
msvQECGMainWindowPrivate::~msvQECGMainWindowPrivate()
{
  this->clear();
}

//-----------------------------------------------------------------------------
void msvQECGMainWindowPrivate::clear()
{
  this->timePlayerWidget->play(false);            // stop the player widget
  this->threeDRenderer->RemoveAllViewProps();     // clean up the renderer
  this->cartoPointsReader->RemoveAllFileNames();  // clean up the reader
  this->cartoPointsMapper->Update();              // update the pipeline
  this->timePlayerWidget->updateFromFilter();     // update the player widget
  this->buttonsManager->Clear();                  // clean up the buttonsManager
}

//-----------------------------------------------------------------------------
void msvQECGMainWindowPrivate::setup(QMainWindow * mainWindow)
{
  Q_Q(msvQECGMainWindow);

  this->setupUi(mainWindow);
  this->setupView();
  q->setupMenuActions();
}

//-----------------------------------------------------------------------------
void msvQECGMainWindowPrivate::setupUi(QMainWindow * mainWindow)
{
  this->Ui_msvQECGMainWindow::setupUi(mainWindow);

  // Connect Menu ToolBars actions
  q->connect(this->actionOpen, SIGNAL(triggered()), q, SLOT(openData()));
  q->connect(this->actionClose, SIGNAL(triggered()), q, SLOT(closeData()));
  q->connect(this->actionExit, SIGNAL(triggered()), q, SLOT(close()));

  // Playback Controller
  q->connect(this->timePlayerWidget, SIGNAL(currentTimeChanged(double)), q, SLOT(updateView()));

  // Associate the TimePlayerWidget to the sink (mapper)
  this->timePlayerWidget->setFilter(this->cartoPointsMapper);
}

//-----------------------------------------------------------------------------
void msvQECGMainWindowPrivate::setupView()
{
  this->threeDView->GetRenderWindow()->AddRenderer(this->threeDRenderer);
}

//-----------------------------------------------------------------------------
void msvQECGMainWindowPrivate::update()
{
  this->updateUi();
  this->updateView();
}

//-----------------------------------------------------------------------------
void msvQECGMainWindowPrivate::updateUi()
{}

//-----------------------------------------------------------------------------
void msvQECGMainWindowPrivate::updateView()
{
  this->threeDView->GetRenderWindow()->Render();
}

//-----------------------------------------------------------------------------
void msvQECGMainWindowPrivate::readCartoData(const QString& rootDirectory)
{
  QDir dir(rootDirectory);

  //if (dir.cd(QString("CartoSignals"))) {
  //  this->readCartoSignals
  //  dir.cd(QString(".."))
  //}

  if (dir.cd(QString("CartoPoints")))
    this->readCartoPoints(dir);


  // Render
  double extent[6];
  this->cartoPointsMapper->GetBounds(extent);
  this->cartoPointsActor->VisibilityOn();
  this->threeDRenderer->AddActor(this->cartoPointsActor);
  this->threeDRenderer->ResetCamera(extent);
  this->update();
}

void msvQECGMainWindowPrivate::readCartoPoints(QDir& dir)
{
  // Set file series patterns recognition
  QStringList filters;
  filters << "*.vtk";
  dir.setNameFilters(filters);
  QStringList files = dir.entryList(QDir::Files,QDir::Name);

  // Resort files using the index number
  qSort(files.begin(), files.end(), msvQECGMainWindowPrivate::fileLessThan);

  // Fill the FileSerieReader
  QStringList::const_iterator constIt;
  for (constIt = files.constBegin(); constIt != files.constEnd(); ++constIt)
    this->cartoPointsReader->AddFileName(
      dir.filePath(*constIt).toLocal8Bit().constData());

  // Create Instance of vtkDataObject for all outputs ports
  // Call REQUEST_DATA_OBJECT && REQUEST_INFORMATION
  this->cartoPointsReader->UpdateInformation();
  this->cartoPointsReader->SetOutputTimeRange(0,2500);

  // Update the Widget given the info provided
  this->timePlayerWidget->updateTimeInformation();
}

bool msvQECGMainWindowPrivate::fileLessThan(const QString &s1, const QString &s2)
{
  // Compare file by the index contained within.
  QString fileA, fileB;
  QRegExp indexExp("(\\d+)");

  int pos = indexExp.indexIn(s1);
  fileA = (pos > -1) ? indexExp.cap() : "0";
  pos = indexExp.indexIn(s2);
  fileB = (pos > -1) ? indexExp.cap() : "0";

  return fileA.toInt() < fileB.toInt();
}

//-----------------------------------------------------------------------------
// msvQECGMainWindow methods

//-----------------------------------------------------------------------------
msvQECGMainWindow::msvQECGMainWindow(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new msvQECGMainWindowPrivate(*this))
{
  Q_D(msvQECGMainWindow);
  d->setup(this);
}

//-----------------------------------------------------------------------------
msvQECGMainWindow::~msvQECGMainWindow()
{}

//-----------------------------------------------------------------------------
void msvQECGMainWindow::setupMenuActions()
{
  Q_D(msvQECGMainWindow);

  // Connect Menu ToolBars actions
  this->connect(d->actionOpen, SIGNAL(triggered()), this, SLOT(openData()));
  this->connect(d->actionClose, SIGNAL(triggered()), this, SLOT(closeData()));
  this->connect(d->actionExit, SIGNAL(triggered()), this, SLOT(close()));

  // Playback Controller
  this->connect(d->timePlayerWidget, SIGNAL(timestepChanged()), this, SLOT(updateView()));
}

//-----------------------------------------------------------------------------
void msvQECGMainWindow::openData()
{
  Q_D(msvQECGMainWindow);

  QString dir = QFileDialog::getExistingDirectory(
    this, tr("Select root CartoData Folder"), QDir::homePath(),
    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  if (dir.isNull())
    return;

  d->clear();             // Clean Up data and scene
  d->readCartoData(dir);  // Load data
  d->update();            // Update the Ui and the View
}

//-----------------------------------------------------------------------------
void msvQECGMainWindow::closeData()
{
  Q_D(msvQECGMainWindow);

  d->clear();
  d->update();
}

//-----------------------------------------------------------------------------
void msvQECGMainWindow::updateView()
{
  Q_D(msvQECGMainWindow);

  d->updateView();
}
