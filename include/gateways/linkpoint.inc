<?php

class Linkpoint{

	var $debug = false;
	
	var $cfg = array();
	var $declineMessage = null;
	var $error = null;
	var $hostip = null;
	var $keyfile = null;
	var $next_order_number = null;
	var $params = array();
	var $results = array();

	// --------------------------------------------------------------
	function __construct(){
        $this->hostip = getenv('REMOTE_HOST');
        if(trim($this->hostip) == ""){
            $this->hostip = getenv('REMOTE_ADDR');
        }
	}

	// --------------------------------------------------------------
	function processPayment(){
		
		global $_Common;
		global $_Template;
		global $_Totals;
		
		$this->loadParams();
		
		if($this->debug){
			$_Common->debugPrint($_Totals, "Cart Totals");
			$_Common->debugPrint($this->params, "Linkpoint Parameters to send");
		}
		
		$xml = $this->buildXML($this->params);

		$response = $this->sendRequest($xml);
		if(!$response){
			$_Common->printErrorMessage("Linkpoint Payment Module Error","There was no response from the curl request.");
		}
		
		$this->results = $response;
		
		if($this->debug){
			$_Common->debugPrint($response,'Linkpoint Response');
		}
		
		if($this->results['r_approved'] == "APPROVED"){
			return true;	
		}
		else{
			$vars['error_title'] = "Payment Processing Error";
			$vars['decline_message'] = $this->results['r_error'];
			print $_Template->Open("templates/declined.html",$vars,true);
			exit;
		}
	}
	
	// --------------------------------------------------------------
	function loadParams(){
		
		global $_CF;
		global $_Common;
		global $_Totals;

		include_once("cp/include/gateway.inc");
		$gateway = new Gateway();
		$this->cfg = $gateway->readConfig('linkpoint');

		if(count($this->cfg) == 0){
			$_Common->printErrorMessage("Payment Module Error",
										"Could not read configuration information for linkpoint payment module");
		}
		
		if($this->cfg['fields']['debug'] == "true"){
			$this->debug = true;
		}
		
		if($this->debug){
			$_Common->debugPrint($this->cfg,"Linkpoint configuration settings");
		}


		# TOTALS
		$this->params["subtotal"]    = 'subtotal';
		$this->params["tax"]         = 'salestax';
		$this->params["shipping"]    = 'shipping';
		$this->params["chargetotal"] = 'grandtotal';

		# CARD INFO
		$_REQUEST['expire_yr'] = substr($_REQUEST['expire_year'],2,2);
		$this->params["cardnumber"]   = 'card_number';
		$this->params["cardexpmonth"] = 'expire_month';
		$this->params["cardexpyear"]  = 'expire_yr';
		$this->params["cvmindicator"] = "provided";
		$this->params["cvmvalue"]     = 'cvv2';

		# BILLING INFO
		$this->params["name"]     = 'billaddress_firstname, ,billaddress_lastname';
		$this->params["company"]  = 'billaddress_company';
		$this->params["address1"] = 'billaddress_addr1';
		$this->params["address2"] = 'billaddress_addr2';
		$this->params["city"]     = 'billaddress_city';
		$this->params["state"]    = 'billaddress_state';
		$this->params["country"]  = 'billaddress_country';
		$this->params["zip"]      = 'billaddress_postalcode';
		$this->params["phone"]    = 'billaddress_areacode,-,billaddress_phone';
		$this->params["email"]    = 'billaddress_email';

		if(!empty($_REQUEST['billaddress_addr1'])){
			$flds = explode(' ',$_REQUEST['billaddress_addr1']);
			$this->params["addrnum"]  = $flds[0];
		}

		# SHIPPING INFO
		$this->params["sname"]     = 'shipaddress_firstname, ,shipaddress_lastname';
		$this->params["saddress1"] = 'shipaddress_addr1';
		$this->params["saddress2"] = 'shipaddress_addr2';
		$this->params["scity"]     = 'shipaddress_city';
		$this->params["sstate"]    = 'shipaddress_state';
		$this->params["szip"]      = 'shipaddress_postalcode';
		$this->params["scountry"]  = 'shipaddress_country';
		$this->params["comments"]  = 'comments';

		# FILL PARAMS
		foreach($this->params as $key=>$val){
			if(strstr($val,',')){
				$flds = explode(',',$val);
				$result = "";
				foreach($flds as $i=>$fld){
					if($fld == " "){
						$result .= " ";
					}
					elseif($fld != ""){
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
				$this->params[$key] = $this->xmlentities($result);
			}
		}

		if(isset($_Totals['discount']) && $_Totals['discount'] > 0){
			$this->params["subtotal"] = $_Common->format_price($this->params['subtotal'] - $_Totals['discount']);
		}

		if(isset($_Totals['insurance']) && $_Totals['insurance'] > 0){
			$this->params["shipping"] = $_Common->format_price($this->params['shipping'] + $_Totals['insurance']);
		}

		$cwd = getcwd();

		# THESE ARE FROM CONFIG OR HARDCODED
		$storeNum = $this->cfg['fields']['store_number'];
		$this->params["host"]			= "secure.linkpt.net";
		$this->params["port"]			= "1129";
		$this->params["configfile"]		= $storeNum;
		$this->params["ordertype"]		= $this->cfg['fields']['order_type'];
		$this->params["result"]			= $this->cfg['fields']['transaction_type'];
		$this->params["ordernum"]		= $this->next_order_number;
		$this->params["taxexempt"]		= "N";
		$this->params["terminaltype"]	= "UNSPECIFIED";
		$this->params["ip"]				= $this->hostip;
		
		$this->params["keyfile"] = "$cwd/include/gateways/$storeNum.pem";
		
		# ADD THE CART STUFF
		$this->setDescription();
		
	}

	// --------------------------------------------------------------
	// Tests a field key to try find a match in the cfg, totals, 
	// request or session variables.
	// --------------------------------------------------------------
	function testField($key){

		global $_Totals;
	
		if(isset($this->cfg['fields'][$key])){
			return trim($this->cfg['fields'][$key]);
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

		$j = 1;
		foreach($_CART as $i=>$flds){

			$this->params['items']["item$j"]['id'] = $flds['sku'];
			$this->params['items']["item$j"]['description'] = $this->xmlentities($flds['name']);
			$this->params['items']["item$j"]['quantity'] = intval($flds['quantity']);
			$this->params['items']["item$j"]['price'] = trim($flds['line_total']);
			
			if(isset($flds['options']) && count($flds['options']) > 0){
				$z = 1;
				foreach($flds['options'] as $k=>$oFlds){
					if(strtolower(trim($oFlds['name'])) == "option"){
						$oFlds['name'] = "Option" . $z;
					}
					$this->params['items']["item$j"]["option$z"]['name'] = $this->xmlentities(trim($oFlds['name']));
					$this->params['items']["item$j"]["option$z"]['value'] = $this->xmlentities(trim($oFlds['value']));
					$z++;
				}
			}
			$j++;
		}
	}
	
	// --------------------------------------------------------------
	// Builds the linkpoint XML
	// --------------------------------------------------------------
	function buildXML($pdata){

		### ORDEROPTIONS NODE ###
		$xml = "<order><orderoptions>";

		if (isset($pdata["ordertype"]))
			$xml .= "<ordertype>" . $pdata["ordertype"] . "</ordertype>";

		if (isset($pdata["result"]))
			$xml .= "<result>" . $pdata["result"] . "</result>";

		$xml .= "</orderoptions>";


		### CREDITCARD NODE ###
		$xml .= "<creditcard>";

		if (isset($pdata["cardnumber"]))
			$xml .= "<cardnumber>" . $pdata["cardnumber"] . "</cardnumber>";

		if (isset($pdata["cardexpmonth"]))
			$xml .= "<cardexpmonth>" . $pdata["cardexpmonth"] . "</cardexpmonth>";

		if (isset($pdata["cardexpyear"]))
			$xml .= "<cardexpyear>" . $pdata["cardexpyear"] . "</cardexpyear>";

		if (isset($pdata["cvmvalue"]))
			$xml .= "<cvmvalue>" . $pdata["cvmvalue"] . "</cvmvalue>";

		if (isset($pdata["cvmindicator"]))
			$xml .= "<cvmindicator>" . $pdata["cvmindicator"] . "</cvmindicator>";

		if (isset($pdata["track"]))
			$xml .= "<track>" . $pdata["track"] . "</track>";

		$xml .= "</creditcard>";


		### BILLING NODE ###
		$xml .= "<billing>";

		if (isset($pdata["name"]))
			$xml .= "<name>" . $pdata["name"] . "</name>";

		if (isset($pdata["company"]))
			$xml .= "<company>" . $pdata["company"] . "</company>";

		if (isset($pdata["address1"]))
			$xml .= "<address1>" . $pdata["address1"] . "</address1>";
		elseif (isset($pdata["address"]))
			$xml .= "<address1>" . $pdata["address"] . "</address1>";

		if (isset($pdata["address2"]))
			$xml .= "<address2>" . $pdata["address2"] . "</address2>";

		if (isset($pdata["city"]))
			$xml .= "<city>" . $pdata["city"] . "</city>";
			
		if (isset($pdata["state"]))
			$xml .= "<state>" . $pdata["state"] . "</state>";
			
		if (isset($pdata["zip"]))
			$xml .= "<zip>" . $pdata["zip"] . "</zip>";

		if (isset($pdata["country"]))
			$xml .= "<country>" . $pdata["country"] . "</country>";

		if (isset($pdata["userid"]))
			$xml .= "<userid>" . $pdata["userid"] . "</userid>";

		if (isset($pdata["email"]))
			$xml .= "<email>" . $pdata["email"] . "</email>";

		if (isset($pdata["phone"]))
			$xml .= "<phone>" . $pdata["phone"] . "</phone>";

		if (isset($pdata["fax"]))
			$xml .= "<fax>" . $pdata["fax"] . "</fax>";

		if (isset($pdata["addrnum"]))
			$xml .= "<addrnum>" . $pdata["addrnum"] . "</addrnum>";

		$xml .= "</billing>";

		
		## SHIPPING NODE ##
		$xml .= "<shipping>";

		if (isset($pdata["sname"]))
			$xml .= "<name>" . $pdata["sname"] . "</name>";

		if (isset($pdata["saddress1"]))
			$xml .= "<address1>" . $pdata["saddress1"] . "</address1>";

		if (isset($pdata["saddress2"]))
			$xml .= "<address2>" . $pdata["saddress2"] . "</address2>";

		if (isset($pdata["scity"]))
			$xml .= "<city>" . $pdata["scity"] . "</city>";

		if (isset($pdata["sstate"]))
			$xml .= "<state>" . $pdata["sstate"] . "</state>";
		elseif (isset($pdata["state"]))
			$xml .= "<state>" . $pdata["sstate"] . "</state>";

		if (isset($pdata["szip"]))
			$xml .= "<zip>" . $pdata["szip"] . "</zip>";
		elseif (isset($pdata["sip"]))
			$xml .= "<zip>" . $pdata["zip"] . "</zip>";

		if (isset($pdata["scountry"]))
			$xml .= "<country>" . $pdata["scountry"] . "</country>";

		if (isset($pdata["scarrier"]))
			$xml .= "<carrier>" . $pdata["scarrier"] . "</carrier>";

		if (isset($pdata["sitems"]))
			$xml .= "<items>" . $pdata["sitems"] . "</items>";

		if (isset($pdata["sweight"]))
			$xml .= "<weight>" . $pdata["sweight"] . "</weight>";

		if (isset($pdata["stotal"]))
			$xml .= "<total>" . $pdata["stotal"] . "</total>";

		$xml .= "</shipping>";


		### TRANSACTIONDETAILS NODE ###
		$xml .= "<transactiondetails>";

		if (isset($pdata["oid"]))
			$xml .= "<oid>" . $pdata["oid"] . "</oid>";

		if (isset($pdata["ponumber"]))
			$xml .= "<ponumber>" . $pdata["ponumber"] . "</ponumber>";

		if (isset($pdata["recurring"]))
			$xml .= "<recurring>" . $pdata["recurring"] . "</recurring>";

		if (isset($pdata["taxexempt"]))
			$xml .= "<taxexempt>" . $pdata["taxexempt"] . "</taxexempt>";

		if (isset($pdata["terminaltype"]))
			$xml .= "<terminaltype>" . $pdata["terminaltype"] . "</terminaltype>";

		if (isset($pdata["ip"]))
			$xml .= "<ip>" . $pdata["ip"] . "</ip>";

		if (isset($pdata["reference_number"]))
			$xml .= "<reference_number>" . $pdata["reference_number"] . "</reference_number>";

		if (isset($pdata["transactionorigin"]))
			$xml .= "<transactionorigin>" . $pdata["transactionorigin"] . "</transactionorigin>";

		if (isset($pdata["tdate"]))
			$xml .= "<tdate>" . $pdata["tdate"] . "</tdate>";

		$xml .= "</transactiondetails>";


		### MERCHANTINFO NODE ###
		$xml .= "<merchantinfo>";

		if (isset($pdata["configfile"]))
			$xml .= "<configfile>" . $pdata["configfile"] . "</configfile>";

		if (isset($pdata["keyfile"]))
			$xml .= "<keyfile>" . $pdata["keyfile"] . "</keyfile>";

		if (isset($pdata["host"]))
			$xml .= "<host>" . $pdata["host"] . "</host>";

		if (isset($pdata["port"]))
			$xml .= "<port>" . $pdata["port"] . "</port>";

		if (isset($pdata["appname"]))
			$xml .= "<appname>" . $pdata["appname"] . "</appname>";

		$xml .= "</merchantinfo>";



		### PAYMENT NODE ###
		$xml .= "<payment>";

		if (isset($pdata["chargetotal"]))
			$xml .= "<chargetotal>" . $pdata["chargetotal"] . "</chargetotal>";

		if (isset($pdata["tax"]))
			$xml .= "<tax>" . $pdata["tax"] . "</tax>";

		if (isset($pdata["vattax"]))
			$xml .= "<vattax>" . $pdata["vattax"] . "</vattax>";

		if (isset($pdata["shipping"]))
			$xml .= "<shipping>" . $pdata["shipping"] . "</shipping>";

		if (isset($pdata["subtotal"]))
			$xml .= "<subtotal>" . $pdata["subtotal"] . "</subtotal>";

		$xml .= "</payment>";


		### CHECK NODE ### 


		if (isset($pdata["voidcheck"]))
		{
			$xml .= "<telecheck><void>1</void></telecheck>";
		}
		elseif (isset($pdata["routing"]))
		{
			$xml .= "<telecheck>";
			$xml .= "<routing>" . $pdata["routing"] . "</routing>";

			if (isset($pdata["account"]))
				$xml .= "<account>" . $pdata["account"] . "</account>";

			if (isset($pdata["bankname"]))
				$xml .= "<bankname>" . $pdata["bankname"] . "</bankname>";
	
			if (isset($pdata["bankstate"]))
				$xml .= "<bankstate>" . $pdata["bankstate"] . "</bankstate>";

			if (isset($pdata["ssn"]))
				$xml .= "<ssn>" . $pdata["ssn"] . "</ssn>";

			if (isset($pdata["dl"]))
				$xml .= "<dl>" . $pdata["dl"] . "</dl>";

			if (isset($pdata["dlstate"]))
				$xml .= "<dlstate>" . $pdata["dlstate"] . "</dlstate>";

			if (isset($pdata["checknumber"]))
				$xml .= "<checknumber>" . $pdata["checknumber"] . "</checknumber>";
				
			if (isset($pdata["accounttype"]))
				$xml .= "<accounttype>" . $pdata["accounttype"] . "</accounttype>";

			$xml .= "</telecheck>";
		}


		### PERIODIC NODE ###

		if (isset($pdata["startdate"]))
		{
			$xml .= "<periodic>";

			$xml .= "<startdate>" . $pdata["startdate"] . "</startdate>";

			if (isset($pdata["installments"]))
				$xml .= "<installments>" . $pdata["installments"] . "</installments>";

			if (isset($pdata["threshold"]))
						$xml .= "<threshold>" . $pdata["threshold"] . "</threshold>";

			if (isset($pdata["periodicity"]))
						$xml .= "<periodicity>" . $pdata["periodicity"] . "</periodicity>";

			if (isset($pdata["pbcomments"]))
						$xml .= "<comments>" . $pdata["pbcomments"] . "</comments>";

			if (isset($pdata["action"]))
				$xml .= "<action>" . $pdata["action"] . "</action>";

			$xml .= "</periodic>";
		}


		### NOTES NODE ###

		if (isset($pdata["comments"]) || isset($pdata["referred"]))
		{
			$xml .= "<notes>";

			if (isset($pdata["comments"]))
				$xml .= "<comments>" . $pdata["comments"] . "</comments>";

			if (isset($pdata["referred"]))
				$xml .= "<referred>" . $pdata["referred"] . "</referred>";

			$xml .= "</notes>";
		}

		### ITEMS AND OPTIONS NODES ###
	
		if ($this->debug)	// make it easy to see
		{						// LSGS doesn't mind whitespace
			reset($pdata);

			while (list ($key, $val) = each ($pdata))
			{
				if (is_array($val))
				{
					$otag = 0;
					$ostag = 0;
					$items_array = $val;
					$xml .= "\n<items>\n";

					while(list($key1, $val1) = each ($items_array))
					{
						$xml .= "\t<item>\n";

						while (list($key2, $val2) = each ($val1))
						{
							if (!is_array($val2))
								$xml .= "\t\t<$key2>$val2</$key2>\n";

							else
							{
								if (!$ostag)
								{
									$xml .= "\t\t<options>\n";
									$ostag = 1;
								}

								$xml .= "\t\t\t<option>\n";
								$otag = 1;
								
								while (list($key3, $val3) = each ($val2))
									$xml .= "\t\t\t\t<$key3>$val3</$key3>\n";
							}

							if ($otag)
							{
								$xml .= "\t\t\t</option>\n";
								$otag = 0;
							}
						}

						if ($ostag)
						{
							$xml .= "\t\t</options>\n";
							$ostag = 0;
						}
					$xml .= "\t</item>\n";
					}
				$xml .= "</items>\n";
				}
			}
		}

		else // !debug
		{
			while (list ($key, $val) = each ($pdata))
			{
				if (is_array($val))
				{
					$otag = 0;
					$ostag = 0;
					$items_array = $val;
					$xml .= "<items>";

					while(list($key1, $val1) = each ($items_array))
					{
						$xml .= "<item>";

						while (list($key2, $val2) = each ($val1))
						{
							if (!is_array($val2))
								$xml .= "<$key2>$val2</$key2>";

							else
							{
								if (!$ostag)
								{
									$xml .= "<options>";
									$ostag = 1;
								}

								$xml .= "<option>";
								$otag = 1;
								
								while (list($key3, $val3) = each ($val2))
									$xml .= "<$key3>$val3</$key3>";
							}

							if ($otag)
							{
								$xml .= "</option>";
								$otag = 0;
							}
						}

						if ($ostag)
						{
							$xml .= "</options>";
							$ostag = 0;
						}
					$xml .= "</item>";
					}
				$xml .= "</items>";
				}
			}
		}

		$xml .= "</order>";

		return $xml;
	}
	
	// --------------------------------------------------------------
	// Sends the request to linkpoint
	// --------------------------------------------------------------
	
	function sendRequest($xml){

		global $_Common;

        if($this->debug){
            $_Common->debugPrint(curl_version(),"Curl Version:");
            $this->showXML($xml);

        }
        
		$port = $this->params['port'];
		$host = "https://" . $this->params['host'] . ":" . $port . "/LSGSXML";
		$key = $this->params["keyfile"];
		
		$ch = curl_init ();
		curl_setopt($ch, CURLOPT_URL,$host);
        curl_setopt($ch, CURLOPT_HEADER, 0);
        curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
		curl_setopt($ch, CURLOPT_POST, 1); 
		curl_setopt($ch, CURLOPT_POSTFIELDS, $xml);
		curl_setopt($ch, CURLOPT_SSLCERT, $key);
		curl_setopt($ch, CURLOPT_SSLVERSION, 2);
		
		if(isset($_SERVER['windir'])){
			curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 0);
			curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, 0);
		}

		if($this->debug){
			curl_setopt($ch, CURLOPT_VERBOSE, 1);
		}

		#  use curl to send the xml SSL string
		$result = curl_exec($ch);
		$info = curl_getinfo($ch);
		curl_close($ch);

	    # no response
		if (strlen($result) < 2){
			if($this->debug){
				$_Common->debugPrint($result,"curl response");
				$_Common->debugPrint($info,"curl response info");
				//exit;
			}
			return $this->decodeXML("<r_approved>FAILURE</r_approved><r_error>Could not connect to linkpoint.</r_error>"); 
		}
		else{
			if($this->debug){
				$respArray = $this->decodeXML($result);
				$_Common->debugPrint($respArray,"LinkPoint Response");
				//exit;
			}
			return $this->decodeXML($result);	
		}


	}
	
	// --------------------------------------------------------------
	// Parses the result xml into an array
	// --------------------------------------------------------------
	function decodeXML($xmlstg){
		preg_match_all ("/<(.*?)>(.*?)\</", $xmlstg, $out, PREG_SET_ORDER);
		$n = 0;
		while (isset($out[$n])){
			$retarr[$out[$n][1]] = strip_tags($out[$n][0]);
			$n++; 
		}
		return $retarr;
	}

	// XML Entity Mandatory Escape Characters
	function xmlentities($string) {
		return str_replace ( array ( '&', '"', "'", '<', '>' ), array ( '&amp;' , '&quot;', '&apos;' , '&lt;' , '&gt;' ), $string );
	} 
	
	// --------------------------------------------------------------
	// debug display of the xml
	// --------------------------------------------------------------
    function showXML($xml){
        $xml = preg_replace("/</","&lt;",$xml);
        $xml = preg_replace("/>/","&gt;",$xml);
        print "<pre>$xml</pre>";
    }

	// --------------------------------------------------------------
	// Loads the default values into the database when uploaded
	// --------------------------------------------------------------
	function loadDefaults(){

		global $_DB;

		$count = $_DB->getCount('payment_gateways', "WHERE LOWER(gateway_name) = 'linkpoint'");

		if($count == 0){

			$sql = "INSERT INTO `payment_gateways` ( `gateway_name`, `active`, `debug`, `related_payment_form` ) 
											VALUES ( 'linkpoint', 'false', 'false', 'credit_card.html')";

			$_DB->execute($sql);
			$pgid = $_DB->getInsertID('payment_gateways','pgid');
			
			if(!empty($pgid)){

				$details['store_number'] = 'your linkpoint store number';
				$details['order_type'] = 'SALE or PREAUTH';
				$details['transaction_type'] = 'LIVE';
				$details['debug'] = 'false';

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