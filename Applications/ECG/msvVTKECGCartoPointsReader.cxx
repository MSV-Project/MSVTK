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
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataReader.h>
#include <vtkStringArray.h>

// MSVTK includes
#include "msvVTKECGCartoPointsReader.h"

//------------------------------------------------------------------------------
vtkStandardNewMacro(msvVTKECGCartoPointsReader);

//------------------------------------------------------------------------------
msvVTKECGCartoPointsReader::msvVTKECGCartoPointsReader()
{
}

//------------------------------------------------------------------------------
msvVTKECGCartoPointsReader::~msvVTKECGCartoPointsReader()
{
}

//------------------------------------------------------------------------------
void msvVTKECGCartoPointsReader::SetReader(vtkAlgorithm* reader)
{
  this->Superclass::SetReader(vtkPolyDataReader::SafeDownCast(reader));
}

//------------------------------------------------------------------------------
int msvVTKECGCartoPointsReader::CanReadFile(const char* filename)
{
  if (!this->Reader)
    {
    return 0;
    }

  if (this->UseMetaFile)
    {
    vtkNew<vtkStringArray> dataFiles;
    // filename really points to a metafile.
    // Iterate over all files pointed to by the metafile and check if readable.
    if (this->ReadMetaDataFile(filename, dataFiles.GetPointer(), 1))
      {
      if (dataFiles->GetNumberOfValues() > 0)
        {
        return msvVTKECGCartoPointsReader::
          CanReadFile(this->Reader,dataFiles->GetValue(0).c_str());
        }
      }
    return 0;
    }
  else
    {
    return msvVTKECGCartoPointsReader::CanReadFile(this->Reader, filename);
    }
}

//------------------------------------------------------------------------------
int msvVTKECGCartoPointsReader::CanReadFile(vtkAlgorithm* algo,
                                            const char* filename)
{
  vtkPolyDataReader* reader = vtkPolyDataReader::SafeDownCast(algo);
  if(!reader || !filename)
    {
    return 0;
    }

  reader->SetFileName(filename);
  return reader->IsFileValid("polydata");
}

//------------------------------------------------------------------------------
void msvVTKECGCartoPointsReader::SetReaderFileName(const char* fname)
{
  vtkPolyDataReader* reader = vtkPolyDataReader::SafeDownCast(this->Reader);
  if (reader)
    {
    // We want to suppress the modification time change in the Reader.  See
    // msvVTKFileSeriesReader::GetMTime() for details on how this works.
    this->SavedReaderModification = this->GetMTime();
    reader->SetFileName(fname);
    this->HiddenReaderModification = this->Reader->GetMTime();
    }
  this->SetCurrentFileName(fname);
}

//------------------------------------------------------------------------------
void msvVTKECGCartoPointsReader::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
