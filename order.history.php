<?php
// initialize the program and read the config
include_once("include/initialize.inc");
$init = new Initialize();

// Login is required
$_CF['login']['require_login'] = true;
$_SESSION['logging_in_from'] = "order.history.php";

// get the login class and see if required
$login = $_Registry->LoadClass('login');
$login->checkLogin();

$queryTotals = array();
$queryTotal = 0;
$records = array();
$totals = array();

$hits = 0;
if(isset($_REQUEST['hits']) && is_numeric($_REQUEST['hits']) && intval($_REQUEST['hits']) > 0){
	$hits = intval($_REQUEST['hits']);
}

$max = 50;
$count = 0;
$links = null;
$start = 0;
$end = 0;
$limit = null;
$customerName = null;

if(!empty($_SESSION['cid'])){
	
	$cid = trim($_SESSION['cid']);
	if(strlen($cid) < 32){
		$cid = md5($cid);	
	}

	$countsql = "SELECT COUNT(orid) as count, SUM(grandtotal) as queryTotal
				 FROM orders,customers
				 WHERE orders.cid = customers.cid
				 AND MD5(customers.cid) = '$cid'";
	$queryTotals = $_DB->getRecord($countsql);

	if(!empty($queryTotals['count'])){
		$count = $queryTotals['count'];
		$queryTotal	= $queryTotals['querytotal'];
		//$_Common->debugPrint($queryTotals);
	}
	
	if($count > 0){
		
		if($max > $count){
			$max = $count + 1;
		}
		list($start,$end,$limit) = $_DB->getLimits($count,$max,"order.history.php");
		
		if($queryTotals['count'] > $max){
			$_DB->createPreviousNextLinks($hits,$count,$max,"order.history.php");
			$links = $_DB->previousNextLinks; 
		}
		
		$sql = "SELECT orders.*,customers.*,customer_shipping.*,
				CONCAT(customers.billaddress_firstname,' ',customers.billaddress_lastname) AS customer_name,
				DATE_FORMAT(orders.transaction_date,'%c-%d-%Y') AS order_date,
				DATE_FORMAT(orders.date_shipped,'%c-%d-%Y') AS date_shipped
				FROM orders,customers,customer_shipping
				WHERE orders.cid = customers.cid
				AND orders.csid = customer_shipping.csid
				AND MD5(customers.cid) = '$cid'
				ORDER BY orders.order_number,orders.transaction_date $limit";

		$records = $_DB->getRecords($sql);
		$customerName = $records[0]['customer_name'];

		//$_Common->debugPrint($records);
		
		//oridList is list of order ids
		$oridList = array();
		$fieldsToRemove = array('card_number','cvv2','expire_month','expire_year',
								'bank_name','name_on_account','account_number','aba_routing_code',
								'username','password');

		foreach($records as $i=>$orderData){
			$oridList[] = $orderData['orid'];
			foreach($fieldsToRemove as $j=>$fld){
				unset($records[$i][$fld]);
			}
		}
		$orderIds = "'" . join("','",$oridList) . "'";

		//$_Common->debugPrint($records);

		//Get shipping, sales tax, discounts, etc for the displayed records
		$sql = "SELECT SUM(subtotal) as 'page subtotal',
				SUM(discount) as 'discounts',
				SUM(salestax) as 'sales tax',
				SUM(shipping) as 'shipping fees',
				SUM(grandtotal) as 'page total'
				FROM orders WHERE orid IN($orderIds)";
		$totals = $_DB->getRecords($sql);
		$totals[0]['Report Total'] = $queryTotal;
		//$_Common->debugPrint($totals);
	}

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
			
			$orderDetails[$ordid]['price'] = $_Common->format_price($orderDetails[$ordid]['price'],true);
			$orderDetails[$ordid]['options'][] = $option;
		}
	}
}
?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<title>Order History</title>
<link rel="stylesheet" type="text/css" href="styles/cart.styles.css" />
<script type="text/javascript">
function showIt(whichEl){
    if(document.all){
        whichEl = document.all[whichEl];
    }
    else{
        whichEl = document.getElementById(whichEl);
    }
    whichEl.style.display = (whichEl.style.display == "none" ) ? "" : "none";
}
</script>
</head>
<body class="mainBody">

	<div align="center" valign="top">

			<?php if(count($records) > 0):?>

				<?php
					$color = array();
					$color[0] = "#E2EDE2";
					$color[~0] = "#FFFFFF";
					$ck = 0;
					$primaryKey = "orid";
					$fldCount = count(array_keys($records[0])) - 1;
					$cid = "";
				?>

				<H4>Order History for <?=$customerName;?></H4>
				<p>Click on the order number to view details.</p>

				<table border="0" cellpadding="3" cellspacing="0" width="60%" ID="Table1">
				<tr>
					<td colspan="3" align="left">
						<b><?="$start - $end of $count Orders";?></b>
					</td>
					<td colspan="3" align="right">
						<?=$links;?>&nbsp;
					</td>
				</tr>
				<tr>
					<th width="20%" nowrap>Order #</th>
					<th width="20%" nowrap>Order Date</th>
					<th width="20%" nowrap>Paid</th>
					<th width="20%" nowrap>Ship Status</th>
					<th width="20%" nowrap>Date Shipped</th>
					<th width="20%" nowrap>Subtotal</th>
				</tr>
				
				<?php foreach($records as $index=>$data):?>
					
					<?php
						if($data['date_shipped'] == '0-00-0000'){
							$data['date_shipped'] = "&nbsp;";
						}
						$haveShipping = true;
						if(empty($data['shipaddress_firstname'])){
							$haveShipping = false;
						}
					?>
					
					<tr bgcolor="<?=$color[$ck = ~$ck];?>">
						<td align="center">
							<div style="cursor:hand;" onclick="showIt('detail<?=$index;?>');">
								<font color="blue"><u><?=$data['order_number'];?></u></font>
							</div>
						</td>
						<td width="20%" align=center><?=$data['order_date'];?></td>
						<td width="20%" align=center><?=$data['paid'];?></td>
						<td width="20%" align=center><?=$data['status'];?></td>
						<td width="20%" align=center><?=$data['date_shipped'];?></td>
						<td width="20%" align="right" nowrap><?=$data['subtotal'];?></td>
					</tr>

					<tr id="detail<?=$index;?>" style="display:none;">
					
						<td colspan="6" style="padding-top:20px;padding-bottom:20px;border:dashed 1px gray;">

								<table border="0" cellpadding="0" cellspacing="0" width="98%" bgcolor="#FFFFFF">
									<tr valign="top">
										<td width=50%>
											<table width="100%" border="0" cellspacing="0" cellpadding="3">
												<tr>
													<th align="left" class="invoiceHeader">BILL TO:</th>
												</tr>
												<tr>
													<td height="90" valign=top align="left">
														<?php if(!empty($data['billaddress_companyname'])):?>
															<?=$data['billaddress_companyname'];?><br />
														<?php endif;?>
														<?=$data['billaddress_firstname'];?> <?=$data['billaddress_lastname'];?><br />
														<?=$data['billaddress_addr1'];?><br />
														<?php if(!empty($data['billaddress_addr2'])):?>
															<?=$data['billaddress_addr2'];?><br />
														<?php endif;?>
														<?=$data['billaddress_city'];?>,
															<?=$data['billaddress_state'];?>,
															<?=$data['billaddress_postalcode'];?>
															<?=$data['billaddress_country'];?><br />
														(<?=$data['billaddress_areacode'];?>) <?=$data['billaddress_phone'];?><br />
    		  											<?=$data['billaddress_email'];?>
													</td>
												</tr>
											</table>
										</td>
										<td width=50%>
											<table width="100%" border="0" cellspacing="0" cellpadding="3">
												<tr>
													<th align="left" class="invoiceHeader">SHIP TO:</th>
												</tr>
												<?php if($haveShipping):?>
												<tr>
													<td height="90" valign=top align="left">
														<?php if(!empty($data['shipaddress_companyname'])):?>
															<?=$data['shipaddress_companyname'];?><br />
														<?php endif;?>
														<?=$data['shipaddress_firstname'];?> <?=$data['shipaddress_lastname'];?><br />
    		  											<?=$data['shipaddress_addr1'];?><br />
														<?php if(!empty($data['shipaddress_addr2'])):?>
															<?=$data['shipaddress_addr2'];?><br />
														<?php endif;?>
									    		  		
									    		  		
    		  											<?=$data['shipaddress_city'];?>,
    		  												<?=$data['shipaddress_state'];?>,
    		  												<?=$data['shipaddress_postalcode'];?>
    		  												<?=$data['shipaddress_country'];?><br /><br />
									    		  			
														<b>Ship Via:</b> &nbsp; <?=$data['shipping_method'];?><br />
														<b>Ship Date:</b> &nbsp; <?=$data['date_shipped'];?>
														<?php if($data['tracking_number'] != ""):?>
															<br /><b>Tracking Number:</b> &nbsp; <?=$data['tracking_number'];?>
														<?php endif;?>
													</td>
												</tr>
												<?php endif;?>
											</table>

										</td>
									</tr>
									<tr>
										<td colspan="2" valign="top">

											<table border="0" cellpadding="3" cellspacing="0" width="100%">
												<tr>
													<th class="cartHeader" width="50">SKU</th>
													<th class="cartHeader" width="90%" align="left">Description</th>
													<th class="cartHeader" width="50" align="center">Qty</th>
													<th class="cartHeader" width="75" align="center" nowrap>Price</th>
													<th class="cartHeaderEnd" width="75" align="right" nowrap>Total</th>
												</tr>
												<?php
													$orid = $data['orid'];
													$orderDetailSql = "SELECT * FROM order_details WHERE order_details.orid = '$orid'";
													$orderDetails = $_DB->getRecords($orderDetailSql,"ordid");
													// Add order options and adjust price
													checkOrderOptions($orderDetails);
												?>
												<?php foreach($orderDetails as $index=>$detail):?>
													<tr>
														<td class="cartRow" align="left"><?=$detail['sku'];?></td>
														<td class="cartRow" align="left">
															<?=$detail['name'];?>
															<!-- Options -->
															<?php if(!empty($detail['options'])):?>
																<br />
																<?php foreach($detail['options'] as $j=>$option):?>
																	<div style="margin-top:2px;margin-left:10px;">
																		<?php if($option['name'] != "" && $option['name'] == "Option"):?>
																			<?=$option['name'];?> <?=$j+1;?>:
																		<?php elseif($option['name'] != "" && $option['name'] != "Option"):?>
																			<?=$option['name'];?>:
																		<?php endif;?>
																		<?=$option['value'];?>
																		<?php if($option['price'] != "0"):?>
																			<?php if($option['type'] == "option"):?>
																				($<?=$option['price'];?>)
																			<?php elseif($option['type'] == "setup"):?>
																				(Setup Charge: <?=$option['price'];?>)
																			<?php else:?>
																				(<?=$option['price'];?>)
																			<?php endif;?>
																		<?php endif;?>
																	</div>
												      <?php endforeach;?>
															<?php endif;?>
														</td>
														<td class="cartRow"><?=$detail['quantity'];?></td>
														<td class="cartRow" align="right"><?=$detail['price'];?></td>
														<td class="cartRowEnd" align="right">
															<?=$_Common->format_price($detail['price'] * $detail['quantity']);?>
														</td>
													</tr>

												<?php endforeach;?>

												<tr><td colspan="5" style="line-height:5px;">&nbsp;</td></tr>
												<tr>
													<td colspan="4" align="right" width="90%"><b>Subtotal:</b></td>
													<td align="right"><b><?=$data['subtotal'];?></b></td>
												</tr>
												<?php if($data['discount'] > 0):?>
												<tr>
													<td colspan="4" align="right" width="90%"><b>Discount:</b></td>
													<td align="right"><b><?=$data['discount'];?></b></td>
												</tr>
												<?php endif;?>
												<?php if($data['shipaddress_country'] == "CA"):?>
													<tr>
														<td colspan="4" align="right" width="90%"><b>GST:</b></td>
														<td align="right"><b><?=$data['gst'];?></b></td>
													</tr>
													<tr>
														<td colspan="4" align="right" width="90%"><b>HST:</b></td>
														<td align="right"><b><?=$data['hst'];?></b></td>
													</tr>
													<tr>
														<td colspan="4" align="right" width="90%"><b>PST:</b></td>
														<td align="right"><b><?=$data['pst'];?></b></td>
													</tr>
												<?php else:?>
													<tr>
														<td colspan="4" align="right" width="90%"><b>Sales Tax:</b></td>
														<td align="right"><b><?=$data['salestax'];?></b></td>
													</tr>
												<?php endif;?>
												<?php if($haveShipping):?>
													<tr>
														<td colspan="4" align="right" width="90%"><b>Shipping:</b></td>
														<td align="right"><b><?=$data['shipping'];?></b></td>
													</tr>
												<?php endif;?>
													<tr>
														<td colspan="4" align="right" width="90%"><b>Grand Total:</b></td>
														<td align="right"><b><?=$data['grandtotal'];?></b></td>
													</tr>
											</table>
										</td>
									</tr>
								</table>
						</td>
					</tr>

				<?php endforeach;?>
				
					<!-- page totals -->
					<?php if(count($totals) > 0):?>
						<tr>
							<td colspan="5" align="right">&nbsp;</td>
							<td align=right>======</td>
						</tr>
						<?php foreach($totals[0] as $key=>$sum):?>
							<?php $label = ucwords(str_replace("_"," ",$key));?>
							
							<?php if($key == 'Report Total'):?>
								<tr>
									<td colspan="5" align="right">&nbsp;</td>
									<td align=right>======</td>
								</tr>
								<tr>
									<td colspan="5" align="right"><?=$label;?>:</td>
									<td align=right><?=$_Common->format_price($sum,true);?></td>
								</tr>
							<?php else:?>
								<tr>
									<td colspan="5" align="right"><?=$label;?>:</td>
									<td align="right" nowrap><?=$_Common->format_price($sum,true);?></td>
								</tr>
							<?php endif;?>

						<?php endforeach;?>
					<?php endif;?>

				<tr><td colspan="6">&nbsp;</td></tr>

				<tr>
					<td colspan="6">
						<hr size="1" noshade>
						<br /><b><?="$start - $end of $count Orders";?></b><br /><br />
						<?=$links;?>
					</td>
				</tr>

				</table>
				
			<?php else:?>
				<?php if(!empty($_REQUEST['order_number'])):?>
					<p>-- Order Number <?=$_REQUEST['order_number'];?> was not found --</p>
				<?php elseif(!empty($_REQUEST['customer_number'])):?>
					<p>-- An order for customer number <?=$_REQUEST['customer_number'];?> was not found --</p>
				<?php else:?>
					<p>-- No orders to display. --</p>
				<?php endif;?>
			<?php endif;?>
				
				
	</div>
<p>&nbsp;</p>
</body>
</html>
				
				
				
				
				
				
				
				
				
				