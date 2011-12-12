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

#ifndef __msvECGMainWindow_h
#define __msvECGMainWindow_h

// Qt includes
#include <QMainWindow>

// ECG includes
#include "msvECGExport.h"

class msvQECGMainWindowPrivate;

class MSV_ECG_EXPORT msvQECGMainWindow : public QMainWindow
{
  Q_OBJECT
public:
  typedef QMainWindow Superclass;
  msvQECGMainWindow(QWidget *parent=0);
  virtual ~msvQECGMainWindow();

public slots:
  void openData();
  void closeData();
  void updateView();

protected:
  QScopedPointer<msvQECGMainWindowPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(msvQECGMainWindow);
  Q_DISABLE_COPY(msvQECGMainWindow);
};

#endif
