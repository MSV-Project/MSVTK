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

#ifndef __msvVTKPolyDataBoundaryEdgeCaps_h
#define __msvVTKPolyDataBoundaryEdgeCaps_h

#include "vtkPolyDataAlgorithm.h"

class vtkAppendPolyData;
class vtkFeatureEdges;
class vtkPolyDataConnectivityFilter;
class vtkDelaunay2D;

class VTK_FILTERING_EXPORT msvVTKPolyDataBoundaryEdgeCaps : public vtkPolyDataAlgorithm
{
public: 
  vtkTypeMacro(msvVTKPolyDataBoundaryEdgeCaps,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Construct with default NumberOfSubdivisions = 4.
  static msvVTKPolyDataBoundaryEdgeCaps *New();
  
  // Description:
  // Set the number of subdivisions between center of 
  // boundary edge and points on boundary.
  vtkSetMacro(NumberOfSubdivisions,int);
  vtkGetMacro(NumberOfSubdivisions,int);
  
  // Description:
  // Specify a tolerance to control discarding of closely spaced points
  // in the triangulation.
  // This tolerance is specified as a fraction of the diagonal length of
  // the bounding box of the points.
  vtkSetMacro(TriangulationTolerance,double);
  vtkGetMacro(TriangulationTolerance,double);
  
protected:
  msvVTKPolyDataBoundaryEdgeCaps();
  ~msvVTKPolyDataBoundaryEdgeCaps();
  
  // Usual data generation method
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  
  int NumberOfSubdivisions; // 
  double TriangulationTolerance; // 
  
  // used to support algorithm execution
  vtkFeatureEdges *BoundaryEdges;
  vtkPolyDataConnectivityFilter *BoundaryEdgesConnectivity;
  vtkDelaunay2D *Triangulator;
  vtkAppendPolyData *Mesh;
  
private:
  msvVTKPolyDataBoundaryEdgeCaps(const msvVTKPolyDataBoundaryEdgeCaps&);  // Not implemented.
  void operator=(const msvVTKPolyDataBoundaryEdgeCaps&);  // Not implemented.
  
  
  
};

#endif // __msvVTKPolyDataBoundaryEdgeCaps_h
