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

#ifndef __msvVTKPolyDataFileSeriesReader_h
#define __msvVTKPolyDataFileSeriesReader_h

// VTK_PARALLEL includes
#include "msvVTKParallelExport.h"

// VTK includes
#include "msvVTKFileSeriesReader.h"
#include "vtkPolyDataReader.h"

class MSV_VTK_PARALLEL_EXPORT msvVTKPolyDataFileSeriesReader : public msvVTKFileSeriesReader
{
public:
  vtkTypeMacro(msvVTKPolyDataFileSeriesReader, msvVTKFileSeriesReader);
  static msvVTKPolyDataFileSeriesReader *New();
  virtual void PrintSelf(ostream &os, vtkIndent indent);

  // Description:
  // Set / Get the internal reader.
  virtual void SetReader(vtkAlgorithm*);

  virtual int CanReadFile(const char*);
  static int CanReadFile(vtkAlgorithm*, const char*);

protected:
  msvVTKPolyDataFileSeriesReader();
  virtual ~msvVTKPolyDataFileSeriesReader();
  virtual void SetReaderFileName(const char* fname);

private:
  msvVTKPolyDataFileSeriesReader(const msvVTKPolyDataFileSeriesReader&);// Not implemented.
  void operator=(const msvVTKPolyDataFileSeriesReader&);                // Not implemented.
};

#endif
