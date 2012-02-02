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
#include "msvQHAIMainWindow.h"
#include "msvQTimePlayerWidget.h"
#include "msvVTKPolyDataFileSeriesReader.h"
#include "ui_msvQHAIMainWindow.h"
#include "msvQHAIAboutDialog.h"

// VTK includes
#include "vtkActor.h"
#include "vtkAxesActor.h"
#include "vtkAxis.h"
#include "vtkChartXY.h"
#include "vtkCollection.h"
#include "vtkDelimitedTextReader.h"
#include "vtkDoubleArray.h"
#include "vtkNew.h"
#include "vtkOrientationMarkerWidget.h"
#include "vtkPlotBar.h"
#include "vtkPlotLine.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkSmartPointer.h"
#include "vtkTable.h"

//------------------------------------------------------------------------------
class msvQHAIMainWindowPrivate: public Ui_msvQHAIMainWindow
{
  Q_DECLARE_PUBLIC(msvQHAIMainWindow);
protected:
  msvQHAIMainWindow* const q_ptr;

  // Scene Rendering
  vtkSmartPointer<vtkRenderer> threeDRenderer;
  vtkSmartPointer<vtkAxesActor> axes;
  vtkSmartPointer<vtkOrientationMarkerWidget> orientationMarker;

  // CartoPoints Pipeline
  vtkSmartPointer<msvVTKPolyDataFileSeriesReader> cartoPointsReader;
  vtkSmartPointer<vtkPolyDataReader>              polyDataReader;
  vtkSmartPointer<vtkPolyDataMapper>              cartoPointsMapper;
  vtkSmartPointer<vtkActor>                       cartoPointsActor;

public:
  msvQHAIMainWindowPrivate(msvQHAIMainWindow& object);
  ~msvQHAIMainWindowPrivate();

  virtual void setup(QMainWindow*);
  virtual void setupUi(QMainWindow*);
  virtual void setupView();
  virtual void update();
  virtual void updateUi();

  virtual void clear();
};

//------------------------------------------------------------------------------
// msvQHAIMainWindowPrivate methods

//------------------------------------------------------------------------------
msvQHAIMainWindowPrivate::msvQHAIMainWindowPrivate(msvQHAIMainWindow& object)
  : q_ptr(&object)
{
  // Renderer
  this->threeDRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->threeDRenderer->SetBackground(0.1, 0.2, 0.4);
  this->threeDRenderer->SetBackground2(0.2, 0.4, 0.8);
  this->threeDRenderer->SetGradientBackground(true);

  this->axes = vtkSmartPointer<vtkAxesActor>::New();
  this->orientationMarker = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
  this->orientationMarker->SetOutlineColor(0.9300, 0.5700, 0.1300);
  this->orientationMarker->SetOrientationMarker(axes);

  // CartoPoints Readers
  this->polyDataReader    = vtkSmartPointer<vtkPolyDataReader>::New();
  this->cartoPointsReader =
    vtkSmartPointer<msvVTKPolyDataFileSeriesReader>::New();
  this->cartoPointsReader->SetReader(this->polyDataReader);

  // Create Pipeline for the CartoPoints
  this->cartoPointsMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->cartoPointsMapper->ScalarVisibilityOff();
  this->cartoPointsMapper->SetInputConnection(
    this->cartoPointsReader->GetOutputPort());
  this->cartoPointsActor = vtkSmartPointer<vtkActor>::New();
  this->cartoPointsActor->SetMapper(this->cartoPointsMapper);
}

//------------------------------------------------------------------------------
msvQHAIMainWindowPrivate::~msvQHAIMainWindowPrivate()
{
  this->clear();
}

//------------------------------------------------------------------------------
void msvQHAIMainWindowPrivate::clear()
{
  Q_Q(msvQHAIMainWindow);

  this->threeDRenderer->RemoveAllViewProps();     // clean up the renderer
  this->cartoPointsReader->RemoveAllFileNames();  // clean up the reader
  this->cartoPointsMapper->Update();              // update the pipeline
}

//------------------------------------------------------------------------------
void msvQHAIMainWindowPrivate::setup(QMainWindow * mainWindow)
{
  this->setupUi(mainWindow);
  this->setupView();
}

//------------------------------------------------------------------------------
void msvQHAIMainWindowPrivate::setupUi(QMainWindow * mainWindow)
{
  Q_Q(msvQHAIMainWindow);

  this->Ui_msvQHAIMainWindow::setupUi(mainWindow);
  //this->HAIReviewPanel->setVisible(false);
  q->setStatusBar(0);

  // Connect Menu ToolBars actions
  q->connect(this->actionOpen, SIGNAL(triggered()), q, SLOT(openData()));
  q->connect(this->actionClose, SIGNAL(triggered()), q, SLOT(closeData()));
  q->connect(this->actionExit, SIGNAL(triggered()), q, SLOT(close()));
  q->connect(this->actionAboutHAIApplication, SIGNAL(triggered()), q,
             SLOT(aboutApplication()));

  // Playback Controller
  q->connect(this->timePlayerWidget, SIGNAL(currentTimeChanged(double)),
             q, SLOT(onCurrentTimeChanged(double)));

  // Customize QAction icons with standard pixmaps
  QIcon dirIcon = q->style()->standardIcon(QStyle::SP_DirIcon);
  QIcon informationIcon = q->style()->standardIcon(
    QStyle::SP_MessageBoxInformation);

  this->actionOpen->setIcon(dirIcon);
  this->actionAboutHAIApplication->setIcon(informationIcon);

  // Associate the TimePlayerWidget to the sink (mapper)
  this->timePlayerWidget->setFilter(this->cartoPointsMapper);
}

//------------------------------------------------------------------------------
void msvQHAIMainWindowPrivate::setupView()
{
  this->threeDView->GetRenderWindow()->AddRenderer(this->threeDRenderer);

  // Marker annotation
  this->orientationMarker->SetInteractor
    (this->threeDRenderer->GetRenderWindow()->GetInteractor());
  this->orientationMarker->SetEnabled(1);
  this->orientationMarker->InteractiveOn();
}

//------------------------------------------------------------------------------
void msvQHAIMainWindowPrivate::update()
{
  this->updateUi();
  //this->updateView();
}

//------------------------------------------------------------------------------
void msvQHAIMainWindowPrivate::updateUi()
{
}

//------------------------------------------------------------------------------
// msvQHAIMainWindow methods

//------------------------------------------------------------------------------
msvQHAIMainWindow::msvQHAIMainWindow(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new msvQHAIMainWindowPrivate(*this))
{
  Q_D(msvQHAIMainWindow);
  d->setup(this);
}

//------------------------------------------------------------------------------
msvQHAIMainWindow::~msvQHAIMainWindow()
{
}

//------------------------------------------------------------------------------
void msvQHAIMainWindow::aboutApplication()
{
  msvQHAIAboutDialog about(this);
  about.exec();
}
