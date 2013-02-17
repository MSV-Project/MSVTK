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

// MSV includes
#include "msvVTKCompositeFileSeriesReader.h"
#include "msvVTKPolyDataFileSeriesReader.h"

// VTK includes
#include "vtkNew.h"

#include "vtkGenericDataObjectReader.h"
#include "vtkInformation.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkStdString.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStringArray.h"
#include "vtkTypeTraits.h"

#include "vtkSmartPointer.h"

//------------------------------------------------------------------------------
class msvVTKCompositeFileSeriesReaderInternal
{
public:
  msvVTKCompositeFileSeriesReaderInternal();
  ~msvVTKCompositeFileSeriesReaderInternal();

  void CreateFileSeriesReaders();
  static int ReadMetaDataFile(const char *metafilename,
                              vtkStringArray *filesToRead,
                              int maxFilesToRead = VTK_LARGE_INTEGER);

  std::vector<vtkSmartPointer<msvVTKPolyDataFileSeriesReader> > fileSeriesReaders;
  std::vector<std::string> FileNames;
};

//------------------------------------------------------------------------------
// msvVTKCompositeFileSeriesReaderInternal methods

//------------------------------------------------------------------------------
msvVTKCompositeFileSeriesReaderInternal::
msvVTKCompositeFileSeriesReaderInternal()
{

}

msvVTKCompositeFileSeriesReaderInternal::
~msvVTKCompositeFileSeriesReaderInternal()
{

}

void msvVTKCompositeFileSeriesReaderInternal::CreateFileSeriesReaders()
{
  // Avoid resiz
  this->fileSeriesReaders.resize(this->FileNames.size());

  for (std::vector<std::string>::iterator it = this->FileNames.begin();
       it != this->FileNames.end(); ++it)
    {
    vtkNew<msvVTKPolyDataFileSeriesReader> fileSeriesReader;

    this->fileSeriesReaders.push_back(fileSeriesReader.GetPointer());
    }
}

//------------------------------------------------------------------------------
// msvVTKCompositeFileSeriesReader methods

//------------------------------------------------------------------------------
vtkStandardNewMacro(msvVTKCompositeFileSeriesReader);

//------------------------------------------------------------------------------
msvVTKCompositeFileSeriesReader::msvVTKCompositeFileSeriesReader()
{
  this->SetNumberOfInputPorts(0);
//  this->SetNumberOfOutputPorts(1);

  this->Reader = 0;

  this->HiddenReaderModification = 0;
  this->SavedReaderModification = 0;

  this->Internal = new msvVTKCompositeFileSeriesReaderInternal();

  this->FileNameMethod = NULL;
  //this->SetFileNameMethod("SetFileName");

  this->MetaFileName = NULL;
  this->UseMetaFile = 1;
  this->CurrentFileName = 0;

  this->IgnoreReaderTime = 0;
}

//------------------------------------------------------------------------------
msvVTKCompositeFileSeriesReader::~msvVTKCompositeFileSeriesReader()
{
}

//------------------------------------------------------------------------------
int msvVTKCompositeFileSeriesReader::CanReadFile(const char* filename)
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
    if (msvVTKCompositeFileSeriesReaderInternal::ReadMetaDataFile(filename, dataFiles.GetPointer(), 1))
      {
      if (dataFiles->GetNumberOfValues() > 0)
        {
        return msvVTKCompositeFileSeriesReader::
          CanReadFile(this->Reader,dataFiles->GetValue(0).c_str());
        }
      }
    return 0;
    }
  else
    {
    return msvVTKCompositeFileSeriesReader::CanReadFile(this->Reader, filename);
    }
}

//------------------------------------------------------------------------------
int msvVTKCompositeFileSeriesReader::CanReadFile(vtkXMLReader* xmlReader,
                                                 const char* filename)
{
  if(!xmlReader || !filename)
    {
    return 0;
    }

  return xmlReader->CanReadFile(filename);
}

//------------------------------------------------------------------------------
void msvVTKCompositeFileSeriesReader::SetReaderFileName(const char* fname)
{
  if (this->Reader)
    {
    // We want to suppress the modification time change in the Reader. See
    // msvVTKFileSeriesReader::GetMTime() for details on how this works.
    this->SavedReaderModification = this->GetMTime();
    this->Reader->SetFileName(fname);
    this->HiddenReaderModification = this->Reader->GetMTime();
    }

  this->SetCurrentFileName(fname);
}

//------------------------------------------------------------------------------
void msvVTKCompositeFileSeriesReader::SetCurrentFileName(const char *fname)
{
  // Basically operates the same as the code created by the vtkSetStringMacro
  // except that it does NOT call Modified.  This method is only called
  // internally and just manages the state of the actuall reader, usually
  // while in ProcessRequest.
  if (this->CurrentFileName == fname) return;
  if (this->CurrentFileName)
    {
    delete[] this->CurrentFileName;
    }
  if (fname)
    {
    this->CurrentFileName = new char[strlen(fname) + 1];
    strcpy(this->CurrentFileName, fname);
    }
  else
    {
    this->CurrentFileName = NULL;
    }
}

//------------------------------------------------------------------------------
void msvVTKCompositeFileSeriesReader::AddFileName(const char* name)
{
  this->Internal->FileNames.push_back(name);
  this->Modified();
}

//------------------------------------------------------------------------------
void msvVTKCompositeFileSeriesReader::RemoveAllFileNames()
{
  this->Internal->FileNames.clear();
  this->Modified();
}

//----------------------------------------------------------------------------
unsigned int msvVTKCompositeFileSeriesReader::GetNumberOfFileNames()
{
  return static_cast<unsigned int>(this->Internal->FileNames.size());
}

//----------------------------------------------------------------------------
const char* msvVTKCompositeFileSeriesReader::GetFileName(unsigned int idx)
{
  if (idx >= this->Internal->FileNames.size())
    {
    return 0;
    }

  return this->Internal->FileNames[idx].c_str();
}

//------------------------------------------------------------------------------
int msvVTKCompositeFileSeriesReaderInternal::ReadMetaDataFile
  (const char *metafilename, vtkStringArray *filesToRead, int maxFilesToRead)
{
  // Open the metafile.
  ifstream metafile(metafilename);
  if (metafile.bad())
    {
    return 0;
    }
  // Get the path of the metafile for relative paths within.
  std::string filePath = metafilename;
  std::string::size_type pos = filePath.find_last_of("/\\");
  if(pos != filePath.npos)
    {
    filePath = filePath.substr(0, pos+1);
    }
  else
    {
    filePath = "";
    }

  // Iterate over all files pointed to by the metafile.
  filesToRead->SetNumberOfTuples(0);
  filesToRead->SetNumberOfComponents(1);
  while (   metafile.good() && !metafile.eof()
         && (filesToRead->GetNumberOfTuples() < maxFilesToRead) )
    {
    std::string fname;
    metafile >> fname;
    if (fname.empty()) continue;
    if ((fname.at(0) != '/') && ((fname.size() < 2) || (fname.at(1) != ':')))
      {
      fname = filePath + fname;
      }
    filesToRead->InsertNextValue(fname);
    }

  return 1;
}

//------------------------------------------------------------------------------
void msvVTKCompositeFileSeriesReader::UpdateMetaData()
{
  if (this->UseMetaFile && (this->MetaFileReadTime < this->MTime))
    {
    vtkNew<vtkStringArray> dataFiles;
    if (!msvVTKCompositeFileSeriesReaderInternal::ReadMetaDataFile(this->MetaFileName, dataFiles.GetPointer()))
      {
      vtkErrorMacro(<< "Could not open metafile " << this->MetaFileName);
      return;
      }

    this->RemoveAllFileNames();
    for (int i = 0; i < dataFiles->GetNumberOfValues(); i++)
      {
      this->AddFileName(dataFiles->GetValue(i).c_str());
      }

    this->MetaFileReadTime.Modified();
    }
}

//------------------------------------------------------------------------------
void msvVTKCompositeFileSeriesReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

// TODO DELETE THIS METHOD WHICH WILL BE CALL THROUGH THE REQUEST INFO
void msvVTKCompositeFileSeriesReader::CreateFileSeriesReaders()
{
  this->Internal->CreateFileSeriesReaders();
}
