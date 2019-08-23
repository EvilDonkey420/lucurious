/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Lucurious Labs
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <lucom.h>
#include <wlu/utils/log.h>
#include <wlu/shader/shade.h>

#include <shaderc/shaderc.h>

static const unsigned int shader_map_table[] = {
  [0x00000000] = shaderc_glsl_infer_from_source,
  [0x00000001] = shaderc_glsl_vertex_shader,
  [0x00000002] = shaderc_glsl_tess_control_shader,
  [0x00000004] = shaderc_glsl_tess_evaluation_shader,
  [0x00000008] = shaderc_glsl_geometry_shader,
  [0x00000010] = shaderc_glsl_fragment_shader,
  [0x00000020] = shaderc_glsl_compute_shader,
};

void wlu_shaderc_release(
  shaderc_compiler_t compiler,
  shaderc_compilation_result_t result,
  shaderc_compile_options_t options
) {
  if (options) shaderc_compile_options_release(options);
  if (result) shaderc_result_release(result);
  if (compiler) shaderc_compiler_release(compiler);
}

void wlu_freeup_shi(wlu_shader_info *shi) {
  if (shi)
    wlu_shaderc_release(NULL, shi->result, NULL);
}

/* Returns GLSL shader source text after preprocessing */
wlu_shader_info wlu_preprocess_shader(
  unsigned int kind,
  const char *source,
  const char *input_file_name,
  const char *entry_point_name
) {

  wlu_shader_info shinfo = {NULL, 0, NULL};

  const char *name = "MY_DEFINE";
  const char *value = "1";

  shaderc_compiler_t compiler = shaderc_compiler_initialize();
  shaderc_compile_options_t options = shaderc_compile_options_initialize();
  shaderc_compilation_result_t result = 0;

  shaderc_compile_options_add_macro_definition(options, name, strlen(name), value, strlen(value));

  result = shaderc_compile_into_preprocessed_text(
           compiler, source, strlen(source),
           shader_map_table[kind], input_file_name,
           entry_point_name, options);

  if (!result) {
    wlu_log_me(WLU_DANGER, "[x] shaderc_compile_into_preprocessed_text failed, ERROR code: %d", result);
    wlu_shaderc_release(compiler, NULL, options);
    return shinfo;
  }

  if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) {
    wlu_log_me(WLU_DANGER, "[x]\n%s", shaderc_result_get_error_message(result));
    wlu_shaderc_release(compiler, result, options);
    return shinfo;
  }

  shinfo.byte_size = shaderc_result_get_length(result);
  shinfo.bytes = shaderc_result_get_bytes(result);
  shinfo.result = result;

  wlu_log_me(WLU_WARNING, "SPIRV BYTES: %ld - %ld bytes", shinfo.bytes, shinfo.byte_size);

  wlu_shaderc_release(compiler, NULL, options);

  return shinfo;
}

/* Compiles a shader to SPIR-V assembly. Returns the assembly text as a string. */
wlu_shader_info wlu_compile_to_assembly(
  unsigned int kind,
  const char *source,
  const char *input_file_name,
  const char *entry_point_name
) {

  wlu_shader_info shinfo = {NULL, 0, NULL};

  const char *name = "MY_DEFINE";
  const char *value = "1";

  shaderc_compiler_t compiler = shaderc_compiler_initialize();
  shaderc_compile_options_t options = shaderc_compile_options_initialize();
  shaderc_compilation_result_t result = 0;

  shaderc_compile_options_add_macro_definition(options, name, strlen(name), value, strlen(value));

  shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level_size);

  result = shaderc_compile_into_spv_assembly(
           compiler, source, strlen(source),
           shader_map_table[kind], input_file_name,
           entry_point_name, options);

  if (!result) {
    wlu_log_me(WLU_DANGER, "[x] shaderc_compile_into_spv_assembly failed, ERROR code: %d", result);
    wlu_shaderc_release(compiler, NULL, options);
    return shinfo;
  }

  if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) {
    wlu_log_me(WLU_DANGER, "[x]\n%s", shaderc_result_get_error_message(result));
    wlu_shaderc_release(compiler, NULL, options);
    return shinfo;
  }

  shinfo.byte_size = shaderc_result_get_length(result);
  shinfo.bytes = shaderc_result_get_bytes(result);
  shinfo.result = result;

  wlu_log_me(WLU_WARNING, "SPIRV BYTES: %ld - %ld bytes", shinfo.bytes, shinfo.byte_size);

  wlu_shaderc_release(compiler, NULL, options);

  return shinfo;
}

/* Compiles a shader to a SPIR-V binary */
wlu_shader_info wlu_compile_to_spirv(
  unsigned int kind,
  const char *source,
  const char *input_file_name,
  const char *entry_point_name
) {

  wlu_shader_info shinfo = {NULL, 0, NULL};

  const char *name = "MY_DEFINE";
  const char *value = "1";

  shaderc_compiler_t compiler = shaderc_compiler_initialize();
  shaderc_compile_options_t options = shaderc_compile_options_initialize();
  shaderc_compilation_result_t result = 0;

  shaderc_compile_options_add_macro_definition(options, name, strlen(name), value, strlen(value));

  shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level_size);

  result = shaderc_compile_into_spv(
           compiler, source, strlen(source),
           shader_map_table[kind], input_file_name,
           entry_point_name, options);

  if (!result) {
    wlu_log_me(WLU_DANGER, "[x] shaderc_compile_into_spv failed, ERROR code: %d", result);
    wlu_shaderc_release(compiler, NULL, options);
    return shinfo;
  }

  if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) {
    wlu_log_me(WLU_DANGER, "%s", shaderc_result_get_error_message(result));
    wlu_shaderc_release(compiler, result, options);
    return shinfo;
  }

  shinfo.byte_size = shaderc_result_get_length(result);
  shinfo.bytes = shaderc_result_get_bytes(result);
  shinfo.result = result;

  wlu_log_me(WLU_WARNING, "SPIRV BYTES: %ld - %ld bytes", shinfo.bytes, shinfo.byte_size);

  wlu_shaderc_release(compiler, NULL, options);

  return shinfo;
}
