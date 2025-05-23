<?php
// initialize the program and read the config
include_once("include/initialize.inc");
$init = new Initialize();

$login = $_Registry->LoadClass('login');

if(!empty($_REQUEST['user']) || !empty($_REQUEST['forgot']) || !empty($_REQUEST['logout'])){
	$login->checkLogin();
}

if(!empty($_SERVER['HTTP_REFERER']) && $_SERVER['HTTP_REFERER'] != "login.php"){
	//unset($_SESSION['login_message']);
}

// load the cart for the minicart display in the store template 
$cart = $_Registry->LoadClass('cart'); 
$cart->initialize();

$vars = array();

$vars['message'] = "Please Login...";
if(isset($_SESSION['login_message'])){
	$vars['message'] = $_SESSION['login_message'];
}

// Set the page title
$vars['pageTitle'] = "Please Login...";

// get the login.html template for display
$loginPage = $_Template->Open("templates/login.php",$vars,true);
?>
<?=$loginPage;?>


