<?php

class X_Shipping_Rates extends Shipping_Rates{
	
	var $cfg = null;
	var $debug = false;
	var $totalweight = 0;
	var $totalpackages = 0;
	
	// --------------------------------------------------------------
	function __construct(){
		
	}
	// ----------------------------------------------------	
	function GetShippingRateList($forGoogle = false){ //marcello add variable to get rid of php error

		global $_CF;
		global $_CART;
		global $_Common;
		global $_Totals;

		//$this->debug = true;

		global $_DB;
		$shipCalcFields = $_DB->getFieldProperties('shipping');
		if(!isset($shipCalcFields['use_decimal_weight'])){
			$_DB->execute("ALTER TABLE `shipping` ADD `use_decimal_weight` ENUM( 'true', 'false' ) NOT NULL DEFAULT 'false'");
			$_DB->execute("INSERT INTO `help` (`section`, `section_help`, `key`, `key_help`) VALUES ('shipping', '', 'use_decimal_weight', 'Use decimal weights for shipping plug-ins? ie. 1.4 lbs.')");
		}

		// load the config file for UPS
		$cfg = parse_ini_file("include/shipping/ups/ups.config_chests.php",true);
		$this->cfg = $cfg;

		if(isset($_REQUEST['process'])){
			if(isset($_REQUEST['insurance'])){
				$_SESSION['insurance'] = true;
			}
			else{
				unset($_SESSION['insurance']);
			}
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
			if(isset($_SESSION['insuranceRates']) && isset($_SESSION['insuranceRates'][$this->carrier])){
				if($this->debug){
					$_Common->debugPrint($_SESSION['insuranceRates'],"insurance rates");
				}
				$_Totals['insurance'] = $_SESSION['insuranceRates'][$this->carrier];	
			}
			return array($rates,$firstRate);
		}

		$xmlFile = "include/shipping/ups/ups.rate.request_chests.xml";
		if(!file_exists($xmlFile)){
			$xmlFile = basename($xmlFile);
			die("<pre><B>UPS PROGRAM ERROR:</b> Cannot open xml template: $xmlFile</pre>");
		}
		
		// otherwise, send request to UPS
		$country = $this->Country;
		$state = $this->State;
		$zip = $this->Zip;

		$ResidentialAddress = "1";
		if(isset($customerData['shipaddress_delivery_type']) && $customerData['shipaddress_delivery_type'] == 'commercial'){
			$ResidentialAddress = "0";
		}

		// Ship From:
        $orig_country = trim($cfg['settings']['originCountryCode']);
        $orig_zip = trim($cfg['settings']['originPostalCode']);
        $orig_city = trim($cfg['settings']['originCity']);

		// make shipments by zip code
		$shipment = array();
		$this->buildPackages($shipment);
		
		if($this->debug){
			$_Common->debugPrint($_CART);
			$_Common->debugPrint($this->totalweight,"Shipping Weight");
			$_Common->debugPrint($shipment,"packages");
		}

		// weight undefined.
		if($this->totalweight == 0 || count($shipment) == 0){
			$rates[$_CF['shipping']['free_shipping_text']] = $_Common->format_price(0);
			$firstRate = $_Common->format_price(0);
			$_SESSION['rates'] = $rates;
			$_SESSION['shippingRate'] = $firstRate;
			return array($rates,$firstRate);
		}

		$rates = array();
		$firstRate = false;
		// make a call to ups for each shipment based on from zip
		foreach($shipment as $orig_zip=>$packages){

			$xml = NULL;
			error_reporting(E_PARSE|E_WARNING);
			ob_start();
			include($xmlFile);
			$xml = ob_get_contents();
			ob_end_clean();
			error_reporting(E_ALL);

			$xml = str_replace('\?','?',$xml);

			if($this->debug){
				$this->showXML($xml,"XML to send to UPS");	
			}

			$result = $this->sendRequest($xml,$cfg['settings']['RateURL']);

			if($this->debug){
				$this->showXML($result,"XML result from UPS");	
				print "\n\n";
			}
			
			// check for errors
			$records = array();
			$records = $this->parseResultXML($result,"Error");
			if(count($records) > 0){
				$_Common->printErrorMessage("Shipping Rate Error",$records[0]['ErrorDescription']);
			}
	        
			// no errors, continue;
			$records = $this->parseResultXML($result,"RatedShipment");

			$handling = $cfg['settings']['handling_charge'];
				
			if($this->debug){
				$_Common->debugPrint($cfg['services'],"Selected UPS Services");	
			}

			if(count($records) > 0){

				if($this->debug){
					$_Common->debugPrint($records,"UPS Records");	
				}
				
				$service_codes = $cfg['services'];
				$firstRate = false;
				$insuranceRates = array();
				
				
				$offer_free_shipping = false;
				if(isset($cfg['free shipping'])){
					extract($cfg['free shipping']);
				}
				
				
				foreach($records as $index=>$fields){
					$code = intval(trim($fields['Service/Code']));
					
					if(isset($service_codes[$code])){
						$service = $service_codes[$code];
						$postage = 0;
						
						if(isset($_SESSION['insurance']) && isset($fields['RatedPackage/TransportationCharges/MonetaryValue']) && $fields['RatedPackage/TransportationCharges/MonetaryValue'] > 0){
							$charges = explode('|',$fields['RatedPackage/TransportationCharges/MonetaryValue']);
						}
						else{
							if($country == "US" && $orig_country == 'US'){
								$charges = explode('|',$fields['RatedPackage/TotalCharges/MonetaryValue']);
							}
							else{
								if(isset($_SESSION['insurance']) && isset($fields['TransportationCharges/MonetaryValue']) && $fields['TransportationCharges/MonetaryValue'] > 0){
									$charges = explode('|',$fields['TransportationCharges/MonetaryValue']);
								}
								elseif(!empty($fields['TotalCharges/MonetaryValue']) && $fields['TotalCharges/MonetaryValue'] > 0){
									$charges = explode('|',$fields['TotalCharges/MonetaryValue']);
								}
								elseif(!empty($fields['RatedPackage/TotalCharges/MonetaryValue']) && $fields['RatedPackage/TotalCharges/MonetaryValue'] > 0){
									$charges = explode('|',$fields['RatedPackage/TotalCharges/MonetaryValue']);
								}
							}
						}
						
						foreach($charges as $j=>$rate){
							$postage += $rate;	
						}
						$postage += $handling;
						
						// check for free shipping
						if($offer_free_shipping && $_Totals['subtotal'] >= $free_shipping_subtotal &&
						   strtolower($free_shipping_service) == strtolower($service)){

							$blnOk = true;
							$statesToExclude = explode(',',$exclude_states);
							foreach($statesToExclude as $s=>$sCode){
								if(trim($sCode) == trim($state)){
									$blnOk = false;
								}
							}
							if($blnOk){
								$postage = '0.00';
								$service = $free_shipping_text;
							}
						}

						if(isset($fields['RatedPackage/ServiceOptionsCharges/MonetaryValue']) && $fields['RatedPackage/ServiceOptionsCharges/MonetaryValue'] > 0){
							$insuranceCharges = explode('|',$fields['RatedPackage/ServiceOptionsCharges/MonetaryValue']);
						}
						elseif(isset($fields['ServiceOptionsCharges/MonetaryValue'])){
							$insuranceCharges = explode('|',$fields['ServiceOptionsCharges/MonetaryValue']);
						}
					
						$insurance = 0;
						foreach($insuranceCharges as $k=>$irate){
							$insurance += $irate;	
						}

						// All additional charges, including things like overweight and insurance charges,
						// are all in the same XML field. So, we check for the session[insurance]
						// flag to see if it's set and show it as insurance. Otherwise, add
						// it to the postage charge.

						if(isset($_SESSION['insurance'])){
							if(isset($insuranceRates[$service])){
								$insuranceRates[$service] += $_Common->format_price($insurance);
							}
							else{
								$insuranceRates[$service] = $_Common->format_price($insurance);
							}
						}
						else{
							$postage += $insurance;
						}


						if(isset($rates[$service])){
							$rates[$service] += $_Common->format_price($postage);
						}
						else{
							$rates[$service] = $_Common->format_price($postage);
						}
						
						// Get only the selected matching carrier
						if(isset($_REQUEST['shipping_method'])){
							if(trim($_REQUEST['shipping_method']) == trim($service)){
								$firstRate = $postage;
								$this->carrier = trim($service);
							}
						}
					}
				}
			}
		} // endforeach shipment


		asort($rates);
		
		if($this->debug){
			$_Common->debugPrint($rates,"UPS Shipping Rates");	
		}
	
		if(!$firstRate){
			// pick the first carrier
			foreach($rates as $firstCarrier=>$firstRate){
				$this->carrier = $firstCarrier;
				break;
			}
		}
		
		// save for refresh on shipping page
		$_SESSION['rates'] = $rates;
		$_SESSION['shippingRate'] = $firstRate;

		$_SESSION['insuranceRates'] = $insuranceRates;
		if(isset($insuranceRates[$this->carrier])){
			if($this->debug){
				$_Common->debugPrint($_SESSION['insuranceRates'][$this->carrier],"insurance rate");
			}
			$_Totals['insurance'] = $insuranceRates[$this->carrier];	
		}
		
		return array($rates,$firstRate);
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
	//---------------------------------------------------------------

	function get_prod_row_by_sku($currentsku){
		
		$link = mysqli_connect(DB_HOST, DB_USERNAME, DB_PASSWORD);
		//$query = sprintf("SELECT * FROM products WHERE sku = '%s'", mysql_real_escape_string($currentsku) );
		$query = sprintf("SELECT * FROM products WHERE sku = '%s'", mysqli_real_escape_string($link, $currentsku) );
		// Perform Query
		//$result = mysql_query($query);
		//$row = mysql_fetch_array($result);
		$result = mysqli_query($link, $query);
		$row = mysqli_fetch_array($result);
		return $row;
	}
		// packaging
		list($PackagingCode,$PackagingDesc) = explode(',',$this->cfg['settings']['PackagingType']);
		
		$orderWeight = 0;
		//use cart content to build package
//		foreach($_CART as $i=>$fields){
		
// values for calculating shipping for the chests marcello fix this
// get this from the SKU
			$current_SKU = $_SESSION['current_SKU'];
			 // Get the product data			
			$qty = $_SESSION['current_qty'];
			$weight = $_SESSION['current_weight'];
			$size = $_SESSION['current_size'];
			$length = $_SESSION['current_length'];
			$width = $_SESSION['current_width'];
			$height = $_SESSION['current_height'];
			$declaredValue = $_SESSION['current_declaredValue'];
			$optionWeight = $_SESSION['current_optionWeight'];

			
			// check option weights
			//if(isset($fields['options']) && is_array($fields['options']) && count($fields['options']) > 0){
//				foreach($fields['options'] as $k=>$flds){
//					if(is_numeric($flds['weight']) && $flds['weight'] > 0){
//						$optionWeight += $flds['weight'];
//					}
//				}
//			}
//			if(!strstr($fields['weight'],":") && $optionWeight > 0){
			// Marcello make option weights dependet of item qty (required for build a kit)
//				$fields['weight'] += $optionWeight;
			// Marcello make option weights independet of item qty
			//	$fields['weight'] += ($optionWeight / $qty);
//			}

//			if($this->debug){
//				$_Common->debugPrint($fields['weight'],'cart weight');	
//			}

//			if(!strstr($fields['weight'],":") && trim($fields['weight']) == ""){
//				continue;	
//			}
//			elseif(!strstr($fields['weight'],":") && $fields['weight'] == 0 && $optionWeight == 0){
//				continue;
//			}
//			elseif(strstr($fields['weight'],":")){
//
//				@list($weight,$size,$type,$box,$zip) = explode(':',$fields['weight']);
//				
//				if($optionWeight > 0){
//					$weight += $optionWeight;	
//				}
//
//				if(trim($size) != ""){
//                    $size = strtolower(trim($size));
//                }
//                else{
//					if(!empty($fields['size'])){
//						$size = strtolower(trim($fields['size']));
//					}
//				}
//                
//                if(strstr($size,'x')){
//					list($length,$width,$height) = explode('x',strtolower(trim($size)));	
//				}
                
//                if(trim($type) == ""){
                    $type = "S";
//                }
//				if(trim($zip) == ""){
					$zip = trim($this->cfg['settings']['originPostalCode']);
//				}
				
				// default package settings
				$pkgCode = $PackagingCode;
				$pkgDesc = $PackagingDesc;
				
//				if(trim($box) != "" && strtolower($box) != 'none'){
//					@list($pkgCode,$pkgDesc) = explode(',',$box);
//				}
//				elseif($pkgCode == ''){
					$pkgCode = '02';
					$pkgDesc = 'Package';
//				}

//                if(strtoupper(substr($type,0,1)) == "M"){
//					
//					// multi-packages
//                    @list($flag,$numperbox) = explode('-',$type);
//                   
//                    if(is_numeric($numperbox) && $numperbox > 0){
//						
//                        $pkgweight = $weight;
//                        $numboxes = ceil($qty / $numperbox);
//                        $remaining = $qty;
//
//                        for($i = 1; $i<=$numboxes; $i++){
//							
//                            $pkgqty = $numperbox;
//                            if($i == $numboxes){
//                                $pkgqty = $remaining;
//                            }
//                            
//                            $weight = $this->formatWeight($pkgweight * $pkgqty);
//
//							if($weight < 1){
//								$weight = 1.0;
//							}
//                            
//							$shipment[$zip][] = array(	'weight'   => $weight,
//														'length'   => trim($length),
//														'width'	   => trim($width),
//														'height'   => trim($height),
//														'pkgcode'  => trim($pkgCode),
//														'pkgdesc'  => trim($pkgDesc),
//														'declaredValue' => $declaredValue);
//                            
//                            $this->totalpackages++;
//                            $this->totalweight += $weight;
//                            $remaining -= $pkgqty;
//                        }
//                    }
//					else{
						// make single package with total weight
//                        $weight = $this->formatWeight($weight * $qty);

						if($weight < 1){
							$weight = 1.0;
						}
                        
						$shipment[$zip][] = array('weight'   => $weight,
												  'length'   => trim($length),
												  'width'	 => trim($width),
												  'height'   => trim($height),
												  'pkgcode'  => trim($pkgCode),
												  'pkgdesc'  => trim($pkgDesc),
												  'declaredValue' => $declaredValue);
	                    
						$this->totalpackages++;
						$this->totalweight += $weight;
//					}
//				}
//				else{
//					// make single packages
//					for($i = 1; $i<=$qty; $i++){
//						$weight = $this->formatWeight($weight);
//						
//						if($weight < 1){
//							$weight = 1.0;
//						}
//						
//						$shipment[$zip][] = array('weight'   => $weight,
//												  'length'   => trim($length),
//												  'width'	 => trim($width),
//												  'height'   => trim($height),
//												  'pkgcode'  => trim($pkgCode),
//												  'pkgdesc'  => trim($pkgDesc),
//												  'declaredValue' => $declaredValue);
//	                    
//						$this->totalpackages++;
//						$this->totalweight += $weight;
//					}
//				}
//			}
//			else{
//				$orderWeight += $this->formatWeight($fields['weight'] * $qty);
//			}
//		}
		//end und cart content
		if($orderWeight > 0){
			
			$weight = $orderWeight;
			
            $this->totalweight += $weight;

			if($weight < 1){
				$weight = 1.0;
			}
			
			if($weight > 150){
				$loopWeight = $weight;
				// loop and create multiple packages weighing 150 lbs
				while($loopWeight > 150){
					$this->addOrderPackage($shipment,150,$declaredValue);
					$loopWeight -= 150;
				}
				// get any remaining packages
				if($loopWeight > 0){
					
					if($loopWeight < 1){
						$loopWeight = 1.0;
					}
					
					$this->addOrderPackage($shipment,$loopWeight,$declaredValue);
				}
			}
			else{
				$this->addOrderPackage($shipment,$weight,$declaredValue);
			}
		}
		
		$_SESSION['number_of_packages'] = $this->totalpackages;
		
	}

	//---------------------------------------------------------------
	function addOrderPackage(&$shipment,$weight,$value){
	
		$zip = trim($this->cfg['settings']['originPostalCode']);
		list($PackagingCode,$PackagingDesc) = explode(',',$this->cfg['settings']['PackagingType']);
		
		$shipment[$zip][] = array('weight'   => $weight,
								  'length'   => trim($this->cfg['settings']['default_package_length']),
								  'width'	 => trim($this->cfg['settings']['default_package_width']),
								  'height'   => trim($this->cfg['settings']['default_package_height']),
								  'pkgcode'  => trim($PackagingCode),
								  'pkgdesc'  => trim($PackagingDesc),
								  'declaredValue' => $value);
		$this->totalpackages++;
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
    function sendRequest($xml,$url){

		global $_Common;

		$resultXML = "";

		if (!curl_version()) die ("ERROR (UPS Shipping): Cannot contact rate server. cURL not installed.");

		if($this->debug){
			$_Common->debugPrint("Do Curl Request");
		}

		// cURL ENGINE
		$ch = curl_init();
		if($this->debug){
			curl_setopt($ch, CURLOPT_VERBOSE, true);
		}
		curl_setopt($ch, CURLOPT_URL,"$url");
		curl_setopt($ch, CURLOPT_HEADER, false);
		curl_setopt($ch, CURLOPT_POST, true);
		curl_setopt($ch, CURLOPT_POSTFIELDS, "$xml");
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);

		$resultXML = curl_exec($ch);
		curl_close ($ch);

		if($this->debug){
			$_Common->debugPrint($this->showXML($resultXML),"Curl Result");
		}

		return $resultXML;
    }

    // -------------------------------------------------------------------
    function parseResultXML($xml,$searchKey){
        include_once("include/xml.search.inc");
        $search = new XMLSearch($searchKey);
        $records = $search->search($xml);
        return $records;
    }

    // -------------------------------------------------------------------
    function showXML($xml){
        $xml = preg_replace("/</","&lt;",$xml);
        $xml = preg_replace("/>/","&gt;",$xml);
        print "<pre>$xml</pre>";
    }
	
}

?>