#include <stdio.h>
#include <stdlib.h>
#include "libcrush.h"

void find_rule() {
  librados::Rados rados;
  create_client(rados, "/etc/ceph/ceph.conf", "ceph", "client.admin", 0);

  const struct crush_map* crushmap = rados.get_crushmap();
  // function signature:
  //  int crush_find_rule(const struct crush_map *map, int ruleset, int type, int size)
  // compare result with `cehp osd crush rule dump`
  int rule_id = crush_find_rule(crushmap, 0, 1, 2);
  printf("rule id: %d\n", rule_id);
  rados.put_crushmap();
}

int main() {
  find_rule();
}
