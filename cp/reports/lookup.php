<?php

$wsdlPath = "http://www.postcoderwebsoap.co.uk/websoap/websoap.php?wsdl";
require('lib/nusoap.php');
$client = new soap_client($wsdlPath,'wsdl');
$proxy = $client->getproxy();
$username = 'satsuper';
$password = 'super99';

$identifier = 'telephone_inv';
$postcode 	= trim('RH10 3LG');
$result = $proxy->getThrfareAddresses($postcode,$identifier,$username,$password);

?>
<pre>
<?php print_r($result);?>
</pre>