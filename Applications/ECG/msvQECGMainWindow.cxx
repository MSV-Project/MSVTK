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

// Qt includes

// MSV includes
#include "msvQECGMainWindow.h"
#include "msvVTKFileSeriesReader.h"
#include "ui_msvQECGMainWindow.h"

// VTK includes
#include "vtkNew.h"


//-----------------------------------------------------------------------------
class msvQECGMainWindowPrivate: public Ui_msvQECGMainWindow
{
  Q_DECLARE_PUBLIC(msvQECGMainWindow);
protected:
  msvQECGMainWindow* const q_ptr;

public:
  msvQECGMainWindowPrivate(msvQECGMainWindow& object);
  virtual void setupUi(QMainWindow * mainWindow);
};

//-----------------------------------------------------------------------------
// msvQECGMainWindowPrivate methods

msvQECGMainWindowPrivate::msvQECGMainWindowPrivate(msvQECGMainWindow& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
void msvQECGMainWindowPrivate::setupUi(QMainWindow * mainWindow)
{
  this->Ui_msvQECGMainWindow::setupUi(mainWindow);
}

//-----------------------------------------------------------------------------
// msvQECGMainWindow methods

//-----------------------------------------------------------------------------
msvQECGMainWindow::msvQECGMainWindow(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new msvQECGMainWindowPrivate(*this))
{
  Q_D(msvQECGMainWindow);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
msvQECGMainWindow::~msvQECGMainWindow()
{
}
