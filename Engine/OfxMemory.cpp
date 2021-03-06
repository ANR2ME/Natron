/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron <http://www.natron.fr/>,
 * Copyright (C) 2013-2017 INRIA and Alexandre Gauthier-Foichat
 *
 * Natron is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Natron is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Natron.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
 * ***** END LICENSE BLOCK ***** */

// ***** BEGIN PYTHON BLOCK *****
// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>
// ***** END PYTHON BLOCK *****

#include "OfxMemory.h"

#include <cassert>
#include <stdexcept>

CLANG_DIAG_OFF(deprecated)
#include <QtCore/QMutex>
CLANG_DIAG_ON(deprecated)

#include "Engine/EffectInstance.h"
#include "Engine/PluginMemory.h"

NATRON_NAMESPACE_ENTER;

OfxMemory::OfxMemory(const EffectInstancePtr& effect)
    : OFX::Host::Memory::Instance()
    , _lock()
    , _lockedCount(0)
    , _memory( new PluginMemory(effect) )
{
    
}

OfxMemory::~OfxMemory()
{
}

void*
OfxMemory::getPtr()
{
    return _memory->getPtr();
}

bool
OfxMemory::alloc(size_t nBytes)
{

    QMutexLocker l(&_lock);

    if (_lockedCount) {
        return false;
    }

    PluginMemAllocateMemoryArgs args(nBytes);
    try {
        _memory->allocateMemory(args);
    } catch (const std::bad_alloc &) {
        return false;
    }

    return true;
}

void
OfxMemory::freeMem()
{
    _memory->deallocateMemory();
}

void
OfxMemory::lock()
{
    QMutexLocker l(&_lock);

    ++_lockedCount;
}

void
OfxMemory::unlock()
{
    QMutexLocker l(&_lock);

    // http://openfx.sourceforge.net/Documentation/1.3/ofxProgrammingReference.html#OfxImageEffectSuiteV1_imageMemoryUnlock
    // "Also note, if you unlock a completely unlocked handle, it has no effect (ie: the lock count can't be negative)."
    if (_lockedCount > 0) {
        --_lockedCount;
    }

}

NATRON_NAMESPACE_EXIT;
