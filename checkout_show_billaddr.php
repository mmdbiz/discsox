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

// load the cart
$cart = $_Registry->LoadClass('cart');

// get checkout page variables
$checkout = $_Registry->LoadClass('checkout');
$vars = get_object_vars($checkout);

$label = "Billing";
if(!$_CF['cart']['show_prices']){
	$label = "Contact";
}

// Set the page variables
$vars['pageTitle'] = "Checkout: $label Address Information";
if($_CF['shipping']['require_shipping']){
	$vars['pageTitle'] = "Checkout: $label/Shipping Address Information";
}

// get the checkout.html template for display
$checkoutPage = $_Template->Open("templates/checkout_show_billaddr.html",$vars,true);
?>
<?=$checkoutPage;?>










