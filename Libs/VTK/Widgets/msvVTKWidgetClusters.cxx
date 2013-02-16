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
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCommand.h>
#include <vtkCamera.h>
#include <vtkCubeSource.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkIdList.h>
#include <vtkInformation.h>
#include <vtkInformationDoubleVectorKey.h>
#include <vtkInformationIdTypeKey.h>
#include <vtkInformationIntegerVectorKey.h>
#include <vtkInformationKey.h>
#include <vtkKdTreePointLocator.h>
#include <vtkLine.h>
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
#include <vtkVertex.h>
#include <vtkWidgetEvent.h>
#include <vtkSmartPointer.h>

// MSVTK includes
#include "msvVTKWidgetClusters.h"
#include "msvVTKProp3DButtonRepresentation.h"
#include "msvVTKButtonsManager.h"

// STD Includes
#include <map>
#include <vector>

// ------------------------------------------------------------------------------
class msvVTKWidgetClusters::vtkInternal
{
public:
  vtkInternal(msvVTKWidgetClusters* external);
  ~vtkInternal();

  struct ClusterProp : vtkObjectBase
  {
    ClusterProp();

    vtkSmartPointer<vtkPolyData>       Graph;
    vtkSmartPointer<vtkPolyDataMapper> GraphMapper;
    vtkSmartPointer<vtkActor>          GraphActor;

    bool Hidden;
  };

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

  typedef std::vector<vtkSmartPointer<ClusterProp> >
    VectorOfClusterRepresentations;

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
  void GetClustersButtonPositions(vtkPoints*    widgetPositions,
                                  IndexMapType& indexMap,
                                  vtkPoints*    clusterPositions);
  ButtonHandleReprensentation* GetButtonHandle();
  void SetButtons(vtkPoints *points, ButtonListType& buttonList);
  void SetButtons(vtkPoints *points, msvVTKButtonsGroup *buttonList);
  vtkDataObject *GetChild(size_t index);
  void SetChild(size_t index, vtkDataObject*);
  size_t GetNumberOfChildren();
  void SetNumberOfChildren(size_t num);
  void CreateClustersRepresentations(size_t group);

  msvVTKWidgetClusters* External;

  // Maps cluster buttons with ButtonList
  VectorOfIndexMaps IndexMaps;
  // This is the main button list, it doesn't change once created.
  ButtonListType ButtonList;
  // List of buttons created by the clustering, change with each interaction.
  ButtonListType ClusterButtons;

  VectorOfDataObjets Children;

  VectorOfClusterRepresentations ClustersRepresentations;

  vtkSmartPointer<msvVTKButtonsManager> ButtonManager;
};

// ------------------------------------------------------------------------------
// vtkInternal methods
// ------------------------------------------------------------------------------
msvVTKWidgetClusters::vtkInternal::vtkInternal(msvVTKWidgetClusters* ext)
{
  this->External      = ext;
  this->ButtonManager = msvVTKButtonsManager::New();
  // Create the group containing clusters buttons
  this->ButtonManager->CreateGroup();
}

// ------------------------------------------------------------------------------
msvVTKWidgetClusters::vtkInternal::~vtkInternal()
{
  this->ClearClusterButtons();
  // We Have to first delete the HandleReprensentation of each vtkButtonWidget
  for(size_t i = 0, end = ButtonList.size(); i < end; ++i)
    {
    this->ButtonList[i]->Delete();
    }
  this->ButtonList.clear();
  this->ClustersRepresentations.clear();
}

// ------------------------------------------------------------------------------
msvVTKWidgetClusters::vtkInternal::ButtonProp::ButtonProp()
{
  this->Cube       = vtkCubeSource::New();
  this->CubeMapper = vtkPolyDataMapper::New();
  this->CubeActor  = vtkActor::New();

  this->CubeMapper->SetInputConnection(this->Cube->GetOutputPort());
  this->CubeActor->SetMapper(this->CubeMapper);

  this->Hidden = false;
}

// ------------------------------------------------------------------------------
msvVTKWidgetClusters::vtkInternal::ClusterProp::ClusterProp()
{
  this->Graph       = vtkPolyData::New();
  this->GraphMapper = vtkPolyDataMapper::New();
  this->GraphActor  = vtkActor::New();

  this->GraphMapper->SetInput(this->Graph);
  this->GraphActor->SetMapper(this->GraphMapper);

  this->Hidden = false;
}

// ------------------------------------------------------------------------------
msvVTKWidgetClusters::vtkInternal::ButtonHandleReprensentation::
  ButtonHandleReprensentation()
{
  this->ButtonWidget = 0;
  this->PropButton   = new ButtonProp();
}

// ------------------------------------------------------------------------------
void msvVTKWidgetClusters::vtkInternal::ButtonHandleReprensentation::Show()
{
  this->ButtonWidget->GetRepresentation()->VisibilityOn();
  this->ButtonWidget->EnabledOn();
}

// ------------------------------------------------------------------------------
void msvVTKWidgetClusters::vtkInternal::ButtonHandleReprensentation::Hide()
{
  this->ButtonWidget->GetRepresentation()->VisibilityOff();
  this->ButtonWidget->EnabledOff();
}

// ------------------------------------------------------------------------------
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
    this->External->Renderer->GetRenderWindow()->GetInteractor());
  buttonWidget->SetRepresentation(rep.GetPointer());
  buttonWidget->SetEnabled(1);

  // Insert the ButtonWidget with his associated HandleReprensentations in map
  buttonHandle->ButtonWidget = buttonWidget.GetPointer();
  return buttonHandle;
}

// ------------------------------------------------------------------------------
void msvVTKWidgetClusters::vtkInternal::CreateClustersRepresentations(
  size_t group)
{
  vtkNew<vtkMath>       math;
  vtkMultiPieceDataSet* groupDS = vtkMultiPieceDataSet::SafeDownCast(this->GetChild(
      group));
  unsigned int numPieces = groupDS->GetNumberOfPieces();
  if(this->External->ClusteringWithinGroups)
    {
    vtkNew<vtkPoints> groupPoints;
    for(size_t i = 0; i < numPieces; ++i)
      {
      vtkPolyData* ds = vtkPolyData::SafeDownCast(groupDS->GetPiece(i));

      vtkPoints *points = ds->GetPoints();
      for(vtkIdType j = 0; j < points->GetNumberOfPoints(); ++j)
        {
        double point[3] = {0};
        points->GetPoint(j, point);
        groupPoints->InsertNextPoint(point);
        }
      }

    vtkIdType       clusterIndex   = 0;
    int             buttonRange[2] = {0};
    vtkInformation* info           = groupDS->GetMetaData(0u);
    if(info)
      {
      clusterIndex = info->Get(CLUSTER_IDX());
      info->Get(CLUSTER_BUTTONS_OFFSET(),buttonRange);
      }

    vtkInternal::IndexMapType &map = this->IndexMaps.at(clusterIndex);

    vtkNew<vtkPoints> clusterPoints;
    this->GetClustersButtonPositions(
      groupPoints.GetPointer(),map,clusterPoints.GetPointer());

    for(vtkIdType j = 0, end = clusterPoints->GetNumberOfPoints(); j < end; ++j)
      {
      double clusterCenter[3] = {0};
      clusterPoints->GetPoint(j,clusterCenter);

      vtkNew<vtkPoints>      graphPoints;
      vtkNew<vtkCellArray>   graphLines;
      vtkNew<vtkLookupTable> clusterColor;
      vtkNew<vtkFloatArray>  cellData;

      size_t clusterPointsSize = map[j].size();
      clusterColor->SetNumberOfTableValues(clusterPointsSize);

      double color[3] = {math->Random(0.,1.),math->Random(0.,1.),math->Random(
                           0.,           1.)};

      graphPoints->SetNumberOfPoints(clusterPointsSize+1);
      graphPoints->SetPoint(0,clusterCenter);
      for(size_t k = 0; k < clusterPointsSize; ++k)
        {
        vtkNew<vtkLine> line;
        double          clusterSatelite[3] = {0};
        groupPoints->GetPoint(map[j][k],clusterSatelite);
        graphPoints->SetPoint(k+1,clusterSatelite);
        line->GetPointIds()->SetId(0,0);
        line->GetPointIds()->SetId(0,k+1);
        graphLines->InsertNextCell(line.GetPointer());
        cellData->InsertNextValue(k + 1);
        clusterColor->SetTableValue(k,color[0],color[1],color[2]);
        }
      ClustersRepresentations.push_back(new ClusterProp());
      ClusterProp *clusterGraph = ClustersRepresentations.back();
      clusterGraph->Graph->SetPoints(graphPoints.GetPointer());
      clusterGraph->Graph->SetLines(graphLines.GetPointer());
      clusterGraph->Graph->Update();
      clusterGraph->Graph->GetCellData()->SetScalars(
        cellData.GetPointer());
      clusterGraph->GraphMapper->SetLookupTable(
        clusterColor.GetPointer());
      if(!this->External->UsePlainVTKButtons)
        {
        msvVTKButtonsGroup *buttonGroup = msvVTKButtonsGroup::SafeDownCast(ButtonManager->GetElement(
            0));
        msvVTKButtons *button           = msvVTKButtons::SafeDownCast(buttonGroup->GetElement(
            buttonRange[0]+j));
        button->SetBounds(clusterGraph->Graph->GetBounds());
        button->SetData(clusterGraph->Graph);
        button->Update();
        }
      this->External->Renderer->AddActor(clusterGraph->GraphActor);
      }
    return;
    }

  for(unsigned int i = 0; i < numPieces; ++i)
    {
    vtkPolyData* dataSet =
      vtkPolyData::SafeDownCast(groupDS->GetPiece(i));
    vtkPoints *     dataSetPoints  = dataSet->GetPoints();
    vtkIdType       clusterIndex   = 0;
    int             buttonRange[2] = {0};
    vtkInformation* info           = groupDS->GetMetaData(i);
    if(info)
      {
      clusterIndex = info->Get(CLUSTER_IDX());
      info->Get(CLUSTER_BUTTONS_OFFSET(),buttonRange);
      }

    vtkInternal::IndexMapType &map = this->IndexMaps.at(clusterIndex);

    vtkNew<vtkPoints> clusterPoints;
    this->GetClustersButtonPositions(dataSetPoints,map,
      clusterPoints.GetPointer());

    for(vtkIdType j = 0, end = clusterPoints->GetNumberOfPoints(); j < end; ++j)
      {
      double clusterCenter[3] = {0};
      clusterPoints->GetPoint(j,clusterCenter);

      vtkNew<vtkPoints>      graphPoints;
      vtkNew<vtkCellArray>   graphLines;
      vtkNew<vtkLookupTable> clusterColor;
      vtkNew<vtkFloatArray>  cellData;

      size_t clusterPointsSize = map[j].size();
      clusterColor->SetNumberOfTableValues(clusterPointsSize);

      double color[3] =
        {
        math->Random(0.,1.),
        math->Random(0.,1.),
        math->Random(0.,1.)
        };

      graphPoints->SetNumberOfPoints(clusterPointsSize+1);
      graphPoints->SetPoint(0,clusterCenter);
      ClustersRepresentations.push_back(new ClusterProp());
      ClusterProp *clusterGraph = ClustersRepresentations.back();
      for(size_t k = 0; k < clusterPointsSize; ++k)
        {
        vtkNew<vtkLine> line;
        double          clusterSatelite[3] = {0};
        dataSetPoints->GetPoint(map[j][k],clusterSatelite);
        graphPoints->SetPoint(k+1,clusterSatelite);
        line->GetPointIds()->SetId(0,0);
        line->GetPointIds()->SetId(0,k+1);
        graphLines->InsertNextCell(line.GetPointer());
        cellData->InsertNextValue(k + 1);
        clusterColor->SetTableValue(k,color[0],color[1],color[2]);
        }
      clusterGraph->Graph->SetPoints(graphPoints.GetPointer());
      clusterGraph->Graph->SetLines(graphLines.GetPointer());
      clusterGraph->Graph->Update();
      clusterGraph->Graph->GetCellData()->SetScalars(cellData.GetPointer());
      clusterGraph->GraphMapper->SetLookupTable(clusterColor.GetPointer());
      if(!this->External->UsePlainVTKButtons)
        {
        msvVTKButtonsGroup *buttonGroup = msvVTKButtonsGroup::SafeDownCast(ButtonManager->GetElement(
            0));
        msvVTKButtons *button           = msvVTKButtons::SafeDownCast(buttonGroup->GetElement(
            buttonRange[0]+j));
        button->SetBounds(clusterGraph->Graph->GetBounds());
        button->SetData(clusterGraph->Graph);
        button->Update();
        }
      this->External->Renderer->AddActor(clusterGraph->GraphActor);
      }
    }
}

// ------------------------------------------------------------------------------
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

// ------------------------------------------------------------------------------
void msvVTKWidgetClusters::vtkInternal::SetButtons(
  vtkPoints*          points,
  msvVTKButtonsGroup* buttonList)
{
  vtkIdType numberOfPoints = points->GetNumberOfPoints();
  if (numberOfPoints == 0)
    {
    return;
    }

  double size      = this->External->ButtonWidgetSize;
  double center[3] = {};
  for(vtkIdType i = 0; i < numberOfPoints; ++i)
    {
    points->GetPoint(i,center);
    double         bounds[6] = { center[0] - size / 2,
                                 center[0] + size / 2,
                                 center[1] - size / 2,
                                 center[1] + size / 2,
                                 center[2] - size / 2,
                                 center[2] + size / 2};
    msvVTKButtons *button  = buttonList->CreateButtons();
    const char     label[] = {"Zoom"};
    button->SetShowButton(true);
    button->SetOnCenter(true);
    button->SetCurrentRenderer(this->External->Renderer);
    button->SetLabelText(label);
    if(this->External->ButtonIcon)
      {
      button->SetImage(this->External->ButtonIcon);
      }
    if(this->External->UsePlainVTKButtons)
      {
      button->SetBounds(bounds);
      button->Update();
      }
    }
//   for(size_t i = 0; i < buttonList->GetNumberOfElements(); ++i)
//     {
//     msvVTKButtons *button = msvVTKButtons::SafeDownCast(buttonList->GetElement(
//         i));
//     double pos[2];
//     button->GetDisplayPosition(pos);
//     std::cout << " button[" << i << "] -> " << "[" << pos[0] << "," << pos[1] <<
//       "]" << std::endl;
//     }
}

// ----------------------------------------------------------------------------
size_t msvVTKWidgetClusters::vtkInternal::GetNumberOfChildren()
{
  return Children.size();
}

// ----------------------------------------------------------------------------
void msvVTKWidgetClusters::vtkInternal::SetNumberOfChildren(size_t num)
{
  this->Children.resize(num);
}

// ------------------------------------------------------------------------------
vtkDataObject *msvVTKWidgetClusters::vtkInternal::GetChild(size_t index)
{
  if(index < this->Children.size())
    {
    return this->Children[index].DataObject;
    }
  return 0;
}

// ----------------------------------------------------------------------------
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

// ------------------------------------------------------------------------------
void msvVTKWidgetClusters::vtkInternal::GetClustersButtonPositions(
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

    if(this->External->ShiftWidgetCenterToCorner)
      {
      vtkNew<vtkPoints> points;
      for (size_t j = 0; j < numberOfElements; ++j)
        {
        double point[3] = {0};
        widgetPositions->GetPoint (i->second[j],point);
        points->InsertNextPoint(point);
        }

      double bounds[6] = {0};
      points->GetBounds(bounds);
      double ext[3] =
        {
        .5*(bounds[0]-bounds[1]),
        .5*(bounds[2]-bounds[3]),
        .5*(bounds[4]-bounds[5])
        };
      math->Add(center,ext,center);
      }
    clusterPositions->SetPoint (idx,center);
    }
}

// ------------------------------------------------------------------------------
void msvVTKWidgetClusters::vtkInternal::ClearClusterButtons()
{
  // We have to first delete the HandleReprensentation of each vtkButtonWidget
  for(size_t i = 0, end = this->ClusterButtons.size(); i < end; ++i)
    {
    this->ClusterButtons[i]->Delete();
    }
  msvVTKButtonsGroup *clusterButtons
    = msvVTKButtonsGroup::SafeDownCast(this->ButtonManager->GetElement(0));
  clusterButtons->RemoveElements();
  this->ClusterButtons.clear();
  this->IndexMaps.clear();
  for(size_t i = 0, end = this->ClustersRepresentations.size(); i < end; ++i)
    {
    this->External->Renderer->RemoveActor(ClustersRepresentations[i]->GraphActor);
    }
  this->ClustersRepresentations.clear();
}

// ------------------------------------------------------------------------------
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

// ------------------------------------------------------------------------------
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
  this->GetClustersButtonPositions(displayPoints, indexMap,
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

// ------------------------------------------------------------------------------
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

// ------------------------------------------------------------------------------
void msvVTKWidgetClusters::vtkInternal::GetDisplayCoordinates(vtkPoints* from,
                                                              vtkPoints* to)
{
  if((!from || !to) || from == to)
    {
    return;
    }

  vtkIdType    sizeFrom = from->GetNumberOfPoints();
  vtkRenderer *renderer = this->External->Renderer;

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

// ------------------------------------------------------------------------------
// msvVTKWidgetClusters methods
// ------------------------------------------------------------------------------
vtkStandardNewMacro(msvVTKWidgetClusters);
vtkInformationKeyMacro(msvVTKWidgetClusters, CLUSTER_IDX, IdType);
vtkInformationKeyMacro(msvVTKWidgetClusters, DATASET_BUTTONS_OFFSET, IdType);
vtkInformationKeyRestrictedMacro(msvVTKWidgetClusters, CLUSTER_BUTTONS_OFFSET,
  IntegerVector, 2);

// ------------------------------------------------------------------------------
msvVTKWidgetClusters::msvVTKWidgetClusters()
{
  this->Internal         = new vtkInternal(this);
  this->ColorLookUpTable = vtkLookupTable::New();
  this->ColorLookUpTable->SetNumberOfColors(5);
  this->ColorLookUpTable->Build();

  this->UseImprovedClustering     = true;
  this->Clustering                = true;
  this->ButtonWidgetSize          = 3,
  this->PixelRadius               = 100;
  this->Renderer                  = 0;
  this->ShiftWidgetCenterToCorner = false;
  this->ClusteringWithinGroups    = false;
  this->UsePlainVTKButtons        = true;
}

// ------------------------------------------------------------------------------
msvVTKWidgetClusters::~msvVTKWidgetClusters()
{
  delete this->Internal;
  this->ColorLookUpTable->Delete();
}

// ------------------------------------------------------------------------------
vtkCxxSetObjectMacro(msvVTKWidgetClusters,ColorLookUpTable,vtkLookupTable);

// ------------------------------------------------------------------------------
vtkCxxSetObjectMacro(msvVTKWidgetClusters,ButtonIcon,vtkImageData);

// ------------------------------------------------------------------------------
void msvVTKWidgetClusters::SetDataSet(size_t group, size_t idx,
                                      vtkPoints* points)
{
  if (group >= this->GetNumberOfGroups())
    {
    this->SetNumberOfLevels(group+1);
    }
  vtkMultiPieceDataSet* groupDS = vtkMultiPieceDataSet::SafeDownCast(
    this->Internal->GetChild(group));
  if (groupDS)
    {
    vtkNew<vtkPolyData> polyData;
    polyData->SetPoints(points);
    groupDS->SetPiece(idx, polyData.GetPointer());

    vtkInformation* info = groupDS->GetMetaData(idx);
    if (info)
      {
      vtkIdType clusterIdx = 0;
      info->Set(CLUSTER_IDX(),clusterIdx);
      vtkIdType offset = this->Internal->ButtonList.size();
      info->Set(DATASET_BUTTONS_OFFSET(), offset);
      }
//     if(this->UsePlainVTKButtons)
//       {
//       this->Internal->SetButtons(points, this->Internal->ButtonList);
//       }
//     else
//       {
//       msvVTKButtonsGroup *buttonGroup =
//         this->Internal->ButtonManager->CreateGroup();
//       this->Internal->SetButtons(points,buttonGroup);
//       std::cout << "Number of buttons in group ["
//                 << group
//                 << ", idx "
//                 << idx
//                 << "] = " << buttonGroup->GetNumberOfElements() << std::endl;
//       }
    }
}

// ----------------------------------------------------------------------------
void msvVTKWidgetClusters::SetNumberOfLevels(size_t numLevels)
{
  this->Internal->SetNumberOfChildren(numLevels);

  // Initialize each group with a vtkMultiPieceDataSet.
  // vtkMultiPieceDataSet is an overkill here, since the datasets with in a
  // group cannot be composite datasets themselves.
  // This will make is possible for the user to set information with each group
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

// ----------------------------------------------------------------------------
void msvVTKWidgetClusters::SetNumberOfDataSets(size_t group,
                                               size_t numDS)
{
  if (group >= this->GetNumberOfGroups())
    {
    this->SetNumberOfLevels(group+1);
    }
  vtkMultiPieceDataSet* groupDS = vtkMultiPieceDataSet::SafeDownCast(
    this->Internal->GetChild(group));
  if (groupDS)
    {
    groupDS->SetNumberOfPieces(numDS);
    }
}

// ----------------------------------------------------------------------------
size_t msvVTKWidgetClusters::GetNumberOfDataSets(size_t group)
{
  vtkMultiPieceDataSet* groupDS = vtkMultiPieceDataSet::SafeDownCast(
    this->Internal->GetChild(group));
  if (groupDS)
    {
    return groupDS->GetNumberOfPieces();
    }
  return 0;
}

// ----------------------------------------------------------------------------
size_t msvVTKWidgetClusters::GetNumberOfGroups()
{
  return this->Internal->GetNumberOfChildren();
}

// ----------------------------------------------------------------------------
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

// ------------------------------------------------------------------------------
void msvVTKWidgetClusters::UpdateWidgets()
{
  this->Clear();
  if(!this->Clustering)
    {
    return;
    }

  size_t numGroups = this->GetNumberOfGroups();
  for(size_t groupIdx = 0; groupIdx < numGroups; ++groupIdx)
    {
    vtkMultiPieceDataSet* groupDS = vtkMultiPieceDataSet::SafeDownCast(
      this->Internal->GetChild(groupIdx));
    size_t numDataSets = this->GetNumberOfDataSets(groupIdx);
    if(groupDS)
      {

      if(this->ClusteringWithinGroups)
        {
        vtkNew<vtkPoints> groupPoints;
        for(size_t dataSetIdx = 0; dataSetIdx < numDataSets; ++dataSetIdx)
          {

          vtkPolyData* ds
            = vtkPolyData::SafeDownCast(groupDS->GetPiece(dataSetIdx));

          vtkPoints *points = ds->GetPoints();
          for(vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i)
            {
            double point[3] = {0};
            points->GetPoint(i,  point);
            groupPoints->InsertNextPoint(point);
            }
          }
        size_t clusterIdx = this->Internal->ComputeClusters(
          groupPoints.GetPointer());

        // Get the cluster structure and assign a button to each cluster
        vtkInternal::IndexMapType& indexMap =
          this->Internal->IndexMaps[clusterIdx];

        vtkNew<vtkPoints> clusterPositions;

        this->Internal->GetClustersButtonPositions(
          groupPoints.GetPointer(),indexMap,
          clusterPositions.GetPointer());

        vtkInformation* info = groupDS->GetMetaData(0u);

        if(this->UsePlainVTKButtons)
          {
          if(info)
            {
            int range[2] = {0};
            range[0] = this->Internal->ClusterButtons.size();
            range[1] = clusterPositions->GetNumberOfPoints();
            info->Set(CLUSTER_IDX(),clusterIdx);
            info->Set(CLUSTER_BUTTONS_OFFSET(),range, 2);
            }
          this->Internal->SetButtons(clusterPositions.GetPointer(),
            this->Internal->ClusterButtons);
          }
        else
          {
          msvVTKButtonsGroup *buttonGroup
            = msvVTKButtonsGroup::SafeDownCast(
            this->Internal->ButtonManager->GetElement(0));
          if(info)
            {
            int range[2] = {0};
            range[0] = buttonGroup->GetNumberOfElements();
            range[1] = clusterPositions->GetNumberOfPoints();
            info->Set(CLUSTER_IDX(),clusterIdx);
            info->Set(CLUSTER_BUTTONS_OFFSET(),range,2);
            }
          this->Internal->SetButtons(clusterPositions.GetPointer(),buttonGroup);
          }
        if(this->CreateClustersRepresentations)
          {
          SetClustersRepresentations(groupIdx);
          }
        continue;
        }
      for(size_t dataSetIdx = 0; dataSetIdx < numDataSets; ++dataSetIdx)
        {

        vtkPolyData* ds
          = vtkPolyData::SafeDownCast(groupDS->GetPiece(dataSetIdx));
        // Recompute clusters for each dataset
        vtkPoints *points     = ds->GetPoints();
        size_t     clusterIdx = this->Internal->ComputeClusters(points);

        // Get the cluster structure and assign a button to each cluster
        vtkInternal::IndexMapType& indexMap =
          this->Internal->IndexMaps[clusterIdx];

        vtkNew<vtkPoints> clusterPositions;
        this->Internal->GetClustersButtonPositions(points,indexMap,
          clusterPositions.GetPointer());
        vtkInformation* info = groupDS->GetMetaData(dataSetIdx);

        if(this->UsePlainVTKButtons)
          {
          if(info)
            {
            int range[2] = {0};
            range[0] = this->Internal->ClusterButtons.size();
            range[1] = clusterPositions->GetNumberOfPoints();
            info->Set(CLUSTER_IDX(),clusterIdx);
            info->Set(CLUSTER_BUTTONS_OFFSET(),range,2);
            }
          this->Internal->SetButtons(
            clusterPositions.GetPointer(), this->Internal->ClusterButtons);
          }
        else
          {
          msvVTKButtonsGroup *buttonGroup
            = msvVTKButtonsGroup::SafeDownCast(
            this->Internal->ButtonManager->GetElement(0));
          if(info)
            {
            int range[2] = {0};
            range[0] = buttonGroup->GetNumberOfElements();
            range[1] = clusterPositions->GetNumberOfPoints();
            info->Set(CLUSTER_IDX(),clusterIdx);
            info->Set(CLUSTER_BUTTONS_OFFSET(),range,2);
            }
          this->Internal->SetButtons(clusterPositions.GetPointer(),
            buttonGroup);
          }
        }
      }
    if(this->CreateClustersRepresentations)
      {
      SetClustersRepresentations(groupIdx);
      }
    }
}

// ------------------------------------------------------------------------------
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

  self->UpdateWidgets();
  self->InvokeEvent (vtkCommand::UpdateDataEvent);
}

// ------------------------------------------------------------------------------
void msvVTKWidgetClusters::Clear()
{
  this->Internal->ClearClusterButtons();
}

// ------------------------------------------------------------------------------
void msvVTKWidgetClusters::ShowClusterButtons()
{
  if(this->UsePlainVTKButtons)
    {
    size_t end = this->Internal->ClusterButtons.size();
    for (size_t i = 0; i != end; ++i)
      {
      this->Internal->ClusterButtons[i]->Show();
      }
    }
  else
    {
    msvVTKButtonsManager *buttonManager = this->Internal->ButtonManager;
    msvVTKButtonsGroup *  clusterGroup  = msvVTKButtonsGroup::SafeDownCast(buttonManager->GetElement(
        0));
    size_t numButtons = clusterGroup->GetNumberOfElements();
    for(size_t i = 0; i < numButtons; ++i)
      {
      msvVTKButtons *button = msvVTKButtons::SafeDownCast(clusterGroup->GetElement(
          i));
      button->SetShowButton(true);
      }
    }
}

// ------------------------------------------------------------------------------
void msvVTKWidgetClusters::HideClusterButtons()
{
  if(this->UsePlainVTKButtons)
    {
    size_t end = this->Internal->ClusterButtons.size();
    for (size_t i = 0; i != end; ++i)
      {
      this->Internal->ClusterButtons[i]->Hide();
      }
    }
  else
    {
    msvVTKButtonsManager *buttonManager = this->Internal->ButtonManager;
    msvVTKButtonsGroup *  clusterGroup  = msvVTKButtonsGroup::SafeDownCast(buttonManager->GetElement(
        0));
    size_t numButtons = clusterGroup->GetNumberOfElements();
    for(size_t i = 0; i < numButtons; ++i)
      {
      msvVTKButtons *button = msvVTKButtons::SafeDownCast(clusterGroup->GetElement(
          i));
      button->SetShowButton(false);
      }
    }
}

// ------------------------------------------------------------------------------
void msvVTKWidgetClusters::ShowButtons()
{
  if(this->UsePlainVTKButtons)
    {
    for (size_t i = 0, end = this->Internal->ButtonList.size(); i != end; ++i)
      {
      this->Internal->ButtonList[i]->Show();
      }
    }
  else
    {
    msvVTKButtonsManager *buttonManager   = this->Internal->ButtonManager;
    size_t                numButtonGroups =
      buttonManager->GetNumberOfElements();
    for(size_t i = 1; i< numButtonGroups; ++i)
      {
      msvVTKButtonsGroup *buttonGroup = msvVTKButtonsGroup::SafeDownCast(buttonManager->GetElement(
          i));
      size_t numButtons = buttonGroup->GetNumberOfElements();
      for(size_t j = 0; j < numButtons; ++j)
        {
        msvVTKButtons *button = msvVTKButtons::SafeDownCast(buttonGroup->GetElement(
            j));
        button->SetShowButton(true);
        }
      }
    }
}

// ------------------------------------------------------------------------------
void msvVTKWidgetClusters::HideButtons()
{
  if(this->UsePlainVTKButtons)
    {
    for (size_t i = 0, end = this->Internal->ButtonList.size(); i != end; ++i)
      {
      this->Internal->ButtonList[i]->Hide();
      }
    }
  else
    {
    msvVTKButtonsManager *buttonManager   = this->Internal->ButtonManager;
    size_t                numButtonGroups =
      buttonManager->GetNumberOfElements();
    for(size_t i = 1; i< numButtonGroups; ++i)
      {
      msvVTKButtonsGroup *buttonGroup = msvVTKButtonsGroup::SafeDownCast(buttonManager->GetElement(
          i));
      size_t numButtons = buttonGroup->GetNumberOfElements();
      for(size_t j = 0; j < numButtons; ++j)
        {
        msvVTKButtons *button = msvVTKButtons::SafeDownCast(buttonGroup->GetElement(
            j));
        button->SetShowButton(false);
        }
      }
    }
}

// ------------------------------------------------------------------------------
void msvVTKWidgetClusters::ShowClusterButtons(size_t group)
{
  vtkMultiPieceDataSet* groupDS = vtkMultiPieceDataSet::SafeDownCast(
    this->Internal->GetChild(group));

  if(groupDS)
    {

    if(this->GetClusteringWithinGroups())
      {
      vtkInformation* info = groupDS->GetMetaData(0u);
      if(info)
        {
        int offset[2];
        info->Get(CLUSTER_BUTTONS_OFFSET(),offset);
        for(int i = 0; i < offset[1]; ++i)
          {
          this->Internal->ClusterButtons[offset[0]+i]->Show();
          }
        }
      return;
      }
    size_t numDataSets = this->GetNumberOfDataSets(group);
    for(size_t dataSetIdx = 0; dataSetIdx < numDataSets; ++dataSetIdx)
      {
      vtkInformation* info = groupDS->GetMetaData(dataSetIdx);
      if(info)
        {
        int offset[2];
        info->Get(CLUSTER_BUTTONS_OFFSET(),offset);
        for(int i = 0; i < offset[1]; ++i)
          {
          this->Internal->ClusterButtons[offset[0]+i]->Show();
          }
        }
      }
    }
}

// ------------------------------------------------------------------------------
void msvVTKWidgetClusters::HideClusterButtons(size_t group)
{
  if(this->Internal->ClusterButtons.size() == 0)
    {
    return;
    }
  vtkMultiPieceDataSet* groupDS = vtkMultiPieceDataSet::SafeDownCast(
    this->Internal->GetChild(group));

  if(groupDS)
    {
    if(this->GetClusteringWithinGroups())
      {
      vtkInformation* info = groupDS->GetMetaData(0u);
      if(info)
        {
        int offset[2];
        info->Get(CLUSTER_BUTTONS_OFFSET(),offset);
        for(int i = 0; i < offset[1]; ++i)
          {
          this->Internal->ClusterButtons[offset[0]+i]->Hide();
          }
        }
      return;
      }
    size_t numDataSets = this->GetNumberOfDataSets(group);
    for(size_t dataSetIdx = 0; dataSetIdx < numDataSets; ++dataSetIdx)
      {
      vtkInformation* info = groupDS->GetMetaData(dataSetIdx);
      if(info)
        {
        int offset[2];
        info->Get(CLUSTER_BUTTONS_OFFSET(),offset);
        for(int i = 0; i < offset[1]; ++i)
          {
          this->Internal->ClusterButtons[offset[0]+i]->Hide();
          }
        }
      }
    }
}

// ------------------------------------------------------------------------------
void msvVTKWidgetClusters::ShowButtons(size_t group)
{
  if(this->Internal->ClusterButtons.size() == 0)
    {
    return;
    }
  vtkMultiPieceDataSet* groupDS = vtkMultiPieceDataSet::SafeDownCast(
    this->Internal->GetChild(group));

  if(groupDS)
    {
    size_t numDataSets = this->GetNumberOfDataSets(group);
    if(this->GetClusteringWithinGroups())
      {
      size_t numButtons = 0;
      for(size_t dataSetIdx = 0; dataSetIdx < numDataSets; ++dataSetIdx)
        {
        numButtons += vtkPolyData::SafeDownCast(groupDS->GetPiece(
            dataSetIdx))->GetPoints()->GetNumberOfPoints();;
        }
      vtkInformation* info = groupDS->GetMetaData(0u);
      if(info)
        {
        vtkIdType offset = info->Get(DATASET_BUTTONS_OFFSET());
        for(size_t i = offset, end = offset+numButtons; i < end; ++i)
          {
          this->Internal->ButtonList[i]->Show();
          }
        }
      return;
      }
    for(size_t dataSetIdx = 0; dataSetIdx < numDataSets; ++dataSetIdx)
      {

      size_t numButtons =
        vtkPolyData::SafeDownCast(groupDS->GetPiece(dataSetIdx))->GetPoints()->
          GetNumberOfPoints();
      vtkInformation* info = groupDS->GetMetaData(dataSetIdx);
      if(info)
        {
        vtkIdType offset = info->Get(DATASET_BUTTONS_OFFSET());
        for(size_t i = offset, end = offset+numButtons; i < end; ++i)
          {
          this->Internal->ButtonList[i]->Show();
          }
        }
      }
    }
}

// ------------------------------------------------------------------------------
void msvVTKWidgetClusters::HideButtons(size_t group)
{
  vtkMultiPieceDataSet* groupDS = vtkMultiPieceDataSet::SafeDownCast(
    this->Internal->GetChild(group));

  if(groupDS)
    {
    size_t numDataSets = this->GetNumberOfDataSets(group);
    if(this->GetClusteringWithinGroups())
      {
      size_t numButtons = 0;
      for(size_t dataSetIdx = 0; dataSetIdx < numDataSets; ++dataSetIdx)
        {
        numButtons += vtkPolyData::SafeDownCast(groupDS->GetPiece(dataSetIdx))
                        ->GetPoints()->GetNumberOfPoints();
        }
      vtkInformation* info = groupDS->GetMetaData(0u);
      if(info)
        {
        vtkIdType offset = info->Get(DATASET_BUTTONS_OFFSET());
        for(size_t i = offset, end = offset+numButtons; i < end; ++i)
          {
          this->Internal->ButtonList[i]->Hide();
          }
        }
      return;
      }
    for(size_t dataSetIdx = 0; dataSetIdx < numDataSets; ++dataSetIdx)
      {
      size_t numButtons =
        vtkPolyData::SafeDownCast(groupDS->GetPiece(dataSetIdx))->GetPoints()->
          GetNumberOfPoints();
      vtkInformation* info = groupDS->GetMetaData(dataSetIdx);
      if(info)
        {
        vtkIdType offset = info->Get(DATASET_BUTTONS_OFFSET());
        for(size_t i = offset, end = offset+numButtons; i < end; ++i)
          {
          this->Internal->ButtonList[i]->Hide();
          }
        }
      }
    }
}

// ------------------------------------------------------------------------------
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

// ------------------------------------------------------------------------------
void msvVTKWidgetClusters::SetClustersRepresentations()
{
  size_t numberOfGroups = this->GetNumberOfGroups();
  for(size_t i = 0; i < numberOfGroups; ++i)
    {
    this->Internal->CreateClustersRepresentations(i);
    }
}

// ------------------------------------------------------------------------------
void msvVTKWidgetClusters::SetClustersRepresentations(size_t group)
{
  this->Internal->CreateClustersRepresentations(group);
}



// ------------------------------------------------------------------------------
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
