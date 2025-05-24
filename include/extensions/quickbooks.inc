<?php

class QuickBooks {
	
	public bool $debug = false;
	public array $cfg = [];
	
	
	// ----------------------------------------------------
	function __construct(){
		
		global $_Common;
		
		// load the config file for quickbooks
		$this->cfg = parse_ini_file("include/extensions/quickbooks.config.php");
		//$_Common->debugPrint($this->cfg);
	}
	
	// ----------------------------------------------------
	function WriteLog($order_number){
		
		global $_CART;
		global $_Common;
		global $_CR;
		
		if(!$this->cfg['write_xml_files']){
			return true;
		}
		
		$transtype = "Invoice";
		if(strtolower($this->cfg['transaction_type']) != "invoice"){
			$transtype = "SalesReceipt";
		}
		
		if(!empty($_SESSION['payment_method']) && $_SESSION['payment_method'] == "credit_card.html"){
			$paymentMethod = "Credit Card";
			// marcello: Add Credit Card Deposit Account 
      $deposit_To_Account = $this->cfg['deposit_to_account_cc'];
		}
		elseif(!empty($_SESSION['payment_method']) && $_SESSION['payment_method'] == "check.html"){
			$paymentMethod = "Check";
			// marcello: Add Check Deposit Account 
      $deposit_To_Account = $this->cfg['deposit_to_account_check'];
		}		
		elseif(!empty($_SESSION['payment_method']) && $_SESSION['payment_method'] == "mail_in_payment.html"){
			$paymentMethod = "Check";
			// marcello: Add Check Deposit Account 
      $deposit_To_Account = $this->cfg['deposit_to_account_check'];
		}		
		elseif(!empty($_SESSION['payment_method']) && $_SESSION['payment_method'] == "paypal.html"){
			$paymentMethod = "PayPal";
			// marcello: Add PayPal Account 
      $deposit_To_Account = $this->cfg['deposit_to_account_pp'];
		}
		else{
			$paymentMethod = "Cash";
			// marcello: Add Cash Deposit Account 
      $deposit_To_Account = $this->cfg['deposit_to_account_cash'];
		}
      // marcello: Add Customer Message 
          $customer_Msg = $this->cfg['customer_message']; 
		  
      // marcello: Add Customer Type 
          $customer_Type = $this->cfg['customer_type']; 
   
      // marcello: Add default class 
          $default_class = $this->cfg['default_class'];    
      
      // marcello: Add Item Type (group or inventory) 
          $item_Type = $this->cfg['item_type'];    
      
      // marcello: specify SKUs (items) that should be ignored but not their options
         $ignore_Skus = explode(" ",$this->cfg['ignore_skus']);    

      // marcello: Add words to ignore in options 
          $ignore_Words = explode(" ",$this->cfg['ignore_words']);
      
      // marcello: Add items who's options should be ignored 
          $ignore_Items = explode(" ",$this->cfg['ignore_items']);
         
      // marcello: Add itmes that are priced in options 
      //    $priced_Items = explode(" ",$this->cfg['priced_items']);    
         
      // marcello: Add itmes that are free in options 
          $free_options = explode(" ",$this->cfg['free_options']);       
		  
      // marcello: Add itmes  which quantity should be used in determining quantity in options 
          $items_Drive_Option_Qty = explode(" ",$this->cfg['items_drive_option_qty']);       

		$order_date = date("Y-m-d"); 
// Marcello-  change time format
//		$job_order_date = date("l, F d, Y at g A");
	  $job_order_date = date("l M dS Y, h_i_s A");
		
			// Form Comments field

		$_SESSION['memo'] = "";
		$comments = null;

		if(!empty($_SESSION['card_number'])){
// Marcello only show last for digits of Credit Card
//			$comments .= $_SESSION['card_number'] . ", ";
			$comments .= "..." . substr($_SESSION['card_number'],-4,4) . ", **/**";
//			$comments .= "..." . substr($_SESSION['card_number'],-4,4) . ", ";
//			$comments .= $_SESSION['expire_month'] . '/' . $_SESSION['expire_year'] . ", ";
//			$comments .= $_SESSION['cvv2'];
		}

		if(isset($_SESSION['comments'])){
			$comments .= " - " . $_SESSION['comments'];
		}
			// QuickBooks Memo field

		if($comments){
			$_SESSION['memo'] = "<Memo>$comments</Memo>\n";
		}

		$salesTaxCode = "Non";
		$salesTaxItem = "Out of State";
		
		if($_SESSION['totals']['salestax'] > 0){
			$salesTaxCode = $this->cfg['sales_tax_code'];
			$_SESSION['sales_tax_code'] = $salesTaxCode;
			if($salesTaxCode != "Non"){
				$salesTaxItem = $_SESSION['shipaddress_state'];
				// Marcello - add string " Sales Tax" to state abbreviation 
				$salesTaxItem .=" Sales Tax";
				//Marcello check for County Sales Tax and adjust accordingly
				//We are using Zip codes for tax calculation not counties anymore
//				if(strpos($_SESSION['shipaddress_county'], 'Santa') !== false){
//					$salesTaxItem .=" SC County";
//				}
//				if(strpos($_SESSION['shipaddress_county'], 'Alameda') !== false){
//					$salesTaxItem .=" AL County";
//				}
			}
		}
		else{
			//non_taxable_item
			if($this->cfg['non_taxable_item'] != ""){
				$salesTaxItem = $this->cfg['non_taxable_item'];
			}
		}

		// shipping method
		$shipping_method = null;
		if(!empty($_SESSION['shipping_method']) && trim($_SESSION['shipping_method']) != ""){
			if(strlen($_SESSION['shipping_method']) > 15){
				$_SESSION['shipping_method'] = str_replace('U.S.P.S ','',$_SESSION['shipping_method']);
				$_SESSION['shipping_method'] = substr($_SESSION['shipping_method'],0,15);
			}
			$shipping_method = trim($_SESSION['shipping_method']);
			$shippingItem = $this->cfg['qb_shipping_item'];
		}
		
		//Marcello use the FOB field for getting the Country into an exportable field
		//this is done in the XML file

		//Marcello use the REP field for getting the carrier (preferred_shipper) into QB
		$sales_rep = "UPS"; // default to UPS if all fails
		if(!empty($_SESSION['preferred_shipper'])){
			$sales_rep = $_SESSION['preferred_shipper'];
		}
		// Add the Sales Rep if applicable
//		$sales_rep = null;
//		if(!empty($_SESSION['affiliate_id'])){
//			$sales_rep = substr($_SESSION['affiliate_id'],0,5);
//		}
//		else{
//			if($this->cfg['sales_rep'] != ""){
//				$sales_rep = substr($this->cfg['sales_rep'],0,5);
//			}
//		}

        // Set the right format for the customer name
		$nameFmt = $this->cfg['customer_name_format'];
		if(strstr($nameFmt,',')){
			$nameFlds = explode(',',$nameFmt);
			$_SESSION['billaddress_name'] = $_SESSION[$nameFlds[0]] . ", " . $_SESSION[$nameFlds[1]];
		}
		else{
			$nameFlds = explode(' ',$nameFmt);
			$_SESSION['billaddress_name'] = $_SESSION[$nameFlds[0]] . " " . $_SESSION[$nameFlds[1]];
		}

		$coupon = null;
		if($_SESSION['totals']['discount'] > 0 && isset($_SESSION['coupon'])){
//  Marcello swap coupon description with coupon item; this way 
//  only one one item has to be setup in QuickBooks for all discounts
//			$coupon = $_SESSION['coupon'];
//			$coupon_description = $this->cfg['qb_discount_desc'];
			$coupon = $this->cfg['qb_discount_desc'];
			$coupon_description = $_SESSION['coupon'];
		}

    foreach($_CART as $i=>$fields){
			$_CART[$i]['fullname'] = $fields['name'];
			if(isset($fields['options']) && count($fields['options']) > 0){
			   foreach($fields['options'] as $j=>$flds){
				   $_CART[$i]['fullname'] .= ", " . $flds['value'];
			   }
			}
		}

		$shipTaxCode = $salesTaxCode;
		if(!$this->cfg['shipping_is_taxable']){
			$shipTaxCode = "Non";
		}

		// TODO: Investigate quickbooks.xml. If it's an XML data file, load with file_get_contents() instead of include_once.
		error_reporting(E_PARSE|E_WARNING);
		ob_start();
		include_once('include/extensions/quickbooks.xml');
		$xml = ob_get_contents();
		ob_end_clean();
		error_reporting(E_ALL);

		$xml = str_replace("\t","",$xml);
		$xml = str_replace(">$_CR",">",$xml);
		$xml = str_replace("%$_CR","%",$xml);
		$xml = str_replace("=$_CR","=",$xml);
		while(strstr($xml,"  ")){
			$xml = str_replace("  "," ",$xml);	
		}
		$xml = str_replace("> <","><",$xml);

		$fileName = $this->cfg['xml_file_name'];
		$fileName = str_replace('ORDERNUM',$order_number,$fileName);

		if(!is_dir("./orders")){
			if(!mkdir("./orders",0777)){
				$_Common->printErrorMessage("QuickBooks Plugin Error","Could not create orders directory for QuickBooks xml files.");
			}
			if(!($HT = fopen("./orders/.htaccess", "w"))){
				$_Common->printErrorMessage("Program Error","Could not create .htaccess file in orders.");
			}
			fputs($HT,"<Limit GET POST>$_CR");
			fputs($HT,"deny from all$_CR");
			fputs($HT,"</Limit>$_CR");
			fclose($HT);
			
			if(!($IN = fopen("./orders/index.html", "w"))){
				$_Common->printErrorMessage("Program Error","Could not create index.html file in orders.");
			}
			fputs($IN,"<html><head><title>403 Forbidden</title></head><body><h1>Forbidden</h1>$_CR");
			fputs($IN,"<p>You don't have permission to access /orders on this server.</p>$_CR");
			fputs($IN,"</body></html>$_CR");
			fclose($IN);
		}

		$orderFile = "./orders/$fileName";

		if(!($LOG = fopen($orderFile, "w"))){
			$orderFile = basename($orderFile);
			$_Common->printErrorMessage("Program Error","Could not write to $orderFile log file");
		}
		fputs($LOG,$xml);
		fclose($LOG);	
		
		if($this->debug){
			global $_Totals;
			$_Common->debugPrint($_Totals,"Totals");
			$this->showXML($xml);
			exit;
		}
	}
	
	
    // -------------------------------------------------------------------
    function showXML($xml){
        $xml = preg_replace("/</","&lt;",$xml);
        $xml = preg_replace("/>/","&gt;",$xml);
        print "<pre>$xml</pre>";
    }
	
	
}




















?>