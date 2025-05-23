<?php
$_isAdmin = true;
$_adminFunction = "summary";

// initialize the program and read the config(s)
include_once("../include/initialize.inc");
$init = new Initialize();

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();


/* Summary of activity in the store */
$today = date("Y-m-d");

// orders
$shipped = $_DB->getCount("orders","WHERE status = 'Complete'");
$unShipped = $_DB->getCount("orders","WHERE status = 'Not Shipped' OR status = 'Partial Shipped'");
$newOrders = $_DB->getCount("orders","WHERE transaction_date = '$today'");
if(strtolower($_SESSION['title']) == "admin"){
	$sql = "SELECT SUM(grandtotal) as total FROM orders WHERE status = 'Complete'";
	$orderTotals = $_DB->getRecord($sql);
	$orderTotal = $_Common->format_number($orderTotals['total'],2,".",",");
}

// customers
$totalCustomers = $_DB->getCount("customers","WHERE active = 'true'");
$newCustomers = $_DB->getCount("customers","WHERE active = 'true' AND active_date = '$today'");

// top sellers
// WHERE order_details.orid = orders.orid AND DATE_SUB(CURDATE(),INTERVAL 7 DAY) <= orders.transaction_date
$sql = "SELECT order_details.sku, order_details.name, SUM(order_details.quantity) as qty, orders.transaction_date
		FROM order_details,orders
		WHERE order_details.orid = orders.orid
		GROUP BY order_details.sku ORDER BY qty DESC LIMIT 10";

$topSellers = $_DB->getRecords($sql);

?>

<html>
<head>
<title>welcome...</title>
<script language="JavaScript">
//<!--
    var sHeight = screen.height;
    var sWidth = screen.width;
    var styles = "admin.800.css";
    if(sWidth > 800){
        styles = "admin.1024.css";
    }
    if(sWidth > 1024){
        styles = "admin.1152.css";
    }
    if(sWidth > 1100){
        styles = "admin.1280.css";
    }
    document.write('<link rel="stylesheet" href="stylesheets/' + styles + '" type="text/css">');
    
</script>
<base target="content">
<style type="text/css">
body {
	margin-left: 0;
	margin-top: 20;
	margin-right: 0;
	margin-bottom: 0;
}
td{
	line-height: 17px;
}
.blankRow{
	line-height: 10px;
}
.highlight{
	background-color: #FFFF00;
	display: block;
}
.header {
     font-family: Verdana, Arial, Helvetica, sans-serif;
     color: #0005CE;
     font-size: 14px;
}
</style>
</head>
<body class=menuBackground>

<table align=left width="134" border="0" cellspacing="0" cellpadding="0" style="margin-left:3px; margin-right:3px;">

	<tr><td class="header"><br/><b>Store Summary</b></td></tr>
	<tr><td class="blankRow"><hr size="1" noshade width="125" align="left"></td></tr>

	<tr><td nowrap><b>Customers:</b><br></td></tr>
	<tr><td><li><?=$totalCustomers;?> total</td></tr>
	<?php if($newCustomers > 1):?>
		<tr><td class="highlight"><li><a href="reports.php?run=true&report=customers"><?=$newCustomers;?> new customers</a></td></tr>
	<?php elseif($newCustomers == 1):?>
		<tr><td class="highlight"><li><a href="reports.php?run=true&report=customers"><?=$newCustomers;?> new customer</a></td></tr>
	<?php else:?>
		<tr><td><li>0 new customers</td></tr>
	<?php endif;?>

	<tr><td class="blankRow"><hr size="1" noshade width="125" align="left"></td></tr>
	<tr><td nowrap><b>Orders:</b><br></td></tr>
	<tr><td><li><?=$shipped;?> shipped</td></tr>
	<tr><td><li><?=$unShipped;?> unshipped</td></tr>
	<?php if($newOrders > 1):?>
		<tr><td class="highlight"><li><a href="reports.php"><?=$newOrders;?> orders today</a></td></tr>
	<?php elseif($newOrders == 1):?>
		<tr><td class="highlight"><li><a href="reports.php"><?=$newOrders;?> order today</a></td></tr>
	<?php else:?>
		<tr><td><li>No orders today</td></tr>
	<?php endif;?>
	<?php if(strtolower($_SESSION['title']) == "admin"):?>
		<tr><td><li>YTD Shipped: <br/> &nbsp; &nbsp; <?=$orderTotal;?></td></tr>
	<?php endif;?>
	

	<tr><td class="blankRow"><hr size="1" noshade width="125" align="left"></td></tr>
	<tr><td nowrap><b>Top Sellers:</b><br></td></tr>
	
	<?php foreach($topSellers as $i=>$fields):?>
		<?php 
			if(strlen($fields['name']) > 12){
				$fields['name'] = substr($fields['name'],0,12) . "...";
			}
		?>
		<tr><td>(<?=$fields['qty'];?>) <?=$fields['name'];?></td></tr>
	<?php endforeach;?>
	
	
</table>
</body>
</html>
    
    
    