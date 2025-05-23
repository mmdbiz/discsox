<?php

class Convert{

    var $debug = false;
	
	// ----------------------------------------------------
	function __construct(){
	
	}
	// ----------------------------------------------------
	function DoConversion(&$lineItems,&$lineOptions,$page,$cartName = NULL){
		
		global $_CF;
		global $_Common;

		if($this->debug){
			$_Common->debugPrint("Convert Cart: $cartName");
		}

		if($cartName != "" && method_exists($this,$cartName)){
			$this->$cartName($lineItems,$lineOptions,$page);
		}
		
	}
	
	// ----------------------------------------------------
	function PayPal(&$lineItems,&$lineOptions,$page){

		$sessid = session_id();

		// PayPal fields
		$sku = 1;
		$price = 0;
		$name = "";
		
		if(isset($_REQUEST['item_number'])){
			$sku = $_REQUEST['item_number'];
		}
		if(isset($_REQUEST['item_name'])){
			$name = $_REQUEST['item_name'];
		}
		if(isset($_REQUEST['amount'])){
			$price = $_REQUEST['amount'];
		}

		$lineItem['sku'] = $sku;
		$lineItem['price'] = $price;
		$lineItem['name'] = $name;
		$lineItem['size'] = "";
		$lineItem['weight'] = 0;
		$lineItem['quantity'] = 1;
		$lineItem['page'] = $page;
		$lineItem['sessionid'] = $sessid;
		$lineItems[$sku] = $lineItem;
		
		// Options
		if(!empty($_REQUEST['on0']) && !empty($_REQUEST['os0'])){
			$oData = array();
			$oData['oid'] = 0;
			$oData['odid'] = 0;
			$oData['name'] = trim($_REQUEST['on0']);
			$oData['price'] = 0;
			$oData['value'] = trim($_REQUEST['os0']);
			$oData['weight'] = 0;
			$oData['type'] = "option";
			$lineOptions[$sku][] = $oData;
		}
		if(!empty($_REQUEST['on1']) && !empty($_REQUEST['os1'])){
			$oData = array();
			$oData['oid'] = 0;
			$oData['odid'] = 0;
			$oData['name'] = trim($_REQUEST['on1']);
			$oData['price'] = 0;
			$oData['value'] = trim($_REQUEST['os1']);
			$oData['weight'] = 0;
			$oData['type'] = "option";
			$lineOptions[$sku][] = $oData;
		}
		if(isset($_REQUEST['business']) && trim($_REQUEST['business']) != ""){
			$_SESSION['business'] = trim($_REQUEST['business']);
		}
		if(isset($_REQUEST['no_note']) && trim($_REQUEST['no_note']) != ""){
			$_SESSION['no_note'] = trim($_REQUEST['no_note']);
		}
		if(isset($_REQUEST['currency_code']) && trim($_REQUEST['currency_code']) != ""){
			$_SESSION['currency_code'] = trim($_REQUEST['currency_code']);
		}
	}
	
	// ----------------------------------------------------
	function QuikStore(&$lineItems,&$lineOptions,$page){
		
		global $_Common;
		$sessid = session_id();
		
		foreach($_REQUEST as $key=>$val){

			$qty = 0;
			$lineItem = array();
			
			// HTML items from web page
			if(substr(strtolower($key),0,5) == "item-"){
				$qty = trim($_REQUEST[$key]);
				
				if($qty == ''){
					continue;	
				}
				
				$this->checkQty($qty);
				if($qty >= 1){
					$key = preg_replace("/item-/i","",$key);
					$itemFlds = explode("|",$key);
					$sku = trim($itemFlds[0]);
					$lineItem['sku'] = $sku;
					$lineItem['price'] = str_replace("_",".",trim($itemFlds[1]));
					$lineItem['name'] = str_replace("_"," ",trim($itemFlds[2]));
					$lineItem['size'] = trim(str_replace("_"," ",trim($itemFlds[3])));
					$lineItem['weight'] = str_replace("_",".",trim($itemFlds[4]));
					$lineItem['quantity'] = $qty;
					$lineItem['page'] = $page;
					$lineItem['sessionid'] = $sessid;
					
					$lineItem['is_html_page'] = 'true';
					
					if(!strstr($lineItem['price'],':')){
						$lineItem['price'] = $_Common->format_price($lineItem['price']);
					}
					
					$lineItems[$sku] = $lineItem;
				}
			}
			
			// HTML Options from web page			
			elseif(strtoupper(substr($key,0,7)) == "OPTION|" && trim($val) != ""){
				$oData = array();
				list($itemid,$oData) = $this->convertQuikstoreOption($key,$val);
				$lineOptions[$itemid][] = $oData;
			}
		}
	}

	// -------------------------------------------------------------------
	function convertQuikstoreOption($key,$val){
		
		global $_CF;
		global $_Common;
		$oData = array();
		
		$value = null;
		$price = null;
		$weight = null;
		
		if(strtoupper(substr($val,0,7)) == "INVALID"){
			@list($optionName,$optionPrice,$optionWeight) = explode('|',$val);
			$errorMessage = $_CF['cart']['option_required_message'];
			$errorMessage = preg_replace("|%option_name%|i","$optionPrice",$errorMessage);
			$_Common->sendMessage($errorMessage,true,1);
			exit;
		}
		else{
			// OPTION|1|101
			$optionFlags = array();
			$optionFlags  = explode('|',$key);
			$marker = $optionFlags[0];
			$optionNumber = $optionFlags[1];
			$itemid = $optionFlags[2];

			$optionName = "Option";
			if(!is_numeric($optionNumber)){
				$optionName = trim($optionNumber);
			}

			$optionType = "option";
			if(isset($optionFlags[3])){
				$optionType = $optionFlags[3];
			}
			
			// Frosted|2.95|1
			$optionFlds = array();
			$optionFlds = explode('|',$val);
		
			if(count($optionFlds) == 1){
				$value = trim($optionFlds[0]);
				$price = 0;
				$weight = 0;
			}
			elseif(count($optionFlds) == 2){
				$value = trim($optionFlds[0]);
				$price = trim($optionFlds[1]);
				$weight = 0;
			}
			elseif(count($optionFlds) == 3){
				$value = trim($optionFlds[0]);
				$price = trim($optionFlds[1]);
				$weight = trim($optionFlds[2]);
			}
			
			if($price == ""){
				$price = 0;	
			}
			if($weight == ""){
				$weight = 0;	
			}

			$oData['oid'] = 0;
			$oData['odid'] = 0;
			$oData['name'] = $optionName;
			$oData['price'] = $_Common->format_price($price);
			$oData['value'] = $value;
			$oData['weight'] = $weight;
			$oData['type'] = $optionType;
		}
		
		return array($itemid,$oData);
	}



    // -------------------------------------------------------------------
    function checkQty($qty){
		global $_Common;
        $qty = trim($qty);
        if(!is_numeric($qty)){
			$_Common->sendMessage("You did not enter a valid quantity for the item. Please try again...",true);
			exit;
        }

		// check for fractions        
        $remainder = $qty - intval($qty);
        if($remainder > 0){
			$_Common->sendMessage("You did not enter a valid quantity for the item. Please try again...",true);
			exit;
		}
        
        return true;
    }





}
?>