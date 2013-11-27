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
        proxy_cache_path  /tmp/cache  keys_zone=tmpcache:10m;

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
        proxy_cache_path  /tmp/cache  keys_zone=tmpcache:10m;

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
 

Declaration
========
This README template copy from [ngx_cache_purge][] module and make a little modification.


See also
========
- [ngx_cache_purge][]

[ngx_cache_purge]: https://github.com/FRiCKLE/ngx_cache_purge

