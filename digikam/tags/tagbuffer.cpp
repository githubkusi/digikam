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

// digikam includes
#include "databasechangesets.h"
#include "tagscache.h"

#include "tagbuffer.h"

using namespace std;

namespace Digikam
{
    
// TagBuffer::tagbuffer() {
//     state = state::stateapplybuffertonext;
// }   
    
void TagBuffer::eventFired(Event e)
{
    switch(state)
    {
        case stateApplyBufferToCurrent:
            switch(e)
            {
                case eventApplyTagBuffer: e11b();break;
                case eventSelectPicture:  e11a();break;
                case eventTagging:        e12();state=stateChangeBuffer;;
            }            
            break;
            
        case stateChangeBuffer:
            switch(e)
            {
                case eventSelectPicture:  e21();state=stateApplyBufferToCurrent;break;
                case eventApplyTagBuffer: e23();state=stateApplyBufferToNext;break;
                case eventTagging:        e22();break;
            }
            break;
            
        case stateApplyBufferToNext:
            switch(e)
            {
                case eventSelectPicture:  e31();state=stateApplyBufferToCurrent;break;
                case eventApplyTagBuffer: e33();break;
                case eventTagging:        e32();state=stateChangeBuffer;break;
            }
            break;
    }    
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
    emit signalNextItem();
}

void TagBuffer::e12()
{
    cout << "e12" << endl;
    tagBuffer.clear();
    
    if (curTagId != emptyTag)
        tagBuffer.insert(curTagId);
    
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
    emit signalNextItem();
    applyBuffer();
}


void TagBuffer::e32()
{
    cout << "e32" << endl;
    
    tagBuffer.clear();    
    if (curTagId != emptyTag)
        tagBuffer.insert(curTagId);    
}

void TagBuffer::e33()
{
    cout << "e33" << endl;
    emit signalNextItem();
    applyBuffer();
}
//------actions--------------------------------------

void TagBuffer::applyBuffer()
{
    cout << "apply buffer: " << tagBuffer.count() << "tags" << endl;
    emit signalApplyBuffer(tagBuffer);
}

//------event mapping
void TagBuffer::slotEventSelectPicture()
{
    eventFired(eventSelectPicture);
}

void TagBuffer::slotEventTagging(ImageTagChangeset changeset)
{   
    TagsCache * tc = TagsCache::instance();
    
    curTagId = emptyTag;
    
    if (false && (changeset.operation()==ImageTagChangeset::Added || changeset.operation()==ImageTagChangeset::Removed))
    {
        cout << "images: ";
        for (int i=0;i<changeset.ids().count();i++)        
        {
            cout << changeset.ids().at(i);
        }
        cout << endl;
        
        if (changeset.operation()==ImageTagChangeset::Added)
            cout << "added:";
        else
            cout << "removed:";
        
        for(int i=0;i<changeset.tags().count();i++)
        {
            int tagId = changeset.tags().at(i);
            cout << " " << tagId;
            if (tc->isInternalTag(tagId))
            {
                cout << "(internaltag) ";
            }
            else
            {
                //the real thing of this fcn
                curTagId = tagId;
            }
        }
        cout << endl;
        
        cout << "slotEventTagging" << endl;
        eventFired(eventTagging);
    }
    else
    {
        //non-debug code
        if (changeset.operation()==ImageTagChangeset::Added)
        {
            QList<int> t = tc->publicTags(changeset.tags());
            
            //if this doesn't hold true, we need to declare curTagId as QList instead of string
            Q_ASSERT(t.count()<=1);
            
            for(int i=0;i<t.count();i++)
            {
                curTagId = t.at(i);
            }
            
            if (curTagId != emptyTag)
                eventFired(eventTagging);
        }
    }
    
}


TagBuffer::TagBuffer()
{
    //initial state
    state = stateApplyBufferToCurrent;        
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