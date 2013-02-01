/*==============================================================================

  Library: MSVTK

  Copyright (c) The University of Auckland

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

#ifndef __msvGridViewerMainWindow_h
#define __msvGridViewerMainWindow_h

// Qt includes
#include <QMainWindow>

// CTK includes
#include <ctkVTKObject.h>

// GridViewer includes
#include "msvGridViewerExport.h"

class msvQGridViewerMainWindowPrivate;
class QListWidgetItem;

class MSV_GridViewer_EXPORT msvQGridViewerMainWindow : public QMainWindow
{
  Q_OBJECT
  QVTK_OBJECT
public:
  typedef QMainWindow Superclass;
  msvQGridViewerMainWindow(QWidget *parent=0);
  virtual ~msvQGridViewerMainWindow();

public slots:
  void openData();
  void closeData();
  void resetCamera();
  void autorangeScalar();
  void aboutApplication();
  void updateView();
  void onActorsListItemChanged(QListWidgetItem * item);

protected slots:
  void onCurrentTimeChanged(double);

protected:
  QScopedPointer<msvQGridViewerMainWindowPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(msvQGridViewerMainWindow);
  Q_DISABLE_COPY(msvQGridViewerMainWindow);
};

#endif
