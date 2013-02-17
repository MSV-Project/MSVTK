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

#ifndef __msvQVTKButtonsAboutDialog_h
#define __msvQVTKButtonsAboutDialog_h

// Qt includes
#include <QDialog>

// VTKButtons includes
#include "msvVTKButtonsExport.h"

class msvQVTKButtonsAboutDialogPrivate;

/// \brief About dialog for the VTKButtons demo.
class MSV_VTKButtons_EXPORT msvQVTKButtonsAboutDialog : public QDialog
{
  Q_OBJECT
public:
  msvQVTKButtonsAboutDialog(QWidget* parentWidget = 0);
  virtual ~msvQVTKButtonsAboutDialog();

protected:
  QScopedPointer<msvQVTKButtonsAboutDialogPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(msvQVTKButtonsAboutDialog);
  Q_DISABLE_COPY(msvQVTKButtonsAboutDialog);
};

#endif
