#include "GDSystemInterface.hpp"
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/translation.hpp>
#include <godot_cpp/classes/translation_server.hpp>
#include "Conversion.h"

using namespace gdrml;
using namespace godot;

GDSystemInterface::GDSystemInterface()
{
}

double GDSystemInterface::GetElapsedTime()
{
    // oops this is seconds
    return static_cast<double>(Time::get_singleton()->get_ticks_msec()) / 1000.0;
}

bool GDSystemInterface::LogMessage(Rml::Log::Type type, const Rml::String& message)
{
    switch (type)
    {
    case Rml::Log::Type::LT_DEBUG:
        if (OS::get_singleton()->is_debug_build())
        {
            UtilityFunctions::print_rich("[color=gray]RmlUI: ", Conversion::stringToGodot(message), "[/color]");
        }
        return true;

    case Rml::Log::Type::LT_ASSERT:
        UtilityFunctions::push_error("RmlUI: ", Conversion::stringToGodot(message));
        return true;
        
    case Rml::Log::Type::LT_ERROR:
        UtilityFunctions::push_error("RmlUI: ", Conversion::stringToGodot(message));
        return false;

    case Rml::Log::Type::LT_WARNING:
        UtilityFunctions::push_warning("RmlUI: ", Conversion::stringToGodot(message));
        return true;

    case Rml::Log::Type::LT_ALWAYS:
    case Rml::Log::Type::LT_INFO:
    case Rml::Log::Type::LT_MAX:
        UtilityFunctions::print("RmlUI: ", Conversion::stringToGodot(message));
        return true;
    }
}

int GDSystemInterface::TranslateString(Rml::String& translated, const Rml::String& input)
{
    String orig = Conversion::stringToGodot(input);
    String gdTrans = TranslationServer::get_singleton()->translate(orig);

    translated = Conversion::stringToStd(gdTrans);

    if (orig == gdTrans)
    {
        return 0;
    }
    return 1;
}
