<?php
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "calculations";

// initialize the program and read the config(s)
include_once("../include/initialize.inc");
$init = new Initialize();

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

$calcs = $_Registry->LoadClass("calculations");
$fldProperties = array();

$data = array();
$display = false;
$detail = false;
$add = false;
$edit = false;
$welcome = true;
$title = NULL;

if(!empty($_REQUEST['detail'])){
	$detail = true;	
	$_REQUEST['display'] = 1;
}

$idFld = NULL;
$type = NULL;

if(!empty($_REQUEST['type'])){
    switch($_REQUEST['type']){
		case "discounts":
			$idFld = "did";
			$type = "discounts";
			$title = "Order Discounts";
			break;
		case "sales_tax":
			$idFld = "stid";
			$type = "sales_tax";
			$title = "General Sales Tax Settings";
			break;
		case "sales_tax_us":
			$idFld = "stusid";
			$type = "sales_tax_us";
			$title = "US Sales Tax";
			break;
		case "sales_tax_ca":
			$idFld = "stcaid";
			$type = "sales_tax_ca";
			$title = "Canadian Sales Tax";
			break;
		case "sales_tax_vat":
			$idFld = "stvatid";
			$type = "sales_tax_vat";
			$title = "VAT Sales Tax";
			break;
		case "shipping":
			$idFld = "shid";
			$type = "shipping";
			$title = "General Shipping Settings";
			break;
		case "shipping_rates":
			$idFld = "srid";
			$type = "shipping_rates";
			$title = "Shipping Rates";
			break;
		case "calculation_sequence":
			$idFld = "csid";
			$type = "calculation_sequence";
			$title = "Calculation Sequence";
			break;
		case "price_code_calc_sequence":
			$idFld = "pcsid";
			$type = "price_code_calc_sequence";
			$title = "Price Code Calculation Sequence";
			break;
			
	}
}


foreach(array_keys($_REQUEST) as $i=>$key){

    $RUN = false;
    switch($key){
        case "display":
            $display = true;
            $calcs->display($type);
            $RUN = 1;
            break;
        case "add":
			$add = true;
			$edit = true;
            $calcs->add($type);
            $RUN = 1;
            break;
        case "edit":
			$edit = true;
            $calcs->display($type);
            $RUN = 1;
            break;
        case "add_new":
        case "update":
            $display = true;
            $calcs->update($type);
			if($type == "shipping" || $type == "sales_tax" || $type == "calculation_sequence"){
				$display = false;
				$edit = true;
				$_REQUEST['edit'] = true;
			}
            $calcs->display($type);
            $RUN = 1;
            break;
        case "delete":
			$display = true;
            $calcs->update($type);
            $calcs->display($type);
            $RUN = 1;
            break;
    } // End switch

    if($RUN){
        break;
    }

}

$help = array();
// get the help for the keys being displayed.
$rs = $_DB->execute("SELECT * FROM help WHERE `section` = '$type'");
if($_DB->numrows($rs) > 0){
	while($row = $_DB->fetchrow($rs, "ASSOC")){
		if(!empty($row['section_help'])){
			$help['section_help'] = $row['section_help'];
		}
		elseif(!empty($row['key_help'])){
			$help[$row['key']] = $row['key_help'];
		}
	}
}
//$_Common->debugPrint($help);
?>

<html>
<head>
<title>welcome...</title>
<script language="JavaScript">
//<!--
if(eval(parent.menu)) {
	var fileName = parent.menu.location.pathname.substring(parent.menu.location.pathname.lastIndexOf('/')+1);
	if(fileName != "calcs.menu.html"){
		parent.menu.location = 'menus/calcs.menu.html';
	}
}
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
    
    
// -------------------------------------------------------------------
function OpenQtyWindow(fld){
    var winQuery = "templates/qty.ranges.html?fldid=" + fld;
	var qtyWindow = window.open(winQuery,"_blank","toolbar=no,scrollbars=yes,resizable=yes,width=400,height=475,screenX=25,screenY=75,top=25,left=75");

    if(!qtyWindow.opener){
        qtyWindow.opener = self;
    }
return false;
}
</script>
<style>
.calcsHeader{
	border-style: solid;
	border-color: #CDCDCD;
	border-width: 1px 0px 1px 1px;
	background-color: #3366CC;
	color: #ffffff;
	font-family: Arial, Helvetica, sans-serif;
	font-size: 12px;
}
.calcsHeaderEnd{
	border-style: solid;
	border-color: #CDCDCD;
	border-width: 1px 1px 1px 1px;
	background-color: #3366CC;
	color: #ffffff;
	font-family: Arial, Helvetica, sans-serif;
	font-size: 12px;
}
.calcsRow{
	vertical-align: top;
	font-size: 10px;
	border-style: solid;
	border-color: #CDCDCD;
	border-width: 0px 0px 1px 1px;
}
.calcsRowEnd{
	vertical-align: top;
	font-size: 10px;
	border-style: solid;
	border-color: #CDCDCD;
	border-width: 0px 1px 1px 1px;
}
</style>
</head>
<body>
<div align="center">

<?php if($display || $edit):?>
	<br/>
	<form method="POST"	action="calculations.php">
	<h4><?=$title;?></h4>
	<table border="0" width="90%" cellspacing="0" cellpadding="3" align="center">
<?php endif;?>

<?php if(($display || $edit) && count($data) == 0):?>

	<tr><td align="center">No records to display</td></tr>
	<tr>
		<td align="center"><br/><br/>
			<a href="calculations.php?add=true&type=<?=$type;?>">Add New Entries</a>
		</td>
	</tr>

<?php elseif($display):?>

	<?php
		// remove the help fields
		foreach($data as $h=>$flds){
			foreach($flds as $k=>$v){
				if(substr($k,-4) == "help"){
					$data[$h][$k] = NULL;
					unset($data[$h][$k]);
				}	
			}
		}
	?>


    <?php $headers = array_keys($data[0]);?>
    <?php $colSpan = count($headers) + 2;?>
    <tr>
        <?php foreach($headers as $i=>$key):?>
			<?php $displayKey = ucwords(preg_replace("|\_|"," ",$key));?>
			<?php if($key == $idFld){continue;}?>
            <th class="calcsHeader" align="left" valign="top"><?=$displayKey;?></th>
        <?php endforeach;?>
        <th class="calcsHeaderEnd" colspan="2" valign="top">Manage</th>
    </tr>

    <?php
		$color = array();
		$color[0] = "#FFFFFF";
		$color[~0] = "#e2eDe2";
        $ck = 0;
    ?>

    <?php foreach($data as $index=>$fields):?>
        <tr bgcolor="<?=$color[$ck = ~$ck];?>">
            <?php foreach($fields as $key=>$value):?>
                <?php if($key == $idFld){$id = $value; continue;}?>
                <?php if($key == "help"){continue;}?>
                <?php if(trim($value) == ""){$value = "&nbsp;";}?>
                <td class="calcsRow"><?=$value;?></td>
            <?php endforeach;?>
            <td class="calcsRowEnd" align="center" valign="top" colspan="2" nowrap>
				<a href="calculations.php?edit=true&<?=$idFld;?>=<?=$id;?>&type=<?=$type;?>">Edit</a> |
				<a href="calculations.php?delete=true&<?=$idFld;?>=<?=$id;?>&type=<?=$type;?>">Delete</a>
			</td>
        </tr>
    <?php endforeach;?>

		<tr>
			<td colspan="<?=$colSpan;?>" align="center"><br/><br/>
				<a href="calculations.php?add=true&type=<?=$type;?>">Add New Entries</a>
			</td>
		</tr>
		
<?php elseif($edit):?>

    <input type="hidden" name="<?=$idFld;?>" value="<?=$data[0][$idFld];?>">
    <input type="hidden" name="type" value="<?=$type;?>">

	<?php if(!empty($data[0]['section_help'])):?>
		<tr><td colspan="2"><?=$data[0]['section_help'];?></td></tr>
		<tr><td colspan="2"><hr size="1" noshade></td></tr>
		<?php unset($data[0]['section_help']);?>
	<?php elseif(!empty($help['section_help'])):?>
		<tr><td colspan="2"><?=$help['section_help'];?></td></tr>
		<tr><td colspan="2"><hr size="1" noshade></td></tr>
	<?php endif;?>

    <?php foreach($data[0] as $key=>$value):?>
		<?php if($key == $idFld){continue;}?>
        <?php $displayKey = ucwords(preg_replace("|\_|"," ",$key));?>
		<?php if(substr($key,-4) == "help" && trim($value) == ""){continue;}?>
		
		<tr>
        <?php if(!empty($help[$key])):?>
			<td valign="top" align="right" width="30%" nowrap>&nbsp;</td>
			<td valign="top" class="comments"><?=$help[$key];?></td>
		</tr>
		<tr>
        <?php endif;?>
			<td valign="top" align="right" width="30%" nowrap><?=$displayKey;?>: </td>
			<td>
				<?php if(strstr($value,"<select")):?>
					<?=$value;?>
				<?php elseif($key == 'catid'):?>
					<input type="hidden" name="<?=$key;?>" value="<?=$value;?>">
					<?=$value;?>
				<?php else:?>
					<?php if($fldProperties[$key][1] == "text" && strlen($value) > 50):?>
						<textarea name="<?=$key;?>" rows="5" cols="49" wrap="virtual"><?=$value;?></textarea>
					<?php else:?>
						<input type="text" name="<?=$key;?>" value="<?=$value;?>" size="50">
					<?php endif;?>
					<?php if(stristr($key,"range") || stristr($key,"surcharge")):?>
					<a href="#" onClick="return OpenQtyWindow('<?=$key;?>')">Create/Edit</a>
					<?php endif;?>
				<?php endif;?>
			</td>
        </tr>
    <?php endforeach;?>

	<tr><td colspan="2"><hr size="1" noshade></td></tr>
	<tr>
		<td colspan="2" align="center">
			<?php if($add):?>
				<input type="submit" name="add_new" value="Add Record">
			<?php else:?>
				<input type="submit" name="update" value="Update Record">
			<?php endif;?>
		</td>
	</tr>


<?php else:?>

	<table border="0" width="600" cellspacing="0" cellpadding="3" align="center">
	<tr>
		<td align="center"><h4 align="center">Order Calculations</h4></td>
	</tr>
	<tr>
		<td align="left" style="font-size:12px;">
		This section provides all the functions related to order calculations.
		Each section has detailed instructions for each setting that explains what
	   it does or the format of the data to be entered. Click a link in the menu on the left to begin.
	   </td>
	</tr>
	</table>

<?php endif;?>


<?php if($display || $edit):?>
</table>
</form>
<?php endif;?>


</div>
</body>
</html>

















