<?php

class Usaepay{
	
	var $debug = false;
	var $gatewayName = 'usaepay';
	var $cfg = array();
	var $params = array();
	var $results = array();
	
	var $next_customer_number = null;
	var $next_order_number = null;
	var $declineMessage = null;
	var $submission_URL = "https://www.usaepay.com/gate.php";
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
			if($this->debug){
				$_Common->debugPrint($this->results, $this->gatewayName . " results");
			}
			if(strtolower($this->results['UMstatus']) != 'approved'){
				$this->declineMessage = trim(urldecode($this->results['UMerror']));
				return false;	
			}
			else{
				return true;	
			}
		}
	}

	// --------------------------------------------------------------
	function loadParams(){
		
		global $_CF;
		global $_Common;
		global $_DB;
		global $_Totals;

		$this->cfg = $this->readConfig($this->gatewayName);
		if(count($this->cfg) == 0){
			$_Common->printErrorMessage("Payment Module Error",
										"Could not read configuration information for " . $this->gatewayName . " payment module");
		}

            // billing

		$this->params['UMbillcompany'] = 'billaddress_company';
		$this->params['UMbillfname'] = 'billaddress_first_name';
		$this->params['UMbilllname'] = 'billaddress_last_name';
		$this->params['UMbillstreet'] = 'billaddress_address';
		$this->params['UMbillstreet2'] = 'billaddress_address2';
		$this->params['UMbillcity'] = 'billaddress_city';
		$this->params['UMbillstate'] = 'billaddress_state';
		$this->params['UMbillcountry'] = 'billaddress_country';
		$this->params['UMbillzip'] = 'billaddress_postalcode';
		$this->params['UMbillphone'] = 'billaddress_phone';
		$this->params['UMcustemail'] = 'billaddress_email';
		$this->params['UMfax'] = 'billaddress_fax';

            // shipping

		$this->params['UMshipfname'] = 'shipaddress_first_name';
		$this->params['UMshiplname'] = 'shipaddress_last_name';
		$this->params['UMshipstreet'] = 'shipaddress_address';
		$this->params['UMshipstreet2'] = 'shipaddress_address2';
		$this->params['UMshipstate'] = 'shipaddress_state';
		$this->params['UMshipcity'] = 'shipaddress_city';
		$this->params['UMshipcountry'] = 'shipaddress_country';
		$this->params['UMshipzip'] = 'shipaddress_postalcode';
		$this->params['UMshipphone'] = 'shipaddress_phone';

            // payment

		if($_REQUEST['payment_method'] == "credit_card.html"){
			$this->params['UMname'] = 'name_on_card';
			$this->params['UMcard'] = 'card_number';
			$this->params['UMexpir'] = 'expire_date';
			$this->params['UMcvv2'] = 'cvv2';
			$this->params['UMstreet'] = 'billaddress_address';
			$this->params['UMzip'] = 'billaddress_postalcode';
			$this->params['UMemail'] = 'billaddress_email';
		}

		if($_REQUEST['payment_method'] == "check.html"){
			$this->params['UMrouting'] = 'aba_routing_code';
			$this->params['UMaccount'] = 'account_number';
			$this->params['UMssn'] = 'ssn';
			$this->params['UMdlnum'] = 'dlnum';
			$this->params['UMdlstate'] = 'dlstate';
		}

            // totals

		$this->params['UMsubtotal'] = 'subtotal';
		$this->params['UMdiscount'] = 'discount';
		$this->params['UMtax'] = 'salestax';
		$this->params['UMshipping'] = 'shipping';
		$this->params['UMamount'] = 'grandtotal';

            // config

		$this->params['UMkey'] = 'source_key';
		$this->params['UMcurrency'] = 'currency_code';
		$this->params['UMcommand'] = 'transaction_type';
		$this->params['UMtestmode'] = 'test_mode';
	
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
			$this->params['UMshipping'] = $_Common->format_price($this->params['UMshipping'] + $_Totals['insurance']);
		}

		$this->params['UMcustid'] = $this->next_customer_number;
		$this->params['UMinvoice'] = $this->next_order_number;
		$this->params['UMwebsite'] = $_CF['basics']['store_name'];
		$this->params['UMredir'] = $_CF['basics']['web_site_url'];
		$this->params['UMcustreceipt'] = 'no';

        $this->user_host = $_SERVER['REMOTE_HOST'];
        if(trim($this->user_host) == ""){
            $this->user_host = $_SERVER['REMOTE_ADDR'];
        }

		$this->params['UMip'] = $this->user_host;
		$this->params['UMdescription'] = $this->setDescription();
		
		if($this->debug){
			$this->params['UMtestmode'] = 'true';
		}
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

		if(!function_exists('curl_version')){
			exit("<b>ERROR: Cannot Continue. Curl is not available through PHP</b>");
		}
		
		$ch = curl_init();
		
		//curl_setopt($ch, CURLOPT_VERBOSE, true);
		curl_setopt($ch, CURLOPT_URL, $this->submission_URL);
		curl_setopt($ch, CURLOPT_HEADER, true);
		curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($ch, CURLOPT_POST, true);
		curl_setopt($ch, CURLOPT_POSTFIELDS, $requestStr);
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
		curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, false);

		$result = curl_exec($ch);
		$info = curl_getinfo($ch);

		curl_close($ch);	

		if($info['http_code'] != 200){
			return null;
		}

		// Since they are trying to do a redirect, we need to parse the HTTP headers
		// to get the results out

		list($response_code,$response_headers,$response_body) = $this->parseResponseHeader($result);

		if($this->debug){
			//$_Common->debugPrint($info,"Curl Info");
			//$_Common->debugPrint($response_code,"Response Code");
			//$_Common->debugPrint($response_headers,"Response Headers");
			//$_Common->debugPrint($response_body,"Response Body");
			//$_Common->debugPrint($result,"curl result");
		}	

		@list($url,$str) = explode('?',$response_headers['Location']);
		$fields = explode('&',$str);
		foreach($fields as $k=>$v){
			@list($key,$val) = explode('=',$v);
			$results[$key] = $val;	
		}
		
	return $results;
	}



	// --------------------------------------------------------------
	function parseResponseHeader($response){
		
		list($response_headers,$response_body) = explode("\r\n\r\n",$response,2); 
		$response_header_lines = explode("\r\n",$response_headers); 

		// first line of headers is the HTTP response code 
		if (preg_match('@^HTTP/[0-9]\.[0-9] ([0-9]{3})@',$response_header_lines[0], $matches)) { 
			$response_code = $matches[1]; 
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

				$details['source_key'] = "your usaepay source key";
				$details['transaction_type'] = 'sale';
				$details['currency_code'] = 'USD';
				$details['test_mode'] = 'false';

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