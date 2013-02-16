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

#ifndef __msvQButtonClustersAboutDialog_h
#define __msvQButtonClustersAboutDialog_h

// Qt includes
#include <QDialog>

// CTK includes
#include <ctkPimpl.h>

// ButtonClusters includes
#include "msvButtonClustersExport.h"

class msvQButtonClustersAboutDialogPrivate;

/// \brief About dialog for the ButtonClusters demo.
class MSV_ButtonClusters_EXPORT msvQButtonClustersAboutDialog : public QDialog
{
  Q_OBJECT
public:
  msvQButtonClustersAboutDialog(QWidget* parentWidget = 0);
  virtual ~msvQButtonClustersAboutDialog();

protected:
  QScopedPointer<msvQButtonClustersAboutDialogPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(msvQButtonClustersAboutDialog);
  Q_DISABLE_COPY(msvQButtonClustersAboutDialog);
};

#endif
