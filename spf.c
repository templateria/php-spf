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
#include "spf2/spf_dns_zone.h"
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

ZEND_BEGIN_ARG_INFO(arginfo_SpfResponse_getReceivedSpfValue, 0)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO(arginfo_SpfResponse_getExplanation, 0)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO(arginfo_SpfResponse_getSmtpComment, 0)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO(arginfo_SpfResponse_hasErrors, 0)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO(arginfo_SpfResponse_hasWarnings, 0)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO(arginfo_SpfResponse_getErrors, 0)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO(arginfo_SpfResponse_getWarnings, 0)
ZEND_END_ARG_INFO();
/* }}} */

#define SPF_REGISTER_CLASS_CONSTANT_LONG(class, name, value) \
	zend_declare_class_constant_long(spf_ce_ ## class, name, sizeof(name) - 1, (long) value TSRMLS_CC);

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
	PHP_ME(SpfResponse, getReceivedSpfValue, arginfo_SpfResponse_getReceivedSpfValue, ZEND_ACC_PUBLIC)
	PHP_ME(SpfResponse, getExplanation, arginfo_SpfResponse_getExplanation, ZEND_ACC_PUBLIC)
	PHP_ME(SpfResponse, getSmtpComment, arginfo_SpfResponse_getSmtpComment, ZEND_ACC_PUBLIC)
    PHP_ME(SpfResponse, hasErrors, arginfo_SpfResponse_hasErrors, ZEND_ACC_PUBLIC)
    PHP_ME(SpfResponse, getErrors, arginfo_SpfResponse_hasErrors, ZEND_ACC_PUBLIC)
    PHP_ME(SpfResponse, hasWarnings, arginfo_SpfResponse_hasWarnings, ZEND_ACC_PUBLIC)
    PHP_ME(SpfResponse, getWarnings, arginfo_SpfResponse_hasWarnings, ZEND_ACC_PUBLIC)
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

	SPF_REGISTER_CLASS_CONSTANT_LONG(Spf, "TYPE_DNS_RESOLV", SPF_DNS_RESOLV);
	SPF_REGISTER_CLASS_CONSTANT_LONG(Spf, "TYPE_DNS_CACHE",  SPF_DNS_CACHE);
	SPF_REGISTER_CLASS_CONSTANT_LONG(Spf, "TYPE_DNS_ZONE",   SPF_DNS_ZONE);

	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "RESULT_INVALID",   SPF_RESULT_INVALID);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "RESULT_NEUTRAL",   SPF_RESULT_NEUTRAL);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "RESULT_PASS",      SPF_RESULT_PASS);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "RESULT_FAIL",      SPF_RESULT_FAIL);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "RESULT_SOFTFAIL",  SPF_RESULT_SOFTFAIL);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "RESULT_NONE",      SPF_RESULT_NONE);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "RESULT_TEMPERROR", SPF_RESULT_TEMPERROR);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "RESULT_PERMERROR", SPF_RESULT_PERMERROR);

	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_NO_MEMORY", SPF_E_NO_MEMORY);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_NOT_SPF", SPF_E_NOT_SPF);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_SYNTAX", SPF_E_SYNTAX);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_MOD_W_PREF", SPF_E_MOD_W_PREF);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_INVALID_CHAR", SPF_E_INVALID_CHAR);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_UNKNOWN_MECH", SPF_E_UNKNOWN_MECH);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_INVALID_OPT", SPF_E_INVALID_OPT);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_INVALID_CIDR", SPF_E_INVALID_CIDR);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_MISSING_OPT", SPF_E_MISSING_OPT);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_INTERNAL_ERROR", SPF_E_INTERNAL_ERROR);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_INVALID_ESC", SPF_E_INVALID_ESC);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_INVALID_VAR", SPF_E_INVALID_VAR);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_BIG_SUBDOM", SPF_E_BIG_SUBDOM);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_INVALID_DELIM", SPF_E_INVALID_DELIM);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_BIG_STRING", SPF_E_BIG_STRING);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_BIG_MECH", SPF_E_BIG_MECH);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_BIG_MOD", SPF_E_BIG_MOD);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_BIG_DNS", SPF_E_BIG_DNS);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_INVALID_IP4", SPF_E_INVALID_IP4);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_INVALID_IP6", SPF_E_INVALID_IP6);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_INVALID_PREFIX", SPF_E_INVALID_PREFIX);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_RESULT_UNKNOWN", SPF_E_RESULT_UNKNOWN);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_UNINIT_VAR", SPF_E_UNINIT_VAR);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_MOD_NOT_FOUND", SPF_E_MOD_NOT_FOUND);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_NOT_CONFIG", SPF_E_NOT_CONFIG);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_DNS_ERROR", SPF_E_DNS_ERROR);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_BAD_HOST_IP", SPF_E_BAD_HOST_IP);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_BAD_HOST_TLD", SPF_E_BAD_HOST_TLD);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_MECH_AFTER_ALL", SPF_E_MECH_AFTER_ALL);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_INCLUDE_RETURNED_NONE", SPF_E_INCLUDE_RETURNED_NONE);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_RECURSIVE", SPF_E_RECURSIVE);
	SPF_REGISTER_CLASS_CONSTANT_LONG(SpfResponse, "ERROR_MULTIPLE_RECORDS", SPF_E_MULTIPLE_RECORDS);

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

/* {{{ free_spf_response */
void free_spf_response(void *object TSRMLS_DC)
{
	php_spf_response_object *intern = (php_spf_response_object*) object;

	if (intern->spf_response) {
		SPF_response_free(intern->spf_response);
	}

	efree(object);
}
/* }}} */

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

/* {{{ proto void Spf::__construct([int $type[, string $domain[, string $spf]]]) */
PHP_METHOD(Spf, __construct)
{
	int server_type = SPF_DNS_CACHE;
	int domain_len, spf_len;
	char *domain, *spf;
	php_spf_object *obj;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|lss", &server_type, &domain, &domain_len, &spf, &spf_len) == FAILURE) {
		RETURN_FALSE;
	}

	obj = (php_spf_object*) zend_object_store_get_object(getThis() TSRMLS_CC);
	obj->spf_server = SPF_server_new(server_type, 0);

	if (server_type == SPF_DNS_ZONE) {
		if (SPF_dns_zone_add_str(obj->spf_server->resolver, domain, ns_t_txt, NETDB_SUCCESS, spf)) {
			zend_throw_exception(spf_ce_SpfException, "Invalid SPF record", 0 TSRMLS_CC);
		}
	}

	if (!obj->spf_server) zend_throw_exception(spf_ce_SpfException, "could not initialize spf resource", 0 TSRMLS_CC);
}
/* }}} */

/* {{{ proto SpfResponse Spf::query(string $ip, string $helo, string $sender[, string $recipient]) */
PHP_METHOD(Spf, query)
{
    int ip_len, helo_len, sender_len, recipient_len;
    char *ip, *helo, *sender, *recipient;
	SPF_server_t *spf_server = NULL;
    SPF_request_t *spf_request = NULL;
    SPF_response_t *spf_response = NULL;
	php_spf_response_object *response;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss|s", &ip, &ip_len,
	&helo, &helo_len, &sender, &sender_len, &recipient, &recipient_len)) {
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
/* }}} */

/* {{{ proto string SpfResponse::getResult() */
PHP_METHOD(SpfResponse, getResult)
{
	SPF_result_t result;
	SPF_response_t *response;

	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_FALSE;
	}

	SPF_RESPONSE_FROM_OBJECT(response, getThis());

	result = SPF_response_result(response);

	RETURN_STRING(SPF_strresult(result), 1);
}
/* }}} */

/* {{{ proto string SpfResponse::getHeaderComment();
       Returns a string containing additional comments to be added to Received-SPF header. */
PHP_METHOD(SpfResponse, getHeaderComment)
{
	const char *header_comment;
	SPF_response_t *response;

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

	SPF_RESPONSE_FROM_OBJECT(response, getThis());

	header_comment = SPF_response_get_header_comment(response);
	if (header_comment == NULL) {
		RETURN_NULL();
	}

	RETURN_STRING(header_comment, 1);
}
/* }}} */

/* {{{ proto string SpfResponse::getReceivedSpf();
       Returns the complete Received-SPF header, including the field name. */
PHP_METHOD(SpfResponse, getReceivedSpf)
{
	SPF_response_t *response;
	const char *received_spf;

	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_FALSE;
	}

	SPF_RESPONSE_FROM_OBJECT(response, getThis());
	
	received_spf = SPF_response_get_received_spf(response);
	if (received_spf == NULL) {
		RETURN_NULL();
	}

	RETURN_STRING(received_spf, 1);
}
/* }}} */

/* {{{ proto string SpfResponse::getReceivedSpf();
       Returns the value of the Received-SPF header. */
PHP_METHOD(SpfResponse, getReceivedSpfValue)
{
    SPF_response_t *response;
	const char *received_spf_value;

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

    SPF_RESPONSE_FROM_OBJECT(response, getThis());

    received_spf_value = SPF_response_get_received_spf_value(response);
	if (received_spf_value == NULL) {
		RETURN_NULL();
	}

	RETURN_STRING(received_spf_value, 1);
}

/* {{{ proto string SpfResponse::getExplanation();
       Returns a string containing an explanation of the result. */
PHP_METHOD(SpfResponse, getExplanation)
{
    SPF_response_t *response;
    const char *explanation;

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

    SPF_RESPONSE_FROM_OBJECT(response, getThis());

    explanation = SPF_response_get_explanation(response);
    if (explanation == NULL) {
        RETURN_NULL();
    }

    RETURN_STRING(explanation, 1);
}

/* {{{ proto string SpfResponse::getSmtpComment();
       Returns a string containing containing a link to OpenSPF's checker and the reason for the result. */
PHP_METHOD(SpfResponse, getSmtpComment)
{
    SPF_response_t *response;
	const char *smtp_comment;

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

    SPF_RESPONSE_FROM_OBJECT(response, getThis());

    smtp_comment = SPF_response_get_smtp_comment(response);
	if (smtp_comment == NULL) {
		RETURN_NULL();
	}

	RETURN_STRING(smtp_comment, 1);
}

/* {{{ proto boolean SpfResponse::hasErrors();
       Returns whether the SPF query has generated errors. */
PHP_METHOD(SpfResponse, hasErrors)
{
    SPF_response_t *response;
    int errors;

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

    SPF_RESPONSE_FROM_OBJECT(response, getThis());

    errors = SPF_response_errors(response);

    if (errors) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

/* {{{ proto boolean SpfResponse::hasWarnings();
       Returns whether the SPF query has generated warnings. */
PHP_METHOD(SpfResponse, hasWarnings)
{
    SPF_response_t *response;
    int warnings;

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

    SPF_RESPONSE_FROM_OBJECT(response, getThis());

    warnings = SPF_response_warnings(response);

    if (warnings) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

/* {{{ proto boolean SpfResponse::getErrors();
       Returns an array of error messages indexed by error code. */
PHP_METHOD(SpfResponse, getErrors)
{
	int i;
	SPF_response_t *response;

	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_FALSE;
	}

	array_init(return_value);

	SPF_RESPONSE_FROM_OBJECT(response, getThis());

	for (i = 0; i < SPF_response_messages(response); i++) {
		SPF_error_t *err = SPF_response_message(response, i);
		if (!SPF_error_errorp(err)) {
			continue;
		}
		add_index_string(return_value, SPF_error_code(err), SPF_error_message(err), 1);
	}
}

/* {{{ proto boolean SpfResponse::getWarnings();
       Returns an array of warnings indexed by error code. */
PHP_METHOD(SpfResponse, getWarnings)
{
    int i;
    SPF_response_t *response;

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

    array_init(return_value);

    SPF_RESPONSE_FROM_OBJECT(response, getThis());

    for (i = 0; i < SPF_response_messages(response); i++) {
        SPF_error_t *err = SPF_response_message(response, i);
        if (SPF_error_errorp(err)) {
            continue;
        }
        add_index_string(return_value, SPF_error_code(err), SPF_error_message(err), 1);
    }
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
