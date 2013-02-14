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

#ifndef __msvButtonClustersMainWindow_h
#define __msvButtonClustersMainWindow_h

// Qt includes
#include <QMainWindow>
class QListWidgetItem;

// CTK includes
#include <ctkVTKObject.h>

// ButtonClusters includes
#include "msvButtonClustersExport.h"

class msvQButtonClustersMainWindowPrivate;

/// \brief Main window for the ButtonClusters demo
class MSV_ButtonClusters_EXPORT msvQButtonClustersMainWindow
  : public QMainWindow
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef QMainWindow Superclass;
  msvQButtonClustersMainWindow(QWidget *parent=0);
  virtual ~msvQButtonClustersMainWindow();

public slots:
  void openData();
  void closeData();
  void aboutApplication();
  void updateView();

protected slots:
  void on_ShowDiscs_stateChanged(int state);
  void on_ShowVolume_stateChanged(int state);
  void on_EnableClustering_stateChanged(int state);
  void on_ClusterWithinGroups_stateChanged(int state);
  void on_UsePlainVTKButtons_stateChanged(int state);
  void on_ShowClustersRep_stateChanged(int state);
  void on_PixelRadius_valueChanged(double value);

protected:
  QScopedPointer<msvQButtonClustersMainWindowPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(msvQButtonClustersMainWindow);
  Q_DISABLE_COPY(msvQButtonClustersMainWindow);
};

#endif
