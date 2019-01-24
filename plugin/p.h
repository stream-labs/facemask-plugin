#include "Config.hpp"

extern __declspec(dllexport) int test();
extern __declspec(dllexport) bool obs_module_load2(void);
extern __declspec(dllexport) void obs_module_unload2(void);
extern __declspec(dllexport) const char* obs_module_name2();
extern __declspec(dllexport) const char* obs_module_description2();
extern __declspec(dllexport) void obs_module_set_pointer2(obs_module_t *module);
extern __declspec(dllexport) uint32_t obs_module_ver2(void);
extern __declspec(dllexport) obs_module_t *obs_current_module2(void);
extern __declspec(dllexport) const char *obs_module_text2(const char *val);
extern __declspec(dllexport) bool obs_module_get_string2(const char *val, const char **out);
extern __declspec(dllexport) void obs_module_set_locale2(const char *locale);
extern __declspec(dllexport) void obs_module_free_locale2(void);