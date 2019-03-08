#include "CodeGen_WebAssembly.h"
#include "Util.h"
#include "LLVM_Headers.h"

namespace Halide {
namespace Internal {

using std::vector;
using std::string;

using namespace llvm;

CodeGen_WebAssembly::CodeGen_WebAssembly(Target t) : CodeGen_Posix(t) {
    #if !(WITH_WEBASSEMBLY)
    user_error << "llvm build not configured with WebAssembly target enabled.\n";
    #endif
    user_assert(llvm_WebAssembly_enabled) << "llvm build not configured with WebAssembly target enabled.\n";
    user_assert(target.bits == 32) << "Only wasm32 is supported.";
}

string CodeGen_WebAssembly::mcpu() const {
    return "";
}

string CodeGen_WebAssembly::mattrs() const {
    // TODO: it's not clear if we *require* bulk_memory but
    // it apparently goes hand in hand with enabling threads?

    // TODO: we probably don't *require* sign-ext but our codegen
    // is likely much more efficient with it.

    // TODO: do we want/need nontrapping-fptoint?
    // https://github.com/WebAssembly/nontrapping-float-to-int-conversions/blob/master/proposals/nontrapping-float-to-int-conversion/Overview.md

    string s = "+sign-ext";

    if (target.os != Target::EmscriptenThreadlessRT) {
        // In theory, atomics and threads are separate features in wasm,
        // but Emscripten seems to tie them together; if we emit atomics
        // without threads, Emscripten will refuse to link.
        s += ",+atomics";
    }


    // craters compiling our runtime under current trunk
    // s += ",+bulk-memory"

    // s += ",+simd128";

    // TODO: Emscripten doesn't seem to be able to validate wasm that contains this yet.
    // Does it generate these? Is there a flag to control it?
    // s += ",+nontrapping-fptoint";

    return s;
}

bool CodeGen_WebAssembly::use_soft_float_abi() const {
    return false;
}

int CodeGen_WebAssembly::native_vector_bits() const {
    return 128;
}

}}
