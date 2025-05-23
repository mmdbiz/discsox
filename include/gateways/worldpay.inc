<?php
class Worldpay{

	var $debug = false;
	
	var $cancelMessage = "Your Transaction has been cancelled.";
	var $declineMessage = null;
	var $params = array();
	var $results = array();
	var $submission_URL = "https://secure.wp3.rbsworldpay.com/wcc/purchase";	
	
	// --------------------------------------------------------------
	function __construct(){
		
	}
	
	// --------------------------------------------------------------
	function processPayment(){
		
		global $_Common;

		$this->loadParams();
		$this->setDescription();

		if($this->cfg['status']['debug'] == 'true'){
			$this->debug = true;
		}
		if($this->debug){
			$_Common->debugPrint($this->params,"WorldPay params");
		}

		// post to their site
		if(!isset($_REQUEST['transStatus'])){
			global $_Template;
			$vars['hidden_fields'] = $this->params;
			$vars['post_url'] = $this->submission_URL;
			print $_Template->Open("templates/submit.html",$vars,true);
			exit;
		}
		
		// check result from callback
		if($_REQUEST['transStatus'] == 'Y'){
			// give me all the result values except
			// the ones in the params below
			foreach($_POST as $k=>$v){
				if(!isset($this->params[$k])){
					$this->results[$k] = $v;
				}
			}
			return true;	
		}
		else{
			$this->declineMessage = $this->cancelMessage;
			return false;	
		}

		
		
	}
	
// --------------------------------------------------------------
	function loadParams(){
		
		global $_Common;

		include_once("cp/include/gateway.inc");
		$gateway = new Gateway();
		$this->cfg = $gateway->readConfig('worldpay');

		if(count($this->cfg) == 0){
			$_Common->printErrorMessage("Payment Module Error",
										"Could not read configuration information for worldpay payment module");
		}

		foreach($this->cfg['fields'] as $k=>$v){
			if($v != ""){
				$this->params[$k] = $v;
			}
		}

		if($this->cfg['fields']['testMode'] == '100'){
			$this->submission_URL = 'https://select-test.wp3.rbsworldpay.com/wcc/purchase';	
		}
		else{
			unset($this->params['testMode']);
		}

		// form fields	
		$this->params['MC_company'] = 'billaddress_company';
		$this->params['name'] = 'billaddress_firstname, ,billaddress_lastname';
		$this->params['address'] = 'billaddress_addr1, ,billaddress_addr2, ,billaddress_city, ,billaddress_county, ,billaddress_state';
		$this->params['postcode'] = 'billaddress_postalcode';
		$this->params['country'] = 'billaddress_country';
		$this->params['tel'] = 'billaddress_phone';
		$this->params['email'] = 'billaddress_email';
		
		$this->params['hideContact'] = 'true'; 
		$this->params['hideCurrency'] = 'false';

		$this->params['amount'] = 'grandtotal';
		$this->params['cartId'] = session_id();

		foreach($this->params as $key=>$val){
			if(strstr($val,',')){
				$flds = explode(',',$val);
				$result = null;
				foreach($flds as $i=>$fld){
					if(trim($this->testField($fld)) != ''){
						if(!is_null($result)){
							$result .= " " . $this->testField($fld);
						}
						else{
							$result = $this->testField($fld);
						}
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

		$this->params['address']	= trim(substr($this->params['address'],0,255));
		$this->params['name']		= trim(substr($this->params['name'],0,40));
		$this->params['postcode']	= trim(substr($this->params['postcode'],0,12));
		$this->params['tel']		= trim(substr($this->params['tel'],0,30));
		$this->params['email']		= trim(substr($this->params['email'],0,80));
		$this->params['lang']		= 'en-GB';
		
		$this->params['MC_callback'] = $this->params['resultFile'];
		// we don't want the result file to have a php extension
		unset($this->params['resultFile']);
		
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
			if(substr($key,0,11) == 'billaddress'){
				return null;	
			}
			else{
				return trim($key);
			}
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
		$this->params['desc'] = substr($strDesc,0,255);
	}
	


	// --------------------------------------------------------------
	function loadDefaults(){

		global $_DB;

		$count = $_DB->getCount('payment_gateways', "WHERE LOWER(gateway_name) = 'worldpay'");

		if($count == 0){

			$sql = "INSERT INTO `payment_gateways` ( `gateway_name`, `active`, `debug`, `related_payment_form` ) 
											VALUES ( 'worldpay', 'false', 'false', 'credit_card.html')";

			$_DB->execute($sql);
			$pgid = $_DB->getInsertID('payment_gateways','pgid');
			
			if(!empty($pgid)){

				$details['instId'] = 'your account install ID';
				$details['currency'] = 'USD';
				$details['testMode'] = '0';
				$details['authMode'] = 'A';
				$details['accId'] = '';
				$details['resultFile'] = '';

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