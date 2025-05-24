<?php
// initialize the program and read the config(s)
include_once("include/initialize.inc");
$init = new Initialize();
global $_DB;

$debug = false;

	// read	the	post from PayPal system	and	add	'cmd'
	$req = 'cmd=_notify-validate';

	foreach($_POST as $key => $value) {
		// stripslashes removed as magic_quotes_gpc is obsolete in PHP 7.4+
		$value = urlencode($value);
		$req .=	"&$key=$value";
	}

	if($debug){
		logResult("Notify:\n" . parseString($req) . "\n\n");
	}

    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, 'http://www.paypal.com');
    curl_setopt($ch, CURLOPT_HEADER, 0);
    curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($ch, CURLOPT_POST, 1);
    curl_setopt($ch, CURLOPT_POSTFIELDS, $req);
	curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, 0);
	curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 0);
    $res = curl_exec($ch) or die("There has been a cURL error connecting to www.paypal.com.");
    $info = curl_getinfo($ch);
    curl_close($ch);	

	// retrieve payment status
	$payment_status	= $_POST['payment_status'];
	$payment_gross = $_POST['mc_gross'];
	$payer_email = $_POST['payer_email'];
	$ppid = $_POST['custom'];

	if($debug){
		logResult("\n\nIPN Response:\n" . $res . "\n\n");
		logResult("\n\nCurl Info:\n" . print_r($info,true) . "\n\n");
	}

	// process payment
	if(strstr($res, "VERIFIED")){

		// Check the payment amounts
		$sql = "SELECT payment_gross FROM paypal WHERE ppid = '$ppid' LIMIT 1";
		$data = array();
		$data = $_DB->getRecord($sql);
		
		if(count($data) == 0){
			if($debug){
				logResult("Could not find matching paypal table entry for the IPN results:\n\n$sql");	
			}
			sendSuspiciousEmail(parseString($req),"Could not find matching paypal table entry for the IPN results.");
			exit;
		}
		
		if(!empty($data['payment_gross']) && doubleval($data['payment_gross']) != doubleval($payment_gross)){
			if($debug){
				$pGross = doubleval($data['payment_gross']);
				logResult("Paypal payment amount ($payment_gross) does not match submitted amount ($pGross). Possible Fraud.");	
			}
			$payment_status = "Paypal payment amount does not match submitted amount. Possible Fraud.";
			sendSuspiciousEmail(parseString($req),$payment_status);
		}
		else{
			$sql = "UPDATE paypal SET payment_status = '$payment_status', payer_email = '$payer_email'";
			if(!empty($_POST['memo'])){
				$memo = $_POST['memo'];
				$sql .= "memo = '$memo'";
			}
			$sql .= " WHERE ppid = '$ppid'";
			if($debug){
				logResult("SQL:\n $sql\n\n");
			}
			$_DB->execute($sql);
		
			// update the order
			if(!empty($_SESSION['reference_number'])){
				$refnum = $_SESSION['reference_number'];
				$_DB->execute("UPDATE orders SET `paid` = 'true', `transaction_status` = 'Completed' WHERE reference_number = '$refnum'");
			}
		}
		
	}
	elseif(strstr($res, "INVALID")){
		$sql = "UPDATE paypal SET payment_status = 'Invalid' WHERE ppid = '$ppid'";
		$_DB->execute($sql);
		if($debug){
			logResult("SQL:\n $sql\n\n");
		}
		if(!empty($_SESSION['reference_number'])){
			$refnum = $_SESSION['reference_number'];
			$_DB->execute("UPDATE orders SET `paid` = 'false', `transaction_status` = 'INVALID' WHERE reference_number = '$refnum'");
		}
	}

	if($debug){
		logResult("Request:\n" . parseString($req) . "\n\nResponse:\n" . parseString($res) . "\n\n");
	}
	exit;
	
	// ----------------------------------------------------
	function logResult($result){
		global $debug;
		$date = "\n\n" . date("m/d/Y") . "\n";
		if(getenv("windir") != ""){
			$result = str_replace("\n","\r\n",$result);
		}
		if($debug){
			$R = fopen("pp.results.log","a");
			fputs($R,$result);
			fclose($R);
		}

	}
	// --------------------------------------------------------------
	function parseString($res){
		$pairs = explode("&",$res);
		$parsed = array();
		foreach($pairs as $i=>$pair){
			list($name,$val) = explode("=",$pair) + [null, null];
			$parsed[$name] = $val;
		}
		//ksort($parsed);
		return print_r($parsed,true);		
	}
	
	// --------------------------------------------------------------
	function sendSuspiciousEmail($result,$comments){
		global $_Registry;
		global $_CF;
		$email = $_Registry->loadClass('email');
		$storeEmail = trim($_CF['email']['store_email_address']);
		
		if(isset($_POST['payment_status']) && $_POST['payment_status'] != "Refunded"){
			$subject = "Suspicious PayPal Payment";
			$Html = "<html><body><pre>\r\n$comments\r\n\r\n";
			$Html .= $result;
			$Html .= "</pre></body></html>\r\n";
			$email->send($storeEmail, $storeEmail, $subject, $Html, NULL);
		}
	}

	
	
?>