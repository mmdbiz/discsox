<?php
//VersionInfo:Version[3.0.1]

$_isAdmin = true;
$_adminFunction = "reports";

// initialize the program, read the config(s) and set include paths
include_once("../../include/initialize.inc");
$init = new Initialize(true);

$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

$_Form = $_Registry->LoadClass("form");
include_once("include/orders.inc");

$debug = false;
$fldProperties = $_DB->getFieldProperties('orders');
$maxToDisplay = 25;
$hits = 0;
$start = 0;
$startDate = null;
$end = 0;
$endDate = null;
$count = 0;
$links = "";
$records = array();
$totals = array();

// For Pull downs
global $provinces;
global $states;
global $countries;
include_once("../../include/countries.inc");
include_once("../../include/provinces.inc");
include_once("../../include/states.inc");

if(isset($_REQUEST['delete']) || isset($_REQUEST['deleteOrder']) || isset($_REQUEST['deleteDetails'])){
	doDeletes();
}
if(isset($_REQUEST['update']) || isset($_REQUEST['updateOrder'])){
	doUpdates();
}
error_reporting(E_ALL ^ E_NOTICE);
?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<title>Order Report</title>
<script	LANGUAGE="JavaScript">
//<!--
sWidth = screen.width;
var	styles = "admin.800.css";
if(sWidth >	850){
	styles = "admin.1024.css";
}
if(sWidth >	1024){
	styles = "admin.1152.css";
}
if(sWidth >	1100){
	styles = "admin.1280.css";
}

document.write('<link rel="stylesheet" href="../stylesheets/' + styles	+ '" type="text/css">');
//-->
</script>
<script LANGUAGE="JavaScript" src="../javascripts/reports.js"></script>
<SCRIPT LANGUAGE="javascript">
{
self.name="content"
}
function calpopup(Ink){
	window.open(Ink,"calendar","height=250,width=250,scrollbars=no")
}
function setShipDate(val,orid){

	var fldName = 'orders[' + orid + '][date_shipped]';
	// yyyy-mm-dd
	var	today =	new	Date();
	var	dateNow	 = today.getDate();
	var	monthNow = today.getMonth() + 1;
	var	yearNow	 = today.getYear();

	if(monthNow < 10){
		monthNow = "0" + monthNow;
	}
	if(dateNow < 10){
		dateNow = "0" + dateNow;
	}

	if(val == "Partial Shipped" || val == "Complete"){
		if(document.forms['order'].elements[fldName].value == '0000-00-00'){
			document.forms['order'].elements[fldName].value = yearNow + '-' + monthNow + '-' + dateNow;
		}
	}
	else if(val == "Not Shipped" || val == "Cancelled"){
		document.forms['order'].elements[fldName].value = '0000-00-00';
	}
}

function showLineTotal(form,qtyFld,priceFld,fldid,orid){

	if(form.elements[priceFld].value != '' && form.elements[qtyFld].value != ''){
	
		var optionTotalFld = 'order_details['+fldid+'][option_total]';
		var optionTotal = parseFloat(form.elements[optionTotalFld].value);
		
		var productPrice = parseFloat(form.elements[priceFld].value);
		
		//var lineTotal = parseFloat(productPrice) * parseInt(form.elements[qtyFld].value);
		var lineTotal = parseFloat(productPrice + optionTotal) * parseInt(form.elements[qtyFld].value);
		
		document.getElementById('order_details['+fldid+'][total]').innerHTML = lineTotal.toFixed(2);
		updateTotals(form,orid);
	}
}

function updateTotals(form,orid){

	var qty = parseInt(0);
	var price = parseFloat(0);
	var subtotal = parseFloat(0);
	var optionTotal = parseFloat(0);

	for(i=0;i<form.elements.length;i++){
		
		var str = form.elements[i].id;

		if(str.indexOf('order_details') != -1 && str.indexOf('quantity') != -1){
			if(form.elements[i].value != ''){
				qty = parseInt(form.elements[i].value);
			}
		}
		if(str.indexOf('order_details') != -1 && str.indexOf('price') != -1){
			if(form.elements[i].value != ''){
				price = parseFloat(form.elements[i].value);
			}
		}
		if(str.indexOf('order_details') != -1 && str.indexOf('option_total') != -1){
			if(form.elements[i].value != ''){
				optionTotal = parseFloat(form.elements[i].value);
			}
		}
		if(qty > 0 && price > 0){
			subtotal = subtotal + (parseFloat(price) + parseFloat(optionTotal)) * parseInt(qty);
			//subtotal = subtotal + parseFloat(price) * parseInt(qty);
			qty = parseInt(0);
			price = parseFloat(0);
			optionTotal = parseFloat(0);
		}
	}
	
	if(subtotal > 0){
		form.elements['orders['+orid+'][subtotal]'].value = subtotal.toFixed(2);
	
		var flds = new Array('discount','salestax','shipping','insurance');
		var grandTotal = parseFloat(subtotal);
	
		for(j=0;j<flds.length;j++){
			if(form.elements['orders['+orid+']['+flds[j]+']'].value != ''){
	
				if(flds[j] == 'discount'){
					grandTotal = grandTotal - parseFloat(form.elements['orders['+orid+']['+flds[j]+']'].value);
				}
				else{
					grandTotal = grandTotal + parseFloat(form.elements['orders['+orid+']['+flds[j]+']'].value);
				}
			}
		}
		form.elements['orders['+orid+'][grandtotal]'].value = parseFloat(grandTotal).toFixed(2);
	}	
}


</SCRIPT>
<script LANGUAGE="JavaScript" src="../javascripts/popcalendar.js"></script>
</head>
<body class="mainBody">

	<div align=center valign=top>

	<form name="order" method="post" action="orders.php">

		<?php if(empty($_REQUEST['detail'])):?>
		
			<input type="hidden" name="summary" value="true">
		
			<table border="0" cellpadding="3" cellspacing="1">
				<tr><th colspan="3" align="left">Search for orders</th></tr>
				<tr bgcolor="#e2eDe2">
					<td align="left" valign="middle"> Dates: <?=makeCriteriaSelect();?></td>
					<td align=center>
						From: <?=dateSelect(true);?> &nbsp;&nbsp;
						To:   <?=dateSelect(false);?>
					</td>
					<td align="center" valign="middle">Paid: <?=makePaidStatusSelect();?></td>
				</tr>
				<tr bgcolor="#e2eDe2">
					<td align="left" valign="middle">Status: <?=makeOrderStatusSelect();?></td>
					<td align="left" valign="middle">
						Order #: <input type="text" name="order_number" size="15"> &nbsp;
						Customer #: <input type="text" name="customer_number" size="15">
					</td>
					<td align="center" valign="middle">
						<input type="Submit" name="search" value="Search">
					</td>
				</tr>
			</table>

			<?php getSummaryRecords();?>
			<?php if(count($records) > 0):?>

				<?php
					$color = array();
					$color[0] = "#e2eDe2";
					$color[~0] = "#FFFFFF";
					$ck = 0;
					$primaryKey = "orid";
					$fldCount = count(array_keys($records[0])) - 1;
					$cid = "";
				?>

				<table border="0" cellpadding="2" cellspacing="1" width="98%" ID="Table1">
				<tr bgcolor="#F5F5F5">
					<td colspan="2" align="left">
						<?="$start - $end of $count Records";?>
					</td>
					<td colspan="5" align="right">
						<?=$links;?>
					</td>
				</tr>
				<tr>
					<th nowrap>Select</th>
					<th nowrap>Order #</th>
					<th nowrap>Customer</th>
					<th nowrap>Order Date</th>
					<th nowrap>Paid</th>
					<th nowrap>Status</th>
					<th nowrap>Subtotal</th>
				</tr>
				
				<?php foreach($records as $index=>$data):?>
					<?php
						$id = $data[$primaryKey];
						$csid = null;
						
						if(isset($data['csid'])){
							$csid = $data['csid'];	
						}
						
						foreach($data as $key=>$value){
							// loads the default values from the table for enum fields
							$fldname = "orders[" . $id . "][" . $key . "]";
							$data[$key] = $_DB->getDefaultValues($key,$value,true,$fldname);
						}
					?>
					
					<tr bgcolor="<?=$color[$ck = ~$ck];?>">
						<td align="center" width="50">
							<input type="checkbox" name="orders[<?=$id;?>][selected]" value="true">
							<input type="hidden" name="orders[<?=$id;?>][order_number]" value="<?=$data['order_number'];?>">
						</td>
						<td align=center>
							<a href="orders.php?orid=<?=$data['orid'];?>&amp;order_number=<?=$data['order_number'];?>&amp;detail=true"><u><?=$data['order_number'];?></u></a>
						</td>
						<td align=center>
							<a href="customers.php?modify=1&amp;cid=<?=$data['cid'];?>&amp;customer_number=<?=$data['customer_number'];?>&csid=<?=$csid;?>"><u><?=$data['customer_name'];?></u></a>
						</td>
						<td align=center><?=$data['order_date'];?></td>
						<td align=center><?=$data['paid'];?></td>
						<td align=center><?=$data['status'];?></td>
						<td align=right><?=$data['subtotal'];?></td>
					</tr>

				<?php endforeach;?>

				<?php if(count($totals) > 0):?>
					<tr>
						<td colspan="6" align="right">&nbsp;</td>
						<td align=right>========</td>
					</tr>
					<?php foreach($totals[0] as $key=>$sum):?>
						<?php $label = ucwords(str_replace("_"," ",$key));?>
						<tr>
							<td colspan="6" align="right"><?=$label;?></td>
							<td align=right><?=number_format($sum,2);?></td>
						</tr>
					<?php endforeach;?>
				<?php endif;?>
				
				</table>
				<p>
					<input type="submit" name="update" value="Update Selected Orders" onClick="return checkSelected(document.forms['order'],'update','order');">
					<input type="submit" name="delete" value="Delete Selected Orders" onClick="return checkSelected(document.forms['order'],'delete','order');">
				</p>

			<?php else:?>
				<?php if(!empty($_REQUEST['order_number'])):?>
					<p>-- Order Number <?=$_REQUEST['order_number'];?> was not found --</p>
				<?php elseif(!empty($_REQUEST['customer_number'])):?>
					<p>-- An order for customer number <?=$_REQUEST['customer_number'];?> was not found --</p>
				<?php elseif(isset($_REQUEST['searchCriteria'])):?>
					<p>-- No orders to display for <?=$startDate;?> to <?=$endDate;?> --</p>
				<?php else:?>
					<p>-- No orders to display for today --</p>
				<?php endif;?>
			<?php endif;?>


		<?php elseif(!empty($_REQUEST['detail'])):?>

			<input type="hidden" name="detail" value="true">

			<?php getDetailRecords();?>

			<?php if(count($order) > 0):?>

				<?php
					$color = array();
					$color[~0] = "#e2eDe2";
					$color[0] = "#FFFFFF";
					$ck = 0;
					
					$order_number = $order[0]['order_number'];
					$orid = $order[0]['orid'];
					$cid = $order[0]['cid'];
					$csid = $order[0]['csid'];
					
					$payMethod = null;
					if(!empty($order[0]['payment_method'])){
						@list($payMethod,$ext) = explode('.',$order[0]['payment_method']);
					}
				?>
			
				<table border="0" cellpadding="2" cellspacing="0" width="90%">
					<tr>
						<td>
							<!--input type=hidden name="order_number" value="<?=$order_number;?>"-->
							<input type=hidden name="orid" value="<?=$orid;?>">
							
							<style>
							th{
								background-color: #E5E5E5;
								color: #000000;
								font-family: Arial, Helvetica, sans-serif;
								font-size: 12px;
							}
							.topAlign{
								vertical-align:top;
							}
							</style>

							<table border="0" cellpadding="1" cellspacing="2" width="100%" align="left" ID="Table4">
								<tr>
									<td align="left"><b>Order Date:</b> <?=$order[0]['order_date'];?> - <?=$order[0]['order_time'];?></td>
									<td>&nbsp;</td>
									<td align="right"><b>Order Number:</b> <input type=text name="orders[<?=$orid;?>][order_number]" value="<?=$order_number;?>" size="10" style="text-align:right"></td>
								</tr>
								<tr>
									<th align="left" style="padding-left:2px;">Bill To:</th>
									<th align="left" style="padding-left:2px;">
										<?php if(!empty($shipInfo['shipaddress_addr1'])):?>
											Ship To:
										<?php else:?>
											&nbsp;
										<?php endif;?>
									</th>
									<th align="left" style="padding-left:2px;">Payment Info:</th>
								</tr>
								<tr><td colspan="3" style="line-height:5px;">&nbsp;</td></tr>
								<tr>
									<td align="left" class="topAlign">
										<table border="0" cellspacing="0" cellpadding="2" width="100%">
											<tr>
												<td align="right" nowrap>Company: </td>
												<td align="left">
													<input type="text" name="customers[<?=$cid;?>][billaddress_companyname]" value="<?=$billInfo['billaddress_companyname'];?>" size="25">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>First Name: </td>
												<td align="left">
													<input type="text" name="customers[<?=$cid;?>][billaddress_firstname]" value="<?=$billInfo['billaddress_firstname'];?>" size="25">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>Last Name: </td>
												<td align="left">
													<input type="text" name="customers[<?=$cid;?>][billaddress_lastname]" value="<?=$billInfo['billaddress_lastname'];?>" size="25">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>Address: </td>
												<td align="left">
													<input type="text" name="customers[<?=$cid;?>][billaddress_addr1]" value="<?=$billInfo['billaddress_addr1'];?>" size="25">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>Address2: </td>
												<td align="left">
													<input type="text" name="customers[<?=$cid;?>][billaddress_addr2]" value="<?=$billInfo['billaddress_addr2'];?>" size="25">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>City: </td>
												<td align="left">
													<input type="text" name="customers[<?=$cid;?>][billaddress_city]" value="<?=$billInfo['billaddress_city'];?>" size="25">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>
													<?php if($billInfo['billaddress_country'] == "CA"):?>
														Province: 
													<?php else:?>
														State:
													<?php endif;?>
												</td>
												<td align="left">
													<?php if($billInfo['billaddress_country'] == "US"):?>
													<input type="text" name="customers[<?=$cid;?>][billaddress_state]" value="<?=$billInfo['billaddress_state'];?>" size="2">
													<?php else:?>
													<input type="text" name="customers[<?=$cid;?>][billaddress_state]" value="<?=$billInfo['billaddress_state'];?>" size="25">
													<?php endif;?>
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>Postal Code: </td>
												<td align="left">
													<input type="text" name="customers[<?=$cid;?>][billaddress_postalcode]" value="<?=$billInfo['billaddress_postalcode'];?>" size="9">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>Country: </td>
												<td align="left">
													<input type="text" name="customers[<?=$cid;?>][billaddress_country]" value="<?=$billInfo['billaddress_country'];?>" size="2">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>Phone: </td>
												<td align="left">
													<input type="text" name="customers[<?=$cid;?>][billaddress_areacode]" value="<?=$billInfo['billaddress_areacode'];?>" size="3">
													<input type="text" name="customers[<?=$cid;?>][billaddress_phone]" value="<?=$billInfo['billaddress_phone'];?>" size="8">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>Email: </td>
												<td align="left">
													<input type="text" name="customers[<?=$cid;?>][billaddress_email]" value="<?=$billInfo['billaddress_email'];?>" size="25">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>Email List: </td>
												<td align="left">
													<?php $fldname = "orders[" . $orid . "][email_list]";?>
													<?=$_DB->getDefaultValues('email_list',$order[0]['email_list'],true,$fldname);?>
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>Paid Status: </td>
												<td align="left">
													<?php $fldname = "orders[" . $orid . "][paid]";?>
													<?=$_DB->getDefaultValues('paid',$order[0]['paid'],true,$fldname);?>
												</td>
											</tr>
										</table>
										
									</td>
									<?php if(!empty($shipInfo['shipaddress_addr1'])):?>
									<td align="left" class="topAlign">
										<table border="0" cellspacing="0" cellpadding="2" width="100%">
											<tr>
												<td align="right" nowrap>Company: </td>
												<td align="left">
													<input type="text" name="customer_shipping[<?=$csid;?>][shipaddress_companyname]" value="<?=$shipInfo['shipaddress_companyname'];?>" size="25">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>First Name: </td>
												<td align="left">
													<input type="text" name="customer_shipping[<?=$csid;?>][shipaddress_firstname]" value="<?=$shipInfo['shipaddress_firstname'];?>" size="25">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>Last Name: </td>
												<td align="left">
													<input type="text" name="customer_shipping[<?=$csid;?>][shipaddress_lastname]" value="<?=$shipInfo['shipaddress_lastname'];?>" size="25">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>Address: </td>
												<td align="left">
													<input type="text" name="customer_shipping[<?=$csid;?>][shipaddress_addr1]" value="<?=$shipInfo['shipaddress_addr1'];?>" size="25">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>Address2: </td>
												<td align="left">
													<input type="text" name="customer_shipping[<?=$csid;?>][shipaddress_addr2]" value="<?=$shipInfo['shipaddress_addr2'];?>" size="25">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>City: </td>
												<td align="left">
													<input type="text" name="customer_shipping[<?=$csid;?>][shipaddress_city]" value="<?=$shipInfo['shipaddress_city'];?>" size="25">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>
													<?php if($shipInfo['shipaddress_country'] == "CA"):?>
														Province: 
													<?php else:?>
														State:
													<?php endif;?>
												</td>
												<td align="left">
													<?php if($shipInfo['shipaddress_country'] == "US"):?>
													<input type="text" name="customer_shipping[<?=$orid;?>][shipaddress_state]" value="<?=$shipInfo['shipaddress_state'];?>" size="2">
													<?php else:?>
													<input type="text" name="customer_shipping[<?=$orid;?>][shipaddress_state]" value="<?=$shipInfo['shipaddress_state'];?>" size="25">
													<?php endif;?>
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>Postal Code: </td>
												<td align="left">
													<input type="text" name="customer_shipping[<?=$csid;?>][shipaddress_postalcode]" value="<?=$shipInfo['shipaddress_postalcode'];?>" size="9">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>Country: </td>
												<td align="left">
													<input type="text" name="customer_shipping[<?=$csid;?>][shipaddress_country]" value="<?=$shipInfo['shipaddress_country'];?>" size="2">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>Phone: </td>
												<td align="left">
													<input type="text" name="customer_shipping[<?=$csid;?>][shipaddress_areacode]" value="<?=$shipInfo['shipaddress_areacode'];?>" size="3">
													<input type="text" name="customer_shipping[<?=$csid;?>][shipaddress_phone]" value="<?=$shipInfo['shipaddress_phone'];?>" size="8">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>Email: </td>
												<td align="left">
													<input type="text" name="customer_shipping[<?=$csid;?>][shipaddress_email]" value="<?=$shipInfo['shipaddress_email'];?>" size="25">
												</td>
											</tr>


										</table>
										
									</td>
									<?php else:?>
									<td>&nbsp;</td>
									<?php endif;?>
									
									<td align="left" class="topAlign">
									
										<table border="0" cellspacing="0" cellpadding="1" width="100%">

										<tr>
											<td align="right">Payment Method: </td>
											<td align="left"><?=$payMethod;?></td>
										</tr>
									
										<?php if($order[0]['name_on_card'] != ""):?>
											<tr>
												<td align="right">Card Type: </td>
												<td align="left">
													<input type="text" name="orders[<?=$orid;?>][credit_card_type]" value="<?=$order[0]['credit_card_type'];?>" size="20">
												</td>
											</tr>
											<tr>
												<td align="right">Name on Card: </td>
												<td align="left">
													<input type="text" name="orders[<?=$orid;?>][name_on_card]" value="<?=$order[0]['name_on_card'];?>" size="20">
												</td>
											</tr>
											<tr>
												<td align="right">Number: </td>
												<td align="left">
													<input type="text" name="orders[<?=$orid;?>][card_number]" value="<?=$order[0]['card_number'];?>" size="20">
												</td>
											</tr>
											<tr>
												<td align="right">Expire Date: </td>
												<td align="left">
													<input type="text" name="orders[<?=$orid;?>][expire_month]" value="<?=$order[0]['expire_month'];?>" size="2">
													<input type="text" name="orders[<?=$orid;?>][expire_year]" value="<?=$order[0]['expire_year'];?>" size="4">
												</td>
											</tr>
											<tr>
												<td align="right">CVV2: </td>
												<td align="left">
													<input type="text" name="orders[<?=$orid;?>][cvv2]" value="<?=$order[0]['cvv2'];?>" size="4">
												</td>
											</tr>
										<?php elseif($order[0]['bank_name'] != ""):?>
											<tr>
												<td align="right">Bank: </td>
												<td align="left">
													<input type="text" name="orders[<?=$orid;?>][bank_name]" value="<?=$order[0]['bank_name'];?>" size="20">
												</td>
											</tr>
											<tr>
												<td align="right">Type: </td>
												<td align="left">
													<input type="text" name="orders[<?=$orid;?>][account_type]" value="<?=$order[0]['account_type'];?>" size="20">
												</td>
											</tr>
											<tr>
												<td align="right">Account: </td>
												<td align="left">
													<input type="text" name="orders[<?=$orid;?>][account_number]" value="<?=$order[0]['account_number'];?>" size="20">
												</td>
											</tr>
											<tr>
												<td align="right">Name: </td>
												<td align="left">
													<input type="text" name="orders[<?=$orid;?>][name_on_account]" value="<?=$order[0]['name_on_account'];?>" size="20">
												</td>
											</tr>
											<tr>
												<td align="right">ABA: </td>
												<td align="left">
													<input type="text" name="orders[<?=$orid;?>][aba_routing_code]" value="<?=$order[0]['aba_routing_code'];?>" size="20">
												</td>
											</tr>
										<?php elseif($order[0]['po_number'] != ""):?>
											<tr>
												<td align="right">PO Number: </td>
												<td align="left">
													<input type="text" name="orders[<?=$orid;?>][po_number]" value="<?=$order[0]['po_number'];?>" size="20">
												</td>
											</tr>
										<?php endif;?>
										
											<tr><td colspan="2" style="line-height:5px;">&nbsp;</td></tr>
											<tr><th colspan="2" style="padding-left:2px;"><b>Shipping Status:</b> </th></tr>
											<tr><td colspan="2" style="line-height:5px;">&nbsp;</td></tr>

											<tr>
												<td align="right" nowrap>Order Status: </td>
												<td align="left">
													<?php $fldname = "orders[" . $orid . '][status]" onchange="setShipDate(this.options[this.selectedIndex].value,' . $orid . ');';?>
													<?=$_DB->getDefaultValues('status',$order[0]['status'],true,$fldname);?>
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>Ship Via: </td>
												<td align="left">
													<input type="text" name="orders[<?=$orid;?>][shipping_method]" value="<?=$order[0]['shipping_method'];?>" size="20">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>Date Shipped: </td>
												<td align="left" valign="middle">
													<input type="text" name="orders[<?=$orid;?>][date_shipped]" value="<?=$order[0]['date_shipped'];?>" size="10">
													<img src="../images/calendaricon.gif" height="17" width="17" border=0 onClick="popUpCalendar(this, document.getElementById('orders[<?=$orid;?>][date_shipped]'), 'yyyy-mm-dd', -120, 20)">
												</td>
											</tr>
											<tr>
												<td align="right" style="vertical-align:top;">Tracking #: </td>
												<td align="left">
													<?php if(strstr($order[0]['tracking_number'],',')):?>
														
														<?php 
															$tNumbers = explode(',',$order[0]['tracking_number']);
															if(isset($order[0]['number_of_packages']) && $order[0]['number_of_packages'] > 1){
																$tCount = $order[0]['number_of_packages'];
															}
															else{
																$tCount = count($tNumbers);	
															}
														?>
														<?php for($t=0;$t<$tCount;$t++):?>
															<?php if(!empty($tNumbers[$t])):?>
																<input type="text" name="orders[<?=$orid;?>][tracking_number][]" value="<?=$tNumbers[$t];?>" size="20"><br />
															<?php else:?>
																<input type="text" name="orders[<?=$orid;?>][tracking_number][]" value="" size="20"><br />
															<?php endif;?>
														<?php endfor;?>
														
													<?php elseif(empty($order[0]['tracking_number']) && isset($order[0]['number_of_packages']) && $order[0]['number_of_packages'] > 1):?>
													
														<?php $tCount = $order[0]['number_of_packages'];?>
														<?php for($t=0;$t<$tCount;$t++):?>
															<input type="text" name="orders[<?=$orid;?>][tracking_number][]" value="" size="20"><br />
														<?php endfor;?>

													<?php else:?>
													<input type="text" name="orders[<?=$orid;?>][tracking_number][]" value="<?=$order[0]['tracking_number'];?>" size="20">
													<?php endif;?>
												</td>
											</tr>

											<?php if(isset($order[0]['number_of_packages'])):?>
											<tr>
												<td align="right" nowrap># of Packages: </td>
												<td align="left" valign="middle">
													<input type="text" name="orders[<?=$orid;?>][number_of_packages]" value="<?=$order[0]['number_of_packages'];?>" size="3" style="text-align:right;">
												</td>
											</tr>
											<?php endif;?>
											
											<tr><td colspan="2" style="line-height:2px;">&nbsp;</td></tr>
											<tr><th colspan="2" style="padding-left:5px;"><b>Misc Info:</b> </th></tr>
											<tr><td colspan="2" style="line-height:5px;">&nbsp;</td></tr>
											
											<tr>
												<td align="right" nowrap>Coupon: </td>
												<td align="left">
													<input type="text" name="orders[<?=$orid;?>][coupon]" value="<?=$order[0]['coupon'];?>" size="20">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>Affiliate: </td>
												<td align="left">
													<input type="text" name="orders[<?=$orid;?>][affiliate]" value="<?=$order[0]['affiliate'];?>" size="20">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>Browser: </td>
												<td align="left">
													<input type="text" name="orders[<?=$orid;?>][browser]" value="<?=$order[0]['browser'];?>" size="20">
												</td>
											</tr>
											<tr>
												<td align="right" nowrap>User IP: </td>
												<td align="left">
													<input type="text" name="orders[<?=$orid;?>][user_host]" value="<?=$order[0]['user_host'];?>" size="20">
												</td>
											</tr>
										</table>
									</td>
									
								</tr>
								<tr>
									<td colspan="3" align="left">
										<?php if($order[0]['comments'] != ""):?>
											<p><br /><b>Comments:</b> <?=$order[0]['comments'];?><br />&nbsp;</p>
										<?php else:?>
											&nbsp;
										<?php endif;?>
									</td>
								</tr>
							</table>

							<br clear="all" />





							<table border="0" cellpadding="3" cellspacing="0" width="100%">

								<tr>
									<th class="cartHeader" width="50">Select</th>
									<th class="cartHeader" width="50">SKU</th>
									<th class="cartHeader" width="90%">Description</th>
									<th class="cartHeader" width="50" align="center">Qty</th>
									<th class="cartHeader" width="75" align="center" nowrap>Price</th>
									<th class="cartHeaderEnd" width="75" align="right" nowrap>Total</th>
								</tr>
								
								<?php foreach($orderDetails as $index=>$data):?>
									<?php 
										$id = $data['ordid'];
										$optionTotal = 0;
										foreach($data as $key=>$value){
											// loads the default values from the table for enum fields
											$data[$key] = $_DB->getDefaultValues($key,$value,true);
										}
										$data['price'] = $_Common->format_price($data['price']);
									?>

									<tr>
										<td class="cartRow" width="50" valign="top">
											<input type="checkbox" name="order_details[<?=$id;?>][delete]" value="true">
										</td>
										<td class="cartRow" align="left"><?=$data['sku'];?></td>
										<td class="cartRow" align="left">
											<?=$data['name'];?>
											<!-- Options -->
											<?php if(!empty($data['options'])):?>
												<br />
												<?php foreach($data['options'] as $j=>$option):?>
													<div style="margin-top:2px;margin-left:10px;">
														<?php if($option['name'] != "" && $option['name'] == "Option"):?>
															<?=$option['name'];?> <?=$j+1;?>:
														<?php elseif($option['name'] != "" && $option['name'] != "Option"):?>
															<?=$option['name'];?>:
														<?php endif;?>
														<?=$option['value'];?>
														<?php if($option['price'] != "0"):?>
															<?php if($option['type'] == "option"):?>
																(<?=$_Common->format_price($option['price']);?> each)
															<?php elseif($option['type'] == "setup"):?>
																(Setup Charge: <?=$_Common->format_price($option['price']);?>)
															<?php else:?>
																(<?=$_Common->format_price($option['price']);?>)
															<?php endif;?>
															<?php $optionTotal += $_Common->format_price($option['price']); ?>
														<?php endif;?>
													</div>
												<?php endforeach;?>
												<input type="hidden" name="order_details[<?=$id;?>][option_total]" id="order_details[<?=$id;?>][option_total]" value="<?=$_Common->format_price($optionTotal);?>">
											<?php endif;?>
										</td>
										<td class="cartRow">
											<input type="text" id="order_details[<?=$id;?>][quantity]" name="order_details[<?=$id;?>][quantity]" value="<?=$data['quantity'];?>" size="6" style="text-align:right;" onBlur="showLineTotal(this.form,'order_details[<?=$id;?>][quantity]','order_details[<?=$id;?>][price]',<?=$id;?>,<?=$orid;?>)">
										</td>
										<td class="cartRow" align="right">
											<input type="text" id="order_details[<?=$id;?>][price]" name="order_details[<?=$id;?>][price]" value="<?=$_Common->format_price($data['price']);?>" size="10" style="text-align:right" onBlur="showLineTotal(this.form,'order_details[<?=$id;?>][quantity]','order_details[<?=$id;?>][price]',<?=$id;?>,<?=$orid;?>)">
										</td>
										<td class="cartRowEnd" align="right" id="order_details[<?=$id;?>][total]">
											<?=$_Common->format_price(($data['price'] + $optionTotal) * $data['quantity']);?>
										</td>
									</tr>

								<?php endforeach;?>

									<!-- blank line for adding items -->
									<tr>
										<td class="cartRow" width="50" style="vertical-align:middle">Add:</td>
										<td class="cartRow" align="left">
											<input type="text" name="order_details[add][sku]" size="15">
										</td>
										<td class="cartRow" align="left">
											<input type="text" name="order_details[add][name]" value="" style="width:98%">
										</td>
										<td class="cartRow">
											<input type="text" id="order_details[add][quantity]" name="order_details[add][quantity]" value="" size="6" style="text-align:right;" onBlur="showLineTotal(this.form,'order_details[add][quantity]','order_details[add][price]','add',<?=$orid;?>)">
										</td>
										<td class="cartRow" align="right">
											<input type="text" id="order_details[add][price]" name="order_details[add][price]" value="" size="10" style="text-align:right" onBlur="showLineTotal(this.form,'order_details[add][quantity]','order_details[add][price]','add',<?=$orid;?>)">
										</td>
										<td class="cartRowEnd" align="right" id="order_details[add][total]">&nbsp;</td>
									</tr>


									<tr>
										<td colspan="6" style="line-height:5px;">&nbsp;</td>
									</tr>

								<!-- totals -->
								<?php if(count($totals) > 0):?>
									<?php foreach($totals[0] as $tFld=>$total):?>
										<?php
											if(preg_match("|\((.*)\)|",$tFld)){
												$matches = array();
												$fldStr = preg_match("|\((.*)\)|",$tFld,$matches);
												$fldName = ucwords($matches[1]);
											}
											else{
												$fldName = ucwords($tFld);
											}
											$total = $_Common->format_price($total,false);
										?>
										<tr>
											<td colspan="5" align="right" width="90%">
												<b><?=$fldName;?>:</b>
											</td>
											<td align="right">
												<input style="text-align:right" type="text" name="orders[<?=$orid;?>][<?=strtolower($fldName);?>]" value="<?=$total;?>" size="8">
											</td>
										</tr>
									<?php endforeach;?>
								<?php endif;?>
							</table>
						</td>
					</tr>
				</table>
				
				<p>
				<?php if(!empty($_SESSION['last_query'])):?>
					<input type="button" name="summary" value="Summary Screen" onClick="document.location = '<?=$_SESSION['last_query'];?>';">&nbsp;	
				<?php else:?>
					<input type="button" name="summary" value="Summary Screen" onClick="javascript:history.go(-1)">
				<?php endif;?>
				
				<input type="button" name="invoice" value="Invoice" onClick="document.location = 'invoice.php?orid=<?=$order[0]['orid'];?>&amp;order_number=<?=$order[0]['order_number'];?>';">
				<!-- a href="reports.php?run=true&report=orders&showInvoice=true&order_number=<?=$orderNum;?>">Show Invoice</a -->

				<input type="submit" name="updateOrder" value="Update Order">&nbsp;
				<input type="submit" name="deleteOrder" value="Delete Order" onClick="return confirmDelete('order');">
				<?php if(count($orderDetails) > 1):?>
					&nbsp;<input type=submit name="deleteDetails" value="Delete Selected Details" onClick="return checkSelected(document.forms['order'],'delete','order detail');">
				<?php endif;?>
				</p>
				
			<?php endif;?>


		<?php endif;?>
		
	</form>
	</div>
<p>&nbsp;</p>
</body>
</html>
