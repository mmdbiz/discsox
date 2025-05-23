<?php

class Protx
{
	
	var $debug = true;
	var $gatewayName = 'protx';
	var $cfg = array();
	var $params = array();
	var $results = array();
	
	var $next_customer_number = null;
	var $next_order_number = null;
	var $declineMessage = null;
	var $submission_URL = null;
	var $user_host = null;
	
	// --------------------------------------------------------------
	function Protx(){
		
		global $cart;
		global $_Totals;
		
		if(empty($_Totals) && !empty($cart->totals)){
			$_Totals = $cart->totals;
		}

        $this->user_host = $_SERVER['REMOTE_HOST'];
        if(trim($this->user_host) == ""){
            $this->user_host = $_SERVER['REMOTE_ADDR'];
        }

	}
	
	// --------------------------------------------------------------
	function processPayment(){
		
		global $_Common;
		global $_CR;

		$this->loadParams();
		
		if($this->debug){
			$_Common->debugPrint($this->params, $this->gatewayName . " params");
		}

		$request = $this->createRequestString();
		$response = null;

		if(!is_null($this->submission_URL)){
			$response = $this->sendRequest($this->submission_URL,$request);
		}
		else{
			$_Common->printErrorMessage($this->gatewayName . " Payment Module Error","The submission URL is not set in the configuration.");
		}
		if(!$response){
			$_Common->printErrorMessage($this->gatewayName . " Payment Module Error","There was no response from the curl request.");
		}
		else{
			$results = explode($_CR,$response);
			foreach($results as $i=>$row){
				@list($key,$val) = explode('=',$row);
				if(trim($key) != ""){
					$this->results[trim($key)] = trim($val);
				}
			}
			if($this->debug){
				$_Common->debugPrint($this->results, $this->gatewayName . " results");
			}
			if($this->results['Status'] != 'OK'){
				$this->declineMessage = trim($this->results['StatusDetail']);
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
		global $_Totals;

		include_once("cp/include/gateway.inc");
		$gateway = new Gateway();
		$this->cfg = $gateway->readConfig($this->gatewayName);
		if(count($this->cfg) == 0){
			$_Common->printErrorMessage("Payment Module Error",
										"Could not read configuration information for " . $this->gatewayName . " payment module");
		}
		$this->submission_URL = trim($this->cfg['fields']['post_url']);
		unset($this->cfg['fields']['post_url']);
		
		if($this->cfg['fields']['test_mode'] == 'true' || $this->debug){
			$this->submission_URL = 'https://ukvpstest.protx.com/VSPSimulator/VSPDirectGateway.asp';
		}

		$_SESSION['expire_yr'] = substr($_SESSION['expire_year'],-2);
		$_SESSION['pre_yr'] = substr($_SESSION['pre_year'],-2);

		// form fields
		$this->params['CardHolder'] = 'name_on_card';
		$this->params['CardNumber'] = 'card_number';
		$this->params['StartDate'] = 'pre_month|pre_yr';
		$this->params['ExpiryDate'] = 'expire_month|expire_yr';
		$this->params['IssueNumber'] = 'issuenumber';
		$this->params['CV2'] = 'cvv2';
		$this->params['CardType'] = 'credit_card_type';

		$this->params['CustomerName'] = 'billaddress_firstname| |billaddress_lastname';
		$this->params['BillingAddress'] = 'billaddress_addr1|,|billaddress_addr2|,|billaddress_city';
		$this->params['BillingPostCode'] = 'billaddress_postalcode';
		$this->params['ContactNumber'] = 'billaddress_phone';
		$this->params['CustomerEMail'] = 'billaddress_email';

		$this->params['DeliveryAddress'] = 'shipaddress_firstname| |shipaddress_lastname|,|shipaddress_addr1|,|shipaddress_addr2|,|shipaddress_city|,|shipaddress_country';
		$this->params['DeliveryPostCode'] = 'shipaddress_postalcode';

		$this->params['Amount'] = 'grandtotal';

		foreach($this->params as $key=>$val){
			if(strstr($val,'|')){
				$flds = explode('|',$val);
				$result = "";
				foreach($flds as $i=>$fld){
					if($fld == " "){
						$result .= " ";
					}
					elseif(trim($fld) == ","){
						$result .= ", ";
					}
					else{
						$result .= $this->testField($fld);
					}
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

		// misc
		$this->params['ClientIPAddress'] = $this->user_host;
		$this->params['VendorTxCode'] = $this->next_customer_number . '.' . $this->next_order_number . '.' . rand(0,32000);
		
		$this->params['Description'] = $_CF['basics']['store_name'] . " Order";
		$this->params['Basket'] = $this->setBasket();
		
		// merge params from cfg
		$this->params = array_merge($this->params,$this->cfg['fields']);
		
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
            $str .= "$key=" . urlencode(preg_replace('/,/', '', $val)) . '&';
        }
	    // remove the last ampersand
        $str = substr($str, 0, -1);
        
        if($this->debug){
			$_Common->debugPrint($str, "Request String");
		}
        
		return $str;		
	}

	// --------------------------------------------------------------
	// Creates a description from the cart fields
	// --------------------------------------------------------------
	function setBasket(){
		
		global $_CART;
		global $_Common;
		global $_Totals;

		/*
		Number of lines of detail in the basket field 
		Item 1 Description 
		Quantity of item 1 
		Unit cost item 1 without tax 
		Tax applied to of item 1 
		Cost of Item 1 inc tax 
		Total cost of item 1 (Quantity x cost inc tax)
		*/

		$vat = 0;
		if(isset($_Totals['VAT'])){
			$vat = $_Totals['VAT'];
		}

		include_once('salestax.inc');
		$taxClass = new Salestax();

		$desc = array();
		foreach($_CART as $i=>$flds){
			$sku = $flds['sku'];
			$qty = intval($flds['quantity']);
			$price = $_Common->format_price($flds['price']);
			$name = $flds['name'];
			$total = trim($flds['line_total']);
			if(isset($flds['options']) && count($flds['options']) > 0){
				foreach($flds['options'] as $j=>$oFlds){
					$oValue = trim($oFlds['value']);
					$name .= ":$oValue";
					
					if($oFlds['price'] > 0){
						$price += $oFlds['price'];
					}
				}
			}
			$tax = $taxClass->calculateVatTax($price,0);
			$itemtotal = $_Common->format_price($price + $tax);
			$total = $_Common->format_price(($price + $tax) * $qty);
			$desc[] = "$name:$qty:$price:$tax:$itemtotal:$total";
		}
		
		if($_Totals['shipping'] > 0){
			$shipping = $_Totals['shipping'];
			$method = 'Delivery';
			if(!empty($_SESSION['shipping_method'])){
				$method = $_SESSION['shipping_method'];
			}
			$desc[] = "$method:::::$shipping";
		}
		
		$strCart = count($desc) . ':';
		$strCart .= join(":",$desc);
		$strCart = str_replace("\r","",$strCart);
		$strCart = str_replace("\n","",$strCart);
		
		// reset the original VAT
		$_Totals['VAT'] = $vat;
		
		return $strCart;
	}

	// --------------------------------------------------------------
	// send request through curl
	// --------------------------------------------------------------
	function sendRequest($URL,$request){

		global $_Common;

		if(!function_exists('curl_version')){
			exit("<b>ERROR: Cannot Continue. Curl is not available through PHP</b>");
		}
		
		if(!$request){
			return false;
		}
		
		$result = NULL;

		if($this->debug){
			$_Common->debugPrint(curl_version(),"Curl Version:");
		}

		$ch = curl_init();
		if($this->debug){
			//curl_setopt($ch, CURLOPT_VERBOSE, 1);
		}
		curl_setopt($ch, CURLOPT_URL, $URL);
		curl_setopt($ch, CURLOPT_HEADER, 0);
		curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
		curl_setopt($ch, CURLOPT_POST, 1);
		curl_setopt($ch, CURLOPT_POSTFIELDS, $request);
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, 0);
		curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 0);
		//curl_setopt($ch, CURLOPT_SSLVERSION, 2);

		$result = curl_exec($ch) or die("There has been a cURL error connecting to $URL.");
		$info = curl_getinfo($ch);

		curl_close($ch);    

		if($this->debug){
			$_Common->debugPrint($result,"curl result");
			$_Common->debugPrint($info,"curl info");
		}

	return $result;
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

				$details['Vendor'] = 'Your Protx Vendor Login Name';
				$details['ClientNumber'] = '1';
				$details['TxType'] = 'PAYMENT';
				$details['VPSProtocol'] = '2.22';
				$details['post_url'] = 'https://ukvps.protx.com/VSPDirectAuth/PaymentGateway3D.asp';
				$details['Currency'] = 'GBP';
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