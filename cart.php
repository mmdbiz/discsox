<?php
// initialize the program and read the config
include_once("include/initialize.inc");
$init = new Initialize();

// check if login is required
if($_CF['login']['require_login']){
	$login = $_Registry->LoadClass('login');
	$login->checkLogin();
}

$vars = array();
// login ok, get cart page variables
$cart = $_Registry->LoadClass('cart');
$cart->initialize();

$vars['relatedItems'] = $cart->relatedItems;

$cart->createGoogleCheckoutButton();
$vars['googleButton'] = $cart->googleCheckoutButton;

// get the cart.html template for display
//$cartPage = $_Template->Open("templates/cart.html",$vars,true);
$cartPage = $_Template->Open("templates/cart.php",$vars,true);
?>
<?=$cartPage;?>










