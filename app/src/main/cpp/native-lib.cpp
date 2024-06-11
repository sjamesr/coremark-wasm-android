#include <android/log.h>
#include <jni.h>
#include <string>
#include <cstdint>
#include "third_party/wasm-micro-runtime/core/iwasm/include/wasm_export.h"
#include "third_party/wasm-micro-runtime/core/iwasm/common/wasm_runtime_common.h"

extern "C" int my_vprintf(const char *fmt, va_list ap)
{
    /* or output to string buffer and print the string */
    char buf[4096];
    int res = vsnprintf(buf, sizeof(buf), fmt, ap);
    __android_log_write(ANDROID_LOG_INFO, "CoreMarkWASM", buf);
    return res;
}

void my_log(uint32_t log_level, const char *file, int line, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    __android_log_vprint(ANDROID_LOG_INFO, "CoreMarkWASM", fmt, args);
    va_end(args);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myapplication_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */, jbyteArray image) {
    jbyte* bufferPtr = env->GetByteArrayElements(image, nullptr);
    jsize len = env->GetArrayLength(image);

    char *error_buf[128];
    wasm_module_t module;
    wasm_module_inst_t module_inst;
    wasm_exec_env_t exec_env;
    uint32_t size, stack_size = 8092, heap_size = 8092;

    /* initialize the wasm runtime by default configurations */
    wasm_runtime_init();

    /* read WASM file into a memory buffer */

    /* add line below if we want to export native functions to WASM app */
//    wasm_runtime_register_natives();

    /* parse the WASM file from buffer and create a WASM module */
    module = wasm_runtime_load(reinterpret_cast<uint8_t *>(bufferPtr),
                               len, (char *) error_buf,
                               sizeof(error_buf));

    /* create an instance of the WASM module (WASM linear memory is ready) */
    module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                           (char *) error_buf, sizeof(error_buf));

    wasm_function_inst_t start_func = wasm_runtime_lookup_function(module_inst, "_start");

    exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);

    uint32_t argv[0];

    if (wasm_runtime_call_wasm(exec_env, start_func, 0, argv)) {
        printf("Success!");
    } else {
        printf("%s\n", wasm_runtime_get_exception(module_inst));
    }

    return env->NewStringUTF("Hello");
}