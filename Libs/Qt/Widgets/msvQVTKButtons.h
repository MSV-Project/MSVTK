/*
 *  msvQVTKButtons.h
 *  
 *
 *  Created by Daniele Giunchi on 13/01/12.
 *  Copyright 2011 B3C. All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#ifndef msvQVTKButtons_H
#define msvQVTKButtons_H

#include "msvQtWidgetsExport.h"
#include "msvQVTKButtonsInterface.h"

//forward declarations
class vtkDataSet;

/**
 Class name: msvQVTKButtons
 This is the tool representing a VTK buttons.
 */
class MSV_QT_WIDGETS_EXPORT msvQVTKButtons : public msvQVTKButtonsInterface {
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

    QImage getPreview(int width, int height);

    /// set the data for preview
    void setData(vtkDataSet *data);

    /// Return FlyTo flag
    bool flyTo() const;

    /// Allow to set button position on center or on corner
    void setOnCenter(bool onCenter);

    /// Return OnCenter flag
    bool onCenter() const;

    void setCurrentRenderer(vtkRenderer *renderer);

    void setShowButton(bool visible);

private:
    /// calculate Position (center or corner)
    void calculatePosition();
    vtkDataSet* m_Data; ///< Data associated to the button (used to create the preview)
    bool m_FlyTo; ///< Flag to activate FlyTo animation
    bool m_OnCenter; ///< Flag to set button position on center or on corner (can be refactored with a enum??)
};

/////////////////////////////////////////////////////////////
// Inline methods
/////////////////////////////////////////////////////////////

// inline void msvQVTKButtons::setShowTooltip(bool value) {
//     if(value) {
//         Q_EMIT showTooltip(m_Tooltip);
//     } else {
//         Q_EMIT hideTooltip();
//     }
// }

inline bool msvQVTKButtons::flyTo() const {
  return m_FlyTo;
}

inline void msvQVTKButtons::setOnCenter(bool onCenter) {
  m_OnCenter = onCenter;
}

inline bool msvQVTKButtons::onCenter() const {
  return m_OnCenter;
}

#endif // msvQVTKButtons_H
