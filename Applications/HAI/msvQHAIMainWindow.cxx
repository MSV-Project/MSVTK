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
#include <QFileDialog>

// MSV includes
#include "msvQHAIMainWindow.h"
#include "msvVTKXMLMultiblockLODReader.h"
#include "ui_msvQHAIMainWindow.h"
#include "msvQHAIAboutDialog.h"
//#include "msvVTKCompositeActor.h"
//#include "msvVTKCompositePainter.h"
#include "msvVTKLODWidget.h"
#include "msvVTKProp3DButtonRepresentation.h"

// VTK includes
#include "vtkAxesActor.h"
#include "vtkAxis.h"
#include "vtkCollection.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositePolyDataMapper2.h"
#include "vtkDefaultPainter.h"
#include "vtkDoubleArray.h"
#include "vtkInformation.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkOrientationMarkerWidget.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkSmartPointer.h"

//------------------------------------------------------------------------------
class msvQHAIMainWindowPrivate: public Ui_msvQHAIMainWindow
{
  Q_DECLARE_PUBLIC(msvQHAIMainWindow);
protected:
  msvQHAIMainWindow* const q_ptr;

  void setupView();
  void updateItem(QTreeWidgetItem* item, vtkDataObject* dataObject);

  // Scene Rendering
  vtkSmartPointer<vtkRenderer> threeDRenderer;
  vtkSmartPointer<vtkAxesActor> axes;
  vtkSmartPointer<vtkOrientationMarkerWidget> orientationMarker;

  // Pipeline
  vtkSmartPointer<msvVTKXMLMultiblockLODReader> lodReader;
  vtkSmartPointer<vtkCompositePolyDataMapper2> lodMapper;
  vtkSmartPointer<vtkActor> lodActor;

  vtkSmartPointer<msvVTKLODWidget> lodWidget;
public:
  msvQHAIMainWindowPrivate(msvQHAIMainWindow& object);
  ~msvQHAIMainWindowPrivate();

  virtual void setupUi(QMainWindow*);
  void update();
  void updateUi();

  void readCompositeFile(const QString& fileName);

  void clear();
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

  // Create the reader
  this->lodReader = vtkSmartPointer<msvVTKXMLMultiblockLODReader>::New();

  // Pipeline
  this->lodMapper = vtkSmartPointer<vtkCompositePolyDataMapper2>::New();
  this->lodMapper->SetInputConnection(
    this->lodReader->GetOutputPort());
  //vtkSmartPointer<msvVTKCompositePainter> compositePainter =
  //  vtkSmartPointer<msvVTKCompositePainter>::New();
  //vtkDefaultPainter::SafeDownCast(this->lodMapper->GetPainter())->SetCompositePainter(
  //  compositePainter);

  //this->lodActor = vtkSmartPointer<msvVTKCompositeActor>::New();
  this->lodActor = vtkSmartPointer<vtkActor>::New();
  this->lodActor->SetMapper(lodMapper.GetPointer());
  this->threeDRenderer->AddActor(lodActor);

  // Depth peeling
  //this->threeDRenderer->SetUseDepthPeeling(1);
  //this->threeDRenderer->SetMaximumNumberOfPeels(200);
  //this->threeDRenderer->SetOcclusionRatio(0.1);

  //this->lodActor->GetProperty()->SetOpacity(0.5);
  //this->lodActor->GetProperty()->SetBackfaceCulling(1);
  //this->lodActor->GetProperty()->SetFrontfaceCulling(0);
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

  this->lodReader->SetFileName("");  // clean up the reader
  this->update(); // update the pipeline
}

//------------------------------------------------------------------------------
void msvQHAIMainWindowPrivate::setupUi(QMainWindow * mainWindow)
{
  Q_Q(msvQHAIMainWindow);

  this->Ui_msvQHAIMainWindow::setupUi(mainWindow);
  this->organsTreeWidget->setColumnCount(2);

  // Connect Menu ToolBars actions
  q->connect(this->actionOpen, SIGNAL(triggered()), q, SLOT(addData()));
  q->connect(this->actionClose, SIGNAL(triggered()), q, SLOT(clearData()));
  q->connect(this->actionExit, SIGNAL(triggered()), q, SLOT(close()));
  q->connect(this->actionAboutHAIApplication, SIGNAL(triggered()),
             q, SLOT(aboutApplication()));
  q->connect(this->lodComboBox, SIGNAL(currentIndexChanged(int)),
             q, SLOT(setDefaultLOD(int)));
  q->connect(this->organsTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
             q, SLOT(updateLODFromItem(QTreeWidgetItem*)));

  // Customize QAction icons with standard pixmaps
  QIcon dirIcon = q->style()->standardIcon(QStyle::SP_DirIcon);
  QIcon informationIcon = q->style()->standardIcon(
    QStyle::SP_MessageBoxInformation);

  this->actionOpen->setIcon(dirIcon);
  this->actionAboutHAIApplication->setIcon(informationIcon);

  this->setupView();
}

//------------------------------------------------------------------------------
void msvQHAIMainWindowPrivate::setupView()
{
  this->threeDView->GetRenderWindow()->AddRenderer(this->threeDRenderer);

  // Marker annotation
  this->orientationMarker->SetInteractor(
    this->threeDRenderer->GetRenderWindow()->GetInteractor());
  this->orientationMarker->SetEnabled(1);
  this->orientationMarker->InteractiveOn();

  vtkRenderWindowInteractor* iren =
    this->threeDView->GetRenderWindow()->GetInteractor();
  lodWidget = vtkSmartPointer<msvVTKLODWidget>::New();
  lodWidget->SetInteractor(iren);
  lodWidget->SetEnabled(1);
}

//------------------------------------------------------------------------------
void msvQHAIMainWindowPrivate::update()
{
  this->threeDRenderer->ResetCamera();
  this->threeDView->GetRenderWindow()->Render();
  this->updateUi();
}

//------------------------------------------------------------------------------
void msvQHAIMainWindowPrivate::readCompositeFile(const QString& fileName)
{
  this->lodReader->SetFileName(fileName.toLatin1());
  this->update();
}

//------------------------------------------------------------------------------
void msvQHAIMainWindowPrivate::updateUi()
{
  this->organsTreeWidget->clear();
  vtkMultiBlockDataSet* dataSet = vtkMultiBlockDataSet::SafeDownCast(
    this->lodReader->GetOutput());
  if (!dataSet)
    {
    return;
    }

//  dataSet->Print(std::cout);

  QTreeWidgetItem* rootItem = new QTreeWidgetItem();
  rootItem->setText(0, QFileInfo(this->lodReader->GetFileName()).baseName());
  this->organsTreeWidget->addTopLevelItem(rootItem);
  this->organsTreeWidget->blockSignals(true);
  this->updateItem(rootItem, dataSet);
  this->organsTreeWidget->blockSignals(false);
  this->organsTreeWidget->expandItem(rootItem);
}

//------------------------------------------------------------------------------
void msvQHAIMainWindowPrivate::updateItem(QTreeWidgetItem* item, vtkDataObject* dataObject)
{
  if (!dataObject)
    {
    item->setText(1, "not loaded");
    }
  if (vtkCompositeDataSet::SafeDownCast(dataObject))
    {
    vtkCompositeDataSet* compositeDataSet =
      vtkCompositeDataSet::SafeDownCast(dataObject);
    QString type;
    if (item->parent() == 0)
      {
      type = "Piece";
      }
    else
      {
      type = "LOD";
      }
    vtkCompositeDataIterator* it = compositeDataSet->NewIterator();
    it->SetVisitOnlyLeaves(0);
    it->SetTraverseSubTree(0);
    it->SetSkipEmptyNodes(0);
    unsigned int childIndex = 0;
    int usedLODIndex = -1;
    for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextItem())
      {
      vtkDataObject* childObject = it->GetCurrentDataObject();
      QTreeWidgetItem* childItem = new QTreeWidgetItem();
      item->addChild(childItem);
      childItem->setText(0, QString("%1 #%2").arg(type).arg(childIndex++));
      this->updateItem(childItem, childObject);
      if (childObject != 0)
        {
        usedLODIndex = childIndex -1;
        }
      }
    if (childIndex == 3)
      {
      item->setData(1, Qt::DisplayRole, QVariant(usedLODIndex));
      item->setFlags(item->flags() | Qt::ItemIsEditable);
      }
    else
      {
      item->setText(1, QString("%1 %2s").arg(childIndex).arg(type));
      }
    }
  else if (vtkPolyData::SafeDownCast(dataObject))
    {
    vtkPolyData* polyData = vtkPolyData::SafeDownCast(dataObject);
    item->setText(1, QString::number(polyData->GetNumberOfPolys()));
    }

  /*--------------------------------------------------------------------------*/
  // CompositeDataIterator
  /*--------------------------------------------------------------------------
  vtkCompositeDataIterator* it = dataSet->NewIterator();
  it->InitTraversal ();
  if (!it)
    {
    std::cerr << "Error: vtkMultiBlockDataSet = null"
              << std::endl;
    return EXIT_FAILURE;
    }

  while ( it->IsDoneWithTraversal() == 0 )
    {
    std::cout << "Active Data [FlatIndex]: "
              << it->GetCurrentFlatIndex() << std::endl;
    it->GoToNextItem();
    }
  */
}

//------------------------------------------------------------------------------
// msvQHAIMainWindow methods

//------------------------------------------------------------------------------
msvQHAIMainWindow::msvQHAIMainWindow(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new msvQHAIMainWindowPrivate(*this))
{
  Q_D(msvQHAIMainWindow);
  d->setupUi(this);
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

//------------------------------------------------------------------------------
void msvQHAIMainWindow::addData()
{
  Q_D(msvQHAIMainWindow);

  QString file = QFileDialog::getOpenFileName(
    this, tr("Select Anatomy file (.xml)"));
  if (file.isEmpty())
    return;

  d->readCompositeFile(file);
}

//------------------------------------------------------------------------------
void msvQHAIMainWindow::clearData()
{
  Q_D(msvQHAIMainWindow);
  d->clear();
}

//------------------------------------------------------------------------------
void msvQHAIMainWindow::setDefaultLOD(int lod)
{
  Q_D(msvQHAIMainWindow);
  d->lodReader->SetDefaultLOD(lod);
  d->update();
}

//------------------------------------------------------------------------------
void msvQHAIMainWindow::updateLODFromItem(QTreeWidgetItem* item)
{
  Q_D(msvQHAIMainWindow);
  QTreeWidgetItem* parent = item ? item->parent() : 0;
  if (!parent)
    {
    return;
    }
  int index = parent->indexOfChild(item);
  Q_ASSERT(index >= 0);
  d->lodReader->SetPieceLOD(index, item->data(1,Qt::DisplayRole).toInt());
  d->update();
}
