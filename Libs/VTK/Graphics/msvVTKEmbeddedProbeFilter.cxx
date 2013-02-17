/*==============================================================================

  Library: MSVTK

  Copyright (c) The University of Auckland.

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
#include "msvVTKEmbeddedProbeFilter.h"

#include "vtkCellData.h"
#include "vtkCell.h"
#include "vtkDoubleArray.h"
#include "vtkImageData.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include <vector>

vtkStandardNewMacro(msvVTKEmbeddedProbeFilter);

class msvVTKEmbeddedProbeFilter::vtkVectorOfArrays : 
  public std::vector<vtkDataArray*>
{
};

//----------------------------------------------------------------------------
msvVTKEmbeddedProbeFilter::msvVTKEmbeddedProbeFilter()
{
  this->SetNumberOfInputPorts(2);
  this->CellIdArrayName = 0;
  this->ParametricCoordinateArrayName = 0;
  this->CellArrays = new vtkVectorOfArrays();
  this->PointList = 0;
  this->CellList = 0;
}

//----------------------------------------------------------------------------
msvVTKEmbeddedProbeFilter::~msvVTKEmbeddedProbeFilter()
{
  delete this->CellArrays;
  delete this->PointList;
  delete this->CellList;
}

//----------------------------------------------------------------------------
void msvVTKEmbeddedProbeFilter::SetSourceConnection(
  vtkAlgorithmOutput* algOutput)
{
  this->SetInputConnection(1, algOutput);
}

//----------------------------------------------------------------------------
int msvVTKEmbeddedProbeFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *sourceInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkDataSet *source = vtkDataSet::SafeDownCast(
    sourceInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkDataSet *output = vtkDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  return this->Probe(input, source, output);
}

//----------------------------------------------------------------------------
void msvVTKEmbeddedProbeFilter::BuildFieldList(vtkDataSet* source)
{
  delete this->PointList;
  delete this->CellList;

  this->PointList = new vtkDataSetAttributes::FieldList(1);
  this->PointList->InitializeFieldList(source->GetPointData());

  this->CellList = new vtkDataSetAttributes::FieldList(1);
  this->CellList->InitializeFieldList(source->GetCellData());
}

//----------------------------------------------------------------------------
// * input -- dataset probed with
// * source -- dataset probed into
// * output - output.
void msvVTKEmbeddedProbeFilter::InitializeForProbing(vtkDataSet* input,
  vtkDataSet* output)
{
  if (!this->PointList || !this->CellList)
    {
    vtkErrorMacro("BuildFieldList() must be called before calling this method.");
    return;
    }

  vtkIdType numPts = input->GetNumberOfPoints();

  // First, copy the input to the output as a starting point
  output->CopyStructure(input);

  // Pass here so that the attributes/fields can be over-written later
  output->GetPointData()->PassData(input->GetPointData());
  output->GetCellData()->PassData(input->GetCellData());
  output->GetFieldData()->PassData(input->GetFieldData());

  // vtkPointSet-derived outputs have points coordinates probed,
  // so allocate and replace the referenced ones set by CopyStructure()
  vtkPointSet *pointSet = vtkPointSet::SafeDownCast(output);
  if (pointSet)
    {
    vtkPoints *points = vtkPoints::New();
    points->DeepCopy(pointSet->GetPoints());
    pointSet->SetPoints(points);
    points->Delete();
    }

  vtkPointData* outPD = output->GetPointData();

  // Allocate storage for output PointData
  // All source PD is sampled to output as PD. Those arrays in source CD that
  // are not present in output PD will be passed as output PD.
  outPD = output->GetPointData();
  outPD->InterpolateAllocate((*this->PointList), numPts, numPts);

  vtkCellData* tempCellData = vtkCellData::New();
  tempCellData->InterpolateAllocate( (*this->CellList), numPts, numPts);

  this->CellArrays->clear();
  int numCellArrays = tempCellData->GetNumberOfArrays();
  for (int cc=0; cc < numCellArrays; cc++)
    {
    vtkDataArray* inArray = tempCellData->GetArray(cc);
    if (inArray && inArray->GetName() && !outPD->GetArray(inArray->GetName()))
      {
      outPD->AddArray(inArray);
      this->CellArrays->push_back(inArray);
      }
    }
  tempCellData->Delete();
}

//----------------------------------------------------------------------------
int msvVTKEmbeddedProbeFilter::Probe(vtkDataSet *input, vtkDataSet *source,
  vtkDataSet *output)
{
  if ((!input) || (!source) || (!output))
    {
    return 0;
    }
  this->BuildFieldList(source);
  this->InitializeForProbing(input, output);
  return this->PerformProbe(input, 0, source, output);
}

//----------------------------------------------------------------------------
int msvVTKEmbeddedProbeFilter::PerformProbe(vtkDataSet *input, 
  int srcIdx, vtkDataSet *source, vtkDataSet *output)
{
  vtkDebugMacro(<<"Probing data");

  int numPts = input->GetNumberOfPoints();
  vtkPointData *inputPD = input->GetPointData();
  vtkPointData *sourcePD = source->GetPointData();
  vtkCellData *sourceCD = source->GetCellData();
  vtkPointData *outputPD = output->GetPointData();

  vtkIdTypeArray *cellIdArray = 0;
  int cellIdArrayNumberOfTuples = 0;
  if (this->CellIdArrayName)
    {
    vtkDataArray *cellIdDataArray = inputPD->GetArray(this->CellIdArrayName);
    if (!cellIdDataArray)
      {
      vtkErrorMacro(<<"Cell Id array not found or non-numeric.");
      return 0;
      }
    const int cellIdArrayNumberOfComponents =
      cellIdDataArray->GetNumberOfComponents();
    cellIdArray = vtkIdTypeArray::SafeDownCast(cellIdDataArray);
    if ((cellIdArrayNumberOfComponents != 1) || (!cellIdArray))
      {
      vtkErrorMacro(<<"Cell Id array is not scalar vtkIdType.");
      return 0;
      }
    cellIdArrayNumberOfTuples = cellIdArray->GetNumberOfTuples();
    if (cellIdArrayNumberOfTuples < 1)
      {
      vtkErrorMacro(<<"Cell Id array must have at least 1 value.");
      return 0;
      }
    }

  if (!this->ParametricCoordinateArrayName)
    {
    vtkErrorMacro(<<"Parametric coordinate array not set.");
    return 0;
    }
  vtkDataArray *pcoordArray =
    inputPD->GetArray(this->ParametricCoordinateArrayName);
  if (!pcoordArray)
    {
    vtkErrorMacro(<<"Parametric coordinate array not found or non-numeric.");
    return 0;
    }
  const int pcoordArrayNumberOfComponents =
    pcoordArray->GetNumberOfComponents();
  if (pcoordArrayNumberOfComponents > 3)
    {
    vtkErrorMacro(<<"Parametric coordinate array has more than 3 components.");
    return 0;
    }
  const int pcoordArrayNumberOfTuples = pcoordArray->GetNumberOfTuples();
  if (pcoordArrayNumberOfTuples < numPts)
    {
    vtkErrorMacro(<<"Parametric coordinate array has fewer tuples than number"
      " of points in dataset.");
    return 0;
    }

  double *weights = new double[source->GetMaxCellSize()];
  double pcoords[3] = { 0.0, 0.0, 0.0 };
  vtkIdType cellId = 0;
  vtkCell *cell = 0;
  // vtkPointSet-derived outputs have point coordinates probed
  vtkPointSet *pointSet = vtkPointSet::SafeDownCast(output);
  int subId = 0;
  double coords[3];
  vtkPoints *points = 0;
  if (pointSet)
    {
    points = pointSet->GetPoints();
    }
  int result = 1;
  for (vtkIdType ptId = 0; ptId < numPts; ++ptId)
    {
    if (ptId < cellIdArrayNumberOfTuples)
      {
      cellId = cellIdArray->GetValue(ptId);
      }
    if (cellId >= 0)
      {
      cell = source->GetCell(cellId);
      }
    else
      {
      cell = 0;
      }
    if (!cell)
      {
      vtkErrorMacro(<<"No cell found with ID "<<cellId);
      result = 0;
      break;
      }
    pcoordArray->GetTuple(ptId, pcoords);

    if (points)
      {
      cell->EvaluateLocation(subId, pcoords, coords, weights);
      points->SetPoint(ptId, coords);
      }
    else
      {
      cell->InterpolateFunctions(pcoords, weights);
      }

    // Interpolate the point data
    outputPD->InterpolatePoint((*this->PointList), sourcePD, srcIdx, ptId,
      cell->PointIds, weights);
    vtkVectorOfArrays::iterator iter;
    for (iter = this->CellArrays->begin(); iter != this->CellArrays->end();
      ++iter)
      {
      vtkDataArray* inArray = sourceCD->GetArray((*iter)->GetName());
      if (inArray)
        {
        outputPD->CopyTuple(inArray, *iter, cellId, ptId);
        }
      }
    }
  delete[] weights;

  return result;
}

//----------------------------------------------------------------------------
int msvVTKEmbeddedProbeFilter::RequestInformation(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *sourceInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  outInfo->CopyEntry(sourceInfo, 
                     vtkStreamingDemandDrivenPipeline::TIME_STEPS());
  outInfo->CopyEntry(sourceInfo, 
                     vtkStreamingDemandDrivenPipeline::TIME_RANGE());

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
               inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()),
               6);
  outInfo->Set(vtkStreamingDemandDrivenPipeline::MAXIMUM_NUMBER_OF_PIECES(),
               inInfo->Get(
                 vtkStreamingDemandDrivenPipeline::MAXIMUM_NUMBER_OF_PIECES()));

  return 1;
}

//----------------------------------------------------------------------------
int msvVTKEmbeddedProbeFilter::RequestUpdateExtent(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *sourceInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  
  inInfo->Set(vtkStreamingDemandDrivenPipeline::EXACT_EXTENT(), 1);
  sourceInfo->Set(
    vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER(), 0);
  sourceInfo->Set(
    vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES(), 1);
  sourceInfo->Set(
    vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(), 0);
  inInfo->Set(
    vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),
    outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT()), 6);

  return 1;
}

//----------------------------------------------------------------------------
void msvVTKEmbeddedProbeFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  vtkDataObject *source = 0;
  if (this->GetNumberOfInputConnections(1) > 0)
    {
    source = this->GetExecutive()->GetInputData(1, 0);
    }
  os << indent << "Source: " << source << "\n";
  os << indent << "CellIdArrayName: " << (this->CellIdArrayName ?
    this->CellIdArrayName : "<none>") << "\n";
  os << indent << "ParametricCoordinateArrayName: " <<
    (this->ParametricCoordinateArrayName ?
    this->ParametricCoordinateArrayName : "<none>") << "\n";
}
