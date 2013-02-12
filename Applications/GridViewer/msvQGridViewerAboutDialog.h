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

#ifndef __msvQGridViewerAboutDialog_h
#define __msvQGridViewerAboutDialog_h

// Qt includes
#include <QDialog>

// CTK includes
#include <ctkPimpl.h>

// GridViewer includes
#include "msvGridViewerExport.h"

class msvQGridViewerAboutDialogPrivate;

/// \brief About dialog for the GridViewer demo.
class MSV_GridViewer_EXPORT msvQGridViewerAboutDialog : public QDialog
{
  Q_OBJECT
public:
  msvQGridViewerAboutDialog(QWidget* parentWidget = 0);
  virtual ~msvQGridViewerAboutDialog();

protected:
  QScopedPointer<msvQGridViewerAboutDialogPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(msvQGridViewerAboutDialog);
  Q_DISABLE_COPY(msvQGridViewerAboutDialog);
};

#endif
