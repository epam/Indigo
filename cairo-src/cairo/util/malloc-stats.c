/* -*- Mode: c; c-basic-offset: 4; indent-tabs-mode: t; tab-width: 8; -*- */
/*
 * Copyright © 2007 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Red Hat, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Red Hat, Inc. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * RED HAT, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL RED HAT, INC. BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Behdad Esfahbod <behdad@behdad.org>
 */

/* A simple malloc wrapper that prints out statistics on termination */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>

/* caller-logging */

#include <string.h>

struct alloc_stat_t {
	int num;
	long size;
};

struct alloc_stats_t {
	struct alloc_stat_t malloc, realloc, total;
};

struct func_stat_t {
	const void *addr;
	const char *name;

	struct alloc_stats_t stat;
};

static struct func_stat_t *func_stats = NULL;
static int func_stats_num = 0;
static int func_stats_size = 0;

static void
alloc_stats_add (struct alloc_stats_t *stats, int is_realloc, size_t size)
{
	struct alloc_stat_t *stat = is_realloc ? &stats->realloc : &stats->malloc;

	stats->total.num++;
	stats->total.size += size;

	stat->num++;
	stat->size += size;
}

#include <execinfo.h>

static const char *
resolve_addr (const void *addr) {

	char **strings;
	char *p;
	const char *name = NULL;

	if (addr == NULL)
		return "(other)";
	if (addr == (void *) -1)
		return "(total)";

	strings = backtrace_symbols ((void**)&addr, 1);
	name = strdup (strings[0]);

	p = strchr (name, '\t');
	if (p)
		name = p + 1;

	free (strings);

	return name;
}

static void
func_stats_add (const void *caller, int is_realloc, size_t size)
{
	int i;
	const char *name;

	if (caller != (void *) -1 && caller != NULL)
		func_stats_add ((void *) -1, is_realloc, size);

	for (i = 0; i < func_stats_num; i++) {
		if (func_stats[i].addr == caller) {
			alloc_stats_add (&func_stats[i].stat, is_realloc, size);
			return;
		}
	}

	if (i == func_stats_size) {
		func_stats_size = func_stats_size ? func_stats_size * 2 : 16;
		func_stats = realloc (func_stats, func_stats_size * sizeof (func_stats[0]));
	}

	name = resolve_addr (caller);

	if (name) {
		func_stats_num++;
		func_stats[i].addr = caller;
		func_stats[i].name = name;
		memset (&func_stats[i].stat, 0, sizeof (func_stats[i].stat));
		alloc_stats_add (&func_stats[i].stat, is_realloc, size);
		return;
	}

	func_stats_add (NULL, is_realloc, size);
}

/* wrapper stuff */

#include <malloc.h>

static void *(*old_malloc)(size_t, const void *);
static void *(*old_realloc)(void *, size_t, const void *);

static void *my_malloc(size_t, const void *);
static void *my_realloc(void *, size_t, const void *);

static void
save_hooks (void)
{
	old_malloc  = __malloc_hook;
	old_realloc = __realloc_hook;
}

static void
old_hooks (void)
{
	__malloc_hook  = old_malloc;
	__realloc_hook  = old_realloc;
}

static void
my_hooks (void)
{
	/* should always save the current value */
	save_hooks ();

	__malloc_hook  = my_malloc;
	__realloc_hook  = my_realloc;
}

static void *
my_malloc(size_t size, const void *caller)
{
	void *ret;

	old_hooks ();

	func_stats_add (caller, 0, size);

	ret = malloc (size);
	my_hooks ();

	return ret;
}

static void *
my_realloc(void *ptr, size_t size, const void *caller)
{
	void *ret;

	old_hooks ();

	func_stats_add (caller, 1, size);

	ret = realloc (ptr, size);
	my_hooks ();

	return ret;
}

static void
my_init_hook(void) {
	my_hooks ();
}

void (*__malloc_initialize_hook) (void) = my_init_hook;


/* reporting */

#include <locale.h>

static void
add_alloc_stats (struct alloc_stats_t *a, struct alloc_stats_t *b)
{
	a->total.num += b->total.num;
	a->total.size += b->total.size;
	a->malloc.num += b->malloc.num;
	a->malloc.size += b->malloc.size;
	a->realloc.num += b->realloc.num;
	a->realloc.size += b->realloc.size;
}

static void
dump_alloc_stats (struct alloc_stats_t *stats, const char *name)
{
	printf ("%8d %'11ld	%8d %'11ld	%8d %'11ld	%s\n",
		stats->total.num, stats->total.size,
		stats->malloc.num, stats->malloc.size,
		stats->realloc.num, stats->realloc.size,
		name);
}

static int
compare_func_stats_name (const void *pa, const void *pb)
{
	const struct func_stat_t *a = pa, *b = pb;
	int i;

	i = strcmp (a->name, b->name);
	if (i)
		return i;

	return ((char *) a->addr - (char *) b->addr);
}

static int
compare_func_stats (const void *pa, const void *pb)
{
	const struct func_stat_t *a = pa, *b = pb;

	if (a->stat.total.num != b->stat.total.num)
		return (a->stat.total.num - b->stat.total.num);

	if (a->stat.total.size != b->stat.total.size)
		return (a->stat.total.size - b->stat.total.size);

	return compare_func_stats_name (pa, pb);
}

static void
merge_similar_entries (void)
{
	int i, j;

	j = 0;
	for (i = 1; i < func_stats_num; i++) {
		if (i != j && 0 == strcmp (func_stats[i].name, func_stats[j].name)) {
			add_alloc_stats (&func_stats[j].stat, &func_stats[i].stat);
		} else {
			j++;
			if (i != j)
				func_stats[j] = func_stats[i];
		}
	}
	j++;
	if (j < func_stats_num)
		func_stats_num = j;
}

__attribute__ ((destructor))
void
malloc_stats (void)
{
	int i;

	old_hooks ();

	/* merge entries with same name */
	qsort (func_stats, func_stats_num, sizeof (func_stats[0]), compare_func_stats_name);
	merge_similar_entries ();
	qsort (func_stats, func_stats_num, sizeof (func_stats[0]), compare_func_stats);

	if (func_stats_num) {
		setlocale (LC_ALL, "");

		printf ("	 TOTAL			 MALLOC			REALLOC\n");
		printf ("     num	size	     num	size	     num	size\n");

		for (i = 0; i < func_stats_num; i++) {
			dump_alloc_stats (&func_stats[i].stat, func_stats[i].name);
		}
	}
}
