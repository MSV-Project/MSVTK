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

// By default the LOD setted is clampled to the closest available
// CAUTION: please do not try to introduce optimization within the three searc
// as it will break the FlatIndex

#ifndef __msvVTKXMLMultiblockLODReader_h
#define __msvVTKXMLMultiblockLODReader_h

// VTK_PARALLEL includes
#include "msvVTKParallelExport.h"

#include "vtkXMLMultiBlockDataReader.h"


class msvVTKXMLMultiblockLODReaderInternal;

class MSV_VTK_PARALLEL_EXPORT msvVTKXMLMultiblockLODReader : public vtkXMLMultiBlockDataReader
{
public:
  static msvVTKXMLMultiblockLODReader* New();
  vtkTypeMacro(msvVTKXMLMultiblockLODReader, vtkXMLMultiBlockDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Get / Set the level of details by default for the whole composite dataset
  // The SetDefaultLOD method restore each piece to the DefaultLOD.
  unsigned int GetDefaultLOD();
  void SetDefaultLOD(unsigned int lod);

  // Restore each piece to the LOD by default
  void RestoreDefaultLOD();

  // Givent the  number Index of a piece, request him to change its LOD.
  void SetPieceLOD(int,unsigned int);

protected:
  msvVTKXMLMultiblockLODReader();
  ~msvVTKXMLMultiblockLODReader();

  // Read the XML element for the subtree of a the composite dataset.
  // dataSetIndex is used to rank the leaf nodes in an inorder traversal.
  virtual void ReadComposite(vtkXMLDataElement* element,
    vtkCompositeDataSet* composite, const char* filePath,
    unsigned int &dataSetIndex);

  virtual int RequestInformation(vtkInformation*,
                                 vtkInformationVector**,
                                 vtkInformationVector*);

  virtual int RequestData(vtkInformation *request,
                          vtkInformationVector **inputVector,
                          vtkInformationVector *outputVector);

  virtual vtkXMLReader* GetReaderOfType(const char* type);

  // Description:
  // Given the inorder index for a leaf node, this method tells if the current
  // process should get read the dataset within the composite.
  bool ShouldGetDataSet(int datasetIndex, vtkXMLDataElement* node);

private:
  msvVTKXMLMultiblockLODReader(const msvVTKXMLMultiblockLODReader&);  // Not implemented.
  void operator=(const msvVTKXMLMultiblockLODReader&);                // Not implemented.

  msvVTKXMLMultiblockLODReaderInternal* Internal;
};

#endif
