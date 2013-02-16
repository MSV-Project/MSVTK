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

// GridViewer includes
#include "msvQGridViewerAboutDialog.h"
#include "ui_msvQGridViewerAboutDialog.h"

//------------------------------------------------------------------------------
// msvQGridViewerAboutDialogPrivate methods

//------------------------------------------------------------------------------
class msvQGridViewerAboutDialogPrivate: public Ui_msvQGridViewerAboutDialog
{
public:
};

//------------------------------------------------------------------------------
// msvQGridViewerAboutDialog methods

//------------------------------------------------------------------------------
// msvQGridViewerAboutDialog methods
msvQGridViewerAboutDialog::msvQGridViewerAboutDialog(QWidget* parentWidget)
  : QDialog(parentWidget), d_ptr(new msvQGridViewerAboutDialogPrivate)
{
  Q_D(msvQGridViewerAboutDialog);
  d->setupUi(this);

  d->CreditsTextEdit->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
}

//------------------------------------------------------------------------------
msvQGridViewerAboutDialog::~msvQGridViewerAboutDialog()
{
}
