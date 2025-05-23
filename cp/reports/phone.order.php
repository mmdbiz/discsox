<?php
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "reports";

$debug = false;

// initialize the program, read the config(s) and set include paths
include_once("../../include/initialize.inc");
$init = new Initialize(true);

$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

$orderSaved = null;
$emailSent = null;

if(isset($_REQUEST['doorder'])){
	include_once("include/phoneorders.inc");
	$phoneOrder = new PhoneOrders();
	$phoneOrder->saveOrder();
	$orderSaved = $phoneOrder->orderSaved;
	$emailSent = $phoneOrder->emailSent;
}

// For Pull downs
global $provinces;
global $states;
global $countries;
include_once("../../include/countries.inc");
include_once("../../include/provinces.inc");
include_once("../../include/states.inc");

$color = array();
$color[~0] = "#e2eDe2";
$color[0] = "#FFFFFF";
$ck = 0;

global $_DB;
$nextOrderNumber = "PHONE1";

$result = $_DB->getRecord("SELECT MAX(order_number) AS lastnumber FROM orders");
if(isset($result['lastnumber']) && $result['lastnumber'] != ""){
    $nextOrderNumber = $result['lastnumber'] + 1;
}

$fldProperties = $_DB->getFieldProperties("orders");

error_reporting(E_ALL ^ E_NOTICE);
?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<title>Order Report</title>
<script language="javascript1.2">
{
self.name="content"
}

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
function calpopup(Ink){
	window.open(Ink,"calendar","height=250,width=250,scrollbars=no")
}
function setShipDate(val){

	var fldName = 'orders[date_shipped]';
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
function showPaymentForm(whichEl,show){
	if(document.all){
		whichEl = document.all[whichEl];
	}
	else{
		whichEl = document.getElementById(whichEl);
	}
	if(whichEl){
		if(show){
			whichEl.style.display = "";
		}
		else{
			whichEl.style.display = "none";
		}
	}
}
</script>
<script LANGUAGE="JavaScript" src="../javascripts/reports.js"></script>
<link type="text/css" rel="stylesheet" href="../stylesheets/calendar.css">
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

.title {
     font-family: Verdana, Arial, Helvetica, sans-serif;
     color: #0005CE;
     font-size: 14px;
     font-weight:700;
     line-height: 14px;
}

.totals{
  border:1px solid #C8C8C8;
  text-align:right;
}
.totalsOver{
  border:1px solid #0000FF;
  text-align:right;
}

#sku {width:20em;height:1.4em;}
.skuContainer {position:absolute;z-index:9050;}
.skuContainer .yui-ac-content {position:absolute;left:15em;top:0;width:60em;border:1px solid #404040;background:#fff;overflow-x:hidden;overflow-y:hidden;text-align:left;z-index:9050;}
.skuContainer .yui-ac-shadow {position:absolute;left:15em;top:0;margin:.3em;background:#a0a0a0;z-index:9049;}
.skuContainer ul {padding:5px 0;width:100%;}
.skuContainer li {padding:0 5px;cursor:default;white-space:nowrap;}
.skuContainer li.yui-ac-highlight {background:#ff0;}

#cal1Container { display:none; position:absolute; left:270px; top:20px; }
#cal2Container { display:none; position:absolute; left:843px; top:300px; }
</style>

</head>
<body class="mainBody">

<div id="cal1Container"></div>
<div id="cal2Container"></div>

	<div id="main" align=center valign=top style="display:none;">
	
		<?php if($orderSaved):?>
			<p style="font-size:12px;color:red"><br />The order has been saved<?php if($emailSent):?> and a receipt was sent to the customer.<?php else:?>.<?php endif;?><br /></p>
		<?php endif;?>
	
	
		<form name="order" method="post" action="phone.order.php">

			<input type="hidden" id="customers[cid]" name="customers[cid]" value="">

			<table border="0" cellpadding="2" cellspacing="0" width="90%">
				<tr>
					<td>
						<table border="0" cellpadding="2" cellspacing="0" width="100%" align="left" ID="Table4">
							<tr>
								<td align="left"><span class="title">Order Date:</span>
									<input type="text" id="orders[order_date]" name="orders[order_date]" value="<?=date("m/d/Y");?>" size="10">
									<span style="position:relative; left:0px; top:3px;">
										<img src="../images/calendaricon.gif" height="17" width="17" border=0 onClick="showCalendar('cal1Container', 'orders[order_date]')">
									</span>
								</td>
								<td>&nbsp;</td>
								<td align="right"><span class="title">Order Number:</span>
									<input type="text" name="orders[order_number]" value="<?=$nextOrderNumber;?>" size="10" style="text-align:right">
								</td>
							</tr>
							<tr>
								<th align="left" style="padding-left:2px;">Bill To:</th>
								<th align="left" style="padding-left:2px;">Ship To:</th>
								<th align="left" style="padding-left:2px;">Payment Info:</th>
							</tr>
							<tr><td colspan="3" style="line-height:5px;">&nbsp;</td></tr>
							<tr>
								<td align="left" class="topAlign" height="300">
									<table border="0" cellspacing="0" cellpadding="2" width="100%" height="300">
										<tr>
											<td align="right" nowrap>Company: </td>
											<td align="left">
												<input type="text" id="customers[billaddress_companyname]" name="customers[billaddress_companyname]" value="" size="25">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>First Name: </td>
											<td align="left">
												<input type="text" id="customers[billaddress_firstname]" name="customers[billaddress_firstname]" value="" size="25">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap><b>Last Name:</b> </td>
											<td align="left">
												<input type="text" id="customers[billaddress_lastname]" name="customers[billaddress_lastname]" value="" size="25" onClick="customer.doLookup();" title="Enter 2 or more characters of the last name to see a list of matching customers">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Address: </td>
											<td align="left">
												<input type="text" id="customers[billaddress_addr1]" name="customers[billaddress_addr1]" value="" size="25">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Address2: </td>
											<td align="left">
												<input type="text" id="customers[billaddress_addr2]" name="customers[billaddress_addr2]" value="" size="25">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>City: </td>
											<td align="left">
												<input type="text" id="customers[billaddress_city]" name="customers[billaddress_city]" value="" size="25">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Province/State: </td>
											<td align="left">
												<?php if($billInfo['billaddress_country'] == "US"):?>
												<input type="text" id="customers[billaddress_state]" name="customers[billaddress_state]" value="" size="2">
												<?php else:?>
												<input type="text" id="customers[billaddress_state]" name="customers[billaddress_state]" value="" size="25">
												<?php endif;?>
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Postal Code: </td>
											<td align="left">
												<input type="text" id="customers[billaddress_postalcode]" name="customers[billaddress_postalcode]" value="" size="9">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Country: </td>
											<td align="left">
												<input type="text" id="customers[billaddress_country]" name="customers[billaddress_country]" value="" size="2">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Phone: </td>
											<td align="left">
												<input type="text" id="customers[billaddress_areacode]" name="customers[billaddress_areacode]" value="" size="3">
												<input type="text" id="customers[billaddress_phone]" name="customers[billaddress_phone]" value="" size="15">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Email: </td>
											<td align="left">
												<input type="text" id="customers[billaddress_email]" name="customers[billaddress_email]" value="" size="25">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Email List: </td>
											<td align="left">
												<select id="customers[email_list]" name="customers[email_list]">
													<option value="true">true</option>
													<option value="false">false</option>
												</select>
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Paid Status: </td>
											<td align="left">
												<select id="orders[paid]" name="orders[paid]">
													<option value="false">false</option>
													<option value="true">true</option>
												</select>
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Taxable: </td>
											<td align="left">
												<select id="customers[is_taxable]" name="customers[is_taxable]" onChange="salestax.getRecords();">
													<option value="false">false</option>
													<option value="true">true</option>
												</select>
											</td>
										</tr>
									</table>
									<div class="skuContainer" id="customerLookupContainer"></div>
								</td>

								<td align="left" class="topAlign" height="300">
								
									<table border="0" cellspacing="0" cellpadding="2" width="100%" height="300">
										<tr id="shipSelect" style="display:none">
											<td align="right" nowrap>Address Book: </td>
											<td align="left">
												<select id="customer_shipping[csid]" name="customer_shipping[csid]" onChange="shipping.showResults(this.selectedIndex);">
													<option value="">Select Ship Address</option>
												</select>
											</td>
										</tr>
										<tr id="samebilling">
											<td align="right" nowrap>Same as billing: </td>
											<td align="left">
												<input type="checkbox" id="sameasbilling" name="sameasbilling" value="true" onClick="customer.sameAsBilling(this.checked);">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Company: </td>
											<td align="left">
												<input type="text" id="customer_shipping[shipaddress_companyname]" name="customer_shipping[shipaddress_companyname]" value="" size="25">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>First Name: </td>
											<td align="left">
												<input type="text" id="customer_shipping[shipaddress_firstname]" name="customer_shipping[shipaddress_firstname]" value="" size="25">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Last Name: </td>
											<td align="left">
												<input type="text" id="customer_shipping[shipaddress_lastname]" name="customer_shipping[shipaddress_lastname]" value="" size="25">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Address: </td>
											<td align="left">
												<input type="text" id="customer_shipping[shipaddress_addr1]" name="customer_shipping[shipaddress_addr1]" value="" size="25">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Address2: </td>
											<td align="left">
												<input type="text" id="customer_shipping[shipaddress_addr2]" name="customer_shipping[shipaddress_addr2]" value="" size="25">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>City: </td>
											<td align="left">
												<input type="text" id="customer_shipping[shipaddress_city]" name="customer_shipping[shipaddress_city]" value="" size="25">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Province/State: </td>
											<td align="left">
												<?php if($shipInfo['shipaddress_country'] == "US"):?>
												<input type="text" id="customer_shipping[shipaddress_state]" name="customer_shipping[shipaddress_state]" value="" size="2">
												<?php else:?>
												<input type="text" id="customer_shipping[shipaddress_state]" name="customer_shipping[shipaddress_state]" value="" size="25">
												<?php endif;?>
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Postal Code: </td>
											<td align="left">
												<input type="text" id="customer_shipping[shipaddress_postalcode]" name="customer_shipping[shipaddress_postalcode]" value="" size="9">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Country: </td>
											<td align="left">
												<input type="text" id="customer_shipping[shipaddress_country]" name="customer_shipping[shipaddress_country]" value="" size="2">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Phone: </td>
											<td align="left">
												<input type="text" id="customer_shipping[shipaddress_areacode]" name="customer_shipping[shipaddress_areacode]" value="" size="3">
												<input type="text" id="customer_shipping[shipaddress_phone]" name="customer_shipping[shipaddress_phone]" value="" size="8">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Email: </td>
											<td align="left">
												<input type="text" id="customer_shipping[shipaddress_email]" name="customer_shipping[shipaddress_email]" value="" size="25">
											</td>
										</tr>
									</table>

								</td>
								<td align="left" class="topAlign">

									<table border="0" cellspacing="0" cellpadding="1" width="100%">
										<tr>
											<td align="right" width="118">Payment Type: </td>
											<td align="left">
												<select name="orders[payment_method]" onChange="showPaymentForm('ccpayment',false);showPaymentForm('checkpayment',false);showPaymentForm('popayment',false);showPaymentForm(this.options[this.selectedIndex].value,true);">
													<option value="ccpayment">Credit Card</option>
													<option value="checkpayment">Check</option>
													<option value="popayment">Purchase Order</option>
												</select>
											</td>
										</tr>
									</table>
								
								    <div id="ccpayment" style="display:true">
									<table border="0" cellspacing="0" cellpadding="1" width="100%">
										<tr>
											<td align="right" width="118">Card Type: </td>
											<td align="left">
												<input type="text" name="orders[credit_card_type]" value="" size="20">
											</td>
										</tr>
										<tr>
											<td align="right">Name on Card: </td>
											<td align="left">
												<input type="text" name="orders[name_on_card]" value="" size="20">
											</td>
										</tr>
										<tr>
											<td align="right">Number: </td>
											<td align="left">
												<input type="text" name="orders[card_number]" value="" size="20">
											</td>
										</tr>
										<tr>
											<td align="right">Expire Date: </td>
											<td align="left">
												<input type="text" name="orders[expire_month]" value="" size="2">
												<input type="text" name="orders[expire_year]" value="" size="4">
											</td>
										</tr>
										<tr>
											<td align="right">CVV2: </td>
											<td align="left">
												<input type="text" name="orders[verification_number]" value="" size="4">
											</td>
										</tr>
									</table>	
									</div>
										
									<div id="checkpayment" style="display:none">
									<table border="0" cellspacing="0" cellpadding="1" width="100%">
										<tr>
											<td align="right" width="118">Bank: </td>
											<td align="left">
												<input type="text" name="orders[bank_name]" value="" size="20">
											</td>
										</tr>
										<tr>
											<td align="right">Type: </td>
											<td align="left">
												<input type="text" name="orders[account_type]" value="" size="20">
											</td>
										</tr>
										<tr>
											<td align="right">Account: </td>
											<td align="left">
												<input type="text" name="orders[account_number]" value="" size="20">
											</td>
										</tr>
										<tr>
											<td align="right">Name: </td>
											<td align="left">
												<input type="text" name="orders[name_on_account]" value="" size="20">
											</td>
										</tr>
										<tr>
											<td align="right">ABA: </td>
											<td align="left">
												<input type="text" name="orders[aba_routing_code]" value="" size="20">
											</td>
										</tr>
									</table>
									</div>
									
									<div id="popayment" style="display:none">
									<table border="0" cellspacing="0" cellpadding="1" width="100%">
										<tr>
											<td align="right" width="118">PO Number: </td>
											<td align="left">
												<input type="text" name="orders[po_number]" value="" size="20">
											</td>
										</tr>
									</table>
									</div>
									
									
									<table border="0" cellspacing="0" cellpadding="1" width="100%">
										<tr><td colspan="2" style="line-height:5px;">&nbsp;</td></tr>
										<tr><th colspan="2" style="padding-left:2px;"><b>Status Info:</b> </th></tr>
										<tr><td colspan="2" style="line-height:5px;">&nbsp;</td></tr>

										<tr>
											<td align="right" nowrap width="118">Order Status: </td>
											<td align="left">
												<?php $fldname = 'orders[status]" onchange="setShipDate(this.options[this.selectedIndex].value);';?>
												<?=$_DB->getDefaultValues('status',null,true,$fldname);?>
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Ship Via: </td>
											<td align="left">
												<input type="text" name="orders[shipping_method]" value="" size="20">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Date Shipped: </td>
											<td align="left" valign="middle">
												<input type="text" id="orders[date_shipped]" name="orders[date_shipped]" value="<?=date("m/d/Y");?>" size="10">
												<span style="position:relative; left:0px; top:3px;">
													<img src="../images/calendaricon.gif" height="17" width="17" border=0 onClick="showCalendar('cal2Container', 'orders[date_shipped]')">
												</span>
											</td>
										</tr>
										<tr>
											<td align="right">Tracking #: </td>
											<td align="left">
												<input type="text" name="orders[tracking_number]" value="" size="20">
											</td>
										</tr>
										
										<tr><td colspan="2" style="line-height:2px;">&nbsp;</td></tr>
										<tr><th colspan="2" style="padding-left:5px;"><b>Misc Info:</b> </th></tr>
										<tr><td colspan="2" style="line-height:5px;">&nbsp;</td></tr>
										
										<tr>
											<td align="right" nowrap>Coupon: </td>
											<td align="left">
												<input type="text" name="orders[coupon]" value="" size="20">
											</td>
										</tr>
										<tr>
											<td align="right" nowrap>Affiliate: </td>
											<td align="left">
												<input type="text" name="orders[affiliate]" value="" size="20">
											</td>
										</tr>

									</table>
								</td>
								
							</tr>
							<tr>
								<td id="cart" colspan="3" height="100" align="left">
									<table border="0" cellpadding="3" cellspacing="0" width="100%">
										<thead>
											<tr><td colspan="5"><span class="title">Order Items:</span></td></tr>
											<tr>
												<th class="cartHeader" width="75">Remove</th>
												<th class="cartHeader" width="75">Item Code</th>
												<th class="cartHeader" width="90%" align="left">Description</th>
												<th class="cartHeader" width="75">Price</th>
												<th class="cartHeader" width="50">Qty</th>
												<th class="cartHeaderEnd" width="75">Total</th>
											</tr>
										</thead>
										<tfoot>
											<tr>
												<td colspan="2" style="padding-top:5px" >
													<img src="images/arrow_right.gif"> <a href="javascript:cart.addCartRow();">Add a line</a>
												</td>
												<td colspan="3" align="right" style="padding-top:5px"><b>Subtotal:</b></td>
												<td id="subtotal" align="right" style="padding-top:5px">
													<input class="totals" type="text" id="orders[subtotal]" name="orders[subtotal]" value="" size="10" onclick="this.className='totalsOver'" onblur="this.className='totals';">
												</td>
											</tr>
											<tr>
												<td colspan="5" align="right"><b>Discount:</b></td>
												<td id="salestax" align="right">
													<input class="totals" type="text" id="orders[discount]" name="orders[discount]" value="" size="10" onclick="this.className='totalsOver'" onblur="this.className='totals';cart.displayTotals();">
												</td>
											</tr>
											<tr>
												<td colspan="5" align="right"><b>Shipping:</b></td>
												<td id="salestax" align="right">
													<input class="totals" type="text" id="orders[shipping]" name="orders[shipping]" value="" size="10" onclick="this.className='totalsOver'" onblur="this.className='totals';cart.displayTotals();">
												</td>
											</tr>
											<tr>
												<td colspan="5" align="right"><b>Sales Tax:</b></td>
												<td id="salestax" align="right">
													<input class="totals" type="text" id="orders[salestax]" name="orders[salestax]" value="" size="10" onclick="this.className='totalsOver'" onblur="this.className='totals';cart.displayTotals();">
												</td>
											</tr>
											<tr>
												<td colspan="5" align="right"><b>Order Total:</b></td>
												<td id="salestax" align="right">
													<input class="totals" type="text" id="orders[grandtotal]" name="orders[grandtotal]" value="" size="10" onclick="this.className='totalsOver'" onblur="this.className='totals'">
												</td>
											</tr>
										</tfoot>
										<tbody id="cartrow">
											<tr id="row[0]">
												<td class="cartRow" align="center">
													<input type="checkbox" name="remove[0]" id="remove[0]" onclick="cart.removeRow(this.id);">
												</td>
												<td class="cartRow" align="center">
													<input type="text" id="sku[0]" name="sku[0]" size="15" title="Enter 2 or more characters of the item code to see a list of matching products" onClick="cart.initCartFields(this.name);">
												</td>
												<td class="cartRow" align="left">
													<input type="text" id="description[0]" name="description[0]" value="" style="width:98%">
												</td>
												<td class="cartRow" align="center">
													<input type="hidden" id="qtyprice[0]" name="qtyprice[0]" value="" size="10" style="text-align:right">
													<input type="text" id="price[0]" name="price[0]" value="" size="10" style="text-align:right">
												</td>
												<td class="cartRow" align="center">
													<input type="text" id="quantity[0]" name="quantity[0]" value="" size="5" style="text-align:right" onBlur="cart.setTotals();">
												</td>
												<td class="cartRowEnd" align="center">
													<input type="text" id="total[0]" name="total[0]" value="" size="10" style="text-align:right">
												</td>
											</tr>
										</tbody>
									</table>
									<div class="skuContainer" id="cartLookupContainer"></div>
								</td>
							</tr>
							<tr><td colspan="3">&nbsp;</td></tr>
							<tr>
								<td colspan="3" align="left">
									<span class="title">Comments:</span>
									<textarea name="orders[comments]" rows="3" style="width:99%"></textarea>
								</td>
							</tr>
							<tr><td colspan="3">&nbsp;</td></tr>
							<tr>
								<td colspan="3" align="center" valign="middle" style="vertical-align:middle">
									<input type="checkbox" name="sendreceipt" value="true" style="padding-top:5px"> Send Receipt?  &nbsp;
									<input type="submit" name="doorder" value="Save Order">
								</td>
							</tr>
						</table>

					</td>
				</tr>
			</table>

		</form>
	</div>
<p>&nbsp;</p>

<!-- Logger window begins -->
<div id="logger" style="display:none"></div>
<!-- Logger window ends -->

<!-- javascript files for auto-complete functions -->
<script type="text/javascript" src="../javascripts/yui/yahoo-min.js"></script>
<script type="text/javascript" src="../javascripts/yui/dom-min.js"></script>
<script type="text/javascript" src="../javascripts/yui/event-min.js"></script>
<script type="text/javascript" src="../javascripts/yui/connection-min.js"></script>
<script type="text/javascript" src="../javascripts/yui/animation-min.js"></script>
<script type="text/javascript" src="../javascripts/yui/json.js"></script>
<script type="text/javascript" src="../javascripts/yui/autocomplete-min.js"></script>
<script type="text/javascript" src="../javascripts/yui/logger.js"></script>
<script type="text/javascript" src="../javascripts/yui/calendar-min.js"></script>  

<!-- dom helper for cart row template -->
<script type="text/javascript" src="../javascripts/yui/ext/yui-ext.js"></script>
<script type="text/javascript" src="../javascripts/yui/ext/DomHelper.js"></script>
<script type="text/javascript"> 
	var dom = YAHOO.util.Dom;
	var mylogger = new YAHOO.widget.LogReader("logger");
	YAHOO.util.Event.addListener(this,"load");
	var symbol = '<?=$_CF['basics']['currency_symbol'];?>';

	YAHOO.namespace("example.calendar");
	var dateFld = null;
	
	function handleDateSelect(type,args,obj) {
		var dates = args[0];
		var date = dates[0];
		var year = date[0], month = date[1], day = date[2];

		//alert(month + "/" + day + "/" + year);
		//var txtDate1 = document.getElementById("date1");
		//txtDate1.value = month + "/" + day + "/" + year;
		
		if(dateFld){
			dateFld.value = month + "/" + day + "/" + year;
		}
		YAHOO.example.calendar.cal1.hide();
		
	}

	function showCalendar(container,frmFld){
	
		dateFld = dom.get(frmFld);
	
		YAHOO.example.calendar.cal1 = new YAHOO.widget.Calendar("cal1",container,{ title:"Choose a date:",
																				   close:true,
																				   mindate:"1/1/2006",
																				   maxdate:"12/31/2008" });
		
		// Set the Calendar's date and select the page to display
		frmValue = dom.get(frmFld).value
		YAHOO.example.calendar.cal1.select(frmValue);
		var firstDate = YAHOO.example.calendar.cal1.getSelectedDates()[0];
		YAHOO.example.calendar.cal1.cfg.setProperty("pagedate", (firstDate.getMonth()+1) + "/" + firstDate.getFullYear());
		YAHOO.example.calendar.cal1.render();
		YAHOO.example.calendar.cal1.show();
		
		YAHOO.example.calendar.cal1.selectEvent.subscribe(handleDateSelect, YAHOO.example.calendar.cal1, true);
	}
</script>
<!-- javascript file for cart functions -->
<script type="text/javascript" language="javascript1.2" src="../javascripts/phone_orders/cart.js"></script>
<!-- javascript file for customer functions -->
<script type="text/javascript" language="javascript1.2" src="../javascripts/phone_orders/customers.js"></script>

<script type="text/javascript">
	var maindiv = dom.get('main');
	maindiv.style.display = "";
</script>

</body>
</html>