#pragma once
#pragma message("Precompiling headers.\n")

#include <stdio.h>
#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <map>
#include <cstdarg>
#include <cassert>
#include <cstdlib>
#include <cstddef>

// SIMD
//#include <emmintrin.h>
//#include <smmintrin.h>
//#include <directxmath.h>
#include <xmmintrin.h>

#include <core/utils.h>
#include <core/rmem.h>
#include <core/BinaryIO.h>

#include <core/math/mathlib.h>
#include <core/math/float16.h>
#include <core/math/vector.h>
#include <core/math/Vector2D.h>
#include <core/math/vector4d.h>
#include <core/math/color32.h>
#include <core/math/compressed_vector.h>
#include <core/math/matrix3x4.h>