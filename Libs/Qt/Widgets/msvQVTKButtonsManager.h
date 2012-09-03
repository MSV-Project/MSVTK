/*
 *  msvQVTKButtonsManager.h
 *  
 *
 *  Created by Alberto Losi on 08/08/12.
 *  Copyright 2009 B3C.s All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#ifndef msvQVTKButtonsManager_H
#define msvQVTKButtonsManager_H

// Includes list

#include "msvQtWidgetsExport.h"
#include "msvQVTKButtonsGroup.h"
/**
Class name: msvQVTKButtonsManager
Manager class to manage groups of msvQVTKButtons and msvVTKButtonsGroup
*/

class MSV_QT_WIDGETS_EXPORT msvQVTKButtonsManager : public msvQVTKButtonsGroup 
{
    Q_OBJECT

public:
    /// Object destructor.
    virtual ~msvQVTKButtonsManager();

    /// Get the singleton instance of the manager
    static msvQVTKButtonsManager* instance();

private:
    /// Object constructor.
    msvQVTKButtonsManager(QObject *parent = 0);
};

/////////////////////////////////////////////////////////////
// Inline methods
/////////////////////////////////////////////////////////////

#endif // msvQVTKButtonsManager_H
