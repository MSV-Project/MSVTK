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

#include "msvQVTKButtonsInterface.h"

msvQVTKButtonsInterface::msvQVTKButtonsInterface(QObject *parent) : QObject(parent), m_ShowButton(false), m_ShowLabel(true), m_FlyTo(true), m_OnCenter(false) {

}

msvQVTKButtonsInterface::~msvQVTKButtonsInterface() {

}