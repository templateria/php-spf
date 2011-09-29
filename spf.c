/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2011 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Pedro Padron <ppadron@php.net>                               |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_exceptions.h"
#include "php_spf.h"
#include "netinet/in.h"
#include "spf2/spf.h"
#include "spf2/spf_lib_version.h"

zend_class_entry *spf_ce_Spf;
zend_class_entry *spf_ce_SpfResponse;
zend_class_entry *spf_ce_SpfException;

/* {{{ ZEND_BEGIN_ARG_INFO */
ZEND_BEGIN_ARG_INFO(arginfo_Spf___construct, 0)
	ZEND_ARG_INFO(0, serverType)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_Spf_query, 0, 0, 0)
    ZEND_ARG_INFO(0, ip_address)
    ZEND_ARG_INFO(0, helo)
    ZEND_ARG_INFO(0, sender)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO(arginfo_SpfResponse_getResult, 0)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO(arginfo_SpfResponse_getHeaderComment, 0)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO(arginfo_SpfResponse_getReceivedSpf, 0)
ZEND_END_ARG_INFO();
/* }}} */

/* True global resources - no need for thread safety here */
static int le_spf;

/* {{{ spf_methods[]
 */
const zend_function_entry spf_methods[] = {
	PHP_ME(Spf, __construct, arginfo_Spf___construct, ZEND_ACC_PUBLIC)
    PHP_ME(Spf, query, arginfo_Spf_query, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};
/* }}} */

const zend_function_entry spf_response_methods[] = {
	PHP_ME(SpfResponse, getResult, arginfo_SpfResponse_getResult, ZEND_ACC_PUBLIC)
	PHP_ME(SpfResponse, getHeaderComment, arginfo_SpfResponse_getResult, ZEND_ACC_PUBLIC)
	PHP_ME(SpfResponse, getReceivedSpf, arginfo_SpfResponse_getReceivedSpf, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}	
};

/* {{{ spf_module_entry
 */
zend_module_entry spf_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"spf",
	NULL,
	PHP_MINIT(spf),
	PHP_MSHUTDOWN(spf),
	PHP_RINIT(spf),
	PHP_RSHUTDOWN(spf),	
	PHP_MINFO(spf),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1",
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SPF
ZEND_GET_MODULE(spf)
#endif

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(spf)
{
    zend_class_entry ce, ce_response, ce_exception;

    INIT_CLASS_ENTRY(ce, "Spf", spf_methods);
	ce.create_object = create_spf;
    spf_ce_Spf = zend_register_internal_class(&ce TSRMLS_CC);

	INIT_CLASS_ENTRY(ce_response, "SpfResponse", spf_response_methods);
	ce_response.create_object = create_spf_response;
	spf_ce_SpfResponse = zend_register_internal_class(&ce_response TSRMLS_CC);

	INIT_CLASS_ENTRY(ce_exception, "SpfException", NULL);
	spf_ce_SpfException = zend_register_internal_class_ex(&ce_exception, (zend_class_entry*) zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(spf)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(spf)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(spf)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(spf)
{
    char version[5];
    sprintf(version, "%d.%d.%d", SPF_LIB_VERSION_MAJOR, SPF_LIB_VERSION_MINOR, SPF_LIB_VERSION_PATCH);
	php_info_print_table_start();
	php_info_print_table_header(2, "spf support", "enabled");
    php_info_print_table_row(2, "libspf2 version", version);
	php_info_print_table_end();
}
/* }}} */

/* {{{ SPF_SERVER_FROM_OBJECT */
#define SPF_SERVER_FROM_OBJECT(spf_server, object) \
{ \
	php_spf_object *obj = (php_spf_object*) zend_object_store_get_object(object TSRMLS_CC); \
	spf_server = obj->spf_server; \
	if (!spf_server) { \
		zend_throw_exception(spf_ce_SpfException, "Invalid or uninitialized SPF object", 0 TSRMLS_CC); \
		RETURN_FALSE; \
	} \
}
/* }}} */

/* {{{ SPF_RESPONSE_FROM_OBJECT */
#define SPF_RESPONSE_FROM_OBJECT(intern, object) \
{ \
    php_spf_response_object *obj = (php_spf_response_object*) zend_object_store_get_object(object TSRMLS_CC); \
    intern = obj->spf_response; \
    if (!intern) { \
        zend_throw_exception(spf_ce_SpfException, "Invalid or uninitialized SPF response", 0 TSRMLS_CC); \
        RETURN_FALSE; \
    } \
}
/* }}} */

/* {{{ create SpfResponse */
zend_object_value create_spf_response(zend_class_entry *class_type TSRMLS_DC)
{
	zend_object_value retval;
	php_spf_response_object *intern;
	zval *tmp;

	intern = (php_spf_response_object*) emalloc(sizeof(php_spf_response_object));
	memset(intern, 0, sizeof(php_spf_response_object));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	zend_hash_copy(intern->std.properties,
		&class_type->default_properties,
		(copy_ctor_func_t) zval_add_ref,
		(void *) &tmp,
		sizeof(zval*));
	retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t) zend_objects_destroy_object, free_spf_response, NULL TSRMLS_CC);
	retval.handlers = zend_get_std_object_handlers();

	return retval;
}
/* }}} */

void free_spf_response(void *object TSRMLS_DC)
{
	php_spf_response_object *intern = (php_spf_response_object*) object;

	if (intern->spf_response) {
		SPF_response_free(intern->spf_response);
	}

	efree(object);
}

/* {{{ create_spf */
zend_object_value create_spf(zend_class_entry *class_type TSRMLS_DC)
{
	zend_object_value retval;
	php_spf_object *intern;
	zval *tmp;

	intern = (php_spf_object*) emalloc(sizeof(php_spf_object));
	memset(intern, 0, sizeof(php_spf_object));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	zend_hash_copy(intern->std.properties,
		&class_type->default_properties,
		(copy_ctor_func_t) zval_add_ref,
		(void *) &tmp,
		sizeof(zval*));

	retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t) zend_objects_destroy_object, free_spf, NULL TSRMLS_CC);
	retval.handlers = zend_get_std_object_handlers();

	return retval;		
}
/* }}} */

/* {{{ free_spf */
void free_spf(void *object TSRMLS_DC)
{
	php_spf_object *spf_object = (php_spf_object*) object;

	if (spf_object->spf_server) {
		SPF_server_free(spf_object->spf_server);
	}

	efree(object);
}
/* }}} */

PHP_METHOD(Spf, __construct)
{
	int server_type = SPF_DNS_CACHE;
	php_spf_object *obj;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &server_type) == FAILURE) {
		RETURN_FALSE;
	}

	obj = (php_spf_object*) zend_object_store_get_object(getThis() TSRMLS_CC);
	obj->spf_server = SPF_server_new(server_type, 0);

	if (!obj->spf_server) zend_throw_exception(spf_ce_SpfException, "could not initialize spf resource", 0 TSRMLS_CC);
}


PHP_METHOD(Spf, query)
{
    int ip_len, helo_len, sender_len, recipient_len;
    char *ip, *helo, *sender, *recipient;
	SPF_server_t *spf_server = NULL;
    SPF_request_t *spf_request = NULL;
    SPF_response_t *spf_response = NULL;
	php_spf_response_object *response;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss|s", &ip, &ip_len, &helo, &helo_len, &sender, &sender_len, &recipient, &recipient_len)) {
        RETURN_FALSE;
    }

    SPF_SERVER_FROM_OBJECT(spf_server, getThis());

    spf_request = SPF_request_new(spf_server);

    if (SPF_request_set_ipv4_str(spf_request, ip)) {
        zend_throw_exception(spf_ce_SpfException, "invalid ip address", 0 TSRMLS_CC);
    }

    if (SPF_request_set_helo_dom(spf_request, helo)) {
        zend_throw_exception(spf_ce_SpfException, "invalid HELO domain", 0 TSRMLS_CC);
    }

    if (SPF_request_set_env_from(spf_request, sender)) {
        zend_throw_exception(spf_ce_SpfException, "invalid envelope from", 0 TSRMLS_CC);
    }

    SPF_request_query_mailfrom(spf_request, &spf_response);

	object_init_ex(return_value, spf_ce_SpfResponse);
	response = (php_spf_response_object*) zend_object_store_get_object(return_value TSRMLS_CC);
	response->spf_response = spf_response;
}

PHP_METHOD(SpfResponse, getResult)
{
	SPF_response_t *response;

	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_FALSE;
	}

	SPF_RESPONSE_FROM_OBJECT(response, getThis());

	RETURN_STRING(SPF_strresult(SPF_response_result(response)), 1);
}

PHP_METHOD(SpfResponse, getHeaderComment)
{
	SPF_response_t *response;

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

	SPF_RESPONSE_FROM_OBJECT(response, getThis());

	RETURN_STRING(SPF_response_get_header_comment(response), 1);
}

PHP_METHOD(SpfResponse, getReceivedSpf)
{
	SPF_response_t *response;

	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_FALSE;
	}

	SPF_RESPONSE_FROM_OBJECT(response, getThis());

	RETURN_STRING(SPF_response_get_received_spf(response), 1);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
