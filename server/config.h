#ifndef CONFIG_H
#define CONFIG_H

#define FORMAT_INT "%d"
#define FORMAT_FLOAT "%f"
#define FORMAT_STRING "%s"

struct config_map_entry {
	char *entry_name;
	char *modifier;
	void *destination;
} __attribute__ ((packed));

#endif /* !CONFIG_H */