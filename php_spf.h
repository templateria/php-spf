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

/* $Id: header 310447 2011-04-23 21:14:10Z bjori $ */
#include "netinet/in.h"
#include "spf2/spf.h"

#ifndef PHP_SPF_H
#define PHP_SPF_H

extern zend_module_entry spf_module_entry;
#define phpext_spf_ptr &spf_module_entry

#ifdef PHP_WIN32
#	define PHP_SPF_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_SPF_API __attribute__ ((visibility("default")))
#else
#	define PHP_SPF_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

extern zend_class_entry* spf_ce_Spf;

typedef struct _spf_object {
	zend_object std;
	SPF_server_t *spf_server;
} php_spf_object;

zend_object_value create_spf(zend_class_entry *class_type TSRMLS_DC);
void free_spf(void *object TSRMLS_DC);

PHP_MINIT_FUNCTION(spf);
PHP_MSHUTDOWN_FUNCTION(spf);
PHP_RINIT_FUNCTION(spf);
PHP_RSHUTDOWN_FUNCTION(spf);
PHP_MINFO_FUNCTION(spf);

PHP_METHOD(Spf, __construct);
PHP_METHOD(Spf, query);

#ifdef ZTS
#define SPF_G(v) TSRMG(spf_globals_id, zend_spf_globals *, v)
#else
#define SPF_G(v) (spf_globals.v)
#endif

#endif


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
