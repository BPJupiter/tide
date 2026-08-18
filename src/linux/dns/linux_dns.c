
//////////////////////////////////
// @per_os_impl System DNS Info

internal String8_List dns_get_local_nameservers(Arena *arena)
{
    String8_List result = {0};

    FILE *file = fopen("/etc/resolv.conf", "r");
    if (!file) {
        return result;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *ptr = line;

        while (*ptr && isspace((unsigned char)*ptr)) ptr++;

        if (*ptr == '#' || *ptr == ';') {
            continue;
        }

        const char *prefix = "nameserver";
        size_t prefix_len = strlen(prefix);

        if (strncmp(ptr, prefix, prefix_len) == 0 && isspace((unsigned char)ptr[prefix_len])) {
            ptr += prefix_len;

            while (*ptr && isspace((unsigned char)*ptr)) ptr++;

            char *ip_start = ptr;
            while (*ptr && !isspace((unsigned char)*ptr)) ptr++;
            *ptr = '\0';

            if (*ip_start != '\0') {
                str8_list_pushf(arena, &result, "%s", ip_start);
            }
        }
    }

    fclose(file);
    return result;
}
