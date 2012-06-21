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

#ifndef __msvVTKBUTTONSMainWindow_h
#define __msvVTKBUTTONSMainWindow_h

// Qt includes
#include <QMainWindow>
class QListWidgetItem;

// CTK includes
#include <ctkVTKObject.h>

// ECG includes
#include "msvVTKButtonsExport.h"

class msvQVTKButtonsMainWindowPrivate;

class MSV_VTKButtons_EXPORT msvQVTKButtonsMainWindow : public QMainWindow
{
  Q_OBJECT
  QVTK_OBJECT
public:
  typedef QMainWindow Superclass;
  msvQVTKButtonsMainWindow(QWidget *parent=0);
  virtual ~msvQVTKButtonsMainWindow();

public slots:
  void openData();
  void closeData();
  void aboutApplication();
  void updateView();
  void setCurrentSignal(int pointId);
  void showTooltip(QString text);
  
  void on_checkBoxShowButtons_stateChanged(int state);

protected slots:
  void onPointSelected();
  void onCurrentTimeChanged(double);
  void onVTKButtonsSelectionChanged();
  void onCurrentItemChanged(QListWidgetItem* current,
                            QListWidgetItem* previous);

protected:
  QScopedPointer<msvQVTKButtonsMainWindowPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(msvQVTKButtonsMainWindow);
  Q_DISABLE_COPY(msvQVTKButtonsMainWindow);
};

#endif
