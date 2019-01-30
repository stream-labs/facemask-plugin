
typedef bool(*obs_module_load_fun)(void);
typedef void(*obs_module_unload_fun)(void);
typedef const char* (*obs_module_name_fun)();
typedef const char* (*obs_module_description_fun)();
typedef void(*obs_module_set_pointer_fun)(obs_module_t *module);
typedef uint32_t(*obs_module_ver_fun)(void);
typedef obs_module_t *(*obs_current_module_fun)(void);
typedef const char *(*obs_module_text_fun)(const char *val);
typedef bool(*obs_module_get_string_fun)(const char *val, const char **out);
typedef void(*obs_module_set_locale_fun)(const char *locale);
typedef void(*obs_module_free_locale_fun)(void);