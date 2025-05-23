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

$cfg = array();
$originPostalCode = null;
if(!empty($_GET['shipper'])){
	$plugin = strtolower($_GET['shipper']);
	$cfg = @parse_ini_file("../include/shipping/$plugin/$plugin.config.php",true);
	if($plugin == "usps"){
		$originPostalCode = $cfg['settings']['usps_origin_postal_code'];
	}
	elseif($plugin == "ups"){
		$originPostalCode = $cfg['settings']['originPostalCode'];
	}	
}


$upsPackages = array('02,Package'		 => 'Your Packaging',
					 '01,UPS letter'	 => 'UPS letter',
					 '03,UPS Tube'		 => 'UPS Tube',
					 '04,UPS Pak'		 => 'UPS Pak',
					 '21,UPS Express Box'=> 'UPS Express Box',
					 '24,UPS 25KG Box'	 => 'UPS 25KG Box');

$uspsPackages = array('None'				=> 'Your Packaging',
					  'Flat Rate Box'		=> 'Flat Rate Box',
					  'Flat Rate Envelope'	=> 'Flat Rate Envelope');
?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<title>Shipping Definition</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
<meta name=vs_targetSchema content="http://schemas.microsoft.com/intellisense/ie5">
<script type="text/javascript" >
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
<script type="text/javascript">

	var fldid = "<?=$_GET['fldid'];?>";
	var defaultZip = "<?=$originPostalCode;?>";

	// -------------------------------------------------------------------
	function loadValues(){
		
		var form = document.forms[0];
		
		if(opener.document.forms[0].elements[fldid].value != ''){
		
			var parts = opener.document.forms[0].elements[fldid].value.split(':');
			
			if(isNaN(parts[0])){
				parts[0] = parseFloat(0).toFixed(2);
			}
			else{
				parts[0] = parseFloat(parts[0]).toFixed(2);
			}
			
			var weight = parts[0].split('.');
			
			form.pounds.value = parseInt(weight[0]);
		
			// if ounces are higher than 16, this must be a fractional amount of a lb.		
			if(parseInt(weight[1]) > 15){
				form.ounces.value = parseInt((weight[1] / 100) * 16);
			}
			else{
				form.ounces.value = parseInt(weight[1]);
			}
			
			var dimensions = parts[1].split('x');
			
			form.length.value = dimensions[0];
			form.width.value = dimensions[1];
			form.height.value = dimensions[2];
			
			if(parts.length > 2){
				var type = parts[2];
				if(type == 'S'){
					form.type.options[1].selected = true;
				}
				if(type.indexOf('-') != -1){
					var flds = type.split('-');
					form.perbox.value = flds[1];
				}
			} 
			if(parts.length > 3){
				var packaging = parts[3];
				for(i=0;i<form.packaging.length;i++){
					if(form.packaging.options[i].value == packaging){
						form.packaging.options[i].selected = true;
						break;
					}
				}
			}
			if(parts.length > 4){
				form.originzip.value = parts[4];
			}

			if((form.originzip.value == '' || form.originzip.value == 'undefined') && defaultZip != ''){
				form.originzip.value = defaultZip;
			}
		}
	}
    
	// -------------------------------------------------------------------
	function sendFeldText(form){
	
		var data = new Array();
		
		var pounds = parseInt(form.pounds.value);

		// if ounces are higher than 16, this must be a fractional amount of a lb.		
		if(parseInt(form.ounces.value) > 15){
			var lbsfromoz = parseInt(form.ounces.value / 16);
			pounds =  pounds + lbsfromoz;
			form.ounces.value = parseInt(form.ounces.value) - lbsfromoz * 16;
		}
		var ounces = parseInt(form.ounces.value);

		data[0] = parseFloat(pounds + (ounces / 16)).toFixed(2);
		
		var length = parseFloat(form.length.value);
		var width = parseFloat(form.width.value);
		var height = parseFloat(form.height.value);
		
	    data[1] = '';
		if(length >= 0 && width >= 0 && height >= 0){
			data[1] = length + 'x' + width + 'x' + height;
		}
		
		var typeIndex = form.type.selectedIndex;
		data[2] = form.type.options[typeIndex].value;

		if(parseInt(form.perbox.value) > 0){
			data[2] = data[2] + '-' + parseInt(form.perbox.value);
		}

		var packIndex = form.packaging.selectedIndex;
		data[3] = form.packaging.options[packIndex].value;

		data[4] = form.originzip.value;

		var strValue = data.join(':');

		//alert(strValue);

		opener.document.forms[0].elements[fldid].value = strValue;
		window.close();
		return false;
	}

	function checkBox(val){
	
		if(document.all){
			whichEl = document.all['boxmsg'];
		}
		else{
			whichEl = document.getElementById('boxmsg');
		}
		if(val == 'Flat Rate Box'){
			window.resizeTo(600, 725);
			whichEl.style.display = "";
		}
		else{
			whichEl.style.display = "none";
			window.resizeTo(600, 525);
		}
	
	}
alert(scr_h);

</script>
</head>
<body onLoad="loadValues();" style="background-color: white; color: black;margin-left:5;margin-top:5;margin-right:5;margin-bottom:5;">
<div align='center'>

<form name="shipform">
<h4>Define <?=$_GET['shipper'];?> Shipping Package</h4>
<table border="1" cellspacing="1" cellpadding="4" width="500">
	<tr>
		<td valign="top" align="right">Shipping Weight: </td>
		<td align="left">
			<input type="text" size="5" name="pounds" value="0"> pounds  &nbsp;
			<input type="text" size="5" name="ounces" value="0" ID="Text7"> ounces
		</td>
	</tr>
	<tr>
		<td valign="top" align="right">Length: </td>
		<td align="left"><input type="text" size="5" name="length" value="0" ID="Text2"> inches</td>
	</tr>
	<tr>
		<td valign="top" align="right">Width: </td>
		<td align="left"><input type="text" size="5" name="width" value="0" ID="Text3"> inches</td>
	</tr>
	<tr>
		<td valign="top" align="right">Height: </td>
		<td align="left"><input type="text" size="5" name="height" value="0" ID="Text4"> inches</td>
	</tr>
	<tr>
		<td valign="top" align="right">Package Type: <br /><br />
			<select name="type" ID="Select2">
				<option value="M">Multiple</option>
				<option value="S">Single</option>
			</select>
		</td>
		<td align="left">
			Single - Ship each item in its' own box (if the user gets 3 of this item, there will be 3 boxes) <br /><br />
			Multiple - Ship mutiple quantities of the item together in one box.
		</td>
	</tr>
	<tr>
		<td valign="top" align="right">If Multiple: <br /><br />
			Quantity: <input type="text" size="5" name="perbox" ID="Text5">
		</td>
		<td align="left">
			You can also specify how many of an item go in each box.
			If you leave this blank, and you are using the Multiple type above,
			then all of that item will be in the same box irregardless of the quantity.
		</td>
	</tr>
	<tr>
		<td valign="top" align="right">Packaging: </td>
		<td align="left">
			<select name="packaging" ID="Select1" onChange="checkBox(this.options[this.selectedIndex].value);">
			
				<?php if(!empty($_GET['shipper']) && $_GET['shipper'] == 'UPS'):?>
			
					<?php foreach($upsPackages as $val=>$txt):?>
					<option value="<?=$val;?>"><?=$txt;?></option>
					<?php endforeach;?>
				
				<?php elseif(!empty($_GET['shipper']) && $_GET['shipper'] == 'USPS'):?>
				
					<?php foreach($uspsPackages as $val=>$txt):?>
					<option value="<?=$val;?>"><?=$txt;?></option>
					<?php endforeach;?>
					
				<?php endif;?>
				
			</select>
		</td>
	</tr>
	
	<tr id="boxmsg" style="display:none;">
		<td valign="top" align="right" nowrap><font color=red><b>USPS NOTE:</b></font> </td>
		<td align="left">
			Flat Rate Boxes are available in two sizes:<br /><br />
			Box One: 11-7/8 x 3-3/8 x 13-5/8 inches (ideal for garments, board games, books and other relatively thin items).<br /><br />
			Box Two: 11 x 8-1/2 x 5-1/2 inches (perfect for shoes, model cars, and taller items).<br /><br />
			Rate is $8.95, regardless of weight or destination for items mailed in the Priority Mail Flat-Rate Boxes provided by the Postal Service.<br /><br />
			So, it may be cheaper to use your own box.
		</td>
	</tr>
	
	
	<tr>
		<td valign="top" align="right" nowrap>Ship From Zip Code: </td>
		<td align="left">
			<input type="text" size="10" name="originzip" ID="Text6">
		</td>
	</tr>

</table>
<p>
	<input class='buttons' type='reset' value='Clear' ID="Reset1" NAME="Reset1"/>
	<input class='buttons' type='button' value='Close' onclick="window.close()" ID="Button2" NAME="Button2"/>
  	<input class='buttons' type='button' value='Save' onclick="sendFeldText(this.form)" ID="Button3" NAME="Button3"/>
</p>
</form>
</div>
</body>
</html>
