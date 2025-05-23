<?php
// -----------------------------------------------------------------------
//         Class: ShippingRates
//       Purpose: This class does all the default shipping calculations for
//				  for the shipping and receipt pages.
// Last Modified: March 08, 2006
//
// -----------------------------------------------------------------------
class Shipping_rates{

    var $debug = false;
    
    // these vars get passed from calcs.inc
	var $Country = "US";
	var $State = "";
	var $Zip = "";
	var $County = "";
	var $carrier = "";
	var $freeShipping = false;
	var $freeShippingText = "";
	
	// ----------------------------------------------------
    function __construct($calcs = null){

		if(is_object($calcs)){
			// load class vars from calcs
			foreach(get_object_vars($calcs) as $name=>$value){
				if(isset($this->$name)){
					$this->$name = $value;
				}
			}
		}
	}
	
	//-----------------------------------------------------
	function calculateShipping(){
		
		global $_CF;
		global $_Common;
		global $_Totals;
		
		$shipping = 0;
		$shippingRateList = array();

		if($_Totals['totalWeight'] == 0 && $_CF['shipping']['hide_shipping_on_zero_weight']){
			return array(array(),"0",$_CF['shipping']['free_shipping_text']);
		}
		else{
			list($shippingRateList,$rate) = $this->GetShippingRateList(true);//marcello	
		}
		
		if($this->debug){
			$_Common->debugPrint(get_object_vars($this),"Shipping.Rates Class Properties");	
		}
		
		return array($shippingRateList,$rate,$this->carrier);
	}

	// ----------------------------------------------------	
	function GetShippingRateList($forGoogle = false){

		global $calcs;
		global $_Common;
		global $_DB;
		global $_Totals;

		//get all region codes first so we can test against them
		$shipToRegion = "US";
		$regions = array();
		$sql = "SELECT DISTINCT region FROM shipping_rates";
		$rs = $_DB->execute($sql);
		while($row = $_DB->fetchrow($rs, "ASSOC")){
			$regions[$row['region']] = 1;
		}
		$_DB->free_result($rs);

		$shipToRegion = $this->Country;
		if(!isset($regions[$shipToRegion]) && $shipToRegion != "ALL"){
			$shipToRegion = "other";
		}
		$surchargeTo = $this->State;
		if($shipToRegion == "other"){
			$surchargeTo = $this->Country;
		}
		
		// get records for rates
		$rates = array();
		
		$sql = "SELECT * FROM shipping_rates WHERE region = '$shipToRegion'";
		if($forGoogle){
			$sql = "SELECT * FROM shipping_rates";
		}
		
		$rates = $_DB->getRecords($sql);

		if($this->debug){
			$_Common->debugPrint($_Totals,"Cart Totals");
		}
		
		$weight = 0;
		if(isset($_Totals['shippingWeight'])){
			$weight = $_Totals['shippingWeight'];
		}
		elseif(isset($_Totals['totalWeight'])){
			$weight = $_Totals['totalWeight'];
		}
		
		$weight = $this->formatWeight($weight);
		
		$subtotal = $_Totals['subtotal'];
		if(isset($_Totals['shippingSubtotal']) && $_Totals['shippingSubtotal'] > 0){
			$subtotal = $_Totals['shippingSubtotal'];
		}
		
		// get a list of applicable rates
		list($shippingRateList,$shippingRate) = $this->calculateShippingRate($subtotal,
																			 $_Totals['totalQuantity'],
																			 $weight,
																			 $surchargeTo,
																			 $rates,
																			 $forGoogle);
		if($this->debug){
			$_Common->debugPrint($shippingRateList,"Shipping Rates");
		}
		
		return array($shippingRateList,$shippingRate);
	}


    // -------------------------------------------------------------------
    function calculateShippingRate($subtotal,$quantity,$weight,$surchargeTo,$rates = array(),$forGoogle = false){
		
		global $calcs;
		global $_Common;
		
		$shippingRateList = array();
		$firstRate = false;
		
		if($this->freeShipping){
			$shippingRateList[$this->freeShippingText] = $_Common->format_number(0);
		}

		if($this->debug){
			$_Common->DebugPrint($rates,"Shipping Logic Rates");
		}
		
		foreach($rates as $i=>$rateFields){
			
			$region = $rateFields['region'];
			$carrier = $rateFields['carrier'];
            $subRates = $rateFields['subtotal_range'];
            $qtyRates = $rateFields['quantity_range'];
            $weightRates = $rateFields['weight_range'];
            $surcharges = $rateFields['surcharge'];
            $handling = $rateFields['handling'];

			if(trim($handling) == "0"){
				$handling = "";
			}

            $match = false;
            $shipping = "0.00";
            
            if(trim($subRates) != "" && trim($subRates) != "0"){
                list($shipping,$match) = $this->calculatePrice($subRates,$subtotal,$subtotal);
				if($match && $this->debug){ $_Common->DebugPrint("$carrier shipping subtotal Rate: $shipping");}
            }
            elseif(trim($qtyRates) != "" && trim($qtyRates) != "0"){
                list($shipping,$match) = $this->calculatePrice($qtyRates,$quantity,$quantity);
				if($match && $this->debug){$_Common->DebugPrint("$carrier shipping qty Rate: $shipping");}
            }
            elseif(trim($weightRates) != "" && trim($weightRates) != "0"){
                list($shipping,$match) = $this->calculatePrice($weightRates,$weight,$weight);
				if($match && $this->debug){$_Common->DebugPrint("$carrier shipping weight Rate: $shipping");}
            }

			if(!$match && $surcharges == "" && $handling == ""){
				continue;	
			}

            // Add in surcharges
			if($surcharges != "" && trim($surchargeTo) != ""){
				$states = explode(",",$surcharges);
				foreach($states as $j=>$state){
					@list($abrev,$surcharge) = explode(":",$state);
					if(strtoupper(trim($abrev)) == strtoupper(trim($surchargeTo))){
						$surchargeTotal = $this->calcRate($subtotal,$quantity,$surcharge);
						$shipping += $surchargeTotal;
						if($this->debug){$_Common->DebugPrint("$abrev Surcharge added for $carrier: $surchargeTotal");}
						break;
					}
				}
			}
			// Add in handling
			if($handling > 0){
				$handlingCharge = $this->calcRate($subtotal,$quantity,$handling);
				$shipping += $handlingCharge;
				if($this->debug){$_Common->DebugPrint("Handling charge added for $carrier: $handlingCharge");}
			}

			$shipping = $_Common->format_number($shipping);

			// Get only the selected matching carrier
            if(isset($_REQUEST['shipping_method']) && trim($_REQUEST['shipping_method']) != ""){
				if(trim($_REQUEST['shipping_method']) == trim($carrier)){
					$firstRate = $shipping;
					$this->carrier = $carrier;
				}
            }
            // returning from payment processors
            elseif(isset($_SESSION['shipping_method']) && trim($_SESSION['shipping_method']) != ""){
				if(trim($_SESSION['shipping_method']) == trim($carrier)){
					$firstRate = $shipping;
					$this->carrier = $carrier;
				}
            }

			if($forGoogle){
				$shippingRateList[$region][$carrier]['rate'] = $shipping;
				$shippingRateList[$region][$carrier]['surcharges'] = $surcharges;
			}
			else{
				$shippingRateList[$carrier] = $shipping;
			}

		}

		asort($shippingRateList);

		if(!$firstRate){
			// pick the first carrier
			foreach($shippingRateList as $firstCarrier=>$firstRate){
				$this->carrier = $firstCarrier;
				break;
			}
		}
		
	return array($shippingRateList,$firstRate);
	}

    // -------------------------------------------------------------------
    function calculatePrice($rates,$quantity,$subTotal){

		global $_Common;

		$match = false;
		if(strstr($rates,":")){
			
			$qtyPrices = explode(",",$rates);

			foreach($qtyPrices as $i=>$rate){
				
				@list($qty,$prc) = explode(":",$rate);

				if($prc == ""){
					$prc = "0.00";	
				}

				//calc the rate for this range
				$prc = $this->calcRate($subTotal,$quantity,$prc);
				
				if(strstr($qty,"-")){
					
					@list($low,$high) = explode("-",$qty);
					if($low == ""){
						$low = 1;
					}
					if($high == "" || $high == "+"){
						$high = 99999999;
					}
					if($quantity >= $low && $quantity <= $high){
						$price = $prc;
						$match = true;
					}
				}
				else{
					if($quantity >= $qty){
						$price = $prc;
						$match = true;
					}
				}
			}
		}
		else{
			// coupon discounts for free shipping with no prices			
			if(strstr($rates,"-")){
				@list($low,$high) = explode("-",$rates);
				if($low == ""){
					$low = 1;
				}
				if($high == "" || $high == "+"){
					$high = 99999999;
				}
				if($quantity >= $low && $quantity <= $high){
					$match = true;
					$price = 0;
				}
			}
			else{
				if($quantity >= $price){
					$match = true;
					$price = 0;
				}
			}
		}

		if(!$match){
			$price = 0;
		}

		$price = $_Common->format_number($price);

    return array($price,$match);
    }

	// -------------------------------------------------------------------
	//baseCharge - base pre existing charge/rate, used in percent pricing
	//quantity - how many parts in case of an "each" pricing
	//rate - raw rate calcuation
	//each marker must come at end of string, so percent marker
	//must come first if present. 
	function calcRate($baseCharge,$quantity, $rate){

		//check for special handleing flags.
		$isPercent = preg_match('/%/',$rate);
		$isEach = false;
		if(stristr($rate,'each') || stristr($rate,'ea') || stristr($rate,'e')){
			$isEach = true;
		}
		
		//now that we know what type of rate it is, get down to only the number
		$rate = preg_replace("/\%/","",$rate);
		$rate = preg_replace("/each/","",$rate);
		$rate = preg_replace("/ea/","",$rate);
		$rate = preg_replace("/e/","",$rate);
		
		//now run the full matrix
		if($isEach && !$isPercent){
			//per each
			$charge = $quantity * $rate;
		}
		elseif(!$isEach && $isPercent){
			//its a percent
			$charge = ($baseCharge * $rate) / 100;	
		}
		elseif($isEach && $isPercent){
			//combined percent per each
			$charge =  $quantity * ($baseCharge * $rate) / 100;
		}
		elseif(!$isEach && !$isPercent){
			//flat rate
			$charge = $rate;
		}
				
		return $charge;
	}

	//---------------------------------------------------------------
	function formatWeight($weight){
		
		global $_CF,$_Common;

		if($this->debug){
			$_Common->debugPrint($weight,"format weight");	
		}

		// This checks to see if we are using fractions
		// of a lb and converts that to integer ounces
		
		// check the total weight and calculate
		$weight = number_format($weight,2,'.','');

		// int weight will give lbs
		$lbs = intval($weight);
	
		if(isset($_CF['shipping']['use_decimal_weight']) && $_CF['shipping']['use_decimal_weight']){
			
			$multiplier = ($weight - $lbs);
			$oz = round(16 * $multiplier / 100,2);
			
			if($this->debug){
				$_Common->debugPrint($oz,"decimal ounce conversion");	
			}
		}
		else{
			$oz = number_format($weight - $lbs,2) * 100;
			while($oz > 15){
				$lbs += 1;
				$oz -= 16;	
			}
			$oz = ($oz/100);
		}
		
		$weight = number_format(($lbs + $oz),'2','.','');

		if($this->debug){
			$_Common->debugPrint($weight,"format weight result");	
		}
		
		return $weight;
	}

}

?>