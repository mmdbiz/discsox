<?php
// initialize the program and read the config
include_once("include/initialize.inc");
$init = new Initialize();

// get the login class and see if required
$login = $_Registry->LoadClass('login');
$login->checkLogin();

// load the cart so we can display mini-cart
$cart = $_Registry->LoadClass('cart');

// get the about.html page
$vars = array();
$indexPage = $_Template->Open("templates/about.html",$vars,true);
?>
<?=$indexPage;?>
