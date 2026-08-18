/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: easing.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Easing Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/log.h>


//////////////////////////
// Vital: Tool: Easing //
//////////////////////////

namespace Vital::Tool::Easing {
    inline constexpr double PI = 3.14159265358979323846;

    inline double linear(double progress) { return progress; }

    inline double sine_in(double progress)     { return 1.0 - std::cos((progress*PI)*0.5); }
    inline double sine_out(double progress)    { return std::sin((progress*PI)*0.5); }
    inline double sine_in_out(double progress) { return -(std::cos(PI*progress) - 1.0)*0.5; }

    inline double quad_in(double progress)     { return progress*progress; }
    inline double quad_out(double progress)    { return 1.0 - (1.0-progress)*(1.0-progress); }
    inline double quad_in_out(double progress) { return progress < 0.5 ? 2.0*progress*progress : 1.0 - std::pow(-2.0*progress + 2.0, 2)*0.5; }

    inline double cubic_in(double progress)     { return progress*progress*progress; }
    inline double cubic_out(double progress)    { return 1.0 - std::pow(1.0-progress, 3); }
    inline double cubic_in_out(double progress) { return progress < 0.5 ? 4.0*progress*progress*progress : 1.0 - std::pow(-2.0*progress + 2.0, 3)*0.5; }

    inline double quart_in(double progress)     { return progress*progress*progress*progress; }
    inline double quart_out(double progress)    { return 1.0 - std::pow(1.0-progress, 4); }
    inline double quart_in_out(double progress) { return progress < 0.5 ? 8.0*progress*progress*progress*progress : 1.0 - std::pow(-2.0*progress + 2.0, 4)*0.5; }

    inline double quint_in(double progress)     { return progress*progress*progress*progress*progress; }
    inline double quint_out(double progress)    { return 1.0 - std::pow(1.0-progress, 5); }
    inline double quint_in_out(double progress) { return progress < 0.5 ? 16.0*progress*progress*progress*progress*progress : 1.0 - std::pow(-2.0*progress + 2.0, 5)*0.5; }

    inline double expo_in(double progress)  { return progress <= 0.0 ? 0.0 : std::pow(2.0, 10.0*progress - 10.0); }
    inline double expo_out(double progress) { return progress >= 1.0 ? 1.0 : 1.0 - std::pow(2.0, -10.0*progress); }
    inline double expo_in_out(double progress) {
        if (progress <= 0.0) return 0.0;
        if (progress >= 1.0) return 1.0;
        return progress < 0.5 ? std::pow(2.0, 20.0*progress - 10.0)*0.5 : (2.0 - std::pow(2.0, -20.0*progress + 10.0))*0.5;
    }

    inline double circ_in(double progress)  { return 1.0 - std::sqrt(1.0 - std::pow(progress, 2)); }
    inline double circ_out(double progress) { return std::sqrt(1.0 - std::pow(progress - 1.0, 2)); }
    inline double circ_in_out(double progress) {
        return progress < 0.5
            ? (1.0 - std::sqrt(1.0 - std::pow(2.0*progress, 2)))*0.5
            : (std::sqrt(1.0 - std::pow(-2.0*progress + 2.0, 2)) + 1.0)*0.5;
    }

    inline double back_in(double progress, double overshoot = 1.70158) {
        double c1 = overshoot;
        double c3 = c1 + 1.0;
        return c3*progress*progress*progress - c1*progress*progress;
    }
    inline double back_out(double progress, double overshoot = 1.70158) {
        double c1 = overshoot;
        double c3 = c1 + 1.0;
        return 1.0 + c3*std::pow(progress - 1.0, 3) + c1*std::pow(progress - 1.0, 2);
    }
    inline double back_in_out(double progress, double overshoot = 1.70158) {
        double c1 = overshoot;
        double c2 = c1*1.525;
        return progress < 0.5
            ? (std::pow(2.0*progress, 2)*((c2 + 1.0)*2.0*progress - c2))*0.5
            : (std::pow(2.0*progress - 2.0, 2)*((c2 + 1.0)*(progress*2.0 - 2.0) + c2) + 2.0)*0.5;
    }

    inline double elastic_in(double progress, double amplitude = 1.0, double period = 0.3) {
        if (progress <= 0.0) return 0.0;
        if (progress >= 1.0) return 1.0;
        double a = std::max(amplitude, 1.0);
        double s = (a > 1.0) ? (period/(2.0*PI))*std::asin(1.0/a) : period*0.25;
        progress -= 1.0;
        return -(a*std::pow(2.0, 10.0*progress)*std::sin((progress - s)*(2.0*PI)/period));
    }
    inline double elastic_out(double progress, double amplitude = 1.0, double period = 0.3) {
        if (progress <= 0.0) return 0.0;
        if (progress >= 1.0) return 1.0;
        double a = std::max(amplitude, 1.0);
        double s = (a > 1.0) ? (period/(2.0*PI))*std::asin(1.0/a) : period*0.25;
        return a*std::pow(2.0, -10.0*progress)*std::sin((progress - s)*(2.0*PI)/period) + 1.0;
    }
    inline double elastic_in_out(double progress, double amplitude = 1.0, double period = 0.45) {
        if (progress <= 0.0) return 0.0;
        if (progress >= 1.0) return 1.0;
        double a = std::max(amplitude, 1.0);
        double s = (a > 1.0) ? (period/(2.0*PI))*std::asin(1.0/a) : period*0.25;
        progress = progress*2.0;
        if (progress < 1.0) {
            progress -= 1.0;
            return -0.5*(a*std::pow(2.0, 10.0*progress)*std::sin((progress - s)*(2.0*PI)/period));
        }
        progress -= 1.0;
        return a*std::pow(2.0, -10.0*progress)*std::sin((progress - s)*(2.0*PI)/period)*0.5 + 1.0;
    }

    inline double bounce_out(double progress) {
        const double n1 = 7.5625;
        const double d1 = 2.75;
        if (progress < 1.0/d1) return n1*progress*progress;
        if (progress < 2.0/d1) { progress -= 1.5/d1; return n1*progress*progress + 0.75; }
        if (progress < 2.5/d1) { progress -= 2.25/d1; return n1*progress*progress + 0.9375; }
        progress -= 2.625/d1;
        return n1*progress*progress + 0.984375;
    }
    inline double bounce_in(double progress) { return 1.0 - bounce_out(1.0 - progress); }
    inline double bounce_in_out(double progress) {
        return progress < 0.5
            ? (1.0 - bounce_out(1.0 - 2.0*progress))*0.5
            : (1.0 + bounce_out(2.0*progress - 1.0))*0.5;
    }
}
