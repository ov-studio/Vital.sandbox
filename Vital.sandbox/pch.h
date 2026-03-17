/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: pch.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Precompiled Header
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <mutex>
#include <atomic>
#include <chrono>
#include <cctype>
#include <functional>
#include <filesystem>
#include <fstream>
#include <thread>
#include <future>
#include <exception>
#include <queue>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <utility>
#include <string>
#include <string_view>
#include <sstream>
#include <memory>
#include <cstdint>
#include <stdexcept>
#include <initializer_list>

#if defined(Vital_SDK_MACOS) || defined(Vital_SDK_LINUX)
#include <termios.h>
#include <unistd.h>
#endif