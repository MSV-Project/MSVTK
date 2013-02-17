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

// ButtonClusters includes
#include "msvQButtonClustersAboutDialog.h"
#include "ui_msvQButtonClustersAboutDialog.h"

//------------------------------------------------------------------------------
// msvQButtonClustersAboutDialogPrivate methods

//------------------------------------------------------------------------------
class msvQButtonClustersAboutDialogPrivate: public Ui_msvQButtonClustersAboutDialog
{
public:
};

//------------------------------------------------------------------------------
// msvQButtonClustersAboutDialog methods

//------------------------------------------------------------------------------
// msvQButtonClustersAboutDialog methods
msvQButtonClustersAboutDialog::msvQButtonClustersAboutDialog(QWidget* parentWidget)
  : QDialog(parentWidget), d_ptr(new msvQButtonClustersAboutDialogPrivate)
{
  Q_D(msvQButtonClustersAboutDialog);
  d->setupUi(this);

  d->CreditsTextEdit->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
}

//------------------------------------------------------------------------------
msvQButtonClustersAboutDialog::~msvQButtonClustersAboutDialog()
{
}
