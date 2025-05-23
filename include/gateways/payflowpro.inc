<?php
class Payflowpro {
	
	var $debug = false;
	var $gatewayName = 'payflowpro';
	var $cfg = array();
	var $results = array();
	var $declineMessage = null;
	var $test_host = "https://pilot-payflowpro.paypal.com";
	var $host = "https://payflowpro.paypal.com";
	
	// --------------------------------------------------------------
	function __construct(){
		
	}
	// --------------------------------------------------------------
	function Verisign_pro(){
		
		global $cart;
		global $_Totals;
		
		if(empty($_Totals) && !empty($cart->totals)){
			$_Totals = $cart->totals;
		}

	}
	// --------------------------------------------------------------
	function processPayment(){
		
		global $_Common;
		global $ip_address;
		global $_Totals;
		
		$cfg = $this->readConfig($this->gatewayName);
		if(count($cfg) == 0){
			$_Common->printErrorMessage("Payment Module Error",
										"Could not read configuration information for " . $this->gatewayName . " payment module");
		}

		$this->cfg = $cfg;
		if($this->cfg['status']['debug'] == 'true'){
			$this->debug = true;	
		}
		if($this->debug){
			$_Common->debugPrint($cfg,"Config Settings:");
		}

		$desc = $this->setDescription();

		//name_on_card

		$fields = array(
			'USER'       => $cfg['fields']['username'],
			'VENDOR'     => $cfg['fields']['vendor'],
			'PARTNER'    => $cfg['fields']['partner'],
			'PWD'        => $cfg['fields']['password'],
			'TENDER'     => 'C',  // C - Direct Payment using credit card
			'TRXTYPE'    => $cfg['fields']['transaction_type'],  // A - Authorization, S - Sale
			'ACCT'       => preg_replace("/[^0-9]/","",$_SESSION['card_number']),
			'CVV2'       => preg_replace("/[^0-9]/","",$_SESSION['cvv2']),
			'EXPDATE'    => $_SESSION['expire_month'] . substr($_SESSION['expire_year'],-2),
			'ACCTTYPE'   => $_SESSION['credit_card_type'],
			'AMT'        => number_format($_Totals['grandtotal'],2),
			'CURRENCY'   => $cfg['fields']['currency'],
			'FIRSTNAME'  => $_SESSION['billaddress_firstname'],
			'LASTNAME'   => $_SESSION['billaddress_lastname'],
			'STREET'     => $_SESSION['billaddress_addr1'],
			'CITY'       => $_SESSION['billaddress_city'],
			'STATE'      => $_SESSION['billaddress_state'],
			'ZIP'        => $_SESSION['billaddress_postalcode'],
			'COUNTRY'    => $_SESSION['billaddress_country'],
			'EMAIL'      => $_SESSION['billaddress_email'],
			'CUSTIP'     => $ip_address,
			'COMMENT1'   => $_REQUEST['comments'],
			'INVNUM'     => $this->next_customer_number . '.' . $this->next_order_number,
			'ORDERDESC'  => $desc,
			'VERBOSITY'  => 'MEDIUM',
		);

		if(!empty($_SESSION['card_start'])){
			$fields['CARDSTART'] = $_SESSION['card_start'];
		}
		if(!empty($_SESSION['card_issue'])){
			$fields['CARDISSUE'] = $_SESSION['card_issue'];
		}

		if($this->debug){
			$_Common->debugPrint($fields,"Request Parameters");
		}

		$paypal_query = null;
		foreach($fields as $key=>$value) {
			if($key == 'USER') {
				$paypal_query .= $key.'['.strlen($value).']='.$value;
			}
			else{
				$paypal_query .= '&'.$key.'['.strlen($value).']='.$value;
			}
		}

		$this->SendRequest($paypal_query);
		
		if($this->debug){
			$_Common->debugPrint($this->results,"Parsed Results");
		}
		
		if(count($this->results) > 0){
			return $this->ParseResultCode();	
		}
		
	}
	//-----------------------------------------------------
	function readConfig(){
		
		global $_Common;
		global $_DB;
		
		$gateway = $this->gatewayName;
		
		$records = $_DB->getRecords("SELECT payment_gateways.*,payment_gateway_details.*
									 FROM payment_gateways
									 LEFT JOIN payment_gateway_details ON payment_gateways.pgid = payment_gateway_details.pgid
									 WHERE payment_gateways.gateway_name = '$gateway'
									 ORDER BY payment_gateway_details.sequence");
		$cfg = array();
		foreach($records as $j=>$flds){
			$cfg['status']['active'] = $flds['active'];
			$cfg['status']['debug'] = $flds['debug'];
			$cfg['status']['related_payment_form'] = $flds['related_payment_form'];
			$cfg[$flds['section']][$flds['key']] = trim($flds['value']);
		}

		return $cfg;	
	}
	
	// --------------------------------------------------------------
	// Creates a description from the cart fields
	// --------------------------------------------------------------
	function setDescription(){
		
		global $_CART;

		$desc = array();
		foreach($_CART as $i=>$flds){
			$sku = $flds['sku'];
			$qty = intval($flds['quantity']);
			$name = $flds['name'];
			$total = trim($flds['line_total']);
			if(isset($flds['options']) && count($flds['options']) > 0){
				foreach($flds['options'] as $j=>$oFlds){
					$oValue = trim($oFlds['value']);
					$name .= ":$oValue";
				}
			}
			$desc[] = "$qty:$sku:$name:$total";
		}
		$strDesc = join("; ",$desc);
		$strDesc = str_replace("\r","",$strDesc);
		$strDesc = str_replace("\n","",$strDesc);
		return substr($strDesc,0,255);
	}
	
	// --------------------------------------------------------------
	// Makes the curl request to paypal
	// --------------------------------------------------------------
	function SendRequest($data) {

		global $_Common;

		$unique_id = session_id();

		$submiturl = $this->host;
		if($this->debug || strtolower($this->cfg['fields']['test_mode']) == 'true'){
			$submiturl = $this->test_host;
		}

		// get data ready for API
		$user_agent = $_SERVER['HTTP_USER_AGENT'];
		// Here's your custom headers; adjust appropriately for your setup:
		$headers[] = "Content-Type: text/namevalue"; //or text/xml if using XMLPay.
		$headers[] = "Content-Length : " . strlen ($data);  // Length of data to be passed 

		// Here I set the server timeout value to 45, but notice below in the cURL section, I set the timeout
		// for cURL to 90 seconds.  You want to make sure the server timeout is less, then the connection.

		// Header changes for Version 4
		//$headers[] = "X-VPS-Timeout: 45";
		//$headers[] = "X-VPS-Request-ID:" . $unique_id;
		$headers[] = "X-VPS-CLIENT-TIMEOUT: 45";
		$headers[] = "X-VPS-REQUEST-ID:" . $unique_id;

		// Optional Headers.  If used adjust as necessary.
		//$headers[] = "X-VPS-VIT-OS-Name: Linux";                  // Name of your OS
		//$headers[] = "X-VPS-VIT-OS-Version: RHEL 4";          // OS Version
		//$headers[] = "X-VPS-VIT-Client-Type: PHP/cURL";          // What you are using
		//$headers[] = "X-VPS-VIT-Client-Version: 0.01";          // For your info
		//$headers[] = "X-VPS-VIT-Client-Architecture: x86";          // For your info
		//$headers[] = "X-VPS-VIT-Integration-Product: PHPv4::cURL";  // For your info, would populate with application name
		//$headers[] = "X-VPS-VIT-Integration-Version: 0.01";         // Application version
		
		$ch = curl_init();
		curl_setopt($ch, CURLOPT_URL, $submiturl);
		curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
		curl_setopt($ch, CURLOPT_USERAGENT, $user_agent);
		curl_setopt($ch, CURLOPT_HEADER, 1);                // tells curl to include headers in response
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);        // return into a variable
		curl_setopt($ch, CURLOPT_TIMEOUT, 90);              // times out after 90 secs
		curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 0);
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, 0);        // this line makes it work under https
		curl_setopt($ch, CURLOPT_POSTFIELDS, $data);        //adding POST data
		curl_setopt($ch, CURLOPT_SSL_VERIFYHOST,  2);       //verifies ssl certificate
		curl_setopt($ch, CURLOPT_FORBID_REUSE, TRUE);       //forces closure of connection when done
		curl_setopt($ch, CURLOPT_POST, 1); 									//data sent as POST

		// Try to submit the transaction up to 3 times with 5 second delay.  This can be used
		// in case of network issues.  The idea here is since you are posting via HTTPS there
		// could be general network issues, so try a few times before you tell customer there
		// is an issue.

		$i=1;
		while ($i++ <= 3) {
			$result = curl_exec($ch);
			$headers = curl_getinfo($ch);
			if($headers['http_code'] != 200) {
				sleep(5);  // Let's wait 5 seconds to see if its a temporary network issue.
			}
			elseif($headers['http_code'] == 200) {
				// we got a good response, drop out of loop.
				break;
			}
		}
		// In this example I am looking for a 200 response from the server prior to continuing with
		// processing the order.  You can use this or other methods to validate a response from the
		// server and/or timeout issues due to network.
		if($headers['http_code'] != 200) {
			$this->results = array();
			$this->declineMessage = "Unable to communicate with PayPal server.";
			curl_close($ch);
			if($this->debug){
				$_Common->debugPrint($headers,"Result Headers");	
			}
		}
		else{
			curl_close($ch);
			$result = strstr($result, "RESULT");
			// prepare responses into array
			$this->results = array();
			while(strlen($result)){
				// name
				$keypos= strpos($result,'=');
				$keyval = substr($result,0,$keypos);
				// value
				$valuepos = strpos($result,'&') ? strpos($result,'&'): strlen($result);
				$valval = substr($result,$keypos+1,$valuepos-$keypos-1);
				// decoding the respose
				$this->results[$keyval] = $valval;
				$result = substr($result,$valuepos+1,strlen($result));
			}
		}
	}

	// --------------------------------------------------------------
	// Evaluate Result Code returned from PayPal.
	// --------------------------------------------------------------
	function ParseResultCode(){
	
		$isOk = false;
		
		switch($this->results['RESULT']){
			
			case '0':
				$isOk = true;
				break;

			case '1':
			case '26':
				$this->declineMessage = "You account configuration is set incorrectly.";
				break;
				
			case '12':
				// Hard decline from bank.
				$this->declineMessage = "Your transaction was declined.";
				break;

			case '23':
			case '24':
				// Issue with credit card number or expiration date.
				$this->declineMessage = "Your transaction was declined. Invalid credit card information.";
				break;

			case '13':
				// Voice authorization required.
				$this->declineMessage = "Your transaction is pending. Please contact customer service to complete your order.";
				break;

			case '125':
			case '126':
			case '127':
				// 125, 126 and 127 are Fraud Responses.
				$this->results['FRAUD_FILTER_TRIGGERED'] = "You need to manually verify this order.";
				$this->declineMessage = "Your transaction is under review. Please contact customer service for details.";
				break;
		}
		
		if(isset($this->results['DUPLICATE']) && $this->results['DUPLICATE'] == "1" && $this->declineMessage == ""){
			$this->declineMessage = "This transaction has already been processed.";
			$isOk = false;
		}

		return $isOk;
	}


	
	// --------------------------------------------------------------
	// Loads the default values into the database when uploaded
	// --------------------------------------------------------------
	function loadDefaults(){

		global $_DB;

		$gateway = strtolower($this->gatewayName);

		$count = $_DB->getCount('payment_gateways', "WHERE LOWER(gateway_name) = '$gateway'");

		if($count == 0){

			$sql = "INSERT INTO `payment_gateways` ( `gateway_name`, `active`, `debug`, `related_payment_form` ) 
											VALUES ( '$gateway', 'false', 'false', 'credit_card.html')";

			$_DB->execute($sql);
			$pgid = $_DB->getInsertID('payment_gateways','pgid');
			
			if(!empty($pgid)){

				$details['username'] = '';
				$details['password'] = '';
				$details['vendor'] = '';
				$details['partner'] = 'Paypal';
				$details['transaction_type'] = 'S';
				$details['currency'] = 'USD';
				$details['test_mode'] = 'false';

				$counter = 0;
				foreach($details as $key=>$value){
					$sql = "INSERT INTO `payment_gateway_details` (`pgid`,`section`,`sequence`,`key`,`value`)
														   VALUES ('$pgid','fields','$counter','$key','$value')";
					$_DB->execute($sql);
					$counter++;
				}
				
				$sql = "INSERT INTO `help`  (`section`, `section_help`, `key`, `key_help`) VALUES 
										('$gateway', 'These are the Paypal PayFlowPro Gateway parameters', '', ''),
										('$gateway', '', 'vendor', 'Set to the same as your Verisign Login'),
										('$gateway', '', 'partner', 'This is typically Paypal'),
										('$gateway', '', 'transaction_type', 'Sale or Authorization (S or A)'),
										('$gateway', '', 'currency', 'Your three character currency code (USD)'),
										('$gateway', '', 'username', 'Your Verisign username'),
										('$gateway', '', 'password', 'Your Verisign password'),
										('$gateway', '', 'test_mode', 'Run in test mode?')";
				$_DB->execute($sql);
			}
		}
	}
}
?>