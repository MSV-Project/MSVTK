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

// VTK includes
#include <vtkButtonWidget.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkCamera.h>
#include <vtkCubeSource.h>
#include <vtkIdList.h>
#include <vtkInformation.h>
#include <vtkInformationDoubleVectorKey.h>
#include <vtkInformationIdTypeKey.h>
#include <vtkInformationIntegerVectorKey.h>
#include <vtkInformationKey.h>
#include <vtkKdTreePointLocator.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkMultiPieceDataSet.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp3DButtonRepresentation.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkWidgetEvent.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>

// MSVTK includes
#include "msvVTKWidgetClusters.h"
#include "msvVTKProp3DButtonRepresentation.h"

// STD Includes
#include <map>
#include <vector>

//------------------------------------------------------------------------------
class msvVTKWidgetClusters::vtkInternal
{
public:
  vtkInternal(msvVTKWidgetClusters* external);
  ~vtkInternal();

  struct ButtonProp : vtkObjectBase
  {
    ButtonProp();

    // vtkProp for the first state of our button
    vtkSmartPointer<vtkCubeSource>     Cube;
    vtkSmartPointer<vtkPolyDataMapper> CubeMapper;
    vtkSmartPointer<vtkActor>          CubeActor;

    bool Hidden;
  };

  struct ButtonHandleReprensentation : vtkObjectBase
  {
    ButtonHandleReprensentation();

    vtkSmartPointer<vtkButtonWidget> ButtonWidget;
    vtkSmartPointer<ButtonProp>      PropButton;
    void Show();
    void Hide();
  };

  struct vtkDataSetItem
  {
    vtkSmartPointer<vtkDataObject> DataObject;

  };

  typedef std::map<vtkIdType, std::vector<vtkIdType> > IndexMapType;
  typedef std::vector<vtkSmartPointer<ButtonHandleReprensentation> >
    ButtonListType;

  typedef std::vector<IndexMapType>   VectorOfIndexMaps;
  typedef std::vector<vtkDataSetItem> VectorOfDataObjets;

  void GetDisplayCoordinates(vtkPoints* from,vtkPoints* to);
  void ClearClusterButtons();
  size_t ComputeClusters(vtkPoints* positions);
  void RefineClusters(vtkPoints*        points,
                      std::vector<int> &labels,
                      std::vector<int> &intersections);
  void FindNeighbors(vtkKdTreePointLocator *kdtree,
                     double                 radius,
                     const vtkIdType        queryPointId,
                     vtkIdList *            neighbors);
  void GetClustersPositions(vtkPoints*    widgetPositions,
                            IndexMapType& indexMap,
                            vtkPoints*    clusterPositions);
  ButtonHandleReprensentation* GetButtonHandle();
  void SetButtons(vtkPoints *points, ButtonListType& buttonList);
  vtkDataObject *GetChild(size_t index);
  void SetChild(size_t index, vtkDataObject*);
  size_t GetNumberOfChildren() {
    return Children.size();
  }
  void SetNumberOfChildren(size_t num);

  msvVTKWidgetClusters* External;

  // Maps cluster buttons with ButtonList
  VectorOfIndexMaps IndexMaps;
  // This is the main button list, it doesn't change once created.
  ButtonListType ButtonList;
  // List of buttons created by the clustering, change with each interaction.
  ButtonListType ClusterButtons;

  VectorOfDataObjets Children;
};

//------------------------------------------------------------------------------
// vtkInternal methods
//------------------------------------------------------------------------------
msvVTKWidgetClusters::vtkInternal::vtkInternal(msvVTKWidgetClusters* ext)
{
  this->External = ext;
}

//------------------------------------------------------------------------------
msvVTKWidgetClusters::vtkInternal::~vtkInternal()
{
  this->ClearClusterButtons();
  // We Have to first delete the HandleReprensentation of each vtkButtonWidget
  for(size_t i = 0, end = ButtonList.size(); i < end; ++i)
    {
    this->ButtonList[i]->Delete();
    }
  this->ButtonList.clear();
}

//------------------------------------------------------------------------------
msvVTKWidgetClusters::vtkInternal::ButtonProp::ButtonProp()
{
  this->Cube       = vtkCubeSource::New();
  this->CubeMapper = vtkPolyDataMapper::New();
  this->CubeActor  = vtkActor::New();

  this->CubeMapper->SetInputConnection(this->Cube->GetOutputPort());
  this->CubeActor->SetMapper(this->CubeMapper);

  this->Hidden = false;
}

//------------------------------------------------------------------------------
msvVTKWidgetClusters::vtkInternal::ButtonHandleReprensentation::
  ButtonHandleReprensentation()
{
  this->ButtonWidget = 0;
  this->PropButton   = new ButtonProp();
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::vtkInternal::ButtonHandleReprensentation::Show()
{
  this->ButtonWidget->GetRepresentation()->VisibilityOn();
  this->ButtonWidget->EnabledOn();
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::vtkInternal::ButtonHandleReprensentation::Hide()
{
  this->ButtonWidget->GetRepresentation()->VisibilityOff();
  this->ButtonWidget->EnabledOff();
}

//------------------------------------------------------------------------------
msvVTKWidgetClusters::vtkInternal::ButtonHandleReprensentation*
msvVTKWidgetClusters::vtkInternal::GetButtonHandle()
{
  // Instantiate the ButtonHandleRepresentation
  vtkSmartPointer<ButtonHandleReprensentation> buttonHandle =
    new ButtonHandleReprensentation();

  // Instantiate the ButtonRepresentation
  vtkNew<msvVTKProp3DButtonRepresentation> rep;
  rep->SetNumberOfStates (1);
  rep->SetButtonProp (0, buttonHandle->PropButton->CubeActor);

  buttonHandle->PropButton->CubeMapper->SetLookupTable(
    this->External->ColorLookUpTable);
  vtkNew<vtkButtonWidget> buttonWidget;
  buttonWidget->SetInteractor(
    this->External->GetRenderer()->GetRenderWindow()->GetInteractor());
  buttonWidget->SetRepresentation(rep.GetPointer());
  buttonWidget->SetEnabled(1);

  // Insert the ButtonWidget with his associated HandleReprensentations in map
  buttonHandle->ButtonWidget = buttonWidget.GetPointer();
  return buttonHandle;
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::vtkInternal::SetButtons(
  vtkPoints*                   points,
  vtkInternal::ButtonListType& buttonList)
{
  vtkIdType numberOfPoints = points->GetNumberOfPoints();
  if (numberOfPoints == 0)
    {
    return;
    }

  // Create a non mutating list of widgets and place them in space
  msvVTKWidgetClusters::vtkInternal::ButtonHandleReprensentation* buttonHandle;

  double size      = this->External->ButtonWidgetSize;
  double center[3] = {};
  for(vtkIdType i = 0; i < numberOfPoints; ++i)
    {
    buttonList.push_back(this->GetButtonHandle());
    buttonHandle = buttonList.back().GetPointer();

    points->GetPoint(i,center);
    double bounds[6] = { center[0] - size / 2,
                         center[0] + size / 2,
                         center[1] - size / 2,
                         center[1] + size / 2,
                         center[2] - size / 2,
                         center[2] + size / 2};
    buttonHandle->ButtonWidget->GetRepresentation()->PlaceWidget (bounds);
    }
}

//----------------------------------------------------------------------------
void msvVTKWidgetClusters::vtkInternal::SetNumberOfChildren(size_t num)
{
  this->Children.resize(num);
}

//------------------------------------------------------------------------------
vtkDataObject *msvVTKWidgetClusters::vtkInternal::GetChild(size_t index)
{
  if(index < this->Children.size())
    {
    return this->Children[index].DataObject;
    }
  return 0;
}

//----------------------------------------------------------------------------
void msvVTKWidgetClusters::vtkInternal::SetChild(size_t         index,
                                                 vtkDataObject* dobj)
{
  if (this->Children.size() <= index)
    {
    this->SetNumberOfChildren(index+1);
    }

  vtkDataSetItem& item = this->Children[index];
  item.DataObject = dobj;
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::vtkInternal::GetClustersPositions(
  vtkPoints *                widgetPositions,
  vtkInternal::IndexMapType &indexMap,
  vtkPoints *                clusterPositions
  )
{
  vtkNew<vtkMath> math;
  // Compute cluster centroid
  IndexMapType::const_iterator i;
  IndexMapType::const_iterator end = indexMap.end();

  clusterPositions->SetNumberOfPoints (indexMap.size());
  vtkIdType idx = 0;

  for (i = indexMap.begin(); i != end; ++i, ++idx)
    {
    double center[3]        = {0};
    size_t numberOfElements = i->second.size();
    for (size_t j = 0; j < numberOfElements; ++j)
      {
      double point[3] = {0};
      widgetPositions->GetPoint (i->second[j],point);
      math->Add(center,point,center);
      }
    math->MultiplyScalar(center,1.0/numberOfElements);
    clusterPositions->SetPoint (idx,center);
    }
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::vtkInternal::ClearClusterButtons()
{
  // We have to first delete the HandleReprensentation of each vtkButtonWidget
  for(size_t i = 0, end = this->ClusterButtons.size(); i < end; ++i)
    {
    this->ClusterButtons[i]->Delete();
    }

  this->ClusterButtons.clear();
  this->IndexMaps.clear();
}

//------------------------------------------------------------------------------
size_t msvVTKWidgetClusters::vtkInternal::ComputeClusters(
  vtkPoints* positions)
{

  vtkIdType         numberOfPoints = positions->GetNumberOfPoints();
  vtkNew<vtkPoints> displayPoints;
  displayPoints->SetNumberOfPoints(numberOfPoints);

  this->GetDisplayCoordinates(positions,displayPoints.GetPointer());

  // This vector keeps track of the buttons that are already assigned to
  // a super button
  std::vector<int> clusterLabels(numberOfPoints, -1);
  // This vector collects intersection points (points that may belong
  // to 2 or more clusters) for later refinement of clustering.
  std::vector<int> intersectionPoints(numberOfPoints, -1);

  // Create a kd tree dataset holding the position of widgets
  vtkNew<vtkKdTreePointLocator> kdTree;
  vtkNew<vtkPolyData>           poly;
  poly->SetPoints(displayPoints.GetPointer());

  kdTree->SetDataSet (poly.GetPointer());
  kdTree->BuildLocator();

  int numberOfClusters = 0;

  for (vtkIdType pointID = 0; pointID < numberOfPoints; ++pointID)
    {
    // If no neighbors already had labels, assign this point to a new cluster
    if (clusterLabels[pointID] != -1)
      {
      continue;
      }
    clusterLabels[pointID] = numberOfClusters++;

    // Find all the points around the query point
    vtkNew<vtkIdList> neighbors;
    this->FindNeighbors (
      kdTree.GetPointer(),this->External->PixelRadius,pointID,
      neighbors.GetPointer());
    vtkIdType neigbhorhoodSize = neighbors->GetNumberOfIds();

    // Label all neighbors that are unlabeled the same as the current point
    for (vtkIdType n = 0; n < neigbhorhoodSize; ++n)
      {
      vtkIdType neighborID = neighbors->GetId (n);
      if (clusterLabels[neighborID] == -1)
        {
        clusterLabels[neighborID] = clusterLabels[pointID];
        }
      else
        {
        intersectionPoints[neighborID] = clusterLabels[pointID];
        }
      } // end for
    } // end for

  if(this->External->GetUseImprovedClustering())
    {
    this->RefineClusters(
      displayPoints.GetPointer(), clusterLabels, intersectionPoints);
    }

  this->IndexMaps.push_back(IndexMapType());
  IndexMapType& indexMap = this->IndexMaps.back();
  for (vtkIdType i = 0; i < numberOfPoints; ++i)
    {
    indexMap[clusterLabels[i]].push_back (i);
    }

  return this->IndexMaps.size()-1;
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::vtkInternal::RefineClusters(
  vtkPoints *       displayPoints,
  std::vector<int> &clusterLabels,
  std::vector<int> &intersectionPoints
  )
{
  if(!displayPoints)
    {
    return;
    }

  vtkIdType    numberOfPoints = displayPoints->GetNumberOfPoints();
  IndexMapType indexMap;
  for (vtkIdType i = 0; i < numberOfPoints; ++i)
    {
    indexMap[clusterLabels[i]].push_back (i);
    }
  vtkNew<vtkMath> math;

  vtkNew<vtkPoints> clusterPoints;
  this->GetClustersPositions(displayPoints, indexMap,
    clusterPoints.GetPointer());

  for(vtkIdType i = 0; i < numberOfPoints; ++i)
    {
    if(intersectionPoints[i] > -1)
      {
      double point[2][3] = {{0}};
      double dist[2]     = {0};
      displayPoints->GetPoint(i,point[0]);
      clusterPoints->GetPoint(clusterLabels[i],point[1]);
      dist[0] = math->Distance2BetweenPoints(point[0],point[1]);
      clusterPoints->GetPoint(intersectionPoints[i],point[1]);
      dist[1] = math->Distance2BetweenPoints(point[0],point[1]);
      if(dist[1] < dist[0])
        {
        clusterLabels[i] = intersectionPoints[i];
        }
      }
    }
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::vtkInternal::FindNeighbors(
  vtkKdTreePointLocator* kdtree, double radius,
  const vtkIdType queryPointId,
  vtkIdList *neighbors)
{
  if (!kdtree || radius == 0)
    {
    return;
    }
  double queryPoint[3];
  vtkDataSet::SafeDownCast(kdtree->GetDataSet())->GetPoint( queryPointId,
    queryPoint );
  kdtree->FindPointsWithinRadius(radius, queryPoint, neighbors);
  neighbors->DeleteId(queryPointId);
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::vtkInternal::GetDisplayCoordinates(vtkPoints* from,
                                                              vtkPoints* to)
{
  if((!from || !to) || from == to)
    {
    return;
    }

  vtkIdType    sizeFrom = from->GetNumberOfPoints();
  vtkRenderer *renderer = this->External->GetRenderer();

  for(vtkIdType i = 0; i < sizeFrom; ++i)
    {
    double pointFrom[3] = {0};
    double pointTo[3]   = {0};
    from->GetPoint(i,pointFrom);

    vtkInteractorObserver::ComputeWorldToDisplay(
      renderer,
      pointFrom[0],
      pointFrom[1],
      pointFrom[2],
      pointTo);
    pointTo[2] = 0;

    to->SetPoint(i,pointTo);
    }
}

//------------------------------------------------------------------------------
// msvVTKWidgetClusters methods
//------------------------------------------------------------------------------
vtkStandardNewMacro(msvVTKWidgetClusters);
vtkInformationKeyMacro(msvVTKWidgetClusters, CLUSTER_IDX, IdType);
vtkInformationKeyRestrictedMacro(msvVTKWidgetClusters, CLUSTER_BUTTONS_RANGE,
  IntegerVector, 2);
vtkInformationKeyRestrictedMacro(msvVTKWidgetClusters, DATASET_BUTTONS_RANGE,
  IntegerVector, 2);
vtkInformationKeyRestrictedMacro(msvVTKWidgetClusters, DATA_DIMENSIONALITY,
  DoubleVector, 6);

//------------------------------------------------------------------------------
vtkCxxSetObjectMacro(msvVTKWidgetClusters,ColorLookUpTable,vtkLookupTable);

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::SetDataSet(size_t level, size_t idx,
                                      vtkPoints* points)
{
  vtkIdType clusterIdx = this->Internal->ComputeClusters(points);

  if (level >= this->GetNumberOfLevels())
    {
    this->SetNumberOfLevels(level+1);
    }
  vtkMultiPieceDataSet* levelDS = vtkMultiPieceDataSet::SafeDownCast(
    this->Internal->GetChild(level));
  if (levelDS)
    {
    vtkSmartPointer<vtkUnstructuredGrid> grid = vtkUnstructuredGrid::New();
    grid->SetPoints(points);
    levelDS->SetPiece(idx, grid);

    vtkInformation* info = levelDS->GetMetaData(idx);
    if (info)
      {
      double *bounds = points->GetBounds();
      info->Set(DATA_DIMENSIONALITY(),
        bounds[0], bounds[1], bounds[2],
        bounds[3], bounds[4], bounds[5]);
      info->Set(CLUSTER_IDX(),clusterIdx);

      int range[2] = {0};
      range[0] = this->Internal->ButtonList.size();
      range[1] = this->Internal->ButtonList.size() +
                 points->GetNumberOfPoints();

      info->Set(DATASET_BUTTONS_RANGE(), range, 2);

      }
    this->Internal->SetButtons(points,this->Internal->ButtonList);
    }
}

//----------------------------------------------------------------------------
void msvVTKWidgetClusters::SetNumberOfLevels(size_t numLevels)
{
  this->Internal->SetNumberOfChildren(numLevels);

  // Initialize each level with a vtkMultiPieceDataSet.
  // vtkMultiPieceDataSet is an overkill here, since the datasets with in a
  // level cannot be composite datasets themselves.
  // This will make is possible for the user to set information with each level
  // (in future).
  for (size_t cc=0; cc < numLevels; cc++)
    {
    if (!this->Internal->GetChild(cc))
      {
      vtkMultiPieceDataSet* mds = vtkMultiPieceDataSet::New();
      this->Internal->SetChild(cc, mds);
      mds->Delete();
      }
    }
}

//----------------------------------------------------------------------------
void msvVTKWidgetClusters::SetNumberOfDataSets(size_t level,
                                               size_t numDS)
{
  if (level >= this->GetNumberOfLevels())
    {
    this->SetNumberOfLevels(level+1);
    }
  vtkMultiPieceDataSet* levelDS = vtkMultiPieceDataSet::SafeDownCast(
    this->Internal->GetChild(level));
  if (levelDS)
    {
    levelDS->SetNumberOfPieces(numDS);
    }
}

//----------------------------------------------------------------------------
size_t msvVTKWidgetClusters::GetNumberOfDataSets(size_t level)
{
  vtkMultiPieceDataSet* levelDS = vtkMultiPieceDataSet::SafeDownCast(
    this->Internal->GetChild(level));
  if (levelDS)
    {
    return levelDS->GetNumberOfPieces();
    }
  return 0;
}

//----------------------------------------------------------------------------
size_t msvVTKWidgetClusters::GetNumberOfLevels()
{
  return this->Internal->GetNumberOfChildren();
}

//----------------------------------------------------------------------------
void msvVTKWidgetClusters::SetRenderer(vtkRenderer* renderer)
{
  if (!renderer)
    {
    return;
    }

  if (this->Renderer != renderer)
    {
    this->Renderer = renderer;
    this->Modified();
    }

  vtkSmartPointer<vtkCallbackCommand> callbackCommand =
    vtkCallbackCommand::New();
  callbackCommand->SetClientData (this);
  callbackCommand->SetCallback (msvVTKWidgetClusters::UpdateWidgets);

  this->Renderer->GetRenderWindow()->GetInteractor()->GetInteractorStyle()->
    AddObserver(vtkCommand::EndInteractionEvent, callbackCommand);
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::UpdateWidgets()
{
  if(!this->ClusteringEnabled)
    {
    return;
    }
  this->Clear();

  size_t numLevels = this->GetNumberOfLevels();
  for(size_t levelIdx = 0; levelIdx < numLevels; ++levelIdx)
    {
    vtkMultiPieceDataSet* levelDS = vtkMultiPieceDataSet::SafeDownCast(
      this->Internal->GetChild(levelIdx));
    if(levelDS)
      {
      size_t numDataSets = this->GetNumberOfDataSets(levelIdx);
      for(size_t dataSetIdx = 0; dataSetIdx < numDataSets; ++dataSetIdx)
        {
        vtkUnstructuredGrid* ds
          = vtkUnstructuredGrid::SafeDownCast(levelDS->GetPiece(dataSetIdx));
        // Recompute clusters for each dataset
        vtkPoints *points     = ds->GetPoints();
        vtkIdType  clusterIdx = this->Internal->ComputeClusters(points);

        // Get the cluster structure and assign a button to each cluster
        vtkInternal::IndexMapType& indexMap = this->Internal->IndexMaps.back();

        vtkNew<vtkPoints> clusterPositions;
        this->Internal->GetClustersPositions(points,indexMap,
          clusterPositions.GetPointer());
        vtkInformation* info = levelDS->GetMetaData(dataSetIdx);
        if(info)
          {

          int range[2] = {0};
          range[0] = this->Internal->ClusterButtons.size();
          range[1] = this->Internal->ClusterButtons.size() +
                     clusterPositions->GetNumberOfPoints();
          info->Set(CLUSTER_IDX(),clusterIdx);
          info->Set(CLUSTER_BUTTONS_RANGE(),range,2);
          }
        this->Internal->SetButtons(
          clusterPositions.GetPointer(), this->Internal->ClusterButtons);
        }
      }
    }
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::UpdateWidgets(vtkObject *   vtkNotUsed(caller),
                                         unsigned long vtkNotUsed(event),
                                         void *        clientData,
                                         void *        vtkNotUsed(calldata))
{
  msvVTKWidgetClusters* self =
    reinterpret_cast<msvVTKWidgetClusters*> (clientData);

  if (!clientData)
    {
    return;
    }

  if(self->GetClusteringEnabled())
    {
    self->UpdateWidgets();
    self->InvokeEvent (vtkCommand::UpdateDataEvent);
    }
}

//------------------------------------------------------------------------------
msvVTKWidgetClusters::msvVTKWidgetClusters()
{
  this->Internal         = new vtkInternal(this);
  this->ColorLookUpTable = vtkLookupTable::New();
  this->ColorLookUpTable->SetNumberOfColors(5);
  this->ColorLookUpTable->Build();

  this->UseImprovedClustering = false;
  this->ClusteringEnabled     = true;
  this->ButtonWidgetSize      = 3,
  this->PixelRadius           = 100;
  this->Renderer              = 0;
}

//------------------------------------------------------------------------------
msvVTKWidgetClusters::~msvVTKWidgetClusters()
{
  delete this->Internal;
  this->ColorLookUpTable->Delete();
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::Clear()
{
  this->Internal->ClearClusterButtons();
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::ShowClusterButtons()
{
  size_t end = this->Internal->ClusterButtons.size();
  for (size_t i = 0; i != end; ++i)
    {
    this->Internal->ClusterButtons[i]->Show();
    }
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::HideClusterButtons()
{
  size_t end = this->Internal->ClusterButtons.size();
  for (size_t i = 0; i != end; ++i)
    {
    this->Internal->ClusterButtons[i]->Hide();
    }
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::ShowButtons()
{
  size_t end = this->Internal->ButtonList.size();
  for (size_t i = 0; i != end; ++i)
    {
    this->Internal->ButtonList[i]->Show();
    }
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::HideButtons()
{
  size_t end =this->Internal->ButtonList.size();
  for (size_t i = 0; i != end; ++i)
    {
    this->Internal->ButtonList[i]->Hide();
    }
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::ShowClusterButtons(size_t level)
{
  vtkMultiPieceDataSet* levelDS = vtkMultiPieceDataSet::SafeDownCast(
    this->Internal->GetChild(level));

  if(levelDS)
    {
    size_t numDataSets = this->GetNumberOfDataSets(level);
    for(size_t dataSetIdx = 0; dataSetIdx < numDataSets; ++dataSetIdx)
      {
      vtkInformation* info = levelDS->GetMetaData(dataSetIdx);
      if(info)
        {
        int buttonRange[2];
        info->Get(CLUSTER_BUTTONS_RANGE(),buttonRange);
        for(int i = buttonRange[0]; i < buttonRange[1]; ++i)
          {
          this->Internal->ClusterButtons[i]->Show();
          }
        }
      }
    }
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::HideClusterButtons(size_t level)
{
  if(this->Internal->ClusterButtons.size() == 0)
    {
    return;
    }
  vtkMultiPieceDataSet* levelDS = vtkMultiPieceDataSet::SafeDownCast(
    this->Internal->GetChild(level));

  if(levelDS)
    {
    size_t numDataSets = this->GetNumberOfDataSets(level);
    for(size_t dataSetIdx = 0; dataSetIdx < numDataSets; ++dataSetIdx)
      {
      vtkInformation* info = levelDS->GetMetaData(dataSetIdx);
      if(info)
        {
        int buttonRange[2];
        info->Get(CLUSTER_BUTTONS_RANGE(),buttonRange);
        for(int i = buttonRange[0]; i < buttonRange[1]; ++i)
          {
          this->Internal->ClusterButtons[i]->Hide();
          }
        }
      }
    }
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::ShowButtons(size_t level)
{
  if(this->Internal->ClusterButtons.size() == 0)
    {
    return;
    }
  vtkMultiPieceDataSet* levelDS = vtkMultiPieceDataSet::SafeDownCast(
    this->Internal->GetChild(level));

  if(levelDS)
    {
    size_t numDataSets = this->GetNumberOfDataSets(level);
    for(size_t dataSetIdx = 0; dataSetIdx < numDataSets; ++dataSetIdx)
      {
      vtkInformation* info = levelDS->GetMetaData(dataSetIdx);
      if(info)
        {
        int buttonRange[2];
        info->Get(DATASET_BUTTONS_RANGE(),buttonRange);
        for(int i = buttonRange[0]; i < buttonRange[1]; ++i)
          {
          this->Internal->ButtonList[i]->Show();
          }
        }
      }
    }
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::HideButtons(size_t level)
{
  vtkMultiPieceDataSet* levelDS = vtkMultiPieceDataSet::SafeDownCast(
    this->Internal->GetChild(level));

  if(levelDS)
    {
    size_t numDataSets = this->GetNumberOfDataSets(level);
    for(size_t dataSetIdx = 0; dataSetIdx < numDataSets; ++dataSetIdx)
      {
      vtkInformation* info = levelDS->GetMetaData(dataSetIdx);
      if(info)
        {
        int buttonRange[2];
        info->Get(DATASET_BUTTONS_RANGE(),buttonRange);
        for(int i = buttonRange[0]; i < buttonRange[1]; ++i)
          {
          this->Internal->ButtonList[i]->Hide();
          }
        }
      }
    }
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::SetCustersButtonsVisibility(bool show)
{
  if(show)
    {
    this->HideButtons();
    this->ShowClusterButtons();
    }
  else
    {
    this->HideClusterButtons();
    this->ShowButtons();
    }
}

//------------------------------------------------------------------------------
void msvVTKWidgetClusters::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

//   os << indent << this->Renderer;
//   os << indent << "Number of ButtonWidgets: "
//      << this->Internal->ClusterIndexMap.size();
//   os << indent << "ButtonWidgetSize: " << this->ButtonWidgetSize;
//
//   os << indent << "ButtonWidgets: \n";
//   int                                                                  i=0;
//   msvVTKWidgetClusters::vtkInternal::HandleButtonWidgetsType::iterator it;
//   for (it = this->Internal->HandleButtonWidgets.begin();
//        it != this->Internal->HandleButtonWidgets.end(); ++it, ++i)
//     {
//     os << indent << "  (" << i << "): " << (*it).first << "\n";
//     }
}
