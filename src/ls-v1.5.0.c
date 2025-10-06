/*
 * ls-v1.5.0.c
 * Feature-6: Colorized Output Based On File Type
 * - preserves the "down-then-across" column layout from v1.4.0
 * - uses lstat() to determine file types and prints ANSI colors
 * - only emits color when stdout is a terminal (auto), respects NO_COLOR env var
 *
 * Build:
 *   mkdir -p bin
 *   gcc -std=c11 -Wall -Wextra -O2 -o bin/ls-v1.5.0 src/ls-v1.5.0.c
 *
 * Test:
 *   ./bin/ls-v1.5.0 [path]
 */

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>
#include <ctype.h>

/* ANSI color/style defines */
#define ANSI_RESET    "\033[0m"
#define ANSI_BLUE     "\033[0;34m"   /* directories */
#define ANSI_GREEN    "\033[0;32m"   /* executables */
#define ANSI_RED      "\033[0;31m"   /* archives */
#define ANSI_PINK     "\033[0;35m"   /* symbolic links */
#define ANSI_REVERSE  "\033[7m"      /* special files (reverse video) */

/* spacing between columns */
#define COL_SPACING 2

static int cmpstr(const void *a, const void *b) {
    const char *sa = *(const char **)a;
    const char *sb = *(const char **)b;
    return strcasecmp(sa, sb);
}

static bool ends_with(const char *str, const char *suffix) {
    if (!str || !suffix) return false;
    size_t lenstr = strlen(str);
    size_t lensuf = strlen(suffix);
    if (lensuf > lenstr) return false;
    return strcmp(str + lenstr - lensuf, suffix) == 0;
}

static bool is_archive_name(const char *name) {
    if (!name) return false;
    /* common archive suffixes (case-insensitive check) */
    char lower[NAME_MAX + 1];
    size_t n = strlen(name);
    if (n > NAME_MAX) n = NAME_MAX;
    for (size_t i = 0; i < n; ++i) lower[i] = (char)tolower((unsigned char)name[i]);
    lower[n] = '\0';

    if (ends_with(lower, ".tar") || ends_with(lower, ".tar.gz") ||
        ends_with(lower, ".tgz") || ends_with(lower, ".gz") ||
        ends_with(lower, ".zip")) return true;
    return false;
}

/* Print name with color determined by lstat(st_mode) and filename */
static void print_name_colored(const char *dirpath, const char *name, bool color_enabled, int colwidth) {
    char fullpath[PATH_MAX];
    struct stat st;
    int rc;

    if (snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, name) >= (int)sizeof(fullpath)) {
        /* fallback: just print name (no path) */
        printf("%s", name);
        int pad = colwidth - (int)strlen(name) + COL_SPACING;
        for (int i = 0; i < pad; ++i) putchar(' ');
        return;
    }

    rc = lstat(fullpath, &st);
    /* If lstat fails, print plain */
    if (rc == -1) {
        printf("%s", name);
        int pad = colwidth - (int)strlen(name) + COL_SPACING;
        for (int i = 0; i < pad; ++i) putchar(' ');
        return;
    }

    const char *start = NULL;
    const char *end = ANSI_RESET;

    if (!color_enabled) {
        start = NULL;
        end = NULL;
    } else if (S_ISLNK(st.st_mode)) {
        start = ANSI_PINK;
    } else if (S_ISDIR(st.st_mode)) {
        start = ANSI_BLUE;
    } else if (is_archive_name(name)) {
        start = ANSI_RED;
    } else if (S_ISREG(st.st_mode) && (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))) {
        start = ANSI_GREEN;
    } else if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode) || S_ISSOCK(st.st_mode) || S_ISFIFO(st.st_mode)) {
        start = ANSI_REVERSE;
    } else {
        start = NULL; /* no color */
        end = NULL;
    }

    size_t namelen = strlen(name);

    if (start) printf("%s%s%s", start, name, end);
    else printf("%s", name);

    /* pad spaces to reach column width + spacing
       Note: pad based on name length (printable characters) so colors don't affect alignment */
    int pad = colwidth - (int)namelen + COL_SPACING;
    if (pad < 0) pad = COL_SPACING;
    for (int i = 0; i < pad; ++i) putchar(' ');
}

int main(int argc, char **argv) {
    const char *dirpath = ".";
    if (argc > 1) dirpath = argv[1];

    DIR *d = opendir(dirpath);
    if (!d) {
        fprintf(stderr, "error: cannot open '%s': %s\n", dirpath, strerror(errno));
        return 2;
    }

    struct dirent *entry;
    size_t capacity = 128;
    size_t count = 0;
    char **names = malloc(capacity * sizeof(char*));
    if (!names) { perror("malloc"); closedir(d); return 1; }

    while ((entry = readdir(d)) != NULL) {
        /* skip . and .. */
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        if (count >= capacity) {
            capacity *= 2;
            char **tmp = realloc(names, capacity * sizeof(char*));
            if (!tmp) { perror("realloc"); break; }
            names = tmp;
        }
        names[count] = strdup(entry->d_name);
        if (!names[count]) { perror("strdup"); break; }
        count++;
    }
    closedir(d);

    if (count == 0) {
        free(names);
        return 0;
    }

    /* sort names alphabetically (case-insensitive) */
    qsort(names, count, sizeof(char*), cmpstr);

    /* determine maximum name length (printable) */
    int maxlen = 0;
    for (size_t i = 0; i < count; ++i) {
        int l = (int)strlen(names[i]);
        if (l > maxlen) maxlen = l;
    }

    /* determine terminal width */
    int term_width = 80;
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) term_width = ws.ws_col;

    int colwidth = maxlen; /* width reserved for each column (without spacing) */
    int total_col_w = colwidth + COL_SPACING;
    int cols = term_width / total_col_w;
    if (cols < 1) cols = 1;
    if (cols > (int)count) cols = (int)count;

    int rows = (int)((count + cols - 1) / cols); /* ceil(count/cols) */

    /* color enabled? (only when stdout is a tty and NO_COLOR unset) */
    bool color_enabled = isatty(STDOUT_FILENO) && getenv("NO_COLOR") == NULL;

    /* print in down-then-across order */
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int idx = c * rows + r;
            if (idx >= (int)count) continue;
            print_name_colored(dirpath, names[idx], color_enabled, colwidth);
        }
        putchar('\n');
    }

    /* cleanup */
    for (size_t i = 0; i < count; ++i) free(names[i]);
    free(names);
    return 0;
}

