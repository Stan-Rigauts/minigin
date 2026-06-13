#include "servicelocator.h"

std::unique_ptr<sound_system> servicelocator::soundSystemInstance = std::make_unique<null_sound_system>();