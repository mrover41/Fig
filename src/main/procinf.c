#include <api/procinf.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <alloca.h>
#include <stdio.h>
#include <math.h>

#define BUFFER_SIZE 255

bool getBaseAddr(const pid_t pid, const char *name, size_t *addr, size_t *addre) {
	char *maps_path;

	size_t length = (pid > 0 ? log10(pid) + 1 : 1);
	maps_path = alloca(length + 12);

	sprintf(maps_path, "/proc/%i/maps", pid);
#ifdef DEBUG
	fprintf(stdout, "maps path: %s\n", maps_path);
	fprintf(stdout, "maps str length: %li + 12 = %li\n", length, length + 12);
#endif

	FILE *fp = fopen(maps_path, "r");
	if (fp == NULL) {
		fprintf(stderr, "Open file error: %s\n", maps_path);
		return false;
	}

	char *line = NULL;
	size_t len = 0;
	while(getdelim(&line, &len, '\n', fp) != -1) {
		char *n;
		if((n = strrchr(line, '/')) == NULL) continue;
		n += 1;
		if (strcmp(n, name) == 0) break;
#ifdef DEBUG
		fprintf(stdout, "Str not have '/': line: %s, n: %s\n", line, n);
#endif
	}
	
	char *addrs = NULL;
	if ((addrs = strchr(line, '-')) == NULL) {
		free(line);
		fclose(fp);
		fprintf(stderr, "Syntax field, '-' not found in string: %s\n", line);
		return false;
	}

	*addrs = '\0';
	*addr = (size_t)strtoul(line, NULL, 16);

#ifdef DEBUG
	fprintf(stdout, "Base addr found, base adress: \n%p\nstring:\n%s\n\n", (void *)*addr, line);
#endif

	char *line_cpy = addrs + 1;
	addrs = NULL;
	if ((addrs = strchr(line_cpy, ' ')) == NULL) {
		free(line);
		fclose(fp);
		fprintf(stderr, "Syntax field, ' ' not found in string: %s\n", line_cpy);
		return false;
	}
	*addrs = '\0';

	*addre = strtoul(line_cpy, NULL, 16);
#ifdef DEBUG
	fprintf(stdout, "Base addr end found, base adress: \n%p\nstring:\n%s\n\n", (void *)*addre, line_cpy);
#endif

	free(line);
	fclose(fp);
	return true;
}

void getName(pid_t pid, void *buff) {
	sprintf(buff, "/proc/%d/comm", pid);

	FILE *fp = fopen(buff, "r");
	if (fp == NULL) {
		fprintf(stderr, "Open file error: %s\n", (char *)buff);
		return;
	}
	
	if (fgets(buff, BUFFER_SIZE, fp) != NULL) {
#ifdef DEBUG
		fprintf(stdout, "[PROCESS_C] Programm name readed: %s\n", (char *)buff);
#endif
		fclose(fp);
		return;
	}

	fprintf(stderr, "Read file error: %s\n", (char *)buff);
	fclose(fp);
	buff = NULL;
	return;
}
