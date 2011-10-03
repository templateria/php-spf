--TEST--
should return errors when appropriate
--SKIPIF--
<?php if (!extension_loaded('spf')) die ('spf not present');
--FILE--
<?php
$spf = new Spf(Spf::TYPE_DNS_ZONE, "gmail.com", "v=spf1 messed up TXT record");
$response = $spf->query("216.239.32.2", "gmail.com", "pgpadron@gmail.com");
var_dump($response->getErrors());
?>
--EXPECTF--
array(2) {
  [6]=>
  string(37) "Unknown mechanism found near 'record'"
  [2]=>
  string(44) "Failed to compile SPF record for 'gmail.com'"
}
