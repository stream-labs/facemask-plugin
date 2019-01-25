#include "Config.hpp"

extern __declspec(dllexport) int test();
extern __declspec(dllexport) bool obs_module_load1(void);
extern __declspec(dllexport) void obs_module_unload1(void);
extern __declspec(dllexport) const char* obs_module_name1();
extern __declspec(dllexport) const char* obs_module_description1();
extern __declspec(dllexport) void obs_module_set_pointer1(obs_module_t *module);
extern __declspec(dllexport) uint32_t obs_module_ver1(void);
extern __declspec(dllexport) obs_module_t *obs_current_module1(void);
extern __declspec(dllexport) const char *obs_module_text1(const char *val);
extern __declspec(dllexport) bool obs_module_get_string1(const char *val, const char **out);
extern __declspec(dllexport) void obs_module_set_locale1(const char *locale);
extern __declspec(dllexport) void obs_module_free_locale1(void);