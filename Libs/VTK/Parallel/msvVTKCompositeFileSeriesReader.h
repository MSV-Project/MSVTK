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

#ifndef __msvVTKCompositeFileSeriesReader_h
#define __msvVTKCompositeFileSeriesReader_h

// VTK_PARALLEL includes
#include "msvVTKParallelExport.h"

#include "vtkDataObjectAlgorithm.h"
#include "vtkXMLReader.h"

class msvVTKCompositeFileSeriesReaderInternal;
class vtkStringArray;

class MSV_VTK_PARALLEL_EXPORT msvVTKCompositeFileSeriesReader : public vtkDataObjectAlgorithm
{
public:
  static msvVTKCompositeFileSeriesReader* New();
  vtkTypeMacro(msvVTKCompositeFileSeriesReader, vtkDataObjectAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/get the internal reader.
  vtkSetObjectMacro(Reader, vtkXMLReader);
  vtkGetObjectMacro(Reader, vtkXMLReader);

  void CreateFileSeriesReaders();

  // Description:
  // CanReadFile is forwarded to the internal reader if it supports it.
  virtual int CanReadFile(const char*);

  // Description:
  // Static method to check wether a reader can read a file or not.
  static int CanReadFile(vtkXMLReader* reader, const char* filename);

  // Description:
  // Adds names of files to be read. The files are read in the order
  // they are added.
  virtual void AddFileName(const char* fname);

  // Description:
  // Remove all file names.
  virtual void RemoveAllFileNames();

  // Description:
  // Returns the most recent filename used.
  vtkGetStringMacro(CurrentFileName);

  // Description:
  // Returns the number of file names added by AddFileName.
  virtual unsigned int GetNumberOfFileNames();

  // Description:
  // Returns the name of a file with index idx.
  virtual const char* GetFileName(unsigned int idx);

  // Get the total number of pieces within the composite data set produced
  vtkGetMacro(NumberOfPieces, int);

protected:
  msvVTKCompositeFileSeriesReader();
  ~msvVTKCompositeFileSeriesReader();

  virtual void SetReaderFileName(const char* fname);
  vtkXMLReader* Reader;

  unsigned long HiddenReaderModification;
  unsigned long SavedReaderModification;

  virtual void SetCurrentFileName(const char *fname);
  char* CurrentFileName;
  char* FileNameMethod;

  char* MetaFileName;
  int UseMetaFile;
  vtkTimeStamp MetaFileReadTime;

  // Description:
  // Reads a metadata file and returns a list of filenames (in filesToRead).  If
  // the file could not be read correctly, 0 is returned.
  /*virtual int ReadMetaDataFile(const char *metafilename,
                               vtkStringArray *filesToRead,
                               int maxFilesToRead = VTK_LARGE_INTEGER);*/

  // Description:
  // Re-reads information from the metadata file, if necessary.
  virtual void UpdateMetaData();

  int IgnoreReaderTime;
  int NumberOfPieces;

private:
  msvVTKCompositeFileSeriesReader(const msvVTKCompositeFileSeriesReader&);  // Not implemented.
  void operator=(const msvVTKCompositeFileSeriesReader&);                   // Not implemented.

  msvVTKCompositeFileSeriesReaderInternal* Internal;
};

#endif
