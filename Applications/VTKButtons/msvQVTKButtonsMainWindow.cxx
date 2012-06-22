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
#include <QToolTip>
#include <QVector>

// MSV includes
#include "msvQVTKButtonsMainWindow.h"
#include "msvQTimePlayerWidget.h"
#include "msvVTKButtonsManager.h"
#include "msvVTKPolyDataFileSeriesReader.h"
#include "ui_msvQVTKButtonsMainWindow.h"
#include "msvQVTKButtonsAboutDialog.h"

// VTK includes
#include "vtkAlgorithmOutput.h"
#include "vtkInformation.h"
#include "vtkActor.h"
#include "vtkAppendFilter.h"
#include "vtkAxesActor.h"
#include "vtkAxis.h"
#include "vtkDoubleArray.h"
#include "vtkNew.h"
#include "vtkOrientationMarkerWidget.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkRenderWindow.h"
#include "vtkSmartPointer.h"
#include "vtkTable.h"

//new
#include "msvToolVTKButtons.h"
#include <vtkEllipticalButtonSource.h>
#include <vtkTexturedButtonRepresentation.h>
#include <vtkDataSetReader.h>
#include <vtkButtonWidget.h>

#define VTK_CREATE(type, name) \
vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

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
class msvQVTKButtonsMainWindowPrivate: public Ui_msvQVTKButtonsMainWindow
{
  Q_DECLARE_PUBLIC(msvQVTKButtonsMainWindow);
protected:
  msvQVTKButtonsMainWindow* const q_ptr;

  void importVTKData(QString &filePath);
  void addVTKButton(QObject *parent);
  void setToolTip(msvToolVTKButtons *b);
    
  void showButtons(bool value);
  void showLabels(bool value);
    
  // Scene Rendering
  vtkSmartPointer<vtkRenderer> threeDRenderer;
  vtkSmartPointer<vtkAxesActor> axes;
  vtkSmartPointer<vtkOrientationMarkerWidget> orientationMarker;

  // CartoSignals
  //vtkSmartPointer<vtkPlotLine> currentTimePlot;
  vtkSmartPointer<vtkTable> currentTimeLine;

  // CartoPoints Pipeline
  vtkSmartPointer<vtkDataSetReader>              polyDataReader;

  vtkSmartPointer<vtkPolyDataMapper>              surfaceMapper;
  vtkSmartPointer<vtkActor>                       surfaceActor;

  unsigned int currentColor;
  const unsigned int colorCount;

  QVector<msvToolVTKButtons *> buttons;
public:
  msvQVTKButtonsMainWindowPrivate(msvQVTKButtonsMainWindow& object);
  ~msvQVTKButtonsMainWindowPrivate();

  virtual void setup(QMainWindow*);
  virtual void setupUi(QMainWindow*);
  virtual void setupView();
  virtual void update();
  virtual void updateUi();
  virtual void updateView();

  virtual void clear();

};

//------------------------------------------------------------------------------
// msvQVTKButtonsMainWindowPrivate methods

//------------------------------------------------------------------------------
msvQVTKButtonsMainWindowPrivate::msvQVTKButtonsMainWindowPrivate(msvQVTKButtonsMainWindow& object)
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
  this->polyDataReader    = vtkSmartPointer<vtkDataSetReader>::New();

  // Create Pipeline for the CartoPoints
  this->surfaceMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->surfaceMapper->ScalarVisibilityOff();
  this->surfaceActor = vtkSmartPointer<vtkActor>::New();
  this->surfaceActor->GetProperty()->SetOpacity(0.66);
  this->surfaceActor->GetProperty()->SetColor(226. / 255., 93. /255., 94. / 255.);
  this->surfaceActor->GetProperty()->BackfaceCullingOn();
  this->surfaceActor->SetMapper(this->surfaceMapper);
}

//------------------------------------------------------------------------------
msvQVTKButtonsMainWindowPrivate::~msvQVTKButtonsMainWindowPrivate()
{
  this->clear();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindowPrivate::clear()
{
  Q_Q(msvQVTKButtonsMainWindow);

  this->timePlayerWidget->play(false);            // stop the player widget
  this->threeDRenderer->RemoveAllViewProps();     // clean up the renderer
  this->timePlayerWidget->updateFromFilter();     // update the player widget
}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindowPrivate::setup(QMainWindow * mainWindow)
{
  this->setupUi(mainWindow);
  this->setupView();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindowPrivate::setupUi(QMainWindow * mainWindow)
{
  Q_Q(msvQVTKButtonsMainWindow);

  this->Ui_msvQVTKButtonsMainWindow::setupUi(mainWindow);

  this->ecgPanel->toggleViewAction()->setText("VTKButtons panel");
  this->ecgPanel->toggleViewAction()->setShortcut(QKeySequence("Ctrl+1"));
  this->menuView->addAction(this->ecgPanel->toggleViewAction());

  this->ecgReviewPanel->toggleViewAction()->setText("VTKButtons review panel");
  this->ecgReviewPanel->toggleViewAction()->setShortcut(QKeySequence("Ctrl+2"));
  this->menuView->addAction(this->ecgReviewPanel->toggleViewAction());

  q->setStatusBar(0);

  // Connect Menu ToolBars actions
  q->connect(this->actionOpen, SIGNAL(triggered()), q, SLOT(openData()));
  q->connect(this->actionClose, SIGNAL(triggered()), q, SLOT(closeData()));
  q->connect(this->actionExit, SIGNAL(triggered()), q, SLOT(close()));
  q->connect(this->actionAboutVTKButtonsApplication, SIGNAL(triggered()), q,
             SLOT(aboutApplication()));

  // Playback Controller
  q->connect(this->timePlayerWidget, SIGNAL(currentTimeChanged(double)),
             q, SLOT(onCurrentTimeChanged(double)));

  // Customize QAction icons with standard pixmaps
  QIcon dirIcon = q->style()->standardIcon(QStyle::SP_DirIcon);
  QIcon informationIcon = q->style()->standardIcon(
    QStyle::SP_MessageBoxInformation);

  this->actionOpen->setIcon(dirIcon);
  this->actionAboutVTKButtonsApplication->setIcon(informationIcon);
}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindowPrivate::setupView()
{
  this->threeDView->GetRenderWindow()->AddRenderer(this->threeDRenderer);

  // Marker annotation
  this->orientationMarker->SetInteractor
    (this->threeDRenderer->GetRenderWindow()->GetInteractor());
  this->orientationMarker->SetEnabled(1);
  this->orientationMarker->InteractiveOn();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindowPrivate::update()
{
  this->updateUi();
  this->updateView();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindowPrivate::updateUi()
{
}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindowPrivate::updateView()
{
  this->threeDView->GetRenderWindow()->Render();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindowPrivate::importVTKData(QString &filePath)
{
    //import VTK Data
    polyDataReader->SetFileName(filePath.toAscii().data());
    polyDataReader->Update();
    
    int type = polyDataReader->ReadOutputType();
    
    if(type == 0) {
        // render data into the scene
        this->surfaceMapper->SetInputConnection(polyDataReader->GetOutputPort());
        this->threeDRenderer->AddActor(this->surfaceActor);
        this->threeDRenderer->ResetCamera();
    } else {
        qWarning() << "Current Data is not a polydata";
    }
}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindowPrivate::setToolTip(msvToolVTKButtons *b)
{
    double *bounds = polyDataReader->GetOutput()->GetBounds();
    QString text("<b>Data type</b>: ");
    text.append("vtkPolyData");
    text.append("<br>");
    
    QString matrixString("1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1"); //identity matrix
    QStringList list = matrixString.split(" ");
    int numElement = list.count();
    int i = 0;
    
    text.append("<b>Pose Matrix</b>:");
    text.append("<table border=\"0.2\">");
    for ( ; i < numElement; i++ ) {
        text.append("<tr>");
        text.append("<td>" + list[i] +"</td>");
        i++;
        text.append("<td>" + list[i] +"</td>");
        i++;
        text.append("<td>" + list[i] +"</td>");
        i++;
        text.append("<td>" + list[i] +"</td>");
        text.append("</tr>");
    }
    text.append("</table>");
    text.append("<b>Bounds: (min - max)</b>:");
    text.append("<table border=\"0.2\">");
    text.append("<tr>");
    text.append("<td>" + QString::number(bounds[0]) +"</td>");
    text.append("<td>" + QString::number(bounds[1]) +"</td>");
    text.append("</tr>");
    text.append("<tr>");
    text.append("<td>" + QString::number(bounds[2]) +"</td>");
    text.append("<td>" + QString::number(bounds[3]) +"</td>");
    text.append("</tr>");
    text.append("<tr>");
    text.append("<td>" + QString::number(bounds[4]) +"</td>");
    text.append("<td>" + QString::number(bounds[5]) +"</td>");
    text.append("</tr>");
    text.append("</table>");

    b->setToolTip(text);
}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindowPrivate::addVTKButton(QObject *parent) {
    msvToolVTKButtons *toolButton = new msvToolVTKButtons();
    buttons.append(toolButton);
    QString name("TestData");
    QString iconFileName("/Users/dannox/Pictures/testIcon.png");
    toolButton->setIconFileName(iconFileName);
    toolButton->setLabel(name);
    toolButton->setBounds(polyDataReader->GetOutput()->GetBounds());
    setToolTip(toolButton);
    QObject::connect(toolButton, SIGNAL(showTooltip(QString)), parent, SLOT(showTooltip(QString)));
    toolButton->setCurrentRenderer(this->threeDRenderer);
}

void msvQVTKButtonsMainWindowPrivate::showButtons(bool value) {
    Q_FOREACH(msvToolVTKButtons *button, buttons) {
        button->setShowButton(value);
        button->update();
    }
}

void msvQVTKButtonsMainWindowPrivate::showLabels(bool value) {
    Q_FOREACH(msvToolVTKButtons *button, buttons) {
        button->setShowLabel(value);
        button->update();
    }
}

//------------------------------------------------------------------------------
// msvQVTKButtonsMainWindow methods

//------------------------------------------------------------------------------
msvQVTKButtonsMainWindow::msvQVTKButtonsMainWindow(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new msvQVTKButtonsMainWindowPrivate(*this))
{
  Q_D(msvQVTKButtonsMainWindow);
  d->setup(this);
}

//------------------------------------------------------------------------------
msvQVTKButtonsMainWindow::~msvQVTKButtonsMainWindow()
{
}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindow::openData()
{
  Q_D(msvQVTKButtonsMainWindow);

  QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Data"), QDir::homePath(), tr("VTK Files (*.vtk)"));
  d->clear();             // Clean Up data and scene
  d->importVTKData(fileName);  // Load data
  d->addVTKButton(this);
  d->update();            // Update the Ui and the View
}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindow::closeData()
{
  Q_D(msvQVTKButtonsMainWindow);

  d->clear();
  d->update();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindow::aboutApplication()
{
  msvQVTKButtonsAboutDialog about(this);
  about.exec();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindow::updateView()
{
  Q_D(msvQVTKButtonsMainWindow);

  d->updateView();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindow::setCurrentSignal(int pointId)
{
  Q_D(msvQVTKButtonsMainWindow);
}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindow::onPointSelected()
{
  Q_D(msvQVTKButtonsMainWindow);
}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindow::onCurrentTimeChanged(double time)
{
  this->updateView();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindow::onVTKButtonsSelectionChanged()
{

}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindow::onCurrentItemChanged(QListWidgetItem * current, QListWidgetItem * previous)
{

}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindow::on_checkBoxShowButtons_stateChanged(int state) {
    Q_D(msvQVTKButtonsMainWindow);
    
    d->showButtons(state);
}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindow::on_checkBoxShowLabels_stateChanged(int state) {
    Q_D(msvQVTKButtonsMainWindow);
    
    d->showLabels(state);
}

//------------------------------------------------------------------------------
void msvQVTKButtonsMainWindow::showTooltip(QString text) {
    //show tooltip near the current mouse position
    QToolTip::showText(QCursor::pos(), text);
}
