<?php
// -----------------------------------------------------------------------
//         Class: Calculations
//       Purpose: This class does all the calculations for
//				  for the shipping and receipt pages.
// Last Modified: March 08, 2006
//
// -----------------------------------------------------------------------
class Calcs{

    var $debug = false;
    
	var $Country = "US";
	var $State = "";
	var $Zip = "";
	var $County = "";

	var $carrier;
	var $freeShipping = false;
	var $freeShippingText = "";
	var $shippingRateList = array();
    var $results = "";
    
    
    //-----------------------------------------------------
	function __construct($forReport = false){

		global $_CART;
		global $_CF;
		global $_Common;
		global $_Registry;
		
		if(!$forReport){
		
			if(count($_CART) == 0){
				$cart = $_Registry->LoadClass('Cart',true);
				$cart->view(true);
			}

			// put back address fields into request from reload
			foreach($_SESSION as $fld=>$val){
				if($fld != "shipping_method" && (substr($fld,0,4) == "ship" || substr($fld,0,4) == "bill" || substr($fld,0,6) == "coupon")){
					$_REQUEST[$fld] = $val;
					//$_Common->debugPrint("$fld = $val");
				}	
			}

		}

			// Country Field
		if(!empty($_REQUEST['shipaddress_country'])){
			$this->Country = trim(strtoupper($_REQUEST['shipaddress_country']));
		}
		elseif(!empty($_REQUEST['billaddress_country'])){
			$this->Country = trim(strtoupper($_REQUEST['billaddress_country']));
		}

			// State Field
		if(!empty($_REQUEST['shipaddress_state'])){
			$this->State = trim(strtoupper($_REQUEST['shipaddress_state']));
		}
		elseif(!empty($_REQUEST['billaddress_state'])){
			$this->State = trim(strtoupper($_REQUEST['billaddress_state']));
		}

			// PostalCode
		if(!empty($_REQUEST['shipaddress_postalcode'])){
			$this->Zip = trim(strtoupper($_REQUEST['shipaddress_postalcode']));
		}
		elseif(!empty($_REQUEST['billaddress_postalcode'])){
			$this->Zip = trim(strtoupper($_REQUEST['billaddress_postalcode']));
		}

			// County
		if(!empty($_REQUEST['shipaddress_county'])){
			$this->County = trim(strtoupper($_REQUEST['shipaddress_county']));
		}
		elseif(!empty($_REQUEST['billaddress_county'])){
			$this->County = trim(strtoupper($_REQUEST['billaddress_county']));
		}
		
		$this->freeShippingText = $_CF['shipping']['free_shipping_text'];

		if(!$forReport){
			$this->performCalculations();
		}

	}
	
	//-----------------------------------------------------
	function performCalculations(){

		global $_CF;
		global $_Common;
		global $_DB;
		global $_Registry;
		global $_Totals;
		
		// Initialize the totals
		$subtotal = $_Totals['subtotal'];
		$quantity = $_Totals['totalQuantity'];
		$weight = $_Totals['totalWeight'];
		$grandTotal = $subtotal;
		$fDiscount = $_Common->format_number(0);
		$fShipping = $_Common->format_number(0);
		$fSalesTax = $_Common->format_number(0);
		$freeShipping = false;

		$sequence = $_DB->getRecord("SELECT `calculate_discount`,`calculate_sales_tax`,`calculate_shipping` FROM calculation_sequence");

		if(isset($_SESSION['isRegistered']) && isset($_SESSION['is_taxable']) && $_SESSION['is_taxable'] == 'false'){
			$sequence['calculate_sales_tax'] = 0;
		}

		// load the classes for the calculations and 
		// pass reference to this class so we can get vars
		$discountClass = $_Registry->loadClass("Discounts",$this);
		$salestaxClass = $_Registry->loadClass("Salestax",$this);

		if($this->debug){
			$_Common->debugPrint("Shipping Weight: $weight");
		}
		
		$byPass = false;
		$pickup = false;
		if($weight == 0 && isset($_CF['shipping']['bypass_plugin_on_zero_weight']) && $_CF['shipping']['bypass_plugin_on_zero_weight']){
			// no weight, bypass shipping plug-ins. This allows them 
			// to still use the default logic and set a handling charge
			$byPass = true;
		}
		
		if(isset($_CF['shipping']['offer_local_pickup']) && $_CF['shipping']['offer_local_pickup']){
			if(basename($_SERVER['PHP_SELF']) == "shipping.php" && isset($_REQUEST['pickup'])){
				$_SESSION['pickup'] = "true";
			}
			elseif(basename($_SERVER['PHP_SELF']) == "shipping.php" && !isset($_REQUEST['pickup']) && isset($_SESSION['pickup'])){
				unset($_SESSION['pickup']);	
			}
			if(isset($_SESSION['pickup'])){
				$rates[$_CF['shipping']['local_pickup_text']] = $_Common->format_price(0);
				$_REQUEST['shipping_method'] = $_CF['shipping']['local_pickup_text'];
				$firstRate = $_Common->format_price(0);
				$_SESSION['rates'] = $rates;
				$_SESSION['shippingRate'] = $firstRate;
				$byPass = true;
				$pickup = true;
			}
		}
		

		if(!$byPass){
			// check the shipping table to see if Shipping Plugin Name is added and
			// "Use Shipping Plugin" is on. If so, try to dynamically load it here
			$shipExt = $_DB->getRecord("SELECT use_shipping_plugin,shipping_plugin_name FROM shipping");
			
			if($shipExt['use_shipping_plugin'] == 'true' && trim($shipExt['shipping_plugin_name']) != ""){
				$fileNames = explode(',',trim($shipExt['shipping_plugin_name']));
				$plugin = $fileNames[0];
				if(!empty($_REQUEST['preferred_shipper'])){
					$_SESSION['preferred_shipper'] = trim($_REQUEST['preferred_shipper']);
				}
				if(count($fileNames) > 1){
					foreach($fileNames as $i=>$name){
						if(!empty($_SESSION['preferred_shipper']) && substr($name,0,strlen($_SESSION['preferred_shipper'])) == $_SESSION['preferred_shipper']){
							$plugin = trim($name);
							break;
						}
					}
				}
				if($this->debug){
					$_Common->debugPrint($plugin,"Selected Shipping Plug-in");
				}
				if($_Registry->file_exists_incpath($plugin)){
					$_Registry->registeredClasses['shipping.rates.inc'] = $plugin;
				}
			}
		}
		if(!$pickup){
			$shiprateClass = $_Registry->loadClass("Shipping.Rates",$this);
		}

		$temp_total = $subtotal;

		for($i=1;$i<=3;$i++) {

			$discount = 0;
			$shipping = 0;
			$sales_tax = 0;

			if(intval($sequence['calculate_discount']) == $i){
				list($discount,$this->freeShipping) = $discountClass->calculateDiscount($temp_total,$quantity);
				$shiprateClass->freeShipping = $this->freeShipping;
				if($this->debug){
					print "<pre>(sequence - $i)discount done: $discount, free shipping: " . $this->freeShipping . "\n";
				}
			}

			if(intval($sequence['calculate_sales_tax']) == $i){
				$taxableShipping = $fShipping;
				if($this->freeShipping){
					$taxableShipping = $_Common->format_number(0);
				}
				$sales_tax = $salestaxClass->calculateSalesTax($taxableShipping,$fDiscount);
				if($this->debug){
					print "<pre>(sequence - $i)salestax done: $sales_tax\n";
				}
			}

			if($_CF['shipping']['require_shipping'] && intval($sequence['calculate_shipping']) == $i){
				if($pickup){
					$this->carrier = $_CF['shipping']['local_pickup_text'];
					$this->shippingRateList = $_SESSION['rates'];
					$this->shipping = $_Common->format_number(0);
				}
				else{
					$_Totals['shippingSubtotal'] = $temp_total;
					list($shippingRateList,$shipping,$this->carrier) = $shiprateClass->calculateShipping();
					
					$this->shippingRateList = $shippingRateList;
					$this->shipping = $shipping;
					
					if($this->freeShipping){
						$this->carrier = $_CF['shipping']['free_shipping_text'];
						$this->shippingRateList[$_CF['shipping']['free_shipping_text']] = $_Common->format_price(0);
						$this->shipping = $_Common->format_price(0);
					}
				}
				if($this->debug){
					print "<pre>(sequence - $i) shipping done: $shipping, carrier: " . $this->carrier . "\n";
				}
			}

			// Set the final totals for each
			if($discount > 0){
				$fDiscount = $_Common->format_number($discount);
				$grandTotal -= $fDiscount;
			}
			if($shipping > 0){
				$fShipping = $_Common->format_number($shipping);
				$grandTotal += $fShipping;
				if(isset($_Totals['insurance']) && $_Totals['insurance'] > 0){
					$grandTotal += $_Common->format_number($_Totals['insurance']);
				}
			}
			if($sales_tax > 0){
				$fSalesTax = $_Common->format_number($sales_tax);
				$grandTotal += $fSalesTax;
			}
			
			$temp_total = $temp_total - $discount + $shipping + $sales_tax;
			
		}

		// free shipping
		if($this->freeShipping && (trim($this->carrier) == "" || empty($_REQUEST['shipping_method']))){
			$this->carrier = $this->freeShippingText;
		}
		elseif($freeShipping && !empty($_REQUEST['shipping_method']) && $_REQUEST['shipping_method'] == $this->freeShippingLabel){
			$this->carrier = $this->freeShippingLabel;
		}
		if($this->freeShipping && $fShipping > 0 && $this->carrier == $this->freeShippingText){
			$grandTotal -= $fShipping;
			$fShipping = $_Common->format_price("0.00");
		}

		$_Totals['discount'] = $_Common->format_number($fDiscount);
		$_Totals['shipping'] = $_Common->format_number($fShipping);
		$_Totals['salestax'] = $_Common->format_number($fSalesTax);
		$_Totals['grandtotal'] = $_Common->format_number($grandTotal);
		$_Totals['free_shipping'] = $this->freeShipping;
		
		$_SESSION['totals'] = $_Totals;
		
		if($this->debug){
			$_Common->debugPrint($_Totals,"Calculated Totals");	
		}
		
	}
}
?>