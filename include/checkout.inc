<?php
// -----------------------------------------------------------------------
//         Class: Checkout
//       Purpose: This class controls the first step in the checkout functionality.
// Last Modified: March 08, 2006
//
// -----------------------------------------------------------------------
class Checkout{

    public bool $debug = false;
    public array $addressBook = [];
    public bool $haveCoupons = false;
    public array $paymentMethods = [];
    public bool $taxTable = false;
    public $taxTableField = null;
    public $countyJava = null;
    public $paymentPage = null;
    public array $requiredFields = [];
    public array $shippingPlugins = [];
    
    //-----------------------------------------------------
	function __construct(){
		
		global $cart;
		global $_CF;
		global $_Common;
		global $secure_url;
		global $_Totals;

		$cart->LoadCart();
		
		// set return page for login on checkout form
		$_SESSION['logging_in_from'] = "$secure_url/checkout.php";

		// set the last visited page in the cart for
		// abandon carts

		$cart->updateLastPage("checkout.php");

		// Get all form variables for checkout		
		$this->GetAddressFormVars();

		// Get list of required fields
		$this->getRequiredFields();

		if(isset($_Totals['totalWeight']) && $_Totals['totalWeight'] == 0){
			if($_CF['shipping']['hide_shipping_on_zero_weight']){
				$_CF['shipping']['require_shipping'] = false;
			}
			else{
				if(isset($_CF['shipping']['bypass_plugin_on_zero_weight']) && !$_CF['shipping']['bypass_plugin_on_zero_weight']){
					// see how many shipping plugins are set
					$this->checkShippingPlugins();
				}
			}
		}
		else{
			// see how many shipping plugins are set
			$this->checkShippingPlugins();
		}

	}
	
	//-----------------------------------------------------
	function GetAddressFormVars(){

		global $_CART;
		global $_Common;
		global $_DB;
		global $_Registry;

		$cart = $_Registry->LoadClass('Cart');

		if(count($_CART) == 0){
			return false;	
		}

		$this->paymentMethods = $this->getPaymentMethods();
		$this->haveCoupons = $this->checkCoupons();
		
		if(!empty($_SESSION['cid'])){
			$cid = $_SESSION['cid'];
			if(strlen($cid) < 32){
				$cid = md5($cid);	
			}
			$sql = "SELECT * FROM customers WHERE MD5(cid) = '$cid' LIMIT 1";
			$custRecord = $_DB->getRecord($sql);
			if(count($custRecord) > 0){
				foreach($custRecord as $key=>$val){
					if(substr($key,0,11) == "billaddress"){
						$_SESSION[$key] = trim($val);
					}	
				}
				// get shipping records				
				$this->getAddressBook();
			}
		}
		
		$this->checkForTaxTable();
		
		return true;
	}
	
	//-----------------------------------------------------
	function getPaymentMethods(){
		
		global $_CF;
		global $_Common;
		global $_DB;
		global $_Registry;

		// get payment page names
		$payPage = null;
		$paymentMethods = array();
		$methods = $_DB->getRecords("SELECT * FROM payment_methods WHERE `active` = 'true'",'method');
		
		if(count($methods) > 0){
			foreach(array_keys($methods) as $i=>$name){
				$flds = explode('.',$name);
				$text = $flds[0];
				$text = strtolower(str_replace(".html","",$text));
				$text = ucwords(str_replace("_"," ",$text));

				$paymentMethods[$name] = trim($text);

				if(!$payPage){
					$payPage = $name;
				}
			}
		}
		
		// load the payment page here if we are not showing shipping		
		if(!$_CF['shipping']['require_shipping'] && $payPage){

			//$_Common->debugPrint($payPage);
			
			if(file_exists("templates/payment_methods/$payPage")){
				$vars['defaultPaymentMethod'] = $payPage;
				$vars['cardTypes'] = array_keys($_DB->getRecords("SELECT `card_name` FROM `credit_cards` WHERE `active` = 'true'",'card_name'));
				$template = $_Registry->loadClass('template');
				$this->paymentPage = $template->open("templates/payment_methods/$payPage",$vars,false,true);
				$_SESSION['payment_method'] = $payPage;
			}
			else{
				$_Common->printErrorMessage("Checkout Form Missing","The Checkout Form ($payPage) is Missing");
			}
		}
		
		return $paymentMethods;
	}


	//-----------------------------------------------------
	function checkCoupons(){
		global $_DB;
		$count = $_DB->getCount("discounts","WHERE coupon != ''");
		if($count > 0){
			return true;	
		}
	return false;	
	}

	// --------------------------------------------------------------
	function getAddressBook(){
		
		global $_Common;
		global $_DB;
		
		$cid = NULL;
		
		$addressBook = array();
		if(isset($_SESSION['cid']) && $_SESSION['cid'] != "0"){
			$cid = $_SESSION['cid'];
		}
	
		if($cid != ""){
			$sql = "SELECT *,CONCAT(`shipaddress_firstname`,' ',`shipaddress_lastname`,', ',shipaddress_addr1,', ',shipaddress_addr2) as shipaddress_selection
					FROM customer_shipping WHERE MD5(cid) = '$cid' ORDER BY primary_address";
			if($this->debug){
				print "<pre>$sql\n";
			}
			$addressBook = $_DB->getRecords($sql);

			if(count($addressBook) > 0){
				$csid = $addressBook[0]['csid'];
				if(isset($_REQUEST['csid']) && $_REQUEST['csid'] != "0"){
					$csid = trim($_REQUEST['csid']);
				}
				foreach($addressBook as $i=>$fields){
					unset($addressBook[$i]['cid']);
					if($csid && $csid == $fields['csid']){
						foreach($fields as $key=>$val){
							if($key != "cid"){
								$_SESSION[$key] = trim($val);
							}
						}	
					}
				}
			}
		}
		$this->addressBook = $addressBook;
		if($this->debug){
			$_Common->debugPrint($addressBook,"Address Book records");
		}
	}
	
	// --------------------------------------------------------------
	function checkForTaxTable(){

		global $_Common;
		global $_DB;

		//get the list of all states using county based tax tables
		$sql = "Select * from sales_tax_us WHERE use_tax_table = 'true' AND tax_table_form_field = 'shipaddress_county'";
        $salesTaxData = $_DB->getRecords($sql,'state');

		$counties = array();
		
		foreach($salesTaxData as $ab=>$flds){

			if(file_exists("./$ab.tax")){
				$this->taxTable = true;
				$this->taxTableField = "shipaddress_county";
				$fileData = file("./$ab.tax");
				$list = array();
				foreach($fileData as $i=>$row){
					if(trim($row) != ""){
						$fields = explode("|",$row);
						$name = $fields[0];
						//filter out any ' as they will case the JS to choke
						//and filter any " for safety to help prevent chokes..
						$name = str_replace(array("'", '"')," ",$name);
						if(strtolower($name) != 'default'){
							//maybe a default tax rate entry in the counties file
							//dont show it in the counties list
							$list[$name] = 1;
						}
					}
				}
				if(count($list) > 0){
					ksort($list);
					$counties[$ab] = array_keys($list);
				}
			}
		}
		
		$countyJava = "";
		foreach($counties as $ab=>$cList){
			if(is_array($cList)){
				$List = "'" . join("','",$cList) . "'";
				$this->countyJava .= "\tthis.$ab = new Array($List);\n";
			}
		}
	}
	// --------------------------------------------------------------
	function getRequiredFields(){
		global $_CF;
		if(!empty($_CF['required_fields'])){
			foreach($_CF['required_fields'] as $name=>$value){
				if($value == true){
					$this->requiredFields[$name] = true;
				}
			}
		}
	}
	
	// --------------------------------------------------------------
	function checkShippingPlugins(){
		
		global $_Common,$_DB;
		
		$shipExt = $_DB->getRecord("SELECT use_shipping_plugin,shipping_plugin_name FROM shipping");
		if($shipExt['use_shipping_plugin'] == 'true' && trim($shipExt['shipping_plugin_name']) != ""){
			$fileNames = explode(',',trim($shipExt['shipping_plugin_name']));
			foreach($fileNames as $i=>$name){
				$flds = explode('.',$name);
				$this->shippingPlugins[] = $flds[0];	
			}
		}
		//$_Common->debugPrint($this->shippingPlugins);
	}
	
	
	
}






?>