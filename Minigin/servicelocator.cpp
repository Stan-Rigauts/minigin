#include "servicelocator.h"

std::unique_ptr<sound_system> servicelocator::_ss_instance = std::make_unique<null_sound_system>();