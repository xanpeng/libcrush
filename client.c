#include "client.h"

int create_client(rados_t *client,
    const char *conf_file,
    const char *cluster_name,
    const char *user_name,
    uint64_t flags) {
  int ret = 0;
  ret = rados_create2(client, cluster_name, user_name, flags);
  if (ret < 0) return ret;

  ret = rados_conf_read_file(*client, conf_file);
  if (ret < 0) return ret;

  ret = rados_connect(*client);
  return ret;
}

struct crush_map* ref_crushmap(rados_t client) {
  return (struct crush_map*) rados_get_crushmap(client);
}

void unref_crushmap(rados_t client) {
  rados_put_crushmap(client);
}

/*
int main() {
  rados_t client;
  uint64_t flags = 0;
  create_client(&client, "/etc/ceph/ceph.conf", "ceph", "client.admin", flags);
  ref_crushmap(client);
  unref_crushmap(client);
}
*/
