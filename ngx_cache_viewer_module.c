/*
 *  Copyright (C) agile6v
 */

#include <nginx.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


#if (NGX_HTTP_CACHE)

typedef struct {
    ngx_flag_t                    enable;
    ngx_str_t                     method;
    ngx_array_t                  *access;   /* array of ngx_in_cidr_t */
    ngx_array_t                  *access6;  /* array of ngx_in6_cidr_t */
} ngx_http_cache_viewer_conf_t;

typedef struct {
# if (NGX_HTTP_FASTCGI)
    ngx_http_cache_viewer_conf_t   fastcgi;
# endif /* NGX_HTTP_FASTCGI */
# if (NGX_HTTP_PROXY)
    ngx_http_cache_viewer_conf_t   proxy;
# endif /* NGX_HTTP_PROXY */
# if (NGX_HTTP_SCGI)
    ngx_http_cache_viewer_conf_t   scgi;
# endif /* NGX_HTTP_SCGI */
# if (NGX_HTTP_UWSGI)
    ngx_http_cache_viewer_conf_t   uwsgi;
# endif /* NGX_HTTP_UWSGI */

    ngx_http_cache_viewer_conf_t  *conf;
    ngx_http_handler_pt           handler;
    ngx_http_handler_pt           original_handler;
} ngx_http_cache_viewer_loc_conf_t;

# if (NGX_HTTP_FASTCGI)
char       *ngx_http_fastcgi_cache_viewer_conf(ngx_conf_t *cf,
                ngx_command_t *cmd, void *conf);
ngx_int_t   ngx_http_fastcgi_cache_viewer_handler(ngx_http_request_t *r);
# endif /* NGX_HTTP_FASTCGI */

# if (NGX_HTTP_PROXY)
char       *ngx_http_proxy_cache_viewer_conf(ngx_conf_t *cf,
                ngx_command_t *cmd, void *conf);
ngx_int_t   ngx_http_proxy_cache_viewer_handler(ngx_http_request_t *r);
# endif /* NGX_HTTP_PROXY */

# if (NGX_HTTP_SCGI)
char       *ngx_http_scgi_cache_viewer_conf(ngx_conf_t *cf,
                ngx_command_t *cmd, void *conf);
ngx_int_t   ngx_http_scgi_cache_viewer_handler(ngx_http_request_t *r);
# endif /* NGX_HTTP_SCGI */

# if (NGX_HTTP_UWSGI)
char       *ngx_http_uwsgi_cache_viewer_conf(ngx_conf_t *cf,
                ngx_command_t *cmd, void *conf);
ngx_int_t   ngx_http_uwsgi_cache_viewer_handler(ngx_http_request_t *r);
# endif /* NGX_HTTP_UWSGI */

ngx_int_t   ngx_http_cache_viewer_access_handler(ngx_http_request_t *r);
ngx_int_t   ngx_http_cache_viewer_access(ngx_array_t *a, ngx_array_t *a6,
    struct sockaddr *s);

ngx_int_t   ngx_http_cache_viewer_send_response(ngx_http_request_t *r, ngx_uint_t flag);
ngx_int_t   ngx_http_cache_viewer_init(ngx_http_request_t *r,
    ngx_http_file_cache_t *cache, ngx_http_complex_value_t *cache_key);
void        ngx_http_cache_viewer_handler(ngx_http_request_t *r);

ngx_int_t   ngx_http_file_cache_viewer(ngx_http_request_t *r);

char       *ngx_http_cache_viewer_conf(ngx_conf_t *cf,
    ngx_http_cache_viewer_conf_t *cpcf);

void       *ngx_http_cache_viewer_create_loc_conf(ngx_conf_t *cf);
char       *ngx_http_cache_viewer_merge_loc_conf(ngx_conf_t *cf,
    void *parent, void *child);

#if (NGX_DEBUG)
static ngx_uint_t ngx_http_viewer_cache_node_count(ngx_http_file_cache_t *cache);
#endif

static ngx_int_t ngx_http_viewer_cache_node_info(ngx_http_request_t *r, ngx_chain_t *out);
static ngx_int_t ngx_http_viewer_cache_shm_info(ngx_http_request_t *r, ngx_chain_t *out);

static ngx_command_t  ngx_http_cache_viewer_module_commands[] = {

# if (NGX_HTTP_FASTCGI)
    { ngx_string("fastcgi_cache_viewer"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_1MORE,
      ngx_http_fastcgi_cache_viewer_conf,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },
# endif /* NGX_HTTP_FASTCGI */

# if (NGX_HTTP_PROXY)
    { ngx_string("proxy_cache_viewer"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_1MORE,
      ngx_http_proxy_cache_viewer_conf,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },
# endif /* NGX_HTTP_PROXY */

# if (NGX_HTTP_SCGI)
    { ngx_string("scgi_cache_viewer"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_1MORE,
      ngx_http_scgi_cache_viewer_conf,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },
# endif /* NGX_HTTP_SCGI */

# if (NGX_HTTP_UWSGI)
    { ngx_string("uwsgi_cache_viewer"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_1MORE,
      ngx_http_uwsgi_cache_viewer_conf,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },
# endif /* NGX_HTTP_UWSGI */

      ngx_null_command
};

static ngx_http_module_t  ngx_http_cache_viewer_module_ctx = {
    NULL,                                  /* preconfiguration */
    NULL,                                  /* postconfiguration */
    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */
    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */
    ngx_http_cache_viewer_create_loc_conf, /* create location configuration */
    ngx_http_cache_viewer_merge_loc_conf   /* merge location configuration */
};

ngx_module_t  ngx_http_cache_viewer_module = {
    NGX_MODULE_V1,
    &ngx_http_cache_viewer_module_ctx,     /* module context */
    ngx_http_cache_viewer_module_commands, /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

#define NGX_VIEWER_LEN(STR)         sizeof(STR) - 1
#define SYMBOL_SEPARATOR            "-------------------------------------------"
#define SHM_TITLE_LINE              "################## share memory ##################"
#define SHM_NAME_HDR                "name:             "
#define SHM_MEM_SIZE_HDR            "mem size(Kb):     "
#define SHM_DISK_BLKS_HDR           "disk blocks:      "
#define SHM_USED_DISK_BLKS_HDR      "used disk blocks: "
#define SHM_DISK_BLK_SIZE_HDR       "disk block size:  "
#define SHM_NODE_INACTIVE_TIME      "inactive time(s): "
#define SHM_PATH_HDR                "cache path:       "
#define SHM_FILE_COUNT_HDR          "cache node count: "

#define CACHE_NODE_FILENAME         "filename:         "
#define CACHE_NODE_VALID_SEC        "valid_sec:        "
#define CACHE_NODE_COUNT            "count:            "
#define CACHE_NODE_USES             "uses:             "
#define CACHE_NODE_EXISTS           "exists:           "
#define CACHE_NODE_UPDATING         "updating:         "
#define CACHE_NODE_DELETING         "deleting:         "
#define CACHE_NODE_EXPIRE           "expire(UTC):      "
#define CACHE_NODE_BODYSTART        "body_start:       "
#define CACHE_NODE_FS_SIZE          "fs_size(block):   "


# if (NGX_HTTP_FASTCGI)
extern ngx_module_t  ngx_http_fastcgi_module;

typedef struct {
    ngx_http_upstream_conf_t       upstream;

    ngx_str_t                      index;

    ngx_array_t                   *flushes;
    ngx_array_t                   *params_len;
    ngx_array_t                   *params;
    ngx_array_t                   *params_source;
    ngx_array_t                   *catch_stderr;

    ngx_array_t                   *fastcgi_lengths;
    ngx_array_t                   *fastcgi_values;

#  if defined(nginx_version) && (nginx_version >= 8040)
    ngx_hash_t                     headers_hash;
    ngx_uint_t                     header_params;
#  endif /* nginx_version >= 8040 */

#  if defined(nginx_version) && (nginx_version >= 1001004)
    ngx_flag_t                     keep_conn;
#  endif /* nginx_version >= 1001004 */

    ngx_http_complex_value_t       cache_key;

#  if (NGX_PCRE)
    ngx_regex_t                   *split_regex;
    ngx_str_t                      split_name;
#  endif /* NGX_PCRE */
} ngx_http_fastcgi_loc_conf_t;

char *
ngx_http_fastcgi_cache_viewer_conf(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf)
{
    ngx_http_compile_complex_value_t   ccv;
    ngx_http_cache_viewer_loc_conf_t   *cplcf;
    ngx_http_core_loc_conf_t          *clcf;
    ngx_http_fastcgi_loc_conf_t       *flcf;
    ngx_str_t                         *value;

    cplcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_cache_viewer_module);

    /* check for duplicates / collisions */
    if (cplcf->fastcgi.enable != NGX_CONF_UNSET) {
        return "is duplicate";
    }

    if (cf->args->nelts != 3) {
        return ngx_http_cache_viewer_conf(cf, &cplcf->fastcgi);
    }

    if (cf->cmd_type & (NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF)) {
        return "(separate location syntax) is not allowed here";
    }

    flcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_fastcgi_module);

    if (flcf->upstream.cache != NGX_CONF_UNSET_PTR
        && flcf->upstream.cache != NULL)
    {
        return "is incompatible with \"fastcgi_cache\"";
    }

    if (flcf->upstream.upstream || flcf->fastcgi_lengths) {
        return "is incompatible with \"fastcgi_pass\"";
    }

    if (flcf->upstream.store > 0 || flcf->upstream.store_lengths) {
        return "is incompatible with \"fastcgi_store\"";
    }

    value = cf->args->elts;

    /* set fastcgi_cache part */
    flcf->upstream.cache = ngx_shared_memory_add(cf, &value[1], 0,
                                                 &ngx_http_fastcgi_module);
    if (flcf->upstream.cache == NULL) {
        return NGX_CONF_ERROR;
    }

    /* set fastcgi_cache_key part */
    ngx_memzero(&ccv, sizeof(ngx_http_compile_complex_value_t));

    ccv.cf = cf;
    ccv.value = &value[2];
    ccv.complex_value = &flcf->cache_key;

    if (ngx_http_compile_complex_value(&ccv) != NGX_OK) {
        return NGX_CONF_ERROR;
    }

    /* set handler */
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

    cplcf->fastcgi.enable = 0;
    clcf->handler = ngx_http_fastcgi_cache_viewer_handler;

    return NGX_CONF_OK;
}

ngx_int_t
ngx_http_fastcgi_cache_viewer_handler(ngx_http_request_t *r)
{
    ngx_http_fastcgi_loc_conf_t  *flcf;

    flcf = ngx_http_get_module_loc_conf(r, ngx_http_fastcgi_module);

    if (ngx_http_cache_viewer_init(r, flcf->upstream.cache->data,
                                  &flcf->cache_key)
        != NGX_OK)
    {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

#  if defined(nginx_version) && (nginx_version >= 8011)
    r->main->count++;
#  endif

    ngx_http_cache_viewer_handler(r);

    return NGX_DONE;
}
# endif /* NGX_HTTP_FASTCGI */

# if (NGX_HTTP_PROXY)
extern ngx_module_t  ngx_http_proxy_module;

typedef struct {
    ngx_str_t                      key_start;
    ngx_str_t                      schema;
    ngx_str_t                      host_header;
    ngx_str_t                      port;
    ngx_str_t                      uri;
} ngx_http_proxy_vars_t;

typedef struct {
    ngx_http_upstream_conf_t       upstream;

    ngx_array_t                   *flushes;
    ngx_array_t                   *body_set_len;
    ngx_array_t                   *body_set;
    ngx_array_t                   *headers_set_len;
    ngx_array_t                   *headers_set;
    ngx_hash_t                     headers_set_hash;

    ngx_array_t                   *headers_source;
#  if defined(nginx_version) && (nginx_version < 8040)
    ngx_array_t                   *headers_names;
#  endif /* nginx_version < 8040 */

    ngx_array_t                   *proxy_lengths;
    ngx_array_t                   *proxy_values;

    ngx_array_t                   *redirects;
#  if defined(nginx_version) && (nginx_version >= 1001015)
    ngx_array_t                   *cookie_domains;
    ngx_array_t                   *cookie_paths;
#  endif /* nginx_version >= 1001015 */

    ngx_str_t                      body_source;

    ngx_str_t                      method;
    ngx_str_t                      location;
    ngx_str_t                      url;

    ngx_http_complex_value_t       cache_key;

    ngx_http_proxy_vars_t          vars;

    ngx_flag_t                     redirect;

#  if defined(nginx_version) && (nginx_version >= 1001004)
    ngx_uint_t                     http_version;
#  endif /* nginx_version >= 1001004 */

    ngx_uint_t                     headers_hash_max_size;
    ngx_uint_t                     headers_hash_bucket_size;
} ngx_http_proxy_loc_conf_t;

char *
ngx_http_proxy_cache_viewer_conf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_compile_complex_value_t   ccv;
    ngx_http_cache_viewer_loc_conf_t   *cplcf;
    ngx_http_core_loc_conf_t          *clcf;
    ngx_http_proxy_loc_conf_t         *plcf;
    ngx_str_t                         *value;

    cplcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_cache_viewer_module);

    /* check for duplicates / collisions */
    if (cplcf->proxy.enable != NGX_CONF_UNSET) {
        return "is duplicate";
    }

    if (cf->args->nelts != 3) {
        return ngx_http_cache_viewer_conf(cf, &cplcf->proxy);
    }

    if (cf->cmd_type & (NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF)) {
        return "(separate location syntax) is not allowed here";
    }

    plcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_proxy_module);

    if (plcf->upstream.cache != NGX_CONF_UNSET_PTR
        && plcf->upstream.cache != NULL)
    {
        return "is incompatible with \"proxy_cache\"";
    }

    if (plcf->upstream.upstream || plcf->proxy_lengths) {
        return "is incompatible with \"proxy_pass\"";
    }

    if (plcf->upstream.store > 0 || plcf->upstream.store_lengths) {
        return "is incompatible with \"proxy_store\"";
    }

    value = cf->args->elts;

    /* set proxy_cache part */
    plcf->upstream.cache = ngx_shared_memory_add(cf, &value[1], 0,
                                                 &ngx_http_proxy_module);
    if (plcf->upstream.cache == NULL) {
        return NGX_CONF_ERROR;
    }

    /* set proxy_cache_key part */
    ngx_memzero(&ccv, sizeof(ngx_http_compile_complex_value_t));

    ccv.cf = cf;
    ccv.value = &value[2];
    ccv.complex_value = &plcf->cache_key;

    if (ngx_http_compile_complex_value(&ccv) != NGX_OK) {
        return NGX_CONF_ERROR;
    }

    /* set handler */
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

    cplcf->proxy.enable = 0;
    clcf->handler = ngx_http_proxy_cache_viewer_handler;

    return NGX_CONF_OK;
}

ngx_int_t
ngx_http_proxy_cache_viewer_handler(ngx_http_request_t *r)
{
    ngx_http_proxy_loc_conf_t  *plcf;

    plcf = ngx_http_get_module_loc_conf(r, ngx_http_proxy_module);

    if (ngx_http_cache_viewer_init(r, plcf->upstream.cache->data,
                                  &plcf->cache_key)
        != NGX_OK)
    {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

#  if defined(nginx_version) && (nginx_version >= 8011)
    r->main->count++;
#  endif

    ngx_http_cache_viewer_handler(r);

    return NGX_DONE;
}
# endif /* NGX_HTTP_PROXY */

# if (NGX_HTTP_SCGI)
extern ngx_module_t  ngx_http_scgi_module;

typedef struct {
    ngx_http_upstream_conf_t   upstream;

    ngx_array_t               *flushes;
    ngx_array_t               *params_len;
    ngx_array_t               *params;
    ngx_array_t               *params_source;

    ngx_hash_t                 headers_hash;
    ngx_uint_t                 header_params;

    ngx_array_t               *scgi_lengths;
    ngx_array_t               *scgi_values;

    ngx_http_complex_value_t   cache_key;
} ngx_http_scgi_loc_conf_t;

char *
ngx_http_scgi_cache_viewer_conf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_compile_complex_value_t   ccv;
    ngx_http_cache_viewer_loc_conf_t   *cplcf;
    ngx_http_core_loc_conf_t          *clcf;
    ngx_http_scgi_loc_conf_t          *slcf;
    ngx_str_t                         *value;

    cplcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_cache_viewer_module);

    /* check for duplicates / collisions */
    if (cplcf->scgi.enable != NGX_CONF_UNSET) {
        return "is duplicate";
    }

    if (cf->args->nelts != 3) {
        return ngx_http_cache_viewer_conf(cf, &cplcf->scgi);
    }

    if (cf->cmd_type & (NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF)) {
        return "(separate location syntax) is not allowed here";
    }

    slcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_scgi_module);

    if (slcf->upstream.cache != NGX_CONF_UNSET_PTR
        && slcf->upstream.cache != NULL)
    {
        return "is incompatible with \"scgi_cache\"";
    }

    if (slcf->upstream.upstream || slcf->scgi_lengths) {
        return "is incompatible with \"scgi_pass\"";
    }

    if (slcf->upstream.store > 0 || slcf->upstream.store_lengths) {
        return "is incompatible with \"scgi_store\"";
    }

    value = cf->args->elts;

    /* set scgi_cache part */
    slcf->upstream.cache = ngx_shared_memory_add(cf, &value[1], 0,
                                                 &ngx_http_scgi_module);
    if (slcf->upstream.cache == NULL) {
        return NGX_CONF_ERROR;
    }

    /* set scgi_cache_key part */
    ngx_memzero(&ccv, sizeof(ngx_http_compile_complex_value_t));

    ccv.cf = cf;
    ccv.value = &value[2];
    ccv.complex_value = &slcf->cache_key;

    if (ngx_http_compile_complex_value(&ccv) != NGX_OK) {
        return NGX_CONF_ERROR;
    }

    /* set handler */
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

    cplcf->scgi.enable = 0;
    clcf->handler = ngx_http_scgi_cache_viewer_handler;

    return NGX_CONF_OK;
}

ngx_int_t
ngx_http_scgi_cache_viewer_handler(ngx_http_request_t *r)
{
    ngx_http_scgi_loc_conf_t  *slcf;

    slcf = ngx_http_get_module_loc_conf(r, ngx_http_scgi_module);

    if (ngx_http_cache_viewer_init(r, slcf->upstream.cache->data,
                                  &slcf->cache_key)
        != NGX_OK)
    {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

#  if defined(nginx_version) && (nginx_version >= 8011)
    r->main->count++;
#  endif

    ngx_http_cache_viewer_handler(r);

    return NGX_DONE;
}
# endif /* NGX_HTTP_SCGI */

# if (NGX_HTTP_UWSGI)
extern ngx_module_t  ngx_http_uwsgi_module;

typedef struct {
    ngx_http_upstream_conf_t   upstream;

    ngx_array_t               *flushes;
    ngx_array_t               *params_len;
    ngx_array_t               *params;
    ngx_array_t               *params_source;

    ngx_hash_t                 headers_hash;
    ngx_uint_t                 header_params;

    ngx_array_t               *uwsgi_lengths;
    ngx_array_t               *uwsgi_values;

    ngx_http_complex_value_t   cache_key;

    ngx_str_t                  uwsgi_string;

    ngx_uint_t                 modifier1;
    ngx_uint_t                 modifier2;
} ngx_http_uwsgi_loc_conf_t;

char *
ngx_http_uwsgi_cache_viewer_conf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_compile_complex_value_t   ccv;
    ngx_http_cache_viewer_loc_conf_t   *cplcf;
    ngx_http_core_loc_conf_t          *clcf;
    ngx_http_uwsgi_loc_conf_t         *ulcf;
    ngx_str_t                         *value;

    cplcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_cache_viewer_module);

    /* check for duplicates / collisions */
    if (cplcf->uwsgi.enable != NGX_CONF_UNSET) {
        return "is duplicate";
    }

    if (cf->args->nelts != 3) {
        return ngx_http_cache_viewer_conf(cf, &cplcf->uwsgi);
    }

    if (cf->cmd_type & (NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF)) {
        return "(separate location syntax) is not allowed here";
    }

    ulcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_uwsgi_module);

    if (ulcf->upstream.cache != NGX_CONF_UNSET_PTR
        && ulcf->upstream.cache != NULL)
    {
        return "is incompatible with \"uwsgi_cache\"";
    }

    if (ulcf->upstream.upstream || ulcf->uwsgi_lengths) {
        return "is incompatible with \"uwsgi_pass\"";
    }

    if (ulcf->upstream.store > 0 || ulcf->upstream.store_lengths) {
        return "is incompatible with \"uwsgi_store\"";
    }

    value = cf->args->elts;

    /* set uwsgi_cache part */
    ulcf->upstream.cache = ngx_shared_memory_add(cf, &value[1], 0,
                                                 &ngx_http_uwsgi_module);
    if (ulcf->upstream.cache == NULL) {
        return NGX_CONF_ERROR;
    }

    /* set uwsgi_cache_key part */
    ngx_memzero(&ccv, sizeof(ngx_http_compile_complex_value_t));

    ccv.cf = cf;
    ccv.value = &value[2];
    ccv.complex_value = &ulcf->cache_key;

    if (ngx_http_compile_complex_value(&ccv) != NGX_OK) {
        return NGX_CONF_ERROR;
    }

    /* set handler */
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

    cplcf->uwsgi.enable = 0;
    clcf->handler = ngx_http_uwsgi_cache_viewer_handler;

    return NGX_CONF_OK;
}

ngx_int_t
ngx_http_uwsgi_cache_viewer_handler(ngx_http_request_t *r)
{
    ngx_http_uwsgi_loc_conf_t  *ulcf;

    ulcf = ngx_http_get_module_loc_conf(r, ngx_http_uwsgi_module);

    if (ngx_http_cache_viewer_init(r, ulcf->upstream.cache->data,
                                  &ulcf->cache_key)
        != NGX_OK)
    {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

#  if defined(nginx_version) && (nginx_version >= 8011)
    r->main->count++;
#  endif

    ngx_http_cache_viewer_handler(r);

    return NGX_DONE;
}
# endif /* NGX_HTTP_UWSGI */

ngx_int_t
ngx_http_cache_viewer_access_handler(ngx_http_request_t *r)
{
    ngx_http_cache_viewer_loc_conf_t   *cplcf;

    cplcf = ngx_http_get_module_loc_conf(r, ngx_http_cache_viewer_module);

    if (r->method_name.len != cplcf->conf->method.len
        || (ngx_strncmp(r->method_name.data, cplcf->conf->method.data,
                        r->method_name.len)))
    {
        return cplcf->original_handler(r);
    }

    if ((cplcf->conf->access || cplcf->conf->access6)
         && ngx_http_cache_viewer_access(cplcf->conf->access,
                                        cplcf->conf->access6,
                                        r->connection->sockaddr) != NGX_OK)
    {
        return NGX_HTTP_FORBIDDEN;
    }

    if (cplcf->handler == NULL) {
        return NGX_HTTP_NOT_FOUND;
    }

    return cplcf->handler(r);
}

ngx_int_t
ngx_http_cache_viewer_access(ngx_array_t *access, ngx_array_t *access6,
    struct sockaddr *s)
{
    in_addr_t         inaddr;
    ngx_in_cidr_t    *a;
    ngx_uint_t        i;
# if (NGX_HAVE_INET6)
    struct in6_addr  *inaddr6;
    ngx_in6_cidr_t   *a6;
    u_char           *p;
    ngx_uint_t        n;
# endif /* NGX_HAVE_INET6 */

    switch (s->sa_family) {
    case AF_INET:
        if (access == NULL) {
            return NGX_DECLINED;
        }

        inaddr = ((struct sockaddr_in *) s)->sin_addr.s_addr;

# if (NGX_HAVE_INET6)
    ipv4:
# endif /* NGX_HAVE_INET6 */

        a = access->elts;
        for (i = 0; i < access->nelts; i++) {
            if ((inaddr & a[i].mask) == a[i].addr) {
                return NGX_OK;
            }
        }

        return NGX_DECLINED;

# if (NGX_HAVE_INET6)
    case AF_INET6:
        inaddr6 = &((struct sockaddr_in6 *) s)->sin6_addr;
        p = inaddr6->s6_addr;

        if (access && IN6_IS_ADDR_V4MAPPED(inaddr6)) {
            inaddr = p[12] << 24;
            inaddr += p[13] << 16;
            inaddr += p[14] << 8;
            inaddr += p[15];
            inaddr = htonl(inaddr);

            goto ipv4;
        }

        if (access6 == NULL) {
            return NGX_DECLINED;
        }

        a6 = access6->elts;
        for (i = 0; i < access6->nelts; i++) {
            for (n = 0; n < 16; n++) {
                if ((p[n] & a6[i].mask.s6_addr[n]) != a6[i].addr.s6_addr[n]) {
                    goto next;
                }
            }

            return NGX_OK;

        next:
            continue;
        }

        return NGX_DECLINED;
# endif /* NGX_HAVE_INET6 */
    }

    return NGX_DECLINED;
}

#if (NGX_DEBUG)
static ngx_uint_t
ngx_http_viewer_cache_node_count(ngx_http_file_cache_t *cache)
{
    ngx_queue_t                 *q;
    ngx_uint_t                   count;
    ngx_http_file_cache_node_t  *fcn;
    
    count = 0;
    
    ngx_shmtx_lock(&cache->shpool->mutex);

    for (q = ngx_queue_last(&cache->sh->queue);
         q != ngx_queue_sentinel(&cache->sh->queue);
         q = ngx_queue_prev(q))
    {
        fcn = ngx_queue_data(q, ngx_http_file_cache_node_t, queue);
        
        ngx_log_debug6(NGX_LOG_DEBUG_HTTP, ngx_cycle->log, 0,
                  "http viewer file cache: #%d %d %02xd%02xd%02xd%02xd",
                  fcn->count, fcn->exists,
                  fcn->key[0], fcn->key[1], fcn->key[2], fcn->key[3]);
    
        count++;
    }

    ngx_shmtx_unlock(&cache->shpool->mutex);
    
    return count;
}
#endif

static ngx_int_t
ngx_http_viewer_cache_shm_info(ngx_http_request_t *r, ngx_chain_t *out)
{
    size_t                  len;
    ngx_buf_t              *b;
    ngx_http_file_cache_t  *cache;
    ngx_shm_zone_t         *shm_zone;
    
    cache = r->cache->file_cache;
    shm_zone = cache->shm_zone;
    
    len = NGX_VIEWER_LEN(SHM_TITLE_LINE) + NGX_VIEWER_LEN(CRLF) +
          NGX_VIEWER_LEN(SHM_NAME_HDR) + shm_zone->shm.name.len + NGX_VIEWER_LEN(CRLF) +
          NGX_VIEWER_LEN(SHM_MEM_SIZE_HDR) + NGX_OFF_T_LEN + NGX_VIEWER_LEN(CRLF) + 
          NGX_VIEWER_LEN(SHM_DISK_BLKS_HDR) + NGX_OFF_T_LEN + NGX_VIEWER_LEN(CRLF) +
          NGX_VIEWER_LEN(SHM_USED_DISK_BLKS_HDR) + NGX_OFF_T_LEN + NGX_VIEWER_LEN(CRLF) +
          NGX_VIEWER_LEN(SHM_DISK_BLK_SIZE_HDR) + NGX_OFF_T_LEN + NGX_VIEWER_LEN(CRLF) +
          NGX_VIEWER_LEN(SHM_NODE_INACTIVE_TIME) + NGX_OFF_T_LEN + NGX_VIEWER_LEN(CRLF) +
#if (NGX_DEBUG)
          NGX_VIEWER_LEN(SHM_FILE_COUNT_HDR) + NGX_OFF_T_LEN + NGX_VIEWER_LEN(CRLF) +
#endif
          NGX_VIEWER_LEN(SHM_PATH_HDR) + cache->path->len + NGX_VIEWER_LEN(CRLF);

    b = ngx_create_temp_buf(r->pool, len);
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    //  shm title line
    b->last = ngx_sprintf(b->last, SHM_TITLE_LINE CRLF);
    
    //  shm name
    b->last = ngx_sprintf(b->last, SHM_NAME_HDR "%V" CRLF, &shm_zone->shm.name);
    
    //  shm memory size
    b->last = ngx_sprintf(b->last, SHM_MEM_SIZE_HDR "%z" CRLF, (shm_zone->shm.size / 1024));
    
#if (NGX_DEBUG)
    //  node count
    b->last = ngx_sprintf(b->last, SHM_FILE_COUNT_HDR "%ud" CRLF,
                          ngx_http_viewer_cache_node_count(cache));
#endif
    
    //  cache path
    b->last = ngx_sprintf(b->last, SHM_PATH_HDR "%V" CRLF, &cache->path->name);
    
    //  disk blocks count
    b->last = ngx_sprintf(b->last, SHM_DISK_BLKS_HDR "%O" CRLF, cache->max_size);
    
    //  used disk blocks
    ngx_shmtx_lock(&cache->shpool->mutex);
    b->last = ngx_sprintf(b->last, SHM_USED_DISK_BLKS_HDR "%O" CRLF, cache->sh->size);
    ngx_shmtx_unlock(&cache->shpool->mutex);
    
    //  disk block size
    b->last = ngx_sprintf(b->last, SHM_DISK_BLK_SIZE_HDR "%z" CRLF, cache->bsize);
    
    //  node inactive time
    b->last = ngx_sprintf(b->last, SHM_NODE_INACTIVE_TIME "%T" CRLF, cache->inactive);
    
    b->last = ngx_sprintf(b->last, SYMBOL_SEPARATOR CRLF);
    
    out->buf = b;
    out->next = NULL;
    
    return NGX_OK;
}

static ngx_int_t
ngx_http_viewer_cache_node_info(ngx_http_request_t *r, ngx_chain_t *out)
{
    ngx_http_file_cache_t       *cache;
    ngx_http_cache_t            *c;
    ngx_uint_t                   exists;
    size_t                       len;
    ngx_buf_t                   *b;
    
    len = 0;
    c = r->cache;
    cache = c->file_cache;
    
    if (c->node) {
        
        ngx_shmtx_lock(&cache->shpool->mutex);
        exists = c->node->exists;
        ngx_shmtx_unlock(&cache->shpool->mutex);
        
        if (exists) {
            len = NGX_VIEWER_LEN(CACHE_NODE_FILENAME) + c->file.name.len + NGX_VIEWER_LEN(CRLF);
        }
        
        len += NGX_VIEWER_LEN(CACHE_NODE_VALID_SEC) + NGX_OFF_T_LEN + NGX_VIEWER_LEN(CRLF)
             + NGX_VIEWER_LEN(CACHE_NODE_COUNT) + NGX_OFF_T_LEN + NGX_VIEWER_LEN(CRLF)
             + NGX_VIEWER_LEN(CACHE_NODE_USES) + NGX_OFF_T_LEN + NGX_VIEWER_LEN(CRLF)
             + NGX_VIEWER_LEN(CACHE_NODE_EXPIRE) + NGX_OFF_T_LEN + NGX_VIEWER_LEN(CRLF)
             + NGX_VIEWER_LEN(CACHE_NODE_BODYSTART) + NGX_OFF_T_LEN + NGX_VIEWER_LEN(CRLF)
             + NGX_VIEWER_LEN(CACHE_NODE_FS_SIZE) + NGX_OFF_T_LEN + NGX_VIEWER_LEN(CRLF)
             + NGX_VIEWER_LEN(CACHE_NODE_EXISTS) + 1 + NGX_VIEWER_LEN(CRLF)
             + NGX_VIEWER_LEN(CACHE_NODE_UPDATING) + 1 + NGX_VIEWER_LEN(CRLF)
             + NGX_VIEWER_LEN(CACHE_NODE_DELETING) + 1 + NGX_VIEWER_LEN(CRLF);
    
    } else {
        return NGX_HTTP_NOT_FOUND;
    }
    
    b = ngx_create_temp_buf(r->pool, len);
    if (b == NULL) {
        return NGX_ERROR;
    }
    
    if (exists) {
        //  cache filename
        b->last = ngx_sprintf(b->last, CACHE_NODE_FILENAME "%V" CRLF, &c->file.name);
    }
    
    ngx_shmtx_lock(&cache->shpool->mutex);
    
    //  cache valid second 
    b->last = ngx_sprintf(b->last, CACHE_NODE_VALID_SEC "%T" CRLF, c->node->valid_sec);
    
    //  cache count
    b->last = ngx_sprintf(b->last, CACHE_NODE_COUNT "%ui" CRLF, c->node->count);
    
    //  cache uses
    b->last = ngx_sprintf(b->last, CACHE_NODE_USES "%ui" CRLF, c->node->uses);
    
    //  cache expire
    b->last = ngx_sprintf(b->last, CACHE_NODE_EXPIRE "%T" CRLF, c->node->expire);
    
    if (exists) {
        //  cache body_start
        b->last = ngx_sprintf(b->last, CACHE_NODE_BODYSTART "%z" CRLF, c->node->body_start);
        
        //  cache fs_size
        b->last = ngx_sprintf(b->last, CACHE_NODE_FS_SIZE "%O" CRLF, c->node->fs_size);
    }
    
    //  cache exists
    b->last = ngx_sprintf(b->last, CACHE_NODE_EXISTS "%ui" CRLF, exists);
    
    //  cache updating
    b->last = ngx_sprintf(b->last, CACHE_NODE_UPDATING "%ui" CRLF, c->node->updating);
    
    //  cache deleting
    b->last = ngx_sprintf(b->last, CACHE_NODE_DELETING "%ui" CRLF, c->node->deleting);
    
    ngx_shmtx_unlock(&cache->shpool->mutex);
    
    out->buf = b;
    out->next = NULL;

    return NGX_OK;
}

ngx_int_t
ngx_http_cache_viewer_send_response(ngx_http_request_t *r, ngx_uint_t flag)
{
    ngx_chain_t             shm_out, node_out;
    ngx_int_t               rc;
    size_t                  len;
    
    len = 0;
    
    if (flag) {
        
        rc = ngx_http_viewer_cache_node_info(r, &node_out);
        if (rc == NGX_ERROR) {
            return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }
        
        if (rc == NGX_HTTP_NOT_FOUND) {
            return rc;
        }
    }
    
    rc = ngx_http_viewer_cache_shm_info(r, &shm_out);
    if (rc != NGX_OK) {
        return rc;
    }
    
    len = shm_out.buf->last - shm_out.buf->start;
    
    if (flag) {
        len += node_out.buf->last - node_out.buf->start;
        shm_out.next = &node_out;
        node_out.buf->last_buf = 1;
    } else {
        shm_out.buf->last_buf = 1;
    }
    
    if (r->method == NGX_HTTP_HEAD) {
        r->header_only = 1;
    }
    
    r->headers_out.status = NGX_HTTP_OK;
    ngx_str_set(&r->headers_out.content_type, "text/plain");
    r->headers_out.content_length_n = len;

    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
       return rc;
    }

    return ngx_http_output_filter(r, &shm_out);
}

ngx_int_t
ngx_http_cache_viewer_init(ngx_http_request_t *r, ngx_http_file_cache_t *cache,
    ngx_http_complex_value_t *cache_key)
{
    ngx_http_cache_t  *c;
    ngx_str_t         *key;
    ngx_int_t          rc;

    rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK) {
        return NGX_ERROR;
    }

    c = ngx_pcalloc(r->pool, sizeof(ngx_http_cache_t));
    if (c == NULL) {
        return NGX_ERROR;
    }

    rc = ngx_array_init(&c->keys, r->pool, 1, sizeof(ngx_str_t));
    if (rc != NGX_OK) {
        return NGX_ERROR;
    }

    key = ngx_array_push(&c->keys);
    if (key == NULL) {
        return NGX_ERROR;
    }

    rc = ngx_http_complex_value(r, cache_key, key);
    if (rc != NGX_OK) {
        return NGX_ERROR;
    }

    r->cache = c;
    c->body_start = ngx_pagesize;
    c->file_cache = cache;
    c->file.log = r->connection->log;

    ngx_http_file_cache_create_key(r);

    return NGX_OK;
}

void
ngx_http_cache_viewer_handler(ngx_http_request_t *r)
{
    ngx_int_t        rc;
    
    if (r->uri.data[r->uri.len - 1] == '/') {
        ngx_http_finalize_request(r, ngx_http_cache_viewer_send_response(r, 0));
        return;
    }
    
#  if (NGX_HAVE_FILE_AIO)
    if (r->aio) {
        return;
    }
#  endif

    rc = ngx_http_file_cache_viewer(r);

    ngx_log_debug2(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http file cache viewer: %i, \"%s\"",
                   rc, r->cache->file.name.data);

    switch (rc) {
    case NGX_OK:
        r->write_event_handler = ngx_http_request_empty_handler;
        ngx_http_finalize_request(r, ngx_http_cache_viewer_send_response(r, 1));
        return;
    case NGX_DECLINED:
        ngx_http_finalize_request(r, NGX_HTTP_NOT_FOUND);
        return;
#  if (NGX_HAVE_FILE_AIO)
    case NGX_AGAIN:
        r->write_event_handler = ngx_http_cache_viewer_handler;
        return;
#  endif
    default:
        ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
    }
}

ngx_int_t
ngx_http_file_cache_viewer(ngx_http_request_t *r)
{
    ngx_http_file_cache_t  *cache;
    ngx_http_cache_t       *c;

    switch (ngx_http_file_cache_open(r)) {
    case NGX_OK:
    case NGX_HTTP_CACHE_STALE:
#  if defined(nginx_version) \
      && ((nginx_version >= 8001) \
          || ((nginx_version < 8000) && (nginx_version >= 7060)))
    case NGX_HTTP_CACHE_UPDATING:
#  endif
        break;
    case NGX_DECLINED:
        break;
#  if (NGX_HAVE_FILE_AIO)
    case NGX_AGAIN:
        return NGX_AGAIN;
#  endif
    default:
        return NGX_ERROR;
    }
    
    c = r->cache;
    cache = c->file_cache;
    
    ngx_shmtx_lock(&cache->shpool->mutex);
    
    if (!c->node->exists && c->node->uses == 1) {
        c->min_uses = 1;
        ngx_shmtx_unlock(&cache->shpool->mutex);
        return NGX_DECLINED;
    }
   
    ngx_shmtx_unlock(&cache->shpool->mutex);

    return NGX_OK;
}

char *
ngx_http_cache_viewer_conf(ngx_conf_t *cf, ngx_http_cache_viewer_conf_t *cpcf)
{
    ngx_cidr_t       cidr;
    ngx_in_cidr_t   *access;
# if (NGX_HAVE_INET6)
    ngx_in6_cidr_t  *access6;
# endif /* NGX_HAVE_INET6 */
    ngx_str_t       *value;
    ngx_int_t        rc;
    ngx_uint_t       i;

    value = cf->args->elts;

    if (ngx_strcmp(value[1].data, (u_char *) "off") == 0) {
        cpcf->enable = 0;
        return NGX_CONF_OK;

    } else if (ngx_strcmp(value[1].data, (u_char *) "on") == 0) {
        ngx_str_set(&cpcf->method, "VIEWER");

    } else {
        cpcf->method = value[1];
    }

    if (cf->args->nelts < 4) {
        cpcf->enable = 1;
        return NGX_CONF_OK;
    }

    /* sanity check */
    if (ngx_strcmp(value[2].data, (u_char *) "from") != 0) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "invalid parameter \"%V\", expected"
                           " \"from\" keyword", &value[2]);
        return NGX_CONF_ERROR;
    }

    if (ngx_strcmp(value[3].data, (u_char *) "all") == 0) {
        cpcf->enable = 1;
        return NGX_CONF_OK;
    }

    for (i = 3; i < cf->args->nelts; i++) {
        rc = ngx_ptocidr(&value[i], &cidr);

        if (rc == NGX_ERROR) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "invalid parameter \"%V\"", &value[i]);
            return NGX_CONF_ERROR;
        }

        if (rc == NGX_DONE) {
            ngx_conf_log_error(NGX_LOG_WARN, cf, 0,
                               "low address bits of %V are meaningless",
                               &value[i]);
        }

        switch (cidr.family) {
        case AF_INET:
            if (cpcf->access == NULL) {
                cpcf->access = ngx_array_create(cf->pool, cf->args->nelts - 3,
                                                sizeof(ngx_in_cidr_t));
                if (cpcf->access == NULL) {
                    return NGX_CONF_ERROR;
                }
            }

            access = ngx_array_push(cpcf->access);
            if (access == NULL) {
                return NGX_CONF_ERROR;
            }

            access->mask = cidr.u.in.mask;
            access->addr = cidr.u.in.addr;

            break;

# if (NGX_HAVE_INET6)
        case AF_INET6:
            if (cpcf->access6 == NULL) {
                cpcf->access6 = ngx_array_create(cf->pool, cf->args->nelts - 3,
                                                 sizeof(ngx_in6_cidr_t));
                if (cpcf->access6 == NULL) {
                    return NGX_CONF_ERROR;
                }
            }

            access6 = ngx_array_push(cpcf->access6);
            if (access6 == NULL) {
                return NGX_CONF_ERROR;
            }

            access6->mask = cidr.u.in6.mask;
            access6->addr = cidr.u.in6.addr;

            break;
# endif /* NGX_HAVE_INET6 */
        }
    }

    cpcf->enable = 1;

    return NGX_CONF_OK;
}

void
ngx_http_cache_viewer_merge_conf(ngx_http_cache_viewer_conf_t *conf,
    ngx_http_cache_viewer_conf_t *prev)
{
    if (conf->enable == NGX_CONF_UNSET) {
        if (prev->enable == 1) {
            conf->enable = prev->enable;
            conf->method = prev->method;
            conf->access = prev->access;
            conf->access6 = prev->access6;

        } else {
            conf->enable = 0;
        }
    }
}

void *
ngx_http_cache_viewer_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_cache_viewer_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_cache_viewer_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    /*
     * set by ngx_pcalloc():
     *
     *     conf->*.method = { 0, NULL }
     *     conf->*.access = NULL
     *     conf->*.access6 = NULL
     */

# if (NGX_HTTP_FASTCGI)
    conf->fastcgi.enable = NGX_CONF_UNSET;
# endif /* NGX_HTTP_FASTCGI */
# if (NGX_HTTP_PROXY)
    conf->proxy.enable = NGX_CONF_UNSET;
# endif /* NGX_HTTP_PROXY */
# if (NGX_HTTP_SCGI)
    conf->scgi.enable = NGX_CONF_UNSET;
# endif /* NGX_HTTP_SCGI */
# if (NGX_HTTP_UWSGI)
    conf->uwsgi.enable = NGX_CONF_UNSET;
# endif /* NGX_HTTP_UWSGI */

    conf->conf = NGX_CONF_UNSET_PTR;
    conf->handler = NGX_CONF_UNSET_PTR;
    conf->original_handler = NGX_CONF_UNSET_PTR;

    return conf;
}

char *
ngx_http_cache_viewer_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_cache_viewer_loc_conf_t  *prev = parent;
    ngx_http_cache_viewer_loc_conf_t  *conf = child;
    ngx_http_core_loc_conf_t         *clcf;
# if (NGX_HTTP_FASTCGI)
    ngx_http_fastcgi_loc_conf_t      *flcf;
# endif /* NGX_HTTP_FASTCGI */
# if (NGX_HTTP_PROXY)
    ngx_http_proxy_loc_conf_t        *plcf;
# endif /* NGX_HTTP_PROXY */
# if (NGX_HTTP_SCGI)
    ngx_http_scgi_loc_conf_t         *slcf;
# endif /* NGX_HTTP_SCGI */
# if (NGX_HTTP_UWSGI)
    ngx_http_uwsgi_loc_conf_t        *ulcf;
# endif /* NGX_HTTP_UWSGI */

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

# if (NGX_HTTP_FASTCGI)
    ngx_http_cache_viewer_merge_conf(&conf->fastcgi, &prev->fastcgi);

    if (conf->fastcgi.enable && clcf->handler != NULL) {
        flcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_fastcgi_module);

        if (flcf->upstream.upstream || flcf->fastcgi_lengths) {
            conf->conf = &conf->fastcgi;
            conf->handler = flcf->upstream.cache
                          ? ngx_http_fastcgi_cache_viewer_handler : NULL;
            conf->original_handler = clcf->handler;

            clcf->handler = ngx_http_cache_viewer_access_handler;

            return NGX_CONF_OK;
        }
    }
# endif /* NGX_HTTP_FASTCGI */

# if (NGX_HTTP_PROXY)
    ngx_http_cache_viewer_merge_conf(&conf->proxy, &prev->proxy);

    if (conf->proxy.enable && clcf->handler != NULL) {
        plcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_proxy_module);

        if (plcf->upstream.upstream || plcf->proxy_lengths) {
            conf->conf = &conf->proxy;
            conf->handler = plcf->upstream.cache
                          ? ngx_http_proxy_cache_viewer_handler : NULL;
            conf->original_handler = clcf->handler;

            clcf->handler = ngx_http_cache_viewer_access_handler;

            return NGX_CONF_OK;
        }
    }
# endif /* NGX_HTTP_PROXY */

# if (NGX_HTTP_SCGI)
    ngx_http_cache_viewer_merge_conf(&conf->scgi, &prev->scgi);

    if (conf->scgi.enable && clcf->handler != NULL) {
        slcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_scgi_module);

        if (slcf->upstream.upstream || slcf->scgi_lengths) {
            conf->conf = &conf->scgi;
            conf->handler = slcf->upstream.cache
                          ? ngx_http_scgi_cache_viewer_handler : NULL;
            conf->original_handler = clcf->handler;
            clcf->handler = ngx_http_cache_viewer_access_handler;

            return NGX_CONF_OK;
        }
    }
# endif /* NGX_HTTP_SCGI */

# if (NGX_HTTP_UWSGI)
    ngx_http_cache_viewer_merge_conf(&conf->uwsgi, &prev->uwsgi);

    if (conf->uwsgi.enable && clcf->handler != NULL) {
        ulcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_uwsgi_module);

        if (ulcf->upstream.upstream || ulcf->uwsgi_lengths) {
            conf->conf = &conf->uwsgi;
            conf->handler = ulcf->upstream.cache
                          ? ngx_http_uwsgi_cache_viewer_handler : NULL;
            conf->original_handler = clcf->handler;

            clcf->handler = ngx_http_cache_viewer_access_handler;

            return NGX_CONF_OK;
        }
    }
# endif /* NGX_HTTP_UWSGI */

    ngx_conf_merge_ptr_value(conf->conf, prev->conf, NULL);
    ngx_conf_merge_ptr_value(conf->handler, prev->handler, NULL);
    ngx_conf_merge_ptr_value(conf->original_handler, prev->original_handler,
                             NULL);

    return NGX_CONF_OK;
}

#else /* !NGX_HTTP_CACHE */

static ngx_http_module_t  ngx_http_cache_viewer_module_ctx = {
    NULL,                       /* preconfiguration */
    NULL,                       /* postconfiguration */
    NULL,                       /* create main configuration */
    NULL,                       /* init main configuration */
    NULL,                       /* create server configuration */
    NULL,                       /* merge server configuration */
    NULL,                       /* create location configuration */
    NULL,                       /* merge location configuration */
};

ngx_module_t  ngx_http_cache_viewer_module = {
    NGX_MODULE_V1,
    &ngx_http_cache_viewer_module_ctx, /* module context */
    NULL,                              /* module directives */
    NGX_HTTP_MODULE,                   /* module type */
    NULL,                              /* init master */
    NULL,                              /* init module */
    NULL,                              /* init process */
    NULL,                              /* init thread */
    NULL,                              /* exit thread */
    NULL,                              /* exit process */
    NULL,                              /* exit master */
    NGX_MODULE_V1_PADDING
};

#endif /* NGX_HTTP_CACHE */
