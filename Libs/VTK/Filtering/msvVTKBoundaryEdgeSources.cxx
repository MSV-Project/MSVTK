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

// STD INCLUDES
#include <vector>

// VTK INCLUDES
#include <vtkCleanPolyData.h>
#include <vtkFeatureEdges.h>
#include <vtkType.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkDoubleArray.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPointData.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkSelectPolyData.h>

// MSV INCLUDES
#include "msvVTKBoundaryEdgeSources.h"

// ----------------------------------------------------------------------------
class msvVTKBoundaryEdgeSources::vtkInternal
{
public:

  std::vector<double> radii;
  void AddRadius(const double &radius) {
    radii.push_back(radius);
  }
  std::vector<double> &GetRadii() {
    return radii;
  }

};

// ----------------------------------------------------------------------------
vtkStandardNewMacro ( msvVTKBoundaryEdgeSources );

// ----------------------------------------------------------------------------
msvVTKBoundaryEdgeSources::msvVTKBoundaryEdgeSources()
{
  this->Internal = new vtkInternal;
}

// ----------------------------------------------------------------------------
msvVTKBoundaryEdgeSources::~msvVTKBoundaryEdgeSources()
{
  delete this->Internal;
}

int msvVTKBoundaryEdgeSources::RequestData(
  vtkInformation *       vtkNotUsed ( request ),
  vtkInformationVector **inputVector,
  vtkInformationVector * outputVector )
{
  // get the info objects
  vtkInformation *inInfo  = inputVector[0]->GetInformationObject ( 0 );
  vtkInformation *outInfo = outputVector->GetInformationObject ( 0 );

  // get the input and output
  vtkPolyData *input = vtkPolyData::SafeDownCast (
    inInfo->Get ( vtkDataObject::DATA_OBJECT() ) );
  vtkPolyData *output = vtkPolyData::SafeDownCast (
    outInfo->Get ( vtkDataObject::DATA_OBJECT() ) );


  vtkDebugMacro ( <<"Executing boundary edge capping filter." );

  //  Check input/allocate storage
  //
  if ( input->GetPoints() == NULL )
    {
    vtkErrorMacro ( "No points!" );
    return 1;
    }

  // Extract boundary edges
  vtkNew<vtkFeatureEdges> boundaryEdges;
  boundaryEdges->SetInput ( input );
  boundaryEdges->FeatureEdgesOff();
  boundaryEdges->NonManifoldEdgesOff();
  boundaryEdges->ColoringOff();
  boundaryEdges->Update();
  // Extract individual connected regions
  vtkNew<vtkPolyDataConnectivityFilter> boundaryEdgesConnectivity;
  boundaryEdgesConnectivity->SetInput( boundaryEdges->GetOutput() );
  boundaryEdgesConnectivity->SetExtractionModeToSpecifiedRegions();
  boundaryEdgesConnectivity->Update();

  vtkNew<vtkMath>   math;
  vtkNew<vtkPoints> points;

  // Loop over connected regions
  int numRegions = boundaryEdgesConnectivity->GetNumberOfExtractedRegions();
  this->Internal->radii.resize(numRegions,0);

  vtkNew<vtkDoubleArray> radii;
  radii->SetName("radii");
  radii->SetNumberOfValues(numRegions);
  for ( int i = 0; i < numRegions; ++i )
    {
    boundaryEdgesConnectivity->AddSpecifiedRegion(i);
    boundaryEdgesConnectivity->Update();
    // Get rid of orphan nodes
    vtkNew<vtkCleanPolyData> region;
    region->CreateDefaultLocator();
    region->SetInputConnection(boundaryEdgesConnectivity->GetOutputPort());
    region->Update();

    vtkPoints *boundaryPoints = region->GetOutput()->GetPoints();

    double bounds[6] = {0};
    boundaryPoints->GetBounds(bounds);

    double center[] =
      {
      .5*(bounds[0]+bounds[1]),
      .5*(bounds[2]+bounds[3]),
      .5*(bounds[4]+bounds[5])
      };

    //  Compute approximate radius
    double radius = 0;
    for(vtkIdType k = 0; k < boundaryPoints->GetNumberOfPoints(); ++k)
      {
      double v[3] = {0}, u[3] = {0};
      boundaryPoints->GetPoint(k,u);
      math->Subtract(u,center,v);
      double r = math->Norm(v);
      if (radius < r)
        radius = r;
      }
     radii->SetValue(i,.3*radius);

    // The source radius is 30% of entire radius
    this->Internal->radii[i] = .3*radius;
    points->InsertNextPoint(center);

    boundaryEdgesConnectivity->DeleteSpecifiedRegion(i);
    }

  vtkNew<vtkCellArray> vertices;
  for ( vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i )
    {
    vertices->InsertNextCell(1);
    vertices->InsertCellPoint(i);
    }
  output->SetPoints(points.GetPointer());
  output->SetVerts(vertices.GetPointer());
  output->GetPointData()->AddArray(radii.GetPointer());

  return 1;
}

double msvVTKBoundaryEdgeSources::GetRadius(unsigned int i)
{
  return this->Internal->radii[i];
}

std::vector<double> &msvVTKBoundaryEdgeSources::GetRadii()
{
  return this->Internal->radii;
}

void msvVTKBoundaryEdgeSources::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

}
