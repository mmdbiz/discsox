<?php
class Process{

    public bool $debug = false;
    public bool $showTables = false;
    
    public string $browser = "UNDEFINED";
    public array $browsers = ["OPERA","MSIE","NETSCAPE","FIREFOX","SAFARI","KONQUEROR","MOZILLA"];
	public $cid = null;
	public $csid = null;
	public int $customer_number = 1000;
	public $next_customer_number = null;
	public array $downloadLinks = [];
	public string $defaultTransactionStatus = "Pending";
	public $error = null;
    public array $missingFields = [];
    public string $order_date = "00/00/0000";
    public int $order_number = 1000;
    public bool $paymentResultOk = false;
    public $processorPage = null;
    public $selectedCarrier = null;
	public bool $showDeclinePage = false;
    public bool $showSubmitPage = false;
    public string $transaction_date = "0000-00-00";
    public string $transaction_time = "";
    public array $txResults = [];
    public $user_host = null;
	
	// ----------------------------------------------------
	function __construct(){
		
		global $cart;
		global $_CART;
		global $_CF;
		global $_Common;
		global $_Registry;
		global $_Totals;

		// reload the cart
		$cart->LoadCart();
		
		$cart->updateLastPage("process.php");

		if(count($_CART) == 0){
			return;	
		}

		if(isset($_Totals['totalWeight']) && $_Totals['totalWeight'] == 0){
			if($_CF['shipping']['hide_shipping_on_zero_weight']){
				$_CF['shipping']['require_shipping'] = false;
			}
		}

		// Save incoming data to session so we don't need hidden fields
		$_SESSION = array_merge($_SESSION,$_REQUEST);

		// double check required fields in case user has javascript
		// off or there was an error in it.
		if(isset($_REQUEST['required']) && count($_REQUEST['required']) > 0){
			$this->CheckRequiredFields($_REQUEST['required']);
			if(count($this->missingFields) > 0){
				return;	
			}
		}

		// re-load calcs
		$calcs = $_Registry->LoadClass('Calcs');
		$this->selectedCarrier = $calcs->carrier;

		// add browser stat
		$this->getBrowser();

		// Get next available order number
		$this->getNextOrderNumber();

		// get existing customer number or next available number
		$this->getNextCustomerNumber();

		// check payment if applicable
		$this->checkPayment();

		// Check/Update/Insert billing address
		$this->updateBillingAddress();

		if(!$this->cid){
			if(!empty($_SESSION['cid'])){
				$this->cid = $_SESSION['cid'];
			}
			else{
				$this->error = "Customer id key missing. Cannot add shipping record.";
				return;
			}
		}

		if($_CF['shipping']['require_shipping']){
			// Save to address book (update OR insert)
			$this->updateAddressBook();
		}

		if(!$this->csid){
			if(!empty($_SESSION['cid'])){
				$this->cid = $_SESSION['cid'];
			}
			else{
				$this->error = "Customer shipping id missing. Cannot save order.";
				return;
			}
		}

		// Save the order to the database
		$this->saveOrder();

		// Create a custom log of the order
		$this->createLog();

		// Create any download links
		if($_Registry->file_exists_incpath('downloads.inc')){
			$downloads = $_Registry->LoadClass('downloads');
			$emailAdrs = null;
			if(!empty($_SESSION['billaddress_email'])){
				$emailAdrs = $_SESSION['billaddress_email'];
			}
			$this->downloadLinks = $downloads->makeDownloadLinks($this->order_number,$emailAdrs);
			
			if($this->debug){
				$_Common->debugPrint($this->downloadLinks,"Make download links");	
			}
		}

		// Send emails
		$this->sendEmails();

		$this->removeCoupons();

		if($_Registry->file_exists_incpath("inventory.inc")){
			$inv = $_Registry->LoadClass("inventory");
			$inv->UpdateInventory();
		}
		
		// delete cart if required
		if($_CF['cart']['delete_cart_at_checkout']){
			$cart->deleteCart();
			if(!empty($_SESSION['cartTotals'])){
				unset($_SESSION['cartTotals']);
			}
			session_destroy();
		}
		
	}
	
	// --------------------------------------------------------------
	function CheckRequiredFields($fields){
		foreach($fields as $i=>$fldName){
			if(empty($_REQUEST[$fldName]) || strtolower($_REQUEST[$fldName]) == "invalid"){
				$fldText = str_replace("_"," ",$fldName);
				$this->missingFields[$fldName] = $fldText;
			}
		}
	}
	
	// --------------------------------------------------------------
	// gets basic info about the users browser.
	function getBrowser(){
		if(isset($_SERVER['HTTP_USER_AGENT'])){
			foreach($this->browsers as $i=>$parent){
				if(($s = strpos(strtoupper($_SERVER['HTTP_USER_AGENT']), $parent)) !== FALSE ){            
					$f = $s + strlen($parent);
					$version = substr($_SERVER['HTTP_USER_AGENT'], $f, 5);
					$version = preg_replace('/[^0-9,.]/','',$version);
					$this->browser = $parent . " " . $version;
					break; // first match wins
				}
			}
		}
		else{
			$this->browser = "unknown";
		}
		$_SESSION['browser'] = $this->browser;
	}

    // --------------------------------------------------------------	
    function getNextOrderNumber(){
		global $_DB;
        $result = array();
        $sql = "SELECT MAX(order_number) AS lastnumber FROM orders";
        $result = $_DB->getRecords($sql);
        if(isset($result[0]['lastnumber']) && $result[0]['lastnumber'] != ""){
            $this->order_number = $result[0]['lastnumber'] + 1;
            $oid = $this->order_number;
            $count = $_DB->getCount('orders', "WHERE `order_number` = '$oid'");
			if($count > 0){
				while($count > 0){
					$oid++;
					$count = $_DB->getCount('orders', "WHERE `order_number` = '$oid'");
					if($count == 0){
						break;	
					}	
				}
				$this->order_number = $oid;
			}
        }
        else{
			// get the highest order number entered
			$sql = "SELECT order_number FROM orders ORDER BY order_number DESC LIMIT 1";
			$record = $_DB->getRecord($sql);
			if(isset($record['order_number']) && $record['order_number'] != ""){
				$this->order_number = $record['order_number'] + 1;
			}
		}
        $_SESSION['next_order_number'] = $this->order_number;
    }
    
    // --------------------------------------------------------------	
    function getNextCustomerNumber(){
		
		global $_Common;
		global $_DB;

		if(!empty($_SESSION['cid'])){
			$cid = $_SESSION['cid'];
			if(strlen($cid) < 32){
				$cid = md5($cid);	
			}
			$sql = "SELECT customer_number FROM customers WHERE MD5(cid) = '$cid' LIMIT 1";
			$result = $_DB->getRecord($sql);
			if(!empty($result['customer_number'])){
				$this->next_customer_number = $result['customer_number'];
			}
		}
		else{
			$sql = "SELECT MAX(customer_number) AS lastnumber FROM customers";
			$result = $_DB->getRecord($sql);
			if(!empty($result['lastnumber'])){
				$this->next_customer_number = $result['lastnumber'] + 1;
			}
			else{
				// get the highest customer number entered
				$sql = "SELECT customer_number FROM customers ORDER BY customer_number DESC LIMIT 1";
				$record = $_DB->getRecord($sql);
				if(isset($record['customer_number']) && $record['customer_number'] != ""){
					$this->next_customer_number = $record['customer_number'] + 1;
				}	
			}
		}
		$_SESSION['next_customer_number'] = $this->next_customer_number;
    }
    
	// --------------------------------------------------------------
	function checkPayment(){

		global $_CF;
		global $_Common;
		global $_DB;
		global $_Registry;
		global $_Totals;
		
		// save the payment_method for return trips from
		// CC processors who use HTML posts
		if(!isset($_REQUEST['payment_method'])){
			if(empty($_SESSION['payment_method'])){
				return true;
			}
			else{
				$payMethod = $_SESSION['payment_method'];
			}
		}
		else{
			$payMethodString = trim(strtolower($_REQUEST['payment_method']));
			$payMethod = $_DB->escape_string($payMethodString);
			if($payMethod != ""){
				$_SESSION['payment_method'] = $payMethod;
			}
			else{
				return true;
			}			
		}

		// check to see if we have an active payment gateway associated
		// with this payment form. If so, we try to process the payment
		// through it. Otherwise return true and just save the order and
		// send the details in an encrypted email.
		
		$sql = "SELECT gateway_name FROM payment_gateways 
				WHERE LOWER(related_payment_form) = '$payMethod' AND active = 'true' LIMIT 1";
				
		$gateways = $_DB->getRecord($sql);
	
		if($this->debug){
			$_Common->debugPrint($payMethod,"Selected Payment Method");
			$_Common->debugPrint($gateways,"Payment Gateway");	
		}

		// Just return if no gateway found
		if(count($gateways) == 0){
			return true;
		}
		elseif($_Totals['grandtotal'] == 0){
			return true;
		}
		else{
			// make a copy of this class for paypal
			// if we want to save the order to the database.
			global $processClass;
			$processClass = $this;
			
			// try to process the payment
			$gatewayFile = $gateways['gateway_name'] . "inc";
			$payment = $_Registry->loadClass($gateways['gateway_name']);
			$payment->next_order_number = $this->order_number;
			$payment->next_customer_number = $this->next_customer_number;
			
			if(!$payment->processPayment()){
				$this->showDeclinePage($payment->declineMessage);
			}
			
			$this->txResults = $payment->results;
			
			if($this->debug){
				$_Common->debugPrint($this->txResults,"Transaction Results");
			}
			
			return true;
		}
	}

	// --------------------------------------------------------------
	function showDeclinePage($message){
		global $_Template;
		$vars['decline_message'] = $message;
		print $_Template->Open("templates/declined.html",$vars,true);
		exit;		
	}


    
	// --------------------------------------------------------------
	function updateBillingAddress(){

		global $_CF;
		global $_Common;
		global $_DB;

		$cid = null;
		if(!empty($_SESSION['cid']) && $_SESSION['cid'] != "0" && trim($_SESSION['cid']) != ""){
			$cid = $_SESSION['cid'];
			if(strlen($cid) < 32){
				$cid = md5($cid);	
			}
		}

		// get the unencrypted cid value for saving the remaining things to the database.
		if($cid){
			$sql = "SELECT cid,customer_number FROM customers WHERE MD5(cid) = '$cid' LIMIT 1";
			$RS = $_DB->execute($sql);
			$row = $_DB->fetchrow($RS,'ASSOC');
			$_DB->free_result($RS);
			if(count($row) > 0){
				$this->cid = $row['cid'];
				$this->customer_number = $row['customer_number'];
			}
		}

		// get the fields
		$billInfo = array();
		$fieldProperties = array();

		$RS = $_DB->execute("SHOW FIELDS FROM customers LIKE 'billaddress%'");
		while($row = $_DB->fetchrow($RS)){
			$fldName = strtolower($row[0]);
			$fieldProperties[$fldName] = $row;
			if(isset($_SESSION[$fldName])){
				$billInfo[$fldName] = trim($_SESSION[$fldName]);
			}
			else{
				$billInfo[$fldName] = "";
			}
		}
		$_DB->free_result($RS);

		// registration fields
		if(!empty($_SESSION['registration']['username'])){
			$billInfo['username'] = trim($_SESSION['registration']['username']);
			$fieldProperties['username'][0] = "username";
		}
		if(!empty($_SESSION['registration']['password'])){
			if($_CF['login']['encrypt_password']){
				$billInfo['password'] = md5(trim($_SESSION['registration']['password']));
			}
			else{
				$billInfo['password'] = trim($_SESSION['registration']['password']);
			}
			$fieldProperties['password'][0] = "password";
		}
		if(isset($_SESSION['email_list'])){
			$billInfo['email_list'] = trim($_SESSION['email_list']);
			$fieldProperties['email_list'][0] = "email_list";
		}

		if($this->debug){
			$_Common->debugPrint($billInfo,"Billing Address Info: $cid"); 
		}

		// customer logged in and selected update
		if($cid && !empty($_SESSION['billaddress_update']) && $_SESSION['billaddress_update'] == "true"){
			$updateFields = $_DB->makeUpdateFields($fieldProperties,"cid",$billInfo);
			$sql = "UPDATE customers SET $updateFields WHERE MD5(cid) = '$cid' LIMIT 1";
			if($this->debug){
				$_Common->debugPrint($sql);
			}
			$RS = $_DB->execute($sql);
			$_DB->free_result($RS);
			$_SESSION['billaddress_update'] = "false";
		}
		
		// not logged in, save as new
		if(!$cid){
			$fieldProperties['customer_number'][0] = "customer_number";
			$billInfo['customer_number'] = $this->next_customer_number;
			$fieldProperties['active_date'][0] = "active_date";
			$billInfo['active_date'] = date("Y-m-d");
			list($fields,$values) = $_DB->makeAddFields($fieldProperties,"cid",$billInfo);
			$sql = "INSERT INTO customers ($fields) VALUES ($values)";
			$RS = $_DB->execute($sql);
			if($this->debug){
				$_Common->debugPrint($sql,"customer insert");
			}
			$this->cid = $_DB->getInsertID('customers','cid');
			$_SESSION['cid'] = md5($this->cid);
			// Just in case they go back into the store
			$_SESSION['isRegistered'] = true;
		}
	}
	

	// --------------------------------------------------------------
	function updateAddressBook(){
		
		global $_Common;
		global $_DB;

		$shipInfo = array();
		$fieldProperties = array();
		
		$cid = $this->cid;
		$shipInfo['cid'] = $cid;
		$fieldProperties['cid'][0] = "cid";
		
		$csid = NULL;
		if(isset($_SESSION['csid']) && $_SESSION['csid'] != "0"){
			$csid = $_SESSION['csid'];
			$this->csid = $csid;
			$shipInfo['csid'] = $csid;
			$fieldProperties['csid'][0] = "csid";
		}

		$RS = $_DB->execute("SHOW FIELDS FROM customer_shipping LIKE 'shipaddress%'");
		$row = array();
		while($row = $_DB->fetchrow($RS)){
			$fldName = strtolower($row[0]);
			$fieldProperties[$fldName] = $row;
			if(isset($_SESSION[$fldName])){
				$shipInfo[$fldName] = $_SESSION[$fldName];
			}
			else{
				$shipInfo[$fldName] = "";
			}
		}
		$_DB->free_result($RS);

		if($this->debug){
			$_Common->debugPrint($shipInfo,"Shipping Address Fields");
		}

		// we have a csid. Check to see if save new or update is clicked.
		if($csid){
			if(isset($_SESSION['shipaddress_savenew']) && $_SESSION['shipaddress_savenew'] == "true"){
				list($fields,$values) = $_DB->makeAddFields($fieldProperties,"csid",$shipInfo);
				$sql = "INSERT INTO customer_shipping ($fields) VALUES ($values)";
				if($this->debug){
					$_Common->debugPrint($sql,"Save New Customer Shipping Address (existing customer)");
				}
				$RS = $_DB->execute($sql);
				$_SESSION['csid'] = $_DB->getInsertID('customer_shipping','csid');
				$this->csid = $_SESSION['csid'];
				$_DB->free_result($RS);
				$_SESSION['shipaddress_savenew'] = "false";
			}
			elseif(isset($_SESSION['shipaddress_update']) && $_SESSION['shipaddress_update'] == "true"){
				$uFields = $_DB->makeUpdateFields($fieldProperties,"csid",$shipInfo);
				$sql = "UPDATE customer_shipping SET $uFields WHERE cid = '$cid'";
				if($csid){
					$sql .= " AND csid = '$csid'";
				}
				if($this->debug){
					$_Common->debugPrint($sql,"Update Customer Shipping Address");
				}
				$RS = $_DB->execute($sql);
				$_DB->free_result($RS);
				$_SESSION['shipaddress_update'] = "false";
			}	
		}
		else{
			// must be a new customer
			$shipInfo['primary_address'] = true;
			$fieldProperties['primary_address'][0] = "primary_address";
			list($fields,$values) = $_DB->makeAddFields($fieldProperties,"csid",$shipInfo);
			$sql = "INSERT INTO customer_shipping ($fields) VALUES ($values)";
			if($this->debug){
				$_Common->debugPrint($sql,"Insert New Customer Shipping Address");
			}
			$RS = $_DB->execute($sql);
			$_SESSION['csid'] = $_DB->getInsertID('customer_shipping','csid');
			$this->csid = $_SESSION['csid'];
			$_DB->free_result($RS);	
		}
	}

	// --------------------------------------------------------------
	function saveOrder(){
		
		global $_DB;
		global $_Common;
		global $_CART;
		global $_CF;
		global $_Totals;

		$originalCid = $_SESSION['cid'];

        $orderFields = $_DB->getFieldProperties('orders');
        $orderDetailFields = $_DB->getFieldProperties('order_details');
        $orderOptionFields = $_DB->getFieldProperties('order_options');

		if(!isset($orderFields['insurance'])){
			$_DB->execute("ALTER TABLE `orders` ADD `insurance` FLOAT(10,2) NOT NULL DEFAULT '0.00' AFTER `shipping`");
			$orderFields = $_DB->getFieldProperties('orders');	
		}

        if($this->showTables){
           $_Common->debugPrint(array_keys($orderFields),"Order table fields:");
           $_Common->debugPrint(array_keys($orderDetailFields),"Order Detail table fields");
           $_Common->debugPrint(array_keys($orderOptionFields),"Order Options table fields");
        }

        if(!empty($_SESSION['affiliate'])){
			$_SESSION['affiliate'] = trim($_SESSION['affiliate']);
		}

		// merge form fields and totals into order fields
		$orderDataFields = array();
		$orderDataFields = array_merge($_SESSION,$_Totals);


		if(strlen($this->cid) == 32){
			$sql = "SELECT cid FROM customers WHERE MD5(cid) = '$this->cid'";
			$cust = $_DB->getRecord($sql);
			//$_Common->debugPrint($cust,"cust");
			if(count($cust) > 0){
				$this->cid = $cust['cid'];
			}			
		}

		// customer id's
		$orderDataFields['cid'] = $this->cid;
		$orderDataFields['csid'] = $this->csid;

        // Set date fields for database tables, receipt and emails
		$this->transaction_date = date("Y-m-d");
		$this->transaction_time = date("H:i:s");
		
		// sets the date format from CP/Basics
		$dateFormat = "m/d/Y";
		
		if(!empty($_CF['basics']['date_format'])){
			$dateFormat = $_CF['basics']['date_format'];
		}
        $this->order_date = date("$dateFormat, g:i a");

		// reset time using an offset
		if(defined('TIME_OFFSET') && TIME_OFFSET !== 0) {
			$timezone = 3600 * TIME_OFFSET;
			$this->order_date = date("$dateFormat, g:i a", time() + $timezone);
			$this->transaction_time = date("H:i:s", time() + $timezone);
		}

        $orderDataFields['order_date'] = $this->order_date;
        $orderDataFields['transaction_date'] = $this->transaction_date;
        $orderDataFields['transaction_time'] = $this->transaction_time;

        $this->user_host = getenv('REMOTE_HOST');
        if(trim($this->user_host) == ""){
            $this->user_host = getenv('REMOTE_ADDR');
        }

		$orderDataFields['user_host'] = $this->user_host;
		$orderDataFields['order_number'] = $this->order_number;
		$orderDataFields['browser'] = $this->browser;
		$orderDataFields['transaction_status'] = $this->defaultTransactionStatus;
		
		if(isset($_CF['basics']['save_credit_card_info_in_database']) && !$_CF['basics']['save_credit_card_info_in_database']){
			unset($orderDataFields['card_number']);
			unset($orderDataFields['cvv2']);
		}
		
		list($fields, $values) = $_DB->makeAddFields($orderFields, 'orid', $orderDataFields);
		$sql = "INSERT INTO orders ($fields) VALUES ($values)";
		if($this->debug){
			$_Common->debugPrint($sql,"Order Insert");	
		}
		$_DB->execute($sql);
		$orid = $_DB->getInsertID('orders','orid');

		// Load each line item into the table with options if available
		foreach($_CART as $cartid=>$itemFlds){
			
			$itemFlds['cid'] = $this->cid;
			$itemFlds['csid'] = $this->csid;
			$itemFlds['orid'] = $orid;
			$itemFlds['order_number'] = $this->order_number;
			
			if(!empty($itemFlds['page'])){
				$itemFlds['page'] =	basename($itemFlds['page']);
			}

			// save qty price for oreders report updates
			if(!empty($itemFlds['qty_price'])){
				$itemFlds['price'] = $itemFlds['qty_price'];
			}

			if(!empty($itemFlds['page'])){
				$pageFlds = parse_url($itemFlds['page']);
				if(isset($pageFlds['query'])){
					parse_str($pageFlds['query'], $vars);
					// category is often too long for field in db
					if(isset($vars['category'])){
						unset($vars['category']);	
					}
					$itemFlds['page'] = $pageFlds['path'] . "?";
					foreach($vars as $k=>$v){
						$itemFlds['page'] .= "$k=$v&";
					}
				}
				$itemFlds['page'] = chop($itemFlds['page']);
			}


			//$_Common->debugPrint($itemFlds,"Order Detail Insert Fields");


			// Details of the order including items
			list($fields, $values) = $_DB->makeAddFields($orderDetailFields, 'ordid', $itemFlds);
			$sql = "INSERT INTO order_details ($fields) VALUES ($values)";
			if($this->debug){
				$_Common->debugPrint($sql,"Order Detail Insert");	
			}		
			$_DB->execute($sql);
			$itemFlds['ordid'] = $_DB->getInsertID('order_details','ordid');

			if(isset($itemFlds['options']) && count($itemFlds['options']) > 0){
				foreach($itemFlds['options'] as $i=>$optionFlds){
					$optionFlds['ordid'] = $itemFlds['ordid'];
					list($fields, $values) = $_DB->makeAddFields($orderOptionFields, 'oroid', $optionFlds);
					$sql = "INSERT INTO order_options ($fields) VALUES ($values)";
					if($this->debug){
						$_Common->debugPrint($sql,"Order Detail Options Insert");	
					}		
					$_DB->execute($sql);
				}
			}
		}
		
		$_SESSION['cid'] = $originalCid;
		$this->cid = $originalCid;
		
		//$_Common->debugPrint($this->cid,"cid");
		//exit;
		
	}
	
	
	// --------------------------------------------------------------
	function sendEmails(){

		global $_CART;
		global $_CF;
		global $_Common;
		global $_Registry;
		global $_Template;
		global $_Totals;
		
		if(!$_CF['email']['send_email']){
			//print "Don't send email";
			return true;	
		}
		else{
			
			// preset the options for the text emails
			foreach($_CART as $i=>$fields){

				$optionStr = "";
				$_CART[$i]['txtname'] = $fields['name'];
				
				if(!empty($fields['options'])){
					foreach($fields['options'] as $j=>$option){
						$optionStr .= ", ";
						if($option['name'] != "" && $option['name'] == "Option"){
							$optionStr .= "Option " . ($j+1) . ":"; //fixed 3/24/21 marcello
						}
						elseif($option['name'] != "" && $option['name'] != "Option"){
							$optionStr .= $option['name'] . ":";	
						}
						$optionStr .= " " . $option['value'];
						
						if($option['price'] != "0"){
							if($option['type'] == "option"){
								//$optionStr .= " (" . $option['price'] . " each)";
								$optionStr .= " (" . $option['price'] . ")"; //marcello take "each" out of options
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
						  "txResults" => $this->txResults);

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
				$email->send($storeEmailAddress, $storeEmailAddress, $subject, $storeHtml, $storeTxt);				
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
					$email->send($storeEmailAddress, $custEmailAddress, $subject, $customerHtml, $customerTxt);
				}				
			}
		}
		
		if($this->debug){
			$_Common->debugPrint("Done sending mail");	
		}
		
	}
	
	// --------------------------------------------------------------
	// this is a empty function to allow for custom logging extensions.
	function createLog(){
		
		global $_CF,$_Registry;
		
		if($_Registry->file_exists_incpath("quickbooks.inc")){
			$qb = $_Registry->LoadClass("quickbooks");
			$qb->WriteLog($this->order_number);
		}
		
		if(!empty($_CF['order logs']['log_orders']) && $_CF['order logs']['log_orders']){
			if($_Registry->file_exists_incpath("logs.inc")){
				$logs = $_Registry->LoadClass("logs");
				$logs->Save(get_object_vars($this));
			}
		}
		
	}
	// --------------------------------------------------------------
	function removeCoupons(){
		
		global $_DB;
		
		if(isset($_SESSION['coupon'])){
			$coupon = trim($_SESSION['coupon']);
			
			$sql = "SELECT * FROM discounts WHERE `coupon` = '$coupon'";
			$data = $_DB->getRecord($sql);
			
			if(count($data) > 0 && $data['remove_after_use'] == 'true'){
				$did = $data['did'];
				$_DB->execute("DELETE FROM discounts WHERE `did` = '$did' LIMIT 1");
			}
		}
	}
}
?>









