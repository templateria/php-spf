--TEST--
should return pass for a valid sender
--SKIPIF--
<?php if (!extension_loaded('spf')) die ('spf not present');
--FILE--
<?php
$spf = new Spf();
$response = $spf->query("216.239.32.2", "gmail.com", "pgpadron@gmail.com");
var_dump($response->getResult());
?>
--EXPECTF--
string(4) "pass"
