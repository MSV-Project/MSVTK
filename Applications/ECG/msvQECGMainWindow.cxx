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
#include "msvQECGAboutDialog.h"

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
class msvQECGMainWindowPrivate: public Ui_msvQECGMainWindow
{
  Q_DECLARE_PUBLIC(msvQECGMainWindow);
protected:
  msvQECGMainWindow* const q_ptr;

  void readCartoSignals(QDir signalsFilesDirectory);
  void readCartoSignal(const QFileInfo& signalFile);
  void readCartoPoints(QDir pointsFilesDirectory);

  // Scene Rendering
  vtkSmartPointer<vtkRenderer> threeDRenderer;
  vtkSmartPointer<vtkAxesActor> axes;
  vtkSmartPointer<vtkOrientationMarkerWidget> orientationMarker;

  // CartoSignals
  vtkSmartPointer<vtkCollection> cartoSignals;
  vtkSmartPointer<vtkPlotLine> signalPlot;
  vtkSmartPointer<vtkPlotLine> currentTimePlot;
  vtkSmartPointer<vtkTable> currentTimeLine;

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

//------------------------------------------------------------------------------
// msvQECGMainWindowPrivate methods

//------------------------------------------------------------------------------
msvQECGMainWindowPrivate::msvQECGMainWindowPrivate(msvQECGMainWindow& object)
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

  // CartoSignals
  this->cartoSignals = vtkSmartPointer<vtkCollection>::New();
  this->currentTimeLine = vtkSmartPointer<vtkTable>::New();
  vtkNew<vtkDoubleArray> xCoords;
  xCoords->SetName("Time (ms)");
  xCoords->InsertNextValue(-1.);
  xCoords->InsertNextValue(-1.);
  this->currentTimeLine->AddColumn(xCoords.GetPointer());

  vtkNew<vtkDoubleArray> yCoords;
  yCoords->SetName("Vertical bar");
  yCoords->InsertNextValue(0.);
  yCoords->InsertNextValue(1.);
  this->currentTimeLine->AddColumn(yCoords.GetPointer());

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

  // Set the buttons manager
  this->buttonsManager = vtkSmartPointer<msvVTKECGButtonsManager>::New();
  this->buttonsManager->SetRenderer(this->threeDRenderer);
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


  this->timePlayerWidget->play(false);            // stop the player widget
  this->threeDRenderer->RemoveAllViewProps();     // clean up the renderer
  this->cartoPointsReader->RemoveAllFileNames();  // clean up the reader
  this->cartoPointsMapper->Update();              // update the pipeline
  this->timePlayerWidget->updateFromFilter();     // update the player widget
  this->buttonsManager->Clear();                  // clean up the buttonsManager
  q->setCurrentSignal(-1);
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
  this->ecgReviewPanel->setVisible(false);
  q->setStatusBar(0);

  // Connect Menu ToolBars actions
  q->connect(this->actionOpen, SIGNAL(triggered()), q, SLOT(openData()));
  q->connect(this->actionClose, SIGNAL(triggered()), q, SLOT(closeData()));
  q->connect(this->actionExit, SIGNAL(triggered()), q, SLOT(close()));
  q->connect(this->actionAboutECGApplication, SIGNAL(triggered()), q,
             SLOT(aboutApplication()));

  // Playback Controller
  q->connect(this->timePlayerWidget, SIGNAL(currentTimeChanged(double)),
             q, SLOT(onCurrentTimeChanged(double)));

  // Customize QAction icons with standard pixmaps
  QIcon dirIcon = q->style()->standardIcon(QStyle::SP_DirIcon);
  QIcon informationIcon = q->style()->standardIcon(
    QStyle::SP_MessageBoxInformation);

  this->actionOpen->setIcon(dirIcon);
  this->actionAboutECGApplication->setIcon(informationIcon);

  // Associate the TimePlayerWidget to the sink (mapper)
  this->timePlayerWidget->setFilter(this->cartoPointsMapper);

  q->qvtkConnect(this->buttonsManager, vtkCommand::InteractionEvent,
                 q, SLOT(onPointSelected()));

  this->ecgView->chart()->SetAutoAxes(false);
  this->ecgView->chart()->GetAxis(vtkAxis::BOTTOM)->SetTitle("Time (ms)");
  this->ecgView->chart()->GetAxis(vtkAxis::LEFT)->SetTitle("Voltage (mV)");
  this->ecgView->chart()->GetAxis(vtkAxis::TOP)->SetVisible(false);
  this->ecgView->chart()->GetAxis(vtkAxis::RIGHT)->SetVisible(false);
  this->ecgView->chart()->GetAxis(vtkAxis::TOP)->SetBehavior(vtkAxis::CUSTOM);
  this->ecgView->chart()->GetAxis(vtkAxis::RIGHT)->SetBehavior(vtkAxis::CUSTOM);
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
  this->buttonsManager->UpdateButtonWidgets(static_cast<vtkPolyData*>
    (this->cartoPointsReader->GetOutput()));
  this->threeDView->GetRenderWindow()->Render();
}

//------------------------------------------------------------------------------
void msvQECGMainWindowPrivate::readCartoData(const QString& rootDirectory)
{
  Q_Q(msvQECGMainWindow);
  QDir dir(rootDirectory);

  if (dir.cd(QString("CartoSignals"))) {
    this->readCartoSignals(dir);
    q->setCurrentSignal(0);
    dir.cdUp();
  }

  if (dir.cd(QString("CartoPoints")))
    this->readCartoPoints(dir);

  // Link to the cartoPoints the buttons
  this->polyDataReader->Update();
  this->buttonsManager->SetNumberOfButtonWidgets(
    this->cartoSignals->GetNumberOfItems());
  this->buttonsManager->Init(this->polyDataReader->GetOutput());

  // Render
  double extent[6];
  this->cartoPointsMapper->GetBounds(extent);
  this->cartoPointsActor->VisibilityOn();
  this->threeDRenderer->AddActor(this->cartoPointsActor);
  this->threeDRenderer->ResetCamera(extent);
}

//------------------------------------------------------------------------------
void msvQECGMainWindowPrivate::readCartoPoints(QDir dir)
{
  // Set file series patterns recognition
  QStringList filters;
  filters << "*.vtk";
  dir.setNameFilters(filters);
  QStringList files = dir.entryList(QDir::Files,QDir::Name);

  // Resort files using the index number
  qSort(files.begin(), files.end(), msvQECGMainWindowPrivate::fileLessThan);

  // Fill the FileSerieReader
  foreach(const QString& file, files){
    this->cartoPointsReader->AddFileName(
      dir.filePath(file).toLatin1().constData());
  }

  // Create Instance of vtkDataObject for all outputs ports
  // Calls REQUEST_DATA_OBJECT && REQUEST_INFORMATION
  this->cartoPointsReader->SetOutputTimeRange(0,2500);
  this->cartoPointsReader->Update();

  // Update the Widget given the info provided
  this->timePlayerWidget->updateFromFilter();
}

//------------------------------------------------------------------------------
void msvQECGMainWindowPrivate::readCartoSignals(QDir dir)
{
  // Set file series patterns recognition
  QStringList signalFileFilters;
  signalFileFilters << "*.csv";
  dir.setNameFilters(signalFileFilters);
  QStringList signalFiles = dir.entryList(QDir::Files,QDir::Name);

  // Resort files using their index number
  qSort(signalFiles.begin(), signalFiles.end(),
        msvQECGMainWindowPrivate::fileLessThan);

  // Fill the FileSerieReader
  foreach(const QString& signalFile, signalFiles)
    {
    this->readCartoSignal(QFileInfo(dir, signalFile));
    }
}

//------------------------------------------------------------------------------
void msvQECGMainWindowPrivate::readCartoSignal(const QFileInfo& signalsFile)
{
  vtkNew<vtkDelimitedTextReader> reader;
  reader->SetDetectNumericColumns(true);
  reader->SetHaveHeaders(true);
  reader->SetFileName(signalsFile.absoluteFilePath().toLatin1().constData());

  this->cartoSignals->AddItem(reader.GetPointer());
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
    this, tr("Select root CartoData Folder"), QDir::homePath(),
    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (dir.isEmpty())
    return;

  d->clear();             // Clean Up data and scene
  d->readCartoData(dir);  // Load data
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

//------------------------------------------------------------------------------
void msvQECGMainWindow::setCurrentSignal(int pointId)
{
  Q_D(msvQECGMainWindow);
  d->ecgView->removeAllPlots();

  if (pointId < 0)
    {
    return;
    }
  // Initialize signal view
  d->signalPlot = vtkSmartPointer<vtkPlotLine>::New();
  d->signalPlot->SetWidth(1.);
  d->signalPlot->SetColor(1., 0., 0.);
  d->ecgView->addPlot(d->signalPlot);
  // Vertical bar for current time
  d->currentTimePlot = vtkSmartPointer<vtkPlotLine>::New();
  d->currentTimePlot->SetInput(d->currentTimeLine, 0, 1);
  // top right corner
  d->ecgView->addPlot(d->currentTimePlot);
  d->ecgView->chart()->SetPlotCorner(d->currentTimePlot, 2);

  vtkTableAlgorithm* cartoSignalsTable = vtkTableAlgorithm::SafeDownCast(
    d->cartoSignals->GetItemAsObject(pointId));
  cartoSignalsTable->Update();
  int xCol = 0;
  int yCol = 1;
  d->signalPlot->SetInput(cartoSignalsTable->GetOutput(), xCol, yCol);
  d->signalPlot->Update();
  d->ecgView->boundAxesToChartBounds();
  d->ecgView->setAxesToChartBounds();
  d->ecgView->chart()->GetAxis(vtkAxis::RIGHT)->SetMinimumLimit(0.);
  d->ecgView->chart()->GetAxis(vtkAxis::RIGHT)->SetMaximumLimit(1.);
  d->ecgView->chart()->GetAxis(vtkAxis::RIGHT)->SetRange(0., 1.);
  d->ecgView->chart()->GetAxis(vtkAxis::TOP)->SetMinimumLimit(0.);
  d->ecgView->chart()->GetAxis(vtkAxis::TOP)->SetMaximumLimit(2500.);
  d->ecgView->chart()->GetAxis(vtkAxis::TOP)->SetRange(0., 2500.);
  this->onCurrentTimeChanged(d->timePlayerWidget->currentTime());
}

//------------------------------------------------------------------------------
void msvQECGMainWindow::onPointSelected()
{
  Q_D(msvQECGMainWindow);
  int pointId = d->buttonsManager->GetIndexFromButtonId(
    d->buttonsManager->GetLastSelectedButton());
  this->setCurrentSignal(pointId);
}

//------------------------------------------------------------------------------
void msvQECGMainWindow::onCurrentTimeChanged(double time)
{
  Q_D(msvQECGMainWindow);
  // update signal chart
  vtkDoubleArray* xCoords = vtkDoubleArray::SafeDownCast(
    d->currentTimeLine->GetColumn(0));
  xCoords->SetValue(0, time);
  xCoords->SetValue(1, time);
  d->currentTimeLine->Modified();
  d->ecgView->update();
  // update 3D view
  this->updateView();
}
