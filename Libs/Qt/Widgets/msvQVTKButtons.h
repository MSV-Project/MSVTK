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

#ifndef msvQVTKButtons_H
#define msvQVTKButtons_H

#include "msvQtWidgetsExport.h"
#include "msvQVTKButtonsInterface.h"

//forward declarations
class vtkRenderer;
class vtkButtonWidget;
class vtkButtonCallback;
class vtkButtonHighLightCallback;
class vtkRenderWindow;
class vtkDataSet;

/**
 Class name: msvQVTKButtons
 This is the tool representing a VTK buttons.
 */
class MSV_QT_WIDGETS_EXPORT msvQVTKButtons : public msvQVTKButtonsInterface
{
  Q_OBJECT
  Q_PROPERTY(bool flyTo READ flyTo WRITE setFlyTo);
  Q_PROPERTY(bool onCenter READ onCenter WRITE setOnCenter);

public Q_SLOTS:
  /// Allow to execute and update the pipeline when something change.
  /*virtual*/ void update();

public:
  /// Object constructor.
  msvQVTKButtons(QObject *parent = 0);

  /// set the icon path
  //void setIconFileName(QString iconFileName);

  // Allow to activate FlyTo animation
  void setFlyTo(bool active);

  /// set bounds
  void setBounds(double b[6]);

  /// Object destructor.
  virtual ~msvQVTKButtons();

  /// Get the button preview image
  QImage getPreview(int width, int height);

  /// set the data for preview
  void setData(vtkDataSet *data);

  /// Return FlyTo flag
  bool flyTo() const;

  /// Allow to set button position on center or on corner
  void setOnCenter(bool onCenter);

  /// Return OnCenter flag
  bool onCenter() const;

  // Set the current renderer
  void setCurrentRenderer(vtkRenderer *renderer);

private:
  /// Calculate position (center or corner)
  void calculatePosition();

  QImage m_Image; ///< button image
  vtkDataSet* m_Data; ///< dataset associated with the button
  vtkRenderWindow *m_Window; ///< render window for offscreen rendering
  bool m_FlyTo; ///< Flag to activate FlyTo animation
  bool m_OnCenter; ///< Flag to set button position on center or on corner (can be refactored with a enum??)
};

/////////////////////////////////////////////////////////////
// Inline methods
/////////////////////////////////////////////////////////////

// inline void msvQVTKButtons::setShowTooltip(bool value)
// {
//   if(value)
//   {
//     Q_EMIT showTooltip(m_Tooltip);
//   }
//   else
//   {
//     Q_EMIT hideTooltip();
//   }
// }

inline bool msvQVTKButtons::flyTo() const
{
  return m_FlyTo;
}

inline void msvQVTKButtons::setOnCenter(bool onCenter)
{
  m_OnCenter = onCenter;
}

inline bool msvQVTKButtons::onCenter() const
{
  return m_OnCenter;
}

#endif // msvQVTKButtons_H
