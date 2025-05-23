<?php
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "reports";

// initialize the program and read the config(s)
include_once("../../include/initialize.inc");
$init = new Initialize(true);

$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

// need this for date select
$_Form = $_Registry->LoadClass("form");
include_once("include/orders.inc");

$startDate = date("m-d");
$endDate = date("Y-m-d");
if(!empty($_REQUEST['search'])){
	$startDay = $_REQUEST['start-day'];
	$startMonth = $_REQUEST['start-month'];
	$startYear = $_REQUEST['start-year'];
	$endDay = $_REQUEST['end-day'];
	$endMonth = $_REQUEST['end-month'];
	$endYear = $_REQUEST['end-year'];
	$startDate = "$startYear-$startMonth-$startDay";
	$endDate = "$endYear-$endMonth-$endDay";
}
else{
	$_REQUEST['start-day'] = '01';
	$startDate = date("Y-m") . '-01';	
}

$debug = false;

// We really only want entries that show the page they left.
$empty = $_DB->getCount('abandoned_carts',"WHERE last_page = '' AND email_address = ''");
if($empty > 0){
	$_DB->execute("DELETE FROM abandoned_carts WHERE last_page = '' AND email_address = ''");
}

$orderCount = $_DB->getCount('orders',"WHERE `transaction_date` >= '$startDate' AND `transaction_date` <= '$endDate'");
$sql = "SELECT date,number_of_items,cart_total,last_page,email_address,username
		FROM abandoned_carts 
		WHERE (`date` >= '$startDate' AND `date` <= '$endDate') ORDER BY date, cart_total DESC LIMIT $orderCount";
		//$_Common->debugPrint($sql);
$data = $_DB->getRecords($sql);

$total = 0;
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
</head>
<body class="mainBody">

<div align=center valign=top>

	<h4><br />Abandoned Carts</h4>

	<form method="get" action="abandoned.carts.php">
		<p>
			From: <?=dateSelect(true);?> &nbsp;&nbsp;
			To: <?=dateSelect(false);?> &nbsp;&nbsp;
			<input type="submit" name="search" value="Go">
		</p>
	</form>

<?php if(count($data) == 0):?>

	<p><br />No abandoned carts to display.</p>

<?php else:?>

	<table border="0" cellpadding="3" cellspacing="0" width="80%">
		<tr>
			<th class="cartHeader">Date</th>
			<th class="cartHeader">Username</th>
			<th class="cartHeader" nowrap>Email Address</th>
			<th class="cartHeader" align="left">Last page visited</th>
			<th class="cartHeader"># of items</th>
			<th class="cartHeaderEnd">Cart total</th>
		</tr>
		<?php foreach($data as $i=>$fields):?>
		<?php 
			$total += $fields['cart_total']; 
			if($fields['username'] == ""){
				$fields['username'] = "&nbsp;";	
			}
			if($fields['email_address'] == ""){
				$fields['email_address'] = "&nbsp;";	
			}
		?>
		<tr>
			<td class="cartRow" align="center" nowrap><?=$fields['date'];?></td>
			<td class="cartRow" align="left"   nowrap><?=$fields['username'];?></td>
			<td class="cartRow" align="left"   nowrap><?=$fields['email_address'];?></td>
			<td class="cartRow" align="left"   nowrap width="50%"><?=$fields['last_page'];?></td>
			<td class="cartRow" align="center"><?=$fields['number_of_items'];?></td>
			<td class="cartRowEnd" align="right"><?=$fields['cart_total'];?></td>
		</tr>
		<?php endforeach;?>
		<tr><td class="cartRowEnd" colspan="6">&nbsp;</td></tr>
		<tr>
			<td class="cartRow" colspan="5" align="right">Total for <?=$i+1;?> abandoned carts: </td>
			<td class="cartRowEnd" align="right"><?=$_Common->format_price($total,true);?></td>
		</tr>
	</table>
	<p>There were <?=$orderCount;?> orders during this time period.</p>
<?php endif;?>
	
</div>
<p>&nbsp;</p>
</body>
</html>