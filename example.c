#include <stdio.h>
#include <stdlib.h>
#include "libcrush.h"

void find_rule() {
  rados_t client;
  int ret = create_client(&client,
      "/etc/ceph/ceph.conf",
      "ceph",
      "client.admin",
      0);
  if (ret < 0) {
    puts("create rados failed.\n");
    exit(ret);
  }

  struct crush_map* crushmap = ref_crushmap(client);
  // function signature:
  //  int crush_find_rule(const struct crush_map *map, int ruleset, int type, int size)
  // compare result with `cehp osd crush rule dump`
  int rule_id = crush_find_rule(crushmap, 0, 1, 2);
  printf("rule id: %d\n", rule_id);
  unref_crushmap(client);
}

int main() {
  find_rule();
}
