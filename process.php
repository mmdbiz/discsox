<?php
// initialize the program and read the config
include_once("include/initialize.inc");
$init = new Initialize();

// variables from the process.inc that we will need in this page
$vars = array();

// check if login is required
if($_CF['login']['require_login']){
	$login = $_Registry->LoadClass('login');
	$login->checkLogin();
}

$cart = $_Registry->LoadClass('cart');

$vars['showSubmitPage'] = false;
$vars['showDeclinePage'] = false;

// login ok, send in the order
$process = $_Registry->LoadClass('process');
$vars = get_object_vars($process);

// Set the page variables
$vars['pageTitle'] = "Thank You for your order.";
if(!$vars['paymentResultOk']){
	if($vars['showSubmitPage']){
		$vars['pageTitle'] = "Processing your order. One Moment Please...";
	}
	elseif($vars['showDeclinePage']){
		$vars['pageTitle'] = "Payment Processing Error";
	}
}
else{
	if(!empty($_REQUEST['payment_method']) && $_REQUEST['payment_method'] == "quote.html"){
		$vars['pageTitle'] = "Thank You for submitting your quote.";
	}
}

// get the receipt.html template for display
//$receiptPage = $_Template->Open("templates/receipt.html",$vars,true);
$receiptPage = $_Template->Open("templates/receipt.php",$vars,true);
?>
<?=$receiptPage;?>

