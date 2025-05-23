<?php

class viaklix
{
	
	var $debug = false;
	var $gatewayName = 'viaklix';
	var $cfg = array();
	var $params = array();
	var $results = array();
	
	var $next_customer_number = null;
	var $next_order_number = null;
	var $declineMessage = null;
	var $submission_URL = null;
	
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
		global $_Template;

		$this->loadParams();
		
		if($this->debug){
			$_Common->debugPrint($this->params, $this->gatewayName . " params");
		}
		
		if(isset($_REQUEST['ssl_result'])){
			
			//$_Common->debugPrint($_REQUEST,"Return Params");
			//exit;

			foreach($_REQUEST as $key=>$val){
				if(substr($key,0,3) == 'ssl'){
					if($key == 'ssl_result_message'){
						$val = $this->getResponseMessage($val);	
					}
					if($key == 'ssl_avs_response'){
						$val = $this->getAvsMessage($val);	
					}
					if($key == 'ssl_cvv2_response'){
						$val = $this->getCvv2Message($val);	
					}
					$this->results[$key] = trim($val);
				}
			}

			if($this->results['ssl_result'] != 0){
				// bad result
				$this->declineMessage = trim($this->results['ssl_result_message']);
				return false;	
			}
			else{
				// good result
				return true;	
			}
		}
		else{
			$vars['hidden_fields'] = $this->params;
			$vars['post_url'] = $this->submission_URL;
			print $_Template->Open("templates/submit.html",$vars,true);
			exit;
		}
	}

	// --------------------------------------------------------------
	function loadParams(){
		
		global $_CF;
		global $_Common;
		global $_DB;
		global $_Registry;
		global $_Totals;

		include_once("cp/include/gateway.inc");
		$gateway = new Gateway();
		$this->cfg = $gateway->readConfig($this->gatewayName);
		if(count($this->cfg) == 0){
			$_Common->printErrorMessage("Payment Module Error",
										"Could not read configuration information for " . $this->gatewayName . " payment module");
		}
		$this->submission_URL = trim($this->cfg['fields']['post_url']);

		$_SESSION['expire_yr'] = substr($_SESSION['expire_year'],-2);
		
		// form fields
		$this->params['ssl_company'] = 'billaddress_company';
		$this->params['ssl_first_name'] = 'billaddress_firstname';
		$this->params['ssl_last_name'] = 'billaddress_lastname';
		$this->params['ssl_avs_address'] = 'billaddress_addr1';
		$this->params['ssl_address2'] = 'billaddress_addr2';
		$this->params['ssl_city'] = 'billaddress_city';
		$this->params['ssl_state'] = 'billaddress_state';
		$this->params['ssl_avs_zip'] = 'billaddress_postalcode';
		$this->params['ssl_country'] = 'billaddress_country';
		$this->params['ssl_phone'] = 'billaddress_areacode,-,billaddress_phone';
		$this->params['ssl_email'] = 'billaddress_email';
		
		$this->params['ssl_ship_to_company'] = 'shipaddress_company';
		$this->params['ssl_ship_to_first_name'] = 'shipaddress_firstname';
		$this->params['ssl_ship_to_last_name'] = 'shipaddress_lastname';
		$this->params['ssl_ship_to_address'] = 'shipaddress_addr1, ,shipaddress_addr2';
		$this->params['ssl_ship_to_city'] = 'shipaddress_city';
		$this->params['ssl_ship_to_state'] = 'shipaddress_state';
		$this->params['ssl_ship_to_zip'] = 'shipaddress_postalcode';
		$this->params['ssl_ship_to_country'] = 'shipaddress_country';
		$this->params['shipping_method'] = 'shipping_method';

		$this->params['ssl_card_number'] = 'card_number';
		$this->params['ssl_exp_date'] = 'expire_month,expire_yr';
		$this->params['ssl_cvv2cvc2'] = 'cvv2';

		// totals
		$this->params['ssl_amount'] = 'grandtotal';
		$this->params['ssl_salestax'] = 'salestax';
		
		// loads the fields based on the values above.
		// test each field to see if it's a valid key
		// in Totals, SESSION, REQUEST, or the config entries
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

		// config fields
		$this->params['ssl_merchant_id'] = $this->cfg['fields']['ssl_merchant_id'];
		$this->params['ssl_user_id'] = $this->cfg['fields']['ssl_user_id'];
		$this->params['ssl_pin'] = $this->cfg['fields']['ssl_pin'];
		$this->params['ssl_transaction_type'] = $this->cfg['fields']['ssl_transaction_type'];
		$this->params['ssl_test_mode'] = $this->cfg['fields']['test_mode'];
		$this->params['ssl_txn_currency_code'] = $this->cfg['fields']['currency_code'];
		
		$this->params['ssl_receipt_link_method'] = 'REDG';
		$this->params['ssl_show_form'] = 'false';

		$this->params['ssl_cvv2'] = 'present';
		
		$cart_id = session_id();
		//$this->params['ssl_receipt_link_url'] = $_CF['basics']['web_site_url'] . "/process.php?cart_id=$cart_id";
		$this->params['ssl_receipt_link_url'] = $_CF['basics']['web_site_url'] . "/process.php";
		if(!empty($_CF['basics']['secure_url'])){
			//$this->params['ssl_receipt_link_url'] = $_CF['basics']['secure_url'] . "/process.php?cart_id=$cart_id";
			$this->params['ssl_receipt_link_url'] = $_CF['basics']['secure_url'] . "/process.php";
		}
		
		$this->params['ssl_customer_code'] = $this->next_customer_number;
		$this->params['ssl_invoice_number'] = $this->next_customer_number . '.' . $this->next_order_number;
	
		$this->params['ssl_description'] = $this->setDescription();
		
		if($this->debug){
			$this->params['ssl_test_mode'] = 'true';
		}
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
	// Gets the full text for a response code
	// --------------------------------------------------------------
	function getResponseMessage($code){
		$responseCodes['APPROVAL'] = 'approved';
		$responseCodes['APPROVED'] = 'approved';
		$responseCodes['ACCEPTED'] = 'frequency approval';
		$responseCodes['BAL.: 99999999.99'] = 'debit card balance inquiry response';
		$responseCodes['PICK UP CARD'] = 'pick up card';
		$responseCodes['AMOUNT ERROR'] = 'tran amount error';
		$responseCodes['APPL TYPE ERROR'] = 'call for assistance';
		$responseCodes['DECLINED'] = 'do not honor';
		$responseCodes['DECLINED-HELP 9999'] = 'system error';
		$responseCodes['EXCEEDS BAL.'] = 'req. exceeds balance';
		$responseCodes['EXPIRED CARD'] = 'expired card';
		$responseCodes['INVALID CARD'] = 'invalid card';
		$responseCodes['INCORRECT PIN'] = 'invalid pin';
		$responseCodes['INVALID TERM ID'] = 'invalid terminal id';
		$responseCodes['INVLD TERM ID 1'] = 'invalid merchant number';
		$responseCodes['INVLD TERM ID 2'] = 'invalid se number';
		$responseCodes['INVLD VOID DATA'] = 'invalid data';
		$responseCodes['MUST SETTLE MMDD'] = 'must settle pos device, open batch is more than 7 days old';
		$responseCodes['ON FILE'] = 'cardholder not found';
		$responseCodes['RECORD NOT FOUND'] = 'record not on host';
		$responseCodes['SERV NOT ALLOWED'] = 'invalid request';
		$responseCodes['SEQ ERR PLS CALL'] = 'call for assistance';
		$responseCodes['CALL AUTH. CENTER'] = 'refer to issuer';
		$responseCodes['CALL REF.; 999999'] = 'refer to issuer';
		$responseCodes['DECLINE CVV2'] = 'do not honor; declined due to cvv2 mismatch/failure';
		$code = strtoupper($code);
		if(isset($responseCodes[$code])){
			return $responseCodes[$code];
		}
		else{
			return $code;
		}
	}
	// --------------------------------------------------------------
	// Gets the full text for a AVS response code
	// --------------------------------------------------------------
	function getAvsMessage($code){
		$avsCodes['A'] = 'address (street) matches, postalcode does not match.';
		$avsCodes['E'] = 'avs error';
		$avsCodes['N'] = 'no match on address (street) or postalcode';
		$avsCodes['R'] = 'retry, system unavailable or timed out';
		$avsCodes['S'] = 'service not supported by user';
		$avsCodes['U'] = 'address information is unavailable';
		$avsCodes['W'] = '9 digit postalcode matches, address (street) does not match.';
		$avsCodes['X'] = 'extract avs match';
		$avsCodes['Y'] = 'address (street) and 5 digit postalcode match.';
		$avsCodes['Z'] = '5 digit postalcode matches, address (street) does not match.';
		$code = strtoupper($code);
		if(isset($avsCodes[$code])){
			return $avsCodes[$code];
		}
		else{
			return $code;	
		}
	}
	// --------------------------------------------------------------
	// Gets the full text for a CVV2 response code
	// --------------------------------------------------------------
	function getCvv2Message($code){
		$cvv2Codes['M'] = 'CVV2 Match';
		$cvv2Codes['N'] = 'CVV2 No match';
		$cvv2Codes['P'] = 'Not Processed';
		$cvv2Codes['S'] = 'Issuer indicates that CVV2 data should be present on the card, but the merchant has indicated that the CVV2 data is not resent on the card';
		$cvv2Codes['U'] = 'Issuer has not certified for CVV2 or Issuer has not provided Visa with the CVV2 encryption Keys.';
		$code = strtoupper($code);
		if(isset($cvv2Codes[$code])){
			return $cvv2Codes[$code];
		}
		else{
			return $code;	
		}
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

				$details['ssl_merchant_id'] = 'your viaklix merchant id';
				$details['ssl_user_id'] = 'your viaklix user id';
				$details['ssl_pin'] = 'your viaklix PIN';
				$details['post_url'] = 'https://www2.viaklix.com/process.asp';
				$details['ssl_transaction_type'] = 'SALE';
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