// --Scratch buffer

#ifndef MAX_BUFFER_SIZE
#define MAX_BUFFER_SIZE 1000000
#endif

struct CAD_Scratch_Buf { char str[MAX_BUFFER_SIZE]; size_t len;};
struct CAD_Scratch_Buf cad_scratch_buffer;

void sb_append_len(const char *string, size_t len) {
    if (len + cad_scratch_buffer.len > MAX_BUFFER_SIZE - 1)
        exit(-1);

    memcpy(cad_scratch_buffer.str + cad_scratch_buffer.len, string, len);
    cad_scratch_buffer.len += (size_t)len;
}

void sb_append(const char *string) {
    sb_append_len(string, strlen(string));
}

void sb_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    size_t available = MAX_BUFFER_SIZE - cad_scratch_buffer.len;
    size_t len_needed = (size_t)vsnprintf(&cad_scratch_buffer.str[cad_scratch_buffer.len], available, format, args);
    if (len_needed > available - 1)
        exit(-1);
    va_end(args);
    cad_scratch_buffer.len += len_needed;
}

void sb_append_signed_int(int64_t i) {
    sb_printf("%lld", (long long)i);
}

void sb_append_double(double d) {
    sb_printf("%f", d);

    //removing unused zeroes and dot
    while (cad_scratch_buffer.len > 0) {
        if (cad_scratch_buffer.str[cad_scratch_buffer.len - 1] != '0' && cad_scratch_buffer.str[cad_scratch_buffer.len - 1] != '.')
            return;
        cad_scratch_buffer.len--;
    }
}

void sb_append_char(char c) {
    if (cad_scratch_buffer.len + 1 > MAX_BUFFER_SIZE - 1)
        exit(-1);
    cad_scratch_buffer.str[cad_scratch_buffer.len++] = c;
}

char *str_copy(const char *start, size_t str_len) {
    char* dst = (char*)calloc(str_len, sizeof(char));
    memcpy(dst, start, str_len);
    // No need to set the end
    return dst;
}

char *sb_to_string(void) {
    cad_scratch_buffer.str[cad_scratch_buffer.len] = '\0';
    return cad_scratch_buffer.str;
}

char *sb_copy(void) {
    return str_copy(cad_scratch_buffer.str, cad_scratch_buffer.len);
}

char *sb_str_copy(void) {
    cad_scratch_buffer.str[cad_scratch_buffer.len] = '\0';
    return str_copy(cad_scratch_buffer.str, cad_scratch_buffer.len + 1);
}

void sb_clear() {
    cad_scratch_buffer.len = 0;
}
