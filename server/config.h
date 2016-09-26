#ifndef CONFIG_H
#define CONFIG_H

#define FORMAT_INT "%d"
#define FORMAT_FLOAT "%f"
#define FORMAT_STRING "%s"
#define FORMAT_DOUBLE "%lf"

struct config_map_entry {
    char *entry_name;
    char *modifier;
    void *destination;
};

#endif /* !CONFIG_H */