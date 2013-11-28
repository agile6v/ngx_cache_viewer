About
=====
nginx module which adds ability to view cache node info from FastCGI, proxy, SCGI and uWSGI caches.
It inspired by [ngx_cache_purge][] module.


Configuration directives (same location syntax)
=====

fastcgi_cache_viewer
-------------------
* **syntax**: `fastcgi_cache_viewer on|off|<method> [from all|<ip> [.. <ip>]]`
* **default**: `none`
* **context**: `http`, `server`, `location`

Allow viewing of selected pages from `FastCGI`'s cache.


proxy_cache_viewer
-----------------
* **syntax**: `proxy_cache_viewer on|off|<method> [from all|<ip> [.. <ip>]]`
* **default**: `none`
* **context**: `http`, `server`, `location`

Allow viewing of selected pages from `proxy`'s cache.


scgi_cache_viewer
----------------
* **syntax**: `scgi_cache_viewer on|off|<method> [from all|<ip> [.. <ip>]]`
* **default**: `none`
* **context**: `http`, `server`, `location`

Allow viewing of selected pages from `SCGI`'s cache.


uwsgi_cache_viewer
-----------------
* **syntax**: `uwsgi_cache_viewer on|off|<method> [from all|<ip> [.. <ip>]]`
* **default**: `none`
* **context**: `http`, `server`, `location`

Allow viewing of selected pages from `uWSGI`'s cache.


Configuration directives (separate location syntax)
===================================================
fastcgi_cache_viewer
-------------------
* **syntax**: `fastcgi_cache_viewer zone_name key`
* **default**: `none`
* **context**: `location`

Sets area and key used for viewing selected pages from `FastCGI`'s cache.


proxy_cache_viewer
-----------------
* **syntax**: `proxy_cache_viewer zone_name key`
* **default**: `none`
* **context**: `location`

Sets area and key used for viewing selected pages from `proxy`'s cache.


scgi_cache_viewer
----------------
* **syntax**: `scgi_cache_viewer zone_name key`
* **default**: `none`
* **context**: `location`

Sets area and key used for viewing selected pages from `SCGI`'s cache.


uwsgi_cache_viewer
-----------------
* **syntax**: `uwsgi_cache_viewer zone_name key`
* **default**: `none`
* **context**: `location`

Sets area and key used for viewing selected pages from `uWSGI`'s cache.


Sample configuration (same location syntax)
===========================================
    http {
        proxy_cache_path  /tmp/cache  keys_zone=tmpcache:10m max_size=1g;

        server {
            location / {
                proxy_pass         http://127.0.0.1:8000;
                proxy_cache        tmpcache;
                proxy_cache_key    $uri$is_args$args;
                proxy_cache_viewer VIEWER from 127.0.0.1;
            }
        }
    }


Sample configuration (separate location syntax)
===============================================
    http {
        proxy_cache_path  /tmp/cache  keys_zone=tmpcache:10m max_size=1g;

        server {
            location / {
                proxy_pass         http://127.0.0.1:8000;
                proxy_cache        tmpcache;
                proxy_cache_key    $uri$is_args$args;
            }

            location ~ /viewer(/.*) {
                allow              127.0.0.1;
                deny               all;
                proxy_cache_viewer tmpcache $1$is_args$args;
            }
        }
    }
    
Sample Output
===============================================
```bash

$ curl http://127.0.0.1:8000/003 -X VIEWER
################## share memory ##################
name:             tmpcache
mem size(Kb):     10240
cache node count: 3
cache path:       /tmp/cache
disk blocks:      262144
used disk blocks: 3
disk block size:  4096
inactive time(s): 600
-------------------------------------------
filename:         /tmp/cache/f16f9677cebace31cfe18821d4da093e
valid_sec:        0
count:            1
uses:             3
expire(UTC):      1385640959
body_start:       340
fs_size(block):   1
exists:           1
updating:         0
deleting:         0


$ curl http://127.0.0.1:8000/ -X VIEWER
################## share memory ##################
name:             tmpcache
mem size(Kb):     10240
cache node count: 3
cache path:       /tmp/cache
disk blocks:      262144
used disk blocks: 3
disk block size:  4096
inactive time(s): 600
-------------------------------------------

```

Declaration
========
This README template copy from [ngx_cache_purge][] module and make a little modification.


See also
========
- [ngx_cache_purge][]

[ngx_cache_purge]: https://github.com/FRiCKLE/ngx_cache_purge

