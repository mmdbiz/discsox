<?php
// initialize the program and read the config
include_once("include/initialize.inc");
$init = new Initialize();

// variables from the checkout.inc that we will need in this page
$vars = array();

// check if login is required
if($_CF['login']['require_login']){
	$login = $_Registry->LoadClass('login');
	$login->checkLogin();
}

$cart = $_Registry->LoadClass('cart');

// login ok, get checkout shipping page variables
$checkoutShipping = $_Registry->LoadClass('Checkout.Shipping');
$vars = $checkoutShipping->vars;

if(empty($_SESSION['orderSubmitted'])){
	$_SESSION['orderSubmitted'] = '0';	
}

$vars['payment_method'] = null;
if(!empty($_REQUEST['payment_method'])){
	$vars['payment_method'] = $_REQUEST['payment_method'];
}

// Set the page title variable
if($_CF['shipping']['require_shipping']){
	$vars['pageTitle'] = "Checkout: Review totals and select your shipping preferences below.";
}
if(!$_CF['cart']['show_prices'] && $vars['payment_method'] == "quote.html"){
	$vars['pageTitle'] = "Checkout: Review Quote";
}
elseif(!$_CF['cart']['show_prices'] && $vars['payment_method'] != "quote.html"){
	$vars['pageTitle'] = "Checkout Review";
}
elseif($vars['paymentPage'] != "" && $_CF['shipping']['require_shipping']){
	$vars['pageTitle'] = "Checkout -> Review Order -> Payment Info";
}
elseif($vars['paymentPage'] != "" && !$_CF['shipping']['require_shipping']){
	$vars['pageTitle'] = "Checkout: Review totals and enter your payment information below.";
}


// get the shipping.html template for display
//$shippingPage = $_Template->Open("templates/shipping.html",$vars,true);
$shippingPage = $_Template->Open("templates/shipping.php",$vars,true);

?>
<?=$shippingPage;?>
