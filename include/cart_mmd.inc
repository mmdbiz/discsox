<?php

// fix 151, shipping by item
// add to cart, check fields line 99. Match to config fields


// -----------------------------------------------------------------------
//         Class: Cart
//       Purpose: This class controls the cart functionality.
// Last Modified: March 08, 2006
//
// -----------------------------------------------------------------------
class Cart{

    var $debug = false;
    var $cartFields = array();
    var $cartDetailFields = array();
    var $haveInventory = false;
    var $payPal = false;
    var $qs212 = false;
    var $pidList = array();
    var $results = "";
    var $relatedItems = array();
    var $googleCheckoutButton = null;
    
	function Cart(){
		$this->getCart();
		global $_Registry;
		$this->haveInventory = $_Registry->file_exists_incpath("inventory.inc");
	}

	//-----------------------------------------------------
	// functions that run when we add, modify, or view the cart
	//----------------------------------------------------- 
	function initialize(){
		
		global $_Common,$_CF;
		
		// PayPal product pages
		if(isset($_REQUEST['cmd']) && ($_REQUEST['cmd'] = "_xclick" || $_REQUEST['cmd'] = "_cart")){
			if(!empty($_REQUEST['display'])){
				unset($_REQUEST['cmd']);
				$_REQUEST['view'] = 1;
			}
			else{
				$this->payPal = true;
				$_REQUEST['add'] = 1;	
			}
		}
		// QuikStore 2.12 product pages
		foreach($_REQUEST as $k=>$v){
			if(substr($k,0,5) == "item-" && strstr($k,"|")){
				$this->qs212 = true;
				break;	
			}	
		}

		// return page
		if(empty($_REQUEST['page']) && isset($_SERVER['HTTP_REFERER']) && !strstr($_SERVER['HTTP_REFERER'],'cart.php')){
			$_REQUEST['page'] = $_SERVER['HTTP_REFERER'];
		}

		// clean up frontpage html rewriting from 2.12 pages
		$functions = array("add_to_cart","modify_cart","view_cart","page");
		foreach($_REQUEST as $key=>$value){
			foreach($functions as $i=>$function){
				if(substr($key,0,strlen($function)) == $function && strlen($key) > strlen($function)){
					unset($_REQUEST[$key]);
					$_REQUEST[$function] = $value;
					break;
				}
			}
		}

		// then, figure out what to do
		$run = false;
		foreach($_REQUEST as $key=>$value){
			switch($key){
				case "add":
				case "add_x":
				case "add_to_cart":
				case "add_to_cart_x":
					$this->add();
//					if(!empty($_REQUEST['page'])){
//						$_Common->redirect($_REQUEST['page']);
//					}
//					else{
						$_Common->redirect("cart.php?view=1");
//					}
					$run = true;
					break;
				case "modify":
				case "modify_x":
				case "modify_cart":
				case "modify_cart_x":
					$this->modify();
					$_Common->redirect("cart.php?view=1");
					$run = true;
					break;
				case "view":
				case "view_x":
				case "view_cart":
				case "view_cart_x":
					$this->LoadCart();
					$run = true;
					break;
			}
			if($run){
				break;	
			}
		}
		if(!$run){
			$this->LoadCart();
		}		
	}

	//-----------------------------------------------------
    function getCart(){

		global $_CART;
		global $_Common;
		global $_DB;
		$sessid = session_id();
		$_CART = array();

		$this->cartFields = $_DB->getFieldProperties('carts');
		$this->cartDetailFields = $_DB->getFieldProperties('cart_details');

		if(!isset($this->cartFields['category'])){
			$_DB->execute("ALTER TABLE `carts` ADD `category` VARCHAR( 255 ) NULL");
		}

		if(!isset($this->cartFields['is_html_page'])){
			$_DB->execute("ALTER TABLE `carts` ADD `is_html_page` ENUM( 'true', 'false' ) NOT NULL DEFAULT 'false'");
		}
		
		
		$sql = "SELECT * FROM carts WHERE sessionid = '$sessid' ORDER BY cartid";
		$rs = $_DB->execute($sql);
		
		if($_DB->numrows($rs) > 0){
			while($row = $_DB->fetchrow($rs, "ASSOC")){
				$cartid = $row['cartid'];
				$this->pidList[$row['sku']] = $row['sku'];
				
				foreach($row as $name=>$value){
					$row[$name] = stripslashes($value);	
				}
				
				// Cart details
				$row['options'] = array();
				$detailSql = "SELECT * FROM cart_details WHERE cartid = '$cartid' ORDER BY cdid";
				$drs = $_DB->execute($detailSql);
				if($_DB->numrows($drs) > 0){
					while($drow = $_DB->fetchrow($drs, "ASSOC")){
						foreach($drow as $oName=>$oVal){
							$drow[$oName] = stripslashes($oVal);	
						}
						$row['options'][] = $drow;
					}
					$_DB->free_result($drs);
				}
				
				$_CART[$cartid] = $row;
			}
		}
    }


	//-----------------------------------------------------
	function add(){
		
		global $_CART;
		global $_Common;
		global $_CF;
		global $_DB;
		global $_Registry;
		$sessid = session_id();
		
		if($this->debug){
			$_Common->debugPrint("Add to Cart");
		}
		
		
		if(!$this->payPal && !$this->qs212){
			$this->convertFormFields();
		}

		$page = "";
		if(!empty($_REQUEST['page'])){
			$page = $_REQUEST['page'];
			$_SESSION['last_page'] = $page;
		}
		
		$lineItems = array();
		$lineOptions = array();

		// Convert form code from another cart to our format			
		if($_CF['cart']['convert_cart_html_code'] || $this->payPal || $this->qs212){
			$convert = $_Registry->LoadClass('Convert');
			$cartName = NULL;
			if($this->payPal){
				$cartName = "PayPal";
			}
			elseif($this->qs212){
				$cartName = "QuikStore";
			}
			else{
				if(!empty($_CF['cart']['cart_name_to_convert'])){
					$cartName = trim($_CF['cart']['cart_name_to_convert']);
				}
			}
			if($cartName){
				$convert->DoConversion($lineItems,$lineOptions,$page,$cartName);
			}
		}

		if(!empty($_REQUEST['item']) && is_array($_REQUEST['item'])){
			
			// Read default form fields and build cart data fields
			foreach($_REQUEST['item'] as $sku=>$fields){

				$qty = 0;
				$lineItem = array();

				if(isset($fields['quantity']) && is_numeric($fields['quantity']) && intval($fields['quantity']) > 0){

					$qty = $fields['quantity'];

					if(empty($fields['name'])){
						$record = $this->lookupItem($sku,$qty);
						if(count($record) > 0){
							//$_Common->debugPrint($record,'db record');
							$fields = array_merge($fields,$record);
						}
						else{
							$_Common->sendMessage("Could not find item number $sku in the database",true,1);
							exit;
						}
					}
				
					$lineItem['sku'] = $sku;
					foreach($fields as $key=>$value){
						//if(isset($this->cartFields[$key])){
						$lineItem[$key] = $value;
						//}
					}
					$lineItem['page'] = $page;
					$lineItem['sessionid'] = $sessid;
					$lineItems[$sku] = $lineItem;
					
					if($_CF['cart']['verify_price']){
						$this->verifyPrice($lineItem);
					}
				}
				
				if(!empty($_REQUEST['option'][$sku]) && 
				   is_array($_REQUEST['option'][$sku]) &&
				   !empty($_REQUEST['item'][$sku]['quantity']) && 
				   is_numeric($_REQUEST['item'][$sku]['quantity']) &&
				   $_REQUEST['item'][$sku]['quantity'] > 0){

					// Check option types first
					$optionTypes = array();
					foreach($_REQUEST['option'][$sku] as $optionKey=>$optionVal){
						if($optionKey == "type"){
							@list($oName,$oType) = explode("|",$optionVal);
							$optionTypes[trim($oName)] = trim($oType);
							unset($_REQUEST['option'][$sku][$optionKey]);
						}
					}
					// Now do the options
					foreach($_REQUEST['option'][$sku] as $optionName=>$optionValues){


						if($this->debug){
							$_Common->debugPrint($_REQUEST['option'][$sku],"Option");
							//exit;
						}

						$optionType = "option";
						if(!empty($optionTypes[$optionName])){
							$optionType = strtolower($optionTypes[$optionName]);
						}

						// Frosted|2.95|1
						$optionFlds = array();
						$optionFlds = explode("|",$optionValues);

						if(strtolower(trim($optionFlds[0])) == "invalid"){
							$errorMessage = $_CF['cart']['option_required_message'];
							$errorMessage = preg_replace("|%option_name%|i",$optionFlds[1],$errorMessage);
							$_Common->sendMessage($errorMessage,true,1);
							exit;
						}
			
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
							//marcello change option weight to be independent of the quantity
							$weight =	$weight/$_REQUEST['item'][$sku]['quantity'];
						}
						
						if($price == ""){
							$price = 0;	
						}
						if($weight == ""){
							$weight = 0;	
						}

						$oData = array();
						$oData['oid'] = 0;
						$oData['odid'] = 0;
						$oData['name'] = $optionName;
						$oData['price'] = $price;
						$oData['value'] = $value;
						$oData['weight'] = $weight;
						$oData['type'] = $optionType;
						$lineOptions[$sku][] = $oData;
					}
				}
			}
		}

		if($this->debug){
			//$_Common->debugPrint($optionTypes,"Option Types");
			$_Common->debugPrint($lineItems,"Incoming Line Items");
			$_Common->debugPrint($lineOptions,"Incoming Options");
			exit;
		}

		$newCart = array();

		if(count($lineItems) > 0){
			foreach($lineItems as $pid=>$pData){
				
				// validate price
				//if($_CF['cart']['do_lookups']){
				//	$this->lookupItem($pData);
				//}
				
				// set default values if not there so
				// we can check for duplicates lineItems
				foreach($this->cartFields as $key=>$props){
					if(!isset($pData[$key])){
						$pData[$key] = $props[4];
					}
				}
				// Add options if they match
				$pData['options'] = array();
				if(isset($lineOptions[$pid])){
					$pData['options'] = $lineOptions[$pid];
				}
				$duplicates = $this->checkForDuplicates($pData);
				if(!$duplicates){
					
					if($this->haveInventory){
						//$_Common->debugPrint($pData,"Add Line Items/Check inventory");
						//exit;
						$this->checkInventory($pData);
					}
					
					$newCart[] = $pData;
				}
			}
			unset($pData);
		}

		// Add new cart rows
		if(count($newCart) > 0){
			if($this->debug){
				$_Common->debugPrint($newCart,"New Cart Items");
			}
			foreach($newCart as $index=>$data){
				$this->insertCartRow($data);	
			}
		}

		if($this->debug){
			// get the new version of the cart
			$this->getCart();
			$_Common->debugPrint($_CART,"Current Cart");
			exit;
		}
	
	}

	// --------------------------------------------------------------
	// converts form field names from item_100_name to arrays.
	// We do this because javascript is too difficult to validate
	// when you use the php array convention. It breaks the javascript
	// regexp pattern matching.
	function convertFormFields(){
		
		global $_CF;
		global $_Common;
		global $_DB;
		
		//$_Common->debugPrint($_REQUEST);
		
		foreach($_REQUEST as $key=>$value){
			if(strtolower(substr($key,0,5)) == "item_"){
				if(substr($key,-9) == '_quantity'){
					$key = str_replace('item_','',$key);	
					$key = str_replace('_quantity','',$key);
					$sku = $key;
					$_REQUEST['item'][$key]['quantity'] = $value;
					unset($_REQUEST[$key]);
				}
				else{
					$flds = explode("_",$key);
					$_REQUEST['item'][$flds[1]][$flds[2]] = $value;
					unset($_REQUEST[$key]);
					$sku = $flds[1];
				}
				
				// find options
				foreach($_REQUEST as $k=>$v){
					$skuLen = strlen($sku) + 7;
					$optionFldName = "option_" . strtolower($sku) . "_";
					//if(strtolower(substr($k,0,$skuLen + 1)) == $optionFldName && strstr($v,'|')){
					if(strtolower(substr($k,0,$skuLen + 1)) == $optionFldName){
						if(substr($k,-5) == '_type'){
							continue;	
						}
						$fld = str_replace($sku,'',$k);
						$flds = explode("_",$fld);
						array_shift($flds);
						$name = trim(join(" ",$flds));
						//$name = array_pop($flds);
						$_REQUEST['option'][$sku][$name] = $v;
						unset($_REQUEST[$k]);
					}
				}
			}	
		}
		
//		$_Common->debugPrint($_REQUEST['item']);
//		$_Common->debugPrint($_REQUEST['option']);
//		exit;
		
	}
	
	// --------------------------------------------------------------
	function checkForDuplicates($data){
		
		global $_CART;
		global $_Common;
		global $_Registry;
		
		// unset quantities so we can match against arrays.
		$oldQty = $data['quantity'];
		unset($data['quantity']);
		$updated = false;
		foreach($_CART as $index=>$row){
			
			$cartQty = $row['quantity'];
			unset($row['quantity']);

			// check for exact match and update if no difference
			//$_Common->debugPrint($row,"Existing Cart Row");
			//$_Common->debugPrint($data,"Incoming Data to match");
			
			if($row['sku'] == $data['sku'] && $row['name'] == $data['name']){

				// fill in a couple of missing varaibles in the new data
				if(isset($row['quantity_available'])){
					$data['quantity_available'] = $row['quantity_available'];
				}
				if(isset($row['page']) && empty($data['last_page'])){
					$data['last_page'] = trim(basename($data['page']));
				}

				$difference = $this->array_diff_assoc_recursive($row,$data);

				if(!$difference){
					if(count($row['options']) != count($data['options'])){
						$difference = true;
					}
				}
			
				if(!$difference){
					$newQty = ($cartQty + $oldQty);

					if($this->haveInventory){
						$pData['cartquantity'] = intval($cartQty);
						$pData['quantity'] = intval($newQty);
						$pData['name'] = $row['name'];
						$pData['sku'] = $row['sku'];
						$pData['options'] = $row['options'];
						$this->checkInventory($pData);
					}
					
					$this->updateCartRow($row['cartid'],$newQty);
					$updated = true;
					break;
				}
			}
		}

		return $updated;
	}

	// --------------------------------------------------------------
	// Tests to see if the new cart is different than the old one.
	// returns true if different or false if same

	function array_diff_assoc_recursive($array1, $array2){

		global $_Common;

		foreach($array1 as $key => $value){
			
			if(is_array($value)){
				if(isset($array2[$key])){
					if(!is_array($array2[$key])){
						$difference[$key] = $value;
							//$_Common->debugPrint($difference,"Difference1");
					}
					else{
						$new_diff = $this->array_diff_assoc_recursive($value, $array2[$key]);
						if($new_diff != FALSE){
							$difference[$key] = $new_diff;
								//$_Common->debugPrint($difference,"Difference2");
						}
					}
				}
			}
			elseif(is_null($value) && !empty($array2[$key]) && !is_null($array2[$key])){
				// we pass these since they won't be set in the new cart
				if($key == "cdid" || $key == "cartid" || $key == "quantity" || $key == "sessionid"){
					continue;
				}
				$difference[$key] = $value;
					//$_Common->debugPrint($difference,"Difference3");
			}
			elseif(!isset($array2[$key]) || $array2[$key] != $value){
				// we pass these since they won't be set in the new cart
				if($key == "cdid" || $key == "cartid" || $key == "quantity" || $key == "sessionid" || $key == "last_page"){
					continue;
				}
				if(is_null($value) && (empty($array2[$key]) Or is_null($array2[$key]))){
					continue;	
				}
				$difference[$key] = $value;
					//$_Common->debugPrint($difference,"Difference4");
			}
		}

		return !isset($difference) ? FALSE : $difference;
	}

	// -----------------------------------------------------------------
	function checkInventory(&$pData){

		global $_Common;
		global $_DB;
		global $_Registry;

		if(!isset($pData['cartquantity'])){
			$sku = $pData['sku'];
			$cQty = 0;			
			global $_CART;
			foreach($_CART as $i=>$row){
				if($row['sku'] == $sku){
					$cQty += $row['quantity'];
				}
			}
			$pData['cartquantity'] = $cQty;
		}

		$options = array();
		if(!empty($pData['options'])){
			$options = $pData['options'];	
		}

		if(empty($pData['inventory_item']) && !empty($pData['sku'])){
			$sku = $pData['sku'];
			$pCheck = $_DB->getRecord("SELECT inventory_item, inventory_options FROM products WHERE sku = '$sku'");
			if(count($pCheck) > 0){
				$pData = array_merge($pData,$pCheck);
			}
		}
		if(isset($pData['inventory_item']) && $pData['inventory_item'] == 'false'){
			return;	
		}

		if(isset($pData['inventory_options']) && $pData['inventory_options'] == 'false'){
			$options = array();
		}

		$inv = $_Registry->LoadClass("inventory");
		$inventoryData = $inv->checkInventoryForCart($pData['sku'],$pData['quantity'],$options);

		if($this->debug){
			$_Common->debugPrint($pData,"checkInventory");
			$_Common->debugPrint($inventoryData,"Inventory Data");
			exit;
		}

		$cartQty = 0;
		if(isset($pData['cartquantity']) && $pData['cartquantity'] > 0){
			$cartQty = $pData['cartquantity'];
		}
		
		if(count($inventoryData) > 0 && !empty($inventoryData['inventory_item'])){
			
			$product = $pData['name'];
			if($inventoryData['options']){
				$product .= ", " . $inventoryData['options'];
			}
			
			if($inventoryData['inventory_item'] == 'true' && $inventoryData['quantity_available'] == 0){
				// display inventory error
				$message = $inv->soldOutMessage;
				$message = str_replace('%name%',$product,$message);
				$_Common->sendMessage($message,true);
				exit;
			}
			elseif($inventoryData['inventory_item'] == 'true'){
				if(($pData['quantity'] + $cartQty) > $inventoryData['quantity_available']){

					$message = $inv->qtyNotAvailableMessage;
					$message = str_replace('%count%',$inventoryData['quantity_available'],$message);
					$message = str_replace('%name%',$product,$message);
					if($cartQty > 0){
						$message .= '\n' . $inv->qtyInCartMessage;
						$message = str_replace('%quantity%',$cartQty,$message);
					}
					
					$_Common->sendMessage($message,true);
					exit;
				}
			}
			else{
				$pData['quantity_available'] = $inventoryData['quantity_available'];
			}
		}
	}


	
	// -----------------------------------------------------------------
    function insertCartRow($data){

		global $_DB;
		$cartFields = $this->cartFields;
		$cartDetailFields = $this->cartDetailFields;
		$data['sessionid'] = session_id();

		if(isset($data['page']) && trim($data['page']) != ""){
			$data['last_page'] = trim(basename($data['page']));
			$this->updateLastPage($data['last_page']);
		}

		list($fields, $values) = $_DB->makeAddFields($cartFields, 'cartid', $data);
		$sql = "INSERT INTO carts ($fields) VALUES ($values)";
		$rs = $_DB->execute($sql);
		$newcartid = $_DB->getInsertID('carts','cartid');

		if(isset($data['options']) && count($data['options']) > 0){
			foreach($data['options'] as $i=>$detail){
				$detail['cartid'] = $newcartid;
				ksort($detail);
				list($fields, $values) = $_DB->makeAddFields($cartDetailFields, 'cdid', $detail);
				$sql = "INSERT INTO cart_details ($fields) VALUES ($values)";
				$rs = $_DB->execute($sql);
			}
		}
    }
	
	
	//-----------------------------------------------------
	function modify(){
		
		global $_CART;
		global $_Common;
		global $_Registry;
		
		if($this->debug){
			$_Common->debugPrint("Modify Cart");
		}
		
		$sessid = session_id();

		if(isset($_REQUEST['delete']) && count($_REQUEST['delete']) > 0){
			foreach($_REQUEST['delete'] as $cartid=>$deleteFlag){
				unset($_REQUEST['quantity'][$cartid]);
				$this->deleteCartRow($cartid);
			}
		}
		if(isset($_REQUEST['quantity'])){
			
			foreach($_REQUEST['quantity'] as $cartid=>$qty){
				
				$this->checkQty($qty);
				
				if($qty > 0){
				
					if($this->haveInventory){
						$pData['cartquantity'] = intval($_CART[$cartid]['quantity']);
						$pData['quantity'] = intval($qty);
						$pData['name'] = $_CART[$cartid]['name'];
						$pData['sku'] = $_CART[$cartid]['sku'];
						$pData['options'] = $_CART[$cartid]['options'];
						$this->checkInventory($pData);
						//exit;
					}
					$this->updateCartRow($cartid,$qty);
				}
				else{
					$this->deleteCartRow($cartid);
				}
			}
		}
	}
	
	//-----------------------------------------------------
	function LoadCart(){
		
		global $_CART;
		global $_Common;
		global $_DB;
		global $_Registry;
		global $_Totals;

		if($this->debug){
			$_Common->debugPrint("View Cart");
		}

		$_Totals = array();
		$_Totals['totalQuantity'] = 0;
		$_Totals['totalWeight'] = 0;
		$_Totals['subtotal'] = $_Common->format_number(0);
		$_Totals['shipping'] = $_Common->format_number(0);
		$_Totals['grandtotal'] = $_Common->format_number(0);
		$_Totals['cartTaxableTotal'] = $_Common->format_number(0);
		$_Totals['cartTaxTotal'] = $_Common->format_number(0);

		$lastItemSKU = NULL;

		// get the product data for all items in the cart so we 
		// can display all fields for the product and check discounts
		$pData = array();
		$idList = "'" . join("','",$this->pidList) . "'";
		$sql = "SELECT categories.*,product_categories.*,products.*,product_custom.* FROM (categories,product_categories,products)
				LEFT JOIN product_custom ON products.pid = product_custom.custom_pid
				WHERE LOWER(products.sku) IN ($idList)
				AND product_categories.pid = products.pid
				AND categories.catid = product_categories.catid";
		$pData = $_DB->getRecords($sql,'sku');

		// add the product data into the cart fields
		// so we can send it through the discount routines
		foreach($_CART as $k=>$flds){
			$quantity = $flds['quantity'];
			if(!empty($flds['page'])){
				$page = $flds['page'];
			}
			if(isset($pData[$flds['sku']]['options'])){
				unset($pData[$flds['sku']]['options']);
			}
			if(isset($pData[$flds['sku']]) && count($pData[$flds['sku']]) > 0){
				// a field that's in the product data and we don't want to reset in the cart.
				if(isset($page)){
					$pData[$flds['sku']]['page'] = $page;	
				}
				$price = $flds['price'];
				if(strstr($price,":")){
					$price = $_Common->calculateQuantityPrice($price,$quantity);
				}
				
				$_CART[$k] = array_merge($_CART[$k],$pData[$flds['sku']]);

// use the price from the html page
if(isset($_CART[$k]['is_html_page']) && $_CART[$k]['is_html_page'] == 'true'){
	$_CART[$k]['price'] = $price;
}
				
				// this is to make sure we have the right category fields for this item.
				// it could be in 2 or more different categories
				if(!empty($_CART[$k]['category'])){
					
					$catLink = trim($_CART[$k]['category']);
					$sql = "SELECT * FROM categories WHERE category_link = '$catLink'";
					$catData = $_DB->getRecord($sql);
					
					if(count($catData) > 0){
						foreach($catData as $key=>$val){
							if(isset($_CART[$k][$key])){
								$_CART[$k][$key] = $val;
							}
						}
					}
					//$_Common->debugPrint($catData, "category fields correction");
				}
			}
		}

		if($this->debug){
			$_Common->debugPrint($_CART,"Current Cart");
		}

		// load discount class and calculate discounts
		$discounts = $_Registry->LoadClass("Discounts");
		$discounts->calculateProductDiscounts($_CART,true);

		// loop through cart and add up totals
		foreach($_CART as $i=>$fields){

			$lineTotal = 0;
			$quantity = $fields['quantity'];
			$_Totals['totalQuantity'] += $quantity;
			$price = $fields['price'];
			
			if(strstr($price,":")){
				$price = $_Common->calculateQuantityPrice($price,$quantity);
				$_CART[$i]['price'] = $_Common->format_number($price);
			}

			$lineTotal += $_Common->format_number($price * $quantity);
			
			if(is_numeric($fields['weight'])){
				$_Totals['totalWeight'] += ($fields['weight'] * $quantity);
			}
			elseif(strstr($fields['weight'],':')){
				$pkgFlds = explode(':',$fields['weight']);
				if(trim($pkgFlds[0]) == ""){
					$pkgFlds[0] = "0";	
				}
				if(trim($pkgFlds[1]) == ""){
					$pkgFlds[1] = "0";	
				}
				$pkgWeight = $pkgFlds[0] . '.' . $pkgFlds[1];
				$_Totals['totalWeight'] += ($pkgWeight * $quantity);
			}
			
			if(isset($fields['options']) && is_array($fields['options']) && count($fields['options']) > 0){
				foreach($fields['options'] as $j=>$option){
					// qty price for options
					if(strstr($option['price'],":")){
						$option['price'] = $_Common->calculateQuantityPrice($option['price'],$quantity);
					}
					
					if($option['price'] > 0){
						$optionPrice = 0;
						// one time charge
						if($option['type'] == "setup"){
							$optionPrice = $_Common->format_number($option['price']);
						}
						elseif($option['type'] == "option"){
							$optionPrice = $_Common->format_number($option['price'] * $quantity);
						}
						
						if($this->debug){
							$oName = $option['name'];
							$_Common->debugPrint($optionPrice,"found extended option price for $oName");
						}
						
						$lineTotal += $optionPrice;
					}
					elseif($option['price'] < 0){

						// negative numbers
						$option['price'] = substr($option['price'],1);

						// one time charge
						if($option['type'] == "setup"){
							$optionPrice = $option['price'];
						}
						elseif($option['type'] == "option"){
							$optionPrice = ($option['price'] * $quantity);
						}

						$lineTotal = ($lineTotal - $optionPrice);
						
						if($this->debug){
							$oName = $option['name'];
							$_Common->debugPrint($optionPrice,"found extended (negative) option price for $oName");
							$_Common->debugPrint($lineTotal,"(negative) line total");
						}
					}
					
					if(is_numeric($option['weight'])){
						//marcello change option weight to be independent of the quantity
						//$_Totals['totalWeight'] += ($option['weight'] * $quantity);
						$_Totals['totalWeight'] += ($option['weight']);
					}
				}	
			}
			$_CART[$i]['line_total'] = $_Common->format_number($lineTotal);
			$_Totals['subtotal'] += $_Common->format_number($lineTotal);

			if(empty($fields['is_taxable'])){
				$_Totals['cartTaxableTotal'] += $_Common->format_number($lineTotal);
			}

			if(isset($fields['is_taxable']) && $fields['is_taxable'] == "true"){
				$_Totals['cartTaxableTotal'] += $_Common->format_number($lineTotal);
				if(isset($fields['tax_rate']) && $fields['tax_rate'] > 0){
					$_Totals['cartTaxTotal'] += $_Common->format_number($lineTotal * $fields['tax_rate']);
				}
			}
			$lastItemSKU = $fields['sku'];
		}

		$_Totals['cartTaxableTotal'] = $_Common->format_number($_Totals['cartTaxableTotal']);
		$_Totals['subtotal'] = $_Common->format_number($_Totals['subtotal']);
		$_Totals['grandtotal'] = $_Common->format_number($_Totals['subtotal'] + $_Totals['shipping']);
		$_Totals['shippingWeight'] = $_Totals['totalWeight'];


		$_SESSION['cartTotals'] = $_Totals;


		if($lastItemSKU){
			$this->getRelatedItems($lastItemSKU);	
		}
		
		if($this->debug){
			$_Common->debugPrint($_CART,"Cart Contents");
			$_Common->debugPrint($_Totals,"Cart Totals");
		}
	}


	//-----------------------------------------------------
	function lookupItem($sku,$qty = 1){
		global $_Common;
		global $_DB;
		global $_Registry;
		
		$data = array();
		$sql = "SELECT products.*,product_custom.* FROM products
				LEFT JOIN product_custom ON products.pid = product_custom.custom_pid
				WHERE LOWER(products.sku) = '$sku' LIMIT 1";
		$data = $_DB->getRecord($sql);
		
		//$_Common->debugPrint($data,$sql);
		
		return $data;
	}


	//-----------------------------------------------------
	function verifyPrice($lineItem){
		
		global $_Common;
		global $_DB;
		//$_Common->debugPrint($lineItem);
		
		$sku = strtolower($lineItem['sku']);
		$price = $lineItem['price'];
		$name = trim($lineItem['name']);

		if(strstr($price,":")){
			$price = $_Common->calculateQuantityPrice($price,$lineItem['quantity']);
		}
		$price = $_Common->format_number($price);

		$sql = "SELECT * FROM products WHERE LOWER(sku) = '$sku' LIMIT 1";
		//$_Common->debugPrint($sql);
		$data = $_DB->getRecord($sql);
		
		if(count($data) > 0){
			if(isset($data['price']) && strstr($data['price'],":")){
				$data['price'] = $_Common->calculateQuantityPrice($data['price'],$lineItem['quantity']);
			}
			if($data['price'] != $price){
				$_Common->printErrorMessage("Add to cart failed","The price for the $name could not be validated.");
			}
		}
		else{
			$_Common->printErrorMessage("Add to cart failed","We could not find a record for the $name in the database.");
		}
	}


    // -------------------------------------------------------------------
    function checkQty($qty){
		global $_CF;
		global $_Common;
        $qty = trim($qty);
        if(!is_numeric($qty)){
			$_Common->sendMessage($_CF['cart']['invalid_quantity_message'],true);
			exit;
        }
        elseif(is_numeric($qty) && intval($qty) == 0){
			$_Common->sendMessage($_CF['cart']['invalid_quantity_message'],true);
			exit;
		}
        
        return true;
    }

	// -----------------------------------------------------------------
	function updateCartRow($cartid,$qty){
		global $_DB;
		$sessid = session_id();
		$lastPage = $_SERVER['PHP_SELF'];
		if(isset($_SESSION['last_page'])){
			$lastPage = $_SESSION['last_page'];
		}
		$sql = "UPDATE carts SET quantity = '$qty'
				WHERE cartid = '$cartid' AND sessionid = '$sessid'";
		$_DB->execute($sql);
		$this->updateLastPage();
	}
	
	// -----------------------------------------------------------------
	function updateLastPage($lastPage = null){
		global $_DB;
		$sessid = session_id();
		if(!$lastPage && isset($_SESSION['last_page'])){
			$lastPage = $_SESSION['last_page'];
		}
		if($lastPage){
			$sql = "UPDATE carts SET last_page = '$lastPage' WHERE sessionid = '$sessid'";
			$_DB->execute($sql);
		}
	}
	
	// -----------------------------------------------------------------
	function deleteCartRow($cartid){
		global $_DB;
		$sessid = session_id();
		$cSql = "DELETE FROM carts WHERE cartid = '$cartid' AND sessionid = '$sessid'";
		$_DB->execute($cSql);
		$dSql = "DELETE FROM cart_details WHERE cartid = '$cartid'";
		$_DB->execute($dSql);
	}

	// -----------------------------------------------------------------
	function deleteCart($last_page = null){
		global $_Common;
		global $_DB;
		$sessid = session_id();
		$sql = "SELECT cartid FROM carts WHERE sessionid = '$sessid'";
		$carts = $_DB->getRecords($sql);
		foreach($carts as $i=>$flds){
			$cartid = $flds['cartid'];
			$cSql = "DELETE FROM carts WHERE cartid = '$cartid'";
			$_DB->execute($cSql);
			$dSql = "DELETE FROM cart_details WHERE cartid = '$cartid'";
			$_DB->execute($dSql);
		}
		
		$this->updateLastPage($last_page);
		
		$_Common->optimizeTables('cart');
	}

	// --------------------------------------------------------------
	function getRelatedItems($sku = NULL){
		
		global $_Common;
		global $_DB;

		if($sku){
			// get a list of order detail id's
			$sql = "SELECT DISTINCT orid FROM order_details WHERE sku = '$sku' ORDER BY orid DESC";
			$orids = array_keys($_DB->getRecords($sql,'orid'));
			
			// now get a list of other products that were purchased with this item
			$oridList = "'" . join("','",$orids) . "'";
			
			$sql = "SELECT DISTINCT order_details.sku,order_details.name,order_details.page,products.thumbnail_image as thumbnail
					FROM order_details LEFT JOIN products ON (products.sku = order_details.sku)
					WHERE order_details.ordid IN($oridList)
					AND order_details.sku != '$sku'
					ORDER BY order_details.sku";
			
			$this->relatedItems = $_DB->getRecords($sql);
			if($this->debug){
				$_Common->debugPrint($this->relatedItems,"Related items for: $sku");
			}
		}
	}
	
	// --------------------------------------------------------------
	// creates the google checkout button for display on the 
	// shipping page.
	// --------------------------------------------------------------
	function createGoogleCheckoutButton(){

		global $_CART;
		global $_CF;
		global $_Common;
		global $_DB;
		global $_Registry;

		include_once("cp/include/gateway.inc");
		$gateways = new Gateway();

		$cfg = $gateways->readConfig('google checkout');
		$googleDebug = false;
		
		if(count($cfg) == 0 || strtolower(trim($cfg['status']['active'])) != 'true'){
			return;	
		}

		if(trim($cfg['fields']['merchant_id']) == "" || trim($cfg['fields']['merchant_key']) == ""){
			$_Common->printErrorMessage("Google Checkout Error",
				"The Google Merchant ID or Merchant Key has not been set. <br />
				 You cannot use the Google Checkout until these parameeters are set properly.");
		}

		if(strtolower(trim($cfg['status']['debug'])) == 'true'){
			$googleDebug = true;	
		}

		//Include all the required files
		require_once('google/googlecart.php');
		require_once('google/googleitem.php');
		require_once('google/googleshipping.php');
		require_once('google/googletaxrule.php');
		require_once('google/googletaxtable.php');

		$merchant_id = $cfg['fields']['merchant_id'];
		$merchant_key = $cfg['fields']['merchant_key'];
		$server_type = $cfg['fields']['server_type'];
		$currency = $cfg['fields']['currency_symbol'];
		$allowedAreas = $cfg['fields']['allowed_shipping_areas'];
		$shippingTaxed = $cfg['fields']['tax_shipping'];
		$buttonSize = $cfg['fields']['checkout_button_size'];

		$taxes = $_DB->getRecords("SELECT state,rate FROM sales_tax_us");

		$shipping = $_Registry->LoadClass('Shipping.rates');
		list($shippingRates,$junk) = $shipping->GetShippingRateList(true);

		$gCart =  new GoogleCart($merchant_id, $merchant_key, $server_type); 

		// Add the items from the cart
		foreach($_CART as $i=>$row){
			
			$qty = intval($row['quantity']);
			$price = $_Common->format_price($row['line_total'] / $qty);
			$sku = $row['sku'];
			$name = $row['name'];

			$desc = array();
			if(isset($row['description'])){
				$desc[0] = $row['description'];
			}
			else{
				$desc[0] = $name;
			}
			
			if(!empty($row['size']) && $row['size'] != "NA"){
				$desc[] = $row['size'];
			}
			if(isset($row['options'])){
				foreach($row['options'] as $j=>$fields){
					$oDesc = null;
					if(!empty($fields['name'])){
						$oDesc = $fields['name'] . ": ";
					}
					if(!empty($fields['value'])){
						$oDesc .= $fields['value'];
					}
					if($oDesc){
						$desc[] = $oDesc;
					}
				}
			}
			$description = join(", ",$desc);
			$item = null;
			$item = new GoogleItem($sku,$name,$description,$qty,$price,$currency);
			$gCart->AddItem($item);
		}

		if(isset($shippingRates['US'])){

			// Only US shipping methods are allowed right now
			$rates = $shippingRates['US'];
			$region = "US";

			//Add shipping options
			foreach($rates as $method=>$flds){
				
				$rate = $flds['rate'];
				$surcharges = $flds['surcharges'];
				
				if(trim($surcharges) != "" && $region == "US"){

					//AK:2.00,HI:2.00
					$surchargeStates = explode(',',$surcharges);
					$xStates = array();
					foreach($surchargeStates as $k=>$sState){
						@list($state,$charge) = explode(':',$sState);
						$xStates[] = strtoupper(trim($state));
					}
					
					// add it for the whole country first and exclude the surcharge states
					$gShip = new GoogleShipping($method, "flat-rate", $rate, $currency);
					$gShip->SetAllowedCountryArea($allowedAreas);
					$gShip->SetExcludedStateAreas($xStates);
					$gCart->AddShipping($gShip);
					

					foreach($surchargeStates as $i=>$sVal){
						@list($state,$charge) = explode(':',$sVal);
						$newRate = $_Common->format_price($rate + $charge);
						$gShip = new GoogleShipping($method . " ($state)", "flat-rate", $newRate, $currency);
						$gShip->SetAllowedCountryArea('ALL');
						$gShip->SetAllowedStateAreas(array($state));
						$gCart->AddShipping($gShip);
					}
				}
				else{
					$gShip = new GoogleShipping($method, "flat-rate", $rate, $currency);
					if($region == "US"){
						$gShip->SetAllowedCountryArea($allowedAreas);
					}
					$gCart->AddShipping($gShip);
				}
			}
		}

		//Add tax options
		$ruleName = "default";
		foreach($taxes as $k=>$taxFlds){
			$state = $taxFlds['state'];
			$rate = $taxFlds['rate'];
			if(strtolower($shippingTaxed) == "true"){
				$tax_rule = new GoogleTaxRule($ruleName,$rate,"ALL",'true');
			}
			else{
				$tax_rule = new GoogleTaxRule($ruleName,$rate);
			}
			$tax_rule->SetStateAreas($state);
			$tax_table = new GoogleTaxTable($ruleName,$state);
			$tax_table->AddTaxRules($tax_rule);
			$gCart->AddTaxTables($tax_table);
			$ruleName = "alternate";
		}

		if($this->debug || $googleDebug){
			//Display XML data
			$_Common->debugPrint(htmlentities($gCart->GetXML()),"Google Checkout XML");
			//exit;
		}

		// set Google Checkout button for display
		$this->googleCheckoutButton = $gCart->CheckoutButtonCode($buttonSize);

	}











}
?>