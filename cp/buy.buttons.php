<?php
$_isAdmin = true;
$_adminFunction = "buy.buttons";

// initialize the program and read the config(s)
include_once("../include/initialize.inc");
$init = new Initialize();

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();


// ADD WELCOME SCREEN WITH INSTRUCTIONS
// ADD ONE EXAMPLE WITH 2 OPTIONS


// Load button default values from database
$defaults = array();
if(isset($_REQUEST['save_defaults'])){
	$fldProperties = $_DB->getFieldProperties("buy_buttons");
	$values = $_DB->makeUpdateFields($fldProperties, 'bbid', $_REQUEST);
	$sql = "UPDATE buy_buttons SET $values WHERE bbid = '1' LIMIT 1";
	$_DB->execute($sql);
	$_REQUEST['defaults'] = "true";
}
$sql = "SELECT * FROM buy_buttons WHERE bbid = '1' LIMIT 1";
$defaults = $_DB->getRecord($sql);
extract($defaults);

// Load last values if we have them
if(count($_REQUEST) == 0 && isset($_SESSION['variables'])){
	if(isset($_REQUEST['clear'])){
		$_Common->unsetSessionVariable("variables");
	}
	else{
		$_REQUEST = $_SESSION['variables'];
		$_Common->unsetSessionVariable("make_code","request");
	}
}
if(empty($_SESSION['save_to_db'])){
	$_SESSION['save_to_db'] = "true";	
}
$today = date("Y-m-d");

$fields = array("sku"			=> NULL,
				"name"			=> NULL,
				"category"		=> NULL,
				"price"			=> NULL,
				"size"			=> NULL,
				"weight"		=> NULL,
				"page"			=> NULL,
				"last_modified" => $today,
				"option"		=> array(),
				"save_to_db"	=> NULL);

foreach($fields as $key=>$val){
	if(!empty($_REQUEST[$key])){
		//print "<pre>$key, " . print_r($_REQUEST[$key]) . "\n";
		if(is_array($fields[$key])){
			$fields[$key] = $_REQUEST[$key];
		}
		else{
			$fields[$key] = trim($_REQUEST[$key]);	
		}
	}	
}

if(isset($_REQUEST['htmlcode'])){
	$_Common->unsetSessionVariable("htmlcode","request");
}

//$_Common->debugPrint($_REQUEST);

$optionCount = 0;
$optionRows = 5;

if(isset($_REQUEST['optionCount'])){
    $optionCount = $_REQUEST['optionCount'];
    $_SESSION['optionCount'] = $optionCount;
}
elseif(isset($_SESSION['optionCount'])){
    $optionCount = $_SESSION['optionCount'];
}
if(isset($_REQUEST['optionRows'])){
    $optionRows = $_REQUEST['optionRows'];
    $_SESSION['optionRows'] = $optionRows;
}
elseif(isset($_SESSION['optionRows'])){
    $optionRows = $_SESSION['optionRows'];
}

if(count($_REQUEST) == 0){
	if(isset($_SESSION['variables']['wysiwyg']) && $_SESSION['variables']['wysiwyg'] == "true"){
		$_REQUEST['wysiwyg'] = "true";
	}
	if(isset($_SESSION['variables']['viewcart']) && $_SESSION['variables']['viewcart'] == "true"){
		$_REQUEST['viewcart'] = "true";
	}
}

$code = NULL;

if(!empty($_REQUEST['save_to_db']) && $_REQUEST['save_to_db'] == "true"){
	$_SESSION['save_to_db'] = "true";
	saveToDatabase();
}

if(!empty($_REQUEST['make_code'])){
	
	if(empty($_REQUEST['wysiwyg'])){
		$_REQUEST['wysiwyg'] = "false";
	}
	if(empty($_REQUEST['viewcart'])){
		$_REQUEST['viewcart'] = "false";
	}	
	
	$_SESSION['variables'] = $_REQUEST;
	makeCode();	
}


// ------------------------------------------------------------------
function makeCode(){
	
	global $code;
	global $_Common;
	global $fields;
	global $itemName;
	global $itemPrice;

	global $scripturl;
	global $buybuttontext;
	global $viewbuttontext;
	global $qtytext;
	global $add_image;
	global $view_image;
	
	$item = array();
	$pid = trim($_REQUEST['sku']);
	$itemName = trim($_REQUEST['name']);
	if(!strstr($_REQUEST['price'],":")){
		$itemPrice = $_Common->format_price($_REQUEST['price']);
	}
	else{
		$itemPrice = trim($_REQUEST['price']);
	}

	$item[0] = trim($_REQUEST['sku']);
	$item[1] = $itemPrice;
	$item[2] = $itemName;
	$item[3] = !empty($_REQUEST['size']) ? trim($_REQUEST['size']) : "NA";
	$item[4] = !empty($_REQUEST['weight']) ? trim($_REQUEST['weight']) : "NA";

	$itemTag = "item-" . join("|",$item);
	$qtyInputTag = "";

		switch($_REQUEST['inputType']){

			case "Text Box":
			$qtyInputTag .= "\t<p>$qtytext: <input type=\"text\" name=\"$itemTag\" size=3></p>\n";
			break;

			case "Select Box":
				$qtyInputTag = "\t<p>Quantity:\n\t<select name=\"$itemTag\">\n";
				for($l=0;$l<=10;$l++){
					$qtyInputTag .= "\t\t<option value=\"$l\">$l</option>\n";
				}
				$qtyInputTag .= "\t</select></p>\n";
			break;

			case "Checkbox":
				$qtyInputTag = "\t<p><input type=\"checkbox\" name=\"$itemTag\" value=1> $buybuttontext</p>";
			break;

			case "Order Button Only":
				$qtyInputTag = "\t<input type=\"hidden\" name=\"$itemTag\" value=1>";
			break;
		}

		//$qtyInputTag .= "\n";

	$pageName = $_REQUEST['page'];

	$code = "\n<p><b>$itemName - \$$itemPrice</b></p>\n\n";
	$code .= "<form method=\"post\" action=\"$scripturl?page=$pageName\">\n";

	$optionTags = "";
	for($k=0;$k<$_REQUEST['optionCount'];$k++){

		if(isset($_REQUEST['option'][$k]['name']) && trim($_REQUEST['option'][$k]['name']) != ""){

			//$_Common->debugPrint($_REQUEST['option'][$k]);
			//exit;

			$optionName = $_REQUEST['option'][$k]['name'];
			$format = $_REQUEST['option'][$k]['format'];
			$type = $_REQUEST['option'][$k]['type'];
			$odid = $k + 1;
			$required = $_REQUEST['option'][$k]['required'];
			
			//"Select Box","Radio Buttons","Text Box"

			switch($format){

				case "text box":
            		$optionTags .= "<p>$optionName: \n";
					$optionTags .= "<input type=\"text\" name=\"OPTION|$optionName|$pid\" value=\"\" size=\"20\">";
					$optionTags .= "</p>\n";           
				break;

				case "select box":
					$optionTags .= "\t<p><select name=\"OPTION|$optionName|$pid\">\n";
					
					if($required == "true"){
						$optionTags .= "\t\t<option value=\"invalid|$optionName\">Select $optionName</option>\n";
					}
					else{
						$optionTags .= "\t\t<option value=\"\">Select $optionName</option>\n";
					}
					
					for($l=0;$l<$_REQUEST['optionRows'];$l++){
						$val = trim($_REQUEST['option'][$k][$l]['value']);
						if($val != ""){
							$price = $_Common->format_price(trim($_REQUEST['option'][$k][$l]['price']));
							$weight = trim($_REQUEST['option'][$k][$l]['weight']) != "" ? trim($_REQUEST['option'][$k][$l]['weight']) : 0;
							$text = trim($_REQUEST['option'][$k][$l]['text']);
							if($text == ""){
								$text = $val;	
							}
							if($text == "" && $price > 0){
								$text .= " ($price)";	
							}
							
							$optionTags .= "\t\t<option value=\"$val|$price|$weight\">$text</option>\n";
						}
					}
					$optionTags .= "\t</select></p>\n";
				break;

				case "radio buttons":
					$optionTags .= "\t<p>$optionName:<br>\n";
					//print "Radio";
					for($l=0;$l<$_REQUEST['optionRows'];$l++){
						$val = trim($_REQUEST['option'][$k][$l]['value']);
						if($val != ""){
							$price = trim($_REQUEST['option'][$k][$l]['price']) != "" ? trim($_REQUEST['option'][$k][$l]['price']) : 0;
							$weight = trim($_REQUEST['option'][$k][$l]['weight']) != "" ? trim($_REQUEST['option'][$k][$l]['weight']) : 0;
							$text = trim($_REQUEST['option'][$k][$l]['text']);
							if($text == ""){
								$text = $val;	
							}						
							if($text == "" && $price > 0){
								$text .= " ($price)";	
							}
							
							if($l == 0 && $required == "true"){
								$optionTags .= "\t<input type=\"radio\" name=\"OPTION|$optionName|$pid\" value=\"$val|$price|$weight\" checked> $text<br>\n";
							}
							else{
								$optionTags .= "\t<input type=\"radio\" name=\"OPTION|$optionName|$pid\" value=\"$val|$price|$weight\"> $text<br>\n";
							}
						}
					}
					$optionTags .= "\t</p>\n";                            
				break;
			}
		}
	}
	$code .= $optionTags;
	$code .= $qtyInputTag;

	if(isset($add_image) && trim($add_image) != ""){
		$code .= "\t<p><input type=\"image\" src=\"$add_image\" name=\"add\" value=\"$buybuttontext\">&nbsp;\n";
	}
	else{
		$code .= "\t<p><input type=\"submit\" name=\"add\" value=\"$buybuttontext\">&nbsp;\n";
	}

	if(isset($_REQUEST['viewcart']) && $_REQUEST['viewcart'] == "true"){
		if(isset($view_image) && trim($view_image) != ""){
			$code .= "\t<input type=\"image\" src=\"$view_image\" name=\"view_cart\" value=\"$viewbuttontext\">&nbsp;\n";
		}
		else{
			$code .= "\t<input type=\"submit\" name=\"view_cart\" value=\"$viewbuttontext\">\n\t</p>\n";	
		}
	}

	$code .= "</form>\n";

	if(trim($itemName) == ""){
		$code = NULL;
	}
}

// ------------------------------------------------------------------
function saveToDatabase(){

	global $_Common;
	global $_DB;

	$productFlds = $_DB->getFieldProperties("products");
	$catid = NULL;
	$pid = NULL;

	//$_Common->debugPrint($_REQUEST);
	//exit;
	
	// Add/Update record in products table
	if(!empty($_REQUEST['sku']) && !empty($_REQUEST['name'])){
		
		$sku = trim($_REQUEST['sku']);
		$sql = "SELECT * FROM products WHERE sku = '$sku'";
		$data = $_DB->getRecord($sql);
		if(count($data) > 0 && !empty($data['pid'])){
			$pid = $data['pid'];
			$values = $_DB->makeUpdateFields($productFlds,'pid',$data);
			$sql = "UPDATE products SET $values WHERE pid = '$pid'";
			$_DB->execute($sql);
		}
		else{
			list($keys,$values) = $_DB->makeAddFields($productFlds,'pid',$_REQUEST);
			$sql = "INSERT INTO products ($keys) VALUES ($values)";
			$_DB->execute($sql);
			$pid = $_DB->getInsertID("products","pid");
		}

		if($pid && !empty($_REQUEST['option']) && count($_REQUEST['option']) > 0){
			// Add/update Options
			saveOptions($pid);
		}

		// add to categories or get catid of existing category
		if($pid && !empty($_REQUEST['category'])){
			$category = trim($_REQUEST['category']);
			$sql = "SELECT catid, category_name FROM categories WHERE LOWER(category_name) = LOWER('$category')";
			$catData = $_DB->getRecord($sql);
			if(count($catData) > 0){
				$catid = $catData['catid'];
				$category = $catData['category_name'];
			}
			else{
				$sql = "INSERT INTO categories (category_name,category_meta_description,category_link) VALUES ('$category','$category','$category')";
				$_DB->execute($sql);
				$catid = $_DB->getInsertID("categories","catid");
				$sql = "UPDATE `categories` SET category_ids = CONCAT(parentid, ':', catid) WHERE catid = '$catid'";
				$_DB->execute($sql);
			}
		}
		// Add to product categories merge table
		if($pid && $catid){
			$sql = "SELECT pid,catid FROM product_categories WHERE pid = '$pid' AND catid = '$catid'";
			$mergeData = $_DB->getRecord($sql);
			if(count($mergeData) == 0){
				$sql = "INSERT INTO product_categories (pid,catid) VALUES ('$pid','$catid')";
				$_DB->execute($sql);
			}
		}
	}
}
// ------------------------------------------------------------------
function saveOptions($pid){
	
	global $_Common;
	global $_DB;
	
	//$_Common->debugPrint($_REQUEST['option']);
	
	foreach($_REQUEST['option'] as $i=>$optionFlds){
		
		if(empty($optionFlds['name'])){
			continue;	
		}
		
		$name = $optionFlds['name'];
		$format = $optionFlds['format'];
		$type = $optionFlds['type'];
		$required = $optionFlds['required'];
		$desc = NULL;
		$oid = NULL;

		// Option description for control panel			
		$desc = NULL;
		for($k=0;$k<$_REQUEST['optionRows'];$k++){
			$val = trim($optionFlds[$k]['value']);
			if($val != ""){
				if($desc){
					$desc .= "," . $val;
				}
				else{
					$desc = $val;
				}
			}
		}
		
		$sql = "SELECT oid,description FROM options WHERE name = '$name' AND description = '$desc' AND format = '$format' AND type = '$type'";
		$optionData = $_DB->getRecord($sql);
		if(count($optionData) == 0){
			$sql = "INSERT INTO options (name,description,format,type,required) VALUES ('$name','$desc','$format','$type','$required')";
			$_DB->execute($sql);
			$oid = $_DB->getInsertID("options","oid");
		}
		else{
			$oid = $optionData['oid'];
			if($desc != $optionData['description']){
				$sql = "UPDATE options SET description = '$desc' WHERE oid = '$oid'";
				$_DB->execute($sql);
			}
		}

		if($oid){
			$odid = NULL;
			
			for($l=0;$l<$_REQUEST['optionRows'];$l++){
				$val = trim($optionFlds[$l]['value']);
				if($val != ""){
					$price = trim($optionFlds[$l]['price']) != "" ? trim($optionFlds[$l]['price']) : 0;
					$weight = trim($optionFlds[$l]['weight']) != "" ? trim($optionFlds[$l]['weight']) : 0;
					$text = trim($optionFlds[$l]['text']);
					if($text == ""){
						$text = $val;	
					}						
					if($text == "" && $price > 0){
						$text .= " ($price)";	
					}
					
					// check to see if it exists
					$sql = "SELECT odid FROM option_details WHERE oid = '$oid' AND value = '$val' AND text = '$text'";
					$optionDetail = $_DB->getRecord($sql);
					if(count($optionDetail) > 0){
						$odid = $optionDetail['odid'];
						$sql = "UPDATE option_details SET price = '$price', weight = '$weight' WHERE odid = '$odid'";
						$_DB->execute($sql);
					}
					else{
						$sql = "INSERT INTO option_details (oid,value,price,weight,text) VALUES ('$oid','$val','$price','$weight','$text')";
						$_DB->execute($sql);
						$odid = $_DB->getInsertID("option_details","odid");
					}
				}
			}

			$sql = "SELECT oid FROM product_options WHERE oid = '$oid' AND pid = '$pid'";
			$productToOption = $_DB->getRecord($sql);
			if(count($productToOption) == 0){
				$sql = "INSERT INTO product_options (oid,pid) VALUES ('$oid','$pid')";
				$_DB->execute($sql);
			}
		}
	}
}

// ------------------------------------------------------------------
function printOptions($fldName,$defaults,$selected = ""){
	foreach($defaults as $index=>$val){
        $label = $val;
        if($selected != "" && $val == $selected){
            print "<option value=\"$val\" selected>$label</option>\n";
		}
        elseif(isset($_REQUEST[$fldName]) && $_REQUEST[$fldName] == trim($val)){
            $_SESSION[$fldName] = $val;
            print "<option value=\"$val\" selected>$label</option>\n";
        }
        elseif(isset($_SESSION[$fldName]) && $_SESSION[$fldName] == trim($val)){
            print "<option value=\"$val\" selected>$label</option>\n";
        }
        else{
            print "<option value=\"$val\">$label</option>\n";
        }
    }
}

?>

<html>
<head>
<title>Buy Button Wizard</title>
<script language="JavaScript">
if(eval(parent.menu)) {
	var fileName = parent.menu.location.pathname.substring(parent.menu.location.pathname.lastIndexOf('/')+1);
	if(fileName != "buy.button.menu.html"){
		parent.menu.location = 'menus/buy.button.menu.html';
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

<?php if(isset($_REQUEST['wysiwyg']) && $_REQUEST['wysiwyg'] == "true"):?>
var formNum = 1;
<?php else:?>
var formNum = 0;
<?php endif;?>

function submitPage(form){
	form.method = "POST";
    form.submit();
}
function highlightmetasearch() {
	document.forms[formNum].elements['htmlcode'].select();
	document.forms[formNum].elements['htmlcode'].focus(); 
} 
function copyHtml() { 
	highlightmetasearch(); 
	textRange = document.forms[formNum].elements['htmlcode'].createTextRange(); 
	textRange.execCommand("RemoveFormat"); 
	textRange.execCommand("Copy"); 
	alert("This HTML code has been copied to your clipboard."); 
}

function addText(fld,txt,reset){
	if(reset){
		fld.value = "";
	}
	fld.value = fld.value + txt;
}
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
.normal
{
	font-family:Courier New,Helvetica, sans-serif;
}
.allBorder{
	border-style: solid;
	border-color: #CDCDCD;
	border-width: 1px 1px 1px 1px;
}
</style>
</head>
<body>
<div align="center">

<?php if($code):?>

	<?php if(isset($_REQUEST['wysiwyg']) && $_REQUEST['wysiwyg'] == "true"):?>
		<?php if(trim($itemName) != ""):?>
			</div>
			<div style=padding-left:10px;>
			<h4>WYSIWYG View:<br><hr size=1 noshade width=250 align=left></h4>
			<?=$code;?>
			</div>
		<?php endif;?>
	<?php endif;?>

	<form>
	<div align="center">
	<h4>Buy Button Code:</h4>
	<p><a href="javascript:copyHtml()">Copy this code</a> and paste it into your HTML product page where you want the button to be displayed.</p>
	<textarea class="normal" name="htmlcode" rows="25" cols="80" wrap="virtual"><?=$code?></textarea><br>
	<br><a href="javascript:copyHtml()">Copy HTML To Clipboard</a>
	</form>


<?php elseif(!empty($_REQUEST['defaults'])):?>

<h4>Buy Button Wizard - Default Values</h4>
<form name="qc" method="POST" action="buy.buttons.php">

    <table border="0" cellpadding=2 cellspacing="0" width="600">
        <tr>
            <td align=right>URL to cart.php:</td>
            <td><input type="text" name="scripturl" size="45" value="<?=$defaults['scripturl'];?>"></td>
        </tr>
        <tr>
            <td align=right>Buy button text:</td>
            <td><input type="text" name="buybuttontext" size="45" value="<?=$defaults['buybuttontext'];?>"></td>
        </tr>
        <tr>
            <td align=right>View Cart button text:</td>
            <td><input type="text" name="viewbuttontext" size="45" value="<?=$defaults['viewbuttontext'];?>"></td>
        </tr>
        <tr>
            <td align=right>Quantity text:</td>
            <td><input type="text" name="qtytext" size="45" value="<?=$defaults['qtytext'];?>"></td>
        </tr>  
        <tr>
            <td align=right>Add to cart image (optional):</td>
            <td>
				<input type="text" name="add_image" size="45" value="<?=$defaults['add_image'];?>">       
            </td>
        </tr>
        <tr>
            <td align=right>View cart image (optional):</td>
            <td>
				<input type="text" name="view_image" size="45" value="<?=$defaults['view_image'];?>">       
            </td>
        </tr>    
    </table>
	<p><input type="submit" name="save_defaults" value="Save Defaults"></p>
	</form>

<?php else:?>

<h4>Buy Button Wizard</h4>
<form name="qc" method="POST" action="buy.buttons.php">

    <table border="0" cellpadding=3 cellspacing="0" width="450" class="allBorder">
		<tr bgcolor="#F5F5F5">
			<td style="border-style:solid; border-color:#CDCDCD; border-width: 0px 0px 1px 0px;" colspan="2">
				<b>Product Information</b>
			</td>
		</tr>
		<tr><td colspan="2" style="line-height:5px;">&nbsp;</td></tr>
		<tr>
			<td>&nbsp;</td>
			<td class="comments">This is the name of the HTML page that this button is being used on:</td>
		</tr>
        <tr>
            <td nowrap align=right>Page name:</td>
            <td width="70%"><input type="text" name="page" size="25" value="<?=$fields['page'];?>"></td>
        </tr>
		<tr>
			<td>&nbsp;</td>
			<td class="comments">This is the the type of quantity box used:</td>
		</tr>
        <tr>
            <td align=right>Quantity type:</td>
            <td>
				<select name="inputType">
					<?php printOptions("inputType",Array("Select Box","Text Box","Checkbox","Order Button Only"));?>
				</select>            
            </td>
        </tr>
		<tr>
			<td>&nbsp;</td>
			<td>Product Details:</td>
		</tr>
        <tr>
            <td align=right>Category:</td>
            <td><input type="text" name="category" size="35" value="<?=$fields['category'];?>"></td>
        </tr>
        <tr>
            <td align=right>SKU:</td>
            <td><input type="text" name="sku" size="20" value="<?=$fields['sku'];?>"></td>
        </tr>
        <tr>
            <td align=right>Name:</td>
            <td><input type="text" name="name" size="35" value="<?=$fields['name'];?>"></td>
        </tr>
        <tr>
            <td align=right>Price:</td>
            <td>
				<input type="text" name="price" size="35" value="<?=$fields['price'];?>">&nbsp;
				<a href="#" onClick="return OpenQtyWindow('price')">Qty Price</a>
			</td>
        </tr>
        <tr>
            <td align=right>Size:</td>
            <td><input type="text" name="size" size="10" value="<?=$fields['size'];?>"></td>
        </tr>
        <tr>
            <td align=right>Weight:</td>
            <td><input type="text" name="weight" size="10" value="<?=$fields['weight'];?>"></td>
        </tr>
        <tr>
            <td align=right>Save to Database:</td>
            <td>
				<select name="save_to_db">
					<?php printOptions("save_to_db",Array("true","false"),$_SESSION['save_to_db']);?>
				</select>
            </td>
        </tr>
        <tr><td colspan="2" style="line-height:5px;">&nbsp;</td></tr>
	</table>
	
	<br/>
    <table border="0" cellpadding=3 cellspacing="0" width="450" class="allBorder">
		<tr bgcolor="#F5F5F5">
			<td style="border-style:solid; border-color:#CDCDCD; border-width: 0px 0px 1px 0px;" colspan="2">
				<b>Product Options</b>
			</td>
		</tr>
		<tr><td colspan="2" style="line-height:5px;">&nbsp;</td></tr>
		<tr>
			<td>&nbsp;</td>
			<td class="comments">This is the count of the different types of product options, like size and color.</td>
		</tr>
        <tr>
            <td align=right>Number of options:</td>
            <td width="70%">
				<select name="optionCount" onChange="submitPage(this.form);">
					<?php printOptions("optionCount",Array(0,1,2,3,4,5));?>
				</select>
            </td>
        </tr>
		<tr>
			<td>&nbsp;</td>
			<td class="comments">This is the count of the different values associated with each option,
				like small, medium, large, etc.</td>
		</tr>
        <tr>
            <td align=right>Number of values:</td>
            <td>
				<select name="optionRows" onChange="submitPage(this.form);">
					<?php printOptions("optionRows",Array(2,3,4,5,10,15,20,25));?>
				</select>
            </td>
        </tr>
        <tr><td colspan="2" style="line-height:5px;">&nbsp;</td></tr>
	</table>

	<?php if($optionCount > 0):?>


		<br/>
		<table border="0" cellpadding=3 cellspacing="0" width="450" class="allBorder">

			<tr bgcolor="#F5F5F5">
				<td style="border-style:solid; border-color:#CDCDCD; border-width: 0px 0px 1px 0px;" colspan="3">
					<b>Option Details</b>
				</td>
			</tr>
			<tr><td colspan="3" style="line-height:5px;">&nbsp;</td></tr>
			
			<?php error_reporting(E_PARSE | E_WARNING); ?>
			<?php for($i=0;$i<$optionCount;$i++):?>

			<?php if($i > 0):?>
				<tr><td colspan="3" style="line-height:5px;"><hr size="1" noshade width="90%"></td></tr>
				<tr><td colspan="3" style="line-height:5px;">&nbsp;</td></tr>
			<?php endif;?>

			<tr>
				<td colspan="3">
					<table border="0" cellpadding=3 cellspacing="0">

					<tr>
						<td align="right"><b>Option <?=($i + 1);?> Name:</b></td>
						<td colspan="2">
							<input type="text" name="option[<?=$i;?>][name]" size="25" value="<?=$fields['option'][$i]['name'];?>">
						</td>
					</tr>
					<tr>
						<td align="right"><b>Display as:</b></td>
						<td colspan="2">
							<select name="option[<?=$i;?>][format]">
								<?php printOptions("option][$i][format",Array("select box","radio buttons","text box"),$fields['option'][$i]['format']);?>
							</select>
						</td>
					</tr>
					<tr>
						<td align="right"><b>Option Type:</b></td>
						<td colspan="2">
							<select name="option[<?=$i;?>][type]">
								<?php printOptions("option][$i][type",Array("option","setup"),$fields['option'][$i]['type']);?>
							</select>
						</td>
					</tr>
					<tr>
						<td align="right"><b>Required:</b></td>
						<td colspan="2">
							<select name="option[<?=$i;?>][required]">
								<?php printOptions("option][$i][required",Array("false","true"),$fields['option'][$i]['required']);?>
							</select>
						</td>
					</tr>
					</table>
				</td>
			</tr>

			<tr><td colspan="3" style="line-height:5px;">&nbsp;</td></tr>
			<tr>
				<td><b>User Selectable Values</b></td>
				<td align="center"><b>Extended Price</b></td>
				<td align="center"><b>Extended Weight</b></td>
			</tr>
			
			<?php for($j=0;$j<$optionRows;$j++):?>
				<?php $textFld = "option[$i][$j][text]";?>
				<tr>
					<td align="left">
						<input type="text" name="option[<?=$i;?>][<?=$j;?>][value]" size="45" value="<?=$fields['option'][$i][$j]['value'];?>" onblur="addText(this.form.elements['<?=$textFld;?>'],this.value,true);">
					</td>
					<td align="center">
						<input type="text" name="option[<?=$i;?>][<?=$j;?>][price]" size="6" value="<?=$fields['option'][$i][$j]['price'];?>" onblur="if(this.value != '' && parseInt(this.value) > 0){addText(this.form.elements['<?=$textFld;?>'],' (' + this.value + ')',false)};">
					</td>
					<td align="center">
						<input type="text" name="option[<?=$i;?>][<?=$j;?>][weight]" size="6" value="<?=$fields['option'][$i][$j]['weight'];?>">
						<input type="hidden" name="option[<?=$i;?>][<?=$j;?>][text]" size="30" value="<?=$fields['option'][$i][$j]['text'];?>">
					</td>
				</tr>
			<?php endfor;?>

			<tr><td colspan=3>&nbsp;</td></tr>

			<?php endfor;?>
			<?php error_reporting(E_ALL); ?>

		</table>
	<?php endif;?>

	<p>
		<input type="submit" name="make_code" value="Make Button Code">
		<input type="button" name="clear" value="Clear" onClick="javascript:location='buy.buttons.php?clear=true';">&nbsp;
	</p>
	
	<p>
	<?php if(isset($_REQUEST['viewcart']) && $_REQUEST['viewcart'] == "true"):?>
		<input type="checkbox" name="viewcart" value="true" checked> Show View Cart Button
	<?php else:?>
		<input type="checkbox" name="viewcart" value="true"> Show View Cart Button
	<?php endif;?>
	&nbsp; &nbsp;
	<?php if(isset($_REQUEST['wysiwyg']) && $_REQUEST['wysiwyg'] == "true"):?>
		<input type="checkbox" name="wysiwyg" value="true" checked> Show WYSIWYG Example
	<?php else:?>
		<input type="checkbox" name="wysiwyg" value="true"> Show WYSIWYG Example
	<?php endif;?>
	</p>

</form>

<?php endif;?>

<p>&nbsp;</p>
</div>
</body>
</html>






