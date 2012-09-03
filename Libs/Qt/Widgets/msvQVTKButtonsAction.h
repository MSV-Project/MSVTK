/*
 *  msvQVTKButtonsAction.h
 *  
 *
 *  Created by Alberto Losi on 01/08/12.
 *  Copyright 2009 B3C.s All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#ifndef msvQVTKButtonsAction_H
#define msvQVTKButtonsAction_H

// Includes list

#include "msvQtWidgetsExport.h"
#include <vtkRenderer.h>


/**
Class name: msvQVTKButtonsAction
Interface abstract class for buttons actions
*/

class MSV_QT_WIDGETS_EXPORT msvQVTKButtonsAction 
{

public:
    /// Object constructor.
    msvQVTKButtonsAction();

    /// Animate the camera to zoom on the passed bounding box.
    virtual void execute(vtkRenderer *renderer, double bounds[6], int numberOfSteps = 120)=0;

    /// Object destructor.
    virtual ~msvQVTKButtonsAction();

};

#endif // msvQVTKButtonsAction_H
