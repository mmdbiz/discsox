<?php
// -----------------------------------------------------------------------
//         Class: Discounts
//       Purpose: This class does all the discount calculations for
//				  the product display, cart display, and checkout pages.
// Last Modified: Oct 12, 2007
//
// -----------------------------------------------------------------------
class Discounts{

    //marcello
	var $debug = false;
    
    // these vars get passed from calcs.inc
    // and are added below
	var $Country = "US";
	var $State = "";
	var $Zip = "";
	var $County = "";
	var $freeShipping = false;
	
    //-----------------------------------------------------
	function __construct($calcs = NULL){
		global $_Common, $_DB;
		
		if(is_object($calcs)){
			// load class vars from calcs
			foreach(get_object_vars($calcs) as $name=>$value){
				if(isset($this->$name)){
					$this->$name = $value;
				}
			}
		}
		//marcello
		$this->debug = false;
	}

	//-----------------------------------------------------
	function calculateDiscount($subtotal,$quantity){
		
		global $_CART;
		global $_Common;
		global $_DB;
		global $_Totals;
		
		$discount = 0;
		$fCoupon = NULL;

		$discRates = array();
		$sql = "SELECT coupon, catid, category, subtotal_ranges, qty_ranges, free_shipping,apply_to_subcategories FROM discounts";
		$discRates = $_DB->getRecords($sql);

		// check for report cart
		if(isset($_SESSION['report_cart'])){
			$_CART = $_SESSION['report_cart'];	
		}

		// get cart category values
		$cats = array();
		$catids = array();
		$subcatids = array();

		foreach($_CART as $j=>$fields){
			if(!empty($fields['catid'])){
				@$catids[$fields['catid']]['quantity'] += $fields['quantity'];
				@$catids[$fields['catid']]['total'] += $fields['line_total'];
			}
			if(!empty($fields['category_ids'])){
				// load all cat ids in the list so we can see if any of them apply
				$ids = explode(':',$fields['category_ids']);
				for($k=1;$k<count($ids);$k++){
					@$subcatids[$ids[$k]]['quantity'] += $fields['quantity'];
					@$subcatids[$ids[$k]]['total'] += $fields['line_total'];
				}
			}
			if(!empty($fields['category_name'])){
				@$cats[$fields['category_name']]['quantity'] += $fields['quantity'];
				@$cats[$fields['category_name']]['total'] += $fields['line_total'];
			}
			elseif(!empty($fields['category'])){
				@$cats[$fields['category']]['quantity'] += $fields['quantity'];
				@$cats[$fields['category']]['total'] += $fields['line_total'];
			}
		}
		
		$tempDiscount = 0;
		if(!empty($_REQUEST['coupon'])){
			$fCoupon = trim(strtolower($_REQUEST['coupon']));
			$_Totals['coupon'] = $fCoupon;
		}
		elseif(!empty($_Totals['coupon'])){
			$fCoupon = $_Totals['coupon'];
		}

		foreach($discRates as $i=>$flds){
			
			$coupon = trim($flds['coupon']);
			$catid = trim($flds['catid']);
			$category = trim($flds['category']);
			$subtotal_range = trim($flds['subtotal_ranges']);
			$qty_range = trim($flds['qty_ranges']);
			$free_shipping = trim($flds['free_shipping']);
			$apply_to_subcategories = trim($flds['apply_to_subcategories']);
			
			$match = false;
			// form coupons and coupon in DB
			if($fCoupon != "" && trim($coupon) != ""){
				
				if($fCoupon == strtolower(trim($coupon))){

					if(($catid != "" && $catid != '0') || trim($category) != ""){
						if($catid != "" && $apply_to_subcategories == 'true' && !empty($subcatids[$catid])){
							$subtotal = $subcatids[$catid]['total'];
							$quantity = $subcatids[$catid]['quantity'];
						}
						elseif($catid != "" && $apply_to_subcategories == 'false' && !empty($catids[$catid])){
							$subtotal = $catids[$catid]['total'];
							$quantity = $catids[$catid]['quantity'];
						}
						elseif(trim($category) != "" && !empty($cats[$category])){
							$subtotal = $cats[$category]['total'];
							$quantity = $cats[$category]['quantity'];
						}
						else{
							$subtotal_range = null;
							$qty_range = null;
						}
						
						if(trim($subtotal_range) != ""){
							list($tempDiscount,$match) = $this->calculatePrice($subtotal_range,$subtotal,$subtotal);
							if($this->debug){$_Common->DebugPrint("Category ($category) subtotal discount with coupon: $coupon, $tempDiscount, $match");}
						}
						elseif(trim($qty_range) != ""){
							list($tempDiscount,$match) = $this->calculatePrice($qty_range,$quantity,$subtotal);
							if($this->debug){$_Common->DebugPrint("Category quantity discount with coupon: $coupon, $tempDiscount, $match");}
						}
					}
					elseif(trim($subtotal_range) != ""){
						list($tempDiscount,$match) = $this->calculatePrice($subtotal_range,$subtotal,$subtotal);
						if($this->debug){$_Common->DebugPrint("Subtotal discount with coupon: $coupon, $tempDiscount, $match");}
					}
					elseif(trim($qty_range) != ""){
						list($tempDiscount,$match) = $this->calculatePrice($qty_range,$quantity,$subtotal);
						if($this->debug){$_Common->DebugPrint("Quantity discount with coupon: $coupon, $tempDiscount, $match");}
					}
					else{
						$match = true;
					}
				}
			}
			// no form coupon and no coupon in DB
			elseif(!$fCoupon && trim($coupon) == ""){
				
				if(($catid != "" && $catid != '0') || trim($category) != ""){
					if($catid != "" && $apply_to_subcategories == 'true' && !empty($subcatids[$catid])){
						$subtotal = $subcatids[$catid]['total'];
						$quantity = $subcatids[$catid]['quantity'];
					}
					elseif($catid != "" && $apply_to_subcategories == 'false' && !empty($catids[$catid])){
						$subtotal = $catids[$catid]['total'];
						$quantity = $catids[$catid]['quantity'];
					}
					elseif(trim($category) != "" && !empty($cats[$category])){
						$subtotal = $cats[$category]['total'];
						$quantity = $cats[$category]['quantity'];
					}
					else{
						$subtotal_range = null;
						$qty_range = null;
					}
					
					if(trim($subtotal_range) != ""){
						list($tempDiscount,$match) = $this->calculatePrice($subtotal_range,$subtotal,$subtotal);
						if($this->debug){$_Common->DebugPrint("Category ($category) subtotal discount without coupon: $coupon, $tempDiscount, $match");}
					}
					elseif(trim($qty_range) != ""){
						list($tempDiscount,$match) = $this->calculatePrice($qty_range,$quantity,$subtotal);
						if($this->debug){$_Common->DebugPrint("Category quantity discount without coupon: $coupon, $tempDiscount, $match");}
					}
				}
				elseif(trim($subtotal_range) != ""){
					list($tempDiscount,$match) = $this->calculatePrice($subtotal_range,$subtotal,$subtotal);
					if($this->debug){$_Common->DebugPrint("Subtotal discount: $tempDiscount,$match");}
				}
				elseif(trim($qty_range) != ""){
					list($tempDiscount,$match) = $this->calculatePrice($qty_range,$quantity,$subtotal);
					if($this->debug){$_Common->DebugPrint("Quantity discount: $tempDiscount,$match");}
				}
			}
			if($match){
				$discount = $tempDiscount;
				if(strtolower(trim($free_shipping)) == "true"){
					$this->freeShipping = true;
				}
				break;
			}
		}

		// check customer discount and override logic discount
		$custDiscount = $this->checkCustomerDiscount($subtotal,$quantity,true);
		if($custDiscount > 0){
			$discount = $custDiscount;
		}

		$discount = $_Common->format_number($discount);

		if($this->debug){
			$_Common->debugPrint(get_object_vars($this),"Discount Class Properties");
		}

		return array($discount,$this->freeShipping);
	}

	// -------------------------------------------------------------------
    function calculatePrice($rates,$quantity,$subTotal){

		global $_Common;

		if($this->debug){
			$_Common->debugPrint($rates, "Discounts - calculate price, $quantity, $subTotal");
		}

		$match = false;
		$price = 0; //marcello $price needs to be initilized
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
					// they set a highest entry but the customer bought more than the highest entry
					// just use the highest price
					elseif($i = (count($qtyPrices) -1)){
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
				if(is_numeric($price) && is_numeric($quantity) && $quantity >= $price){
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

		global $_Common;

		if($this->debug){
			$_Common->debugPrint($rate, "Discounts - calcRate");
		}

		//check for special handleing flags.
		$isPercent = preg_match('/%/',$rate);
		$isEach = preg_match('/each$|ea$|e$/i',$rate);
		
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
	
	// --------------------------------------------------------------
	function checkCustomerDiscount($subtotal,$quantity,$isCheckout = false){
		
		global $_DB;
		global $_Common;

		$discount = 0;

		if($isCheckout && !empty($_SESSION['customer_discount_applied'])){
			return $_Common->format_number(0);
		}

		if(!empty($_SESSION['cid'])){
			$cid = $_SESSION['cid'];
			$sql = "SELECT discount_rate,discount_type,discount_text FROM customers WHERE MD5(cid) = '$cid' LIMIT 1";
			$record = $_DB->getRecord($sql);
			if(count($record) > 0){
				$rate = trim($record['discount_rate']);
				$type = trim($record['discount_type']);
				if(strstr($rate,":")){
					if($type == "percentage"){
						list($discount,$match) = $this->calculatePrice($rate,$subtotal,$subtotal);
					}
					elseif($type == "subtotal"){
						list($discount,$match) = $this->calculatePrice($rate,$subtotal,$subtotal);
					}
					else{
						list($discount,$match) = $this->calculatePrice($rate,$quantity,$subtotal);
					}
				}
				else{
					$isPercent = preg_match('/%/',$rate);
					if($isPercent || $type == "percentage"){
						$rate = preg_replace("/\%/","",$rate);
						$discount = ($subtotal * $rate) / 100;
					}
					else{
						$discount = $rate;
					}
				}
			}
		}
		if($discount > 0 && $isCheckout){
			$_SESSION['discount_text'] = trim($record['discount_text']);
		}
		if($this->debug){
			$_Common->DebugPrint("Customer Discount: $discount, isCheckOut: $isCheckout");
		}
		return $_Common->format_number($discount);
	}

	// --------------------------------------------------------------
	// resets the low-high numbers in the qty price string
	// if the high values are missing.
	function validatePriceStr($qty_price){
		
		global $_Common;
		
		if($this->debug){
			$_Common->debugPrint($qty_price, "Discounts - validatePriceStr");
		}
		
		$fields = explode(",",$qty_price);
        $low = 1;
        $high = "";
        $lastPrice = 0;
		$newQtyStr = array();
        $counter = 0;
        foreach($fields as $i=>$val){

            $qty = "";
            $prc = "";
            list($qty,$prc) = explode(":",$val);
			$lastPrice = $prc;

            if(strstr($qty,"-")){
                $qty = str_replace(" ", "", $qty);
                list($low,$high) = explode("-",$qty);
            }
			if(!strstr($qty,"-") || $high == ""){
				if(strstr($qty,"-")){
					$qty = str_replace("-","",$qty);
				}
				$low = $qty;
				if(isset($fields[$counter + 1])){
					$nextVal = $fields[$counter + 1];
					list($nextqty,$nextprc) = explode(":",$nextVal);
					if(strstr($nextqty,"-")){
						$nextqty = str_replace(" ", "", $nextqty);
						list($nextlow,$nexthigh) = explode("-",$nextqty);
						$high = ($nextlow - 1);
					}
					else{
						$high = ($nextqty - 1);
					}
				}
			}
			$newQtyStr[] = "$low-$high:$lastPrice";
			$counter++;
        }
		if(count($newQtyStr) > 0){
			if($this->debug){
				$_Common->debugPrint(join(",",$newQtyStr), "Discounts - validatePriceStr newQtyStr");
			}
			return join(",",$newQtyStr);
		}
		else{
			if($this->debug){
				$_Common->debugPrint($qty_price, "Discounts - validatePriceStr qty_price");
			}
	        return $qty_price;
	    }
	}
	
	// --------------------------------------------------------------
	// The functions below do the category and customer discounts
	// when items are displayed during a search or when they are
	// displayed in the cart tables
	// --------------------------------------------------------------
	
	function calculateProductDiscounts(&$records,$isCart = false){

		global $_Common,$_DB;

		//$_Common->debugPrint("calculateProductDiscounts");

		if(empty($_SESSION['customer_discount'])){
			$_SESSION['customer_discount'] = 0;	
		}
		
		// customer discount flag
		$useCustomerDiscount = false;
		// get customer discounts
		$customerDiscount = array();
		if(!empty($_SESSION['cid'])){
			$cid = $_SESSION['cid'];
			$sql = "SELECT discount_rate,discount_type,discount_text FROM customers WHERE MD5(cid) = '$cid' LIMIT 1";
			$customerDiscount = $_DB->getRecord($sql);
			if(count($customerDiscount) > 0 && 
			   $customerDiscount['discount_rate'] != '' &&
			   $customerDiscount['discount_rate'] != '0' &&
			   $customerDiscount['discount_rate'] != '0%'){
				$useCustomerDiscount = true;
			}
		}
		
		//get the counts for category discounts
		$catqtys = array();
		if($isCart){
			$catqtys = $this->loadCategoriesFromCartFields($records);
		}

		// Ok, now reset the fields to the new prices
		foreach($records as $i=>$fields){
			
			$quantity = 1;
			$price = 0;
			$discount = 0;
			if(isset($fields['quantity'])){
				$quantity = $fields['quantity'];
			}
			$price = $fields['price'];
			if(strstr($price,":")){
				$fields['price'] = $this->validatePriceStr($fields['price']);
				//$_Common->debugPrint($fields['price']);
				if(empty($records[$i]['qty_price'])){
					$records[$i]['qty_price'] = $fields['price'];
				}
				$price = $_Common->calculateQuantityPrice($fields['price'],$quantity);
			}

			// reset cart fields with discounts
			$discount = 0;
			if($useCustomerDiscount){
				$rate = trim($customerDiscount['discount_rate']);
				$type = trim($customerDiscount['discount_type']);
				if(strstr($rate,":")){
					if($type == "percentage"){
						list($discount,$match) = $discounts->calculatePrice($rate,$price,$price);
					}
					elseif($type == "subtotal"){
						list($discount,$match) = $discounts->calculatePrice($rate,$price,$price);
					}
					else{
						list($discount,$match) = $discounts->calculatePrice($rate,$quantity,$price);
					}
					$records[$i]['qty_price'] = $this->resetQtyPricing($rate,$quantity,$price);
					if($this->debug){
						$_Common->debugPrint($records[$i]['qty_price'],"Customer Discount Based Quantity Pricing");	
					}
				}
				else{
					$isPercent = preg_match('/%/',$rate);
					if($isPercent || $type == "percentage"){
						$rate = preg_replace("/\%/","",$rate);
						$discount = $_Common->format_number(($price * $rate) / 100);
					}
					else{
						$discount = $_Common->format_number($rate);
					}
				}
				if($this->debug){
					$_Common->debugPrint($discount,"Customer Discount");
				}
				if($discount > 0){
					// tell the system that the customer discount was already applied
					// so it's not re-applied at checkout
					$_SESSION['customer_discount_applied'] = true;
					$records[$i]['customer_discount'] = $_Common->format_number($discount);
					$_SESSION['customer_discount'] += $_Common->format_number($discount);
				}
			}

			// category discounts are based on the total quantity of items
			// in the cart that match the category or a parent with the
			// apply_to_subcategories flag set to true
			
			// For the product display, we check the parents to see if
			// there are discounts and use a qty of 1. It also rebuilds
			// the qty pricing to display new rates.
			
			if(count($catqtys) > 0 && isset($fields['category_ids'])){
				$catids = $fields['category_ids'];
				$catQty = $catqtys[$catids]['quantity'];
				$catDiscountRate = $catqtys[$catids]['category_discount'];
				
				//$_Common->debugPrint($catDiscountRate,"cat discount rate");
				
				if($catDiscountRate != "" && $catDiscountRate != "0%" && $catDiscountRate != "0"){
					// calculate the discount based on the total cart items in this category
					list($catdiscount,$match) = $this->calculatePrice($catDiscountRate,$catQty,$price);
					if(strstr($catDiscountRate,':')){
						$records[$i]['qty_price'] = $this->resetQtyPricing($catDiscountRate,($price - $discount));
					}
					if($catdiscount > 0){
						$discount += $catdiscount;	
					}
					if($this->debug){
						$_Common->debugPrint($catDiscountRate,"Category Discount Rates for cart");
						$_Common->debugPrint($catdiscount,"Category Discount for cart");
						$_Common->debugPrint($records[$i]['qty_price'],"Reset Quantity Price for cart");
					}
				}
			}
			elseif(isset($fields['category_ids'])){
				// must be product display and qty is always one
				$catDiscountRate = $this->findDiscountInCategories($fields['catid'],$fields['category_ids']);
				
				//$_Common->debugPrint($catDiscountRate,"catid discount rate");
				
				if($catDiscountRate != "" && $catDiscountRate != "0%" && $catDiscountRate != "0"){
					// calculate the discount based on one item
					list($catdiscount,$match) = $this->calculatePrice($catDiscountRate,$quantity,$price);
					if(strstr($catDiscountRate,':')){
						$records[$i]['qty_price'] = $this->resetQtyPricing($catDiscountRate,($price - $discount));
					}
					if($catdiscount > 0){
						$discount += $catdiscount;	
					}
					if($this->debug){
						$_Common->debugPrint($catDiscountRate,"Category Discount Rates for display");
						$_Common->debugPrint($catdiscount,"Category Discount for display");
						$_Common->debugPrint($records[$i]['qty_price'],"Reset Quantity Price for display");
					}
				}
			}
			if($discount > 0){
				if($this->debug){
					$_Common->debugPrint($discount,"Product Discount");
				}
				$records[$i]['retail_price'] = $_Common->format_number($price);
				$records[$i]['price'] = $_Common->format_number($price - $discount);
			}
		}
	}
	
	//-----------------------------------------------------
	function loadCategoriesFromCartFields($records){
		
		global $_Common,$_DB;

		//marcello
		//$_Common->debugPrint($records,"cart records for discount");

		$catqtys = array();

		foreach($records as $i=>$flds){
			$quantity = 1;
			$price = 0;
			$discount = 0;
			if(isset($flds['quantity'])){
				$quantity = $flds['quantity'];
			}
			$price = $flds['price'];
			if(strstr($price,":")){
				$price = $_Common->calculateQuantityPrice($price,$quantity);
			}
			// create a list of category discounts and 
			// their quantities to calculate on
			if(isset($flds['category_ids'])){
				// get a list of parents and their settings
				$catids = $flds['category_ids'];
				$idlist = explode(':',$catids);
				$ids = "'" . join("','",$idlist) . "'";
				$sql = "SELECT catid,category_link,category_ids,category_discount,apply_to_subcategories
						FROM categories
						WHERE catid IN($ids) 
						AND (category_discount != '' OR category_discount != '0%' OR category_discount != '0')
						ORDER BY category_ids";
				$data = $_DB->getRecords($sql);
				// add up the quantities from each subcat in the
				// cart that match so we get a total quantity for
				// the parent
				foreach($data as $i=>$flds){
					$parentids = $flds['category_ids'];
					@$catqtys[$parentids]['quantity'] += $quantity;
					@$catqtys[$parentids]['subtotal'] += $price;
					@$catqtys[$parentids]['category_discount'] = $flds['category_discount'];
					@$catqtys[$parentids]['apply_to_subcategories'] = $flds['apply_to_subcategories'];
				}
			}
		}


		if(count($catqtys) > 0){
			//sort by category ids
			ksort($catqtys);
			foreach($catqtys as $ids=>$data){
				// if this category has no discount set, check the parents
				// to see if they do and add up the quantities. Else, just use
				// the discount the way it is.
				if($data['category_discount'] == "" || $data['category_discount'] == "0%" || $data['category_discount'] == "0"){
					// explode the ids and check to see if a parent exists in the list			
					$list = explode(':',$ids);
					// use > 2 since we don't want to backup to 0, the main parent
					while(count($list) > 2){
						array_pop($list);
						$parent = join(':',$list);
						if(isset($catqtys[$parent]) && $catqtys[$parent]['apply_to_subcategories'] == 'true'){
							if($catqtys[$parent]['category_discount'] != "" && $catqtys[$parent]['category_discount'] != "0%" && $catqtys[$parent]['category_discount'] != "0"){
								// reset the the child qty to the highest parent qty 
								// for calculations later. This way it applies to the parent
								// and to all the children since we are resetting them in order
								$catqtys[$ids]['quantity'] = $catqtys[$parent]['quantity'];
								$catqtys[$ids]['subtotal'] = $catqtys[$parent]['subtotal'];
								$catqtys[$ids]['category_discount'] = $catqtys[$parent]['category_discount'];
							}
						}
					}
				}
			}
			if($this->debug){
				$_Common->debugPrint($catqtys,"Category Discount Quantities");
			}
			//exit;
		}
		return $catqtys;
	}

	// --------------------------------------------------------------
	// goes through a list of catids to find out if
	// a parent has a discount applied and the apply
	// to subcats is turned on.
	function findDiscountInCategories($catid,$ids){
	
		global $_Common,$_DB;
		
		$idlist = explode(':',$ids);
		
		$catids = "'" . join("','",$idlist) . "'";
		
		$sql = "SELECT catid,category_link,category_discount,apply_to_subcategories
				FROM categories
				WHERE catid IN($catids) AND (category_discount != '' OR category_discount != '0%' OR category_discount != '0')";
		
		$data = $_DB->getRecords($sql);
		$discountRate = null;
		foreach($data as $ids=>$flds){
			if($flds['category_discount'] != "" && $flds['category_discount'] != '0%'){
				if($flds['catid'] == $catid){
					$discountRate = $flds['category_discount'];
				}
				else{
					if($flds['apply_to_subcategories'] == 'true'){
						$discountRate = $flds['category_discount'];
					}
				}
			}
		}
		if($this->debug){
			$_Common->debugPrint($data,"cat discount data: $sql");
			$_Common->debugPrint($discountRate,"cat discount rate");
		}
		return $discountRate;

	}
	
	//-----------------------------------------------------
	function resetQtyPricing($rates,$retailPrice,$qty = 1){
		
		global $_Common;

		//$this->debug = true;

		$newQtyStr = array();
		$fields = explode(",",$rates);
		
		foreach($fields as $i=>$group){
			@list($qty,$price) = explode(":",$group);
            @list($low,$high) = explode("-",$qty);
			if(preg_match('/%/',$price)){
				$price = preg_replace("/\%/","",$price);
				$discount = ($retailPrice * $price) / 100;
			}
			else{
				$discount = $price;
			}
			if($discount > 0){
				$newPrice = $_Common->format_number($retailPrice - $discount);
			}
			$newQtyStr[] = "$low-$high:$newPrice";
		}
		if(count($newQtyStr) > 0){
			
			// add the rate for 1 item if missing from the qty string
			if(isset($newQtyStr[0]) && substr($newQtyStr[0],0,1) != '1'){
				$low = "1";
				// get the next lowest value
				if(isset($newQtyStr[0])){
					@list($nextqty,$nextprice) = explode(":",$newQtyStr[0]);
					@list($nextlow,$nexthigh) = explode("-",$nextqty);
					$high = $nextlow - 1;
				}
				array_unshift($newQtyStr,"$low-$high:$retailPrice");
			}
			
			if($this->debug){
				$_Common->debugPrint(join(",",$newQtyStr),"discounts: resetQtyPricing");
			}
			
			return join(",",$newQtyStr);
		}
		else{
			
			if($this->debug){
				$_Common->debugPrint($rates,"discounts: resetQtyPricing");
			}
			return $rates;	
		}
	}
}

?>