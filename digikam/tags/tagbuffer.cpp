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

#include "tagbuffer.moc"

#include <iostream>
#include <boost/graph/graph_concepts.hpp>

//kde includes
#include <kdebug.h>

// digikam includes
#include "databasechangesets.h"
#include "tagscache.h"

#include "tagbuffer.h"

using namespace std;

namespace Digikam
{
    
TagBuffer::TagBuffer()
{
    lockTagEvent = false;
    lockSelectEvent = false;
    state = stateApplyBufferToCurrent;        
}

void TagBuffer::eventFired(Event e)
{
    State oldState = state;
    
    switch(state)
    {
        case stateApplyBufferToCurrent:
            switch(e)
            {
                case eventApplyTagBuffer: e11b();break;
                case eventSelectPicture:  e11a();break;
                case eventTagging:        e12();state=stateChangeBuffer;break;
                case eventTaggedAndNext:  e14();state=stateWaitForTag;break;
            }            
            break;
            
        case stateChangeBuffer:
            switch(e)
            {
                case eventSelectPicture:  e21();state=stateApplyBufferToCurrent;break;
                case eventApplyTagBuffer: e23();state=stateApplyBufferToNext;break;
                case eventTagging:        e22();break;
                default:kError() << "not possible according to state machine";
            }
            break;
            
        case stateApplyBufferToNext:
            switch(e)
            {
                case eventSelectPicture:  e31();state=stateApplyBufferToCurrent;break;
                case eventApplyTagBuffer: e33();break;
                case eventTagging:        e32();state=stateChangeBuffer;break;
                default:kError() << "not possible according to state machine";
            }
            break;
            
        case stateWaitForTag:
            switch (e)
            {
                case eventTagging: e41();state=stateApplyBufferToCurrent;break;
                default:kError() << "not possible according to state machine";
            }
    }
    
    if (oldState != state)
        cout << "new state " << state << endl;
}

//------events-------------------------------------

void TagBuffer::e11a()
{
    cout << "e11a" << endl;
    
}

void TagBuffer::e11b()
{
    cout << "e11b" << endl;
    applyBuffer();
    nextImage();
    cout << "exit e11b" << endl;
}

void TagBuffer::e11c()
{
    //shortcut of state1:e12->state2:e21->state1
    //tagging event will follow AFTER this event
    cout << "e11c" << endl;
    currentTagToNewBuffer();
    lockTagEvent = tagBuffer.count();
    getTagBufferString();
}

void TagBuffer::e14()
{
    cout << "e14" << endl;
    
   

}

void TagBuffer::e41()
{
    cout << "e41" << endl;
    currentTagToNewBuffer();
    getTagBufferString();
}


void TagBuffer::e12()
{
    cout << "e12" << endl;
    
    currentTagToNewBuffer();
    getTagBufferString();
}

void TagBuffer::e21()
{
    cout << "e21" << endl;
}

void TagBuffer::e31()
{
    cout << "e31" << endl;
}

void TagBuffer::e22()
{
    cout << "e22" << endl;
    
    if (curTagId != emptyTag)
        tagBuffer.insert(curTagId);
    
    getTagBufferString();
}

void TagBuffer::e23()
{
    cout << "e23" << endl;
    nextImage();
    applyBuffer();
    cout << "exit e23" << endl;
}


void TagBuffer::e32()
{
    cout << "e32" << endl;
    currentTagToNewBuffer();
}

void TagBuffer::e33()
{    
    cout << "e33" << endl;
    
    nextImage();
    applyBuffer();
}
//------actions--------------------------------------

void TagBuffer::applyBuffer()
{
    cout << "apply buffer: " << tagBuffer.count() << " tags" << endl;
    
    lockTagEvent = tagBuffer.count();
    emit signalApplyBuffer(tagBuffer.toList());
}

void TagBuffer::nextImage()
{
    lockSelectEvent = 1;
    emit signalNextItem();
}

void TagBuffer::currentTagToNewBuffer()
{
    tagBuffer.clear();    
    if (curTagId != emptyTag)
        tagBuffer.insert(curTagId);      
}


//------event mapping
void TagBuffer::slotEventSelectPicture()
{
    
    if (lockSelectEvent)
    {
        lockSelectEvent=0;
        return;
    }
        
    eventFired(eventSelectPicture);
}

void kusiDispChangeset(ImageTagChangeset changeset)
{
//     cout << "operation: " << changeset.operation() << endl;
//     cout << "#images: " << changeset.ids().count() << endl;
    
    cout << "images in changeset: ";
    for(int i=0;i<changeset.ids().count();i++)
    {
        cout << changeset.ids().at(i) << ", ";
    }
    cout << endl;
}


void TagBuffer::slotEventTagging(ImageTagChangeset changeset)
{   
    
    
//     kusiDispChangeset(changeset);
    TagsCache * tc = TagsCache::instance();
    
    curTagId = emptyTag;
    
    if (changeset.operation()==ImageTagChangeset::Added)
    {
        

        
        
        //Our own signalApplyBuffer will call again this function. Filter this case 
        //emitting our own signal is apparently not blocking
//         static QList<qlonglong> previousImageIds;
//         if (changeset.ids() == previousImageIds)
//         {
//             cout << "receive our own tagging signal, exit" << endl;
//             return;
//         }
        
        QList<int> t = tc->publicTags(changeset.tags());
        
        //if this doesn't hold true, we need to declare curTagId as QList instead of string
        Q_ASSERT(t.count()<=1);
        
        for(int i=0;i<t.count();i++)
        {
            curTagId = t.at(i);
        }
        
        if (curTagId != emptyTag) //curTagId is empty if a internal tag was applied
        {
            cout << "valid tagging event recieved" << endl;
            kusiDispChangeset(changeset);
            
            //don't react on our own select action
            if (lockTagEvent)
            {
                //This is our own event since the lock is set. Release lock to be
                //ready for a real tagEvent
                cout << "caught our own tag, skip. " << lockTagEvent << " to go" << endl;
                lockTagEvent--;
                return;
            }
            
            eventFired(eventTagging);
        }
    }
}

void TagBuffer::slotTaggedAndNext()
{
    cout << "taggedAndNext" << endl;
    //tagging event will follow only after eventTaggedAndNext. Since the logical order is not
    //preserved, omit the following tagging event and take care here
    eventFired(eventTaggedAndNext);
}


QString TagBuffer::getTagBufferString()
{
    QString s;
    TagsCache * tc = TagsCache::instance();
    for(int i=0;i<tagBuffer.count();i++)
    {
        int v = tagBuffer.values().at(i);
        s.append(tc->tagName(v));
        
        if (i<(tagBuffer.count()-1))
            s.append(";");
    }
    cout << "Tagbuffer: " << s.toLatin1().data() << endl;
    return s;
}


}