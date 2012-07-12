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
#include "msvQECGMainWindow.h"
#include "msvQTimePlayerWidget.h"
#include "msvVTKECGButtonsManager.h"
#include "msvVTKPolyDataFileSeriesReader.h"
#include "msvVTKImageDataFileSeriesReader.h"
#include "ui_msvQECGMainWindow.h"
#include "msvQECGAboutDialog.h"

// VTK includes
#include "vtkActor.h"
#include "vtkAppendFilter.h"
#include "vtkAxesActor.h"
#include "vtkAxis.h"
#include "vtkBrush.h"
#include "vtkChartXY.h"
#include "vtkCollection.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkDelaunay3D.h"
#include "vtkDelimitedTextReader.h"
#include "vtkDoubleArray.h"
#include "vtkNew.h"
#include "vtkOrientationMarkerWidget.h"
#include "vtkOutlineCornerFilter.h"
#include "vtkPlotBar.h"
#include "vtkPlotLine.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkRenderWindow.h"
#include "vtkSmartPointer.h"
#include "vtkTable.h"
#include "vtkStructuredPointsReader.h"
#include "vtkPiecewiseFunction.h"
#include "vtkColorTransferFunction.h"
#include "vtkVolumeProperty.h"
// #include "vtkVolumeRayCastMIPFunction.h"
// #include "vtkVolumeRayCastMapper.h"
#include "vtkVolumeTextureMapper3D.h"
#include "vtkHierarchicalPolyDataMapper.h"
#include "vtkHierarchicalDataExtractLevel.h"
#include "vtkCompositeDataGeometryFilter.h"
#include "vtkFixedPointVolumeRayCastMapper.h"
#include "vtkVolume.h"
#include "vtkHierarchicalBoxDataSet.h"
#include "msvFluidSimulator.h"
#include "vtkCompositeDataPipeline.h"
#include "vtkShrinkPolyData.h"
#include "vtkHierarchicalDataSetGeometryFilter.h"
#include "vtkContourFilter.h"
#include "vtkHierarchicalDataExtractLevel.h"
#include "vtkCellDataToPointData.h"
#include "vtkGlyph3D.h"
#include "vtkArrowSource.h"
#include "vtkPointData.h"
#include "vtkXMLHierarchicalBoxDataWriter.h"

const double colors[][3] = { {0.925490196,  0.17254902, 0.2},
                                       {0.070588235, 0.545098039, 0.290196078},
                                       {0.086274509, 0.364705882, 0.654901961},
                                       {0.952941176, 0.482352941, 0.176470588},
                                       {0.396078431, 0.196078431, 0.560784314},
                                       {0.631372549, 0.109803922, 0.176470588},
                                       {0.698039216, 0.235294118, 0.576470588},
                                       {0.003921568, 0.007843137, 0.007843137}};

const char simulator_config[] = {""};                                        

//------------------------------------------------------------------------------
class msvQECGMainWindowPrivate: public Ui_msvQECGMainWindow
{
  Q_DECLARE_PUBLIC(msvQECGMainWindow);
protected:
  msvQECGMainWindow* const q_ptr;
  
  void readImmersedBoundary(QDir dir);
  // Scene Rendering
  vtkSmartPointer<vtkRenderer> threeDRenderer;
  vtkSmartPointer<vtkAxesActor> axes;
  vtkSmartPointer<vtkOrientationMarkerWidget> orientationMarker;

  // CartoSignals
  vtkSmartPointer<vtkTable> currentTimeLine;

  // CartoPoints Pipeline
  vtkSmartPointer<msvVTKPolyDataFileSeriesReader>  boundaryReader;
  
  vtkSmartPointer<vtkPolyDataReader>              polyDataReader;
  
  vtkSmartPointer<vtkPolyDataMapper>              amrLagrangianMapper;
  vtkSmartPointer<vtkActor>                       amrLagrangianActor;
  
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
  
  vtkSmartPointer<vtkDelaunay3D>                  amrLagrangianDelaunayFilter;
  vtkSmartPointer<vtkDataSetSurfaceFilter>        amrLagrangianSurfaceFilter;
  vtkSmartPointer<vtkPolyDataMapper>              amrLagrangianSurfaceMapper;
  vtkSmartPointer<vtkActor>                       amrLagrangianSurfaceActor;
  
  vtkSmartPointer<vtkGlyph3D>                     vectorFilter;
  vtkSmartPointer<vtkArrowSource>                 arrowSource;
  vtkSmartPointer<vtkPolyDataMapper>              vectorMapper;
  vtkSmartPointer<vtkActor>                       vectorActor;

  vtkSmartPointer<vtkAppendFilter>                mergedMapper;

  // buttonsManager
  vtkSmartPointer<msvVTKECGButtonsManager> buttonsManager;
  
  // Fluid solver
  SAMRAI::tbox::Pointer<msvFluidSimulator> fluidSimulator;

  unsigned int currentColor;
  const unsigned int colorCount;
public:
  msvQECGMainWindowPrivate(msvQECGMainWindow& object);
  ~msvQECGMainWindowPrivate();

  virtual void setup(QMainWindow*);
  virtual void setupUi(QMainWindow*);
  virtual void setupView();
  virtual void update();
  virtual void updateUi();
  virtual void updateView();  
  
  void renderScalarDataContour();
  void renderSurfaceVectorField();

  virtual void clear();
  
  static bool fileLessThan(const QString &, const QString &);
    vtkSmartPointer< vtkPolyDataReader > New();
};

//------------------------------------------------------------------------------
// msvQECGMainWindowPrivate methods

//------------------------------------------------------------------------------
msvQECGMainWindowPrivate::msvQECGMainWindowPrivate(msvQECGMainWindow& object)
  : q_ptr(&object)
  , currentColor(0)
  , colorCount(7)
  , fluidSimulator(new msvFluidSimulator)
{

  vtkSmartPointer<vtkCompositeDataPipeline> prototype = vtkSmartPointer<vtkCompositeDataPipeline>::New();
  vtkAlgorithm::SetDefaultExecutivePrototype(prototype);

  // Add orientation axes
  this->axes = vtkSmartPointer<vtkAxesActor>::New();
  this->orientationMarker = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
  this->orientationMarker->SetOutlineColor(0.9300, 0.5700, 0.1300);
  this->orientationMarker->SetOrientationMarker(axes);
  
  this->polyDataReader = vtkSmartPointer<vtkPolyDataReader>::New();
  
  this->amrLagrangianSurfaceMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->amrLagrangianSurfaceMapper->SetInputConnection(0,this->polyDataReader->GetOutputPort(0));
  
  this->amrLagrangianSurfaceActor = vtkSmartPointer<vtkActor>::New();
  this->amrLagrangianSurfaceActor->SetMapper(this->amrLagrangianSurfaceMapper);
  
  // Renderer
  this->threeDRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->threeDRenderer->SetBackground(0.1, 0.2, 0.4);
  this->threeDRenderer->SetBackground2(0.2, 0.4, 0.8);
  this->threeDRenderer->SetGradientBackground(true);
  
  // Add AMR Grid to pipeline
  this->amrGeometryFilter = vtkSmartPointer<vtkHierarchicalDataSetGeometryFilter>::New();
  this->amrGeometryFilter->SetInput(fluidSimulator->getAMRDataSet());
  
  this->amrShrinkFilter = vtkSmartPointer<vtkShrinkPolyData>::New();
  this->amrShrinkFilter->SetShrinkFactor(0.25);
  this->amrShrinkFilter->SetInputConnection(0, this->amrGeometryFilter->GetOutputPort(0));
  
  this->amrHierarchicalMapper = vtkSmartPointer<vtkHierarchicalPolyDataMapper>::New();
  this->amrHierarchicalMapper->SetInputConnection(0, this->amrShrinkFilter->GetOutputPort(0));
  
  this->amrHierarchicalActor = vtkSmartPointer<vtkActor>::New();
  this->amrHierarchicalActor->SetMapper(this->amrHierarchicalMapper);
  
  // corner outline
  this->amrOutlineCornerFilter = vtkSmartPointer<vtkOutlineCornerFilter>::New();
  vtkCompositeDataPipeline* pipeline = vtkCompositeDataPipeline::New();
  this->amrOutlineCornerFilter->SetExecutive(pipeline);
  pipeline->Delete();
  this->amrOutlineCornerFilter->SetInput(fluidSimulator->getAMRDataSet());
  this->amrOutlineCornerMapper = vtkSmartPointer<vtkHierarchicalPolyDataMapper>::New();
  this->amrOutlineCornerMapper->SetInputConnection(0,this->amrOutlineCornerFilter->GetOutputPort(0));
  
  this->amrOutlineCornerActor = vtkSmartPointer<vtkActor>::New();
  this->amrOutlineCornerActor->SetMapper(this->amrOutlineCornerMapper);
  
  this->renderSurfaceVectorField();
  
  // AMR writer
  this->amrDataWriter = vtkSmartPointer<vtkXMLHierarchicalBoxDataWriter>::New();
  this->amrDataWriter->SetInput(this->fluidSimulator->getAMRDataSet());
  this->amrDataWriter->SetDataModeToBinary();
  std::string filename("amr_dataset.");
  filename += this->amrDataWriter->GetDefaultFileExtension();
  this->amrDataWriter->SetFileName(filename.c_str());
}

void msvQECGMainWindowPrivate::renderScalarDataContour()
{
  this->amrExtractLevel = vtkSmartPointer<vtkHierarchicalDataExtractLevel>::New();
  this->amrExtractLevel->SetInput(fluidSimulator->getAMRDataSet());
  this->amrExtractLevel->AddLevel(2);
  
  this->amrCellToPointFilter = vtkSmartPointer<vtkCellDataToPointData>::New();  
  this->amrCellToPointFilter->SetInputConnection(0,this->amrExtractLevel->GetOutputPort(0));
  
  this->amrContour = vtkSmartPointer<vtkContourFilter>::New();
  this->amrContour->SetInputConnection(0,this->amrCellToPointFilter->GetOutputPort(0));
  this->amrContour->SetValue(.1,.5);
  this->amrContour->SetInputArrayToProcess(
    0,0,0,vtkDataObject::FIELD_ASSOCIATION_POINTS,"pressure");
  
  this->amrContourMapper = vtkSmartPointer<vtkHierarchicalPolyDataMapper>::New();
  this->amrContourMapper->SetInputConnection(0, this->amrContour->GetOutputPort(0));
  
  this->amrContourActor = vtkSmartPointer<vtkActor>::New();
  this->amrContourActor->SetMapper(this->amrContourMapper);
  this->amrContourActor->GetProperty()->SetColor(1, 0, 0);
  
  this->threeDRenderer->AddActor(this->amrContourActor);  
}

void msvQECGMainWindowPrivate::renderSurfaceVectorField()
{
  this->arrowSource = vtkSmartPointer<vtkArrowSource>::New();
  this->arrowSource->Update();
  
  this->vectorFilter = vtkSmartPointer<vtkGlyph3D>::New();
  this->vectorFilter->SetSourceConnection(0,this->arrowSource->GetOutputPort(0));
  this->vectorFilter->SetInputConnection(fluidSimulator->getPolyDataSet()->GetProducerPort());
  this->vectorFilter->SetVectorModeToUseVector();
  this->vectorFilter->SetScaleModeToScaleByVector();
  this->vectorFilter->ScalingOn();
  this->vectorFilter->OrientOn();
  
  int n = fluidSimulator->getPolyDataSet()->GetPointData()->GetNumberOfArrays();
  for (int i = 0; i < n; i++)
    std::cout << "name of array[" << i << "]: " << fluidSimulator->getPolyDataSet()->GetPointData()->GetArrayName(i) << std::endl;
  
  this->vectorFilter->SetInputArrayToProcess(0,fluidSimulator->getPolyDataSet()->GetInformation());
  this->vectorMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->vectorMapper->SetInputConnection(0, this->vectorFilter->GetOutputPort(0));
  this->vectorActor = vtkSmartPointer<vtkActor>::New();
  this->vectorActor->SetMapper(this->vectorMapper);
}

void msvQECGMainWindowPrivate::readImmersedBoundary(QDir dir)
{
  if (dir.cd(QString("Morpho"))) {
    this->polyDataReader->SetFileName(
      dir.filePath("geometry.vtk").toLatin1().constData());    
    this->polyDataReader->Update();
  }
  else { /* TODO: Do something. */return; }
  
  this->fluidSimulator->msvInitializeAMR(
    "/home/rortiz/projects/MSVTK/Applications/MSI/input3d",4,5,this->polyDataReader->GetOutput());
  this->fluidSimulator->run();
  this->fluidSimulator->setAMRData();
  
  // Render
  double extent[6];
  
  this->amrLagrangianSurfaceMapper->GetBounds(extent);
  this->threeDRenderer->AddActor(this->amrLagrangianSurfaceActor);   
  this->threeDRenderer->AddActor(this->amrHierarchicalActor); 
  this->threeDRenderer->AddActor(this->amrOutlineCornerActor);
  this->threeDRenderer->AddActor(this->vectorActor); 
  this->threeDRenderer->ResetCamera(extent);
}


//------------------------------------------------------------------------------
msvQECGMainWindowPrivate::~msvQECGMainWindowPrivate()
{
  this->clear();
}

//------------------------------------------------------------------------------
void msvQECGMainWindowPrivate::clear()
{
  Q_Q(msvQECGMainWindow);
  this->threeDRenderer->RemoveAllViewProps();     // clean up the renderer
}

//------------------------------------------------------------------------------
void msvQECGMainWindowPrivate::setup(QMainWindow * mainWindow)
{
  this->setupUi(mainWindow);
  this->setupView();
}

//------------------------------------------------------------------------------
void msvQECGMainWindowPrivate::setupUi(QMainWindow * mainWindow)
{
  Q_Q(msvQECGMainWindow);

  this->Ui_msvQECGMainWindow::setupUi(mainWindow);

  q->setStatusBar(0);

  // Connect Menu ToolBars actions
  q->connect(this->actionOpen, SIGNAL(triggered()), q, SLOT(openData()));
  q->connect(this->actionClose, SIGNAL(triggered()), q, SLOT(closeData()));
  q->connect(this->actionExit, SIGNAL(triggered()), q, SLOT(close()));
  q->connect(this->actionAboutECGApplication, SIGNAL(triggered()), q,
             SLOT(aboutApplication()));

  // Customize QAction icons with standard pixmaps
  QIcon dirIcon = q->style()->standardIcon(QStyle::SP_DirIcon);
  QIcon informationIcon = q->style()->standardIcon(
    QStyle::SP_MessageBoxInformation);

  this->actionOpen->setIcon(dirIcon);
  this->actionAboutECGApplication->setIcon(informationIcon);
}

//------------------------------------------------------------------------------
void msvQECGMainWindowPrivate::setupView()
{
  this->threeDView->GetRenderWindow()->AddRenderer(this->threeDRenderer);

  // Marker annotation
  this->orientationMarker->SetInteractor
    (this->threeDRenderer->GetRenderWindow()->GetInteractor());
  this->orientationMarker->SetEnabled(1);
  this->orientationMarker->InteractiveOn();
}

//------------------------------------------------------------------------------
void msvQECGMainWindowPrivate::update()
{
  this->updateUi();
  this->updateView();
}

//------------------------------------------------------------------------------
void msvQECGMainWindowPrivate::updateUi()
{
}

//------------------------------------------------------------------------------
void msvQECGMainWindowPrivate::updateView()
{
  this->threeDView->GetRenderWindow()->Render();
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
// msvQECGMainWindow methods
//------------------------------------------------------------------------------
msvQECGMainWindow::msvQECGMainWindow(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new msvQECGMainWindowPrivate(*this))
{
  Q_D(msvQECGMainWindow);
  d->setup(this);
}

//------------------------------------------------------------------------------
msvQECGMainWindow::~msvQECGMainWindow()
{
}

//------------------------------------------------------------------------------
void msvQECGMainWindow::openData()
{
  Q_D(msvQECGMainWindow);

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
void msvQECGMainWindow::closeData()
{
  Q_D(msvQECGMainWindow);

  d->clear();
  d->update();
}

//------------------------------------------------------------------------------
void msvQECGMainWindow::aboutApplication()
{
  msvQECGAboutDialog about(this);
  about.exec();
}

//------------------------------------------------------------------------------
void msvQECGMainWindow::updateView()
{
  Q_D(msvQECGMainWindow);
  d->updateView();
}
