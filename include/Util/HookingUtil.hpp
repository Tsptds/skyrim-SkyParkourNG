#pragma once

namespace Hooking {

    template <typename Ret, typename... Args>
    bool InstallVFuncHook(REL::Relocation<uintptr_t> &vtable, std::size_t offset, REL::Relocation<Ret (*)(Args...)> &original,
                          Ret (*replacement)(Args...)) {
        original = vtable.write_vfunc(offset, replacement);
        return original.address() != 0;
    }

}  // namespace Hooking