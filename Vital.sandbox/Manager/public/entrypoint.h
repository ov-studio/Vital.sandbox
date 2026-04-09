/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Manager: entrypoint.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Entrypoint
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Engine/public/network.h>
#include <Vital.sandbox/Engine/public/asset.h>
#include <Vital.sandbox/Engine/public/canvas.h>
#include <Vital.sandbox/Engine/public/model.h>
#include <Vital.sandbox/Engine/public/webview.h>
#include <Vital.sandbox/Engine/public/font.h>
#include <Vital.sandbox/Engine/public/texture.h>
#include <Vital.sandbox/Engine/public/rendertarget.h>
#include <Vital.sandbox/Engine/public/console.h>
#include <Vital.sandbox/Engine/public/resource.h>
#include <Vital.sandbox/Manager/public/sandbox.h>


///////////
// APIs //
///////////

void initialize_gdextension_types();
void uninitialize_gdextension_types();
void initialize_vital_events();