#include "ISink.h"

#include <chrono>
#include <format> 
#include <string>

using namespace chrono;

string ISink::Get_TimeStamp() const {
    auto now = system_clock::now();
    auto seconds = floor<chrono::seconds>(now);
    return format("{:%M:%S}", seconds);
}