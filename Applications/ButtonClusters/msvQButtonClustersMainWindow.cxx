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
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>

// MSV includes
#include "msvQButtonClustersMainWindow.h"
#include "msvVTKWidgetClusters.h"
#include "ui_msvQButtonClustersMainWindow.h"
#include "msvQButtonClustersAboutDialog.h"

// VTK includes
#include "vtkActor.h"
#include "vtkAppendPolyData.h"
#include "vtkAxesActor.h"
#include "vtkAxis.h"
#include "vtkCamera.h"
#include "vtkColorTransferFunction.h"
#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkNew.h"
#include "vtkOrientationMarkerWidget.h"
#include "vtkPiecewiseFunction.h"
#include "vtkPolyDataReader.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkSmartVolumeMapper.h"
#include "vtkSmartPointer.h"
#include "vtkStructuredPoints.h"
#include "vtkStructuredPointsReader.h"
#include "vtkVolumeProperty.h"
#include "vtkVolume.h"


// ------------------------------------------------------------------------------
class msvQButtonClustersMainWindowPrivate
  : public Ui_msvQButtonClustersMainWindow
{
  Q_DECLARE_PUBLIC(msvQButtonClustersMainWindow);
protected:
  msvQButtonClustersMainWindow* const q_ptr;

  // Scene Rendering
  vtkSmartPointer<vtkOrientationMarkerWidget> OrientationMarker;
  vtkSmartPointer<vtkAxesActor>               Axes;
  vtkSmartPointer<vtkRenderer>                ThreeDRenderer;
  
  // Segmentation Pipeline
  vtkSmartPointer<vtkAppendPolyData> PolyDataMerger;
  
  // Volume Pipeline
  vtkSmartPointer<vtkStructuredPointsReader> VolumeDataReader;
  vtkSmartPointer<vtkPiecewiseFunction>      ScalarOpacity;
  vtkSmartPointer<vtkPiecewiseFunction>      GradientOpacity;
  vtkSmartPointer<vtkColorTransferFunction>  ColorTransferFunction;
  vtkSmartPointer<vtkSmartVolumeMapper>      VolumeMapper;
  vtkSmartPointer<vtkVolumeProperty>         VolumeProperty;
  vtkSmartPointer<vtkVolume>                 Volume;

  vtkSmartPointer<vtkPolyDataMapper>       SurfaceMapper;
  vtkSmartPointer<vtkActor>                SurfaceActor;

  // buttonsManager
  vtkSmartPointer<msvVTKWidgetClusters> ButtonsManager;

  class EndInteractionCallbackCommand;
  EndInteractionCallbackCommand *EndInteractionCommand;

public:
  msvQButtonClustersMainWindowPrivate(msvQButtonClustersMainWindow& object);
  ~msvQButtonClustersMainWindowPrivate();

  virtual void setup(QMainWindow*);
  virtual void setupUi(QMainWindow*);
  virtual void setupView();
  virtual void update();
  virtual void updateUi();
  virtual void updateView();

  virtual void showVolume(bool value);
  virtual void showLevel(int value);
  virtual void showDiscs(bool value);
  virtual void enableClustering(bool value);
  virtual void setPixelRadius(double value);

  virtual void clear();

  virtual void readData(const QString&);

  void readSegmentedData(QDir dir, int level);
  void readVolumeData(QDir dir, int level);

};

// ------------------------------------------------------------------------------
// msvQButtonClustersMainWindowPrivate methods

// ------------------------------------------------------------------------------
class msvQButtonClustersMainWindowPrivate::EndInteractionCallbackCommand :
  public vtkCommand
{
public:
  static EndInteractionCallbackCommand *New()
  {
    return new EndInteractionCallbackCommand;
  };
  msvQButtonClustersMainWindowPrivate *Self;

  void Execute(vtkObject *, unsigned long, void *)
  {
    if (this->Self)
      {
      this->Self->update();
      }
  }
protected:
  EndInteractionCallbackCommand() {
    this->Self = NULL;
  };
  ~EndInteractionCallbackCommand() {
  };
};

// ------------------------------------------------------------------------------
msvQButtonClustersMainWindowPrivate::msvQButtonClustersMainWindowPrivate(
  msvQButtonClustersMainWindow& object)
  : q_ptr(&object)
{
  // Renderer
  this->ThreeDRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->ThreeDRenderer->SetBackground(0.1, 0.2, 0.4);
  this->ThreeDRenderer->SetBackground2(0.2, 0.4, 0.8);
  this->ThreeDRenderer->SetGradientBackground(true);

  this->Axes = vtkSmartPointer<vtkAxesActor>::New();
  
  this->OrientationMarker = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
  this->OrientationMarker->SetOutlineColor(0.9300, 0.5700, 0.1300);
  this->OrientationMarker->SetOrientationMarker(Axes);

  // Data merger
  this->PolyDataMerger = vtkSmartPointer<vtkAppendPolyData>::New();

  // Create Pipeline for the Points
  this->SurfaceMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->SurfaceMapper->ScalarVisibilityOff();
  this->SurfaceMapper->SetInputConnection(
    this->PolyDataMerger->GetOutputPort());
  this->SurfaceActor = vtkSmartPointer<vtkActor>::New();
  this->SurfaceActor->GetProperty()->SetColor(226. / 255., 93. /255., 94. /
    255.);
  this->SurfaceActor->GetProperty()->BackfaceCullingOn();
  this->SurfaceActor->SetMapper(this->SurfaceMapper);

  // Volume reader
  this->VolumeDataReader =
    vtkSmartPointer<vtkStructuredPointsReader>::New();
  this->ScalarOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
  this->ScalarOpacity->AddPoint(20.0,0.0);
  this->ScalarOpacity->AddPoint(500.0,0.15);
  this->ScalarOpacity->AddPoint(1000.0,0.15);
  this->ScalarOpacity->AddPoint(1150.0,0.85);

  this->GradientOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
  this->GradientOpacity->AddPoint(0.0,0.0);
  this->GradientOpacity->AddPoint(90.0,0.5);
  this->GradientOpacity->AddPoint(100.0,0.1);

  this->ColorTransferFunction =
    vtkSmartPointer<vtkColorTransferFunction>::New();
  this->ColorTransferFunction->AddRGBPoint(0.0,0.0,0.0,0.0);
  this->ColorTransferFunction->AddRGBPoint(500,  1.0, 0.5, 0.3);
  this->ColorTransferFunction->AddRGBPoint(1000, 1.0, 0.5, 0.3);
  this->ColorTransferFunction->AddRGBPoint(1150, 1.0, 1.0, 0.9);

  this->VolumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
  this->VolumeMapper->SetInputConnection(this->VolumeDataReader->GetOutputPort());
  this->VolumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
  this->VolumeProperty->SetColor(this->ColorTransferFunction);
  this->VolumeProperty->SetScalarOpacity(this->ScalarOpacity);
  this->VolumeProperty->SetGradientOpacity(this->GradientOpacity);
  this->VolumeProperty->SetInterpolationTypeToLinear();
  this->VolumeProperty->ShadeOn(); this->VolumeProperty->SetAmbient(0.4);
  this->VolumeProperty->SetDiffuse(0.6); this->VolumeProperty->SetSpecular(0.2);
  this->Volume = vtkSmartPointer<vtkVolume>::New();
  this->Volume->SetMapper(this->VolumeMapper);
  this->Volume->SetProperty(this->VolumeProperty);

  // Set the buttons manager
  this->ButtonsManager = vtkSmartPointer<msvVTKWidgetClusters>::New();
  this->ButtonsManager->SetUseImprovedClustering(true);

  // Set interaction callback to track when interaction ended
  this->EndInteractionCommand       = EndInteractionCallbackCommand::New();
  this->EndInteractionCommand->Self = this;
  this->ButtonsManager->AddObserver(vtkCommand::UpdateDataEvent,
    this->EndInteractionCommand);

}

// ------------------------------------------------------------------------------
msvQButtonClustersMainWindowPrivate::~msvQButtonClustersMainWindowPrivate()
{
  this->clear();
  if(this->EndInteractionCommand)
    {
    this->EndInteractionCommand->Delete();
    }
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindowPrivate::showDiscs(bool value)
{
  this->SurfaceActor->SetVisibility(value);
  this->updateView();
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindowPrivate::showVolume(bool value)
{
  this->Volume->SetVisibility(value);
  this->updateView();
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindowPrivate::enableClustering(bool value)
{
  this->ButtonsManager->SetClustering(value);
  if(value)
  {
    this->ButtonsManager->HideButtons();
  }
  else
  {
    this->ButtonsManager->ShowButtons();
  }
  this->ButtonsManager->UpdateWidgets();
  this->update();
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindowPrivate::showLevel(int value)
{
  int numberOfLevels = this->ButtonsManager->GetNumberOfLevels();
  for(int i = 0; i < numberOfLevels; ++i)
  {
    if(i == value)
    {
    this->ButtonsManager->ShowClusterButtons(i);
    }
    else
    {
    this->ButtonsManager->HideClusterButtons(i);
    }
  }
  this->updateView();
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindowPrivate::setPixelRadius(double value)
{
  this->ButtonsManager->SetPixelRadius(value);
  this->ButtonsManager->UpdateWidgets();
  this->update();
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindowPrivate::clear()
{
  this->ThreeDRenderer->RemoveAllViewProps();     // clean up the renderer
  this->ButtonsManager->Clear();                  // clean up the buttonsManager
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindowPrivate::setup(QMainWindow * mainWindow)
{
  this->setupUi(mainWindow);
  this->setupView();
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindowPrivate::setupUi(QMainWindow * mainWindow)
{
  Q_Q(msvQButtonClustersMainWindow);

  this->Ui_msvQButtonClustersMainWindow::setupUi(mainWindow);

  this->vtkButtonsReviewPanel->toggleViewAction()->setText(
    "Clusters review panel");
  this->vtkButtonsReviewPanel->toggleViewAction()->setShortcut(QKeySequence(
      "Ctrl+1"));
  this->menuView->addAction(this->vtkButtonsReviewPanel->toggleViewAction());

  q->setStatusBar(0);

  // Connect Menu ToolBars actions
  q->connect(this->actionOpen, SIGNAL(triggered()), q, SLOT(openData()));
  q->connect(this->actionClose, SIGNAL(triggered()), q, SLOT(closeData()));
  q->connect(this->actionExit, SIGNAL(triggered()), q, SLOT(close()));
  q->connect(this->actionAboutButtonClustersApplication, SIGNAL(triggered()),
             q, SLOT(aboutApplication()));

  // Customize QAction icons with standard pixmaps
  QIcon dirIcon         = q->style()->standardIcon(QStyle::SP_DirIcon);
  QIcon informationIcon = q->style()->standardIcon(
    QStyle::SP_MessageBoxInformation);

  this->actionOpen->setIcon(dirIcon);
  this->actionAboutButtonClustersApplication->setIcon(informationIcon);

}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindowPrivate::setupView()
{
  this->threeDView->GetRenderWindow()->AddRenderer(this->ThreeDRenderer);

  // Marker annotation
  this->OrientationMarker->SetInteractor
    (this->ThreeDRenderer->GetRenderWindow()->GetInteractor());
  this->OrientationMarker->SetEnabled(1);
  this->OrientationMarker->InteractiveOff();

  this->ButtonsManager->SetRenderer(this->ThreeDRenderer);
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindowPrivate::update()
{
  this->updateUi();
  this->updateView();
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindowPrivate::updateUi()
{
  this->PixelRadius->setValue(this->ButtonsManager->GetPixelRadius());
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindowPrivate::updateView()
{
  this->threeDView->GetRenderWindow()->Render();
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindowPrivate::readVolumeData(QDir dir, int level)
{
  if(dir.exists("Volume.vtk"))
    {
    this->VolumeDataReader->SetFileName(dir.absoluteFilePath(
        "Volume.vtk").toLatin1().constData());
    this->VolumeDataReader->Update();

    vtkNew<vtkPoints> points;

    double *bounds   = this->VolumeDataReader->GetOutput()->GetBounds();
    double  point[3] = {bounds[0],bounds[2],bounds[4]};
    points->InsertNextPoint(point);
    this->ButtonsManager->SetDataSet(level,0,points.GetPointer());
    }
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindowPrivate::readSegmentedData(QDir dir, int level)
{
  QStringList polyDataFiles = dir.entryList(QDir::Files,QDir::Name).filter(
    "result");
  unsigned int piece = 0;
  foreach(const QString &file, polyDataFiles)
    {
    vtkNew<vtkPolyDataReader> reader;
    vtkNew<vtkPoints>         points;

    QString fileName = QFileInfo(dir,file).absoluteFilePath();

    reader->SetFileName(fileName.toLatin1().constData());
    reader->Update();

    vtkIdType numPoints = reader->GetOutput()->GetPoints()->GetNumberOfPoints();
    vtkIdType N = 10;
    numPoints = numPoints > N ? numPoints/N : numPoints/(N/2);

    for(vtkIdType i = 0; i < N; ++i)
    {
      double point[3] = {0};
      reader->GetOutput()->GetPoints()->GetPoint(i*numPoints,point);
      points->InsertNextPoint(point);
    }
    
    
    // Add points to the widget manager
    this->ButtonsManager->SetDataSet(level,piece++,points.GetPointer());

    this->PolyDataMerger->AddInput(reader->GetOutput());
    }
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindowPrivate::readData(const QString& rootDirectory)
{
  QDir dir(rootDirectory);

  if (dir.cd(QString("Segmentation")))
    {
    QStringList filters;
    filters << "*.vtk";
    dir.setNameFilters(filters);
    readVolumeData(dir,0);
    readSegmentedData(dir,1);

    this->ShowLevel->clear();
    this->ShowLevel->addItem("Level 0");
    this->ShowLevel->addItem("Level 1");
    }
  else
    {
    return;
    }

  // Render
  this->ThreeDRenderer->AddViewProp(this->Volume);
  vtkCamera *camera = this->ThreeDRenderer->GetActiveCamera();
  double *   c      = this->Volume->GetCenter();
  camera->SetFocalPoint(c[0], c[1], c[2]);
  camera->SetPosition(c[0], c[1], c[2] + 1000);
  this->ThreeDRenderer->AddActor(this->SurfaceActor);

}

// ------------------------------------------------------------------------------
// msvQButtonClustersMainWindow methods

// ------------------------------------------------------------------------------
msvQButtonClustersMainWindow::msvQButtonClustersMainWindow(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new msvQButtonClustersMainWindowPrivate(*this))
{
  Q_D(msvQButtonClustersMainWindow);
  d->setup(this);
}

// ------------------------------------------------------------------------------
msvQButtonClustersMainWindow::~msvQButtonClustersMainWindow()
{
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindow::openData()
{
  Q_D(msvQButtonClustersMainWindow);

  QString dir = QFileDialog::getExistingDirectory(
    this, tr("Select root Spine Folder"), QDir::homePath(),
    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if(dir.isEmpty())
    {
    return;
    }
  if (!dir.contains("Spine"))
    {
    QMessageBox err;
    err.setWindowTitle(tr("Error!"));
    err.setText(tr("Wrong data path."));
    err.setDetailedText(tr(
        "Make sure you are pointing to the correct directory.\n"
        "The directory should contain the following directory:\n"
        "Segmentation"));
    err.exec();
    return;
    }

  d->clear();             // Clean Up data and scene
  d->readData(dir);  // Load data
  d->update();            // Update the Ui and the View
}


// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindow::on_ShowVolume_stateChanged(int state)
{
  Q_D(msvQButtonClustersMainWindow);

  d->showVolume(state);
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindow::on_ShowDiscs_stateChanged(int state)
{
  Q_D(msvQButtonClustersMainWindow);

  d->showDiscs(state);
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindow::on_EnableClustering_stateChanged(int state)
{
  Q_D(msvQButtonClustersMainWindow);

  d->enableClustering(state);
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindow::on_PixelRadius_valueChanged(double value)
{
  Q_D(msvQButtonClustersMainWindow);

  d->setPixelRadius(value);
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindow::on_ShowLevel_currentIndexChanged(int value)
{
  Q_D(msvQButtonClustersMainWindow);

  d->showLevel(value);
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindow::closeData()
{
  Q_D(msvQButtonClustersMainWindow);

  d->clear();
  d->update();
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindow::aboutApplication()
{
  msvQButtonClustersAboutDialog about(this);
  about.exec();
}

// ------------------------------------------------------------------------------
void msvQButtonClustersMainWindow::updateView()
{
  Q_D(msvQButtonClustersMainWindow);

  d->updateView();
}
