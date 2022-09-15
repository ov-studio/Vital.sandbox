/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: index.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Module Initializer
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#include <windows.h>


///////////////////////
// Namespace: Vital //
///////////////////////

namespace Vital {
    unsigned int getTick() {
        return GetTickCount()
    }
}