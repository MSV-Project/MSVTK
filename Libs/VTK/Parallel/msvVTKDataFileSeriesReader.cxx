/*==============================================================================

  Library: MSVTK

  Copyright (c) Kitware Inc.
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

// VTK includes
#include <vtkNew.h>
#include <vtkDataReader.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>

// MSVTK includes
#include "msvVTKDataFileSeriesReader.h"

//------------------------------------------------------------------------------
vtkStandardNewMacro(msvVTKDataFileSeriesReader);

//------------------------------------------------------------------------------
msvVTKDataFileSeriesReader::msvVTKDataFileSeriesReader()
{
}

//------------------------------------------------------------------------------
msvVTKDataFileSeriesReader::~msvVTKDataFileSeriesReader()
{
}

//------------------------------------------------------------------------------
void msvVTKDataFileSeriesReader::SetReader(vtkAlgorithm* reader)
{
  this->Superclass::SetReader(vtkDataReader::SafeDownCast(reader));
}

//------------------------------------------------------------------------------
int msvVTKDataFileSeriesReader::CanReadFile(const char* filename)
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
        return msvVTKDataFileSeriesReader::
          CanReadFile(this->Reader,dataFiles->GetValue(0).c_str());
        }
      }
    return 0;
    }
  else
    {
    return msvVTKDataFileSeriesReader::CanReadFile(this->Reader, filename);
    }
}

//------------------------------------------------------------------------------
int msvVTKDataFileSeriesReader::CanReadFile(vtkAlgorithm* algo,
                                            const char* filename)
{
  vtkDataReader* reader = vtkDataReader::SafeDownCast(algo);
  if(!reader || !filename)
    {
    return 0;
    }

  reader->SetFileName(filename);

  if (reader->IsA("vtkPolyDataReader"))
    {
    return reader->IsFilePolyData();
    }
  else if (reader->IsA("vtkRectilinearGridReader"))
    {
    return reader->IsFileRectilinearGrid();
    }
  else if (reader->IsA("vtkStructuredPointsReader"))
    {
    return reader->IsFileStructuredPoints();
    }
  else if (reader->IsA("vtkStructuredGridReader"))
    {
    return reader->IsFileStructuredGrid();
    }
  else if (reader->IsA("vtkUnstructuredGridReader"))
    {
    return reader->IsFileUnstructuredGrid();
    }
  else
    {
    // there is no other type specific check method for other derived classes.
    // Always return 1.
    return 1;
    }

  return 0;
}

//------------------------------------------------------------------------------
void msvVTKDataFileSeriesReader::SetReaderFileName(const char* fname)
{
  vtkDataReader* reader = vtkDataReader::SafeDownCast(this->Reader);
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
void msvVTKDataFileSeriesReader::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
