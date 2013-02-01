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

#ifndef __msvVTKBoundaryEdgeSources_h
#define __msvVTKBoundaryEdgeSources_h

// VTK_FILTERING includes
#include "msvVTKFilteringExport.h"

// VTK includes
#include "vtkPolyDataAlgorithm.h"

class MSV_VTK_FILTERING_EXPORT msvVTKBoundaryEdgeSources : public
                           vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(msvVTKBoundaryEdgeSources,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  static msvVTKBoundaryEdgeSources *New();

  double GetRadius(unsigned int i);
  std::vector<double> &GetRadii();

protected:
  msvVTKBoundaryEdgeSources();
  ~msvVTKBoundaryEdgeSources();

  // Usual data generation method
  int RequestData(vtkInformation *, vtkInformationVector **,
                  vtkInformationVector *);

private:

  class vtkInternal;
  vtkInternal *Internal;

  msvVTKBoundaryEdgeSources(const msvVTKBoundaryEdgeSources&);  // Not implemented.
  void operator=(const msvVTKBoundaryEdgeSources&);  // Not implemented.
};

#endif // __msvVTKBoundaryEdgeSources_h
