<?php

class X_Shipping_Rates extends Shipping_Rates{
	
	var $api = "RateV4";
	var $cfg = null;
	var $debug = false;
	var $totalweight = 0;
	var $totalpackages = 0;
	var $errorMessage = null;
	
	// ----------------------------------------------------	
	function GetShippingRateList($forGoogle = false){ //marcello add variable to get rid of php error

		global $_CF;
		global $_CART;
		global $_Common;
		global $_Config;
		global $_Totals;

		$this->debug = false;

		global $_DB;
		$shipCalcFields = $_DB->getFieldProperties('shipping');
		if(!isset($shipCalcFields['use_decimal_weight'])){
			$_DB->execute("ALTER TABLE `shipping` ADD `use_decimal_weight` ENUM( 'true', 'false' ) NOT NULL DEFAULT 'false'");
			$_DB->execute("INSERT INTO `help` (`section`, `section_help`, `key`, `key_help`) VALUES ('shipping', '', 'use_decimal_weight', 'Use decimal weights for shipping plug-ins? ie. 1.4 lbs.')");
		}

		if($this->debug){
			$_Common->debugPrint($_CART,"Cart");
		}

		// load the config file for usps
		//Marcello for APO load different config file, one that allows priority mail	
		//if (($_SESSION['billaddress_city'] == 'APO') || ($_SESSION['billaddress_city'] == 'apo')){
		//if (($_SESSION['billaddress_city'] == 'APO') || ($_SESSION['billaddress_city'] == 'apo') || ($_SESSION['shipaddress_city'] == 'APO') || ($_SESSION['shipaddress_city'] == 'apo') || ($_SESSION['shipaddress_state'] == 'AA') || ($_SESSION['shipaddress_state'] == 'AE') || ($_SESSION['shipaddress_state'] == 'AP')){
		if (($_SESSION['billaddress_city'] == 'APO') || ($_SESSION['billaddress_city'] == 'apo') || ($_SESSION['billaddress_city'] == 'FPO') || ($_SESSION['billaddress_city'] == 'fpo') || ($_SESSION['billaddress_city'] == 'DPO') || ($_SESSION['billaddress_city'] == 'dpo') || ($_SESSION['shipaddress_city'] == 'APO') || ($_SESSION['shipaddress_city'] == 'apo') || ($_SESSION['shipaddress_city'] == 'FPO') || ($_SESSION['shipaddress_city'] == 'fpo') || ($_SESSION['shipaddress_city'] == 'DPO') || ($_SESSION['shipaddress_city'] == 'dpo') || ($_SESSION['shipaddress_state'] == 'AA') || ($_SESSION['shipaddress_state'] == 'AE') || ($_SESSION['shipaddress_state'] == 'AP')){
			$this->cfg = $_Config->readINIfile("include/shipping/usps/usps.config_apo.php");
		}
		else{
		    $this->cfg = $_Config->readINIfile("include/shipping/usps/usps.config.php");
		}
	
 //	$this->cfg = $_Config->readINIfile("include/shipping/usps/usps.config.php");

		$insurance = $this->getInsurance($_Totals['subtotal']);

		// If we are just picking a different service, use the store values.
		if((isset($_REQUEST['continue']) || isset($_REQUEST['submit_order'])) 
			&& isset($_REQUEST['shipping_method']) && isset($_SESSION['rates'])){
				
			$rates = $_SESSION['rates'];
			$firstRate = $_SESSION['shippingRate'];
			
			foreach($rates as $service=>$postage){
			
				// Express Mail International (EMS) has the parenthesis
				// removed in the REQUEST but not in the service list
				// We will just remove them from any service text so they match

				if(strstr($service,'(')){
					$tmpService = str_replace('(',"",trim($service));
					$tmpService = str_replace(')',"",trim($tmpService));
					// flag selected carrier
					if(trim($_REQUEST['shipping_method']) == trim($tmpService)){
						$firstRate = $postage;
						$this->carrier = trim($service);
					}
				}
				// otherwise just flag selected carrier
				elseif(trim($_REQUEST['shipping_method']) == trim($service)){
					$firstRate = $postage;
					$this->carrier = trim($service);
				}
			}
			return array($rates,$firstRate);
		}

		$xmlFile = "include/shipping/usps/usps.rate.request.xml";
		if(!file_exists($xmlFile)){
			$xmlFile = basename($xmlFile);
			die("<pre><B>usps PROGRAM ERROR:</b> Cannot open xml template: $xmlFile</pre>");
		}
		
		$zipDest = $this->Zip;

		$ResidentialAddress = "1";
		if(isset($customerData['shipaddress_delivery_type']) && $customerData['shipaddress_delivery_type'] == 'commercial'){
			$ResidentialAddress = "0";
		}
		
		// isDomestic
		$isDomestic = true;
		$country = $this->Country;
		if($country != "US"){
			if(isset($this->cfg['country codes'][$country])){
				$country = $this->cfg['country codes'][$country];
				$isDomestic = false;
				$this->api = "IntlRateV2";
			}
		}

		$services = explode(',',$this->cfg['settings']['usps_services']);

		// make shipments by zip code
		$shipments = array();
		$this->buildPackages($shipments);
		
		if($this->debug){
			$_Common->debugPrint($this->totalweight,"Total Shipping Weight");
			$_Common->debugPrint($shipments,"Shipments");
			$_Common->debugPrint($services,"Selected services");
		}

		// weight undefined.
		if($this->totalweight == 0 || count($shipments) == 0){
			$rates[$_CF['shipping']['free_shipping_text']] = $_Common->format_price(0);
			$firstRate = $_Common->format_price(0);
			$_SESSION['rates'] = $rates;
			$_SESSION['shippingRate'] = $firstRate;
			return array($rates,$firstRate);
		}

		$userID = $this->cfg['settings']['usps_user_id'];
		$password = $this->cfg['settings']['usps_password'];
		$id = 0;
		$machinable = $this->cfg['settings']['default_machinable'];

		// load xml file
		error_reporting(E_PARSE|E_WARNING);
		ob_start();
		include_once($xmlFile);
		$xml = ob_get_contents();
		ob_end_clean();
		error_reporting(E_ALL);

		$resultXML = $this->sendRequest($this->api,$xml);

		$this->error = $this->parseResultXML($resultXML,"Error");
		if(count($this->error) > 0){
			$this->errorMessage = $this->error[0]['Description'];
		}	
		
		if($isDomestic){
			$response = $this->parseResultXML($resultXML,"Postage");
		}
		else{
			$response = $this->parseResultXML($resultXML,"Service");
		}

		if($this->debug){
			$this->showXML($xml);
			$_Common->debugPrint($this->error,"Errors");
			$_Common->debugPrint($response,"USPS Response");
		}

		$handling = $this->cfg['settings']['handling_charge'];

		$offer_free_shipping = false;
		$state = $this->State;
		if(isset($this->cfg['free shipping'])){
			extract($this->cfg['free shipping']);
			$statesToExclude = explode(',',$exclude_states);
			foreach($statesToExclude as $s=>$sCode){
				if(trim($sCode) == trim($state)){
					$offer_free_shipping = false;
				}
			}
		}
	
		$rates = array();
		$firstRate = null;
		$serviceCount = array();
		//$delivery = array(); //marcello use delivery times in the future?

		if(count($response) > 0){
			
			foreach($response as $index=>$fields){

				if($isDomestic){
					$service = $fields['MailService'];
					$postage = $fields['Rate'];
				}
				else{
					$service = $fields['SvcDescription'];
					$deliverytime = $fields['SvcCommitments'];
					$postage = $fields['Postage'];
				}
				//fix registered trademark in USPS requrests
				$service = str_replace('&lt;sup&gt;&amp;trade;&lt;/sup&gt;','',$service);
				$service = str_replace('&lt;sup&gt;&amp;reg;&lt;/sup&gt;','',$service);
				$service = str_replace('&lt;sup&gt;&#8482;&lt;/sup&gt;','',$service);
				$service = str_replace('&lt;sup&gt;&#174;&lt;/sup&gt;','',$service);
				
				//remove flat rate envelope for weights > defined by max_weight_flat_rate in CP Shipping - marcello
				//convert totalweight to just ounces			
				$totWeight = round($_Totals['totalWeight'],2);
				$lbs = intval($totWeight);
				//echo 'lbs: ' . $lbs . '<br>';
				$oz = number_format(($totWeight - $lbs) * 16);
				//echo 'oz: ' . $oz . '<br>';
				$tot_oz = $lbs * 16 + $oz;
				if($this->debug){
					echo 'tot_oz: ' . $tot_oz . '<br>';
				}
				if ((($service == "Priority Mail 1-Day Padded Flat Rate Envelope")||($service == "Priority Mail 2-Day Padded Flat Rate Envelope")||($service == "Priority Mail 3-Day Padded Flat Rate Envelope")) && ( $tot_oz > $_CF['shipping']['max_weight_flat_rate'])) {
					continue;
				}
				//if flat rate and weight smaller than the limit then check shopping cart items 
				if ((($service == "Priority Mail 1-Day Padded Flat Rate Envelope")||($service == "Priority Mail 2-Day Padded Flat Rate Envelope")||($service == "Priority Mail 3-Day Padded Flat Rate Envelope")) && ( $tot_oz <= $_CF['shipping']['max_weight_flat_rate'])) {
				  //get the shopping cart skus
				  $totalCartWeightOz = 0;
				  foreach($_CART as $i=>$fields){
					  //check if they are in the exclude list
					  foreach (explode(",", $_CF['shipping']['exclude_skus']) as $j=>$exclude_sku) {
						  //take out spaces
						  $exclude_sku = str_replace(' ', '', $exclude_sku);
						  //exclude all skus that do not fit
						  if ($fields['sku'] == $exclude_sku) {
							  //jump out of all 3 foreach loops
							  continue 3;
						  }
						  // allow up to 4 wedges 
						  if (($fields['sku'] == 'WDG_UNI') && ($fields['quantity'] > 4)) {
							  //jump out of all 3 foreach loops
							  continue 3;
						  }
					  }
				  }
				}

				if(!@$this->cfg['services to display'][$service]){
					continue;
				}

				$isFree = false;
				
				if($offer_free_shipping && $_Totals['subtotal'] >= $free_shipping_subtotal &&
					strtolower($free_shipping_service) == strtolower($service)){

					if($service == $this->cfg['settings']['default_usps_service']){
						$this->cfg['settings']['default_usps_service'] = $free_shipping_text;
					}
					elseif($service == $this->cfg['settings']['default_international_usps_service']){
						$this->cfg['settings']['default_international_usps_service'] = $free_shipping_text;
					}

					$postage = '0.00';
					$service = $free_shipping_text;
					$isFree = true;
				}

				if(isset($rates[$service])){
					// check for free shipping
					$rates[$service] += $_Common->format_price($postage);
				}
				else{
					if($isFree){
						$rates[$service] = $_Common->format_price($postage);
					}
					else{
						// add handling only the first time
						$postage += $handling;
						$rates[$service] = $_Common->format_price($postage);
					}
				}
				$rates[$service] = $_Common->format_price($rates[$service]);
				//@$delivery[$deliverytime]++;//marcello use delivery times in the future?
				@$serviceCount[$service]++;
			}

			asort($rates);

			if($this->debug){
				$_Common->debugPrint($serviceCount,"Service Counts");
				//$_Common->debugPrint($delivery,"Delivery Times");//marcello use delivery times in the future?
			}

			$this->carrier = null;
			$postage = intval(0);
			
			foreach($rates as $firstCarrier=>$postage){

				// kicks out the ones where only partial packages actually got rated
				if($serviceCount[$firstCarrier] != $this->totalpackages){
					unset($rates[$firstCarrier]);
					continue;
				}
				
				// Get only the selected matching carrier
				if(isset($_REQUEST['shipping_method'])){
					if(trim($_REQUEST['shipping_method']) == $firstCarrier){
						$this->carrier = $firstCarrier;
						$firstRate = $postage;
						break;
					}
				}
				elseif(!empty($this->cfg['settings']['default_usps_service']) && trim($this->cfg['settings']['default_usps_service']) == $firstCarrier){
					$this->carrier = $firstCarrier;
					$firstRate = $postage;
					break;
				}
				elseif(!empty($this->cfg['settings']['default_international_usps_service']) && trim($this->cfg['settings']['default_international_usps_service']) == $firstCarrier){
					$this->carrier = $firstCarrier;
					$firstRate = $postage;
					break;
				}
				else{
					// pick the first carrier
					if(is_null($this->carrier)){
						$this->carrier = $firstCarrier;
						$firstRate = $postage;
					}
				}
			}

			if($this->debug){
				$_Common->debugPrint($rates,"USPS Rates");
			}

			// save for refresh on shipping page
			$_SESSION['rates'] = $rates;
			$_SESSION['shippingRate'] = $firstRate;
			//$_SESSION['delivery'] = $delivery;//marcello use delivery times in the future?
			
			return array($rates,$firstRate);
			
		}			
		else{
			if($this->errorMessage){
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
              $width = 0;
              $length = 0;
              $height = 0;
              $girth = 0;

		// This is for the orders report if we make updates
		// to an order we have to recalculate the totals
		if($_isAdmin && !empty($_SESSION['report_cart'])){
			$_CART = $_SESSION['report_cart'];
		}
		
		// defaults
		$pkg = trim($this->cfg['settings']['default_usps_container']);
		$zip = trim($this->cfg['settings']['usps_origin_postal_code']);
		
		$orderWeight = 0;

		if($this->debug){
			//$_Common->debugPrint($_CART,"Cart Data");
		}

		foreach($_CART as $i=>$fields){

			$qty = trim($fields['quantity']);
			$weight = 0;
//            $width = 0;
//            $length = 0;
//            $height = 0;
//            $girth = 0;
			$size = "Regular";
			
			if(!empty($fields['size'])){
				$tempSize = strtolower(trim($fields['size']));
				if($tempSize == "regular" || $tempSize == "large" || $tempSize == "oversize"){
					$size = trim($fields['size']);
				}
			}

			$optionWeight = 0;
			
			// check option weights
			if(isset($fields['options']) && is_array($fields['options']) && count($fields['options']) > 0){
				foreach($fields['options'] as $k=>$flds){
					if(is_numeric($flds['weight']) && $flds['weight'] > 0){
						if($this->debug){
							$_Common->debugPrint($flds['weight'],"option weight");	
						}
						$optionWeight += $flds['weight'];
					}
					//marcello fix for defined packages in options
					// right now we don't need it 
//					elseif(stristr($flds['weight'],":")){
//						list($weight,$tmpsize,$type,$box,$zip) = explode(':',$flds['weight']);
//						if($this->debug){
//							$_Common->debugPrint($weight,"option weight package");	
//						}
//						$optionWeight += $weight;
//					}
				}
			}
			if(!strstr($fields['weight'],":") && $optionWeight > 0){
				//$fields['weight'] += $optionWeight;
				$fields['weight'] = floatval($fields['weight']) + $optionWeight;
			}

			if($this->debug){
				$_Common->debugPrint($fields['weight'],'cart weight');	
			}

			if(!strstr($fields['weight'],":") && trim($fields['weight']) == ""){
				continue;	
			}
			elseif(!strstr($fields['weight'],":") && $fields['weight'] == 0 && $optionWeight == 0){
				continue;
			}
			// Chek for a package being defined
			elseif(strstr($fields['weight'],":")){

				@list($weight,$tmpsize,$type,$box,$zip) = explode(':',$fields['weight']);
				
				if($optionWeight > 0){
					$weight += $optionWeight;	
				}
				
				//$weight = $this->formatWeight($weight);

				if(trim($tmpsize) != ""){
					if(strstr($tmpsize,'x')){
						//marcello for SoxChests: set dimensions to 0 and set zip to default 94587 otherwise USPS tries to ship the chest instead of the options
						if ((strpos($fields['sku'],"CH2") !== false) || (strpos($fields['sku'],"CH3") !== false)) {
							$zip = "94587";
							$width = 0;
							$length = 0;
							$height = 0;
						} else {
							@list($length,$width,$height) = explode('x',strtolower(trim($tmpsize)));
						}
						$girth = ($width + $height + $width + $height);
						if(($length + $girth) <= 84){
							$size = "Regular";
						}
						elseif(($length + $girth) > 84 && ($length + $girth) < 108){
							$size = "Large";
						}
						elseif(($length + $girth) > 108){
							$size = "Oversize";
						}
					}
					else{
						$tmpsize = strtolower(trim($tmpsize));
						if($tmpsize == "regular" || $tmpsize == "large" || $tmpsize == "oversize"){
							$size = $tmpsize;
						}
					}
                }
                
                if(trim($type) == ""){
                    $type = "S";
                }
				if(trim($zip) == ""){
					$zip = trim($this->cfg['settings']['usps_origin_postal_code']);
				}
				
				if(trim($box) != ""){
					//UPS Box - Only 2 boxes allowed now
					if(trim(strtolower($box)) != 'flat rate envelope' && trim(strtolower($box)) != 'flat rate box'){
						$pkg = "RECTANGULAR"; //marcello changed from NONE to RECTANGULAR
					}
					else{
						$pkg = trim($box);
					}
				}
				else{
					$pkg = "RECTANGULAR"; //marcello changed from NONE to RECTANGULAR
				}

                if(strtoupper(substr($type,0,1)) == "M"){
					
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
                            
							$lbs = intval($weight);
							$oz = ceil(($weight - $lbs) * 100);
                            
							$shipment[$zip][] = array(	'lbs'	   => $lbs,
														'width'	   => trim($width),
														'length'   => trim($length),
														'height'   => trim($height),
														'girth'    => trim($girth),
														'oz'	   => $oz,
														'size'	   => trim($size),
														'pkg'	   => trim($pkg));
                            
                            $this->totalpackages++;
                            $this->totalweight += $weight;
                            $remaining -= $pkgqty;
                        }
                    }
					else{
						// make single package with total weight
						$weight = $this->formatWeight($weight * $qty);
						
						$lbs = intval($weight);
						$oz = ceil(($weight - $lbs) * 100);
                        
						$shipment[$zip][] = array(	'lbs'	   => $lbs,
													'width'	   => trim($width),
													'length'   => trim($length),
													'height'   => trim($height),
													'girth'    => trim($girth),
													'oz'	   => $oz,
													'size'	   => trim($size),
													'pkg'	   => trim($pkg));
						$this->totalpackages++;
						$this->totalweight += $weight;

					}
				}
				else{
					// make single packages
					for($i = 1; $i<=$qty; $i++){
						
						$weight = $this->formatWeight($weight);
						
						$lbs = intval($weight);
						$oz = ceil(($weight - $lbs) * 100);
						
						$shipment[$zip][] = array(	'lbs'	   => $lbs,
													'width'	   => trim($width),
													'length'   => trim($length),
													'height'   => trim($height),
													'girth'    => trim($girth),
													'oz'	   => $oz,
													'size'	   => trim($size),
													'pkg'	   => trim($pkg));             
						$this->totalpackages++;
						$this->totalweight += $weight;
					}
				}
			}
			else{
				$orderWeight += $this->formatWeight($fields['weight'] * $qty);
				
				if($this->debug){
					$_Common->debugPrint($fields['weight'] * $qty,"cart weight:");
				}
			}
		}

		if($orderWeight > 0){

			if($this->debug){
				$_Common->debugPrint($orderWeight,"order weight");	
			}

			$weight = $orderWeight;

			// Using the loop weight set in the config, create multiple packages
			// if over 70 lbs or whatever the loopweight is set for the country
			
			$weightLimit = intval(65);
			$country = $this->Country;
			
			if($country != "US" && isset($this->cfg['country codes'][$country])){
				// get the specific country limit
				$fullCountry = $this->cfg['country codes'][$country];
				// get the weight limit
				if(isset($this->cfg['country order package weights'][$fullCountry])){
					$weightLimit = intval($this->cfg['country order package weights'][$fullCountry]);
				}
				
				if($this->debug){
					$_Common->debugPrint($weightLimit,"weight limit for: $fullCountry");	
				}
				
			}
			
			if($weight > $weightLimit){
				$orderLoopWeight = $weight;
				// loop and create multiple packages weighing the loopWeight
				while($orderLoopWeight > $weightLimit){
					$this->addOrderPackage($shipment,$weightLimit);
					$orderLoopWeight -= $weightLimit;
				}
				if($orderLoopWeight > 0){
					if($this->debug){
						$_Common->debugPrint($orderLoopWeight,"remaining order loop weight");	
					}
					$this->addOrderPackage($shipment,$orderLoopWeight);
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
		
		$this->totalweight += $weight;
		$this->totalpackages++;
		
		$size = trim($this->cfg['settings']['default_package_size']);
		$pkg = trim($this->cfg['settings']['default_usps_container']);
		$zip = trim($this->cfg['settings']['usps_origin_postal_code']);
		
		$lbs = intval($weight);
		$oz = ceil(($weight - $lbs) * 100);
		
//		global $_Common;
//		$_Common->debugPrint("weight: $weight, lbs: $lbs, oz: $oz");
		
		$shipment[$zip][] = array(	'lbs'	=> $lbs,
									'oz'	=> $oz,
									'size'	=> trim($size),
									'pkg'	=> trim($pkg));
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
		if($this->debug){
				$_Common->debugPrint($weight,"total weight MBM");	
			} 

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

    # --------------------------------------------------------------
    function getInsurance($subtotal = 0) {
		
		global $_Common, $_Totals;
		
		$insurance = number_format(0,2);

		//$_Common->debugPrint($_SESSION['insurance'],"insurance");

		if(isset($_SESSION['insurance']) || isset($_REQUEST['insurance'])){
			$this->cfg['settings']['add_insurance'] = true;
			$_SESSION['insurance'] = true;
		}
		
		if($this->cfg['settings']['add_insurance']){

			$insuranceRates = $this->cfg['insurance rates'];
			
			foreach($insuranceRates as $index=>$rate){
				list($low,$high,$val) = explode(",",$rate);
					# for subtotals over $600 the insurance fee will be:
					# $7.20 + $1.00 for every $100 or fraction of $100 above $600.
				if($subtotal >= 600 && $low >= 600){
					$remainder = ceil(($subtotal - 600) / 100);
					$insurance = number_format(($val + $remainder),2);
				}
				else{
					if($subtotal >= $low && $subtotal <= $high){
						$insurance = number_format($val,2);
					}
				}
			}			
		}
		
		$_Totals['insurance'] = $insurance;
	
		return $insurance;
	}

	# --------------------------------------------------------------
	function sendRequest($api,$xml){
		
		//Create a cURL instance and retrieve XML response
		if(!is_callable("curl_exec")) die("USPS::submit_request: curl_exec is uncallable");
		$ch = curl_init("https://production.shippingapis.com/ShippingAPI.dll");
		curl_setopt($ch, CURLOPT_POST, 1);
		curl_setopt($ch, CURLOPT_POSTFIELDS, "API=" . $api . "&XML=" . $xml);
		curl_setopt($ch, CURLOPT_TIMEOUT, 60);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);

		$return_xml = curl_exec($ch);
	
	return $return_xml;
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