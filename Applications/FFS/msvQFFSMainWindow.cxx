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
#include <QRegExp>
#include <QString>

// MSV includes
#include "msvQFFSMainWindow.h"
#include "msvVTKImageDataFileSeriesReader.h"
#include "ui_msvQFFSMainWindow.h"
#include "msvQFFSAboutDialog.h"
#include "msvFluidSimulator.h"
#include "msvVTKBoundaryEdgeSources.h"

// VTK includes
#include <vtkActor.h>
#include <vtkAppendFilter.h>
#include <vtkAxesActor.h>
#include <vtkAxis.h>
#include <vtkBrush.h>
#include <vtkChartXY.h>
#include <vtkCollection.h>
#include <vtkColorTransferFunction.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkContourFilter.h>
#include <vtkCompositeDataPipeline.h>
#include <vtkCellDataToPointData.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkDelaunay3D.h>
#include <vtkDelimitedTextReader.h>
#include <vtkDoubleArray.h>
#include <vtkNew.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkOutlineCornerFilter.h>
#include <vtkPlotBar.h>
#include <vtkPlotLine.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkTable.h>
#include <vtkStructuredPointsReader.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkVolumeTextureMapper3D.h>
#include <vtkHierarchicalPolyDataMapper.h>
#include <vtkHierarchicalDataExtractLevel.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkVolume.h>
#include <vtkHierarchicalBoxDataSet.h>
#include <vtkShrinkPolyData.h>
#include <vtkHierarchicalDataSetGeometryFilter.h>
#include <vtkHierarchicalDataExtractLevel.h>
#include <vtkGlyph3D.h>
#include <vtkArrowSource.h>
#include <vtkPointData.h>
#include <vtkXMLHierarchicalBoxDataWriter.h>
#include <vtkFeatureEdges.h>
#include <vtkSelectPolyData.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>

//------------------------------------------------------------------------------
class msvQFFSMainWindowPrivate : public Ui_msvQFFSMainWindow
{
  Q_DECLARE_PUBLIC(msvQFFSMainWindow);
protected:
  msvQFFSMainWindow* const q_ptr;

  void readImmersedBoundary(QDir dir);
  // AMR Rendering
  vtkSmartPointer<vtkDataSetSurfaceFilter> amrLagrangianSurfaceFilter;
  vtkSmartPointer<vtkPolyDataMapper>       amrLagrangianSurfaceMapper;
  vtkSmartPointer<vtkActor>                amrLagrangianSurfaceActor;
  vtkSmartPointer<vtkFeatureEdges>         amrFeatureEdgesFilter;
  vtkSmartPointer<vtkSelectPolyData>       amrSelectBoundaryFilter;
  vtkSmartPointer<vtkActor>                amrSelectBoundaryActor;
  vtkSmartPointer<vtkPolyDataMapper>       amrSelectBoundaryMapper;
  // Scene Rendering
  vtkSmartPointer<vtkRenderer>                          threeDRenderer;
  vtkSmartPointer<vtkAxesActor>                         axes;
  vtkSmartPointer<vtkOrientationMarkerWidget>           orientationMarker;
  vtkSmartPointer<vtkPolyDataReader>                    polyDataReader;
  vtkSmartPointer<vtkPolyDataMapper>                    amrLagrangianMapper;
  vtkSmartPointer<vtkActor>                             amrLagrangianActor;
  vtkSmartPointer<vtkHierarchicalPolyDataMapper>        amrHierarchicalMapper;
  vtkSmartPointer<vtkHierarchicalPolyDataMapper>        amrOutlineCornerMapper;
  vtkSmartPointer<vtkOutlineCornerFilter>               amrOutlineCornerFilter;
  vtkSmartPointer<vtkHierarchicalDataExtractLevel>      amrExtractLevel;
  vtkSmartPointer<vtkCellDataToPointData>               amrCellToPointFilter;
  vtkSmartPointer<vtkShrinkPolyData>                    amrShrinkFilter;
  vtkSmartPointer<vtkContourFilter>                     amrContour;
  vtkSmartPointer<vtkHierarchicalPolyDataMapper>        amrContourMapper;
  vtkSmartPointer<vtkActor>                             amrContourActor;
  vtkSmartPointer<vtkActor>                             amrHierarchicalActor;
  vtkSmartPointer<vtkActor>                             amrOutlineCornerActor;
  vtkSmartPointer<vtkHierarchicalDataSetGeometryFilter> amrGeometryFilter;
  vtkSmartPointer<vtkXMLHierarchicalBoxDataWriter>      amrDataWriter;
  vtkSmartPointer<vtkGlyph3D>                           vectorFilter;
  vtkSmartPointer<vtkArrowSource>                       arrowSource;
  vtkSmartPointer<vtkPolyDataMapper>                    vectorMapper;
  vtkSmartPointer<vtkActor>                             vectorActor;
  vtkSmartPointer<vtkAppendFilter>                      mergedMapper;
  vtkSmartPointer<msvVTKBoundaryEdgeSources>            sourceFilter;
  vtkSmartPointer<vtkPolyDataMapper>                    sourceMapper;
  vtkSmartPointer<vtkActor>                             sourceActor;
  vtkSmartPointer<vtkTransformFilter>                   translatePolyData;
  vtkSmartPointer<vtkTransform>                         transform;
  vtkSmartPointer<msvFluidSimulator>                    fluidSimulator;

  unsigned int       currentColor;
  const unsigned int colorCount;
  
  unsigned int numTimeSteps;
public:
  msvQFFSMainWindowPrivate(msvQFFSMainWindow& object);
  ~msvQFFSMainWindowPrivate();

  virtual void setup(QMainWindow*);
  virtual void setupUi(QMainWindow*);
  virtual void setupView();
  virtual void update();
  virtual void updateUi();
  virtual void updateView();
  virtual void runTimeStep();
  virtual void numberOfTimeSteps(int value);

  void renderSurfaceVectorField();

  virtual void clear();

};

//------------------------------------------------------------------------------
// msvQFFSMainWindowPrivate methods

//------------------------------------------------------------------------------
msvQFFSMainWindowPrivate::msvQFFSMainWindowPrivate(msvQFFSMainWindow& object)
  : q_ptr(&object)
  , currentColor(0)
  , colorCount(7)
{

  vtkSmartPointer<vtkCompositeDataPipeline> prototype =
    vtkSmartPointer<vtkCompositeDataPipeline>::New();
  vtkAlgorithm::SetDefaultExecutivePrototype(prototype);

  this->fluidSimulator = msvFluidSimulator::New();

  // Renderer
  this->threeDRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->threeDRenderer->SetBackground(0.1, 0.2, 0.4);
  this->threeDRenderer->SetBackground2(0.2, 0.4, 0.8);
  this->threeDRenderer->SetGradientBackground(true);

  // Add orientation axes
  this->axes              = vtkSmartPointer<vtkAxesActor>::New();
  this->orientationMarker = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
  this->orientationMarker->SetOutlineColor(0.9300, 0.5700, 0.1300);
  this->orientationMarker->SetOrientationMarker(axes);

  this->polyDataReader = vtkSmartPointer<vtkPolyDataReader>::New();

  this->transform         = vtkSmartPointer<vtkTransform>::New();
  this->translatePolyData = vtkSmartPointer<vtkTransformFilter>::New();
  this->translatePolyData->SetTransform(this->transform);
  this->translatePolyData->SetInput(this->polyDataReader->GetOutput());

  this->amrLagrangianSurfaceMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->amrLagrangianSurfaceMapper->SetInputConnection(
    0,this->translatePolyData->GetOutputPort(0));
  this->amrLagrangianSurfaceActor = vtkSmartPointer<vtkActor>::New();
  this->amrLagrangianSurfaceActor->SetMapper(this->amrLagrangianSurfaceMapper);

  // Add AMR Grid to pipeline
  this->amrGeometryFilter =
    vtkSmartPointer<vtkHierarchicalDataSetGeometryFilter>::New();
  this->amrGeometryFilter->SetInput(fluidSimulator->GetAMRDataset());

  this->amrShrinkFilter = vtkSmartPointer<vtkShrinkPolyData>::New();
  this->amrShrinkFilter->SetShrinkFactor(0.25);
  this->amrShrinkFilter->SetInputConnection(
    0, this->amrGeometryFilter->GetOutputPort(0));

  this->amrHierarchicalMapper =
    vtkSmartPointer<vtkHierarchicalPolyDataMapper>::New();
  this->amrHierarchicalMapper->SetInputConnection(
    0, this->amrShrinkFilter->GetOutputPort(0));

  this->amrHierarchicalActor = vtkSmartPointer<vtkActor>::New();
  this->amrHierarchicalActor->SetMapper(this->amrHierarchicalMapper);
  this->amrHierarchicalActor->GetProperty()->SetRepresentationToWireframe();

  // corner outline
  this->amrOutlineCornerFilter = vtkSmartPointer<vtkOutlineCornerFilter>::New();
  this->amrOutlineCornerMapper =
    vtkSmartPointer<vtkHierarchicalPolyDataMapper>::New();
  this->amrOutlineCornerFilter->SetInput(fluidSimulator->GetAMRDataset());
  this->amrOutlineCornerMapper->SetInputConnection(
    0,this->amrOutlineCornerFilter->GetOutputPort(0));
  this->amrOutlineCornerActor = vtkSmartPointer<vtkActor>::New();
  this->amrOutlineCornerActor->SetMapper(this->amrOutlineCornerMapper);

  this->amrFeatureEdgesFilter = vtkSmartPointer<vtkFeatureEdges>::New();
  this->amrFeatureEdgesFilter->SetInputConnection(
    0,this->translatePolyData->GetOutputPort(0));

  this->amrFeatureEdgesFilter->BoundaryEdgesOn();
  this->amrFeatureEdgesFilter->FeatureEdgesOff();
  this->amrFeatureEdgesFilter->NonManifoldEdgesOff();
  this->amrFeatureEdgesFilter->ColoringOff();
  
  this->amrSelectBoundaryMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->amrSelectBoundaryMapper->SetInputConnection(
    0,this->amrFeatureEdgesFilter->GetOutputPort(0));

  this->amrSelectBoundaryActor = vtkSmartPointer<vtkActor>::New();
  this->amrSelectBoundaryActor->SetMapper(this->amrSelectBoundaryMapper);

  this->sourceFilter = vtkSmartPointer<msvVTKBoundaryEdgeSources>::New();
  this->sourceFilter->SetInput(this->polyDataReader->GetOutput());

}

//------------------------------------------------------------------------------
void msvQFFSMainWindowPrivate::numberOfTimeSteps(int value)
{
  this->numTimeSteps = value;
}

//------------------------------------------------------------------------------
void msvQFFSMainWindowPrivate::runTimeStep()
{
  for(unsigned int i = 0; i < this->numTimeSteps; ++i)
  {
    this->fluidSimulator->Run();
  }
}

//------------------------------------------------------------------------------
void msvQFFSMainWindowPrivate::renderSurfaceVectorField()
{
  this->arrowSource = vtkSmartPointer<vtkArrowSource>::New();
  this->arrowSource->Update();

  this->vectorFilter = vtkSmartPointer<vtkGlyph3D>::New();
  this->vectorFilter->SetSourceConnection(0,
    this->arrowSource->GetOutputPort(0));
  this->vectorFilter->SetInputConnection(0,this->polyDataReader->GetOutputPort(
      0));
  this->vectorFilter->SetVectorModeToUseVector();
  this->vectorFilter->SetScaleModeToScaleByVector();
  this->vectorFilter->ScalingOn();
  this->vectorFilter->OrientOn();

  this->vectorFilter->SetInputArrayToProcess(0,
    this->polyDataReader->GetInformation());
  this->vectorMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->vectorMapper->SetInputConnection(0,
    this->vectorFilter->GetOutputPort(0));
  this->vectorActor = vtkSmartPointer<vtkActor>::New();
  this->vectorActor->SetMapper(this->vectorMapper);
}

//------------------------------------------------------------------------------
void msvQFFSMainWindowPrivate::readImmersedBoundary(QDir dir)
{
  if (dir.cd(QString("Morpho")))
    {
    this->polyDataReader->SetFileName(
      dir.filePath("geometry.vtk").toLatin1().constData());
    this->polyDataReader->Update();
    }
  else { return; }

  double bounds[6] = {0};
  this->polyDataReader->GetOutput()->GetBounds(bounds);
  std::cout << bounds[0] << " " << bounds[1] << " " << bounds[2] << " " << bounds[3] << " " << bounds[4] << " " << bounds[5] << " " << std::endl;
//   this->transform->Scale(.1,.1,.1);
  this->transform->Translate(-bounds[0],-bounds[2],-bounds[4]);
  this->translatePolyData->Update();
  vtkPolyData *data = vtkPolyData::SafeDownCast(this->translatePolyData->GetOutput());
  data->GetBounds(bounds);
  std::cout << bounds[0] << " " << bounds[1] << " " << bounds[2] << " " << bounds[3] << " " << bounds[4] << " " << bounds[5] << " " << std::endl;

  QFileInfo simulatorConfig(QDir(qApp->applicationDirPath()), "Resources/simulator.config");
  this->fluidSimulator->SetInitFile(simulatorConfig.absoluteFilePath().toLatin1());
  this->fluidSimulator->SetMaxLevels(8);
  this->fluidSimulator->SetDataLevel(5);
  this->fluidSimulator->SetRefinamentRatio(4);
  this->fluidSimulator->SetCoarsestGridSpacing(8);
  this->fluidSimulator->Init(data);
  // Render
  double extent[6];
  this->amrLagrangianSurfaceMapper->GetBounds(extent);
  this->threeDRenderer->AddActor(this->amrLagrangianSurfaceActor);
  this->threeDRenderer->AddActor(this->amrHierarchicalActor);
  this->threeDRenderer->AddActor(this->amrOutlineCornerActor);
//   this->threeDRenderer->AddActor(this->vectorActor);
  this->threeDRenderer->AddActor(this->amrSelectBoundaryActor);
//   this->threeDRenderer->AddActor(this->sourceActor);
  this->threeDRenderer->ResetCamera(extent);
}


//------------------------------------------------------------------------------
msvQFFSMainWindowPrivate::~msvQFFSMainWindowPrivate()
{
  this->clear();
}

//------------------------------------------------------------------------------
void msvQFFSMainWindowPrivate::clear()
{
  this->threeDRenderer->RemoveAllViewProps();     // clean up the renderer
}

//------------------------------------------------------------------------------
void msvQFFSMainWindowPrivate::setup(QMainWindow * mainWindow)
{
  this->setupUi(mainWindow);
  this->setupView();
}

//------------------------------------------------------------------------------
void msvQFFSMainWindowPrivate::setupUi(QMainWindow * mainWindow)
{
  Q_Q(msvQFFSMainWindow);

  this->Ui_msvQFFSMainWindow::setupUi(mainWindow);

  q->setStatusBar(0);

  // Connect Menu ToolBars actions
  q->connect(this->actionOpen, SIGNAL(triggered()), q, SLOT(openData()));
  q->connect(this->actionClose, SIGNAL(triggered()), q, SLOT(closeData()));
  q->connect(this->actionExit, SIGNAL(triggered()), q, SLOT(close()));
  q->connect(this->actionAboutFFSApplication, SIGNAL(triggered()), q,
    SLOT(aboutApplication()));

  // Customize QAction icons with standard pixmaps
  QIcon dirIcon         = q->style()->standardIcon(QStyle::SP_DirIcon);
  QIcon informationIcon = q->style()->standardIcon(
    QStyle::SP_MessageBoxInformation);

  this->actionOpen->setIcon(dirIcon);
  this->actionAboutFFSApplication->setIcon(informationIcon);
}

//------------------------------------------------------------------------------
void msvQFFSMainWindowPrivate::setupView()
{
  this->threeDView->GetRenderWindow()->AddRenderer(this->threeDRenderer);

  // Marker annotation
  this->orientationMarker->SetInteractor
    (this->threeDRenderer->GetRenderWindow()->GetInteractor());
  this->orientationMarker->SetEnabled(1);
  this->orientationMarker->InteractiveOn();
}

//------------------------------------------------------------------------------
void msvQFFSMainWindowPrivate::update()
{
  this->updateUi();
  this->updateView();
}

//------------------------------------------------------------------------------
void msvQFFSMainWindowPrivate::updateUi()
{

}

//------------------------------------------------------------------------------
void msvQFFSMainWindowPrivate::updateView()
{
  this->threeDView->GetRenderWindow()->Render();
}

//------------------------------------------------------------------------------
// msvQFFSMainWindow methods
//------------------------------------------------------------------------------
msvQFFSMainWindow::msvQFFSMainWindow(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new msvQFFSMainWindowPrivate(*this))
{
  Q_D(msvQFFSMainWindow);
  d->setup(this);
}

//------------------------------------------------------------------------------
msvQFFSMainWindow::~msvQFFSMainWindow()
{
}

//------------------------------------------------------------------------------
void msvQFFSMainWindow::openData()
{
  Q_D(msvQFFSMainWindow);

  QString dir = QFileDialog::getExistingDirectory(
    this, tr("Select root Data Folder"), QDir::homePath(),
    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (dir.isEmpty())
    return;

  d->clear();             // Clean Up data and scene
  d->readImmersedBoundary(dir);  // Load data
  d->update();            // Update the Ui and the View
}

//------------------------------------------------------------------------------
void msvQFFSMainWindow::closeData()
{
  Q_D(msvQFFSMainWindow);

  d->clear();
  d->update();
}

//------------------------------------------------------------------------------
void msvQFFSMainWindow::on_showCartesianGrid_stateChanged(int state)
{
  Q_D(msvQFFSMainWindow);
  if(state)
    {
    d->amrHierarchicalActor->VisibilityOn();
    }
  else
    {
    d->amrHierarchicalActor->VisibilityOff();
    }
}

//------------------------------------------------------------------------------
void msvQFFSMainWindow::on_showSurface_stateChanged(int state)
{
  Q_D(msvQFFSMainWindow);
  if(state)
    {
    d->amrLagrangianSurfaceActor->VisibilityOn();
    }
  else
    {
    d->amrLagrangianSurfaceActor->VisibilityOff();
    }
}

//------------------------------------------------------------------------------
void msvQFFSMainWindow::on_showBoundaryEdges_stateChanged(int state)
{
  Q_D(msvQFFSMainWindow);
  if(state)
    {
    d->amrSelectBoundaryActor->VisibilityOn();
    }
  else
    {
    d->amrSelectBoundaryActor->VisibilityOff();
    }
}

//------------------------------------------------------------------------------
void msvQFFSMainWindow::on_showOutlineCorners_stateChanged(int state)
{
  Q_D(msvQFFSMainWindow);
  if(state)
    {
    d->amrOutlineCornerActor->VisibilityOn();
    }
  else
    {
    d->amrOutlineCornerActor->VisibilityOff();
    }
}

//------------------------------------------------------------------------------
void msvQFFSMainWindow::on_numberOfTimeSteps_valueChanged(int value)
{
  Q_D(msvQFFSMainWindow);
  
  d->numberOfTimeSteps(value);
}

//------------------------------------------------------------------------------
void msvQFFSMainWindow::on_runTimeStep_clicked()
{
  Q_D(msvQFFSMainWindow);
  
  d->runTimeStep();
  
}

//------------------------------------------------------------------------------
void msvQFFSMainWindow::aboutApplication()
{
  msvQFFSAboutDialog about(this);
  about.exec();
}

//------------------------------------------------------------------------------
void msvQFFSMainWindow::updateView()
{
  Q_D(msvQFFSMainWindow);
  d->updateView();
}

