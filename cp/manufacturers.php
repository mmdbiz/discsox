<?php
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "manufacturers";

// initialize the program and read the config(s)
include_once("../include/initialize.inc");
$init = new Initialize();

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

$idFld = "mid";
$mid = null;
$phpFile = "manufacturers.php";
$data = array();
$fldProperties = array();

$add = false;

if(isset($_REQUEST['add'])){
	$add = true;
}
elseif(isset($_REQUEST['insert'])){
	$edit = true;
	$name = $_REQUEST['name'];
	$display = $_REQUEST['display'];
	$sql = "INSERT INTO manufacturers (mfg_name,display_mfg) VALUES('$name','$display')";
	$_DB->execute($sql);
}
elseif(isset($_REQUEST['update']) && isset($_REQUEST['name']) && is_array($_REQUEST['name'])){
	$names = $_REQUEST['name'];
	$displays = $_REQUEST['display'];
	foreach($names as $mid=>$name){
		$display = $_REQUEST['display'][$mid];
		$sql = "UPDATE manufacturers SET mfg_name = '$name', display_mfg = '$display' WHERE mid = '$mid'";
		$_DB->execute($sql);
	}
}
elseif(isset($_REQUEST['delete']) && !empty($_REQUEST['mid'])){
	$edit = false;
	$add = false;
	$mid = $_REQUEST['mid'];
	$_DB->execute("DELETE FROM manufacturers WHERE mid = '$mid'");
	$_DB->execute("UPDATE products SET mid = '1' WHERE mid = '$mid'");
}
if(!$add){
	$data = $_DB->getRecords("SELECT * FROM manufacturers");
}

?>

<html>
<head>
<title>Product List</title>
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

function showIt(whichEl){

    var imageID = "img." + whichEl;

    if(document.all){
        whichEl = document.all[whichEl];
        imageEl = document.all[imageID];
    }
    else{
        whichEl = document.getElementById(whichEl);
        imageEl = document.getElementById(imageID);
    }

    whichEl.style.display = (whichEl.style.display == "none" ) ? "" : "none";

        // Change images

    var imgPath = unescape(imageEl.src).split('/');
    var imgName = imgPath[imgPath.length - 1];
    if(imgName == "plus.gif"){
        imageEl.src = "images/minus.gif";
    }
    else{
        imageEl.src = "images/plus.gif";
    }
}

//-->
</script>
<script LANGUAGE="JavaScript" src="javascripts/products.js"></script>

</head>
<body class="mainForm">
<div align=center valign=top style="margin-top:10px;">

<?php if(!$add && count($data) == 0):?>
	<p>No manufacturers have been defined in the database.</p>
<?php else:?>

<form id="frmMain" method=post action="manufacturers.php" enctype="multipart/form-data">

	<table border="0" cellpadding="2" cellspacing="1" ID="Table1" width="550">

	<?php if($add):?>
		<tr>
			<th align="left" colspan="2">Add Manufacturer</th>
		</tr>
		<tr>
			<td colspan="2">&nbsp;</td>
		</tr>
		<tr>
			<td align=right>Manufacturer Name: </td>
			<td><input type="text" name="name" value="" size="60"></td>
		</tr>
		<tr>
			<td align=right>Display Their Products: </td>
			<td>
				<select name="display">
					<option value="true" selected>true</option>
					<option value="false">false</option>
				</select>
			</td>
		</tr>
		<tr>
			<td colspan="2">&nbsp;</td>
		</tr>
		<tr>
			<td colspan="2" align="center">
				<input type="submit" name="insert" value="Add Manufacturer"> &nbsp;
				<input type="submit" name="list" value="List Manufacturers">
			</td>
		</tr>

	<?php else:?>

		<tr>
			<td align="left" colspan="3"><b>Manufacturers List</b></td>
		</tr>
		<tr>
			<td colspan="3">&nbsp;</td>
		</tr>
		<tr>
			<th align="left">Name</th>
			<th>Display</th>
			<th>Delete</th>
			<th>Products</th>
		</tr>

		<?php
			$color = array();
			$color[0] = "#e2eDe2";
			$color[~0] = "#FFFFFF";
			$ck = 0;
		?>

		<?php foreach($data as $index=>$fields):?>
			<tr bgcolor="<?=$color[$ck = ~$ck];?>">
				<td><input type="text" name="name[<?=$fields['mid'];?>]" value="<?=$fields['mfg_name'];?>" size="60"></td>
				<td align=center valign=top>
					<select name="display[<?=$fields['mid'];?>]">
						<?php if($fields['display_mfg'] == "true"):?>
							<option value="true" selected>true</option>
							<option value="false">false</option>
						<?php else:?>
							<option value="true">true</option>
							<option value="false" selected>false</option>
						<?php endif;?>
					</select>
					
				</td>
				<td align=center valign=middle><a href="<?=$phpFile;?>?delete=true&mid=<?=$fields['mid'];?>" onclick="return confirm('Are you sure you want to delete this manufacturer?')"><img src="icons/trash.gif" border="0" alt="Delete"></a></td>
				<td align=center valign=middle><a href="products.php?mid=<?=$fields['mid'];?>">Items</a></td>
			</tr>
		<?php endforeach;?>

		<tr>
			<td colspan="4">&nbsp;</td>
		</tr>
		<tr>
			<td colspan="4" align="center">
				<input type="submit" name="update" value="Update Manufacturers"> &nbsp;
				<input type="submit" name="add" value="Add New Manufacturer">
			</td>
		</tr>

	<?php endif;?>

	</table>
	
<?php endif;?>

</form>
</div>
<p>&nbsp;</p>
</body>
</html>









