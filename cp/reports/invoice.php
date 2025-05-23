<?php
$_isAdmin = true;
$_adminFunction = "reports";

// initialize the program, read the config(s) and set include paths
include_once("../../include/initialize.inc");
$init = new Initialize(true);

$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

$_Form = $_Registry->LoadClass("form");
include_once("include/orders.inc");

getDetailRecords();

$color = array();
$color[~0] = "#e2eDe2";
$color[0] = "#FFFFFF";
$ck = 0;
$payMethod = null;
if(!empty($order[0]['payment_method'])){
	@list($payMethod,$ext) = explode('.',$order[0]['payment_method']);
}

$emailSent = false;
$sendEmail = false;
if(isset($_REQUEST['send_email'])){
	$check = array();
	if(preg_match("/^[_\.0-9a-z-]+@([0-9a-z][-0-9a-z\.]+)\.([a-z]{2,3}$)/i",$billInfo['billaddress_email'],$check)){
		$sendEmail = true;
		error_reporting(E_PARSE|E_WARNING);
		ob_start();
		include("../templates/invoice.html");
		$html = ob_get_contents();
		ob_end_clean();
		// subject line comes from title tag of invoice.html template
		$subject = $_Template->getPageTitle($html);
		$email = $_Registry->loadClass('email');
		$text = parseString($html,'<div id="textEmail">','</div>');
		$email->send($_CF['email']['store_email_address'], $billInfo['billaddress_email'], $subject, $html, $text);
		if($email->sent){
			$emailSent = true;
		}
		error_reporting(E_ALL);
		$sendEmail = false;
	}	
}
# -------------------------------------------------------------------
function parseString($str,$left_side,$right_side){

     $start = strpos($str, $left_side);
     $start = $start + strlen($left_side);
       $end = strpos($str, $right_side, $start);
    $result = substr($str, $start, $end - $start);

return $result;
}

error_reporting(E_PARSE|E_WARNING);
ob_start();
include("../templates/invoice.html");
$template = ob_get_contents();
ob_end_clean();
error_reporting(E_ALL);
?>
<?=$template;?>
