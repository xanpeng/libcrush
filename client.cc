#include <cassert>
#include <string>
#include "client.h"

void create_client(librados::Rados& rados,
    std::string conf_file,
    std::string cluster_name,
    std::string user_name,
    uint64_t flags) {
  int ret = rados.init2(user_name.c_str(), cluster_name.c_str(), flags);
  assert(ret == 0);
  
  ret = rados.conf_read_file(conf_file.c_str());
  assert(ret == 0);

  ret = rados.connect();
  assert(ret == 0);
}

/*
int main() {
  librados::Rados rados;
  create_client(rados, "/etc/ceph/ceph.conf", "ceph", "client.admin", 0);
  const struct crush_map *crushmap = rados.get_crushmap();
  (void) crushmap;
  rados.put_crushmap();
}
*/
