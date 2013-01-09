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

// .NAME msvVTKEmbeddedProbeFilter - sample data values at embedded locations.
// .SECTION Description
// msvVTKEmbeddedProbeFilter is a filter that computes point field arrays at
// embedded locations i.e. prescribed cell indexes and parametric coordinates in
// those cells. The filter has two inputs: the Input and Source. The Input
// geometric structure is passed through the filter. Arguments specify Input
// point data fields giving cell index and parametric coordinates at which
// fields from the Source are sampled. Sampled Source fields replace point data
// fields of the same name in the output, but it is an error if they have
// different numbers of components.
// The cell data of the source data is copied to the output based on in
// which source cell each input point is. If an array of the same name exists
// both in source's point and cell data, only the one from the point data is
// probed.
//
// This filter allows a dataset to inherit fields from a host dataset which it
// is considered embedding in, for example, time-varying geometry.

#ifndef __msvVTKEmbeddedProbeFilter_h
#define __msvVTKEmbeddedProbeFilter_h

// MSV_VTK_GRAPHICS includes
#include "msvVTKGraphicsExport.h"

// VTK includes
#include "vtkDataSetAlgorithm.h"
#include "vtkDataSetAttributes.h" // needed for vtkDataSetAttributes::FieldList

class vtkIdTypeArray;
class vtkCharArray;
class vtkMaskPoints;

class MSV_VTK_GRAPHICS_EXPORT msvVTKEmbeddedProbeFilter :
  public vtkDataSetAlgorithm
{
public:
  static msvVTKEmbeddedProbeFilter *New();
  vtkTypeMacro(msvVTKEmbeddedProbeFilter,vtkDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the source dataset to be sampled from. Cell ID and parametric
  // coordinate fields specified elsewhere must refer to cells in the source
  // dataset. Equivalent to SetInputConnection(1, algOutput).
  void SetSourceConnection(vtkAlgorithmOutput* algOutput);

  // Description:
  // Specify the name of the field in the Input point data which gives the
  // cell index to probe at for each point. Field must be scalar integer-valued.
  // If the field array length is less than the number of points, the last value
  // is used for all remaining points. If this field is unspecified, the first
  // cell in the source dataset is used.
  vtkSetStringMacro(CellIdArrayName)
  vtkGetStringMacro(CellIdArrayName)

  // Description:
  // Specify the name of the field in the Input point data which gives the
  // parametric coordinates in the cell at which to probe at. Field should have
  // as many components as the highest dimension of any cells probed in, but is
  // padded with zeros up to 3 dimensions. This must be set, and the field array
  // must have values for every point in the Input.
  vtkSetStringMacro(ParametricCoordinateArrayName)
  vtkGetStringMacro(ParametricCoordinateArrayName)

//BTX 
protected:
  msvVTKEmbeddedProbeFilter();
  ~msvVTKEmbeddedProbeFilter();

  char *CellIdArrayName;
  char *ParametricCoordinateArrayName;

  virtual int RequestData(vtkInformation *, vtkInformationVector **, 
    vtkInformationVector *);
  virtual int RequestInformation(vtkInformation *, vtkInformationVector **, 
    vtkInformationVector *);
  virtual int RequestUpdateExtent(vtkInformation *, vtkInformationVector **, 
    vtkInformationVector *);

  // Description:
  // Equivalent to calling
  // BuildFieldList(); InitializeForProbing(); PerformProbe();
  int Probe(vtkDataSet *input, vtkDataSet *source, vtkDataSet *output);

  // Description:
  // Build the field lists. This is required before calling
  // InitializeForProbing().
  void BuildFieldList(vtkDataSet* source);

  // Description:
  // Initializes output and various arrays which keep track for probing status.
  virtual void InitializeForProbing(vtkDataSet *input, vtkDataSet *output);

  // Description:
  // Perform embedded probe operation.
  // srcIdx is the index in the PointList for the given source. 
  int PerformProbe(vtkDataSet *input, int srcIdx, vtkDataSet *source, 
    vtkDataSet *output);

  vtkDataSetAttributes::FieldList* CellList;
  vtkDataSetAttributes::FieldList* PointList;
private:
  msvVTKEmbeddedProbeFilter(const msvVTKEmbeddedProbeFilter&);
    // Not implemented.
  void operator=(const msvVTKEmbeddedProbeFilter&);  // Not implemented.

  class vtkVectorOfArrays;
  vtkVectorOfArrays* CellArrays;
//ETX
};

#endif
