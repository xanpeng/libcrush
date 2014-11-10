//
// g++ crush_tester.cc -o test_crush -lcrush -lrados --std=c++11 -g -O0
// notice:
// 1. librados is modified based on ceph0.86
// 2. libcrush is not official (see https://github.com/xanpeng/libcrush)
// 3. link libcrush before librados, because librados contains another libcrush codes.
//
#include <algorithm>
#include <vector>
#include <iterator>
#include <cstdio>
#include <stdint.h>
#include <assert.h>
#include <crush/libcrush.h>
using namespace std;

namespace internal {
#define mix(a, b, c)            \
    do {              \
      a = a - b;  a = a - c;  a = a ^ (c >> 13);  \
      b = b - c;  b = b - a;  b = b ^ (a << 8); \
      c = c - a;  c = c - b;  c = c ^ (b >> 13);  \
      a = a - b;  a = a - c;  a = a ^ (c >> 12);  \
      b = b - c;  b = b - a;  b = b ^ (a << 16);  \
      c = c - a;  c = c - b;  c = c ^ (b >> 5); \
      a = a - b;  a = a - c;  a = a ^ (c >> 3); \
      b = b - c;  b = b - a;  b = b ^ (a << 10);  \
      c = c - a;  c = c - b;  c = c ^ (b >> 15);  \
    } while (0)

uint32_t ceph_str_hash_rjenkins(const char *str, const unsigned length) {
  const unsigned char *k = (const unsigned char *)str;
  uint32_t a, b, c;  /* the internal state */
  uint32_t len;      /* how many key bytes still need mixing */
  len = length;   // internal state
  a = 0x9e3779b9;      /* the golden ratio; an arbitrary value */
  b = a;
  c = 0;               /* variable initialization of internal state */

  while (len >= 12) { /* handle most of the key */
    a = a + (k[0] + ((uint32_t)k[1] << 8) + ((uint32_t)k[2] << 16) + ((uint32_t)k[3] << 24));
    b = b + (k[4] + ((uint32_t)k[5] << 8) + ((uint32_t)k[6] << 16) + ((uint32_t)k[7] << 24));
    c = c + (k[8] + ((uint32_t)k[9] << 8) + ((uint32_t)k[10] << 16) + ((uint32_t)k[11] << 24));
    mix(a, b, c);
    k = k + 12;
    len = len - 12;
  }

  /* handle the last 11 bytes */
  c = c + length;
  switch (len) {            /* all the case statements fall through */
  case 11: c = c + ((uint32_t)k[10] << 24);
  case 10: c = c + ((uint32_t)k[9] << 16);
  case 9: c = c + ((uint32_t)k[8] << 8);
  /* the first byte of c is reserved for the length */
  case 8: b = b + ((uint32_t)k[7] << 24);
  case 7: b = b + ((uint32_t)k[6] << 16);
  case 6: b = b + ((uint32_t)k[5] << 8);
  case 5: b = b + k[4];
  case 4: a = a + ((uint32_t)k[3] << 24);
  case 3: a = a + ((uint32_t)k[2] << 16);
  case 2: a = a + ((uint32_t)k[1] << 8);
  case 1: a = a + k[0];
  /* case 0: nothing left to add */
  }
  mix(a, b, c);
  return c;
}

// x是输入变量，b表示bin的数目，bmask取满足2^n-1和b<=bmask的最小值。
// 这是一个稳定求模算法，但不具有绝对的稳定性，只是相对稳定。
// 假设b＝12，bmask则为15：
//   x＝13，1101 & 1111 ＝ 13 > b，返回 1101 & 0111 = 0101 = 5；
//   x＝7， 0111 & 1111 = 0111 ＝ 7 < b，返回7；
// b从12增加到15的过程中，原来的输入x取模的结果没有发生变化。但b超过15时，取模结果就会变化。这就是相对稳定。
// b = 12: 1 2 3 4 5 6 7 8 9 10 11 4 5 6 7 0 1 2 3 4 5 6 7 
// b = 13: 1 2 3 4 5 6 7 8 9 10 11 12 5 6 7 0 1 2 3 4 5 6 7 
// b = 14: 1 2 3 4 5 6 7 8 9 10 11 12 13 6 7 0 1 2 3 4 5 6 7 
// b = 15: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 7 0 1 2 3 4 5 6 7 
// b = 16: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 0 1 2 3 4 5 6 7 
// b = 17: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 1 2 3 4 5 6 7 
// b = 18: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 2 3 4 5 6 7 
// b = 19: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 3 4 5 6 7 
// b = 20: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 4 5 6 7 
// b = 21: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 5 6 7 
// b = 22: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 6 7 
// b = 23: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 7
int crush_stable_mod(int x, int b, int bmask) {
  if ((x & bmask) < b)
    return x & bmask;
  else
    return x & (bmask >> 1);
}
} // namespace internal

int g_pgp_num = 128, g_pgp_num_mask = 127;
int g_pg_num = 128, g_pg_num_mask = 127;
int g_pool_id = 1, g_pool_type = 1; 
string g_pool = "rbd";
int g_replicas = 2;
int g_crush_ruleset = 0;

typedef uint32_t ps_t; // placement seed

struct pg_t {
  uint32_t seed;
  uint64_t pool_id;
  int32_t preferred;
  pg_t(uint32_t seed, uint64_t pool_id, int32_t preferred)
    : seed(seed), pool_id(pool_id), preferred(preferred) {}
};

struct object_t {
  string name;
  object_t(string name) : name(name) {}
};

struct object_locator_t {
  uint64_t pool_id;
  string key;    // 指定hash()的输入，一般为空
  string nspace; // 作为object name的前缀，影响hash值，一般为空
  int64_t hash;  // 指定hash值
  object_locator_t(uint64_t pool_id) : pool_id(pool_id), key(""), nspace(""), hash(-1) {}
};

// step 1: 根据object name定位pg，此时的pg其实就是字符串hash得到的一个uint32_t
pg_t object_to_pg(object_t oid, object_locator_t loc) {
  ps_t ps = internal::ceph_str_hash_rjenkins(oid.name.c_str(), oid.name.length());
  return pg_t(ps, loc.pool_id, -1);
}

// step 2:
pg_t raw_pg_to_pg(pg_t pg) {
  pg.seed = internal::crush_stable_mod(pg.seed, g_pg_num, g_pg_num_mask);
  return pg;
}

// step 3: 将pg映射到osd
void pg_to_up_acting_osds(pg_t pg, vector<int> *up) {
  librados::Rados rados;
  create_client(rados, "/etc/ceph/ceph.conf", "ceph", "client.admin", 0);
  const struct crush_map *crushmap = rados.get_crushmap();
  assert(crushmap != NULL);

  vector<uint32_t> osd_weight = rados.get_osd_weights();
  // vector<uint32_t> osd_weight {1965,655,655,655,1965,655,655,655};
  assert(osd_weight.size() > 0);
  printf("  osd_weight: ");
  for (uint32_t w : osd_weight) printf("%d,", w);
  printf("\n");

  int ruleno = crush_find_rule(crushmap, g_crush_ruleset, g_pool_type, g_replicas);
  printf("  ruleno: %d\n", ruleno);

  ps_t placement_ps = crush_hash32_2(0, // crush_hash_rjenkins1
      internal::crush_stable_mod(pg.seed, g_pgp_num, g_pgp_num_mask),
      g_pool_id);
  printf("  placement_ps: %d\n", placement_ps);

  int rawout[g_replicas], scratch[g_replicas * 3];
  printf("\n---start crush_do_rule---\n");
  int numrep = crush_do_rule(crushmap, ruleno, placement_ps, rawout, g_replicas, &osd_weight[0], osd_weight.size(), scratch);
  printf("---finish crush_do_rule---\n\n");
  up->resize(numrep);
  for (int i = 0; i < numrep; ++i)
    up->at(i) = rawout[i];
  printf("  numrep: %d, raw_osds: [", numrep);
  for (int i = 0; i < numrep; ++i) printf("%d,", rawout[i]);
  printf("]\n");

  // remove nonexistent osds

  rados.put_crushmap();
}

int main(int argc, char **argv) {
  assert(argc == 2);
  string objname = argv[argc-1];

  pg_t pg = object_to_pg(object_t(objname), object_locator_t(g_pool_id));
  printf("objectr_to_pg: %s -> %x\n", objname.c_str(), pg.seed);

  pg_t mpg = raw_pg_to_pg(pg);
  printf("raw_pg_to_pg: %x\n", mpg.seed);

  printf("pg_to_osds:\n");
  vector<int> up;
  pg_to_up_acting_osds(mpg, &up);
}
