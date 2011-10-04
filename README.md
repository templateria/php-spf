PHP bindings for libspf2 (http://libspf2.org)

* Install

Make sure you install libspf2 development package first.

	$ git clone https://github.com/w3p/php-spf
	$ cd php-spf
	$ phpize
	$ ./configure
	$ make
	$ sudo make install
	$ echo "extension=spf.so" > /etc/php5/conf.d/spf.ini

* API

** Spf

	void Spf::__construct([int $type[, string $domain[, string $spf]]])
	SpfResponse Spf::query(string $ip, string $helo, string $sender[, string $recipient])

** SpfResponse

	string SpfResponse::getResult()
	string SpfResponse::getHeaderComment();
	string SpfResponse::getReceivedSpf();
	string SpfResponse::getReceivedSpf();
	string SpfResponse::getExplanation();
	string SpfResponse::getSmtpComment();
	boolean SpfResponse::hasErrors();
	boolean SpfResponse::hasWarnings();
	boolean SpfResponse::getErrors();
	boolean SpfResponse::getWarnings();

