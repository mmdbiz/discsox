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

// reload the cart
$cart = $_Registry->LoadClass('cart');
$cart->LoadCart();

if(!empty($_SESSION['totals'])){
	$_Totals = $_SESSION['totals'];
}

$paypal = $_Registry->loadClass("paypal");

$paypal->processResult();

if(isset($paypal->results['decline_message'])){
	// get the receipt.html template for display
	$vars['decline_message'] = $paypal->results['decline_message'];
	$vars['pageTitle'] = $paypal->results['decline_message'];
	$page = $_Template->Open("templates/declined.html",$vars,true);
}
else{

	$paypal->downloadLinks = array();
	$paypal->order_number = $_SESSION['next_order_number'];
	if(isset($_SESSION['invoice_number'])){
		$paypal->order_number = $_SESSION['invoice_number'];
	}
	$paypal->customer_number = $_SESSION['next_customer_number'];
	$paypal->order_date = $_SESSION['order_date'];
	$paypal->selectedCarrier = null;
	
	if(isset($_SESSION['shipping_method'])){
		$paypal->selectedCarrier = $_SESSION['shipping_method'];
	}

	if($_Registry->file_exists_incpath("inventory.inc")){
		$inv = $_Registry->LoadClass("inventory");
		$inv->UpdateInventory();
	}

	// Create any download links
	if($_Registry->file_exists_incpath('downloads.inc')){
		$downloads = $_Registry->LoadClass('downloads');
		$emailAdrs = null;
		if(!empty($_SESSION['billaddress_email'])){
			$emailAdrs = $_SESSION['billaddress_email'];
		}
		$paypal->downloadLinks = $downloads->makeDownloadLinks($paypal->order_number,$emailAdrs);
	}

	$paypal->sendEmails();

	foreach(get_object_vars($paypal) as $name=>$value){
		$vars[$name] = $value;
	}
	
	// get the receipt.html template for display
	$vars['pageTitle'] = "Thank You for your order.";
	$page = $_Template->Open("templates/receipt.php",$vars,true);

	// delete cart if required
	if($_CF['cart']['delete_cart_at_checkout']){
		$cart->deleteCart();
		if(!empty($_SESSION['cartTotals'])){
			unset($_SESSION['cartTotals']);
		}
		session_destroy();
	}
}
?>
<?=$page;?>






