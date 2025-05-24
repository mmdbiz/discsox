<?php

class Paypal{
	
	public bool $debug = false;
	public array $cfg = [];
	public array $params = [];
	public array $results = [];
	public $declineMessage = null;
	
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
		
		global $_CF;
		global $_Common;
		global $_DB;
		global $_Template;
		global $_Totals;

		if(isset($_REQUEST['shipping_method'])){
			$_SESSION['shipping_method'] = $_REQUEST['shipping_method'];
		}

		$this->loadCfg();

		$this->checkFields();

		$this->loadParams();
		
		if($this->cfg['fields']['send_aggregate_amount'] == "true" || $_Totals['discount'] > 0){
			$this->params['item_name'] = $_CF['basics']['store_name'] . " Order";
			$this->params['amount'] = $_Totals['grandtotal'];
			$this->params['cmd'] = "_xclick";
		}
		else{
			$this->params['tax_cart'] = $_Totals['salestax'];
			$this->loadCart();
		}
		
		$_SESSION['payment_gross'] = $_Totals['grandtotal'];
		
		$vars['hidden_fields'] = $this->params;
		$vars['post_url'] = $this->cfg['fields']['post_url'];

		if(isset($this->cfg['status']['send_preauth_email'])){
			if($this->cfg['status']['send_preauth_email'] == 'true'){
				$this->sendPreAuthEmail();
			}
		}

		// save the order for updating later and
		// also check it to see if it was resubmit
		$id = $this->params['custom'];		
		if(!empty($_SESSION['reference_number'])){
			$id = $_SESSION['reference_number'];
		}
		$isSaved = $_DB->getRecord("SELECT `order_number` FROM `orders` WHERE reference_number = '$id'");
		$invoiceNumber = null;
		if(count($isSaved) == 0){
			global $processClass;
			$_SESSION['reference_number'] = $this->params['custom'];
			$processClass->updateBillingAddress();
			$processClass->updateAddressBook();
			$processClass->saveOrder();
			$processClass->createLog();
			// save process vars for later			
			foreach(get_object_vars($processClass) as $name=>$value){
				$_SESSION[$name] = $value;
			}
			$_SESSION['reference_number'] = $this->params['custom'];
			$invoiceNumber = $processClass->order_number;
			$_SESSION['invoice_number'] = $invoiceNumber;
		}
		else{
			$vars['hidden_fields']['custom'] = $id;
			$invoiceNumber = $isSaved['order_number'];
		}
		$vars['hidden_fields']['invoice'] = $invoiceNumber;

		if($this->debug){
			$_Common->debugPrint($_Totals,"Totals");
			$_Common->debugPrint($this->cfg,"Paypal Config settings");
			$_Common->debugPrint($vars['hidden_fields'],"Paypal post parameters");
		}

		$submitPage = "templates/submit.html";
		if(!file_exists($submitPage) && file_exists("templates/submit.php")){
			$submitPage = "templates/submit.php";
		}
		
		print $_Template->Open($submitPage,$vars,true);
		exit;

	}

	// --------------------------------------------------------------
	function processResult(){

		global $_Common;
		global $_DB;

		$this->loadCfg();

		// parse result
		$result = $this->getResult();
		
		if($result){
			// check to see if we already saved the order
			if(isset($_SESSION['reference_number'])){
				$refnum = $_SESSION['reference_number'];
			}
			elseif(isset($_REQUEST['custom'])){
				$refnum = $_REQUEST['custom'];
			}
			elseif(isset($_REQUEST['cm'])){
				$refnum = $_REQUEST['cm'];
			}
			if($_DB->getCount('orders', "WHERE reference_number = '$refnum'") > 0){
				// if complete, set paid to true.
				if($this->results['transaction_status'] == "Completed"){
					$_DB->execute("UPDATE orders SET `paid` = 'true', `transaction_status` = 'Completed' WHERE reference_number = '$refnum'");
				}
			}
		}
		return $result;
	}

	// --------------------------------------------------------------
	function loadCfg(){
		
		include_once("cp/include/gateway.inc");
		$gateway = new Gateway();
		$this->cfg = $gateway->readConfig('paypal');

		if(count($this->cfg) == 0){
			$_Common->printErrorMessage("Payment Module Error",
										"Could not read configuration information for paypal payment module");
		}
		
		if($this->cfg['status']['debug'] == 'true'){
			$this->debug = true;	
		}	
	}

	// --------------------------------------------------------------
	function loadParams(){
		
		global $_CF;
		global $_Common;
		global $_DB;
		global $_Totals;
		

		// form fields		
		$this->params['company'] = 'billaddress_company';
		$this->params['first_name'] = 'billaddress_firstname';
		$this->params['last_name'] = 'billaddress_lastname';
		$this->params['address1'] = 'billaddress_addr1';
		$this->params['address2'] = 'billaddress_addr2';
		$this->params['city'] = 'billaddress_city';
		$this->params['state'] = 'billaddress_state';
		$this->params['zip'] = 'billaddress_postalcode';
		$this->params['country_code'] = 'billaddress_country';
		$this->params['night_phone_a'] = 'billaddress_areacode';
		$this->params['email'] = 'billaddress_email';

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

		$this->params['night_phone_b'] = substr(trim($_REQUEST['billaddress_phone']),0,3);
		$this->params['night_phone_c'] = substr(trim($_REQUEST['billaddress_phone']),-4);

		$cart_id = session_id();

		$this->params['cancel_return'] = $_CF['basics']['web_site_url'];
		$this->params['shopping_url'] = $_CF['basics']['web_site_url'];
		
		if($this->cfg['status']['use_ipn'] == 'true'){
			$this->params['notify_url'] = $_CF['basics']['web_site_url'] . "/paypal.notify.php?paypal=1&cart_id=$cart_id";
		}

		$this->params['return'] = $_CF['basics']['web_site_url'] . "/process.paypal.php?cart_id=$cart_id";		
		if(!empty($_CF['basics']['secure_url'])){
			$this->params['return'] = $_CF['basics']['secure_url'] . "/process.paypal.php?cart_id=$cart_id";
		}

		// PayPal Return method. Needs to be 2 = post
		$this->params['rm'] = 2;

		$this->params['invoice'] = $this->next_order_number;

		// stores the IPN info in the database
		// and creates the custom param.
		$this->createTimestamp();

		// merge all fields
		$this->params = array_merge($this->params,$this->cfg['fields']);
		
		// some cfg things to remove from the posted fields
		unset($this->params['allow_pending_payments']);
		unset($this->params['valid_IPN_result']);
		unset($this->params['send_aggregate_amount']);
		unset($this->params['send_preauth_email']);
		unset($this->params['display_item_number_in_paypal_cart']);
		unset($this->params['post_url']);

		
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
	function loadCart(){
		
		global $cart;
		global $_CART;
		global $_Common;
		global $_Totals;

//		$_Common->debugPrint($_CART);
//		exit;
		
		if(empty($_CART) && !empty($cart->items)){
			$_CART = $cart->items;	
		}

		$i = 1;
		foreach($_CART as $j=>$fields){

			$fields['quantity'] = intval($fields['quantity']);
			$this->params["item_number_$i"] = $fields['sku'];

			$price = $fields['price'];
			if(strstr($fields['price'],':')){
				$price = $_Common->calculateQuantityPrice($fields['price'],$fields['quantity']);
			}

			$this->params["item_name_$i"] = $fields['name'] . " ($price)";
			$this->params["quantity_$i"] = $fields['quantity'];
			$this->params["amount_$i"] = $_Common->format_price($fields['line_total'] / $fields['quantity']);

			if($i == 1){
				$this->params["shipping_$i"] = $_Totals['shipping'];

				if(isset($_Totals['insurance']) && $_Totals['insurance'] > 0){
					$this->params["shipping_$i"] = $_Common->format_price($_Totals['shipping'] + $_Totals['insurance']);
				}

				$this->params["shipping2_$i"] = "0.00";
				$this->params["handling_$i"] = "0.00";
			}
			
			if(isset($fields['options']) && count($fields['options']) > 0){
				
				$optionCount = 0;
				foreach($fields['options'] as $k=>$optionFields){
					
					$optionType = $optionFields['type'];
    				$name = $optionFields['name'];
					$text = $optionFields['value'];
					$price = $optionFields['price'];
					if(trim($name) == ""){
						$name = ucwords($optionType);	
					}
					if(strtolower($name) == "option"){
						$name .= " " . ($k + 1);
					}
					$strOption = "$name: $text";
					if($price > 0){
						$strOption .= " ($price)";
					}
					if($optionCount < 2){
						$pp_optionName = "on" . $optionCount . "_" . $i;
						$pp_optionVal = "os" . $optionCount . "_" . $i;
						$this->params[$pp_optionName] = $name;
						if($price > 0){
							$this->params[$pp_optionVal] = "$text ($price)";
						}
						else{
							$this->params[$pp_optionVal] = $text;
						}
						$optionCount++;
					}
					else{
						$this->params["item_name_$i"] .= ", $strOption";										
					}
				}
			}
			$this->params["amount_$i"] = $_Common->format_price($this->params["amount_$i"]);
			$i++;
		}
	}

	// --------------------------------------------------------------
	// Used to store results of IPN. For added security, uses a
	// custom id instead of passing the real session id.
	// --------------------------------------------------------------
	function createTimestamp(){
		
		global $_DB;
		global $_Totals;
		
		$sessid = session_id();
		
		$sql = "SELECT ppid FROM paypal WHERE sid = '$sessid'";
		$data = array();
		$data = $_DB->getRecords($sql);
		
		if(count($data) > 0){
			srand(time());
			$sequence = rand(1, 1000);	
			$user = $this->params['email'];
			$amount = $_Totals['grandtotal'];
			$tstamp = time();	
			$ppid = md5($sequence . $tstamp . $amount . $user);
			$this->params['custom'] = $ppid;
			$sql = "UPDATE paypal SET ppid = '$ppid', payment_gross = '$amount' WHERE sid = '$sessid'";
			$_DB->execute($sql);
		}
		else{
			srand(time());
			$sequence = rand(1, 1000);	
			$user = $this->params['email'];
			$amount = $_Totals['grandtotal'];
			$tstamp = time();	
			$ppid = md5($sequence . $tstamp . $amount . $user);
			$this->params['custom'] = $ppid;
			$sql = "INSERT INTO paypal (ppid,sid,payment_gross) VALUES ('$ppid','$sessid','$amount')";
			$_DB->execute($sql);
		}
		// do some cleanup
		$sql = "SELECT sid FROM paypal LEFT JOIN sessions ON paypal.sid = sessions.ses_id WHERE sessions.ses_id IS NULL";
		$rs = $_DB->execute($sql);
		if($_DB->numrows($rs) > 0){
			$ids = array();
			while($row = $_DB->fetchrow($rs, "ASSOC")){
				$ids[] = $row['sid'];
			}
			if(count($ids) > 0){
				$idList = "'" . join("','",$ids) . "'";
				$dSql = "DELETE FROM paypal WHERE sid IN ($idList)";
				$rs = $_DB->execute($dSql);
				$_DB->execute("OPTIMIZE TABLE paypal");
			}
		}
	} 


	// --------------------------------------------------------------
	function getResult(){

		global $_Common;
		global $_DB;

		$result = NULL;

		$sessid = session_id();

		if(isset($this->cfg['status']['validate_results']) && $this->cfg['status']['validate_results'] == 'false'){
			$this->results['transaction_status'] = "Pending Order Review";
			return true;
		}

		if($this->cfg['status']['use_ipn'] == 'true'){
			// give paypal a chance to return and then check the database for status
			sleep(10);
			$sql = "SELECT * FROM paypal WHERE sid = '$sessid' LIMIT 1";
			$data = array();
			$data = $_DB->getRecord($sql);

			if($this->debug){
				$_Common->debugPrint($data,"Paypal IPN Result");
			}
			
			if(isset($data['payment_status'])){
				
				if(isset($data['memo']) && trim($data['memo']) != ""){
					$this->results['memo'] = trim($data['memo']);
				}
				
				if(trim($data['payment_status']) == $this->cfg['fields']['valid_IPN_result']){
					$this->results['transaction_status'] = "Completed";
					return true;
				}
				elseif(trim($data['payment_status']) == "Pending" && $this->cfg['fields']['allow_pending_payments']){
					$this->results['transaction_status'] = "Pending";
					return true;
				}
				elseif(trim($data['payment_status']) == ""){
					$message = "Your payment status was not returned from PayPal. ";
					$message .= "Please contact us for more details.";
					$this->results['transaction_status'] = "NOT returned";
					$this->results['decline_message'] = $message;
					return false;
				}
				else{
					$status = trim($data['payment_status']);
					$message = "Your payment status was returned from PayPal as \"$status\". ";
					$message .= "Please contact us for more details.";
					$this->results['transaction_status'] = $status;
					$this->results['decline_message'] = $message;
					return false;
				}
			}
			else{
				$message = "Your payment status was not returned from PayPal. ";
				$message .= "Please contact us for more details.";
				$this->results['decline_message'] = $message;
				return false;
			}
		}
		else{
			if($this->debug){
				$_Common->debugPrint($_REQUEST,"Paypal Response");
			}

			// Result could be one of these:
			
			// [payment_gross] => 0.10
			// [custom] => 660e192447d369597a0c4e23a027213f
			// [payment_status] => Completed
			// [invoice] => 46165441

			// [st] => Completed
			// [amt] => 0.10
			// [cc] => USD
			// [cm] => bc963efeeb2b11fc2353ae0bbd991a5c
			
			$status = NULL;
			if(!empty($_REQUEST['payment_status'])){
				$status = $_REQUEST['payment_status'];
			}
			elseif(!empty($_REQUEST['st'])){
				$status = $_REQUEST['st'];
			}
			
			$amt = 0;
			if(!empty($_REQUEST['payment_gross'])){
				$amt = $_REQUEST['payment_gross'];
			}
			elseif(!empty($_REQUEST['amt'])){
				$amt = $_REQUEST['amt'];
			}
			
			if($status){

				//$_Common->debugPrint($status,"Status");
				$this->results['transaction_status'] = $status;
			
				// check the returned form fields
				if(!empty($_SESSION['reference_number'])){
					//$_Common->debugPrint($_SESSION['reference_number'],"Refnum");
					$refnum = $_SESSION['reference_number'];
					$data = $_DB->getRecord("SELECT `grandtotal` FROM `orders` WHERE `reference_number` = '$refnum'");
					if(count($data) > 0){
						$gt = 0;
						if(!empty($data['grandtotal'])){
							$gt = $data['grandtotal'];
						}
						// check the total
						if($gt == $amt){
							return true;
						}
						else{
							$this->results['transaction_status'] = "Returned PayPal payment amount ($amt) does not match the grand_total ($gt) in the order";
							return false;	
						}
					}
					$this->results['transaction_status'] = "Unable to locate matching order in the system";
					return false;
				}
				$this->results['transaction_status'] = "Unable to locate matching order in the system";
				return false;
			}
			else{
				$this->results['transaction_status'] = "Status NOT Returned";
				$message = "Your payment status was not returned from PayPal. ";
				$message .= "Please contact us for more details.";
				$this->results['decline_message'] = $message;
				return false;
			}
		}
	}
	// --------------------------------------------------------------
	function sendPreAuthEmail(){
		
		global $_CART, $_CF, $_Common, $_Registry, $_Totals;
		
		$email = $_Registry->loadClass("email");
		
		$body = "\n\nPAYPAL Parameters:\n";
		foreach($this->params as $key=>$val){
			if(strstr($key,'url') || strstr($key,'return')){
				continue;	
			}
			elseif(strstr($key,'night_phone_a')){
				$custPhone = '(' . $val . ') ';
				continue;	
			}
			elseif(strstr($key,'night_phone_b')){
				$custPhone .= $val . '-';
				continue;	
			}
			elseif(strstr($key,'night_phone_c')){
				$custPhone .= $val;
				$body .= "phone: = $custPhone\n";
				continue;	
			}
			elseif(strstr($key,'invoice')){
				$orderNumber = $val;
				//continue;	
			}
			$body .= "$key = $val\n";
		}
		
		$body .= "\n\nItems in CART:\n\n";
		foreach($_CART as $j=>$fields){
			$qty = intval($fields['quantity']);
			$sku = $fields['sku'];
			$price = $fields['price'];
			if(strstr($fields['price'],':')){
				$price = $_Common->calculateQuantityPrice($fields['price'],$fields['quantity']);
			}
			$name = $fields['name'];
			$total = $_Common->format_price($fields['line_total'] / $fields['quantity']);

			$body .= "SKU: $sku\n";
			$body .= "Quantity: $qty\n";
			$body .= "Name: $name\n";
			$body .= "Price: $price\n";

			if(isset($fields['options']) && count($fields['options']) > 0){
				foreach($fields['options'] as $k=>$optionFields){
    				$name = $optionFields['name'];
					$text = $optionFields['value'];
					$price = $optionFields['price'];
					$strOption = "$name: $text";
					if($price > 0){
						$strOption .= " ($price)";
					}
					$body .= "$strOption\n";
				}
			}
			$body .= "Line Total: $total\n\n";
		}
		$body .= "\n\nTOTALS:\n";
		foreach($_Totals as $key=>$val){
			$body .= "$key = $val\n";
		}
		$shipMeth = $_SESSION['shipping_method'];
		$body .= "Ship Method: $shipMeth\n\n";
		
		//missing ship address - marcello		
		$body .= "\n\nSHIPPING ADDRESS:\n";
		if(!empty($_SESSION['shipsameasbilling']) && ($_SESSION['shipsameasbilling'] == "true") ) {	
			$body .= "SHIP and BILL ADDRESS are the SAME!!!\n";
		}
		if(!empty($_SESSION['shipaddress_companyname'])){
			$ShipAddComp = $_SESSION['shipaddress_companyname'];
			$body .= "Company Name: $ShipAddComp\n";
		}

		$ShipAddFirstLast = $_SESSION['shipaddress_firstname'] . " " . $_SESSION['shipaddress_lastname'];
		$body .= "Name: $ShipAddFirstLast\n";
		$ShipAdd1 = $_SESSION['shipaddress_addr1'];
		$body .= "Address Line 1: $ShipAdd1\n";
		if(!empty($_SESSION['shipaddress_addr2'])){
			$ShipAdd2 = $_SESSION['shipaddress_addr2'];
			$body .= "Address Line 2: $ShipAdd2\n";			 
		}
		$ShipAddCity = $_SESSION['shipaddress_city'];
		$body .= "City: $ShipAddCity\n";
		$ShipAddState = $_SESSION['shipaddress_state'];
		$body .= "State: $ShipAddState\n";
		$ShipAddZip = $_SESSION['shipaddress_postalcode'];
		$body .= "ZIP: $ShipAddZip\n";
		$ShipAddCountry = $_SESSION['shipaddress_country'];
		$body .= "Country: $ShipAddCountry\n";
		$ShipEmail = $_SESSION['shipaddress_email'];
		$body .= "Ship Email: $ShipEmail\n";
		$ShipPhone = $_SESSION['shipaddress_areacode'] . '-' . $_SESSION['shipaddress_phone'];
		$body .= "Ship Phone: $ShipPhone\n";

		//missing stuff - marcello		
		$body .= "\n\nMISCELLANEOUS:\n";
		//$Platform =  $_SESSION['DisplayDevice'];
		//$body .= "Platform: $Platform\n";
		$CustComm =  $_SESSION['order_comments'];
		$body .= "Customer comments: $CustComm\n";
		$NewsSubscr =  $_SESSION['no_subscription'];
		$body .= "NewsLine Subscription: $NewsSubscr\n";
		$gdprAgree = $_SESSION['gdpr_agree'];
		$body .= "Agree to GDPR: $gdprAgree\n";
		$SeRef =  $_SESSION['se_engine_ref'];
		$body .= "Search Engine Referral: $SeRef\n";
		$KeyWords =  $_SESSION['se_key_words'];
		$body .= "Key Words used: $KeyWords\n";
		$OthRef =  $_SESSION['other_ref'];
		$body .= "Other Referral: $OthRef\n";
		$IPAddr =  $_SESSION['ip_address'];
		$body .= "IP Address: $IPAddr\n";
		if(isset($_COOKIE['affiliateID'])) {
		  $AffID =  $_COOKIE['affiliateID'];
		  $body .= "Affiliate ID: $AffID\n";
		}
		
		$cust = $_SESSION['billaddress_email'];
		$to = $_CF['email']['store_email_address'];
		$from = $_CF['email']['store_email_address'];
		$subject = "PayPal Preauth Email for $cust Order Number: $orderNumber";
		$html = "<html><body><pre>$body</pre></body></html>";
		$text = $body;
		$email->send($from, $to, $subject, $html, $text);
	}



	// --------------------------------------------------------------
	function sendEmails(){

		global $_CART;
		global $_CF;
		global $_Common;
		global $_DB;
		global $_Registry;
		global $_Template;
		global $_Totals;
		
		if(!$_CF['email']['send_email']){
			//print "Don't send email";
			return true;	
		}
		else{

			// get totals from the order
			if(!empty($_SESSION['reference_number'])){
				$id = $_SESSION['reference_number'];
				
    		if($this->debug){
    			$_Common->debugPrint($id,"reference number");	
    		}
				$order = $_DB->getRecord("SELECT * FROM `orders` WHERE reference_number = '$id'");
				if(count($order) > 0){
					$totalFields = array('subtotal','discount','shipping','insurance','handling','gst','pst','hst','salestax','grandtotal');
					foreach($totalFields as $k=>$f){
						if(!empty($order[$f])){
							$_Totals[$f] = $order[$f];
						}
						if(isset($order[$f])){
							unset($order[$f]);	
						}
					}
					foreach($order as $k=>$v){
						if(substr($k,-2) != 'id'){
							$_SESSION[$k] = $v;	
						}
					}
				}
			}
			
			// preset the options for the text emails
			foreach($_CART as $i=>$fields){

				$optionStr = "";
				$_CART[$i]['txtname'] = $fields['name'];
				
				if(!empty($fields['options'])){
					foreach($fields['options'] as $j=>$option){
						$optionStr .= ", ";
						if($option['name'] != "" && $option['name'] == "Option"){
							$optionStr .= "Option " . ($j+1) . ":";
						}
						elseif($option['name'] != "" && $option['name'] != "Option"){
							$optionStr .= $option['name'] . ":";	
						}
						$optionStr .= " " . $option['value'];
						
						if($option['price'] != "0"){
							if($option['type'] == "option"){
								$optionStr .= " ($" . $option['price'] . ")";
							}
							elseif($option['type'] == "setup"){
								$optionStr .= " (Setup Charge: " . $option['price'] . ")";
							}
							else{
								$optionStr .= " " . $option['price'];
							}
						}
					}
				}
				$_CART[$i]['txtname'] .= $optionStr;
			}			

			$encrypt = $_CF['encryption']['encrypt_orders'];
			
			$email = $_Registry->loadClass('email');
			$storeName = $_CF['basics']['store_name'];			
			$storeEmailAddress = $_CF['email']['store_email_address'];

			$vars = array("storeName" => $storeName,
						  "downloadLinks" => $this->downloadLinks,
						  "order_number" => $this->order_number,
						  "order_date"	=> $this->order_date,
						  "customer_number" => $this->customer_number,
						  "selectedCarrier" => $this->selectedCarrier,
						  "txResults" => $this->results);

			if($_CF['email']['send_store_email']){
				
				// OPEN THE STORE EMAIL AND GET TEXT HERE
				$storeEmail = $_Template->open("templates/store.email.html",$vars,false);
				@list($storeHtml,$storeTxt) = explode("<text>",$storeEmail);
				$subject = $_CF['email']['store_email_subject_line'];
				$subject = str_replace("%orderNumber%",$this->order_number,$subject);

				if($encrypt){
					global $_CR;
					$crypt = $_Registry->loadClass('crypt');
					$storeHtml = "<html><body><pre>$_CR" . $crypt->encrypt($storeHtml) . "$_CR</pre></body></html>";
					$storeTxt = $crypt->encrypt($storeTxt);
				}
				
    		if($this->debug){
    			$_Common->debugPrint($storeEmailAddress,"store Email Address");	
    			$_Common->debugPrint($subject,"Subject");	
    			$_Common->debugPrint($storeHtml,"store Html");	
    			$_Common->debugPrint($storeTxt,"storeTxt");	
    		}
				// Parameters: $email->send(<from email>, <to email>, <subject>, <html version>, <text version>);
				$email->send($storeEmailAddress, $storeEmailAddress, $subject, $storeHtml, $storeTxt); //original version
				//$email->send($storeEmailAddress, "marcello@mmdesign.com", $subject, $storeHtml, $storeTxt);		//marcello fixed in email.inc		
			}
			if($_CF['email']['send_customer_email']){
				
				$subject = $_CF['email']['customer_email_subject_line'];
				$subject = str_replace("%orderNumber%",$this->order_number,$subject);
				
				$custEmailAddress = $_SESSION['billaddress_email'];
				//$_Common->debugPrint($custEmailAddress,"custEmailAddress");
				$check = array();
				if(preg_match("/^[_\.0-9a-z-]+@([0-9a-z][-0-9a-z\.]+)\.([a-z]{2,3}$)/i",$custEmailAddress,$check)){
					// OPEN THE CUSTOMER EMAIL AND GET TEXT HERE
					$customerEmail = $_Template->open("templates/customer.email.html",$vars,false);
					@list($customerHtml,$customerTxt) = explode("<text>",$customerEmail);
      		if($this->debug){
      			$_Common->debugPrint($storeEmailAddress,"store Email Address");	
      			$_Common->debugPrint($custEmailAddress,"store Email Address");	
      			$_Common->debugPrint($subject,"Subject");	
      			$_Common->debugPrint($customerHtml,"store Html");	
      			$_Common->debugPrint($customerTxt,"storeTxt");	
      		}
					$email->send($storeEmailAddress, $custEmailAddress, $subject, $customerHtml, $customerTxt);
				}				
			}
		}
		
		if($this->debug){
			$_Common->debugPrint("Done sending mail");	
		}
		
	}

	
	// --------------------------------------------------------------
	// Loads the default values into the database when uploaded
	// --------------------------------------------------------------
	function loadDefaults(){

		global $_DB;

		$count = $_DB->getCount('payment_gateways', "WHERE LOWER(gateway_name) = 'paypal'");

		if($count == 0){

			$sql = "INSERT INTO `payment_gateways` ( `gateway_name`, `active`, `debug`, `related_payment_form` ) 
											VALUES ( 'paypal', 'false', 'false', 'paypal.html')";

			$_DB->execute($sql);
			$pgid = $_DB->getInsertID('payment_gateways','pgid');
			
			if(!empty($pgid)){

				$details['business'] = 'your paypal email address';
				$details['post_url'] = 'https://www.paypal.com/cgi-bin/webscr';
				$details['currency_code'] = 'USD';
				$details['send_aggregate_amount'] = 'false';
				$details['allow_pending_payments'] = 'true';
				$details['cmd'] = '_cart';
				$details['redirect_cmd'] = '_xclick';
				$details['upload'] = '1';
				$details['no_intl'] = '1';
				$details['valid_IPN_result'] = 'Completed';
				
				$counter = 0;
				foreach($details as $key=>$value){
					$sql = "INSERT INTO `payment_gateway_details` (`pgid`,`section`,`sequence`,`key`,`value`)
														   VALUES ('$pgid','fields','$counter','$key','$value')";
					$_DB->execute($sql);
					$counter++;
				}
				
				// status fields
				$sql = "INSERT INTO `payment_gateway_details` (`pgid`,`section`,`sequence`,`key`,`value`)
													   VALUES ('$pgid','status','$counter','send_preauth_email','false')";
				$sql = "INSERT INTO `payment_gateway_details` (`pgid`,`section`,`sequence`,`key`,`value`)
													   VALUES ('$pgid','status','$counter','use_ipn','false')";
				$_DB->execute($sql);
			}
		}
	}

	// --------------------------------------------------------------
	// Updates to cfg fields
	// --------------------------------------------------------------
	function checkFields(){
		
		global $_DB;
		
		if(!isset($this->cfg['status']['send_preauth_email'])){
			// add the variable if missing
			$data = $_DB->getRecord("SELECT `pgid` FROM `payment_gateways` WHERE LOWER(gateway_name) = 'paypal'");
			if(isset($data['pgid'])){
				$pgid = $data['pgid'];
				$sql = "INSERT INTO `payment_gateway_details` (`pgid`,`section`,`sequence`,`key`,`value`)
													   VALUES ('$pgid','status','12','send_preauth_email','false')";
				$_DB->execute($sql);
			}
			$this->cfg['status']['send_preauth_email'] = 'false';
		}
		
		if(!isset($this->cfg['status']['use_ipn'])){
			// add the variable if missing
			$data = $_DB->getRecord("SELECT `pgid` FROM `payment_gateways` WHERE LOWER(gateway_name) = 'paypal'");
			if(isset($data['pgid'])){
				$pgid = $data['pgid'];
				$sql = "INSERT INTO `payment_gateway_details` (`pgid`,`section`,`sequence`,`key`,`value`)
													   VALUES ('$pgid','status','14','use_ipn','false')";
				$_DB->execute($sql);
			}
			$this->cfg['status']['use_ipn'] = 'false';
		}

		if(!isset($this->cfg['status']['validate_results'])){
			// add the variable if missing
			$data = $_DB->getRecord("SELECT `pgid` FROM `payment_gateways` WHERE LOWER(gateway_name) = 'paypal'");
			if(isset($data['pgid'])){
				$pgid = $data['pgid'];
				$sql = "INSERT INTO `payment_gateway_details` (`pgid`,`section`,`sequence`,`key`,`value`)
													   VALUES ('$pgid','status','15','validate_results','true')";
				$_DB->execute($sql);
			}
			$this->cfg['status']['validate_results'] = 'true';
		}
		
		if(isset($this->cfg['status']['save_to_database_when_submitted'])){
			// Remove it. We don't want them to be able to change it.
			$sql = "DELETE FROM `payment_gateway_details`
					WHERE `section` = 'status' AND `key` = 'save_to_database_when_submitted'";
			$_DB->execute($sql);
			unset($this->cfg['status']['save_to_database_when_submitted']);
		}
	}

}

?>