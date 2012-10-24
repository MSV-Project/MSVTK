/*==============================================================================

  Library: MSVTK

  Copyright (c) SCS s.r.l. (B3C)

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

#ifndef msvQVTKButtonsInterface_H
#define msvQVTKButtonsInterface_H

#include "msvQtWidgetsExport.h"
#include <QImage>

//forward declarations
class vtkRenderer;
class vtkButtonWidget;
class vtkCommand;
class msvQVTKButtonsAction;

/**
 Class name: msvQVTKButtonsInterface
 Interface class for buttons generalization
 */
class MSV_QT_WIDGETS_EXPORT msvQVTKButtonsInterface : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString label READ label WRITE setLabel);
  Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip);
  Q_PROPERTY(QString iconFileName READ iconFileName WRITE setIconFileName);
  Q_PROPERTY(bool showButton READ showButton WRITE setShowButton);
  Q_PROPERTY(bool showLabel READ showLabel WRITE setShowLabel);

public:

  /// Object constructor
  msvQVTKButtonsInterface(QObject *parent = 0);

  /// Allow to show/hide button
  void setShowButton(bool visible);

  /// Return showLabel flag
  bool showButton() const;

  /// Allow to show/hide label
  void setShowLabel(bool show);

  /// set the icon path
  void setIconFileName(QString iconFileName);

  /// Get the icon path
  QString iconFileName();

  /// Return showLabel flag
  bool showLabel() const;

  /// set the text label
  void setLabel(QString text);

  /// Get The string
  QString label();

  /// set the tooltip string
  void setToolTip(QString text);

  /// Get the tooltip string
  QString toolTip();

  /// Object destructor.
  virtual ~msvQVTKButtonsInterface();

  /// retrieve button pointer.
  vtkButtonWidget *button();

  /// add vtk button to Renderer
  void setCurrentRenderer(vtkRenderer *renderer);

  /// set the show/hide signal
  void setShowTooltip(bool value);

  /// update graphic objects
  void update();

  /// get element bounds
  void getBounds(double b[6]);

signals:

  /// signal launched with shown tooltip
  void showTooltip(QString text);

  /// signal launched with shown tooltip
  void hideTooltip();

  /// show/hide
  void show(bool show);

protected:

  QString m_Label; ///< label of the button
  QString m_Tooltip; ///< tooltip associated to the button
  QString m_IconFileName; ///< File name of the image to be applied to the button.
  bool m_ShowButton;///< Flag to show/hide button
  bool m_ShowLabel; ///< Flag to show/hide label
  msvQVTKButtonsAction* m_Action; ///< Action performed when the vtk button is pressed (e.g. fly to)
  vtkButtonWidget *m_ButtonWidget; ///< VTK button widget.
  vtkCommand *m_ButtonCallback; ///< Callback called by picking on vtkButton
  vtkCommand *m_HighlightCallback; ///< Callback called by hovering over the button.
  QImage m_Image; ///< Button image
  double m_Bounds[6]; ///< Bounds of the data related to the button
};

/////////////////////////////////////////////////////////////
// Inline methods
/////////////////////////////////////////////////////////////

inline void msvQVTKButtonsInterface::setShowTooltip(bool value)
{
  if(value) {
    Q_EMIT showTooltip(m_Tooltip);
  } else {
    Q_EMIT hideTooltip();
  }
}

inline bool msvQVTKButtonsInterface::showButton() const
{
  return m_ShowButton;
}

inline void msvQVTKButtonsInterface::setShowLabel(bool show)
{
  m_ShowLabel = show;
}

inline bool msvQVTKButtonsInterface::showLabel() const
{
  return m_ShowLabel;
}

inline void msvQVTKButtonsInterface::setLabel(QString text)
{
  m_Label = text;
}

inline QString msvQVTKButtonsInterface::label()
{
  return m_Label;
}

inline QString msvQVTKButtonsInterface::toolTip()
{
  return m_Tooltip;
}

inline QString msvQVTKButtonsInterface::iconFileName()
{
  return m_IconFileName;
}

inline void msvQVTKButtonsInterface::setToolTip(QString text)
{
  m_Tooltip = text;
}

// inline void msvQVTKButtonsInterface::setIconFileName(QString iconFileName)
// {
//   m_IconFileName = iconFileName;
// }

#endif // msvQVTKButtonsInterface_H
