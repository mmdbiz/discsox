<?php
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "reports";

// initialize the program and read the config(s)
include_once("../../include/initialize.inc");
$init = new Initialize(true);

$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

if(isset($_REQUEST['available']) && isset($_REQUEST['update'])){
	foreach($_REQUEST['available'] as $iid=>$available){

		$min = $_REQUEST['minimum'][$iid];
		$sold = $_REQUEST['sold'][$iid];
		$_DB->execute("UPDATE inventory SET `quantity_available` = '$available', 
											`quantity_sold` = '$sold',
											`minimum_quantity` = '$min'
										WHERE `iid` = '$iid'");

	}
}

$sql = "SELECT products.sku, products.name, products.price,
			   inventory.iid, inventory.minimum_quantity, inventory.quantity_available,inventory.quantity_sold
		FROM products
		LEFT JOIN inventory ON products.pid = inventory.pid
		GROUP BY products.sku
		ORDER BY products.sku";

$data = $_DB->getRecords($sql);

?>
<html>
<head>
<title>Inventory List</title>
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
<body>
<h4 align="center">Simple Inventory List</h4>
<p align="center"><b>NOTE:</b> Does not include inventoried options.</p>
<form method="post" action="inventory.php">

	<table border=1 cellpadding=1 cellspacing=0 width="98%">
	<tr>
		<th align=left>Item</th>
		<th align=left>Name</th>
		<th align=left>Price</th>
		<th align=left>Min Qty</th>
		<th align=left>Available</th>
		<th align=left>Sold</th>
	</tr>
	<?php foreach($data as $i=>$row):?>
		<?php
			if($row['price'] == ""){
				$row['price'] = "&nbsp;"; 
			}
		?>
		<tr>
			<td style="padding-left:2px;"><?=$row['sku'];?></td>
			<td style="padding-left:2px;"><?=$row['name'];?></td>
			<td align="right"><?=$row['price'];?></td>
			<td align="center">
				<input type="text" size="6" name="minimum[<?=$row['iid'];?>]" value="<?=$row['minimum_quantity'];?>" style="text-align:right">
			</td>
			<td align="center">
				<input type="text" size="6" name="available[<?=$row['iid'];?>]" value="<?=$row['quantity_available'];?>" style="text-align:right">
			</td>
			<td align="center">
				<input type="text" size="6" name="sold[<?=$row['iid'];?>]" value="<?=$row['quantity_sold'];?>" style="text-align:right">
			</td>
		</tr>
	<?php endforeach;?>
	</table>
	
	<p align="center"><input type="submit" name="update" value="Update Inventory"></p>
</form>

</body>
</html>
