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

#ifndef __msvQHAIAboutDialog_h
#define __msvQHAIAboutDialog_h

// Qt includes
#include <QDialog>

// CTK includes
#include <ctkPimpl.h>

// HAI includes
#include "msvHAIExport.h"

class msvQHAIAboutDialogPrivate;

class MSV_HAI_EXPORT msvQHAIAboutDialog : public QDialog
{
  Q_OBJECT
public:
  msvQHAIAboutDialog(QWidget* parentWidget = 0);
  virtual ~msvQHAIAboutDialog();

protected:
  QScopedPointer<msvQHAIAboutDialogPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(msvQHAIAboutDialog);
  Q_DISABLE_COPY(msvQHAIAboutDialog);
};

#endif
