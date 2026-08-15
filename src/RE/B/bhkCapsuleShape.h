/* Shamelessly copied from https://github.com/ersh1/Precision/tree/main */

// MIT License

// Copyright (c) 2023 Ersh

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

namespace RE {

    class bhkSphereRepShape : public bhkShape {};
    class bhkConvexShape : public bhkSphereRepShape {};
    class bhkCapsuleShape : public bhkConvexShape {};
    static_assert(sizeof(bhkCapsuleShape) == 0x28);
}  // namespace RE

namespace Offsets {
    typedef void (*tbhkCapsuleShape_ctor)(RE::bhkCapsuleShape *a_this);
    static REL::Relocation<tbhkCapsuleShape_ctor> bhkCapsuleShape_ctor{RELOCATION_ID(76929, 78804)};  // DE2520, E24030

    typedef void (*tbhkCapsuleShape_SetSize)(RE::bhkCapsuleShape *a_this, RE::hkVector4 &a_vertexA, RE::hkVector4 &a_vertexB,
                                             float a_radius);
    static REL::Relocation<tbhkCapsuleShape_SetSize> bhkCapsuleShape_SetSize{RELOCATION_ID(76925, 78800)};  // DE20B0, E23BB0

    typedef RE::hkpAllCdPointCollector *(__fastcall *tGetAllCdPointCollector)(bool a1, bool a2);
    static REL::Relocation<tGetAllCdPointCollector> GetAllCdPointCollector{RELOCATION_ID(25397, 25925)};  // 39FA60, 3B63D0
}  // namespace Offsets