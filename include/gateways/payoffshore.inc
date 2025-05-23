<?php

class Payoffshore{
	
	var $debug = false;
	var $gatewayName = 'payoffshore';
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

		$this->loadParams();
		
		if($this->debug){
			$_Common->debugPrint($this->params, $this->gatewayName . " params");
		}

		include_once("curl.inc");
		$curl = new Curl();
		$request = $this->createRequestString();
		$response = null;
		if(!is_null($this->submission_URL)){
			$response = $curl->sendRequest($this->submission_URL,$request);
		}
		else{
			$_Common->printErrorMessage($this->gatewayName . " Payment Module Error","The submission URL is not set in the configuration.");
		}
		if(!$response){
			$_Common->printErrorMessage($this->gatewayName . " Payment Module Error","There was no response from the curl request.");
		}
		else{
			global $transactionResponse;
			$transactionResponse = new TransactionResponse();
			$transactionResponse->raw = $response;
			$transactionResponse->parseResponse();
			$this->results = get_object_vars($transactionResponse);

			if($this->debug){
				$_Common->debugPrint($this->results, $this->gatewayName . " results");
			}

			if($this->results['authorised'] != 'true'){
				$this->declineMessage = trim($this->results['errormessage']);
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

		// form fields
		$this->params['billto_name'] = 'name_on_card';
		$this->params['billto_address'] = 'billaddress_addr1';
		$this->params['billto_postcode'] = 'billaddress_postalcode';
		$this->params['billto_tel'] = 'billaddress_phone';
		$this->params['billto_email'] = 'billaddress_email';

		$this->params['amount'] = 'grandtotal';

		$this->params['cardnumber'] = 'card_number';
		$this->params['expmonth'] = 'expire_month';
		$this->params['expyear'] = 'expire_year';
		$this->params['cvv2'] = 'cvv2';
		$this->params['premonth'] = 'pre_month';
		$this->params['preyear'] = 'pre_year';
		$this->params['issuenumber'] = 'issuenumber';

		$this->params['merchantid'] = 'merchantid';
		$this->params['licencekey'] = 'licencekey';
		$this->params['test'] = 'test_mode';
		$this->params['currency'] = 'currency_code';
		
		
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
		
		$this->params['amount'] = str_replace(',','',$this->params['amount']);
		$this->params['amount'] = str_replace('.','',$this->params['amount']);
		
		$this->params['identifier'] = $this->next_customer_number . '.' . $this->next_order_number;
		if($this->debug){
			$this->params['debug'] = 'true';
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
	// Loads the default values into the database when uploaded
	// --------------------------------------------------------------
	function loadDefaults(){

		global $_DB;

		$gateway = $this->gatewayName;

		$count = $_DB->getCount('payment_gateways', "WHERE LOWER(gateway_name) = '$gateway'");

		if($count == 0){

			$sql = "INSERT INTO `payment_gateways` ( `gateway_name`, `active`, `debug`, `related_payment_form` ) 
											VALUES ( '$gateway', 'false', 'false', 'credit_card.payoffshore.html')";

			$_DB->execute($sql);
			$pgid = $_DB->getInsertID('payment_gateways','pgid');
			
			if(!empty($pgid)){

				$details['merchantid'] = 'your payoffshore merchant id';
				$details['licencekey'] = '';
				$details['post_url'] = 'https://secure.payoffshore.com/ecom/api.xml';
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




// --------------------------------------------------------------
// Class to parse the result
// --------------------------------------------------------------

/* holder object for the returned parsed xml */
$element_value = "";

/* XML parsing functions */
function start_element($parser, $element_name, $element_attrs){}

function end_element($parser, $element_name) {
	global $element_value;
	global $transactionResponse;
	switch ($element_name) {
        case "AUTHCODE":
            $transactionResponse->authcode=$element_value;
            break;
		case "AMOUNT":
            $transactionResponse->amount=$element_value;
            break;
		case "AUTHORISED":
            $transactionResponse->authorised=$element_value;
            break;
		case "CODE":
            $transactionResponse->errorcode=$element_value;
            break;
		case "ID":
            $transactionResponse->errorid=$element_value;
            break;
		case "MESSAGE":
            $transactionResponse->errormessage=$element_value;
            break;
		case "IDENTIFIER":
            $transactionResponse->identifier=$element_value;
            break;
		case "TRANSNO":
            $transactionResponse->transno=$element_value;
            break;
    }
}
function character_data($parser, $data) {
    global $element_value;
	$element_value = $data;
}

/**
 * Transaction response class
 */
class TransactionResponse {
	var $authcode;
	var $authorised;
	var $amount;
	var $errorcode;
	var $errorid;
	var $errormessage;
	var $identifier;
	var $transno;
	var $raw;
	
	// --------------------------------------------------------------
	function __construct(){
		
	}
	
	function parseResponse()
	{
		$parser = xml_parser_create();
		xml_set_element_handler($parser, 'start_element', 'end_element');
		xml_set_character_data_handler($parser, 'character_data');
		  
		xml_parse($parser, $this->raw, TRUE) or die(sprintf('XML Error: %s at line %d',
                       xml_error_string(xml_get_error_code($parser)),
                       xml_get_current_line_number($parser)));
		xml_parser_free($parser);		
	}
}


?>