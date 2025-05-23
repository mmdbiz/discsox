<?php

// Define UPSOAuth class first
class UPSOAuth {
	private $config;
	private $access_token;
	private $token_expiry;
	private $debug;
	
	public function __construct($config, $debug = false) {
		$this->config = $config;
		$this->debug = $debug;
		$this->access_token = null;
		$this->token_expiry = 0;
	}
	
	public function getAccessToken() {
		if ($this->access_token && time() < $this->token_expiry) {
			return $this->access_token;
		}
		return $this->refreshAccessToken();
	}
	
	private function refreshAccessToken() {
		$credentials = base64_encode($this->config['oauth_settings']['client_id'] . ':' . $this->config['oauth_settings']['client_secret']);
		
		$ch = curl_init();
		curl_setopt($ch, CURLOPT_URL, 'https://onlinetools.ups.com/security/v1/oauth/token');
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($ch, CURLOPT_POST, true);
		curl_setopt($ch, CURLOPT_POSTFIELDS, 'grant_type=client_credentials');
		curl_setopt($ch, CURLOPT_HTTPHEADER, [
			'Authorization: Basic ' . $credentials,
			'Content-Type: application/x-www-form-urlencoded'
		]);
		
		$response = curl_exec($ch);
		$httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
		curl_close($ch);
		
		if ($httpCode !== 200) {
			throw new Exception('Failed to obtain OAuth token: ' . $response);
		}
		
		$tokenData = json_decode($response, true);
		$this->access_token = $tokenData['access_token'];
		$this->token_expiry = time() + ($tokenData['expires_in'] ?? 3600) - 60;
		
		return $this->access_token;
	}
}

class X_Shipping_Rates extends Shipping_Rates{
	
	var $cfg = null;
	var $debug = false;
	var $totalweight = 0;
	var $totalpackages = 0;
	private $oauth = null;
	
	// ----------------------------------------------------	
	function GetShippingRateList($forGoogle = false){ //marcello add variable to get rid of php error
		global $_CF;
		global $_CART;
		global $_Common;
		global $_Totals;	
		$this->debug = false;
		global $_DB;
		$shipCalcFields = $_DB->getFieldProperties('shipping');
		if(!isset($shipCalcFields['use_decimal_weight'])){
			$_DB->execute("ALTER TABLE `shipping` ADD `use_decimal_weight` ENUM( 'true', 'false' ) NOT NULL DEFAULT 'false'");
			$_DB->execute("INSERT INTO `help` (`section`, `section_help`, `key`, `key_help`) VALUES ('shipping', '', 'use_decimal_weight', 'Use decimal weights for shipping plug-ins? ie. 1.4 lbs.')");
		}
		// load the config file for UPS
		$cfg = parse_ini_file("include/shipping/ups/ups.config.php",true);
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
		$xmlFile = "include/shipping/ups/ups.rate.request.xml";
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
		//$insuranceRates = array();
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
			    echo("Rate URL: https://onlinetools.ups.com/api/rating/v1/Shop");
			}

			if ($this->debug) {
				$_Common->debugPrint($shipment, "Shipment Data Before Request");
			}

			$result = $this->sendRequest($xml, "https://onlinetools.ups.com/api/rating/v1/Shop");

			if($this->debug){
				$this->showXML($result,"XML result from UPS");	
				print "\n\n";
			}
			
			// check for errors
			$response = json_decode($result, true);
			if (isset($response['response']['errors'])) {
				$errorMessage = $response['response']['errors'][0]['message'];
				$errorCode = $response['response']['errors'][0]['code'];
				$_Common->printErrorMessage("Shipping Rate Error", "Code: $errorCode - $errorMessage");
				return array(array(), false);
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
				if($this->debug){
					echo "offer_free_shipping: " . $offer_free_shipping;
				}
				
				foreach($records as $index=>$fields){
					$code = intval(trim($fields['Service/Code']));
					
					if(isset($service_codes[$code])){
						$service = $service_codes[$code];
						$postage = 0;
						
						// Get the total charges
						if(isset($fields['TotalCharges/MonetaryValue']) && $fields['TotalCharges/MonetaryValue'] > 0){
							$postage = floatval($fields['TotalCharges/MonetaryValue']);
						}

						// Add handling charge
						$postage += $handling;
						
						// Check for free shipping
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
								if($this->debug){
									$_Common->debugPrint($_Totals['subtotal'],"got free shipping for ");
								}
							}
						}

						// Handle insurance if needed
						if(isset($_SESSION['insurance']) && isset($fields['ServiceOptionsCharges/MonetaryValue'])){
							$insurance = floatval($fields['ServiceOptionsCharges/MonetaryValue']);
							if(isset($insuranceRates[$service])){
								$insuranceRates[$service] += $_Common->format_price($insurance);
							}
							else{
								$insuranceRates[$service] = $_Common->format_price($insurance);
							}
						}

						// Add to rates array
						if(isset($rates[$service])){
							$rates[$service] += $_Common->format_price($postage);
						}
						else{
							$rates[$service] = $_Common->format_price($postage);
						}
						
						// Get selected carrier rate
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

		// packaging
		list($PackagingCode,$PackagingDesc) = explode(',',$this->cfg['settings']['PackagingType']);
		
		$orderWeight = 0;
		
		foreach($_CART as $i=>$fields){

			$qty = trim($fields['quantity']);
			$weight = 0;
			$size = "";
			$length = null;
			$width = null;
			$height = null;
			$declaredValue = number_format($fields['line_total'] * $qty,2,'.','');
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
			// Marcello make option weights dependet of item qty (required for build a kit)
				//$fields['weight'] += $optionWeight;
				$fields['weight'] = floatval($fields['weight']) + $optionWeight;
			// Marcello make option weights independet of item qty
			//	$fields['weight'] += ($optionWeight / $qty);
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
			elseif(strstr($fields['weight'],":")){

				@list($weight,$size,$type,$box,$zip) = explode(':',$fields['weight']);
				
				if($this->debug){
					$_Common->debugPrint($weight,'item weight');
					$_Common->debugPrint($optionWeight,'option weight');	
				}
				//marcello if item weight = 0.00 ignore everything in the item weight, e.g. dimensions etc.
				if($weight == '0.00'){
					if($this->debug){
						$_Common->debugPrint('got here');	
					}
				  $size = "";
				  $length = null;
				  $width = null;
				  $height = null;
				  $zip = trim($this->cfg['settings']['originPostalCode']);
				}
				if($optionWeight > 0){
					$weight += $optionWeight;	
				}

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
				
				if(trim($box) != "" && strtolower($box) != 'none'){
					@list($pkgCode,$pkgDesc) = explode(',',$box);
				}
				elseif($pkgCode == ''){
					$pkgCode = '02';
					$pkgDesc = 'Package';
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

							if(($weight < 1)&&($weight > 0)){
								$weight = 1.0;
							}
                            
							$shipment[$zip][] = array(	'weight'   => $weight,
														'length'   => trim($length),
														'width'	   => trim($width),
														'height'   => trim($height),
														'pkgcode'  => trim($pkgCode),
														'pkgdesc'  => trim($pkgDesc),
														'declaredValue' => $declaredValue);
                            
                            $this->totalpackages++;
                            $this->totalweight += $weight;
                            $remaining -= $pkgqty;
                        }
                    }
					else{
						// make single package with total weight
                        $weight = $this->formatWeight($weight * $qty);

						if(($weight < 1)&&($weight > 0)){
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
					}
				}
				else{
					// make single packages
					for($i = 1; $i<=$qty; $i++){
						$weight = $this->formatWeight($weight);
						
						if($this->debug){
							$_Common->debugPrint($weight,'weight for UPS');	
						}
						
						if($weight < 1){
//							$weight = 1.0;
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
					}
				}
			}
			else{
				$orderWeight += $this->formatWeight($fields['weight'] * $qty);
			}
		}
		
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
    function sendRequest($data, $url) {
        global $_Common;

        if (!$this->oauth) {
            $this->oauth = new UPSOAuth($this->cfg, $this->debug);
        }

        try {
            $token = $this->oauth->getAccessToken();
        } catch (Exception $e) {
            $_Common->printErrorMessage("UPS OAuth Error", $e->getMessage());
            return false;
        }

        if ($this->debug) {
            $_Common->debugPrint($data, "Raw XML Input");
        }

        // Clean up the XML by removing multiple XML declarations
        $cleanXml = preg_replace('/<\?xml.*?\?>\s*/', '', $data);
        $cleanXml = '<?xml version="1.0"?>' . $cleanXml;

        // Extract the RatingServiceSelectionRequest part
        if (preg_match('/<RatingServiceSelectionRequest.*?>(.*?)<\/RatingServiceSelectionRequest>/s', $cleanXml, $matches)) {
            $ratingRequest = $matches[0];
        } else {
            $_Common->printErrorMessage("XML Error", "Could not find RatingServiceSelectionRequest");
            return false;
        }

        // Parse the cleaned XML
        $xmlObj = simplexml_load_string($ratingRequest);
        
        if ($this->debug) {
            $_Common->debugPrint($xmlObj, "Parsed XML Object");
        }

        // Build packages array from XML
        $packages = [];
        if (isset($xmlObj->Shipment->Package)) {
            foreach ($xmlObj->Shipment->Package as $pkg) {
                if ($this->debug) {
                    $_Common->debugPrint($pkg, "Processing Package");
                }

                $package = [
                    "PackagingType" => [
                        "Code" => (string)$pkg->PackagingType->Code,
                        "Description" => (string)$pkg->PackagingType->Description
                    ],
                    "PackageWeight" => [
                        "UnitOfMeasurement" => [
                            "Code" => "LBS",
                            "Description" => "Pounds"
                        ],
                        "Weight" => (string)$pkg->PackageWeight->Weight
                    ]
                ];

                // Add dimensions if they exist
                if (isset($pkg->Dimensions)) {
                    $package["Dimensions"] = [
                        "UnitOfMeasurement" => [
                            "Code" => "IN",
                            "Description" => "Inches"
                        ],
                        "Length" => (string)$pkg->Dimensions->Length,
                        "Width" => (string)$pkg->Dimensions->Width,
                        "Height" => (string)$pkg->Dimensions->Height
                    ];
                }

                $packages[] = $package;
            }
        }

        if ($this->debug) {
            $_Common->debugPrint($packages, "Final Packages Array");
        }

        // Build the REST API request
        $rateRequest = [
            "RateRequest" => [
                "Request" => [
                    "RequestOption" => "Shop",
                    "TransactionReference" => [
                        "CustomerContext" => "Rating and Service"
                    ]
                ],
                "Shipment" => [
                    "Shipper" => [
                        "Name" => "Shipper Name",
                        "Address" => [
                            "AddressLine" => "",
                            "City" => (string)$xmlObj->Shipment->Shipper->Address->City,
                            "StateProvinceCode" => "",
                            "PostalCode" => (string)$xmlObj->Shipment->Shipper->Address->PostalCode,
                            "CountryCode" => (string)$xmlObj->Shipment->Shipper->Address->CountryCode
                        ]
                    ],
                    "ShipTo" => [
                        "Name" => "Ship To Name",
                        "Address" => [
                            "AddressLine" => "",
                            "City" => "",
                            "StateProvinceCode" => $this->State,
                            "PostalCode" => (string)$xmlObj->Shipment->ShipTo->Address->PostalCode,
                            "CountryCode" => (string)$xmlObj->Shipment->ShipTo->Address->CountryCode ?: "US",
                            "ResidentialAddressIndicator" => "true"
                        ]
                    ],
                    "ShipFrom" => [
                        "Name" => "Ship From Name",
                        "Address" => [
                            "AddressLine" => "",
                            "City" => (string)$xmlObj->Shipment->Shipper->Address->City,
                            "StateProvinceCode" => "",
                            "PostalCode" => (string)$xmlObj->Shipment->Shipper->Address->PostalCode,
                            "CountryCode" => (string)$xmlObj->Shipment->Shipper->Address->CountryCode
                        ]
                    ],
                    "Service" => [
                        "Code" => (string)$xmlObj->Shipment->Service->Code,
                        "Description" => "Ground"
                    ],
                    "Package" => $packages
                ]
            ]
        ];

        if ($this->debug) {
            $_Common->debugPrint($rateRequest, "Rate Request Data");
        }

        $ch = curl_init();
        if ($this->debug) {
            curl_setopt($ch, CURLOPT_VERBOSE, true);
        }

        curl_setopt($ch, CURLOPT_URL, "https://onlinetools.ups.com/api/rating/v1/Shop");
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_POST, true);
        curl_setopt($ch, CURLOPT_POSTFIELDS, json_encode($rateRequest));
        curl_setopt($ch, CURLOPT_HTTPHEADER, [
            'Authorization: Bearer ' . $token,
            'Content-Type: application/json',
            'Accept: application/json'
        ]);

        $result = curl_exec($ch);
        $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);

        if ($this->debug) {
            $_Common->debugPrint([
                'http_code' => $httpCode,
                'request' => json_encode($rateRequest, JSON_PRETTY_PRINT),
                'response' => $result
            ], "API Response");
        }

        curl_close($ch);

        if ($httpCode !== 200) {
            $_Common->printErrorMessage("UPS API Error", "Failed to get rates: " . $result);
            return false;
        }

        return $result;
    }

    // -------------------------------------------------------------------
    function parseResultXML($response, $searchKey) {
        $data = json_decode($response, true);
        
        if ($searchKey === "Error" && isset($data['response']['errors'])) {
            return $data['response']['errors'];
        }
        
        if ($searchKey === "RatedShipment" && isset($data['RateResponse']['RatedShipment'])) {
            $rates = [];
            foreach ($data['RateResponse']['RatedShipment'] as $rate) {
                $rates[] = [
                    'Service/Code' => $rate['Service']['Code'],
                    'TotalCharges/MonetaryValue' => $rate['TotalCharges']['MonetaryValue'],
                    'ServiceOptionsCharges/MonetaryValue' => 
                        isset($rate['ServiceOptionsCharges']) ? $rate['ServiceOptionsCharges']['MonetaryValue'] : '0.00'
                ];
            }
            return $rates;
        }
        
        return [];
    }

    // -------------------------------------------------------------------
    function showXML($xml){
        $xml = preg_replace("/</","&lt;",$xml);
        $xml = preg_replace("/>/","&gt;",$xml);
        print "<pre>$xml</pre>";
    }
	
}

?>