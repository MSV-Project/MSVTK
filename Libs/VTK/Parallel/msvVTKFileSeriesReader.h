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

/*==============================================================================

  Program:   ParaView
  Module:    vtkFileSeriesReader.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

==============================================================================*/

/*
 * Copyright 2008 Sandia Corporation.
 * Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 * license for use of this work by or on behalf of the
 * U.S. Government. Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that this Notice and any
 * statement of authorship are reproduced on all copies.
 */

// .NAME msvVTKFileSeriesReader - meta-reader to read file series
//
// .SECTION Description:
//
// msvVTKFileSeriesReader is a meta-reader that can work with various
// readers to load file series. To the pipeline, it looks like a reader
// that supports time. It updates the file name to the internal reader
// whenever a different time step is requested.
//
// If the reader already supports time, then this meta-filter will multiplex the
// time.  It will union together all the times and forward time requests to the
// file with the correct time.  Overlaps are handled by requesting data from the
// file with the upper range the farthest in the future.
//
// There are two ways to specify a series of files.  The first way is by adding
// the filenames one at a time with the AddFileName method.  The second way is
// by providing a single "meta" file.  This meta file is a simple text file that
// lists a file per line.  The files can be relative to the meta file.  This
// method is useful when the actual reader points to a set of files itself.  The
// UseMetaFile toggles between these two methods of specifying files.
//

#ifndef __msvVTKFileSeriesReader_h
#define __msvVTKFileSeriesReader_h

// VTK_PARALLEL includes
#include "msvVTKParallelExport.h"

#include "vtkDataObjectAlgorithm.h"

class vtkStringArray;
struct msvVTKFileSeriesReaderInternals;

class MSV_VTK_PARALLEL_EXPORT msvVTKFileSeriesReader : public vtkDataObjectAlgorithm
{
public:
  vtkTypeMacro(msvVTKFileSeriesReader, vtkDataObjectAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/get the internal reader.
  virtual void SetReader(vtkAlgorithm*);
  vtkGetObjectMacro(Reader, vtkAlgorithm);

  // Description:
  // All pipeline passes are forwarded to the internal reader. The
  // msvVTKFileSeriesReader reports time steps in RequestInformation. It
  // updated the file name of the internal in RequestUpdateExtent based
  // on the time step request.
  virtual int ProcessRequest(vtkInformation*,
                             vtkInformationVector**,
                             vtkInformationVector*);

  // Description:
  // CanReadFile is forwarded to the internal reader if it supports it.
  static int CanReadFile(vtkAlgorithm* vtkNotUsed(reader),
                         const char* vtkNotUsed(filename)){return 0;}

  // Description:
  // Adds names of files to be read. The files are read in the order
  // they are added.
  virtual void AddFileName(const char* fname);

  // Description:
  // Remove all file names.
  virtual void RemoveAllFileNames();

  // Description:
  // Returns the number of file names added by AddFileName.
  virtual unsigned int GetNumberOfFileNames();

  // Description:
  // Returns the name of a file with index idx.
  virtual const char* GetFileName(unsigned int idx);

  // Description:
  // Returns the most recent filename used.
  vtkGetStringMacro(CurrentFileName);

  // Description:
  // Get/set the filename for the meta-file.  Has no effect unless UseMetaFile
  // is true.
  vtkGetStringMacro(MetaFileName);
  vtkSetStringMacro(MetaFileName);

  // Description:
  // If true, then use the meta file.  False by default.
  vtkGetMacro(UseMetaFile, int);
  vtkSetMacro(UseMetaFile, int);
  vtkBooleanMacro(UseMetaFile, int);

  // Description:
  // Return the MTime also considering the internal reader.
  virtual unsigned long GetMTime();

  // Description:
  // Name of the method used to set the file name of the internal
  // reader. By default, this is SetFileName.
  vtkSetStringMacro(FileNameMethod);
  vtkGetStringMacro(FileNameMethod);

  // Description:
  // If true, then treat file series like it does not contain any time step
  // values. False by default.
  vtkGetMacro(IgnoreReaderTime, int);
  vtkSetMacro(IgnoreReaderTime, int);
  vtkBooleanMacro(IgnoreReaderTime, int);

  // Description:
  // Get/Set the output time range.
  // It lets you specifying you own time range if
  // the reader does not manage this information. It linearly associates
  // each step to an interval of time.
  // By default the range do a 1:1 mapping between the
  // Time_Range and the Time_Steps.
  virtual void SetOutputTimeRange(double, double);
  virtual void SetOutputTimeRange(double range[2]);
  virtual void GetOutputTimeRange(double range[2]);

  // Description
  // Update TimeRange linearly for each file when user has set one.
  void UpdateOutputTimeRange();

protected:
  msvVTKFileSeriesReader();
  ~msvVTKFileSeriesReader();

  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);
  virtual int RequestUpdateExtent(vtkInformation*,
                                  vtkInformationVector**,
                                  vtkInformationVector*);
  virtual int RequestData(vtkInformation *request,
                          vtkInformationVector **inputVector,
                          vtkInformationVector *outputVector);

  virtual int FillOutputPortInformation(int port, vtkInformation* info);

  // Description:
  // Make sure the reader's output is set to the given index and, if it changed,
  // run RequestInformation on the reader.
  virtual int RequestInformationForInput(
                                     int index,
                                     vtkInformation *request = NULL,
                                     vtkInformationVector *outputVector = NULL);

  // Description:
  // The last file index for which RequestInformationForInput was run.
  int LastRequestInformationIndex;

  // Description:
  // Reads a metadata file and returns a list of filenames (in filesToRead).  If
  // the file could not be read correctly, 0 is returned.
  virtual int ReadMetaDataFile(const char *metafilename,
                               vtkStringArray *filesToRead,
                               int maxFilesToRead = VTK_LARGE_INTEGER);

  virtual void SetReaderFileName(const char* fname)=0;
  vtkAlgorithm* Reader;

  unsigned long HiddenReaderModification;
  unsigned long SavedReaderModification;

  virtual void SetCurrentFileName(const char *fname);
  char* CurrentFileName;
  char* FileNameMethod;

  char *MetaFileName;
  int UseMetaFile;
  vtkTimeStamp MetaFileReadTime;

  // Description:
  // Re-reads information from the metadata file, if necessary.
  virtual void UpdateMetaData();

  int IgnoreReaderTime;

private:
  msvVTKFileSeriesReader(const msvVTKFileSeriesReader&);  // Not implemented.
  void operator=(const msvVTKFileSeriesReader&);          // Not implemented.

  msvVTKFileSeriesReaderInternals* Internal;
};

#endif
