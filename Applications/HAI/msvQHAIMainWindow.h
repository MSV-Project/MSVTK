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

#ifndef __msvHAIMainWindow_h
#define __msvHAIMainWindow_h

// Qt includes
#include <QMainWindow>

// CTK includes
#include <ctkVTKObject.h>

// ECG includes
#include "msvHAIExport.h"

class QTreeWidgetItem;
class msvQHAIMainWindowPrivate;

class MSV_HAI_EXPORT msvQHAIMainWindow : public QMainWindow
{
  Q_OBJECT
  QVTK_OBJECT
public:
  typedef QMainWindow Superclass;
  msvQHAIMainWindow(QWidget *parent=0);
  virtual ~msvQHAIMainWindow();

public slots:
  void addData();
  void clearData();
  void aboutApplication();
  void setDefaultLOD(int lod);

protected slots:
  void updateLODFromItem(QTreeWidgetItem* item);
protected:
  QScopedPointer<msvQHAIMainWindowPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(msvQHAIMainWindow);
  Q_DISABLE_COPY(msvQHAIMainWindow);
};

#endif
