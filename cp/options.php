<?php
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "options";

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
$pid = NULL;
$pname = NULL;
$optionsAreInventoried = false;

if(!empty($_REQUEST['pid'])){
	$pid = trim($_REQUEST['pid']);
	$pname = trim($_REQUEST['pname']);

	$invOptionCount = $_DB->getCount('products',"WHERE pid = '$pid' AND inventory_item = 'true' AND inventory_options = 'true'");
	if($invOptionCount > 0){
		$optionsAreInventoried = true;
	}
}

$fldProperties = $_DB->getFieldProperties('options');
$data = array();
$options = array();
$selectedOptions = array();
$isInventoried = false;

$add = false;
$edit = false;
$vars = array();

$optionClass = $_Registry->LoadClass("options");

	$RUN = false;
	foreach($_REQUEST as $key=>$value){
		switch($key){

			case "add":
			case "edit":
				$optionClass->editOptions();
				$edit = true;
				$RUN = 1;
				break;

			case "apply":
				$optionClass->applyOptions();
				$optionClass->listOptions();
				$RUN = 1;
				break;

			case "list":
				$optionClass->listOptions();
				$RUN = 1;
				break;

			case "update":
			case "insert":
				$optionClass->updateOptions();
				$optionClass->editOptions();
				$edit = true;
				$RUN = 1;
				break;

			case "delete":
				$optionClass->deleteOptions();
				$optionClass->listOptions();
				$RUN = 1;
				break;
		}
		if($RUN){
			break;
		}
	}
	
	if(!$RUN){
		$optionClass->listOptions();
	}

?>
<html>
<head>
<title>Options List</title>
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
//-->
</script>
<script LANGUAGE="JavaScript" src="javascripts/products.js"></script>
</head>
<body class="mainForm">
<div align="center" valign="top" style="margin-top:10px;">

<?php if(count($data) == 0):?>

	<p>No options have been defined in the database.</p>

<?php elseif($edit):?>

	<?php error_reporting(E_PARSE|E_WARNING);?>

	<form name="optionEdit" method="post" action="options.php">
	<input type="hidden" name="oid" value="<?=$data['oid'];?>">
	
	<?php if(isset($_REQUEST['add'])):?>
		<h4 style="line-height:10px;">Add New Option</h4>
	<?php else:?>
		<h4 style="line-height:10px;">Edit Option</h4>
	<?php endif;?>
	
	<table border=0 cellpadding=3 cellspacing=0 width=655>
  		<tr>
			<td align=right valign=middle>Option Name:</td>
			<td valign=top>
	  			<input type="text" name="name" value="<?=$data['name'];?>">
			</td>
			<td align=right valign=middle nowrap>Autofill Add Text:</td>
			<td valign=top>
			<?php if($data['autofill_text'] == "true"):?>
				<input type="radio" name="autofill_text" value="true" checked> Yes &nbsp;
				<input type="radio" name="autofill_text" value="false"> No
			<?php else:?>
				<input type="radio" name="autofill_text" value="true"> Yes &nbsp;
				<input type="radio" name="autofill_text" value="false" checked> No
			<?php endif;?>
			</td>				
		</tr>
		
  		<tr>
  			<td align=right valign=middle>Option Format:</td>
			<td valign=top>
				<?=$_DB->getDefaultValues("format",$data['format']);?>
			</td>
			<td align=right valign=middle>Add Text:</td>
			<td valign=top>
	  			<input type="text" name="add_text" value="<?=$data['add_text'];?>">
			</td>				
		</tr>
		
  		<tr>
			<td align=right valign=middle nowrap>Option Required:</td>
			<td align=left valign=middle>
				<?php if($data['required'] == "true"):?>
					<input type="radio" name="required" value="true" checked> Yes &nbsp;
					<input type="radio" name="required" value="false"> No
				<?php else:?>
					<input type="radio" name="required" value="true"> Yes &nbsp;
					<input type="radio" name="required" value="false" checked> No
				<?php endif;?>
			</td>
			<td align=right valign=middle>Option Type:</td>
			<td valign=top>
				<?=$_DB->getDefaultValues("type",$data['type']);?>
			</td>
		</tr>
  		<tr>
  			<td align=right valign="middle" nowrap>Number of Option Values:</td>
			<td valign=top>
				<input type="text" name="option_rows" value="<?=$data['option_rows'];?>" size="5" style="text-align:right">
			</td>
			<td colspan=2>&nbsp;</td>				
		</tr>
		
  		<?php if($isInventoried):?>
			<tr>
				<td colspan=4>&nbsp;</td>
			</tr>
  		  	<tr>
  		  		<td colspan="4" align="center" valign="top">
  		  		
  		  			<table border="0" cellpadding="5" cellspacing="3" width="100%">
  		  				<tr>
  		  					<td align="center" valign="top">
  		  						<img src="icons/warning.gif" alt="Caution"><br>
  		  						<b style="color:red">CAUTION</b>
  		  					</td>
  		  					<td valign="top">
								One or more products are using this option for inventory purposes.  
								If you <u>remove</u> any existing values or <u>add</u> new option values below,
								the inventory records will be reset for all products that inventory this option.
  		  					</td>
  		  				</tr>
  		  			</table>
  		  		
  		  		</td>
			</tr>
		<?php endif;?>
		<tr>
			<td colspan=4>&nbsp;</td>
		</tr>
	</table>
		
		
	<table id='options' width=600 border="0" cellpadding=3 cellspacing=1>
		<thead>
            <th align=left valign=top>Order</th>
			<th align=left valign=top>Free SKUs separated by comma</th>
			<th align=left valign=top>Price</th>
			<th align=left valign=top>Weight</th>
			<th align=left valign=top>Promotions Display Text</th>
			<th>&nbsp;</th>
		</thead>
		<tbody>
		<?php for($i=0;$i<=$data['option_rows'];$i++):?>
		<tr>
            <td align=left valign=top>
				<input type="text" name="sequence[<?=$i;?>]" value="<?=$options[$i]['sequence'];?>" size="5" style="text-align:center">
				<input type="hidden" name="odid[<?=$i;?>]" value="<?=$options[$i]['odid'];?>">
            </td>
			<td align=left valign=top>
				<input type="text" name="value[<?=$i;?>]" value="<?=$options[$i]['value'];?>" size=35 onBlur="autofillText(this.form,'<?=$i;?>');">
            </td>
			<td align=left valign=top>
				<input type="text" name="price[<?=$i;?>]" value="<?=$options[$i]['price'];?>" size=10 style="text-align:right" onBlur="autofillText(this.form,'<?=$i;?>')">
            </td>
            <td align=left valign=top>
				<input type="text" name="weight[<?=$i;?>]" value="<?=$options[$i]['weight'];?>" size=5 style="text-align:right" >
            </td>
			<td align=left valign=top>
				<input type="text" name="text[<?=$i;?>]" value="<?=$options[$i]['text'];?>" size=35>
            </td>
			<td align="center" valign="middle">
				<?php if($options[$i]['value'] != ""):?>
					<a href="javascript:clearOptionRow(document.forms['optionEdit'],'<?=$i;?>');" title="Remove this line"><img src="icons/trash.gif" border="0" alt="Remove this line"></a>
				<?php else:?>
					&nbsp;
				<?php endif;?>
			</td>
		</tr>
		<?php endfor;?>
		<tbody>
	</table>

	<?php if(isset($_REQUEST['add'])):?>
		<p><input class="buttons" type="submit" name="insert" value="Add New Option"></p>
	<?php else:?>
		<p><input class="buttons" type="submit" name="update" value="Update Option"></p>
	<?php endif;?>

	</form>


<?php else:?>

	<?php error_reporting(E_PARSE|E_WARNING);?>

	<?php if($pid != ""):?>
		<form method="post" action="options.php">
		<input type="hidden" name="pid" value="<?=$pid;?>">
		<input type="hidden" name="pname" value="<?=$pname;?>">
		<h4>Select options for '<?=$pname;?>'</h4>    
	<?php else:?>
		<h4>Option List</h4>
	<?php endif;?>

	<?php if($optionsAreInventoried):?>
  		<table border="0" cellpadding="5" cellspacing="3" width="95%" style="padding-bottom:20px;">
  			<tr>
  		  		<td align="center" valign="top">
  		  			<img src="icons/warning.gif" alt="Caution"><br>
  		  			<b style="color:red">CAUTION</b>
  		  		</td>
  		  		<td valign="top">
					The product above is using one or more of these options for inventory purposes. 
					If you change any of the existing options or hit apply below, the inventory 
					records for this product will be reset.
  		  		</td>
  			</tr>
  		</table>
  		<br clear="all"/>
	<?php endif;?>


	<table border=0 cellpadding=3 cellspacing=1 align=center width="95%" ID="Table1">
		<tr>
			<th>&nbsp;</th>
			<?php if($pid != ""):?>
				<th>Select</th>
				<th>Order</th>
			<?php endif;?>
			<th>Name</th>
			<th>Description</th>
			<?php if($pid == ""):?>		
				<th colspan="2">Manage</th>
			<?php endif;?>
		</tr>

		<?php
			$counter = 0;
			$blnColor = false;
			$bgcolor1 = "#FFFFFF";
			$bgcolor2 = "#e2eDe2";
		?>


		<?php foreach($data as $index=>$row):?>
			<?php
				$oid = $row['oid'];
				$name = $row['name'];
				$description = $row['description'];

				$order = "";
				$checked = "";
				if(isset($selectedOptions[$oid])){
					$checked = " checked";
					$order = $selectedOptions[$oid]['sequence'];
				}
				
				$counter++;
				
				if($blnColor){
					$trColor = $bgcolor1;
					$blnColor = false;
				}
				else{
					$trColor = $bgcolor2;
					$blnColor = true;
				}
			?>
			
			<tr bgcolor="<?=$trColor;?>">
				<td valign="top" align="right" width=20><?=trim($counter);?></td>
				<?php if($pid != ""):?>
					<td valign="top" align="center">
						<input type="checkbox" name="oid[]" value="<?=$oid;?>"<?=$checked;?>>
					</td>
					<td valign="top" align="center">
						<input type="text" name="sequence[<?=$oid;?>]" value="<?=$order;?>" size="3">
					</td>
				<?php endif;?>
				<td valign="top" nowrap>&nbsp;<?=$name;?></td>
				<td valign="top" width="70%"><?=$description;?></td>
				<?php if($pid == ""):?>
					<td align="center" valign="middle" width="50"><a href="options.php?edit=true&oid=<?=$oid;?>" title="Edit"><img src="icons/txt.gif" border="0" alt="Edit"></a></td>
					<td align="center" valign="middle" width="50"><a href="options.php?delete=true&oid=<?=$oid;?>" title="Delete" onClick="return confirm('Are you sure you want to delete this option?')"><img src="icons/trash.gif" border="0" alt="Delete"></a></td>
				<?php endif;?>
			</tr>
		<?php endforeach;?>
		<tr><td colspan="7">&nbsp;</td></tr>
		<tr><td colspan="7"><b>Total options returned = </b><?=$counter;?></td></tr>
	</table>

    <?php if($pid != ""):?>
    <p>
		<input class="buttons" type="submit" name="apply" value="Apply Options">&nbsp;
		<input class="buttons" type="button" value="Close Window" onClick="javascript:window.close();">
	</p>
    </form>
	<?php endif;?>


<?php endif;?>

</div>
<p>&nbsp;</p>
</body>
</html>







