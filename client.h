#ifndef LIBCRUSH_CLIENT_H
#define LIBCRUSH_CLIENT_H

#include <rados/librados.h>
#include "crush.h"

int create_client(rados_t *client,
    const char *conf_file,
    const char *cluster_name,
    const char *user_name,
    uint64_t flags);

struct crush_map* ref_crushmap(rados_t client);
void unref_crushmap(rados_t client);

#endif

