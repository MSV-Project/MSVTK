/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkHierarchicalBoxDataSet.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "msvVTKPolyDataBoundaryEdgeCaps.h"
#include "vtkVector.h"
#include "vtkPointData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkPolyData.h"
#include "vtkSelectPolyData.h"
#include "vtkDelaunay2D.h"
#include "vtkObjectFactory.h"
#include "vtkFeatureEdges.h"
#include "vtkAppendPolyData.h"
#include "vtkPolyDataConnectivityFilter.h"
#include "vtkCleanPolyData.h"
#include "vtkMath.h"

#include <cmath>


vtkStandardNewMacro ( msvVTKPolyDataBoundaryEdgeCaps );


// Construct with default NumberOfSubdivisions = 3.
msvVTKPolyDataBoundaryEdgeCaps::msvVTKPolyDataBoundaryEdgeCaps()
{
  this->NumberOfSubdivisions = 3;
  this->TriangulationTolerance = 0;
}

msvVTKPolyDataBoundaryEdgeCaps::~msvVTKPolyDataBoundaryEdgeCaps()
{
}

int msvVTKPolyDataBoundaryEdgeCaps::RequestData (
  vtkInformation *vtkNotUsed ( request ),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector )
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject ( 0 );
  vtkInformation *outInfo = outputVector->GetInformationObject ( 0 );

  // get the input and output
  vtkPolyData *input = vtkPolyData::SafeDownCast (
                         inInfo->Get ( vtkDataObject::DATA_OBJECT() ) );
  vtkPolyData *output = vtkPolyData::SafeDownCast (
                          outInfo->Get ( vtkDataObject::DATA_OBJECT() ) );

  vtkPoints *inPts;
  int num_regions;
  double center[3] = {0};
  double bounds[6] = {0};
  double length;
  this->Mesh = vtkAppendPolyData::New();
  
  vtkDebugMacro ( <<"Executing boundary edge capping filter." );

  //  Check input/allocate storage
  //
  inPts = input->GetPoints();

  if ( inPts == NULL )
    {
      vtkErrorMacro ( "No points!" );
      return 1;
    }

  // Extract boundary edges
  this->BoundaryEdges = vtkFeatureEdges::New();
  this->BoundaryEdges->SetInput ( input );
  this->BoundaryEdges->FeatureEdgesOff();
  this->BoundaryEdges->NonManifoldEdgesOff();

  // Extract individual connected regions
  this->BoundaryEdgesConnectivity = vtkPolyDataConnectivityFilter::New();
  this->BoundaryEdgesConnectivity->SetInput( this->BoundaryEdges->GetOutput() );
  this->BoundaryEdgesConnectivity->SetExtractionModeToSpecifiedRegions();
  this->BoundaryEdgesConnectivity->Update();
  
  this->Triangulator = vtkDelaunay2D::New();
  this->Triangulator->SetTolerance(this->TriangulationTolerance);
  this->Triangulator->BoundingTriangulationOff();
  
  vtkMath *math = vtkMath::New();
  
  // Loop over connected regions  
  vtkCleanPolyData *region = vtkCleanPolyData::New();
  region->CreateDefaultLocator();
  num_regions = BoundaryEdgesConnectivity->GetNumberOfExtractedRegions();
    
  for ( int i = 0; i < num_regions; ++i )
    {      
      this->BoundaryEdgesConnectivity->AddSpecifiedRegion(i);
      this->BoundaryEdgesConnectivity->Update();
      
      // Get rid of orphan nodes
      region->SetInputConnection(this->BoundaryEdgesConnectivity->GetOutputPort());
      region->Update();
      
      vtkPoints *boundaryPoints = region->GetOutput()->GetPoints();
      
      vtkIdType numPoints = region->GetOutput()->GetNumberOfPoints();
      vtkIdType numCells = region->GetOutput()->GetNumberOfCells();
      
      std::cout << "# points of region " << i << " is " << numPoints << std::endl;
      std::cout << "# cells of region " << i << " is " << numCells << std::endl;
      
      boundaryPoints->GetBounds(bounds);
      
      center[0] = .5*(bounds[0]+bounds[1]);
      center[1] = .5*(bounds[2]+bounds[3]);
      center[2] = .5*(bounds[4]+bounds[5]);
      
      for (vtkIdType j = 0; j < numPoints;  j+=2)  
      {
        double v[3] = {0};
        boundaryPoints->GetPoint(j,v);
        v[0] -= center[0];
        v[1] -= center[1];
        v[2] -= center[2];
        
        length = math->Norm(v);
        
        v[0] /= length;
        v[1] /= length;
        v[2] /= length;
        
        double dx = length/(this->NumberOfSubdivisions+1);
        
        for(int k = 1; k <= this->NumberOfSubdivisions; ++k)
        {
          double s = k*dx;
          double p[3] = {center[0]+s*v[0],center[1]+s*v[1],center[2]+s*v[2]};
          boundaryPoints->InsertNextPoint(p);
        }
      }
      boundaryPoints->InsertNextPoint(center);
      boundaryPoints->Squeeze();
      
      numPoints = region->GetOutput()->GetNumberOfPoints();
            
      std::cout << "total # points of region " << i << " is " << numPoints << std::endl;
      
      this->Triangulator->SetInputConnection(region->GetOutputPort());
      this->Triangulator->SetSourceConnection(region->GetOutputPort());
      this->Triangulator->Update();
      
      numCells = Triangulator->GetOutput()->GetNumberOfCells();
      
      std::cout << "total # cells of region " << i << " is " << numCells << std::endl;
      
      vtkPolyData *triMesh = this->Triangulator->GetOutput();
      this->Mesh->AddInput(triMesh);       
//       this->Mesh->Update();
      vtkPolyData *out = this->Mesh->GetOutput();

      this->BoundaryEdgesConnectivity->DeleteSpecifiedRegion(i);        
    }
    
    this->Mesh->Update();
    vtkPolyData *out = this->Mesh->GetOutput();
    output->CopyStructure(this->Mesh->GetOutput());
    
    region->Delete();
    math->Delete();
    this->BoundaryEdges->Delete();
    this->BoundaryEdgesConnectivity->Delete();
    this->Triangulator->Delete();
    this->Mesh->Delete();
    
    return 1;
}

void msvVTKPolyDataBoundaryEdgeCaps::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "NumberOfSubdivisions: " << this->NumberOfSubdivisions << "\n";

}
