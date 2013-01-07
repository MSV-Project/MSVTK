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
#include "msvVTKDataFileSeriesReader.h"
#include "msvVTKECGButtonsManager.h"
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
#include "vtkPlotBar.h"
#include "vtkPlotLine.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataReader.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkRenderWindow.h"
#include "vtkSmartPointer.h"
#include "vtkTable.h"

const unsigned int colorCount = 8;
const double colors[colorCount][3] = { {0.925490196,  0.17254902, 0.2},
                                       {0.070588235, 0.545098039, 0.290196078},
                                       {0.086274509, 0.364705882, 0.654901961},
                                       {0.952941176, 0.482352941, 0.176470588},
                                       {0.396078431, 0.196078431, 0.560784314},
                                       {0.631372549, 0.109803922, 0.176470588},
                                       {0.698039216, 0.235294118, 0.576470588},
                                       {0.003921568, 0.007843137, 0.007843137}};

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
  //vtkSmartPointer<vtkPlotLine> currentTimePlot;
  vtkSmartPointer<vtkTable> currentTimeLine;

  // CartoPoints Pipeline
  vtkSmartPointer<msvVTKDataFileSeriesReader> cartoPointsReader;
  vtkSmartPointer<vtkPolyDataReader>              polyDataReader;
  vtkSmartPointer<vtkPolyDataMapper>              cartoPointsMapper;
  vtkSmartPointer<vtkActor>                       cartoPointsActor;

  vtkSmartPointer<vtkDelaunay3D>                  delaunayFilter;
  vtkSmartPointer<vtkDataSetSurfaceFilter>        surfaceFilter;
  vtkSmartPointer<vtkPolyDataMapper>              cartoSurfaceMapper;
  vtkSmartPointer<vtkActor>                       cartoSurfaceActor;

  vtkSmartPointer<vtkAppendFilter>                mergedMapper;

  // buttonsManager
  vtkSmartPointer<msvVTKECGButtonsManager> buttonsManager;

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

  virtual void clear();

  virtual void readCartoData(const QString&);
  void addCartoSignal(vtkDelimitedTextReader* signal);
  void updateECGView(ctkVTKChartView* ecgView,
                     vtkTableAlgorithm* signal);
  void addSignalPlot(ctkVTKChartView* ecgView, vtkPlot* signalPlot);
  void updateECGItem(QListWidgetItem * item);

  static bool fileLessThan(const QString &, const QString &);
};

//------------------------------------------------------------------------------
// msvQECGMainWindowPrivate methods

//------------------------------------------------------------------------------
msvQECGMainWindowPrivate::msvQECGMainWindowPrivate(msvQECGMainWindow& object)
  : q_ptr(&object)
  , currentColor(0)
  , colorCount(7)
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
    vtkSmartPointer<msvVTKDataFileSeriesReader>::New();
  this->cartoPointsReader->SetReader(this->polyDataReader);

  this->delaunayFilter = vtkSmartPointer<vtkDelaunay3D>::New();
  this->delaunayFilter->SetInputConnection(
    this->cartoPointsReader->GetOutputPort());
  this->surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
  this->surfaceFilter->SetInputConnection(this->delaunayFilter->GetOutputPort());

  // Create Pipeline for the CartoPoints
  this->cartoSurfaceMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->cartoSurfaceMapper->ScalarVisibilityOff();
  this->cartoSurfaceMapper->SetInputConnection(
    this->surfaceFilter->GetOutputPort());
  this->cartoSurfaceActor = vtkSmartPointer<vtkActor>::New();
  this->cartoSurfaceActor->GetProperty()->SetOpacity(0.66);
  this->cartoSurfaceActor->GetProperty()->SetColor(226. / 255., 93. /255., 94. / 255.);
  this->cartoSurfaceActor->GetProperty()->BackfaceCullingOn();
  this->cartoSurfaceActor->SetMapper(this->cartoSurfaceMapper);

  // Create Pipeline for the CartoPoints
  this->cartoPointsMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->cartoPointsMapper->ScalarVisibilityOff();
  this->cartoPointsMapper->SetInputConnection(
    this->cartoPointsReader->GetOutputPort());
  this->cartoPointsActor = vtkSmartPointer<vtkActor>::New();
  this->cartoPointsActor->SetMapper(this->cartoPointsMapper);

  this->mergedMapper = vtkSmartPointer<vtkAppendFilter>::New();
  this->mergedMapper->MergePointsOn();
  this->mergedMapper->AddInputConnection(this->cartoPointsReader->GetOutputPort());
  this->mergedMapper->AddInputConnection(this->cartoSurfaceMapper->GetInputConnection(0,0));

  // Set the buttons manager
  this->buttonsManager = vtkSmartPointer<msvVTKECGButtonsManager>::New();
  this->buttonsManager->SetColors(colors, colorCount);
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
  this->mergedMapper->Update();                   // update the pipeline
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

  this->ecgPanel->toggleViewAction()->setText("ECG panel");
  this->ecgPanel->toggleViewAction()->setShortcut(QKeySequence("Ctrl+1"));
  this->menuView->addAction(this->ecgPanel->toggleViewAction());

  this->ecgReviewPanel->toggleViewAction()->setText("ECG review panel");
  this->ecgReviewPanel->toggleViewAction()->setShortcut(QKeySequence("Ctrl+2"));
  this->menuView->addAction(this->ecgReviewPanel->toggleViewAction());

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
  this->timePlayerWidget->setFilter(this->mergedMapper);

  q->qvtkConnect(this->buttonsManager, vtkCommand::InteractionEvent,
                 q, SLOT(onPointSelected()));

  q->connect(this->ecgReviewListWidget, SIGNAL(itemSelectionChanged()),
             q, SLOT(onECGSelectionChanged()));
  q->connect(this->ecgReviewListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
             q, SLOT(onCurrentItemChanged(QListWidgetItem*,QListWidgetItem*)));

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
    dir.cdUp();
  }

  if (dir.cd(QString("CartoPoints"))) {
    this->readCartoPoints(dir);
  }
  // Link to the cartoPoints the buttons
  this->polyDataReader->Update();
  this->buttonsManager->SetNumberOfButtonWidgets(
    this->ecgReviewListWidget->count());
  this->buttonsManager->Init(this->polyDataReader->GetOutput());

  // Render
  double extent[6];
  this->cartoPointsMapper->GetBounds(extent);
  this->threeDRenderer->AddActor(this->cartoPointsActor);
  this->threeDRenderer->AddActor(this->cartoSurfaceActor);
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

  if (signalFiles.size() == 0){
    qWarning() << "No carto signals in" << dir;
  }

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

  this->addCartoSignal(reader.GetPointer());
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
void msvQECGMainWindowPrivate::addCartoSignal(vtkDelimitedTextReader* signal)
{
  Q_Q(msvQECGMainWindow);

  ctkVTKChartView* ecgView = new ctkVTKChartView;
  ecgView->setMinimumSize(QSize(100, 80));
  ecgView->setFocusPolicy(Qt::NoFocus);

  QListWidgetItem* ecgItem = new QListWidgetItem;
  ecgItem->setSizeHint(QSize(100, 80));
  this->ecgReviewListWidget->addItem(ecgItem);
  this->ecgReviewListWidget->setItemWidget(ecgItem, ecgView);
  this->updateECGView(ecgView, vtkTableAlgorithm::SafeDownCast(signal));

  ecgView->setEnabled(false);
}

//------------------------------------------------------------------------------
void msvQECGMainWindowPrivate::updateECGView(ctkVTKChartView* ecgView,
                                             vtkTableAlgorithm* signal)
{
  Q_Q(msvQECGMainWindow);

  vtkSmartPointer<vtkPlotLine> signalPlot;
  if (signal)
    {
    signal->Update();
    // Initialize signal view
    signalPlot = vtkSmartPointer<vtkPlotLine>::New();
    signalPlot->SetWidth(1.);
    const double* color = colors[this->currentColor];
    signalPlot->SetColor(color[0], color[1], color[2]);
    this->currentColor = (this->currentColor + 1) % colorCount;

    const int xCol = 0;
    const int yCol = 1;
    signalPlot->SetInput(signal->GetOutput(), xCol, yCol);
    signalPlot->Update();
    }

  // Vertical bar for current time
  vtkSmartPointer<vtkPlotLine> currentTimePlot = vtkSmartPointer<vtkPlotLine>::New();
  currentTimePlot->SetInput(this->currentTimeLine, 0, 1);

  ecgView->removeAllPlots();
  // top right corner
  ecgView->addPlot(currentTimePlot);
  ecgView->chart()->SetPlotCorner(currentTimePlot, 2);

  ecgView->chart()->SetAutoAxes(false);
  ecgView->chart()->GetAxis(vtkAxis::BOTTOM)->SetTitle("Time (ms)");
  ecgView->chart()->GetAxis(vtkAxis::LEFT)->SetTitle("Voltage (mV)");
  ecgView->chart()->GetAxis(vtkAxis::TOP)->SetVisible(false);
  ecgView->chart()->GetAxis(vtkAxis::RIGHT)->SetVisible(false);
  ecgView->chart()->GetAxis(vtkAxis::TOP)->SetBehavior(vtkAxis::CUSTOM);
  ecgView->chart()->GetAxis(vtkAxis::RIGHT)->SetBehavior(vtkAxis::CUSTOM);

  if (signal)
    {
    this->addSignalPlot(ecgView, signalPlot);
    }
  q->onCurrentTimeChanged(this->timePlayerWidget->currentTime());
}

//------------------------------------------------------------------------------
void msvQECGMainWindowPrivate::addSignalPlot(ctkVTKChartView* ecgView, vtkPlot* signalPlot)
{
  ecgView->addPlot(signalPlot);

  ecgView->boundAxesToChartBounds();
  ecgView->setAxesToChartBounds();
  ecgView->chart()->GetAxis(vtkAxis::RIGHT)->SetMinimumLimit(0.);
  ecgView->chart()->GetAxis(vtkAxis::RIGHT)->SetMaximumLimit(1.);
  ecgView->chart()->GetAxis(vtkAxis::RIGHT)->SetRange(0., 1.);
  ecgView->chart()->GetAxis(vtkAxis::TOP)->SetMinimumLimit(0.);
  ecgView->chart()->GetAxis(vtkAxis::TOP)->SetMaximumLimit(2500.);
  ecgView->chart()->GetAxis(vtkAxis::TOP)->SetRange(0., 2500.);
}

//------------------------------------------------------------------------------
void msvQECGMainWindowPrivate::updateECGItem(QListWidgetItem * ecgItem)
{
  if (!ecgItem)
    {
    return;
    }
  ctkVTKChartView* ecgView = qobject_cast<ctkVTKChartView*>(
    this->ecgReviewListWidget->itemWidget(ecgItem));

  // background color
  QColor highlight = ecgView->palette().color(QPalette::Highlight);
  QColor backgroundColor = QColor(Qt::white);
  if (ecgItem->isSelected())
    {
    backgroundColor = highlight;
    }
  QColor backgroundColor2 = QColor(Qt::white);
  if (ecgItem == this->ecgReviewListWidget->currentItem() ||
      ecgItem->isSelected())
    {
    backgroundColor2 = highlight.lighter(); //orange: QColor(255, 124, 33);
    }

  vtkRendererCollection* renderers = ecgView->GetRenderWindow()->GetRenderers();
  vtkRenderer* aren = 0;
  for (renderers->InitTraversal() ; (aren = renderers->GetNextItem()) ; )
    {
    aren->SetBackground(backgroundColor.redF(),
                        backgroundColor.greenF(),
                        backgroundColor.blueF());
    aren->SetBackground2(backgroundColor2.redF(),
                         backgroundColor2.greenF(),
                         backgroundColor2.blueF());
    aren->SetGradientBackground(true);
    }
  ecgView->GetRenderWindow()->Render();
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

  QListWidgetItem* ecgItem = d->ecgReviewListWidget->item(pointId);
  d->ecgReviewListWidget->scrollToItem(ecgItem);
  d->ecgReviewListWidget->setCurrentItem(ecgItem);
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
  // update all ECGs
  foreach(QWidget* ecgView, this->findChildren<ctkVTKChartView*>())
    {
    ecgView->update();
    }
  // update 3D view
  this->updateView();
}

//------------------------------------------------------------------------------
void msvQECGMainWindow::onECGSelectionChanged()
{
  Q_D(msvQECGMainWindow);
  d->updateECGView(d->ecgView, 0);
  for (int row = 0; row < d->ecgReviewListWidget->count(); ++row)
    {
    QListWidgetItem* ecgItem = d->ecgReviewListWidget->item(row);
    ctkVTKChartView* ecgView = qobject_cast<ctkVTKChartView*>(
      d->ecgReviewListWidget->itemWidget(ecgItem));
    if (ecgItem->isSelected())
      {
      d->addSignalPlot(d->ecgView, ecgView->chart()->GetPlot(1));
      }
    d->updateECGItem(ecgItem);
    }
}

//------------------------------------------------------------------------------
void msvQECGMainWindow::onCurrentItemChanged(QListWidgetItem * current, QListWidgetItem * previous)
{
  Q_D(msvQECGMainWindow);
  d->updateECGItem(current);
  d->updateECGItem(previous);
}
