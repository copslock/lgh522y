#include "util.h"
#include "sysfs.h"
#include "../perf.h"
#include "cpumap.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static struct cpu_map *cpu_map__default_new(void)
{
	struct cpu_map *cpus;
	int nr_cpus;

	nr_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (nr_cpus < 0)
		return NULL;

	cpus = malloc(sizeof(*cpus) + nr_cpus * sizeof(int));
	if (cpus != NULL) {
		int i;
		for (i = 0; i < nr_cpus; ++i)
			cpus->map[i] = i;

		cpus->nr = nr_cpus;
	}

	return cpus;
}

static struct cpu_map *cpu_map__trim_new(int nr_cpus, int *tmp_cpus)
{
	size_t payload_size = nr_cpus * sizeof(int);
	struct cpu_map *cpus = malloc(sizeof(*cpus) + payload_size);

	if (cpus != NULL) {
		cpus->nr = nr_cpus;
		memcpy(cpus->map, tmp_cpus, payload_size);
	}

	return cpus;
}

struct cpu_map *cpu_map__read(FILE *file)
{
	struct cpu_map *cpus = NULL;
	int nr_cpus = 0;
	int *tmp_cpus = NULL, *tmp;
	int max_entries = 0;
	int n, cpu, prev;
	char sep;

	sep = 0;
	prev = -1;
	for (;;) {
		n = fscanf(file, "%u%c", &cpu, &sep);
		if (n <= 0)
			break;
		if (prev >= 0) {
			int new_max = nr_cpus + cpu - prev - 1;

			if (new_max >= max_entries) {
				max_entries = new_max + MAX_NR_CPUS / 2;
				tmp = realloc(tmp_cpus, max_entries * sizeof(int));
				if (tmp == NULL)
					goto out_free_tmp;
				tmp_cpus = tmp;
			}

			while (++prev < cpu)
				tmp_cpus[nr_cpus++] = prev;
		}
		if (nr_cpus == max_entries) {
			max_entries += MAX_NR_CPUS;
			tmp = realloc(tmp_cpus, max_entries * sizeof(int));
			if (tmp == NULL)
				goto out_free_tmp;
			tmp_cpus = tmp;
		}

		tmp_cpus[nr_cpus++] = cpu;
		if (n == 2 && sep == '-')
			prev = cpu;
		else
			prev = -1;
		if (n == 1 || sep == '\n')
			break;
	}

	if (nr_cpus > 0)
		cpus = cpu_map__trim_new(nr_cpus, tmp_cpus);
	else
		cpus = cpu_map__default_new();
out_free_tmp:
	free(tmp_cpus);
	return cpus;
}

static struct cpu_map *cpu_map__read_all_cpu_map(void)
{
	struct cpu_map *cpus = NULL;
	FILE *onlnf;

	onlnf = fopen("/sys/devices/system/cpu/online", "r");
	if (!onlnf)
		return cpu_map__default_new();

	cpus = cpu_map__read(onlnf);
	fclose(onlnf);
	return cpus;
}

struct cpu_map *cpu_map__new(const char *cpu_list)
{
	struct cpu_map *cpus = NULL;
	unsigned long start_cpu, end_cpu = 0;
	char *p = NULL;
	int i, nr_cpus = 0;
	int *tmp_cpus = NULL, *tmp;
	int max_entries = 0;

	if (!cpu_list)
		return cpu_map__read_all_cpu_map();

	if (!isdigit(*cpu_list))
		goto out;

	while (isdigit(*cpu_list)) {
		p = NULL;
		start_cpu = strtoul(cpu_list, &p, 0);
		if (start_cpu >= INT_MAX
		    || (*p != '\0' && *p != ',' && *p != '-'))
			goto invalid;

		if (*p == '-') {
			cpu_list = ++p;
			p = NULL;
			end_cpu = strtoul(cpu_list, &p, 0);

			if (end_cpu >= INT_MAX || (*p != '\0' && *p != ','))
				goto invalid;

			if (end_cpu < start_cpu)
				goto invalid;
		} else {
			end_cpu = start_cpu;
		}

		for (; start_cpu <= end_cpu; start_cpu++) {
			/*                      */
			for (i = 0; i < nr_cpus; i++)
				if (tmp_cpus[i] == (int)start_cpu)
					goto invalid;

			if (nr_cpus == max_entries) {
				max_entries += MAX_NR_CPUS;
				tmp = realloc(tmp_cpus, max_entries * sizeof(int));
				if (tmp == NULL)
					goto invalid;
				tmp_cpus = tmp;
			}
			tmp_cpus[nr_cpus++] = (int)start_cpu;
		}
		if (*p)
			++p;

		cpu_list = p;
	}

	if (nr_cpus > 0)
		cpus = cpu_map__trim_new(nr_cpus, tmp_cpus);
	else
		cpus = cpu_map__default_new();
invalid:
	free(tmp_cpus);
out:
	return cpus;
}

size_t cpu_map__fprintf(struct cpu_map *map, FILE *fp)
{
	int i;
	size_t printed = fprintf(fp, "%d cpu%s: ",
				 map->nr, map->nr > 1 ? "s" : "");
	for (i = 0; i < map->nr; ++i)
		printed += fprintf(fp, "%s%d", i ? ", " : "", map->map[i]);

	return printed + fprintf(fp, "\n");
}

struct cpu_map *cpu_map__dummy_new(void)
{
	struct cpu_map *cpus = malloc(sizeof(*cpus) + sizeof(int));

	if (cpus != NULL) {
		cpus->nr = 1;
		cpus->map[0] = -1;
	}

	return cpus;
}

void cpu_map__delete(struct cpu_map *map)
{
	free(map);
}

int cpu_map__get_socket(struct cpu_map *map, int idx)
{
	FILE *fp;
	const char *mnt;
	char path[PATH_MAX];
	int cpu, ret;

	if (idx > map->nr)
		return -1;

	cpu = map->map[idx];

	mnt = sysfs_find_mountpoint();
	if (!mnt)
		return -1;

	snprintf(path, PATH_MAX,
		"%s/devices/system/cpu/cpu%d/topology/physical_package_id",
		mnt, cpu);

	fp = fopen(path, "r");
	if (!fp)
		return -1;
	ret = fscanf(fp, "%d", &cpu);
	fclose(fp);
	return ret == 1 ? cpu : -1;
}

static int cmp_ids(const void *a, const void *b)
{
	return *(int *)a - *(int *)b;
}

static int cpu_map__build_map(struct cpu_map *cpus, struct cpu_map **res,
			      int (*f)(struct cpu_map *map, int cpu))
{
	struct cpu_map *c;
	int nr = cpus->nr;
	int cpu, s1, s2;

	/*                              */
	c = calloc(1, sizeof(*c) + nr * sizeof(int));
	if (!c)
		return -1;

	for (cpu = 0; cpu < nr; cpu++) {
		s1 = f(cpus, cpu);
		for (s2 = 0; s2 < c->nr; s2++) {
			if (s1 == c->map[s2])
				break;
		}
		if (s2 == c->nr) {
			c->map[c->nr] = s1;
			c->nr++;
		}
	}
	/*                                          */
	qsort(c->map, c->nr, sizeof(int), cmp_ids);

	*res = c;
	return 0;
}

int cpu_map__get_core(struct cpu_map *map, int idx)
{
	FILE *fp;
	const char *mnt;
	char path[PATH_MAX];
	int cpu, ret, s;

	if (idx > map->nr)
		return -1;

	cpu = map->map[idx];

	mnt = sysfs_find_mountpoint();
	if (!mnt)
		return -1;

	snprintf(path, PATH_MAX,
		"%s/devices/system/cpu/cpu%d/topology/core_id",
		mnt, cpu);

	fp = fopen(path, "r");
	if (!fp)
		return -1;
	ret = fscanf(fp, "%d", &cpu);
	fclose(fp);
	if (ret != 1)
		return -1;

	s = cpu_map__get_socket(map, idx);
	if (s == -1)
		return -1;

	/*
                                  
                                      
                                      
                   
  */
	return (s << 16) | (cpu & 0xffff);
}

int cpu_map__build_socket_map(struct cpu_map *cpus, struct cpu_map **sockp)
{
	return cpu_map__build_map(cpus, sockp, cpu_map__get_socket);
}

int cpu_map__build_core_map(struct cpu_map *cpus, struct cpu_map **corep)
{
	return cpu_map__build_map(cpus, corep, cpu_map__get_core);
}
