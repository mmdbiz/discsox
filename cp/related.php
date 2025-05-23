<?php
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "related";

// initialize the program and read the config(s)
include_once("../include/initialize.inc");
$init = new Initialize();

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

if(isset($_REQUEST['apply_related'])){
	if(!empty($_REQUEST['pid']) && !empty($_REQUEST['related_item'])){
		$pid = $_REQUEST['pid'];
		$sql = "DELETE FROM related_products where pid = '$pid'";
		$result = $_DB->execute($sql);
		// may have duplicates selected because products may be in more than one category
		// only enter unique pids
		$pidDone = array();
		foreach($_REQUEST['related_item'] as $i=>$value){
			if(!isset($pidDone[$value])){
				$sql = "INSERT INTO related_products (pid, related_pid) VALUES ('$pid', '$value')";
				$result = $_DB->execute($sql);
			}
			$pidDone[$value] = 1;
		}
	}
	elseif(!empty($_REQUEST['pid']) && empty($_REQUEST['related_item'])){
		$pid = $_REQUEST['pid'];
		$sql = "DELETE FROM related_products where pid = '$pid'";
		$result = $_DB->execute($sql);
	}
}


$data = array();

	$pid = $_REQUEST['pid'];
	$sql = "SELECT * FROM related_products WHERE pid = '$pid'";
	$relatedPids = $_DB->getRecords($sql,"related_pid");
	
	//$_Common->debugPrint($relatedPids,"relatedPids");

    $sql = "SELECT categories.*, COUNT(product_categories.pid) AS count
            FROM categories LEFT JOIN product_categories ON categories.catid = product_categories.catid
			GROUP BY categories.catid ORDER BY category_name";
    $data = $_DB->getRecords($sql,'category_link');
    uksort($data, "strnatcasecmp");

	foreach($data as $link=>$flds){
		$catid = $flds['catid'];
		$sql = "SELECT products.pid,products.name
				FROM products,product_categories
				WHERE product_categories.pid = products.pid
				AND product_categories.catid = '$catid'";
		$products = $_DB->getRecords($sql);
		$data[$link]['products'] = $products;
	}

//$_Common->debugPrint($data,"Categories");

$counter = 0;
$totalItems = 0;
$totalSelected = 0;	
// row backround colors
$color = array();
$color[0] = "#FFFFFF";
$color[~0] = "#E2EDE2";

$color2 = array();
$color2[0] = "#FFFFFF";
$color2[~0] = "#E2EDE2";

$ck = 0;
$ck2 = 0;
?>
<html>
<head>
<title>Related Product List</title>

<style>
.section{
	font-weight:300;
	color:black;
	font-size:10px;
	cursor:hand;
}
.sectionHover{
	font-size:10px;
	font-weight:700;
	color:red;
	cursor:hand;
}
</style>

<script LANGUAGE="JavaScript">
//<!--
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

function showIt(whichEl){

    if(document.all){
        whichEl = document.all[whichEl];
    }
    else{
        whichEl = document.getElementById(whichEl);
    }

    whichEl.style.display = (whichEl.style.display == "none" ) ? "" : "none";

}
//-->
</SCRIPT>
</head>


<body class="mainBody">
<div align=center valign=top style="margin-top:10px;">

<?php if(count($data) > 0):?>

	<h4>Related Items List</h4>
	<p style="font-size:12px;">Click on category name to select related products.</p>
	
	<form method=post action="related.php" enctype="multipart/form-data">
	<input type="hidden" name="pid" value="<?=$pid;?>">

	<table border="1" cellpadding="3" cellspacing="0" align="center" width="95%">
		<tr>
			<th align="left" colspan="2">Categories</th>
		</tr>

		<?php foreach($data as $index=>$row):?>
		<?php
			$counter++; 
			$totalItems+=$row['count'];
		?>
		<tr bgcolor="<?=$color2[$ck2 = ~$ck2];?>">
			<td valign="top" width="30%" nowrap>
				<?php if(count($row['products']) > 0):?>
					<span class="section" onClick="showIt('cat.<?=$counter;?>');" onMouseOver="this.className='sectionHover';" onMouseOut="this.className='section'">
					<?=$row['category_link'];?> (<?=$row['count'];?>)</span>
				<?php else:?>
					<?=$row['category_link'];?>
				<?php endif;?>
			</td>
			<td>
				<?php if(count($row['products']) > 0):?>
					<?php $showThis = false; ?>
					<div id="cat.<?=$counter;?>" name="cat.<?=$counter;?>" style="display=none;">
						<table border="1" cellpadding="3" cellspacing="0" width="100%">
							<tr>
								<th align=center>Related</th>
    							<th align=left>Products</th>
    						</tr>
    						<?php foreach($row['products'] as $pkey=>$pdata):?>
    							<?php
    								$extraAttrib = "";
    								if(isset($relatedPids[$pdata['pid']])){
    									$extraAttrib = "checked";
    									$showThis = true;
    									$totalSelected++;
    								}
    							?>
    							<tr bgcolor="<?=$color[$ck = ~$ck];?>">
    								<td style="line-height:11px;" align="center" width="50">
    									<input type='checkbox' name='related_item[]' value='<?=$pdata['pid'];?>' <?= $extraAttrib; ?>>
    								</td>
    								<td style="line-height:11px;">
    									<?=strip_tags($pdata['name']);?>
    								</td>
    							</tr>
    						<?php endforeach;?>
    					</table>
					</div>
    				<?php if($showThis):?>
    					<script language="javascript">showIt('cat.<?=$counter;?>');</script>
    				<?php endif;?>
    				&nbsp;
				<?php else:?>
					&nbsp;
				<?php endif;?>
			</td>
		</tr>
		
		<?php endforeach;?>
		
		<tr><td colspan="2">&nbsp;</td></tr>
		<tr><td colspan="2">
				<b>Total items in database = </b><?=$totalItems;?> &nbsp;
				<b>Total items selected = </b><?=$totalSelected;?>
			</td>
		</tr>
	</table>
	<p>
		<input class="buttons" type="submit" name="apply_related" value="Apply Related">
		<input class="buttons" type="button" name="close" value="Close Window" onClick="window.close()">
	</p>

<?php else:?>
	<p>No categories/products have been defined in the database.</p>
<?php endif;?>

</form>
</div>
<p>&nbsp;</p>

</body>
</html>






