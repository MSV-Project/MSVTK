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

#ifndef __msvFFSMainWindow_h
#define __msvFFSMainWindow_h

// Qt includes
#include <QMainWindow>

// CTK includes
#include <ctkVTKObject.h>

// FFS includes
#include "msvFFSExport.h"

class msvQFFSMainWindowPrivate;

/// \brief Main window for the FFS demo.
class MSV_FFS_EXPORT msvQFFSMainWindow : public QMainWindow
{
  Q_OBJECT
  QVTK_OBJECT
public:
  typedef QMainWindow Superclass;
  msvQFFSMainWindow(QWidget *parent=0);
  virtual ~msvQFFSMainWindow();

public slots:
  void openData();
  void closeData();
  void aboutApplication();
  void updateView();

protected slots:
  // Automatically connected because of the on_XXX_YYY() slot name.
  void on_maxLevelsSpinBox_valueChanged(int value);
  void on_showSurface_stateChanged(int state);
  void on_showCartesianGrid_stateChanged(int state);
  void on_showOutlineCorners_stateChanged(int state);
  void on_showBoundaryEdges_stateChanged(int state);
  void on_runTimeSteps_clicked();

protected:
  QScopedPointer<msvQFFSMainWindowPrivate> d_ptr;
  
private: 
  Q_DECLARE_PRIVATE(msvQFFSMainWindow);
  Q_DISABLE_COPY(msvQFFSMainWindow);
};

#endif
