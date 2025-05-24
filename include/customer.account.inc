<?php

class Customer_Account{

    public bool $debug = false;
    public $cid = null;
    public $error = null;
    public array $addressBook = [];
    public string $maxHistoryRecords = "50";
    public array $statusMessages = ['password updated' => "Your password has been updated.",
								'password not found' => "We could not find a record that matches the original password you entered.",
								'billing updated' => "Your billing information has been updated",
								'shipping added' => "Your shipping information has been added",
								'shipping updated' => "Your shipping information has been updated",
								'shipping deleted' => "The selected shipping address was deleted",
								'shipping not deleted' => "The selected shipping address was not deleted",
								'blank shipping address' => "Shipping address cannot be left blank",
								'no primary address' => "You do not have a primary shipping address selected.",
								'too many primary addresses' => "You have too many primary shipping addresses selected",
								'cannot delete last address' => "You cannot delete the last shipping address",
							    ];
    
    public array $deliveryTypes = ["residential","commercial"];
    
    //-----------------------------------------------------
	function __construct(){
		
		$this->cid = trim($_SESSION['cid']);
		if(strlen($this->cid) < 32){
			$this->cid = md5($this->cid);	
		}
	}

	//-----------------------------------------------------
	function getSummary(){

		global $_Common;
		global $_DB;
		$cid = $this->cid;

		//Get shipping, sales tax, discounts, etc for the displayed records
		$sql = "SELECT COUNT(cid) as 'number_of_orders',
					   SUM(discount) as 'discounts',
					   SUM(salestax) as 'salestax',
					   SUM(shipping) as 'shipping',
					   SUM(grandtotal) as 'order_totals'
				FROM orders WHERE MD5(cid) = '$cid'";
				
		$summary = $_DB->getRecord($sql);

		return $summary;	
		
	}
	
	//-----------------------------------------------------
	function updatePassword(){
	
		global $_CF;
		global $_Common;
		global $_DB;
		$cid = $this->cid;

		$pass = $_Common->cleanUserInput(trim($_REQUEST['oldpass']));
		$newpass = $_Common->cleanUserInput(trim($_REQUEST['newpass']));
		
		if($_CF['login']['encrypt_password']){
			$pass = md5(trim($_REQUEST['oldpass']));
			$newpass = md5(trim($_REQUEST['newpass']));
		}
		
		if($_DB->getCount("customers", "WHERE password = '$pass' AND MD5(cid) = '$cid'") > 0){
			$_DB->execute("UPDATE customers SET password = '$newpass' WHERE MD5(cid) = '$cid'");
			$status = $this->statusMessages['password updated'];
		}
		else{
			$status = $this->statusMessages['password not found'];
		}

		return $status;
	}


	//-----------------------------------------------------
	function getBillingAddress(){
		
		global $_CF;
		global $_Common;
		global $_DB;
		global $provinces;
		global $states;
		global $countries;
		
		$cid = $this->cid;
		$data = array();
		$data = $_DB->getRecord("SELECT * FROM customers WHERE MD5(cid) = '$cid'");

		if(count($data) > 0){
			foreach($data as $key=>$value){
				if(!strstr($key,'billaddress')){
					unset($data[$key]);
				}
				if($key == 'billaddress_county' && $data['billaddress_country'] != "GB" && ($value == 'out_of_state' || $value == '')){
					unset($data[$key]);
				}
				elseif($key == 'billaddress_county' && $data['billaddress_country'] == "GB" && $value == 'out_of_state'){
					$data[$key] = "";
					unset($data['billaddress_state']);
				}
				if($key == 'billaddress_country' && ($value != 'US' && $value != 'CA')){
					if(!$_CF['basics']['always_display_area_code']){
						unset($data['billaddress_areacode']);
					}
				}
			}
			//$_Common->loadStateCountry($data);	
		}
		else{
			$fldProperties = $_DB->getFieldProperties("customers");
			foreach($fldProperties as $key=>$values){
				if(strstr($key,'billaddress')){
					$data[$key] = "";
				}
			}
			unset($data['billaddress_county']);
			if(substr($_CF['basics']['currency_code'],0,2) == "US"){
				$data['billaddress_country'] = "US";
			}
			//$_Common->loadStateCountry($data);
		}

		return $data;
	}

	//-----------------------------------------------------
	function updateBilling(){
	
		global $_CF;
		global $_Common;
		global $_DB;
		$cid = $this->cid;
		$data = array();
		$status = null;
		
		$fldProperties = $_DB->getFieldProperties("customers");
		foreach($fldProperties as $key=>$values){
			if(strstr($key,'billaddress') && isset($_REQUEST[$key])){
				$data[$key] = $_REQUEST[$key];
			}
			else{
				unset($fldProperties[$key]);
			}
		}
		if($_DB->getCount("customers", "WHERE MD5(cid) = '$cid'") > 0){
			$values = $_DB->makeUpdateFields($fldProperties, 'cid', $data);
			$sql = "UPDATE customers SET $values WHERE MD5(cid) = '$cid'";
			//$_Common->debugPrint($sql);
			//exit;
			$_DB->execute($sql);
			$status = $this->statusMessages['billing updated'];
		}
		
		return $status;
	}

	//-----------------------------------------------------
	function addShippingAddress(){

		global $_CF;
		global $_Common;
		global $_DB;
		global $provinces;
		global $states;
		global $countries;

		$data = array();
		$fldProperties = $_DB->getFieldProperties("customer_shipping");
		foreach($fldProperties as $key=>$values){
			if(strstr($key,'shipaddress')){
				$data[$key] = "";
			}
		}
		
		if(substr($_CF['basics']['currency_code'],0,2) == "US"){
			$data['shipaddress_country'] = "US";
			unset($data['shipaddress_county']);
		}
		if(substr($_CF['basics']['currency_code'],0,2) == "GB"){
			$data['shipaddress_country'] = "UK";
			unset($data['shipaddress_state']);
			if(!$_CF['basics']['always_display_area_code']){
				unset($data['shipaddress_areacode']);
			}
		}
		
		$_Common->loadStateCountry($data);
		$data['shipaddress_delivery_type'] = $_Common->makeSimpleSelectBox('shipaddress_delivery_type',$this->deliveryTypes,$this->deliveryTypes,$data['shipaddress_delivery_type']);
		
		return $data;

	}
	//-----------------------------------------------------
	function getShippingAddress(){

		global $_CF;
		global $_Common;
		global $_DB;
		global $provinces;
		global $states;
		global $countries;
		
		$cid = $this->cid;
		$data = array();
		
		if(isset($_REQUEST['csid'])){
			$csid = $_Common->cleanUserInput(trim($_REQUEST['csid']));
			$data = $_DB->getRecord("SELECT * FROM customer_shipping WHERE MD5(cid) = '$cid' AND csid = '$csid'");
			if(count($data) > 0){
				foreach($data as $key=>$value){
					if(!strstr($key,'shipaddress') && $key != 'primary_address' && $key != 'csid'){
						unset($data[$key]);
					}
					if($key == 'shipaddress_county' && $data['shipaddress_country'] != "GB" && ($value == 'out_of_state' || $value == '')){
						unset($data[$key]);
					}
					elseif($key == 'shipaddress_county' && $data['shipaddress_country'] == "GB" && $value == 'out_of_state'){
						$data[$key] = "";
						unset($data['shipaddress_state']);
					}
					if($key == 'shipaddress_country' && ($value != 'US' || $value != 'CA')){
						if(!$_CF['basics']['always_display_area_code']){
							unset($data['shipaddress_areacode']);
						}
					}
				}
				$_Common->loadStateCountry($data);
				$data['primary_address'] = $_Common->makeSimpleSelectBox('primary_address',array('true','false'),array('true','false'),$data['primary_address']);
				$data['shipaddress_delivery_type'] = $_Common->makeSimpleSelectBox('shipaddress_delivery_type',$this->deliveryTypes,$this->deliveryTypes,$data['shipaddress_delivery_type']);
			}
		}
		else{
			$data = $_DB->getRecords("SELECT * FROM customer_shipping WHERE MD5(cid) = '$cid' ORDER BY shipaddress_lastname, shipaddress_addr1");
			if(count($data) > 0){
				$primarySet = false;
				
				foreach($data as $i=>$fields){
				
					if($i == 0 && !empty($data[0]['csid'])){
						$_REQUEST['csid'] = $data[0]['csid'];
					}
					if($fields['primary_address'] == 'true'){
						$primarySet = true;
					}
					
					foreach($fields as $key=>$value){
						if(!strstr($key,'shipaddress') && $key != 'primary_address' && $key != 'csid'){
							unset($data[$i][$key]);
						}
						if($key == 'shipaddress_county' && $data[$i]['shipaddress_country'] != "GB" && ($value == 'out_of_state' || $value == '')){
							unset($data[$i][$key]);
						}
						elseif($key == 'shipaddress_county' && $data[$i]['shipaddress_country'] == "GB" && $value == 'out_of_state'){
							$data[$key] = "";
							unset($data[$i]['shipaddress_state']);
						}
						if($key == 'shipaddress_country' && ($value != 'US' || $value != 'CA')){
							if(!$_CF['basics']['always_display_area_code']){
								unset($data[$i]['shipaddress_areacode']);
							}
						}
					}
				}
				if(!$primarySet){
					$this->error = $this->statusMessages['no primary address'];	
				}
				
				$_Common->loadStateCountry($data[0]);
				$data[0]['shipaddress_delivery_type'] = $_Common->makeSimpleSelectBox('shipaddress_delivery_type',$this->deliveryTypes,$this->deliveryTypes,$data[0]['shipaddress_delivery_type']);
				$data[0]['primary_address'] = $_Common->makeSimpleSelectBox('primary_address',array('true','false'),array('true','false'),$data[0]['primary_address']);
				return $data[0];
			}
		}

		$this->checkPrimaryAddress();

		return $data;
		
	}
	//-----------------------------------------------------
	function makeShippingSelect(){
		
		global $_Common, $_DB;
		$cid = $this->cid;
		
		$sql = "SELECT csid,CONCAT(shipaddress_firstname, ' ', shipaddress_lastname, ', ', shipaddress_addr1, ', ' ,shipaddress_city) as text
					   FROM customer_shipping
					   WHERE MD5(cid) = '$cid'
					   ORDER BY shipaddress_lastname, shipaddress_addr1";

		$data = $_DB->getRecords($sql);

		$select = null;

		if(count($data) > 1){
			
			$selected = null;
			if(!empty($_REQUEST['csid'])){
				$selected = $_REQUEST['csid'];
			}
			$select = '<select name="csid" onChange="var url=\'myaccount.php?edit_shipping=1&csid=\' + this.options[this.selectedIndex].value;location.replace(url);">';
			
			foreach($data as $i=>$fields){
				$value = $fields['csid'];
				$name = $fields['text']; 
				if(!is_null($selected) && $value == $selected){
					$select .= "\t<option value=\"$value\" selected>$name</option>\n";
				}
				else{
					$select .= "\t<option value=\"$value\">$name</option>\n";
				}
			}
			$select .= "</select>\n";
		}
		
		return $select;
	}
	
	//-----------------------------------------------------
	function updateShippingAddress(){

		global $_CF;
		global $_Common;
		global $_DB;
		$cid = $this->cid;
		$csid = null;
		if(!empty($_REQUEST['csid'])){
			$csid = $_Common->cleanUserInput(trim($_REQUEST['csid']));
		}
		
		$data = array();
		$status = null;
				
		$fldProperties = $_DB->getFieldProperties("customer_shipping");
		foreach($fldProperties as $key=>$values){
			if(strstr($key,'shipaddress') && isset($_REQUEST[$key])){
				$data[$key] = $_REQUEST[$key];
			}
			elseif($key == 'primary_address'){
				$data[$key] = $_REQUEST[$key];
			}
			else{
				unset($fldProperties[$key]);
			}
		}
		if($csid && $_DB->getCount("customer_shipping", "WHERE MD5(cid) = '$cid' AND csid = '$csid'") > 0){
			$values = $_DB->makeUpdateFields($fldProperties, 'csid', $data);
			$_DB->execute("UPDATE customer_shipping SET $values WHERE MD5(cid) = '$cid' AND csid = '$csid'");
			$status = $this->statusMessages['shipping updated'];
		}
		else{
			$cData = $_DB->getRecord("SELECT cid FROM customers WHERE MD5(cid) = '$cid'");
			$_REQUEST['cid'] = $cData['cid'];
			$fldProperties['cid'] = array('cid');
			if(!empty($_REQUEST['shipaddress_addr1'])){
				list($fields,$values) = $_DB->makeAddFields($fldProperties, 'csid', $_REQUEST);
				$sql = "INSERT INTO customer_shipping ($fields) VALUES ($values)";
				$result = $_DB->execute($sql);
				$_REQUEST['csid'] = $_DB->getInsertID('customer_shipping','csid');
				$status = $this->statusMessages['shipping added'];
			}
			else{
				$status = $this->statusMessages['blank shipping address'];	
			}
		}

		$this->checkPrimaryAddress();
		
		return $status;
	}

	//-----------------------------------------------------
	function checkPrimaryAddress(){

		global $_CF;
		global $_Common;
		global $_DB;
		$cid = $this->cid;

		$data = $_DB->getRecords("SELECT csid,primary_address FROM customer_shipping WHERE MD5(cid) = '$cid'");

		if(count($data) > 0){
			$ids = array();
			$primaryCsid = null;
			foreach($data as $i=>$flds){
				$ids[] = $flds['csid'];	
				if($flds['primary_address'] == 'true'){
					$primaryCsid[] = $flds['csid'];
				}
			}
			if(!$primaryCsid){
				$this->error = $this->statusMessages['no primary address'];
			}
			elseif(count($primaryCsid) > 1){
				$this->error = $this->statusMessages['too many primary addresses'];
			}
			else{
				// reset any orders that do not have a good csid
				// because they may have deleted the shipping address
				$csids = "'" . join("','",$ids) . "'";
				$orders = $_DB->getRecords("SELECT orid,order_number FROM orders WHERE MD5(cid) = '$cid' AND csid NOT IN($csids)");
				//$_Common->debugPrint($orders);
				$csid = $primaryCsid[0];
				foreach($orders as $j=>$order){
					$orid = $order['orid'];
					$sql = "UPDATE orders SET csid = '$csid' WHERE orid = '$orid'";
					//$_Common->debugPrint($sql);
					$_DB->execute($sql);
				}
			}
		}
	}


	//-----------------------------------------------------
	function deleteShippingAddress(){

		global $_CF;
		global $_Common;
		global $_DB;
		$cid = $this->cid;
		$csid = $_Common->cleanUserInput(trim($_REQUEST['csid']));

		$status = null;
		if($csid != ""){
			$count = $_DB->getCount('customer_shipping',"WHERE MD5(cid) = '$cid'");
			if($count > 1){
				$_DB->execute("DELETE FROM customer_shipping WHERE MD5(cid) = '$cid' AND csid = '$csid' LIMIT 1");
				$status = $this->statusMessages['shipping deleted'];
				unset($_REQUEST['csid']);
				$this->checkPrimaryAddress();
			}
			else{
				$status = $this->statusMessages['shipping not deleted'];
				$this->error = $this->statusMessages['cannot delete last address'];
			}
		}
	
		return $status;
	}

	//-----------------------------------------------------
	function getOrderHistory(){
		
		global $_CF;
		global $_Common;
		global $_DB;
		
		global $count,$links,$start,$end,$customerName,$totals;
		
		$cid = $this->cid;

		$hits = 0;
		if(isset($_REQUEST['hits']) && is_numeric($_REQUEST['hits']) && intval($_REQUEST['hits']) > 0){
			$hits = intval($_REQUEST['hits']);
		}		
		
		$max = $this->maxHistoryRecords;
		$count = 0;
		$links = null;
		$start = 0;
		$end = 0;
		$limit = null;
		$customerName = null;
		$records = array();

		$countsql = "SELECT COUNT(orid) as count, SUM(grandtotal) as queryTotal
						FROM orders,customers
						WHERE orders.cid = customers.cid
						AND MD5(customers.cid) = '$cid'";
						
		if(isset($_REQUEST['detail']) && !empty($_REQUEST['orid'])){
			$orid = $_REQUEST['orid'];
			$countsql .= " AND orders.orid = '$orid'";
		}
						
		$queryTotals = $_DB->getRecord($countsql);

		if(!empty($queryTotals['count'])){
			$count = $queryTotals['count'];
			$queryTotal	= $queryTotals['querytotal'];
		}
		
		if($count > 0){
			
			if($max > $count){
				$max = $count + 1;
			}
			list($start,$end,$limit) = $_DB->getLimits($count,$max,"myaccount.php");
			
			if($queryTotals['count'] > $max){
				$_DB->queryVars['order_history'] = 1;
				$_DB->createPreviousNextLinks($hits,$count,$max,"myaccount.php");
				$links = $_DB->previousNextLinks; 
			}

			$sql = "SELECT orders.*,customers.*,
					CONCAT(customers.billaddress_firstname,' ',customers.billaddress_lastname) AS customer_name,
					DATE_FORMAT(orders.transaction_date,'%Y-%c-%d') AS order_date,
					DATE_FORMAT(orders.date_shipped,'%Y-%c-%d') AS date_shipped
					FROM orders,customers
					WHERE orders.cid = customers.cid
					AND MD5(customers.cid) = '$cid'";

			if(isset($_REQUEST['detail']) && !empty($_REQUEST['orid'])){
				$orid = $_REQUEST['orid'];
				$sql .= " AND orders.orid = '$orid'";
			}

			$sql .= " ORDER BY orders.order_number,orders.transaction_date $limit";

			$records = $_DB->getRecords($sql);
			
			//$_Common->debugPrint($records,$sql);
			
			$shipping = $_DB->getRecords("SELECT * FROM customer_shipping WHERE MD5(cid) = '$cid'");
			
			//$_Common->debugPrint($shipping);
			
			$customerName = $records[0]['customer_name'];
		
			//oridList is list of order ids
			$oridList = array();
			$fieldsToRemove = array('card_number','cvv2','expire_month','expire_year',
									'bank_name','name_on_account','account_number','aba_routing_code',
									'username','password');

			foreach($records as $i=>$orderData){
				
				$records[$i]['order_date'] = $this->formatDate($records[$i]['order_date']);
				
				if($records[$i]['date_shipped'] == "0000-0-00" || $records[$i]['date_shipped'] == "0000-00-00"){
					$records[$i]['date_shipped'] = $records[$i]['order_date'];
				}
				
				$records[$i]['date_shipped'] = $this->formatDate($records[$i]['date_shipped']);
				
				$found = false;
				foreach($shipping as $k=>$shipData){
					if($shipData['csid'] == $orderData['csid']){
						unset($shipData['cid']);
						$records[$i] = array_merge($records[$i],$shipData);
						$found = true;
						break;
					}
				}
				// they must have deleted the shipping address
				if(!$found && !empty($shipping[0])){
					unset($shipping[0]['cid']);
					$shipInfo = $shipping[0];
					foreach($shipping as $k=>$row){
						if($row['primary_address'] == 'true'){
							unset($shipInfo);
							unset($shipping[$k]['cid']);
							$shipInfo = $shipping[$k];
							break;
						}
					}
					$records[$i] = array_merge($records[$i],$shipInfo);
				}
				
				$oridList[] = $orderData['orid'];
				foreach($fieldsToRemove as $j=>$fld){
					unset($records[$i][$fld]);
				}
			}
			$orderIds = "'" . join("','",$oridList) . "'";

			if(isset($_REQUEST['detail']) && !empty($_REQUEST['orid'])){
				
				$orderDetailSql = "SELECT * FROM order_details WHERE order_details.orid = '$orid'";
				$orderDetails = $_DB->getRecords($orderDetailSql,"ordid");
				// Add order options and adjust price
				$this->checkOrderOptions($orderDetails);
				$records[0]['details'] = $orderDetails;
				
				$records[0]['totals'] = array('Subtotal' => $records[0]['subtotal'],
											  'Credit'   => $records[0]['credit'],
											  'Discount' => $records[0]['discount'],
											  'Shipping' => $records[0]['shipping'],
											  'Insurance'=> $records[0]['insurance'],
											  'Salestax' => $records[0]['salestax'],
											  'Handling' =>	$records[0]['handling'],
											  'GST'      => $records[0]['gst'],
											  'HST'      => $records[0]['hst'],
											  'PST'      => $records[0]['pst'],
											  'Grandtotal' => $records[0]['grandtotal']);

				//$_Common->debugPrint($records[0]);
			}
			else{

				//Get shipping, sales tax, discounts, etc for the displayed records
				$sql = "SELECT SUM(subtotal) as 'page subtotal',
						SUM(discount) as 'discounts',
						SUM(salestax) as 'sales tax',
						SUM(shipping) as 'shipping fees',
						SUM(grandtotal) as 'page total'
						FROM orders WHERE orid IN($orderIds)";
				$totals = $_DB->getRecords($sql);
				$totals[0]['Report Total'] = $queryTotal;
			}

		}
		return $records;
	}
	
	//-------------------------------------------------------------------
	function formatDate($date){
		
		global $_CF,$_Common;

		//$_Common->debugPrint($date);

		$timeStamp = strtotime($date);

		// sets the date format from CP/Basics
		$dateFormat = "m/d/Y";
		
		if(!empty($_CF['basics']['date_format'])){
			$dateFormat = $_CF['basics']['date_format'];
		}
        $date = date($dateFormat,$timeStamp);	
		
		//$_Common->debugPrint($date);
		//exit;
		return $date;
	}
	
	
	//-------------------------------------------------------------------
	function checkOrderOptions(&$orderDetails){
		
		global $_Common,$_DB,$optionTotal;
		
		//$_Common->debugPrint($orderDetails);
		//$_Common->debugPrint($totals);
		
		// Add order options
		foreach($orderDetails as $ordid=>$orderFlds){
			
			if(strstr($orderFlds['price'],',')){
				$orderDetails[$ordid]['price'] = $_Common->calculateQuantityPrice($orderFlds['price'],$orderFlds['quantity']);
			}
			$quantity = $orderDetails[$ordid]['quantity'];
			
			$orderOptionSql = "SELECT name,price,value,weight,type FROM order_options WHERE ordid = '$ordid'";
			$optionDetails = $_DB->getRecords($orderOptionSql);
			//$_Common->debugPrint($optionDetails);
			
			foreach($optionDetails as $i=>$option){
				
				// qty price for options
				if(strstr($option['price'],":")){
					$option['price'] = $_Common->calculateQuantityPrice($option['price'],$quantity);
				}
				if($option['price'] > 0){
					$optionPrice = 0;
					// one time charge
					if($option['type'] == "setup"){
						$optionPrice = $_Common->format_price($option['price'],true);
					}
					// Standard option
					elseif($option['type'] == "option"){
						$optionPrice = $_Common->format_price($option['price'] * $quantity,true);
					}
					$orderDetails[$ordid]['price'] += $_Common->format_number($optionPrice);
					$optionTotal += $optionPrice;
				}
				
				$orderDetails[$ordid]['price'] = $_Common->format_price($orderDetails[$ordid]['price'],false);
				$orderDetails[$ordid]['options'][] = $option;
			}
		}
	}
	
	//-------------------------------------------------------------------
	function doReorder(){
	
		global $_Common,$_DB,$_Registry;
		
		$_REQUEST['detail'] = 1;
		$orid = $_REQUEST['orid'];

		// load them back through the cart in case something
		// has changed in the product. This will also get the
		// category info in case there are discounts.
		$_REQUEST['add_to_cart'] = 1;

		$orderDetailSql = "SELECT * FROM order_details WHERE order_details.orid = '$orid'";
		$orderDetails = $_DB->getRecords($orderDetailSql,"ordid");

		foreach($orderDetails as $ordid=>$fields){
			// rebuild add to cart requests 
			$_REQUEST['item'][$fields['sku']]['quantity'] = $fields['quantity'];
			$orderOptionSql = "SELECT name,price,value,weight,type FROM order_options WHERE ordid = '$ordid'";
			$optionDetails = $_DB->getRecords($orderOptionSql);
			if(count($optionDetails) > 0){
				foreach($optionDetails as $i=>$flds){
					if($flds['price'] == '0.00'){
						$flds['price'] = '0';
					}
					if(strtolower($flds['name']) == "option"){
						$k = $i+1;
						$flds['name'] = "Option $k";
					}
					$_REQUEST['option'][$fields['sku']][$flds['name']] = $flds['value'] . '|' . $flds['price'] . '|' . $flds['weight'];
				}
			}
		}

//		$_Common->debugPrint($_REQUEST);
//		exit;
		
		$cart = $_Registry->LoadClass('cart');
		$cart->initialize();
		exit;
	}
	
}
?>










