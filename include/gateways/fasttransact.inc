<?php

class Fasttransact{
	
	var $debug = false;
	var $gatewayName = 'fasttransact';
	var $cfg = array();
	var $error = null;
	var $params = array();
	var $results = array();
	
	var $next_customer_number = null;
	var $next_order_number = null;
	var $declineMessage = null;
	var $submission_URL = "https://secure.fasttransact.com/gw/native/direct2.1";
	var $user_host = null;
	
	// --------------------------------------------------------------
	function __construct(){
		
		global $cart;
		global $_Totals;
		
		if(empty($_Totals) && !empty($cart->totals)){
			$_Totals = $cart->totals;
		}
	}
	
	// --------------------------------------------------------------
	function processPayment(){
		
		global $_Common;

        $_SESSION['user_ip'] = $_SERVER['REMOTE_HOST'];
        if(trim($_SESSION['user_ip']) == ""){
            $_SESSION['user_ip'] = $_SERVER['REMOTE_ADDR'];
        }
        $_SESSION['user_host'] = "undefined";
		if(!empty($_SESSION['user_ip'])){
			$_SESSION['user_host'] = gethostbyaddr($_SESSION['user_ip']);	
		}

		$this->loadParams();
		
		if($this->debug){
			$_Common->debugPrint($this->params, $this->gatewayName . " params");
		}

		$request = $this->createRequestString();
		$this->results = null;

		if(!is_null($this->submission_URL)){
			$this->results = $this->sendRequest($request);
		}
		else{
			$_Common->printErrorMessage($this->gatewayName . " Payment Module Error","The submission URL is not set in the configuration.");
		}
		
		if(!$this->results){
			$_Common->printErrorMessage($this->gatewayName . " Payment Module Error","There was no response from the curl request.");
		}
		else{
			//[RET_AUTH_MSG] => TEST APPROVED
			//[RET_AUTH_CODE] => 999999
			//[RET_STATUS] => T
			//[RET_AVS_CODE] => X
			//[RET_AVS_MSG] => Street and zip+4 match
			//[RET_TRANS_ID] => 120312045048
			//[RET_CARD_ISSUER] => VISA
			//[RET_CVV2_CODE] => M
			//[RET_CVV2_MSG] => CVV2 match
			if($this->debug){
				$_Common->debugPrint($this->results, $this->gatewayName . " results");
			}
			if($this->results['RET_STATUS'] == "1" || strtolower($this->results['RET_STATUS']) == "t"){
				return true;	
			}			
			elseif(strtolower($this->results['RET_STATUS']) == "d"){
				$this->results['RET_AUTH_MSG'] = "Duplicate Transaction. Your order has already been processed.";
				return false;
			}			
			elseif($this->results['RET_STATUS'] == "0"){
				$this->declineMessage = trim($this->results['RET_AUTH_MSG']);
				return false;	
			}
			else{
				// not sure what the error was but not approved or declined
				$this->declineMessage = trim($this->results['RET_AUTH_MSG']);
				return false;	
			}
		}
	}

	// --------------------------------------------------------------
	function loadParams(){
		
		global $_CF;
		global $_Common;
		global $_DB;
		global $_Totals;

		$_SESSION['pay_type'] = "C";
		if($_REQUEST['payment_method'] == "check.html"){
			$_SESSION['pay_type'] = "K";
		}
		$_SESSION['expire_yr'] = substr($_SESSION['expire_year'],-2);
		
		$this->cfg = $this->readConfig($this->gatewayName);
		if(count($this->cfg) == 0){
			$_Common->printErrorMessage("Payment Module Error",
										"Could not read configuration information for " . $this->gatewayName . " payment module");
		}

		// Account info
		$this->params['GEN_ACCOUNT'] = 'account_id';
		$this->params['GEN_SITETAG'] = 'site_tag';
		$this->params['GEN_TRANS_TYPE'] = 'transaction_type';
		$this->params['GEN_AMOUNT'] = 'grandtotal';
		$this->params['tax_amount'] = 'salestax';
		$this->params['ship_amount'] = 'shipping';

		// Billing Information
		$this->params['CUST_NAME1'] = 'billaddress_firstname';
		$this->params['CUST_NAME2'] = 'billaddress_lastname';
		$this->params['CUST_ADDR_STREET'] = 'billaddress_addr1, ,billaddress_addr2';
		$this->params['CUST_ADDR_CITY'] = 'billaddress_city';
		$this->params['CUST_ADDR_STATE'] = 'billaddress_state';
		$this->params['CUST_ADDR_ZIP'] = 'billaddress_postalcode';
		$this->params['CUST_ADDR_COUNTRY'] = 'billaddress_country';
		$this->params['CUST_PHONE'] = 'billaddress_phone';
		$this->params['CUST_EMAIL'] = 'billaddress_email';
		$this->params['CUST_IP'] = 'user_ip';
		$this->params['CUST_HOST'] = 'user_host';
		
		$this->params['GEN_PAYMENT_TYPE'] = 'pay_type';
		if($_SESSION['pay_type'] == "K"){
			// checks
			$this->params['ACH_ROUTING'] = 'aba_routing_code';
			$this->params['ACH_ACCOUNT'] = 'account_number';
			$this->params['ACH_CHECKNUMBER'] = 'check_number';
			$this->params['ACH_ID_SSN'] = 'ssn';
			$this->params['ACH_ID_DL'] = 'drivers_license_number';
			$this->params['ACH_ID_STATE'] = 'drivers_license_state';
		}
		else{
			// Credit Card Fields
			$this->params['CARD_NUMBER'] = 'card_number';
			$this->params['CARD_EXPIRE'] = 'expire_month,expire_yr';
			$this->params['CARD_CVV2'] = 'cvv2';
		}
	
		foreach($this->params as $key=>$val){
			if(strstr($val,',')){
				$flds = explode(',',$val);
				$result = "";
				foreach($flds as $i=>$fld){
					$result .= $this->testField($fld);
				}
			}
			else{
				$result = $this->testField($val);
			}
	
			if(is_null($result)){
				unset($this->params[$key]);
			}
			else{
				$this->params[$key] = $result;
			}
		}

		if(isset($_Totals['insurance']) && $_Totals['insurance'] > 0){
			$this->params['ship_amount'] = $_Common->format_price($this->params['ship_amount'] + $_Totals['insurance']);
		}

		$this->params['GEN_MISC_INFO'] = "Customer Number: " . $this->next_customer_number . "\n";
		$this->params['GEN_MISC_INFO'] .= "Order Number: " . $this->next_order_number;
		$this->params['GEN_DESCRIPTION'] = $this->setDescription();
		
	}


	//-----------------------------------------------------
	function readConfig($gateway){
		
		global $_Common;
		global $_DB;
		
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

		//$_Common->debugPrint($cfg);

		return $cfg;	
	}


	// --------------------------------------------------------------
	// Tests a field key to try find a match in the cfg, totals, 
	// request or session variables.
	// --------------------------------------------------------------
	function testField($key){

		global $_Totals;
		
		if(isset($this->cfg['fields'][$key])){
			if($this->cfg['fields'][$key] === true){
				return 'TRUE';	
			}
			elseif($this->cfg['fields'][$key] === false){
				return 'FALSE';	
			}
			else{
				return trim($this->cfg['fields'][$key]);
			}
		}
		elseif(isset($_Totals[$key])){
			return trim($_Totals[$key]);
		}
		elseif(isset($_REQUEST[$key])){
			return trim($_REQUEST[$key]);
		}
		elseif(isset($_SESSION[$key])){
			return trim($_SESSION[$key]);
		}
		else{
			return null;
		}
	}


	// --------------------------------------------------------------
	// Creates the encoded request string
	// --------------------------------------------------------------
	function createRequestString(){
		
		global $_Common;
		
        $str = "";
        foreach($this->params as $key=>$val){
            $str .= "$key=" . urlencode(str_replace(',', '', $val)) . '&';
        }
	    // remove the last ampersand
        $str = substr($str, 0, -1);
        
        if($this->debug){
			$_Common->debugPrint($str, "Request String");
		}
        
		return $str;		
	}

	// --------------------------------------------------------------
	function sendRequest($requestStr){
		
		global $_Common;

		if(!function_exists('curl_init')){
			exit("<b>ERROR: Cannot Continue. Curl is not available through PHP</b>");
		}
		
        if($this->debug){
			$_Common->debugPrint($this->submission_URL,"Post URL:");
			$_Common->debugPrint($requestStr,"Request String:");
        }

        $ch = curl_init();
        if($this->debug){
	        curl_setopt($ch, CURLOPT_VERBOSE, 1);
	    }
        curl_setopt($ch, CURLOPT_URL, $this->submission_URL);
        curl_setopt($ch, CURLOPT_HEADER, 1);
        curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($ch, CURLOPT_POST, 1);
        curl_setopt($ch, CURLOPT_POSTFIELDS, $requestStr);
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, 0);
		curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 0);
		curl_setopt($ch, CURLOPT_HTTP_VERSION, 1);

		$result = curl_exec($ch);
		$info = curl_getinfo($ch);
		curl_close($ch);	

		if(trim($result) == ""){
			$results['RET_STATUS'] = "E";
			$results['RET_AUTH_MSG'] = "There was no response from " . $this->gatewayName;
		}
		else{

			list($response_code,$response_headers,$response_body) = $this->parseResponseHeader($result);

			if($this->debug){
				$_Common->debugPrint($response_code,"Response Code");
				$_Common->debugPrint($response_headers,"Response Headers");
				$_Common->debugPrint($response_body,"Response Body");
				$_Common->debugPrint($result,"curl raw result");
				$_Common->debugPrint($info, "curl info");
			}

			if($info['http_code'] != 200){
				if($this->error){
					$results['RET_STATUS'] = "E";
					$results['RET_AUTH_MSG'] = trim($this->error);
				}
				else{
					$results['RET_STATUS'] = "E";
					$results['RET_AUTH_MSG'] = "There has been an error connecting to " . $this->gatewayName;
				}
			}
			else{
				$pairs = explode('&', trim($response_body));
				foreach ($pairs as $i) {
					list($name,$value) = explode('=', $i, 2);
					$results[$name] = urldecode($value);
				}
			}
		}

		if($this->debug){
			$_Common->debugPrint($results,"curl result");
		}
		
		
	return $results;
	}

	// --------------------------------------------------------------
	function parseResponseHeader($response){

		global $_Common;
		
		list($response_headers,$response_body) = explode("\r\n\r\n",$response,2); 
		$response_header_lines = explode("\r\n",$response_headers); 

		// first line of headers is the HTTP response code 
		if (preg_match('@^HTTP/[0-9]\.[0-9] ([0-9]{3})@',$response_header_lines[0], $matches)) { 
			$response_code = $matches[1];
			$this->error = str_replace($matches[0],"",$response_header_lines[0]);
		} 

		// put the rest of the headers in an array 
		$response_header_array = array(); 
		foreach ($response_header_lines as $header_line) { 
			@list($header,$value) = explode(': ',$header_line,2); 
			$response_header_array[$header] = $value; 
		} 

	return array($response_code,$response_header_array,$response_body);	
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
		$strDesc = join(", ",$desc);
		$strDesc = str_replace("\r","",$strDesc);
		$strDesc = str_replace("\n","",$strDesc);
		return $strDesc;
	}



	// --------------------------------------------------------------
	// Loads the default values into the database when uploaded
	// --------------------------------------------------------------
	function loadDefaults(){

		global $_DB;

		$gateway = $this->gatewayName;

		$count = $_DB->getCount('payment_gateways', "WHERE LOWER(gateway_name) = '$gateway'");

		if($count == 0){

			$sql = "INSERT INTO `payment_gateways` ( `gateway_name`, `active`, `debug`, `related_payment_form` ) 
											VALUES ( '$gateway', 'false', 'false', 'credit_card.html')";

			$_DB->execute($sql);
			$pgid = $_DB->getInsertID('payment_gateways','pgid');

			if(!empty($pgid)){

				$details['account_id'] = "your $gateway account id";
				$details['site_tag'] = 'your $gateway site tag';
				$details['transaction_type'] = 'SALE';

				$counter = 0;
				foreach($details as $key=>$value){
					$sql = "INSERT INTO `payment_gateway_details` (`pgid`,`section`,`sequence`,`key`,`value`)
														   VALUES ('$pgid','fields','$counter','$key','$value')";
					$_DB->execute($sql);
					$counter++;
				}
			}
		}
	}
}
?>