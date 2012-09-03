/*
 *  msvQVTKButtonsInterface.h
 *  
 *
 *  Created by Alberto Losi on 10/08/12.
 *  Copyright 2011 B3C. All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#ifndef msvQVTKButtonsInterface_H
#define msvQVTKButtonsInterface_H

#include "msvQtWidgetsExport.h"

#include <QObject>

//forward declarations
class vtkRenderer;

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
    Q_PROPERTY(bool flyTo READ flyTo WRITE setFlyTo);
    Q_PROPERTY(bool onCenter READ onCenter WRITE setOnCenter);

public:
     /// Object constructor.
    msvQVTKButtonsInterface(QObject *parent = 0);

    /// Allow to show/hide button
    void setShowButton(bool show);

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

    /// Allow to activate FlyTo animation
    void setFlyTo(bool active);

    /// Return FlyTo flag
    bool flyTo() const;

    /// Allow to set button position on center or on corner
    void setOnCenter(bool onCenter);

    /// Return OnCenter flag
    bool onCenter() const;
    
    /// set the tooltip string
    void setToolTip(QString text);

    /// Get the tooltip string
    QString toolTip();

    /// add vtk button to Renderer
    virtual void setCurrentRenderer(vtkRenderer *renderer)=0;

    /// Object destructor.
    virtual ~msvQVTKButtonsInterface();

protected:

    QString m_Label; ///< label of the button
    QString m_Tooltip; ///< tooltip associated to the button
    QString m_IconFileName; ///< File name of the image to be applied to the button.
    bool m_ShowButton;///< Flag to show/hide button
    bool m_ShowLabel; ///< Flag to show/hide label
    bool m_FlyTo; ///< Flag to activate FlyTo animation
    bool m_OnCenter; ///< Flag to set button position on center or on corner (can be refactored with a enum??)
};

/////////////////////////////////////////////////////////////
// Inline methods
/////////////////////////////////////////////////////////////

inline void msvQVTKButtonsInterface::setShowButton(bool show) {
  m_ShowButton = show;
}

inline bool msvQVTKButtonsInterface::showButton() const {
  return m_ShowButton;
}

inline void msvQVTKButtonsInterface::setShowLabel(bool show) {
  m_ShowLabel = show;
}

inline bool msvQVTKButtonsInterface::showLabel() const {
  return m_ShowLabel;
}

inline bool msvQVTKButtonsInterface::flyTo() const {
  return m_FlyTo;
}

inline void msvQVTKButtonsInterface::setOnCenter(bool onCenter) {
  m_OnCenter = onCenter;
}

inline bool msvQVTKButtonsInterface::onCenter() const {
  return m_OnCenter;
}

inline void msvQVTKButtonsInterface::setLabel(QString text) {
  m_Label = text;
}

inline QString msvQVTKButtonsInterface::label() {
  return m_Label;
}

inline QString msvQVTKButtonsInterface::toolTip() {
  return m_Tooltip;
}

inline QString msvQVTKButtonsInterface::iconFileName() {
  return m_IconFileName;
}

inline void msvQVTKButtonsInterface::setToolTip(QString text) {
  m_Tooltip = text;
}

inline void msvQVTKButtonsInterface::setFlyTo(bool active) {
  m_FlyTo = active;
}

inline void msvQVTKButtonsInterface::setIconFileName(QString iconFileName) {
  m_IconFileName = iconFileName;
}

#endif // msvQVTKButtonsInterface_H
