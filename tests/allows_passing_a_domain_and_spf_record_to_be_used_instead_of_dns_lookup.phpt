--TEST--
allows passing a domain and spf record to be used instead of dns lookup
--SKIPIF--
<?php if (!extension_loaded('spf')) die ('spf not present');
--FILE--
<?php
$spf = new Spf(Spf::TYPE_DNS_ZONE, "gmail.com", "v=spf1 -all");
$response = $spf->query("216.239.32.2", "gmail.com", "pgpadron@gmail.com");
var_dump($response->getResult());
?>
--EXPECTF--
string(4) "fail"
