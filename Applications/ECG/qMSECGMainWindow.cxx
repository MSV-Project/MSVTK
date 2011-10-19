/*==============================================================================

  Library: MSVECG

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

// MSVTK includes
#include "qMSECGMainWindow.h"
#include "ui_qMSECGMainWindow.h"

// VTK includes

//-----------------------------------------------------------------------------
class qMSECGMainWindowPrivate: public Ui_qMSECGMainWindow
{
  Q_DECLARE_PUBLIC(qMSECGMainWindow);
protected:
  qMSECGMainWindow* const q_ptr;

public:
  qMSECGMainWindowPrivate(qMSECGMainWindow& object);
  virtual void setupUi(QMainWindow * mainWindow);
};

//-----------------------------------------------------------------------------
// qMSECGMainWindowPrivate methods

qMSECGMainWindowPrivate::qMSECGMainWindowPrivate(qMSECGMainWindow& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
void qMSECGMainWindowPrivate::setupUi(QMainWindow * mainWindow)
{
  this->Ui_qMSECGMainWindow::setupUi(mainWindow);
}

//-----------------------------------------------------------------------------
// qMSECGMainWindow methods

//-----------------------------------------------------------------------------
qMSECGMainWindow::qMSECGMainWindow(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMSECGMainWindowPrivate(*this))
{
  Q_D(qMSECGMainWindow);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qMSECGMainWindow::~qMSECGMainWindow()
{
}
