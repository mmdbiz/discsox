<?php
//force this page to use https
if($_SERVER["HTTPS"] != "on")
{
    header("Location: https://" . $_SERVER["HTTP_HOST"] . $_SERVER["REQUEST_URI"]);
    exit();
}
// initialize the program and read the config
include_once("include/initialize.inc");
$init = new Initialize();

$_SESSION['logging_in_from'] = "myaccount.php";
$vars = array();
$vars['summary'] = array();
if(empty($_SESSION['cid']) && !isset($_REQUEST['login']) && !isset($_REQUEST['forgot'])){
	$vars['pageTitle'] = "Please Sign In...";
	$vars['message'] = "<strong>Please Sign In...</strong>";
	if(isset($_SESSION['login_message'])){
		$vars['message'] = $_SESSION['login_message'];
	}
	$page = $_Template->Open("templates/login.php",$vars,true);	
}
else{
	// Login is required
	$_CF['login']['require_login'] = true;
	$login = $_Registry->LoadClass('login');
	$login->checkLogin();

	// For Pull downs
	global $provinces;
	global $states;
	global $countries;
	include_once("include/countries.inc");
	include_once("include/provinces.inc");
	include_once("include/states.inc");


	//$account = $_Registry->LoadClass('customer.account');
	include_once('include/customer.account.inc');
	$account = new Customer_Account;
	//$vars['menuData'] = $account->initializeMenu();
	
	if(isset($_REQUEST['csid'])){
		$vars['csid'] = $_REQUEST['csid'];	
	}
	
	$template = "welcome.html";
	
	foreach($_REQUEST as $key=>$value){
		switch($key){

			case "change_password":
				$template = "change.password.html";
				break;
			case "update_password":
				$vars['message'] = $account->updatePassword();
				$template = "status.html";
				break;
				
			case "edit_billing":
				$vars['data'] = $account->getBillingAddress();
				$template = "edit.billing.html";
				break;
				
			case "update_billing":
				$vars['message'] = $account->updateBilling();
				$vars['data'] = $account->getBillingAddress();
				$template = "edit.billing.html";
				break;

			case "add_shipping":
				$vars['data'] = $account->addShippingAddress();
				$template = "edit.shipping.html";
				break;

			case "delete_shipping":
				$vars['message'] = $account->deleteShippingAddress();
				$vars['data'] = $account->getShippingAddress();
				if(count($vars['data']) > 0){
					$vars['shippingSelect'] = $account->makeShippingSelect();
				}
				$template = "edit.shipping.html";
				break;
				
			case "edit_shipping":
				$vars['data'] = $account->getShippingAddress();
				$vars['shippingSelect'] = $account->makeShippingSelect();
				$template = "edit.shipping.html";
				break;
				
			case "update_shipping":
				$vars['message'] = $account->updateShippingAddress();
				$vars['data'] = $account->getShippingAddress();
				$vars['shippingSelect'] = $account->makeShippingSelect();
				$template = "edit.shipping.html";
				break;
				
			case "order_history":
				$vars['records'] = $account->getOrderHistory();
				$template = "order.history.html";
				if(isset($_REQUEST['detail'])){
					$vars['order'] = $vars['records'][0];
					$template = "order.history.detail.html";
				}
				break;
				
			case "reorder":
				$account->doReorder();
				break;
				
		}
	}

	if($account->error){
		$vars['error'] = $account->error;
	}

	if($template == "welcome.html"){
		$vars['summary'] = $account->getSummary();
	}

	if(isset($_REQUEST['detail'])){
		$page = $_Template->Open("templates/customer.account/$template",$vars,false,false);
	}
	else{
		$vars['page'] = $_Template->Open("templates/customer.account/$template",$vars,false,true);
		$page = $_Template->Open("templates/customer.account/account.html",$vars,true);
	}
}
?>
<?=$page;?>


















