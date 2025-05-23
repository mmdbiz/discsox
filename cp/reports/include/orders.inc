)<?php

$adjustPricesToDatabasePrices = false;

// -------------------------------------------------------------------
function makeCriteriaSelect(){
	global $_Common;
	$names = array("Today","Yesterday","This Month","Last Month","Selected Dates ->","All Dates");
	$values = array("TODAY","YD","TM","LM","SD","ALL");
	
	$select = "<select name=\"searchCriteria\" onChange=\"if(this.options[this.selectedIndex].value != 'SD'){this.form.elements['search'].click();}\">";
	foreach($names as $i=>$name){
		
		if(isset($_REQUEST['searchCriteria']) && $_REQUEST['searchCriteria'] == $values[$i]){
			$select .= '<option value="' . $values[$i] . '" selected>' . $name . '</option>';
		}
		else{
			$select .= '<option value="' . $values[$i] . '">' . $name . '</option>';
		}
	}
	$select .= '</select>';

	return $select;
}

function makePaidStatusSelect(){
	global $_Common;
	$names = array("All","True","False");
	$values = array("all","true","false");
	list($select,$selected) = $_Common->makeSelectBox('paidstatus',$names,$values,NULL,false);
	return $select;
}

function makeOrderStatusSelect(){
	global $_Common;
	$names = array("All","Not Shipped","Partial Shipped","Complete","Cancelled");
	$values = array("all","Not Shipped","Partial Shipped","Complete","Cancelled");
	list($select,$selected) = $_Common->makeSelectBox('orderstatus',$names,$values,NULL,false);
	return $select;
}

// -------------------------------------------------------------------
function dateSelect($start = true){
	
	global $_Form;
	include_once("date.select.inc");
	
	$searchCriteria = $_Form->GetValue('searchCriteria');
	
	if($searchCriteria){
		switch($searchCriteria){
			
			case "TODAY":
				$_Form->SetValue('start-day',date("d"));
				$_Form->SetValue('start-month',date("m"));
				$_Form->SetValue('start-year',date("Y"));
				$_Form->SetValue('end-day',date("d"));
				$_Form->SetValue('end-month',date("m"));
				$_Form->SetValue('end-year',date("Y"));
				break;
			
			case "YD":
				$_Form->SetValue('start-day',date("d")- 1);
				$_Form->SetValue('start-month',date("m"));
				$_Form->SetValue('start-year',date("Y"));
				$_Form->SetValue('end-day',date("d") - 1);
				$_Form->SetValue('end-month',date("m"));
				$_Form->SetValue('end-year',date("Y"));
				break;
			
			case "TM":
				$_Form->SetValue('start-day',"01");
				$_Form->SetValue('start-month',date("m"));
				$_Form->SetValue('start-year',date("Y"));
				$_Form->SetValue('end-day',date("d"));
				$_Form->SetValue('end-month',date("m"));
				$_Form->SetValue('end-year',date("Y"));
				break;
			
			case "LM":
				$_Form->SetValue('start-day',"01");
				
				$_Form->SetValue('start-month',date("m") - 1);
				if(strlen($_Form->GetValue('start-month') == 1)){
					$_Form->SetValue('start-month', "0" . $_Form->GetValue('start-month'));
				}
				$_Form->SetValue('start-year',date("Y"));
				$_Form->SetValue('end-day',31);
				
				$_Form->SetValue('end-month',date("m") - 1);
				if(strlen($_Form->GetValue('end-month')) == 1){
					$_Form->SetValue('end-month', "0" . $_Form->GetValue('end-month'));
				}
				$_Form->SetValue('end-year',date("Y"));
				break;
			
			case "ALL":
				$_Form->SetValue('start-day',date("d"));
				$_Form->SetValue('start-month',date("m"));
				$_Form->SetValue('start-year',date("Y") - 10);
				$_Form->SetValue('end-day',date("d"));
				$_Form->SetValue('end-month',date("m"));
				$_Form->SetValue('end-year',date("Y"));
				break;
			
		} // End switch
	}
	
	if($start){
		$startDay = $_Form->GetValue('start-day',0);
		$startMonth =  $_Form->GetValue('start-month',0);
		$startYear = $_Form->GetValue('start-year',0);
		$minYr = date('Y') - 10;

		list($days,$months,$years) =
			selectDate($startDay,$startMonth,$startYear,"start-day","start-month","start-year",$minYr);
		
	}else{
		$endDay = $_Form->GetValue('end-day',0);;
		$endMonth = $_Form->GetValue('end-month',0);;
		$endYear = $_Form->GetValue('end-year',0);;
		$minYr = date('Y') - 2;
		
		list($days,$months,$years) =
			selectDate($endDay,$endMonth,$endYear,"end-day","end-month","end-year",$minYr);
		
	}
	print $months;
	print $days;
	print $years;
}

// -------------------------------------------------------------------
function getSummaryRecords(){
	
	global $_Common;
	global $count;
	global $_DB;
	global $debug;
	global $end;
	global $endDate;
	global $error;
	global $_Form;
	global $hits;
	global $limit;
	global $links;
	global $maxToDisplay;
	global $orderCounts;
	global $records;
	global $start;
	global $startDate;
	global $totals;
	
//$debug = true;
	
	setLastQuery();
	
	$startDate = date("Y-m-d");
	$endDate = date("Y-m-d");
	if(!empty($_REQUEST['search'])){
		$startDay = $_Form->GetValue('start-day');
		$startMonth = $_Form->GetValue('start-month');
		$startYear = $_Form->GetValue('start-year');
		$endDay = $_Form->GetValue('end-day');
		$endMonth = $_Form->GetValue('end-month');
		$endYear = $_Form->GetValue('end-year');
		$startDate = "$startYear-$startMonth-$startDay";
		$endDate = "$endYear-$endMonth-$endDay";
	}
	
	$sql = "SELECT orders.orid,orders.csid,orders.order_number,
			customers.cid, customers.customer_number,
			CONCAT(customers.billaddress_firstname,' ',customers.billaddress_lastname) AS customer_name,
			orders.transaction_date as order_date,
			orders.paid, orders.status,
			orders.subtotal FROM orders,customers";
	$where = " WHERE orders.cid = customers.cid";
	
	if(!empty($_REQUEST['order_number'])){
		$orderNum = trim($_REQUEST['order_number']);
		$where .= " AND orders.order_number = '$orderNum'";
	}
	elseif(!empty($_REQUEST['orid'])){
		$orid = trim($_REQUEST['orid']);
		$where .= " AND orders.orid = '$orid'";
	}
	elseif(!empty($_GET['cid'])){
		$cid = trim($_GET['cid']);
		$where .= " AND customers.cid = '$cid'";
	}
	elseif(!empty($_REQUEST['customer_number'])){
		$custNum = trim($_REQUEST['customer_number']);
		$where .= " AND customers.customer_number = '$custNum'";
	}
	else{
		$where .= " AND transaction_date >= '$startDate' AND transaction_date <= '$endDate'";
	}
	
	$orderstatus = $_Form->GetValue('orderstatus','all');
	$paidStatus = $_Form->GetValue('paidstatus','all');
	if($orderstatus != "all"){
		$where .= " AND orders.status = '$orderstatus'";
	}
	if($paidStatus != "all"){
		$where .= " AND orders.paid = '$paidStatus'";
	}
	
	$max = $maxToDisplay;
	if(!empty($_REQUEST['max']) && is_numeric($_REQUEST['max'])){
		$max = intval($_REQUEST['max']);
	}
	
	$countsql = "SELECT COUNT(orid) as count, SUM(grandtotal) as queryTotal FROM orders,customers $where";
	$queryTotals = $_DB->getRecords($countsql);
	
	if($debug){
		$_Common->debugPrint($queryTotals,$countsql);
	}
	
	$count = 0;
	$queryTotal = 0;
	if(isset($queryTotals[0]['count']) && $queryTotals[0]['count'] > 0){
		
		print "<p><a href=\"javascript:selectAll(document.forms['order'],true);\">Select All</a>&nbsp;-&nbsp;\n";
		print "<a href=\"javascript:selectAll(document.forms['order'],false);\">Unselect All</a></p>";
		
		$count = $queryTotals[0]['count'];
		$queryTotal = $queryTotals[0]['querytotal'];
		
		if($max > $count){
			$max = $count + 1;
		}
		$hits = 0;
		if(isset($_REQUEST['hits']) && is_numeric($_REQUEST['hits']) && intval($_REQUEST['hits']) > 0){
			$hits = intval($_REQUEST['hits']);
		}
		
		list($start,$end,$limit) = $_DB->getLimits($count,$max,"orders.php");
		
		if($debug){
			$_Common->debugPrint("Limit: $limit, Count: $count, Hits: $hits, Max: $max");
		}
		
		$links = "";
		if($count > $max){
			$queryStr = "";
			$flds = array('searchCriteria',
						  'start-month',
						  'start-day',
						  'start-year',
						  'end-month',
						  'end-day',
						  'end-year',
						  'paidstatus',
						  'orderstatus',
						  'order_number',
						  'customer_number',
						  'search');
			
			foreach($_REQUEST as $key=>$value){
				if(!in_array($key,$flds)){
					continue;	
				}
				if((!is_array($key) || !is_array($value)) && $key != "hits"){
					$_DB->queryVars[$key] = $value;
				}
			}
			//$_Common->debugPrint($_DB->queryVars,"Queryvars");
			$_DB->createPreviousNextLinks($hits,$count,$max,"orders.php");
			$links = $_DB->previousNextLinks; 
		}
		
		
		
		$where .= " ORDER BY order_number,transaction_date $limit";
		$sql .= $where;
		$records = $_DB->getRecords($sql);
		
		if($debug){
			$_Common->debugPrint($records,$sql);
		}
		
		if(count($records) > 0){
			
			//oridList is list of order ids
			$oridList = array();
			foreach($records as $i=>$orderData){
				$oridList[] = $orderData['orid'];
			}
			$orderIds = "'" . join("','",$oridList) . "'";
			
			//Get shipping, sales tax, discounts, etc for the displayed records
			
			$sql = "SELECT SUM(subtotal) as subtotal,
					SUM(discount) as 'order discount',
					SUM(salestax) as 'sales tax',
					SUM(shipping) as shipping,
					SUM(insurance) as insurance,
					SUM(grandtotal) as 'page total'
					FROM orders WHERE orid IN($orderIds)";
			$totals = $_DB->getRecords($sql);
			
			if($debug){
				$_Common->debugPrint($totals,"Totals");
			}
			
			$totals[0]['Report Total'] = $queryTotal;
		}
		
	}
	else{
		$records = array();
	}
}
// -------------------------------------------------------------------
function getDetailRecords(){
	
	global $adjustPricesToDatabasePrices;
	global $_CF;
	global $_Common;
	global $_DB;
	global $debug;
	global $orderNum;
	global $orid;

//$debug = true;
	
	$sVersion = $_DB->serverVersion;
	$crypt_key = $_CF['encryption']['security_phrase'];

	$where = "";

	if(!empty($_REQUEST['orid'])){
		$orid = $_REQUEST['orid'];
		$where = "WHERE orid = '$orid'";
	}
	elseif(!empty($_REQUEST['order_number'])){
		$orderNum = $_REQUEST['order_number'];
		$where = "WHERE order_number = '$orderNum'";
	}

	if($sVersion >= 40002){
		$ordersql = "SELECT DATE_FORMAT(orders.transaction_date,'%c-%d-%Y') AS order_date,
							TIME_FORMAT(orders.transaction_time,'%l:%i %p') AS order_time,
							orders.*,
							AES_DECRYPT(orders.card_number,'$crypt_key') as card_number,
							AES_DECRYPT(orders.cvv2,'$crypt_key') as cvv2
					 FROM orders $where LIMIT 1";
	}
	else{
		$ordersql = "SELECT DATE_FORMAT(orders.transaction_date,'%c-%d-%Y') AS order_date,
							TIME_FORMAT(orders.transaction_time,'%l:%i %p') AS order_time,
							orders.*,
							DECODE(orders.card_number,'$crypt_key') as card_number,
							DECODE(orders.cvv2,'$crypt_key') as cvv2
					 FROM orders $where LIMIT 1";
	}
	
	global $order;
	$order = $_DB->getRecords($ordersql);
	
	if($debug){
		$_Common->debugPrint($order,$ordersql);
	}
	
	if(count($order) > 0){
		
		$orderInfo = $order[0];
		$orid = $order[0]['orid'];
		$cid = $order[0]['cid'];
		$csid = $order[0]['csid'];
		
		if(!is_numeric($order[0]['cvv2'])){
			$order[0]['cvv2'] = null;
		}
		
		$orderFields = $_DB->getFieldProperties('orders');
		$orderDetailFields = $_DB->getFieldProperties('order_details');
		$customerFields = $_DB->getFieldProperties('customers');
		$customerShipFields = $_DB->getFieldProperties('customer_shipping');
		
		//Get order details
		$orderDetailSql = "SELECT * FROM order_details WHERE order_details.orid = '$orid'";
		global $orderDetails;
		$orderDetails = $_DB->getRecords($orderDetailSql,"ordid");

		//$_Common->debugPrint($orderDetails,$orderDetailSql);
		//exit;

		if($adjustPricesToDatabasePrices){
			foreach($orderDetails as $did=>$row){
				if(!isset($row['price'])){
					$orderDetails[$did]['price'] = getProductPrice($row['sku'],$row['price'],$row['quantity']);
				}
			}
		}

		// Add order options and adjust price
		$subtotal = checkOrderOptions($orderDetails);

		
		if($debug){
			$_Common->debugPrint($orderDetails,$orderDetailSql);
		}
		
		// Get customer details
		$custSql = "SELECT * FROM customers WHERE cid = '$cid'";
		$cust = $_DB->getRecords($custSql);
		
		global $billInfo;
		$billInfo = $cust[0];
		
		if($debug){
			$_Common->debugPrint($billInfo,$custSql);
		}
		
		//print customer shipping data
		$custShipSql = "SELECT * FROM customer_shipping WHERE csid = '$csid'";
		$custShipping = $_DB->getRecords($custShipSql);
		
		global $shipInfo;
		if(!empty($custShipping[0])){
			$shipInfo = $custShipping[0];
		}
		else{
			//print "csid = $csid";
			// they must have deleted the shipping record
			$shippingData = $_DB->getRecords("SELECT * FROM customer_shipping WHERE cid = '$cid'");
			if(count($shippingData) > 0){
				$shipInfo = $shippingData[0];
				foreach($shippingData as $k=>$row){
					if($row['primary_address'] == 'true'){
						unset($shipInfo);
						$shipInfo = $shippingData[$k];
						break;
					}	
				}
			}
		}

		if($debug){
			$_Common->debugPrint($shipInfo,$custShipSql);
		}
		
		$sCountry = "US";
		if(isset($shipInfo['shipaddress_country'])){
			$sCountry = strtoupper($shipInfo['shipaddress_country']);
		}
		if($sCountry == "CA"){
			$sql = "SELECT SUM(subtotal),SUM(discount),SUM(gst),SUM(hst),SUM(pst),SUM(shipping),SUM(insurance),SUM(grandtotal) FROM orders WHERE orid = '$orid'";
		}
		else{
			$sql = "SELECT SUM(subtotal),SUM(discount),SUM(salestax),SUM(shipping),SUM(insurance),SUM(grandtotal) FROM orders WHERE orid = '$orid'";						
		}
		global $totals,$optionTotal;
		$totals = $_DB->getRecords($sql);
		
		if(isset($optionTotal) && $optionTotal > 0){
			//$totals[0]['sum(subtotal)'] = $_Common->format_price($totals[0]['sum(subtotal)'] + $optionTotal,true);
			$totals[0]['sum(subtotal)'] = $_Common->format_price($totals[0]['sum(subtotal)'],true);
		}
		
		if($debug){
			$_Common->debugPrint($optionTotal,"Options Total");
			$_Common->debugPrint($totals,$sql);
		}
	}	
}


//-------------------------------------------------------------------
function checkOrderOptions(&$orderDetails){
	
	global $_Common,$_DB,$optionTotal,$totals;
	
	//$_Common->debugPrint($orderDetails);
	//$_Common->debugPrint($totals);
	
	$subtotal = 0;
	
	// Add order options
	foreach($orderDetails as $ordid=>$orderFlds){

		if(strstr($orderFlds['price'],':')){
			$orderDetails[$ordid]['price'] = $_Common->calculateQuantityPrice($orderFlds['price'],$orderFlds['quantity']);
		}
		
		$quantity = $orderDetails[$ordid]['quantity'];
		
		$orderOptionSql = "SELECT name,price,value,weight,type FROM order_options WHERE ordid = '$ordid'";
		$optionDetails = $_DB->getRecords($orderOptionSql);
		//$_Common->debugPrint($optionDetails);

		$optionStr = "";
		$orderDetails[$ordid]['txtname'] = $orderFlds['name'];
		
		foreach($optionDetails as $i=>$option){
			
			// qty price for options
			if(strstr($option['price'],":")){
				$option['price'] = $_Common->calculateQuantityPrice($option['price'],$quantity);
			}
			if($option['price'] > 0){
				$optionPrice = 0;
				// one time charge
				if($option['type'] == "setup"){
					$optionPrice = $_Common->format_price($option['price'],false);
				}
				// Standard option
				elseif($option['type'] == "option"){
					//$optionPrice = $_Common->format_price($option['price'] * $quantity,false);
					$optionPrice = $_Common->format_price($option['price'],false);
				}
//				$orderDetails[$ordid]['price'] += $optionPrice;
				$optionTotal += $optionPrice;
			}

			$optionStr .= ", ";
			if($option['name'] != "" && $option['name'] == "Option"){
				$optionStr .= "Option " . ($i+1) . ":";
			}
			elseif($option['name'] != "" && $option['name'] != "Option"){
				$optionStr .= $option['name'] . ":";	
			}
			$optionStr .= " " . $option['value'];
			
			if($option['price'] != "0"){
				if($option['type'] == "option"){
					$optionStr .= " (" . $option['price'] . " each)";
				}
				elseif($option['type'] == "setup"){
					$optionStr .= " (Setup Charge: " . $option['price'] . ")";
				}
				else{
					$optionStr .= " " . $option['price'];
				}
			}
			
			$orderDetails[$ordid]['price'] = $_Common->format_price($orderDetails[$ordid]['price'],false);
			$orderDetails[$ordid]['options'][] = $option;
		}
		
		$orderDetails[$ordid]['txtname'] .= $optionStr;
		
		$basePrice = $_Common->format_price($orderDetails[$ordid]['price'] + $optionTotal);
		
		$orderDetails[$ordid]['line_total'] = $_Common->format_price($basePrice * $orderDetails[$ordid]['quantity']);
		
		$subtotal += $orderDetails[$ordid]['line_total'];
		
	}
	return $_Common->format_price($subtotal,true);
}

// --------------------------------------------------------------
function setLastQuery(){
	
	if(	isset($_REQUEST['detail']) || 
			isset($_REQUEST['update']) ||
			isset($_REQUEST['delete'])){
		return;
	}

	$flds = array('searchCriteria',
				  'start-month',
				  'start-day',
				  'start-year',
				  'end-month',
				  'end-day',
				  'end-year',
				  'paidstatus',
				  'orderstatus',
				  'order_number',
				  'customer_number');

	
	$qryStr = "orders.php?";
	$i = 0;
	foreach($_REQUEST as $key=>$val){
		
		if(!in_array($key,$flds)){
			continue;	
		}
		if(!is_array($val)){
			if($i == 0){
				$qryStr .= "$key=$val";
			}
			else{
				$qryStr .= "&amp;" . "$key=$val";
			}
			$i++;
		}
	}
	if($i > 0){
		$_SESSION['last_query'] = $qryStr;
	}
}
// -----------------------------------------------------------------
function getLastQuery(){
	global $_Common;
	//$_Common->debugPrint($_SESSION['last_query'],"last query");
	if(isset($_SESSION['last_query'])){
		$params = explode('?',$_SESSION['last_query']);
		$a = explode('&amp;', $params[1]);
		$i = 0;
		while($i < count($a)) {
			$b = preg_split('/=/', $a[$i]);
			$key = htmlspecialchars(urldecode($b[0]));
			$val = htmlspecialchars(urldecode($b[1]));
			$i++;
			$_REQUEST[$key] = $val;
			//$_Common->debugPrint("$key = $val");
		}
	}
}


//-------------------------------------------------------------------
function doUpdates(){
	
	global $_Common,$_DB;
	
	$sendTrackingEmail = false;
	$trackingInfo = array();
	if(!empty($_REQUEST['email_tracking_number']) && $_REQUEST['email_tracking_number'] == "true"){
		$sendTrackingEmail = true;
	}
	
	if(isset($_REQUEST['orders'])){
		foreach($_REQUEST['orders'] as $orid=>$fields){
			if(isset($fields['tracking_number']) && is_array($fields['tracking_number'])){
				$nums = array();
				foreach($fields['tracking_number'] as $t=>$num){
					if(trim($num) != ""){
						$nums[] = $num;
					}
				}
				if(count($nums) > 0){
					$_REQUEST['orders'][$orid]['tracking_number'] = join(',',$fields['tracking_number']);
				}
				else{
					$_REQUEST['orders'][$orid]['tracking_number'] = '';	
				}
			}
		}
	}
	
	$hasChanged = false;
	$orderNumbers = array();

//$_Common->debugPrint($_REQUEST);
//exit;

	
	foreach($_REQUEST as $table=>$data){
		
		// We only want the table fields
		if(!is_array($data)){
			continue;	
		}
		
		$idFld = "";
		switch($table){
			case 'orders':
				$idFld = "orid";
				break;
			case 'order_details':
				$idFld = "ordid";
				break;
			case 'customers':
				$idFld = "cid";
				break;
			case 'customer_shipping':
				$idFld = "csid";
				break;
		}
		
		foreach($data as $id=>$records){
			
			$sql = null;
			
			// if updating from the summary page, only do selected ones
			if(isset($_REQUEST['summary'])){
				$_REQUEST['detail'] = NULL;
				unset($_REQUEST['detail']);
				if(!empty($_SESSION['last_query'])){
					getLastQuery();
				}
				if(!isset($records['selected'])){
					continue;
				}
			}
			else{
				$_REQUEST['detail'] = true;
			}
			
			if($sendTrackingEmail && $table == "customers"){
				$trackingInfo['billaddress_email'] = $records['billaddress_email'];
			}
			if($sendTrackingEmail && $table == "orders"){
				$trackingInfo['order_date'] = $records['transaction_date'];
				$trackingInfo['order_number'] = $records['order_number'];
				$trackingInfo['date_shipped'] = $records['date_shipped'];
				$trackingInfo['tracking_number'] = $records['tracking_number'];
			}
			
			$fieldProperties = $_DB->getFieldProperties($table);
			
			//$_Common->debugPrint($fieldProperties);
			//$_Common->debugPrint($records,"Records");
							
			if($table == 'order_details' && $id == 'add' && $records['quantity'] != ''){
				
				if(!empty($_REQUEST['orid'])){
					$records['orid'] = $_REQUEST['orid'];
				
					list($flds, $values) = $_DB->makeAddFields($fieldProperties,$idFld,$records);
					$sql = "INSERT INTO $table ($flds) VALUES ($values)";
				}
			}
			else{
				$flds = $_DB->makeUpdateFields($fieldProperties,$idFld,$records);
				$where = "$idFld = '$id'";
				$sql = "UPDATE $table SET $flds WHERE $where";
			}

//$_Common->debugPrint($sql);
//exit;
			if($sql){
				$_DB->execute($sql);
			}

			if($sendTrackingEmail){
				sendTrackingEmail($trackingInfo);
			}
			
			if($table == 'order_details' && !empty($records['order_number'])){
				$orderNumbers[] = $records['order_number'];
			}
			elseif($table == 'order_details' && !empty($_REQUEST['order_number'])){
				$orderNumbers[] = $_REQUEST['order_number'];
			}
			if($table == 'orders' && !empty($records['order_number'])){
				$orderNumbers[] = $records['order_number'];
			}
		}
	}
}

//-------------------------------------------------------------------
function sendTrackingEmail($trackingInfo){
	
	global $_CF;
	global $_Common;
	global $_Registry;
	
	$storeName = $_CF['basics']['store_name'];			
	$storeEmailAddress = $_CF['email']['store_owner_email_address'];
	$customerEmail = $trackingInfo['billaddress_email'];
	
	if($_CF['email']['send_email']){
		$email = $_Registry->loadClass('email');
		$template = $_Registry->loadClass("template");
		$vars = array("storeName"	=> $storeName,
				"trackingInfo" => $trackingInfo);
		$HTML = $template->open("../templates/email.tracking.html",$vars);
		
		$subject = "Shipping Confirmation";
		
		if(preg_match("|<title>(.*)</title>|i",$HTML)){
			$match = preg_match("|<title>(.*)</title>|i",$HTML,$matches);
			$subject = $matches[1];
		}
		
		list($html,$text) = split("<text>",$HTML);
		
		$email->send($storeEmailAddress, $customerEmail, $subject, $html, $text);
		
		if($email->sent){
			$_Common->sendMessage("Tracking Email Sent");
		}
		else{
			$_Common->sendMessage("Tracking Email Failed");
		}
	}
}

//-------------------------------------------------------------------
function doDeletes(){
	
	global $_Common;
	global $_DB;
	
	if(isset($_REQUEST['deleteDetails']) && isset($_REQUEST['order_details'])){

		//$_Common->debugPrint($_REQUEST['order_details']);

		foreach($_REQUEST['order_details'] as $ordid=>$flds){
			if(isset($flds['delete'])){
				
				$sql = "DELETE FROM order_details WHERE ordid = '$ordid'";
				$_DB->execute($sql);
				
				$sql = "DELETE FROM order_options WHERE ordid = '$ordid'";
				$_DB->execute($sql);
				
				$_REQUEST['order_details'][$ordid] = NULL;
				unset($_REQUEST['order_details'][$ordid]);
				reset($_REQUEST['order_details']);
				
				if(count($_REQUEST['order_details']) > 0 && isset($_REQUEST['orid'])){
					$lineTotal = $_Common->format_price($flds['price'] * $flds['quantity']);
					$where = "orid = '" . $_REQUEST['orid'] . "'";
					$sql = "UPDATE orders SET subtotal = (subtotal - $lineTotal), grandtotal = (grandtotal - $lineTotal) WHERE $where";
					$_DB->execute($sql);
				}
			}
		}
		if(count($_REQUEST['order_details']) == 0){
			$_Common->printErrorMessage("All Details Removed","All of the details for this order have been removed? This order should be deleted...");	
		}
	}
	elseif(isset($_REQUEST['deleteOrder']) && !empty($_REQUEST['orid'])){
		$id = $_REQUEST['orid'];
		$sql = "DELETE FROM orders WHERE orid = '$id'";
		$_DB->execute($sql);
		
		$sql = "SELECT ordid FROM order_details WHERE orid = '$id'";
		$orderDetails = $_DB->getRecords($sql,"ordid");
		foreach($orderDetails as $ordid=>$fld){
			$sql = "DELETE FROM order_options WHERE ordid = '$ordid'";
			$_DB->execute($sql);
		}
		$sql = "DELETE FROM order_details WHERE orid = '$id'";
		$_DB->execute($sql);
		
		// go back to summary
		unset($_REQUEST['order_number']);
		unset($_REQUEST['orid']);
		unset($_REQUEST['detail']);
		$_REQUEST['summary'] = "true";
		getLastQuery();
		
	}
	else{
		foreach($_REQUEST as $table=>$data){
			// We only want the table fields
			if(!is_array($data)){
				continue;	
			}
			$idFld = NULL;
			switch($table){
				case 'orders':
					$idFld = "orid";
					break;
				case 'order_details':
					$idFld = "ordid";
					break;
			}
			if($idFld){
				if(isset($_REQUEST['order_details'])){
					foreach($data as $id=>$flds){
						$sql = "DELETE FROM $table WHERE $idFld = '$id'";
						//$_Common->debugPrint($sql);
						$_DB->execute($sql);
						if($idFld == "ordid"){
							$sql = "DELETE FROM order_options WHERE $idFld = '$id'";
						}
						//$_Common->debugPrint($sql);
						$_DB->execute($sql);
					}
				}
				else{
					// summary screen
					foreach($data as $id=>$flds){
						if(isset($flds['selected']) && $flds['selected'] == "true"){
							$sql = "DELETE FROM $table WHERE $idFld = '$id'";
							//$_Common->debugPrint($sql);
							$_DB->execute($sql);
							
							$sql = "SELECT ordid FROM order_details WHERE orid = '$id'";
							$orderDetails = $_DB->getRecords($sql,"ordid");
							foreach($orderDetails as $ordid=>$fld){
								$sql = "DELETE FROM order_options WHERE ordid = '$ordid'";
								//$_Common->debugPrint($sql);
								$_DB->execute($sql);
							}
							$sql = "DELETE FROM order_details WHERE orid = '$id'";
							//$_Common->debugPrint($sql);
							$_DB->execute($sql);
						}
					}
				}
			}
		}
		$_REQUEST['detail'] = NULL;
		unset($_REQUEST['detail']);
		$_REQUEST['summary'] = "true";
		getLastQuery();
	}
}

?>