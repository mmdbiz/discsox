<?php

// -----------------------------------------------------------------------
class PhoneOrders{

    var $debug = false;
    var $order_number = null;
    var $order_date = null;
    var $orderSaved = false;
    var $emailSent = false;
    
    //-----------------------------------------------------
	function PhoneOrders(){
		
	}
	
    //-----------------------------------------------------
	function saveOrder(){
		
		global $_CF;
		global $_Common;
		global $_DB;

		$cid = null;
		$csid = null;

		if(isset($_REQUEST['customers']['email_list'])){
			$_REQUEST['orders']['email_list'] = $_REQUEST['customers']['email_list'];
		}
		if(!empty($_REQUEST['customers']['cid'])){
			$cid = $_REQUEST['customers']['cid'];
		}
		if(!empty($_REQUEST['customer_shipping']['csid'])){
			$csid = $_REQUEST['customer_shipping']['csid'];
		}

		$custFields = $_DB->getFieldProperties('customers');
		$shipFields = $_DB->getFieldProperties('customer_shipping');
		$orderFields = $_DB->getFieldProperties('orders');
		$detailFields = $_DB->getFieldProperties('order_details');

		// no selected customer. Add them and get the cid
		if(!$cid){
			list($fields, $values) = $_DB->makeAddFields($custFields,'cid',$_REQUEST['customers']);
			$sql = "INSERT INTO customers ($fields) VALUES ($values)";
			if($this->debug){
				$_Common->debugPrint($sql,"Customer Insert");	
			}
			$_DB->execute($sql);
			$cid = $_DB->getInsertID('customers','cid');
		}
		else{
			// just update the customer record
			$flds = $_DB->makeUpdateFields($custFields,'cid',$_REQUEST['customers']);
			$sql = "UPDATE customers SET $flds WHERE cid = '$cid'";
			if($this->debug){
				$_Common->debugPrint($sql,"Update Customer");
			}
			$_DB->execute($sql);
		}


		// no selected customer shipping. Add them and get the csid
		if(!$csid){
			list($fields, $values) = $_DB->makeAddFields($shipFields,'csid',$_REQUEST['customer_shipping']);
			$sql = "INSERT INTO customer_shipping ($fields) VALUES ($values)";
			if($this->debug){
				$_Common->debugPrint($sql,"Customer Shipping Insert");	
			}
			$_DB->execute($sql);
			$csid = $_DB->getInsertID('customer_shipping','csid');
		}
		else{
			// just update the shipping record
			$flds = $_DB->makeUpdateFields($shipFields,'csid',$_REQUEST['customer_shipping']);
			$sql = "UPDATE customer_shipping SET $flds WHERE csid = '$csid'";
			if($this->debug){
				$_Common->debugPrint($sql,"Update Customer Shipping");
			}
			$_DB->execute($sql);
		}

		$_REQUEST['orders']['cid'] = $cid;
		$_REQUEST['orders']['csid'] = $csid;

		// convert the form dates
		$this->order_date = $_REQUEST['orders']['order_date'];
		list($m,$d,$y) = explode('/',$_REQUEST['orders']['order_date']);
		$_REQUEST['orders']['order_date'] = "$y-$m-$d";
		list($m,$d,$y) = explode('/',$_REQUEST['orders']['date_shipped']);
		$_REQUEST['orders']['date_shipped'] = "$y-$m-$d";
        $_REQUEST['orders']['transaction_date'] = date("Y-m-d");
        $_REQUEST['orders']['transaction_time'] = date("H:i:s");


		// remove any commas from totals
        $_REQUEST['orders']['subtotal'] = $_Common->format_price($_REQUEST['orders']['subtotal']);
        $_REQUEST['orders']['discount'] = $_Common->format_price($_REQUEST['orders']['discount']);
        $_REQUEST['orders']['shipping'] = $_Common->format_price($_REQUEST['orders']['shipping']);
        $_REQUEST['orders']['salestax'] = $_Common->format_price($_REQUEST['orders']['salestax']);
        $_REQUEST['orders']['grandtotal'] = $_Common->format_price($_REQUEST['orders']['grandtotal']);


		// double check the order number
		$result = $_DB->getRecord("SELECT MAX(order_number) AS lastnumber FROM orders");
		if(isset($result['lastnumber']) && $result['lastnumber'] > $_REQUEST['orders']['order_number']){
			$_REQUEST['orders']['order_number'] = $result['lastnumber'] + 1;
		}
		$this->order_number = $_REQUEST['orders']['order_number'];

		// add the order
		list($fields, $values) = $_DB->makeAddFields($orderFields, 'orid', $_REQUEST['orders']);
		$sql = "INSERT INTO orders ($fields) VALUES ($values)";
		if($this->debug){
			$_Common->debugPrint($sql,"Order Insert");	
		}
		$_DB->execute($sql);
		$orid = $_DB->getInsertID('orders','orid');

		// add the details
		global $_CART;
		foreach($_REQUEST['sku'] as $i=>$sku){
			
			$data = array();
			$data['orid'] = $orid;
			$data['sku'] = trim($sku);
			$data['name'] = trim($_REQUEST['description'][$i]);
			$data['price'] = trim($_REQUEST['price'][$i]);
			$data['quantity'] = trim($_REQUEST['quantity'][$i]);
			$data['line_total'] = trim($_REQUEST['total'][$i]);

			$_CART[$i] = $data;

			list($fields, $values) = $_DB->makeAddFields($detailFields, 'ordid', $data);
			$sql = "INSERT INTO order_details ($fields) VALUES ($values)";
			if($this->debug){
				$_Common->debugPrint($sql,"Order Detail Insert");	
			}		
			$_DB->execute($sql);
		}

		$this->orderSaved = true;

		if(isset($_REQUEST['sendreceipt'])){
			$this->sendReceipt();
		}

	}
	
	//-----------------------------------------------------
	function sendReceipt(){
		
		global $_CF;
		global $_CART;
		global $_Common;
		global $_Registry;
		global $_Template;
		global $_Totals;
		
		$_SESSION = array_merge($_SESSION,$_REQUEST['customers']);
		$_SESSION = array_merge($_SESSION,$_REQUEST['customer_shipping']);

        $_Totals['subtotal'] = $_REQUEST['orders']['subtotal'];
        $_Totals['discount'] = $_REQUEST['orders']['discount'];
        $_Totals['shipping'] = $_REQUEST['orders']['shipping'];
        $_Totals['salestax'] = $_REQUEST['orders']['salestax'];
        $_Totals['grandtotal'] = $_REQUEST['orders']['grandtotal'];

		$email = $_Registry->loadClass('email');
		$storeName = $_CF['basics']['store_name'];			
		$storeEmailAddress = $_CF['email']['store_email_address'];
		$subject = $_CF['email']['customer_email_subject_line'];
		$subject = str_replace("%orderNumber%",$this->order_number,$subject);
		
		$custEmailAddress = $_SESSION['billaddress_email'];
		//$_Common->debugPrint($custEmailAddress,"custEmailAddress");
		$check = array();
		if(preg_match("/^[_\.0-9a-z-]+@([0-9a-z][-0-9a-z\.]+)\.([a-z]{2,3}$)/i",$custEmailAddress,$check)){
			
			$vars['storeName'] = $storeName;
			$vars['order_number'] = $this->order_number;
			$vars['order_date'] = $this->order_date;
			$vars['selectedCarrier'] = $_REQUEST['orders']['shipping_method'];
			
			$customerEmail = $_Template->open("../../templates/customer.email.html",$vars,false);
			@list($customerHtml,$customerTxt) = explode("<text>",$customerEmail);
			$email->send($storeEmailAddress, $custEmailAddress, $subject, $customerHtml, $customerTxt);
			$this->emailSent = true;
		}

	}

}

?>









