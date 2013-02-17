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

#ifndef __msvFluidSimulator_h
#define __msvFluidSimulator_h

// VTK includes
#include <vtkObject.h>

// FFS includes
#include "msvFFSExport.h"

class vtkHierarchicalBoxDataSet;
class vtkPolyData;

class MSV_FFS_EXPORT msvFluidSimulator : public vtkObject
{
public:
  static msvFluidSimulator* New();
  vtkTypeMacro(msvFluidSimulator,vtkObject)
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set / Get the simulator init file
  vtkSetStringMacro(InitFile);
  vtkGetStringMacro(InitFile);

  // Description:
  // Set / Get the AMR finest level grid spacing
  vtkSetMacro(FinestGridSpacing,int);
  vtkGetMacro(FinestGridSpacing,int);
  
  // Description:
  // Set / Get the AMR coarsets level grid spacing
  virtual void SetCoarsestGridSpacing(int);
  vtkGetMacro(CoarsestGridSpacing,int);

  // Description:
  // Set / Get the AMR max number of levels
  vtkSetMacro(MaxLevels,int);
  vtkGetMacro(MaxLevels,int);

  // Description:
  // Set / Get the fluid density
  vtkSetMacro(CFLCondition,double);
  vtkGetMacro(CFLCondition,double);

  // Description:
  // Set / Get the fluid density
  vtkSetMacro(FluidDensity,double);
  vtkGetMacro(FluidDensity,double);

  // Description:
  // Set / Get the fluid dynamic viscosity
  vtkSetMacro(FluidViscosity,double);
  vtkGetMacro(FluidViscosity,double);

  // Description:
  // Set / Get the AMR max number of levels
  vtkSetMacro(RefinamentRatio,int);
  vtkGetMacro(RefinamentRatio,int);

  // Description:
  // Set / Get where to put the immersed boundary
  vtkSetClampMacro(DataLevel,int,0,this->MaxLevels-1);
  vtkGetMacro(DataLevel,int);

  // Description:
  // Set / Get the size of largest patch size
  vtkSetVector3Macro(LargestPatch,int);
  vtkGetVector3Macro(LargestPatch,int);

  // Description:
  // Set / Get the size of smallest patch size
  vtkSetVector3Macro(SmallestPatch,int);
  vtkGetVector3Macro(SmallestPatch,int);

  // Description:
  // Set / Get the internal vtk AMR dataset
  virtual void SetAMRDataset(vtkHierarchicalBoxDataSet*);
  vtkGetMacro(AMRDataset,vtkHierarchicalBoxDataSet*);
  
  // Description:
  // Initialize AMR data structure, solver and main algorithms
  virtual void Init(vtkPolyData *polydata);

  // Description:
  // Run one time-step of the fluid solver
  virtual void Run();

protected:
  msvFluidSimulator();
  virtual ~msvFluidSimulator();

  // Set hierarchical dataset
  virtual void SetDataSet();

  char* InitFile;
  int   CoarsestGridSpacing;
  int   FinestGridSpacing;
  int   MaxLevels;
  int   DataLevel;
  int   RefinamentRatio;
  int   LargestPatch[3];
  int   SmallestPatch[3];
  double FluidDensity;
  double FluidViscosity;
  double CFLCondition;

  class vtkInternal;
  vtkInternal * Internal;

  vtkHierarchicalBoxDataSet* AMRDataset;

private:
  msvFluidSimulator(const msvFluidSimulator&);  // Not implemented.
  void operator=(const msvFluidSimulator&);  // Not implemented.
};

#endif
