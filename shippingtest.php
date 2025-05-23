<?php

$debug = false;

// initialize the program and read the config

include_once("include/initialize.inc");

$init = new Initialize();

// values for calculating shipping for the chests
			$_SESSION['current_SKU'] = "CH3DCD";
			$_SESSION['current_qty'] = 1;
			$_SESSION['current_weight'] = 28;
			$_SESSION['current_size'] = "";
			$_SESSION['current_length'] = 19;
			$_SESSION['current_width'] = 29;
			$_SESSION['current_height'] = 21;
			$_SESSION['current_declaredValue'] = 425;
			$_SESSION['current_optionWeight'] = 0;

// check if login is required

//if($_CF['login']['require_login']){
//
//	$login = $_Registry->LoadClass('login');
//
//	$login->checkLogin();
//
//}



//$cart = $_Registry->LoadClass('cart');

//$cart->LoadCart();



//$shippingPlugins = array();

// check the shipping table to see if Shipping Plugin Name is added and

// "Use Shipping Plugin" is on. If so, try to dynamically load it here

//$shipExt = $_DB->getRecord("SELECT use_shipping_plugin,shipping_plugin_name FROM shipping");
//
//if($shipExt['use_shipping_plugin'] == 'true' && trim($shipExt['shipping_plugin_name']) != ""){
//
//	$fileNames = explode(',',trim($shipExt['shipping_plugin_name']));
//
//	foreach($fileNames as $i=>$name){
//
//		$flds = explode('.',$name);
//
//		$shippingPlugins[] = $flds[0];	
//
//	}
//
//}



$haveRates = false;

$noRates = false;

//if(!empty($_REQUEST['quote']) && count($_CART) > 0){
if(!empty($_REQUEST['quote']) ){

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

	

//	if(count($shippingPlugins) > 0){

//		$fileNames = explode(',',trim($shipExt['shipping_plugin_name']));
//
//		$plugin = $fileNames[0];
		$plugin = "ups.xml_chests.inc";
		
//		include_once("include/shipping/ups/ups.xml_chests.inc");
//
//		if(!empty($_REQUEST['preferred_shipper'])){

//			$_SESSION['preferred_shipper'] = trim($_REQUEST['preferred_shipper']);
			$_SESSION['preferred_shipper'] = "ups";
			
			$_SESSION['current_SKU'] = "CH3DCD";

//		}

//		if(count($fileNames) > 1){
//
//			foreach($fileNames as $i=>$name){
//
//				if(!empty($_SESSION['preferred_shipper']) && substr($name,0,strlen($_SESSION['preferred_shipper'])) == $_SESSION['preferred_shipper']){
//
//					$plugin = trim($name);
//
//					break;
//
//				}
//
//			}
//
//		}

		if($debug){

			$_Common->debugPrint($plugin,"Selected Shipping Plug-in");

		}

		if($_Registry->file_exists_incpath($plugin)){

			$_Registry->registeredClasses['shipping.rates.inc'] = $plugin;

		}

//	}



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

error_reporting(E_PARSE|E_WARNING);

?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">

<html>

	<head>

		<title>Shipping Quote</title>

		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">

		<meta name="vs_targetSchema" content="http://schemas.microsoft.com/intellisense/ie5">

		<link rel="stylesheet" type="text/css" href="styles/cart.styles.css" />
		<script language="JavaScript" src="../mobile/jquery-mobile-1.1.0/jquery-1.7.2.min.js" type="text/javascript"></script>


	</head>

<body>



<div align="center">



<?php //if(count($_CART) > 0):?>




	<script type="text/javascript" src="javascripts/checkoutchests.js"></script>



	<style>

	td{

		vertical-align: middle;

		align: left;

	}

	.topAlign{

		vertical-align: top;

	}

	</style>



	<form name="quote" id="quote" method="get" action="shippingtest.php">



	<table border="0" cellspacing="0" cellpadding="2" width="500" align="center" ID="Table1">

  <tr>

			<td colspan="2" align="left">

				<h4>Get Shipping Quote</h4>
            <!--set country to to US-->
            <input type="hidden" name="shipaddress_country" id="shipaddress_country" value="US">
            <input type="hidden" size="1" name="billaddress_city" value="" id="Text6">
            <input type="hidden" size="1" name="shipaddress_city" value="" id="Text16">
            
            <!--set shipper to UPS-->
            <input name="preferred_shipper" type="hidden" id="preferred_shipper" value="ups">
            <!--set state to CA it does not matter-->
            <input name="shipaddress_state" type="hidden" id="shipaddress_state" value="CA">
		</tr>
		<tr>

			<td width="100%" align="right">Enter zip code: </td>

			<td width="64%" align="left">

				<input name="shipaddress_postalcode"  id="shipaddress_postalcode" type="text" onBlur="return isZip(this.value)" value="<?=$_SESSION['shipaddress_postalcode'];?>" size="25" maxlength="10">			</td>
		</tr>

		
		<tr>

			<td colspan="2" width="100%" align="center"><br />
				<?php $_SESSION['billaddress_city'] = "default"; ?>


				<input name="quote" class="buttons" type="submit" value="Get Quote" onClick="return isZip($('#shipaddress_postalcode').val())">			</td>

		</tr>

		<tr><td width="100%" colspan="2" align="center">&nbsp;</td>

	  </tr>

	</table>



<?php if($haveRates):?>

		<br clear="all" />

		<table border="0" cellspacing="0" cellpadding="2" width="600" align="center" ID="Table1">

			<tr>

				<td align="center">

					<h4>Shipping Rates for 2-drawer CD Chest<br>
(w:29&quot;, l:19&quot;, h:21, weight 28lbs, ins. 425, ship from: 19804)</h4>

				</td>

			</tr>

			<?php foreach($shippingRateList as $carrier=>$rate):?>

			<tr>

				<td align="center" style="padding-left:20px;"><li><?=$carrier;?> - <?=$_Common->format_price($rate,true);?></td>

			</tr>

			<?php endforeach;?>

		</table>	

	<?php elseif($noRates):?>

		<p><b>No shipping rates were returned for the values you entered.</b></p>

	<?php endif;?>

		

	</form>



<!--	<script type="text/javascript">

		selectBoxes(document.forms['quote'],true);

	</script>-->



<?php //else:?>



	<!--<p><br /><br /><b>Your cart is currently empty. Cannot supply shipping quote at this time.</b></p>-->



<?php //endif;?>



</div>

<p>&nbsp;</p>

</body>

</html>

	