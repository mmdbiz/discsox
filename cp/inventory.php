<?php
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "products";

$debug = false;

// initialize the program and read the config(s)
include_once("../include/initialize.inc");
$init = new Initialize();

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

$maxPerScreen = 50;
$hits = 0;
if(!empty($_REQUEST['hits'])){
	$hits = intval($_REQUEST['hits']);	
}

if(!isset($_SESSION['sortDir'])){
	$_SESSION['sortDir'] = "ASC";
}

$sortFld = null;
$sortDir = "ASC";

if(!empty($_SESSION['sortby']) && !empty($_REQUEST['sortby']) &&  $_SESSION['sortby'] == $_REQUEST['sortby']){
	if(isset($_SESSION['sortDir']) && $_SESSION['sortDir'] == "ASC"){
		$sortDir = "DESC";
		$_SESSION['sortDir'] = "DESC";
	}
	else{
		$sortDir = "ASC";
		$_SESSION['sortDir'] = "ASC";
	}
}

if(!empty($_REQUEST['sortby'])){
	$sortFld = $_REQUEST['sortby'];
	$_DB->queryVars['sortby'] = $sortFld;
	$_SESSION['sortby'] = $sortFld;
}

$sku = null;
if(!empty($_REQUEST['sku'])){
	$sku = trim($_REQUEST['sku']);
	$_DB->queryVars['sku'] = $sku;
}


$data = array();
$fldProperties = array();


if(isset($_REQUEST['update'])){
	// do updates
	//$_Common->debugPrint($_REQUEST);
	//exit;
	
	if(isset($_REQUEST['products'])){
		$productFields = $_DB->getFieldProperties('products');
		foreach($_REQUEST['products'] as $pid=>$fields){
			$updateFlds = $_DB->makeUpdateFields($productFields,'pid',$fields);
			$sql = "UPDATE products SET $updateFlds WHERE pid = '$pid'";
			if($debug){
				$_Common->debugPrint($sql);
			}
			$_DB->execute($sql);

			if(isset($fields['quantity_available']) && $fields['inventory_item'] == 'true'){
				$qty = $fields['quantity_available'];
				$qtySold = $fields['quantity_sold'];
				if($_DB->getCount('inventory',"WHERE pid = '$pid'") > 0){
					$isql = "UPDATE inventory SET quantity_available = '$qty',quantity_sold = '$qtySold' WHERE pid = '$pid'";
				}
				else{
					$isql = "INSERT INTO inventory (pid,quantity_available,quantity_sold) VALUES('$pid','$qty','$qtySold')";
				}
				if($debug){
					$_Common->debugPrint($isql);
				}
				$_DB->execute($isql);
			}
		}
	}
	if(isset($_REQUEST['inventory'])){
		$invFields = $_DB->getFieldProperties('inventory');
		foreach($_REQUEST['inventory'] as $iid=>$fields){
			$updateFlds = $_DB->makeUpdateFields($invFields,'iid',$fields);
			$sql = "UPDATE inventory SET $updateFlds WHERE iid = '$iid'";
			if($debug){
				$_Common->debugPrint($sql);
			}
			$_DB->execute($sql);
		}
	}
}

$sql = "SELECT	products.pid,
				products.sku,
				products.name,
				products.inventory_item,
				products.inventory_options,
				products.display_when_sold_out,
				SUM(inventory.quantity_sold) as qty_sold
				FROM products LEFT JOIN inventory ON products.pid = inventory.pid";

if($sku){
	$len = strlen($sku);
	$sql .= " WHERE LEFT(sku,$len) = '$sku'";
}				
			
				
$sql .= " GROUP BY products.pid";

if($sortFld){
	$sql .= " ORDER BY $sortFld";
}
else{
	$sql .= " ORDER BY inventory_item ASC, name";
}

if($sortDir){
	$sql .= " " . $sortDir;
}

//print "<pre>$sql</pre>";

// get a count
$rs = $_DB->execute($sql);
$rsCount = $_DB->numrows($rs);
$_DB->free_result($rs);

if($rsCount > 0){
	// Set limits for full sql
	list($start,$end,$limits) = $_DB->getLimits($rsCount,$maxPerScreen,"inventory.php");

	// add previous/next links
	$previousNextLinks = $_DB->previousNextLinks;
	
	// add screen limits
	$sql .= $limits;
	
	// get rows
	$data = $_DB->getRecords($sql);
}


// row backround colors
$color = array();
$color[0] = "#FFFFFF";
$color[~0] = "#E2EDE2";
$ck = 0;

?>
<html>
<head>
<title>Inventory List</title>
<script LANGUAGE="JavaScript">
//<!--
if(eval(parent.menu)) {
	var fileName = parent.menu.location.pathname.substring(parent.menu.location.pathname.lastIndexOf('/')+1);
	if(fileName != "products.menu.html"){
		parent.menu.location = 'menus/products.menu.html';
	}
}
sWidth = screen.width;
var styles = "admin.800.css";
if(sWidth > 850){
    styles = "admin.1024.css";
}
if(sWidth > 1024){
    styles = "admin.1152.css";
}
if(sWidth > 1100){
    styles = "admin.1280.css";
}
document.write('<link rel="stylesheet" href="stylesheets/' + styles + '" type="text/css">');

function showIt(whichEl,clickedEl){
    if(document.all){
        whichEl = document.all[whichEl];
    }
    else{
        whichEl = document.getElementById(whichEl);
    }
    whichEl.style.display = (whichEl.style.display == "none" ) ? "" : "none";
    clickedEl.innerHTML = (clickedEl.innerHTML == "See options" ) ? "Hide options" : "See options";
}

//-->
</script>


</head>
<body>
<div align=center valign=top style="margin-top:10px;">

<form id="frmMain" method=post action="inventory.php">

<p>
	<b>Enter a few letters/numbers of the SKU: </b> <input type="text" name="sku" size="10">
	<input type="submit" name="go" value="Search">
</p>

<?php if(count($data) == 0):?>

	<p>No products found.</p>
	
<?php else:?>

	<table border="0" cellpadding="1" cellspacing="1" align="center" width="95%" ID="Table1">
		<tr>
			<td colspan="3"><h4>Product Inventory</td>
			<td colspan="4" align="right">&nbsp;</td>
		</tr>
		<tr>
			<td style="padding-bottom:5px;" colspan="3">Click on field name to sort results. Click again to toggle between ascending/descending</td>
			<td style="padding-bottom:5px;" colspan="4" align="right">
				<?=@$previousNextLinks;?>
			</td>
		</tr>
		<tr>
			<th><a href="inventory.php?list=true&sortby=sku"><font color=white>SKU</font></a></th>
			<th><a href="inventory.php?list=true&sortby=name"><font color=white>Name</font></a></th>
			<th><a href="inventory.php?list=true&sortby=inventory_item"><font color=white>Inventory Item</font></a></th>
			<th><a href="inventory.php?list=true&sortby=quantity_available"><font color=white>Qty Available</font></a></th>
			<th><a href="inventory.php?list=true&sortby=qty_sold"><font color=white>Qty Sold</font></a></th>
			<th><a href="inventory.php?list=true&sortby=inventory_options"><font color=white>Inventory Options</font></a></th>
			<th><a href="inventory.php?list=true&sortby=display_when_sold_out"><font color=white>Display When Sold Out</font></a></th>
		</tr>

		<?php foreach($data as $index=>$row):?>

			<?php
				$pid = $row['pid'];
				$available = 0;
				$invOptions = array();
				if($row['inventory_item'] == 'true'){
					if($row['inventory_options'] == 'true'){
						$invOptions = $_DB->getRecords("SELECT * FROM inventory WHERE pid = '$pid'");
						$available = "";
					}
					else{
						$invData = $_DB->getRecord("SELECT quantity_available FROM inventory WHERE pid = '$pid'");
						if(isset($invData['quantity_available'])){
							$available = $invData['quantity_available'];
						}
					}
				}
				if($row['qty_sold'] == ""){
					$row['qty_sold'] = 0;	
				}
			?>

			<tr valign="middle" bgcolor="<?=$color[$ck = ~$ck];?>">
				<td>
					<a href="products.php?edit=true&pid=<?=$row['pid'];?>"><?=$row['sku'];?></a>
				</td>
				<td width="50%">
					<?=stripslashes($row['name']);?>
				</td>
				<td align="center">
					<?=$_Common->makeSimpleSelectBox("products[" . $row['pid'] . "][inventory_item]",array('true','false'),array('true','false'),$row['inventory_item']);?>
				</td>
				<td align="center">
					<?php if(is_numeric($available)):?>
						<input class="rightalign" type="text" size="5" name="products[<?=$row['pid'];?>][quantity_available]" value="<?=$available;?>">
					<?php else:?>
						<div style="color:blue;cursor:hand;text-decoration:underline" onclick="showIt('options.<?=$pid;?>',this)">See options</div>
					<?php endif;?>
				</td>
				<td align="right">
					<?php if(is_numeric($available)):?>
					<input class="rightalign" type="text" size="5" name="products[<?=$row['pid'];?>][quantity_sold]" value="<?=$row['qty_sold'];?>">
					<?php else:?>
					<?=$row['qty_sold'];?>
					<?php endif;?>
				</td>
				<td align="center">
					<?=$_Common->makeSimpleSelectBox("products[" . $row['pid'] . "][inventory_options]",array('true','false'),array('true','false'),$row['inventory_options']);?>
				</td>
				<td align="center">
					<?=$_Common->makeSimpleSelectBox("products[" . $row['pid'] . "][display_when_sold_out]",array('true','false'),array('true','false'),$row['display_when_sold_out']);?>
				</td>
			</tr>

			<?php if(count($invOptions) > 0):?>
				<tr id="options.<?=$pid;?>" style="display:none">
					<td>&nbsp;</td>
					<td colspan="5" align="right">
 		  				<table border="1" cellpadding="3" cellspacing="0" style="border-collapse:collapse;" width="60%" align="right">
 		  					<tr>
 		  						<th>Option(s)</th>
 		  						<th>Available</th>
 		  						<th>Sold</th>
 		  					</tr>
 		  					<?php foreach($invOptions as $i=>$flds):?>
 		  					<tr>
 		  						<td>
 		  							<?=$flds['name'];?>
 		  						</td>
 		  						<td align="center">
 		  							<input class="rightalign" type="text" name="inventory[<?=$flds['iid'];?>][quantity_available]" value="<?=$flds['quantity_available'];?>" size="10">
 		  						</td>
 		  						<td align="center">
 		  							<input class="rightalign" type="text" name="inventory[<?=$flds['iid'];?>][quantity_sold]" value="<?=$flds['quantity_sold'];?>" size="10">
 		  						</td>
 		  					</tr>
 		  					<?php endforeach;?>
 		  				</table>
					</td>
				</tr>
			<?php endif;?>


		<?php endforeach;?>

	</table>
	<p><input type="submit" name="update" value="Submit Updates"></p>
	<p><?=@$previousNextLinks;?></p>
	
<?php endif;?>

</form>
</div>
<p>&nbsp;</p>
</body>
</html>









