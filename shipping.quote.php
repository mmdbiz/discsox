<?php
//header("Cache-Control: no-cache, must-revalidate"); // HTTP/1.1
//header("Expires: Sat, 26 Jul 1997 05:00:00 GMT"); // Date in the past
$debug = false;

// initialize the program and read the config
include_once("include/initialize.inc");
$init = new Initialize();

// check if login is required
if($_CF['login']['require_login']){
	$login = $_Registry->LoadClass('login');
	$login->checkLogin();
}
$cart = $_Registry->LoadClass('cart');
$cart->LoadCart();

$shippingPlugins = array();
// check the shipping table to see if Shipping Plugin Name is added and
// "Use Shipping Plugin" is on. If so, try to dynamically load it here
$shipExt = $_DB->getRecord("SELECT use_shipping_plugin,shipping_plugin_name FROM shipping");
if($shipExt['use_shipping_plugin'] == 'true' && trim($shipExt['shipping_plugin_name']) != ""){
	$fileNames = explode(',',trim($shipExt['shipping_plugin_name']));
	foreach($fileNames as $i=>$name){
		$flds = explode('.',$name);
		$shippingPlugins[] = $flds[0];	
	}
}

$haveRates = false;
$noRates = false;
if(!empty($_REQUEST['quote']) && count($_CART) > 0){
	foreach($_REQUEST as $key=>$value){
		if(substr($key,0,11) == "shipaddress"){
			if(is_array($value)){
				$_SESSION[$key] = $value;
			}
			else{
				$_SESSION[$key] = trim($value);
			}
		}
	}
	
	if(count($shippingPlugins) > 0){
		$fileNames = explode(',',trim($shipExt['shipping_plugin_name']));
		$plugin = $fileNames[0];
		if(!empty($_REQUEST['preferred_shipper'])){
			$_SESSION['preferred_shipper'] = trim($_REQUEST['preferred_shipper']);
		}
		if(count($fileNames) > 1){
			foreach($fileNames as $i=>$name){
				if(!empty($_SESSION['preferred_shipper']) && substr($name,0,strlen($_SESSION['preferred_shipper'])) == $_SESSION['preferred_shipper']){
					$plugin = trim($name);
					break;
				}
			}
		}
		if($debug){
			$_Common->debugPrint($plugin,"Selected Shipping Plug-in");
		}
		if($_Registry->file_exists_incpath($plugin)){
			$_Registry->registeredClasses['shipping.rates.inc'] = $plugin;
		}
	}

	$shippers = $_Registry->LoadClass('Shipping.Rates');
	
	$shippers->Country = $_REQUEST['shipaddress_country'];
	if($_REQUEST['shipaddress_country'] == "US" || $_REQUEST['shipaddress_country'] == "CA"){
		$shippers->State = $_REQUEST['shipaddress_state'];
	}
	if(!empty($_REQUEST['shipaddress_postalcode'])){
		$shippers->Zip = $_REQUEST['shipaddress_postalcode'];
	}
	
	@list($shippingRateList,$shippingRate) = $shippers->GetShippingRateList();
	
	if(count($shippingRateList) > 0){
		$haveRates = true;	
	}
	else{
		$noRates = true;	
	}
}
//error_reporting(E_PARSE|E_WARNING);
error_reporting(E_PARSE);
?><!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">

<html>
	<head>
		<title>Shipping Quote</title>        <meta content="width=device-width, initial-scale=1, maximum-scale=1.0, user-scalable=no" name="viewport">
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
		<meta name="vs_targetSchema" content="http://schemas.microsoft.com/intellisense/ie5">
		<link rel="stylesheet" type="text/css" href="styles/cart.styles.css" />
	</head>
<body>
<div align="center">

<?php if(count($_CART) > 0):?>

	<script type="text/javascript">
		var requiredFields = new Array('shipaddress_country','shipaddress_state','shipaddress_postalcode');
		var selectedShipCountry = "<?=$_SESSION['shipaddress_country'];?>";
		var selectedShipState = "<?=$_SESSION['shipaddress_state'];?>";
		var selectedBillState = "";
		var counties = new Array();
		function testFields(form){
			var country = form.elements['shipaddress_country'].options[form.elements['shipaddress_country'].selectedIndex].value;
			if(country != 'US' && country != 'CA'){
				requiredFields.pop();
			}
			return testRequiredFields(form);
		}
		
		var hideareacode = true;
		<?php if(isset($_CF['basics']['always_display_area_code']) && $_CF['basics']['always_display_area_code']):?>
			hideareacode = false;
		<?php endif;?>		
	</script>
	<script type="text/javascript" src="javascripts/checkout.js"></script>
	<style>
	td{
		vertical-align: middle;
		align: left;
	}
	.topAlign{
		vertical-align: top;
	}
	</style>

	<form name="quote" id="quote" method="get" action="shipping.quote.php">
	<table border="0" cellspacing="0" cellpadding="2"  class="m_shipEstimate"  align="center" ID="Table1">
  <tr>
			<td colspan="2" align="left">
				<h4>Get Shipping Quote</h4>
				<p>Please complete the following information for available shipping methods and rates.<br /><br /></p>			</td>
		</tr>
		<tr>
			<td width="100%" align="right">Select Ship to Country: </td>
			<td width="64%" align="left">
    			<select NAME="shipaddress_country" onChange="checkCountry(this.form,this.name);" ID="Select1">
                            <option value="US" selected>United States </option>
                            <option value="AR">Argentina </option>
                            <option value="AU">Australia </option>
                            <option value="AT">Austria </option>
                            <option value="BE">Belgium </option>
                            <option value="BM">Bermuda </option>
                            <option value="BR">Brazil </option>
                            <option value="BG">Bulgaria </option>
                            <option value="CA">Canada </option>
                            <option value="CN">China, Peoples Republic of </option>
                            <option value="CR">Costa Rica </option>
    			  			<option value="HR">Croatia </option>
                            <option value="CZ">Czech Republic </option>
                            <option value="DK">Denmark </option>
                            <option value="DO">Dominican Republic </option>
                            <option value="EN">England </option>
                            <option value="EE">Estonia </option>
                            <option value="FI">Finland </option>
                            <option value="FR">France </option>
                            <option value="DE">Germany </option>
                            <option value="GR">Greece </option>
                            <option value="HO">Holland </option>
                            <option value="HK">Hong Kong </option>
                            <option value="HU">Hungary </option>
                            <option value="IS">Iceland </option>
                            <option value="IN">India </option>
                            <option value="ID">Indonesia </option>
                            <option value="IE">Ireland </option>
                            <option value="IL">Israel </option>
                            <option value="IT">Italy </option>
                            <option value="JM">Jamaica </option>
                            <option value="JP">Japan </option>
                            <option value="LI">Liechtenstein </option>
                            <option value="LU">Luxembourg </option>
                            <option value="MY">Malaysia </option>
                            <option value="MT">Malta </option>
                            <option value="MX">Mexico </option>
                            <option value="MC">Monaco </option>
                            <option value="NL">Netherlands </option>
                            <option value="NZ">New Zealand </option>
                            <option value="NB">Northern Ireland </option>
                            <option value="NO">Norway </option>
                            <option value="PH">Philippines </option>
                            <option value="PL">Poland </option>
                            <option value="PT">Portugal </option>
                            <!--<option value="PR">Puerto Rico </option>-->
                            <option value="IE">Republic of Ireland </option>
                            <option value="RO">Romania </option>
                            <option value="RU">Russia </option>
                            <option value="SF">Scotland </option>
                            <option value="RS">Serbia </option>
                            <option value="SG">Singapore </option>
    			  			<option value="ZA">South Africa </option>
                            <option value="ES">Spain </option>
                            <option value="SE">Sweden </option>
                            <option value="CH">Switzerland </option>
                            <option value="TW">Taiwan </option>
                            <option value="TH">Thailand </option>
                            <option value="TR">Turkey </option>
                            <option value="AE">United Arab Emirates </option>
                            <option value="GB">United Kingdom</option>
                            <option value="VU">Vanuatu </option>
                            <option value="WL">Wales </option>
    			</select>
              </td>
		</tr>
		<tr>
			<td width="100%" align="right">
            <input type="hidden" size="1" name="billaddress_city" maxlength="20" value="" id="Text6">
            <input type="hidden" size="1" name="shipaddress_city" maxlength="20" value="" id="Text16">
	      Select State/Province: </td>
			<td width="64%" align="left">            
				<select NAME="shipaddress_state" onChange="addCounties(this.form,this.name);" ID="Select2">
					<option value="INVALID">Select State</option>
				</select></td>
		</tr>
		<tr>
			<td width="100%" align="right">Enter Postal Code if applicable: </td>
			<td width="64%" align="left">
				<input type="text" size="25" name="shipaddress_postalcode" maxlength="10" value="<?=$_SESSION['shipaddress_postalcode'];?>">			</td>
		</tr>
		<?php if(count($shippingPlugins) > 1):?>
		<tr>
          <td colspan="2" align="center" valign="middle"><img src="../_images/logos/usps_logo_smallx.png" alt="USPS" width="66" height="60" hspace="30" border="0" align="right"><img src="../_images/logos/apkcar_l.gif" alt="UPS" width="83" height="48" hspace="25" border="0" align="left">
            <p>&nbsp;</p>
            <p>&nbsp;</p>
          <p><strong class="standout">&nbsp;Select Shipping Method!&nbsp;</strong></p>          </td>
	  </tr>
		<tr>
			<td width="100%" align="right"> Shipping Method: </td>
			<td width="64%" align="left">
				<select name="preferred_shipper">
				<?php foreach($shippingPlugins as $k=>$shipper):?>                
                    <?php 
					      if (strtoupper($shipper) == "UPS") { 
					         $shipperDisplay = "FedEx";
					         $shipperDisplay = "UPS"; // remove is we do the FedEx thing - marcello
						  } 
                          if (strtoupper($shipper) == "USPS") { 
					         $shipperDisplay = "USPS";
						  } 
					?>
					<?php if($_SESSION['preferred_shipper'] == $shipper):?>
						<option value="<?=$shipper;?>" selected><?=($shipperDisplay);?></option>
					<?php else:?>
						<option value="<?=$shipper;?>"><?=($shipperDisplay);?></option>
					<?php endif;?>
				<?php endforeach;?>
				</select>			</td>
		</tr>
		<?php endif;?>
		<tr>
		  <td width="100%" height="25" colspan="2" align="center" valign="top"><strong class="standout">&nbsp;For international &amp; APO orders choose USPS, not UPS!&nbsp;</strong></td>
	  </tr>
		<tr>
			<td colspan="2" width="100%" align="center"><br />
				<?php $_SESSION['billaddress_city'] = "default";?>
				<input name="quote" class="buttons" type="submit" value="Get Quote" onClick="return testFields(this.form);">			</td>
		</tr>
		<tr><td width="100%" colspan="2" align="center">&nbsp;</td>
	  </tr>
	</table>

<?php if($haveRates):?>
		<br clear="all" />
		<table border="0" align="center" cellpadding="2" cellspacing="0" class="m_shipEstimate" ID="Table1">
			<tr>
				<td align="left">
					<h4>Available Shipping Rates at Checkout</h4>
				</td>
			</tr>
			<?php foreach($shippingRateList as $carrier=>$rate):?>
			<tr>
				<td align="left" style="padding-left:20px;"><li><?=$carrier;?> - <?=$_Common->format_price($rate,true);?></td>
			</tr>
			<?php endforeach;?>
		</table>	
	<?php elseif($noRates):?>
	  <p><b>No shipping rates were returned for the values you entered.</b></p>
	<?php endif;?>		
	</form>

	<script type="text/javascript">
		selectBoxes(document.forms['quote'],true);
	</script>
<?php else:?>

	<p><br /><br /><b>Your cart is currently empty. Cannot supply shipping quote at this time.</b></p>
<?php endif;?>

</div>
<p>&nbsp;</p>
</body>
</html>	