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

// HAI includes
#include "msvQHAIAboutDialog.h"
#include "ui_msvQHAIAboutDialog.h"

//------------------------------------------------------------------------------
// msvQHAIAboutDialogPrivate methods

//------------------------------------------------------------------------------
class msvQHAIAboutDialogPrivate: public Ui_msvQHAIAboutDialog
{
public:
};

//------------------------------------------------------------------------------
// msvQHAIAboutDialog methods

//------------------------------------------------------------------------------
// msvQHAIAboutDialog methods
msvQHAIAboutDialog::msvQHAIAboutDialog(QWidget* parentWidget)
  : QDialog(parentWidget), d_ptr(new msvQHAIAboutDialogPrivate)
{
  Q_D(msvQHAIAboutDialog);
  d->setupUi(this);

  d->CreditsTextEdit->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
}

//------------------------------------------------------------------------------
msvQHAIAboutDialog::~msvQHAIAboutDialog()
{
}
