<?php

$api = "RateV4";
$userid = null;
$results = array();
$errors = null;

if(isset($_POST['userid'])){
	$userid = trim($_POST['userid']);
}
	
if($userid){
	$tests[0] = "<RateV4Request USERID=\"$userid\" PASSWORD=\"\"><Package ID=\"0\"><Service>PRIORITY</Service><ZipOrigination>10022</ZipOrigination><ZipDestination>20008</ZipDestination><Pounds>10</Pounds><Ounces>5</Ounces><Container>FLAT RATE ENVELOPE</Container><Size>REGULAR</Size></Package></RateV4Request>";
	$tests[1] = "<RateV4Request USERID=\"$userid\" PASSWORD=\"\"><Package ID=\"0\"><Service>All</Service><ZipOrigination>10022</ZipOrigination><ZipDestination>20008</ZipDestination><Pounds>10</Pounds><Ounces>5</Ounces><Container></Container><Size>LARGE</Size><Machinable>TRUE</Machinable></Package></RateV4Request>";

	$results = array();
	$errors = null;

	foreach($tests as $i=>$xml){
		//showXML($xml);
		$result = sendRequest($api,$xml);
		//showXML($result);
		$errors = parseResultXML($result,"Error");
		if(count($errors) > 0){
			$results = $errors;
		}
		else{
			$results = parseResultXML($result,"Postage");
		}
	}
}

# --------------------------------------------------------------
function sendRequest($api,$xml){
	
	//Create a cURL instance and retrieve XML response
	if(!is_callable("curl_exec")) die("USPS:TEST:submit_request: curl_exec is missing");
	$ch = curl_init("https://stg-production.shippingapis.com/ShippingAPI.dlll");
	curl_setopt($ch, CURLOPT_POST, 1);
	curl_setopt($ch, CURLOPT_POSTFIELDS, "API=" . $api . "&XML=" . $xml);
	curl_setopt($ch, CURLOPT_TIMEOUT, 60);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);

	$return_xml = curl_exec($ch);

return $return_xml;
}
// -------------------------------------------------------------------
function parseResultXML($xml,$searchKey){
    include_once("include/xml.search.inc");
    $search = new XMLSearch($searchKey);
    $records = $search->search($xml);
    return $records;
}
// -------------------------------------------------------------------
function showXML($xml){
    $xml = preg_replace("/</","&lt;",$xml);
    $xml = preg_replace("/>/","&gt;",$xml);
    print "<pre>$xml</pre>";
}
?>
<html>
<head>
	<link rel="stylesheet" href="styles/cart.styles.css" type="text/css">
	<style>
	li{
		font-family: Verdana, Arial, Helvetica, sans-serif;
		color: #5C4033;
		font-size: 11px;
	}
	</style>
</head>
<body>
	<h4 align="center">USPS Testing</h4>
<?php if(!$userid):?>
	<p>&nbsp;</p>
	<form method="post" action="usps.test.php">
		<p align="center">Enter USPS User ID: <input type="text" name="userid" value=""> &nbsp;
		<input type="submit" name="submit" value="Run Tests"></p>
	</form>

<?php elseif(!$errors): ?>
	<p><font color=blue><b>If shipping rates are displayed below, your test was successful.</b></font></p>
	<p>
		<?php foreach($results as $j=>$flds):?>
			<li><?=$flds['MailService'];?> = <?=$flds['Rate'];?><br />
		<?php endforeach;?>
	</p>
	<p>
		<font color=blue>
			The ICCC is manned from 7:00AM to 11:00PM Eastern Time.<br />
			E-mail: icustomercare@usps.com<br />
			Telephone: 1-800-344-7779 (7:00AM to 11:00PM ET)<br />
			Contact the Internet Customer Care Center (ICCC) by telephone or email and they will activate your user ID so that you have access to the production server at production.shippingapis.com.
		</font>
	</p>
<?php else:?>
	<p><font color=red><b>Your test failed.</b></font></p>
	<pre>
		<?php print_r($errors);?>
	</pre>
<?php endif;?>

</body>
</html>






