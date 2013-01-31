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
#include "vtkActor.h"
#include "vtkAxesActor.h"
#include "vtkAxis.h"
#include "vtkChartXY.h"
#include "vtkDataObjectToTable.h"
#include "vtkDoubleArray.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPlotLine.h"
#include "vtkPointPicker.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkTable.h"
#include "vtkTableAlgorithm.h"

const unsigned int colorCount = 8;
const double colors[colorCount][3] = { {0.925490196,  0.17254902, 0.2},
                                       {0.070588235, 0.545098039, 0.290196078},
                                       {0.086274509, 0.364705882, 0.654901961},
                                       {0.952941176, 0.482352941, 0.176470588},
                                       {0.396078431, 0.196078431, 0.560784314},
                                       {0.631372549, 0.109803922, 0.176470588},
                                       {0.698039216, 0.235294118, 0.576470588},
                                       {0.003921568, 0.007843137, 0.007843137}};


class MouseInteractorStylePP : public vtkInteractorStyleTrackballCamera
{
public:
  static MouseInteractorStylePP* New();

  vtkTypeMacro(MouseInteractorStylePP, vtkInteractorStyleTrackballCamera);

  virtual void OnLeftButtonDown()
    {
 /*   std::cout << "Picking pixel: " << this->Interactor->GetEventPosition()[0] << " " << this->Interactor->GetEventPosition()[1] << std::endl;
    this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0],
        this->Interactor->GetEventPosition()[1],
        0,  // always zero.
        this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
    double picked[3];
    this->Interactor->GetPicker()->GetPickPosition(picked);
    std::cout << "Picked value: " << picked[0] << " " << picked[1] << " " << picked[2] << std::endl;
    */
    // Forward events
    vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
    }
};

vtkStandardNewMacro(MouseInteractorStylePP);

//------------------------------------------------------------------------------
class msvQGridViewerMainWindowPrivate: public Ui_msvQGridViewerMainWindow
{
  Q_DECLARE_PUBLIC(msvQGridViewerMainWindow);
protected:
  msvQGridViewerMainWindow* const q_ptr;
  msvGridViewerPipeline gridPipeline;
  vtkSmartPointer<vtkTable> currentTimeLine;

  unsigned int currentColor;
  const unsigned int colorCount;
public:
  msvQGridViewerMainWindowPrivate(msvQGridViewerMainWindow& object);
  ~msvQGridViewerMainWindowPrivate();

  void addActorListItem(std::string name);
  void addSignal(vtkDataObjectToTable* signal);
  virtual void setup(QMainWindow*);
  virtual void setupUi(QMainWindow*);
  virtual void setupView();
  virtual void update();
  virtual void updateView();
  virtual void updateTime(double);
  void updateActorsList();
  virtual void clear();
  void updateActorVisibility(QListWidgetItem * item);
  void updateECGView(ctkVTKChartView* ecgView,
      vtkTableAlgorithm* signal);
  void addSignalPlot(ctkVTKChartView* ecgView, vtkPlot* signalPlot);

  virtual void readGridData(const QString&);

  vtkSmartPointer<vtkPointPicker> pointPicker;
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;
  vtkSmartPointer<MouseInteractorStylePP> mouseInteractorStyle;
};

//------------------------------------------------------------------------------
// msvQGridViewerMainWindowPrivate methods

//------------------------------------------------------------------------------
msvQGridViewerMainWindowPrivate::msvQGridViewerMainWindowPrivate(msvQGridViewerMainWindow& object)
  : q_ptr(&object)
  , currentColor(0)
  , colorCount(7)
{
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
}

//------------------------------------------------------------------------------
msvQGridViewerMainWindowPrivate::~msvQGridViewerMainWindowPrivate()
{
  this->clear();
}

void msvQGridViewerMainWindowPrivate::addActorListItem(const std::string name)
{
  QListWidgetItem* actorItem = new QListWidgetItem;
  actorItem->setSizeHint(QSize(100, 25));
  //actorItem->setFlags(Qt::ItemIsUserCheckable & Qt::ItemIsEnabled & Qt::ItemIsSelectable);
  actorItem->setCheckState(Qt::Checked);
  actorItem->setText(QString::fromStdString(name));
  this->actorsListWidget->addItem(actorItem);
}

void msvQGridViewerMainWindowPrivate::addSignal(vtkDataObjectToTable* signal)
{
  //ctkVTKChartView* ecgView = new ctkVTKChartView;
  ecgView->setMinimumSize(QSize(100, 80));
  ecgView->setFocusPolicy(Qt::NoFocus);

  QListWidgetItem* ecgItem = new QListWidgetItem;
  ecgItem->setSizeHint(QSize(100, 80));
  this->updateECGView(ecgView, vtkTableAlgorithm::SafeDownCast(signal));
  ecgView->GetRenderWindow()->Render();
 // ecgView->setEnabled(false);
}

//------------------------------------------------------------------------------
void msvQGridViewerMainWindowPrivate::clear()
{
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

  q->connect(this->actorsListWidget, SIGNAL(itemChanged(QListWidgetItem *)),
             q, SLOT(onActorsListItemChanged(QListWidgetItem *)));

  this->actionOpen->setIcon(dirIcon);
  this->actionAboutGridViewerApplication->setIcon(informationIcon);

  this->pointPicker = vtkSmartPointer<vtkPointPicker>::New();

  this->renderWindowInteractor =
      vtkSmartPointer<vtkRenderWindowInteractor>::New();

  this->mouseInteractorStyle =
    vtkSmartPointer<MouseInteractorStylePP>::New();
  this->renderWindowInteractor->SetInteractorStyle(this->mouseInteractorStyle);
  this->renderWindowInteractor->SetPicker(this->pointPicker);
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
  this->addSignal(this->gridPipeline.getDataTable());
}

void msvQGridViewerMainWindowPrivate::updateActorsList()
{
	vtkActorsMap *actorsMap = this->gridPipeline.getActorsMap();
	vtkActorsMap::iterator pos;
	for (pos = actorsMap->begin(); pos != actorsMap->end(); ++pos)
	  {
	  addActorListItem(pos->first);
	  }
}

//------------------------------------------------------------------------------
void msvQGridViewerMainWindowPrivate::updateECGView(ctkVTKChartView* ecgView,
                                             vtkTableAlgorithm* signal)
{
  Q_Q(msvQGridViewerMainWindow);

  vtkSmartPointer<vtkPlotLine> signalPlot;
  if (signal)
    {

    signal->Update();
    vtkTable *table = signal->GetOutput();
    int numberOfRows = table->GetNumberOfRows();
    int numberOfColumns = table->GetNumberOfColumns();
    cerr << "'" << numberOfRows << "' numebr of columns '" << numberOfColumns << "'\n";
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

void msvQGridViewerMainWindowPrivate::addSignalPlot(ctkVTKChartView* ecgView, vtkPlot* signalPlot)
{
  ecgView->addPlot(signalPlot);

  ecgView->boundAxesToChartBounds();
  ecgView->setAxesToChartBounds();
  ecgView->chart()->GetAxis(vtkAxis::RIGHT)->SetMinimumLimit(0.);
  ecgView->chart()->GetAxis(vtkAxis::RIGHT)->SetMaximumLimit(1.);
  ecgView->chart()->GetAxis(vtkAxis::RIGHT)->SetRange(0., 1.);
  ecgView->chart()->GetAxis(vtkAxis::TOP)->SetMinimumLimit(0.);
  ecgView->chart()->GetAxis(vtkAxis::TOP)->SetMaximumLimit(552.);
  ecgView->chart()->GetAxis(vtkAxis::TOP)->SetRange(0., 552.);
}

//------------------------------------------------------------------------------
void msvQGridViewerMainWindowPrivate::updateView()
{
  //this->threeDView->GetRenderWindow()->SetInteractor(this->renderWindowInteractor);
  this->threeDView->GetRenderWindow()->Render();
  //this->renderWindowInteractor->Initialize();
  //this->renderWindowInteractor->Start();
}

//------------------------------------------------------------------------------
void msvQGridViewerMainWindowPrivate::updateTime(double time)
{
  this->gridPipeline.updateTime(time);
  this->updateView();
}

void msvQGridViewerMainWindowPrivate::updateActorVisibility(QListWidgetItem * item)
{
	std::string actorsName = (item->text()).toStdString();
	vtkActorsMap *actorsMap = this->gridPipeline.getActorsMap();
	vtkActor *actor = (*actorsMap)[actorsName];
	actor->SetVisibility((item->checkState() == Qt::Checked));
}

//------------------------------------------------------------------------------
void msvQGridViewerMainWindowPrivate::readGridData(const QString& gridFileName)
{
  QFileInfo fileInfo(gridFileName);
  QDir::setCurrent(fileInfo.absolutePath());
  std::string tmp = gridFileName.toStdString();
  this->gridPipeline.readGridFile(tmp.c_str());

  vtkAlgorithm *endMapper = this->gridPipeline.getEndMapper();
  this->timePlayerWidget->setFilter(endMapper);
  this->timePlayerWidget->updateFromFilter();
  this->updateActorsList();
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
  d->updateTime(time);
}

void msvQGridViewerMainWindow::onActorsListItemChanged(QListWidgetItem * item)
{
	  Q_D(msvQGridViewerMainWindow);
	  // update 3D view
	  d->updateActorVisibility(item);

}
