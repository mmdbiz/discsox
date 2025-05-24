<?php

class Logs{

    public bool $debug = false;
    
    function __construct(){
		
	}
	
	// -------------------------------------------------------------------
	function Save($vars = array()){
	
		global $_CART;
		global $_CF;
		global $_Common;
		global $_CR;
		global $ip_address;
		global $_Totals;

		$hostip = $ip_address;
		$_SESSION = array_merge($_SESSION,$vars);

//		$_Common->debugPrint($_SESSION);
//		exit;

		extract($vars);
		extract($_Totals);
		$format = $_CF['order logs']['log_format'];
		
		$billingFields = array();
		$shippingFields = array();
		foreach($_REQUEST as $key=>$value){
			if(substr($key,0,11) == "billaddress"){
				$billingFields[$key] = $value;
			}
			elseif(substr($key,0,11) == "shipaddress"){
				$shippingFields[$key] = $value;
			}
		}

		if(isset($_SESSION['required']) && count($_SESSION['required']) > 0){
			$paymentFields = $_SESSION['required'];
			foreach($paymentFields as $k=>$v){
				if(isset($_SESSION[$v])){
					$paymentFields[$v] = $_SESSION[$v];
				}
			}
		}

		if(isset($_REQUEST['comments'])){
			$_REQUEST['comments'] = str_replace('<br />',"\r\n",$_REQUEST['comments']);
			$_REQUEST['comments'] = htmlspecialchars($_REQUEST['comments'],ENT_QUOTES,'ISO-8859-1');
		}
		if(isset($_SESSION['comments'])){
			$_SESSION['comments'] = str_replace('<br />',"\r\n",$_SESSION['comments']);
			$_SESSION['comments'] = htmlspecialchars($_SESSION['comments'],ENT_QUOTES,'ISO-8859-1');
		}

		$results = null;
		
		switch($format){
		
			case "csv":
				error_reporting(E_PARSE|E_WARNING);
				ob_start();
				include_once("templates/logs/csv.log.php");
				$results = ob_get_contents();
				ob_end_clean();
				error_reporting(E_ALL);
				$orderFile = $customer_number . '.' . $order_number . ".csv";
				break;
				
			case "standard":
				error_reporting(E_PARSE|E_WARNING);
				ob_start();
				include_once("templates/logs/standard.log.php");
				$results = ob_get_contents();
				$results = preg_replace("/\r/","",$results);
				$results = preg_replace("/\n/",$_CR,$results);
				ob_end_clean();
				error_reporting(E_ALL);
				$orderFile = $customer_number . '.' . $order_number . ".log";
				break;
				
			case "xml":
				$xmlTemplate = "templates/logs/xml.log.php";
				$results = "<?xml version='1.0' encoding='ISO-8859-1' ?>$_CR";
				$results .= "<?Quikstore version='3.0' ?>$_CR";
				error_reporting(E_PARSE|E_WARNING);
				ob_start();
				include_once($xmlTemplate);
				$results .= ob_get_contents();
				ob_end_clean();
				error_reporting(E_ALL);
				$orderFile = $customer_number . '.' . $order_number . ".xml";
				break;
		}
		
		if($results){
			$this->_writeLogFile($orderFile,$results);
		}
		
	}
	// -----------------------------------------------------------------
	function _writeLogFile($orderFile,$data){

		global $_CF,$_CR,$_Registry;

		if($_CF['encryption']['encrypt_orders']){
			$crypt = $_Registry->loadClass('crypt');
			$data = $crypt->encrypt($data);
		}

		include_once("cp/include/ftp.inc");
		$ftp = new Ftp();
		$ftpRoot = $_CF['ftp']['document_root'];
		$ftp->ChDir($ftpRoot);

		if(!is_dir("./orders")){
			$ftp->Mkdir($ftpRoot, "orders", 755);
		}
		
		$ftp->ChDir("$ftpRoot/orders");
		
		if(!file_exists("./orders/.htaccess")){
			$htaccess = "<Limit GET POST>$_CR";
			$htaccess .= "deny from all$_CR";
			$htaccess .= "</Limit>$_CR";
			$ftp->writeFile(".htaccess",$htaccess);
		}
		if(!file_exists("./orders/index.html")){
			$index = "<html><head><title>403 Forbidden</title></head><body><h1>Forbidden</h1>$_CR";
			$index .= "<p>You don't have permission to access /orders on this server.</p>$_CR";
			$index .= "</body></html>$_CR";
			$ftp->writeFile("index.html",$index);
		}
		
		$ftp->writeFile($orderFile,$data);
		$ftp->Close();
	}
	
    
}

?>