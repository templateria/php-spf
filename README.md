# PHP bindings for libspf2 (http://www.libspf2.org)

## Install

Make sure you install libspf2 development package first.

	$ git clone http://github.com/clone_url/php-spf.git
	$ cd php-spf
	$ phpize
	$ ./configure
	$ make
	$ sudo make install
	$ echo "extension=spf.so" > /etc/php5/conf.d/spf.ini

##  API

### Spf

	void        Spf::__construct([int $type[, string $domain[, string $spf]]])
	SpfResponse Spf::query(string $ip, string $helo, string $sender[, string $recipient])

### SpfResponse

	string  SpfResponse::getResult();
	string  SpfResponse::getHeaderComment();
	string  SpfResponse::getReceivedSpf();
	string  SpfResponse::getReceivedSpfValue();
	string  SpfResponse::getExplanation();
	string  SpfResponse::getSmtpComment();
	boolean SpfResponse::hasErrors();
	boolean SpfResponse::hasWarnings();
	array   SpfResponse::getErrors();
	array   SpfResponse::getWarnings();
	
### Example

```php
<?php
$spf = new Spf();
$response = $spf->query("216.239.32.2", "gmail.com", "pgpadron@gmail.com");
var_dump($response->getResult());
?>
```
