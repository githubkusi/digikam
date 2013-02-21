/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-12
 * Description : dialog to edit and create digiKam Tags
 *
 * Copyright (C) 2013 by Markus Leuthold <kusi -at titlis dot org>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */


#ifndef TAGBUFFER_H
#define TAGBUFFER_H

//QT includes
#include <QObject>
#include <QSet>

#include "databasechangesets.h"


namespace Digikam
{
    
class TagBuffer : public QObject
{
    Q_OBJECT
    
public:
    enum Event
    {
        eventSelectPicture,
        eventTagging,
        eventApplyTagBuffer
    };
    
    enum State
    {
        stateApplyBufferToCurrent,
        stateChangeBuffer,
        stateApplyBufferToNext
    };
    
    TagBuffer();
    virtual ~TagBuffer() {}; 
    
    void eventFired(Event);
    QString getTagBufferString();
    
public Q_SLOTS:
    void slotEventSelectPicture();
    void slotEventTagging(ImageTagChangeset);
    
Q_SIGNALS:
    void signalNextItem();
    void signalApplyBuffer(QList<int>);
    
protected:
       
    State state;
    
    //events
    void e11b();    
    void e11a();
    void e12();
    void e21();
    void e22();
    void e23();
    void e31();
    void e32();
    void e33();
    
    //actions
    void applyBuffer();
        
    QSet<int> tagBuffer;    //tagIds of 
    QSet<int> curTagBuffer; //tagIds which belong to current commit to database
                             //eg user toggles several tags before pressing "apply"
                             
    QList<int> curTaggedImages; //tag signal emits
    int curTagId;
    const int emptyTag = -1;
   
};

}
#endif // TAGBUFFER_H
