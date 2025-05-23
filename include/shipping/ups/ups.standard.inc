<?php

class X_Shipping_Rates extends Shipping_Rates{
	
	var $cfg = null;
	var $error = false;
	var $rate_charts = array('1'=>'Regular+Daily+Pickup',
							 '3'=>'Customer+Counter',
                             '6'=>'One+Time+Pickup',
                             '7'=>'On+Call+Air', 
                             '19'=>'Letter+Center',
                             '20'=>'Air+Service+Center');
                             
    var $serviceReferences = array(	'GND'=>3,
									'3DS'=>12,
									'2DA'=>2,
									'2DM'=>59,
									'1DM'=>14,
									'1DA'=>1,
									'1DP'=>13,
									'STD'=>11,
									'XPR'=>7,
									'XDM'=>54,
									'XPD'=>8);
	var $totalweight = 0;
	var $totalpackages = 0;
	
	// --------------------------------------------------------------
	function __construct(){
		
	}
	// ----------------------------------------------------	
	function GetShippingRateList(){

		global $_CF;
		global $_CART;
		global $_Common;
		global $_Totals;

		$this->debug = true;

		global $_DB;
		$shipCalcFields = $_DB->getFieldProperties('shipping');
		if(!isset($shipCalcFields['use_decimal_weight'])){
			$_DB->execute("ALTER TABLE `shipping` ADD `use_decimal_weight` ENUM( 'true', 'false' ) NOT NULL DEFAULT 'false'");
			$_DB->execute("INSERT INTO `help` (`section`, `section_help`, `key`, `key_help`) VALUES ('shipping', '', 'use_decimal_weight', 'Use decimal weights for shipping plug-ins? ie. 1.4 lbs.')");
		}

		// If we are just picking a different service, use the store values.
		if(isset($_REQUEST['continue']) && isset($_REQUEST['shipping_method']) && isset($_SESSION['rates'])){
			$rates = $_SESSION['rates'];
			$firstRate = $_SESSION['shippingRate'];
			foreach($rates as $service=>$postage){
				// flag selected carrier
				if(trim($_REQUEST['shipping_method']) == trim($service)){
					$firstRate = $postage;
					$this->carrier = trim($service);
				}
			}
			return array($rates,$firstRate);
		}

		if($this->debug){
			$_Common->debugPrint($_Totals, "Cart Totals");
		}

		// weight undefined.
		if($_Totals['totalWeight'] == 0){
			$rates[$_CF['shipping']['free_shipping_text']] = $_Common->format_price(0);
			$firstRate = $_Common->format_price(0);
			$_SESSION['rates'] = $rates;
			$_SESSION['shippingRate'] = $firstRate;
			return array($rates,$firstRate);
		}


		// load the config file for UPS
		$this->cfg = parse_ini_file("include/shipping/ups/ups.config.php",true);

		// make shipments by zip code
		$shipments = array();
		$this->buildPackages($shipments);
		
		if($this->debug){
			$_Common->debugPrint($this->totalweight,"Total Shipping Weight");
			$_Common->debugPrint($shipments,"packages");
		}

		if(count($shipments) > 0){

			$rates = array();
			$firstRate = null;
			$handling = $this->cfg['settings']['handling_charge'];
			
			foreach($shipments as $zip=>$pkgs){
			
				foreach($pkgs as $i=>$pkg){
			
					// query UPS
					$response = $this->queryUPS($zip,$pkg['weight'],$pkg['pkgcode']);
					
					if(count($response) > 0){
						
						foreach($response as $index=>$fields){
							
							$code = $fields[5];
							
							if(isset($this->serviceReferences[$code]) && isset($this->cfg['services'][$this->serviceReferences[$code]])){
								$service = $this->cfg['services'][$this->serviceReferences[$code]];
								
								$postage = $fields[14];

								if(isset($rates[$service])){
									$rates[$service] += $_Common->format_price($postage);
								}
								else{
									// add handling only the first time
									$postage += $handling;
									$rates[$service] = $_Common->format_price($postage);
								}
								$rates[$service] = $_Common->format_price($rates[$service]);
							}
						}
					}
					else{
						if($this->error){
							$_Common->printErrorMessage("Shipping Rate Request Error",$this->errorMessage);	
						}
					}
				}
			}
			
			asort($rates);
			
			if($this->debug){
				$_Common->debugPrint($rates, "UPS Rates");
			}

			foreach($rates as $firstCarrier=>$firstRate){
				// Get only the selected matching carrier
				if(isset($_REQUEST['shipping_method'])){
					if(trim($_REQUEST['shipping_method']) == $firstCarrier){
						$this->carrier = $firstCarrier;
						break;
					}
				}
				else{
					// pick the first carrier
					$this->carrier = $firstCarrier;
					break;
				}
			}

			// save for refresh on shipping page
			$_SESSION['rates'] = $rates;
			$_SESSION['shippingRate'] = $firstRate;
			
			return array($rates,$firstRate);
		}
		else{
			if($this->error){
				$_Common->printErrorMessage("Shipping Rate Request Error",$this->errorMessage);	
			}
			else{
				$_Common->printErrorMessage("Shipping Rate Request Error","No packages defined");	
			}
		}
	}


	//---------------------------------------------------------------
	function buildPackages(&$shipment){
		
		global $_CART;
		global $_Common;
		global $_isAdmin;

		// This is for the orders report if we make updates
		// to an order we have to recalculate the totals
		if($_isAdmin && !empty($_SESSION['report_cart'])){
			$_CART = $_SESSION['report_cart'];
		}

		// packaging
		list($PackagingCode,$PackagingDesc) = explode(',',$this->cfg['settings']['PackagingType']);
		
		$orderweight = 0;
		
		foreach($_CART as $i=>$fields){

			$qty = trim($fields['quantity']);
			$weight = 0;
			$size = "";
			$length = null;
			$width = null;
			$height = null;
			$optionWeight = 0;
			
			// check option weights
			if(isset($fields['options']) && is_array($fields['options']) && count($fields['options']) > 0){
				foreach($fields['options'] as $k=>$flds){
					if(is_numeric($flds['weight']) && $flds['weight'] > 0){
						$optionWeight += $flds['weight'];
					}
				}
			}
			if(!strstr($fields['weight'],":") && $optionWeight > 0){
				$fields['weight'] += $optionWeight;
			}

			if($this->debug){
				$_Common->debugPrint($fields['weight'],'cart weight');	
			}

			if(trim($fields['weight']) == "" || trim($fields['weight']) == 0){
				continue;
			}
			elseif(strstr($fields['weight'],":")){

				@list($weight,$size,$type,$box,$zip) = explode(':',$fields['weight']);
				
				if(trim($size) != ""){
                    $size = strtolower(trim($size));
                }
                else{
					if(!empty($fields['size'])){
						$size = strtolower(trim($fields['size']));
					}
				}
                
                if(strstr($size,'x')){
					list($length,$width,$height) = explode('x',strtolower(trim($size)));	
				}
                
                if(trim($type) == ""){
                    $type = "S";
                }
				if(trim($zip) == ""){
					$zip = trim($this->cfg['settings']['originPostalCode']);
				}
				
				// default package settings
				$pkgCode = $PackagingCode;
				$pkgDesc = $PackagingDesc;
				
				if(trim($box) != ""){
					@list($pkgCode,$pkgDesc) = explode(',',$box);
				}

                if(substr($type,0,1) == "M"){
					
					// multi-packages
                    @list($flag,$numperbox) = explode('-',$type);
                   
                    if(is_numeric($numperbox) && $numperbox > 0){
						
                        $pkgweight = $weight;
                        $numboxes = ceil($qty / $numperbox);
                        $remaining = $qty;

                        for($i = 1; $i<=$numboxes; $i++){
							
                            $pkgqty = $numperbox;
                            if($i == $numboxes){
                                $pkgqty = $remaining;
                            }
                            
                            $weight = $this->formatWeight($pkgweight * $pkgqty);

							if($weight < 1){
								$weight = 1.0;
							}
                            
							$shipment[$zip][] = array(	'weight'   => $weight,
														'length'   => trim($length),
														'width'	   => trim($width),
														'height'   => trim($height),
														'pkgcode'  => trim($pkgCode),
														'pkgdesc'  => trim($pkgDesc));
                            
                            $this->totalpackages++;
                            $this->totalweight += $weight;
                            $remaining -= $pkgqty;
                        }
                    }
					else{
						// make single packages
                        $weight = $this->formatWeight($weight * $qty);
                        
                        if($weight < 1){
							$weight = 1.0;
						}
                        
                        for($i = 1; $i<=$qty; $i++){
							$shipment[$zip][] = array(	'weight'   => $weight,
														'length'   => trim($length),
														'width'	   => trim($width),
														'height'   => trim($height),
														'pkgcode'  => trim($pkgCode),
														'pkgdesc'  => trim($pkgDesc));
	                        
							$this->totalpackages++;
							$this->totalweight += $weight;
						}
					}
				}
				else{
					// make single packages
					for($i = 1; $i<=$qty; $i++){
						$weight = $this->formatWeight($weight);
						
                        if($weight < 1){
							$weight = 1.0;
						}
						
						$shipment[$zip][] = array('weight'   => $weight,
											'length'   => trim($length),
											'width'	   => trim($width),
											'height'   => trim($height),
											'pkgcode'  => trim($pkgCode),
											'pkgdesc'  => trim($pkgDesc));
	                    
						$this->totalpackages++;
						$this->totalweight += $weight;
					}
				}
			}
			else{
				$orderweight += $this->formatWeight($fields['weight'] * $qty);
			}
		}
		
		if($orderweight > 0){
			
			$weight = $orderweight;
            
            if($weight < 1){
				$weight = 1.0;
			}
			
			if($weight > 150){
				$loopWeight = $weight;
				// loop and create multiple packages weighing 150 lbs
				while($loopWeight > 150){
					$this->addOrderPackage($shipment,150);
					$loopWeight -= 150;
				}
				// get any remaining packages
				if($loopWeight > 0){
					if($loopWeight < 1){
						$loopWeight = 1.0;
					}
					$this->addOrderPackage($shipment,$loopWeight);
				}
			}
			else{
				$this->addOrderPackage($shipment,$weight);
			}
            
		}
		$_SESSION['number_of_packages'] = $this->totalpackages;
	}

	//---------------------------------------------------------------
	function addOrderPackage(&$shipment,$weight){
	
		$this->totalpackages++;
        $this->totalweight += $weight;
		
		if($weight < 1){
			$weight = 1.0;
		}
		
		$zip = trim($this->cfg['settings']['originPostalCode']);
		list($PackagingCode,$PackagingDesc) = explode(',',$this->cfg['settings']['PackagingType']);
		
		$shipment[$zip][] = array('weight'   => $weight,
								  'length'   => trim($this->cfg['settings']['default_package_length']),
								  'width'	 => trim($this->cfg['settings']['default_package_width']),
								  'height'   => trim($this->cfg['settings']['default_package_height']),
								  'pkgcode'  => trim($PackagingCode),
								  'pkgdesc'  => trim($PackagingDesc));
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

	
	// -------------------------------------------------------------------
	function queryUPS($originPostalCode,$packageWeight,$packaging){

		global $_Common;						
		global $_Totals;

		$destCountry = $this->Country;
		$destZipCode = $this->Zip;

		$subtotal = $_Totals['subtotal'];

		if($packageWeight == 0) {
			$packageWeight = 1.0;
		}
		$defaultSvcCode = "GND";
		$ResidentialAddress = "1";
		if(isset($_REQUEST['shipaddress_delivery_type']) && $_REQUEST['shipaddress_delivery_type'] == 'commercial'){
			$ResidentialAddress = "0";
		}

		// Canadian Orders ONLY
		if($this->Country == "CA"){
			$defaultSvcCode = "STD";
		}
		// Remove the zip code if not in the US or Canada
		if($this->Country != "US" && $this->Country != "CA"){
			$this->Zip = "";
			$defaultSvcCode = "XPR";
		}

		$rateChart = $this->rate_charts[intval($this->cfg['settings']['PickupType'])];

		$url = "http://www.ups.com/using/services/rave/qcost_dss.cgi";
		$request = "AppVersion=1.2&";
		$request .= "AcceptUPSLicenseAgreement=yes&";
		$request .= "ResponseType=application/x-ups-rss&";
		$request .= "ActionCode=4&";
		$request .= "ServiceLevelCode=$defaultSvcCode&";
		$request .= "RateChart=$rateChart&";
		$request .= "ShipperPostalCode=$originPostalCode&";
		$request .= "ConsigneePostalCode=$destZipCode&";
		$request .= "ConsigneeCountry=$destCountry&";
		$request .= "PackageActualWeight=$packageWeight&";
		$request .= "DeclaredValueInsurance=$subtotal&";
		$request .= "ResidentialInd=$ResidentialAddress&";
		$request .= "PackagingType=$packaging";

		$response = $this->fOpenRequest($url,$request);

		if(!$response){
			$response = explode('<br />',nl2br($this->sendRequest($url,$request)));
		}

		foreach($response as $i=>$row){
			
			if(strstr($row,"UPSOnLine")){
			
				$flds = explode('%',$row);
				
				//$_Common->debugPrint($flds);
				
				if($flds[3] != "0000Success"){
					if(trim($flds[3]) == "1035The maximum per package weight for the selected service from the selected country is 150 pounds."){
						$this->error = true;
						$this->errorMessage = "The maximum per package weight has been exceeded";
					}
					elseif(trim($flds[3]) == "6836Invalid ConsigneePostalCode"){
						$this->error = true;
						$this->errorMessage = "Invalid Ship To Postal Code";
					}
					elseif(trim($flds[3]) == "6830Missing ConsigneePostalCode"){
						$this->error = true;
						$this->errorMessage = "Missing Ship To Postal Code";
					}
					elseif(trim($flds[3]) == "1100The requested service is invalid from the selected origin."){
						$this->error = true;
						$this->errorMessage = "The requested service is invalid from the origin postal code ($originPostalCode).";
					}
					else{
						$this->error = true;
						$this->errorMessage = substr(trim($flds[3]),4,-1);
					}
				}
				else{
					$result[] = $flds;
				}
			}			
		}

		if($this->debug){
			$_Common->debugPrint("$url?$request","UPS Request");
			$_Common->debugPrint($result,"UPS Results");
		}

	return $result;
	}

	// -------------------------------------------------------------------
	function fOpenRequest($url,$request){
		
		global $_Common;
		
		$result = null;
		if(!($myUrl = @fopen("$url?$request","r"))){
			return null;
		}
		while(!feof($myUrl)){
			$row = fgets($myUrl,1024);
			if(strstr($row,"UPSOnLine")){
				$result[] = $row;
			}
		}
		fclose($myUrl);
		return $result;
	}
	
    // -------------------------------------------------------------------
    function sendRequest($url,$request){

		global $_Common;

		$result = null;

		if (!curl_version()) die ("ERROR (UPS Shipping): Cannot contact rate server. cURL not installed.");

		if($this->debug){
			$_Common->debugPrint("Do Curl Request");
		}

		// cURL ENGINE
		$ch = curl_init();
		if($this->debug){
			curl_setopt($ch, CURLOPT_VERBOSE, true);
		}
		curl_setopt($ch, CURLOPT_URL,$url);
		curl_setopt($ch, CURLOPT_HEADER, false);
		curl_setopt($ch, CURLOPT_POST, true);
		curl_setopt($ch, CURLOPT_POSTFIELDS, $request);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);

		$result = curl_exec($ch);
		curl_close ($ch);

		if($this->debug){
			$_Common->debugPrint($result,"Curl Result");
		}

		return $result;
    }
	
}
?>
