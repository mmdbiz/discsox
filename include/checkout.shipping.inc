<?php
// -----------------------------------------------------------------------
//         Class: Shipping
//       Purpose: This class displays the shipping page.
// Last Modified: March 08, 2006 
//
// -----------------------------------------------------------------------
class Checkout_Shipping{

    var $debug = false;
    var $vars = array();
    //-----------------------------------------------------
	function __construct(){

		global $calcs;
		global $cart;
		global $_CART;
		global $_CF;
		global $_DB;
		global $_Common;
		global $_Registry;
		global $secure_url;
		global $_Totals;

		$cart->LoadCart();
		$cart->updateLastPage("shipping.php");

		if(isset($_Totals['totalWeight']) && $_Totals['totalWeight'] == 0){
			if($_CF['shipping']['hide_shipping_on_zero_weight']){
				$_CF['shipping']['require_shipping'] = false;
			}
		}

		// save the form fields for reload on the address page.
		foreach($_REQUEST as $key=>$value){
			if(substr($key,0,11) == "billaddress" || substr($key,0,11) == "shipaddress" || 
			   substr($key,0,12) == "registration" || $key == "email_list" || $key == "coupon"){
				$key = trim(str_replace(';','',$key));
				if(is_array($value)){
					$_SESSION[$key] = $value;
				}
				else{
					$_SESSION[$key] = trim($value);
				}
			}
		}

		$this->checkRegistration();

		if(count($_CART) > 0){
			$calcs = $_Registry->LoadClass('Calcs');
			if(count($calcs->shippingRateList) > 0){
				$this->vars['haveShippingRates'] = true;
				$this->vars['shippingRates'] = $calcs->shippingRateList;
				$this->vars['selectedCarrier'] = $calcs->carrier;
				$this->vars['freeShipping'] = $calcs->freeShipping;
				$this->vars['freeShippingText'] = $calcs->freeShippingText;
			}
			
			if($_Totals['grandtotal'] > 0){
				$this->getPaymentMethodPage();
			}
		}
		else{
			$_Common->printErrorMessage("Shopping Cart is Empty","Your Shopping Cart is Empty.");
		}
	}

	// --------------------------------------------------------------
	function checkRegistration(){
		
		global $_Common, $_DB;

		if(!empty($_REQUEST['registration']['username'])){
			$username = trim($_REQUEST['registration']['username']);
			$record = $_DB->getRecord("SELECT username FROM customers WHERE username = '$username'");
			if(count($record) > 0){
				$_Common->printErrorMessage("Registration Error","The login name you entered is already in use. Please back up and try again.");
			}
		}
	}

	//-----------------------------------------------------
	function getPaymentMethodPage(){
		
		global $_CF;
		global $_Common;
		global $_DB;
		global $_Registry;

		if(!empty($_REQUEST['payment_method'])){
			$paymentPage = $_Common->runSecurityCheck($_REQUEST['payment_method']);
		}
		elseif(!empty($_SESSION['payment_method'])){
			$paymentPage = $_SESSION['payment_method'];
		}
		else{
			if(!empty($_CF['credit_cards']['default_payment_method'])){
				$paymentPage = $_CF['credit_cards']['default_payment_method'];
			}
			else{
				$_Common->printErrorMessage("Checkout Form Missing","The Default Payment Method is not set in the config.");	
			}
		}
		
		$count = $_DB->getCount("payment_methods", "WHERE `method` = '$paymentPage' AND `active` = 'true'");
		
		if(file_exists("templates/payment_methods/$paymentPage") && $count > 0){
			$this->vars['defaultPaymentMethod'] = $paymentPage;
			$this->vars['cardTypes'] = array_keys($_DB->getRecords("SELECT `card_name` FROM `credit_cards` WHERE `active` = 'true'",'card_name'));
			$template = $_Registry->loadClass('template');
			$this->vars['paymentPage'] = $template->open("templates/payment_methods/$paymentPage",$this->vars,false,true);
			$_SESSION['payment_method'] = $paymentPage;
		}
		else{
			$_Common->printErrorMessage("Checkout Form Missing","The Checkout Form ($paymentPage) is Missing");
		}
	}

	
}
?>