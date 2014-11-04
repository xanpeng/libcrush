under development...

I want to:

- simulate CRUSH algorithm with online data in a live Ceph cluster.
- access crushmap.

Way I choose: adjust Ceph code, make a new librados.  
Code base: ceph-0.86.  
Diff: libcrush-based-on-ceph0.86.diff.  

I tried:

- extract crush + client code from kernel code. -- difficult.
- use librados directly. -- no access to crushmap.

More ideas:

- crushtool --simulate

