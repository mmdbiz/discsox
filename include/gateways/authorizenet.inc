<?php

class Authorizenet{

	var $debug = false;
	
	var $aim = true;
	var $cfg = array();
	var $next_customer_number = null;
	var $declineMessage = null;
	var $error = null;
	var $failed = false;
	var $next_order_number = null;
	var $params = array();
	var $request = null;
	var $results = array();
	var $submission_URL = 'https://secure2.authorize.net/gateway/transact.dll';
	var $x_Method = 'CC';
	var $x_Version = '3.1';
	
	// --------------------------------------------------------------
	function __construct(){
		
	}

	// --------------------------------------------------------------
	function processPayment(){
		
		global $_Common;
		global $_Template;

		$this->loadParams();
		$this->setDescription();

		if($this->cfg['status']['debug'] == 'true'){
			$this->debug = true;
		}
		if($this->debug){
			$_Common->debugPrint($this->params,"Authorizenet params");
		}

		if($this->aim){
			include_once("curl.inc");
			$curl = new Curl();
			$request = $this->createRequestString();
			$response = $curl->sendRequest($this->submission_URL,$request);
			if(!$response){
				$_Common->printErrorMessage("Authorizenet Payment Module Error","There was no response from the curl request.");
			}
			$this->ParseAIMResponse($response);
		}
		else{
			if(isset($_REQUEST['x_response_code'])){
				$this->ParseSIMResponse();
			}
			else{
				$vars['hidden_fields'] = $this->params;
				$vars['post_url'] = $this->submission_URL;
				print $_Template->Open("templates/submit.html",$vars,true);
				exit;
			}
		}
		
		if($this->debug){
			$_Common->debugPrint($this->results,"Authorizenet Results");
		}
		
		if($this->results['x_response_code'] == 1){
			$_SESSION['paid'] = 'true';
			$_SESSION['transaction_status'] = 'Completed';
			return true;	
		}
		else{
			$this->declineMessage = $this->results['x_response_reason_text'];
			return false;	
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
		$this->cfg = $gateway->readConfig('authorizenet');

		if(count($this->cfg) == 0){
			$_Common->printErrorMessage("Payment Module Error",
										"Could not read configuration information for authorizenet payment module");
		}

		// form fields		
		$this->params['x_Company'] = 'billaddress_company';
		$this->params['x_First_Name'] = 'billaddress_firstname';
		$this->params['x_Last_Name'] = 'billaddress_lastname';
		$this->params['x_Address'] = 'billaddress_addr1,&nbsp;,billaddress_addr2';
		$this->params['x_City'] = 'billaddress_city';
		$this->params['x_State'] = 'billaddress_state';
		$this->params['x_Zip'] = 'billaddress_postalcode';
		$this->params['x_Country'] = 'billaddress_country';
		$this->params['x_Phone'] = 'billaddress_areacode,-,billaddress_phone';
		$this->params['x_Email'] = 'billaddress_email';
		$this->params['x_Avs_Filter'] = 'billaddress_addr1';
		
		$this->params['x_Ship_To_First_Name'] = 'shipaddress_firstname';
		$this->params['x_Ship_To_Last_Name'] = 'shipaddress_lastname';
		$this->params['x_Ship_To_Address'] = 'shipaddress_addr1,&nbsp;,shipaddress_addr2';
		$this->params['x_Ship_To_City'] = 'shipaddress_city';
		$this->params['x_Ship_To_State'] = 'shipaddress_state';
		$this->params['x_Ship_To_Zip'] = 'shipaddress_postalcode';
		$this->params['x_Ship_To_Country'] = 'shipaddress_country';
		$this->params['x_Ship_To_Phone'] = 'shipaddress_areacode,-,shipaddress_phone';
		
		$this->params['x_Card_Type'] = 'credit_card_type';
		$this->params['x_Card_Num'] = 'card_number';
		$this->params['x_Exp_Date'] = 'expire_month,expire_yr';
		$this->params['x_Card_Code'] = 'cvv2';
		
		$this->params['x_Bank_Acct_Num'] = 'account_number';
		$this->params['x_Bank_Name'] = 'bank_name';
		$this->params['x_Bank_ABA_Code'] = 'aba_routing_code';

		$this->params['x_Freight'] = 'shipping';
		$this->params['x_Tax'] = 'salestax';
		$this->params['x_Amount'] = 'grandtotal';

		// manually set this
		if(isset($_SESSION['expire_year'])){
			$_SESSION['expire_yr'] = substr($_SESSION['expire_year'],2,2);
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

		$this->params['x_Method'] = $this->x_Method;
		$this->params['x_Version'] = $this->x_Version;
		$this->params['x_Invoice_Num'] = $this->next_order_number;
		$this->params['x_Cust_ID'] = $this->next_customer_number;

		if($this->cfg['status']['related_payment_form'] == "check.html"){
			$this->cfg['fields']['x_Method'] = "ECHECK";
		}

		if(isset($_Totals['insurance']) && $_Totals['insurance'] > 0){
			$this->params['x_Freight'] = $_Common->format_price($this->params['x_Freight'] + $_Totals['insurance']);
		}
		
		// load the fields from the database into the params
		// and remove the ones we don't need.
		
		if(isset($this->cfg['fields'])){
			
			if($this->cfg['fields']['use_AIM_Method'] == 'false'){
				$this->aim = false;
			}
			unset($this->cfg['fields']['use_AIM_Method']);
			$this->submission_URL = $this->cfg['fields']['x_Submission_URL'];
			unset($this->cfg['fields']['x_Submission_URL']);
			
			// AIM fields
			if($this->aim == 'true'){
				$this->params['x_ADC_Delim_Data'] = 'true';
				$this->params['x_ADC_Delim_Character'] = '|';
				$this->params['x_Echo_Data'] = 'true';
			}
			else{
				// SIM fields
				$cart_id = session_id();
				$this->params['x_Relay_URL'] = $_CF['basics']['web_site_url'] . "/process.php?cart_id=$cart_id";
				if(!empty($_CF['basics']['secure_url'])){
					$this->params['x_Relay_URL'] = $_CF['basics']['secure_url'] . "/process.php?cart_id=$cart_id";
				}
				$this->params['x_Relay_Response'] = 'true';
				
				// Create and add SIM fields to params
				$fpFields = $this->CreateFPFields($this->cfg['fields']['x_Login'], 
												  $this->cfg['fields']['x_Tran_Key'],
												  $this->params['x_Amount'],
												  $this->cfg['fields']['x_Currency_Code']);
												  
				unset($this->cfg['fields']['x_Tran_Key']);
				
				$this->params = array_merge($this->params,$fpFields);
			}
			


			$this->params = array_merge($this->params,$this->cfg['fields']);
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
		$this->params['x_description'] = $strDesc;
	}


	// --------------------------------------------------------------
	// Creates the encoded request string
	// --------------------------------------------------------------
	function createRequestString(){
        $str = "";
        foreach($this->params as $key=>$val){
            $str .= "$key=" . urlencode(str_replace(',', '', $val)) . '&';
        }
	    // remove the last ampersand
        $str = substr($str, 0, -1);	
		return $str;		
	}

	// --------------------------------------------------------------
	// Parses the SIM response fields
	// --------------------------------------------------------------
	function ParseSIMResponse(){

		global $_Common;

		// get just the authorizenet x_ keys
		foreach($_REQUEST as $key=>$val){
			if(substr($key,0,2)== 'x_'){
				$this->results[$key] = trim($val);
			}
		}
		if(intval($this->results['x_response_code']) != 1){
			$responseText = $this->getRealResponseText($this->results['x_response_code'],
													   $this->results['x_response_reason_code'],
										               $this->results['x_response_reason_text']);
			if(trim($responseText) != ""){
				$this->results['x_response_reason_text'] = trim($responseText);
			}
		}
	}
	
	// --------------------------------------------------------------
	// Parses the AIM response fields
	// --------------------------------------------------------------
	function ParseAIMResponse($result){

		global $_Common;
		
		$delim = $this->params['x_ADC_Delim_Character'];

		if($this->debug){
			$_Common->debugPrint($result,"Curl Response");
		}
		
		$results = explode($delim,$result);

		$resultFlds = array('x_response_code' => $results[0],
							'x_response_subcode' => $results[1],
							'x_response_reason_code' => $results[2],
							'x_response_reason_text' => $results[3],
							'x_auth_code' => $results[4],
							'x_avs_code' => $results[5],
							'x_trans_id' => $results[6],
							'x_cvv2_code' => $results[39],
							'avs_response' => $this->getAVSResponseText($results[5]),
							'cvv2_response' => $this->getCVV2ResponseText($results[39]));

		if(intval($results[0]) != 1){
			$responseText = $this->getRealResponseText($results[0],$results[2],$results[3]);
			if(trim($responseText) != ""){
				$this->results['x_response_reason_text'] = trim($responseText);
			}
			if($this->debug){
				$_Common->debugPrint($resultFlds['x_response_reason_text'],"True response message");	
			}
		}
		
		foreach($resultFlds as $key=>$val){
			$this->results[$key] = trim($val);	
		}

	}	
	// --------------------------------------------------------------
	// Sets the actual CVV2 response
	// --------------------------------------------------------------
    function getCVV2ResponseText($cvs_code){
		
		global $_Common;
        if($this->debug){
			$_Common->debugPrint("CVS Code: $cvs_code");
		}

        switch($cvs_code) {
			case "M":
				return "Match";
				break;
			case "N":
				return "No Match";
				break;
			case "P":
				return "Not Processed";
				break;
			case "S":
				return "Should Have Been Present";
				break;
			case "U":
				return "Issuer Unable To Process Request";
				break;
        }
    }
	// --------------------------------------------------------------
	// Sets the actual AVS response
	// --------------------------------------------------------------
    function getAVSResponseText($avs_code) {
		
		global $_Common;
        if($this->debug){
			$_Common->debugPrint("AVS Code: $avs_code");
		}

        switch($avs_code) {
			case "A":
				return "Address (Street) Matches, ZIP does not.";
				break;
			case "B":
				return "Address information not provided for AVS check.";
				break;
			case "E":
				return "AVS Error.";
				break;
			case "G":
				return "Non-US Card Issuing Bank.";
				break;
			case "N":
				return "No Match on Address (Street) or ZIP.";
				break;
			case "P":
				return "AVS not applicable for this transaction.";
				break;
			case "R":
				return "Retry - System unavailable or timed out.";
				break;
			case "S":
				return "Service not supported by issuer.";
				break;
			case "U":
				return "Address information is unavailable.";
				break;
			case "W":
				return "9 digit ZIP Matches, Address (Street) does not.";
				break;
			case "X":
				return "Address (Street) and 9 digit ZIP match.";
				break;
			case "Y":
				return "Address (Street) and 5 digit ZIP match.";
				break;
			case "Z":
				return "5 digit ZIP matches, Address (Street) does not.";
				break;
        }
    }
    // --------------------------------------------------------------
    // Gets the full reason text from a text file based on return codes
	// --------------------------------------------------------------
    function getRealResponseText($resp_code,$sub_code,$msg = NULL) {
		
		global $_Common;
        if($this->debug){
			$_Common->debugPrint("Response Code: $resp_code, Sub Code: $sub_code");
		}
		$codeFile = "include/gateways/authorizenet.response.codes.txt";
		$message = "";
		$handle = fopen($codeFile, "r");
		while(($data = fgetcsv($handle, 1000, "\t")) !== FALSE) {
			if(trim($data[0]) == $resp_code && trim($data[1]) == $sub_code){
				$message = trim($data[2]) . " " . trim($data[3]);
				break;
			}
		}
		fclose($handle);
		return $message;
	}
	

	// SIM Functions

	// Calculates fingerprint.
	function CreateFPFields($loginid, $x_tran_key, $amount, $currency = ""){

		// Seed random number for security and better randomness.
		srand(time());
		$sequence = rand(1, 1000);

		$tstamp = time ();
		$fingerprint = $this->hmac($x_tran_key, $loginid . "^" . $sequence . "^" . $tstamp . "^" . $amount . "^" . $currency);
		$result = array('x_FP_Sequence'  => $sequence,
						'x_FP_Timestamp' => $tstamp,
						'x_FP_Hash'		 => $fingerprint);
		return $result;
	}

	// compute HMAC-MD5 - Uses PHP mhash extension. Be sure to enable the extension
	function hmac ($key, $data){
		return (bin2hex(mhash(MHASH_MD5, $data, $key)));
	}

	// --------------------------------------------------------------
	// Loads the default values into the database when uploaded
	// --------------------------------------------------------------
	function loadDefaults(){

		global $_DB;

		$count = $_DB->getCount('payment_gateways', "WHERE LOWER(gateway_name) = 'authorizenet'");

		if($count == 0){

			$sql = "INSERT INTO `payment_gateways` ( `gateway_name`, `active`, `debug`, `related_payment_form` ) 
											VALUES ( 'authorizenet', 'false', 'false', 'credit_card.html')";

			$_DB->execute($sql);
			$pgid = $_DB->getInsertID('payment_gateways','pgid');
			
			if(!empty($pgid)){

				$details['x_Login'] = 'your account login';
				$details['x_Tran_Key'] = 'your transaction key';
				$details['x_Merchant_Email'] = 'your merchant email address';
				$details['x_Type'] = 'AUTH_CAPTURE';
				$details['x_Email_Customer'] = 'false';
				$details['x_Email_Merchant'] = 'true';
				$details['x_Test_Request'] = 'false';
				$details['x_Submission_URL'] = 'https://secure2.authorize.net/gateway/transact.dll';
				$details['x_Currency_Code'] = 'USD';
				$details['use_AIM_Method'] = 'true';

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









